/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "gz/common/graphics/Types.hh"
#include "gz/common/Console.hh"
#include "gz/common/Material.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/common/Util.hh"
#include "gz/common/AssimpLoader.hh"

#include <queue>
#include <unordered_set>

#include <assimp/Logger.hpp>      // C++ importer interface
#include <assimp/DefaultLogger.hpp>      // C++ importer interface
#include <assimp/GltfMaterial.h>      // GLTF specific material properties
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

namespace gz
{
namespace common
{

/// \brief Private data for the AssimpLoader class
class AssimpLoader::Implementation
{
  public: Assimp::Importer importer;
  /// Convert a color from assimp implementation to Ignition common
  public: math::Color ConvertColor(aiColor4D& _color);

  /// Convert a transformation from assimp implementation to Ignition math
  public: math::Matrix4d ConvertTransform(const aiMatrix4x4& _matrix);

  public: MaterialPtr CreateMaterial(const aiScene* _scene, unsigned _matIdx, const std::string& _path);

  public: void LoadEmbeddedTexture(const aiTexture* _texture);

  public: SubMesh CreateSubMesh(const aiMesh* _assimpMesh, const math::Matrix4d& _transform);

  public: std::unordered_map<std::string, math::Matrix4d> PopulateTransformMap(const aiScene* _scene);

  public: void RecursiveCreate(const aiScene* _scene, const aiNode* _node, const math::Matrix4d& _transform, Mesh* _mesh);

  public: void RecursiveSkeletonCreate(const aiNode* _node, SkeletonNode* _parent, const math::Matrix4d& _transform);

  /// \brief Apply the the inv bind transform to the skeleton pose.
  /// \remarks have to set the model transforms starting from the root in
  /// breadth first order. Because setting the model transform also updates
  /// the transform based on the parent's inv model transform. Setting the
  /// child before the parent results in the child's transform being
  /// calculated from the "old" parent model transform.
  /// \param[in] _skeleton the skeleton to work on
  public: void ApplyInvBindTransform(SkeletonPtr _skeleton);
};

math::Color AssimpLoader::Implementation::ConvertColor(aiColor4D& _color)
{
  math::Color col(_color.r, _color.g, _color.b, _color.a);
  return col;
}

math::Matrix4d AssimpLoader::Implementation::ConvertTransform(const aiMatrix4x4& _sm)
{
  return math::Matrix4d(
      _sm.a1, _sm.a2, _sm.a3, _sm.a4,
      _sm.b1, _sm.b2, _sm.b3, _sm.b4,
      _sm.c1, _sm.c2, _sm.c3, _sm.c4,
      _sm.d1, _sm.d2, _sm.d3, _sm.d4);
}

void AssimpLoader::Implementation::RecursiveCreate(const aiScene* _scene, const aiNode* _node, const math::Matrix4d& _transform, Mesh* _mesh)
{
  if (!_node)
    return;
  // Visit this node, add the submesh
  ignmsg << "Processing node " << _node->mName.C_Str() << " with " << _node->mNumMeshes << " meshes" << std::endl;
  for (unsigned meshIdx = 0; meshIdx < _node->mNumMeshes; ++meshIdx)
  {
    auto assimpMeshIdx = _node->mMeshes[meshIdx];
    auto assimpMesh = _scene->mMeshes[assimpMeshIdx];
    auto nodeName = std::string(_node->mName.C_Str());
    auto subMesh = this->CreateSubMesh(assimpMesh, _transform);
    subMesh.SetName(nodeName);
    // Now add the bones to the skeleton
    if (assimpMesh->HasBones() && _scene->HasAnimations())
    {
      // TODO merging skeletons here
      auto skeleton = _mesh->MeshSkeleton();
      // TODO Append to existing skeleton if multiple submeshes?
      skeleton->SetNumVertAttached(subMesh.VertexCount());
      // Now add the bone weights
      for (unsigned boneIdx = 0; boneIdx < _scene->mMeshes[meshIdx]->mNumBones; ++boneIdx)
      {
        auto bone = assimpMesh->mBones[boneIdx];
        auto boneNodeName = std::string(bone->mNode->mName.C_Str());
        // Apply inverse bind transform to the matching node
        SkeletonNode *skelNode =
            skeleton->NodeByName(boneNodeName);
        skelNode->SetInverseBindTransform(this->ConvertTransform(bone->mOffsetMatrix));
        igndbg << "Bone " << boneNodeName << " has " << bone->mNumWeights << " weights" << std::endl;
        for (unsigned weightIdx = 0; weightIdx < bone->mNumWeights; ++weightIdx)
        {
          auto vertexWeight = bone->mWeights[weightIdx];
          // TODO SetNumVertAttached for performance
          skeleton->AddVertNodeWeight(vertexWeight.mVertexId, boneNodeName, vertexWeight.mWeight);
          //igndbg << "Adding weight at idx " << vertexWeight.mVertexId << " for bone " << bone_name << " of " << vertexWeight.mWeight << std::endl;
        }
      }
      // Add node assignment to mesh
      ignmsg << "submesh has " << subMesh.VertexCount() << " vertices" << std::endl;
      for (unsigned vertexIdx = 0; vertexIdx < subMesh.VertexCount(); ++vertexIdx)
      {
        //ignmsg << "skel at id " << vertexIdx << " has " << skel->VertNodeWeightCount(vertexIdx) << " indices" << std::endl;
        for (unsigned int i = 0;
            i < skeleton->VertNodeWeightCount(vertexIdx); ++i)
        {
          std::pair<std::string, double> nodeWeight =
            skeleton->VertNodeWeight(vertexIdx, i);
          SkeletonNode *node =
              skeleton->NodeByName(nodeWeight.first);
          if (node == nullptr)
          {
            igndbg << "Not found while Looking for node with name " << nodeWeight.first << std::endl;
          }
          subMesh.AddNodeAssignment(vertexIdx,
                          node->Handle(), nodeWeight.second);
          //igndbg << "Adding node assignment for vertex " << vertexIdx << " to node " << node->Name() << " of weight " << nodeWeight.second << std::endl;
        }
      }
    }
    _mesh->AddSubMesh(std::move(subMesh));
  }

  // Iterate over children
  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    // Calculate the transform
    auto child_node = _node->mChildren[childIdx];
    auto nodeTrans = this->ConvertTransform(child_node->mTransformation);
    nodeTrans = _transform * nodeTrans;

    // Finally recursive call to explore subnode
    this->RecursiveCreate(_scene, child_node, nodeTrans, _mesh);
  }
}

void AssimpLoader::Implementation::RecursiveSkeletonCreate(const aiNode* _node, SkeletonNode* _parent, const math::Matrix4d& _transform)
{
  // First explore this node
  auto nodeName = std::string(_node->mName.C_Str());
  //ignmsg << "Exploring node " << nodeName << std::endl;
  // TODO check if node or joint?
  auto skelNode = new SkeletonNode(_parent, nodeName, nodeName, SkeletonNode::JOINT);
  // Calculate transform
  auto nodeTrans = this->ConvertTransform(_node->mTransformation);
  skelNode->SetTransform(nodeTrans);
  nodeTrans = _transform * nodeTrans;
  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    this->RecursiveSkeletonCreate(_node->mChildren[childIdx], skelNode, nodeTrans);
  }
}

//////////////////////////////////////////////////
MaterialPtr AssimpLoader::Implementation::CreateMaterial(const aiScene* _scene, unsigned _matIdx, const std::string& _path)
{
  MaterialPtr mat = std::make_shared<Material>();
  aiColor4D color;
  auto& assimpMat = _scene->mMaterials[_matIdx];
  //igndbg << "Processing material with name " << assimpMat->GetName().C_Str() << std::endl;
  auto ret = assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetDiffuse(this->ConvertColor(color));
  }
  ret = assimpMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetAmbient(this->ConvertColor(color));
  }
  ret = assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetSpecular(this->ConvertColor(color));
  }
  ret = assimpMat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetEmissive(this->ConvertColor(color));
  }
  double shininess;
  ret = assimpMat->Get(AI_MATKEY_SHININESS, shininess);
  if (ret == AI_SUCCESS)
  {
    mat->SetShininess(shininess);
  }
  // TODO more than one texture, Gazebo assumes UV index 0
  aiString texturePath(_path.c_str());
  ret = assimpMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
  // TODO check other arguments, type of mappings to be UV, uv index, blend mode
  if (ret == AI_SUCCESS)
  {
    // Check if the texture is embedded or not
    auto embeddedTexture = _scene->GetEmbeddedTexture(texturePath.C_Str());
    if (embeddedTexture)
    {
      // Load embedded texture
      this->LoadEmbeddedTexture(embeddedTexture);
    }
    else
    {
      mat->SetTextureImage(std::string(texturePath.C_Str()), _path.c_str());
    }
    // Now set the alpha from texture, if enabled, only supported in GLTF
    aiString alphaMode;
    auto paramRet = assimpMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    if (paramRet == AI_SUCCESS)
    {
      // Only enable if it's set to MASK, BLEND not supported yet
      if (strcmp(alphaMode.C_Str(), "MASK") == 0)
      {
        double alphaCutoff = mat->AlphaThreshold();
        bool twoSided = mat->TwoSidedEnabled();
        // Ignore return value, parameter unchanged if value is not set
        assimpMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
        assimpMat->Get(AI_MATKEY_TWOSIDED, twoSided);
        mat->SetAlphaFromTexture(true, alphaCutoff, twoSided);
      }
    }
  }
  ret = assimpMat->GetTexture(aiTextureType_METALNESS, 0, &texturePath);
  Pbr pbr;
  if (ret == AI_SUCCESS)
  {
    pbr.SetMetalnessMap(std::string(texturePath.C_Str()));
  }
  ret = assimpMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath);
  if (ret == AI_SUCCESS)
  {
    pbr.SetNormalMap(std::string(texturePath.C_Str()));
  }
  double value;
  ret = assimpMat->Get(AI_MATKEY_METALLIC_FACTOR, value);
  if (ret == AI_SUCCESS)
  {
    pbr.SetMetalness(value);
  }
  ret = assimpMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value);
  if (ret == AI_SUCCESS)
  {
    pbr.SetRoughness(value);
  }
  mat->SetPbrMaterial(pbr);
  // TODO other properties
  return mat;
}

//////////////////////////////////////////////////
void AssimpLoader::Implementation::LoadEmbeddedTexture(const aiTexture* _texture)
{
  if (_texture->mHeight == 0)
  {
    ignwarn << "Found not supported compressed format " << _texture->achFormatHint << std::endl;
    return;
  }
  ignmsg << "Processing texture in format " << _texture->achFormatHint << std::endl;
}

SubMesh AssimpLoader::Implementation::CreateSubMesh(const aiMesh* _assimpMesh, const math::Matrix4d& _transform)
{
  SubMesh subMesh;
  math::Matrix4d rot = _transform;
  rot.SetTranslation(math::Vector3d::Zero);
  ignmsg << "Mesh has " << _assimpMesh->mNumVertices << " vertices" << std::endl;
  // Now create the submesh
  for (unsigned vertexIdx = 0; vertexIdx < _assimpMesh->mNumVertices; ++vertexIdx)
  {
    // Add the vertex
    math::Vector3d vertex;
    math::Vector3d normal;
    vertex.X(_assimpMesh->mVertices[vertexIdx].x);
    vertex.Y(_assimpMesh->mVertices[vertexIdx].y);
    vertex.Z(_assimpMesh->mVertices[vertexIdx].z);
    normal.X(_assimpMesh->mNormals[vertexIdx].x);
    normal.Y(_assimpMesh->mNormals[vertexIdx].y);
    normal.Z(_assimpMesh->mNormals[vertexIdx].z);
    vertex = _transform * vertex;
    normal = rot * normal;
    normal.Normalize();
    subMesh.AddVertex(vertex);
    subMesh.AddNormal(normal);
    // Iterate over sets of texture coordinates
    int uvIdx = 0;
    while(_assimpMesh->HasTextureCoords(uvIdx))
    {
      math::Vector3d texcoords;
      texcoords.X(_assimpMesh->mTextureCoords[uvIdx][vertexIdx].x);
      texcoords.Y(_assimpMesh->mTextureCoords[uvIdx][vertexIdx].y);
      // TODO why do we need 1.0 - Y?
      subMesh.AddTexCoordBySet(texcoords.X(), 1.0 - texcoords.Y(), uvIdx);
      ++uvIdx;
    }
  }
  for (unsigned faceIdx = 0; faceIdx < _assimpMesh->mNumFaces; ++faceIdx)
  {
    auto face = _assimpMesh->mFaces[faceIdx];
    subMesh.AddIndex(face.mIndices[0]);
    subMesh.AddIndex(face.mIndices[1]);
    subMesh.AddIndex(face.mIndices[2]);
  }
  subMesh.SetMaterialIndex(_assimpMesh->mMaterialIndex);
  return subMesh;
}

//////////////////////////////////////////////////
AssimpLoader::AssimpLoader()
: MeshLoader(), dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  // TODO: remove logger from stdout
  Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);
  this->dataPtr->importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
}

//////////////////////////////////////////////////
AssimpLoader::~AssimpLoader()
{
}

//////////////////////////////////////////////////
Mesh *AssimpLoader::Load(const std::string &_filename)
{
  // TODO share importer
  Mesh *mesh = new Mesh();
  std::string path = common::parentPath(_filename);
  // Load the asset, TODO check if we need to do preprocessing
  const aiScene* scene = this->dataPtr->importer.ReadFile(_filename,
      aiProcess_JoinIdenticalVertices |
      aiProcess_RemoveRedundantMaterials |
      aiProcess_SortByPType |
      aiProcess_PopulateArmatureData |
      aiProcess_Triangulate |
      aiProcess_GenNormals |
      0);
  if (scene == nullptr)
  {
    ignerr << "Unable to import mesh [" << _filename << "]" << std::endl;
    return mesh;
  }
  auto rootNode = scene->mRootNode;
  auto rootName = std::string(rootNode->mName.C_Str());
  auto transform = scene->mRootNode->mTransformation;
  aiVector3D rootScaling, rootAxis, rootPos;
  float angle;
  transform.Decompose(rootScaling, rootAxis, angle, rootPos);
  // drop rotation, but keep scaling and position
  transform = aiMatrix4x4(rootScaling, aiQuaternion(), rootPos);

  auto rootTransform = this->dataPtr->ConvertTransform(transform);

  // TODO remove workaround, it seems imported assets are rotated by 90 degrees
  // as documented here https://github.com/assimp/assimp/issues/849, remove workaround when fixed
  // Add the materials first
  for (unsigned _matIdx = 0; _matIdx < scene->mNumMaterials; ++_matIdx)
  {
    auto mat = this->dataPtr->CreateMaterial(scene, _matIdx, path);
    mesh->AddMaterial(mat);
  }
  if (scene->HasAnimations())
  {
    auto rootSkelNode = new SkeletonNode(nullptr, rootName, rootName, SkeletonNode::NODE);
    rootSkelNode->SetTransform(rootTransform);
    rootSkelNode->SetModelTransform(rootTransform);
    for (unsigned childIdx = 0; childIdx < rootNode->mNumChildren; ++childIdx)
    {
      // First populate the skeleton with the node transforms
      // TODO parse different skeletons and merge them
      this->dataPtr->RecursiveSkeletonCreate(rootNode->mChildren[childIdx], rootSkelNode, rootTransform);
    }
    SkeletonPtr rootSkeleton = std::make_shared<Skeleton>(rootSkelNode);
    mesh->SetSkeleton(rootSkeleton);
  }
  // Now create the meshes
  // Recursive call to keep track of transforms, mesh is passed by reference and edited throughout
  this->dataPtr->RecursiveCreate(scene, rootNode, rootTransform, mesh);
  // Add the animations
  for (unsigned animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
  {
    auto anim = scene->mAnimations[animIdx];
    auto animName = std::string(anim->mName.C_Str());
    ignmsg << "Found animation with name " << animName << std::endl;
    ignmsg << "Animation has " << anim->mNumMeshChannels << " mesh channels" << std::endl;
    ignmsg << "Animation has " << anim->mNumChannels << " channels" << std::endl;
    ignmsg << "Animation has " << anim->mNumMorphMeshChannels << " morph mesh channels" << std::endl;
    SkeletonAnimation* skelAnim = new SkeletonAnimation(animName);
    for (unsigned chanIdx = 0; chanIdx < anim->mNumChannels; ++chanIdx)
    {
      auto animChan = anim->mChannels[chanIdx];
      auto chanName = std::string(animChan->mNodeName.C_Str());
      igndbg << "Node " << chanName << " has " << animChan->mNumPositionKeys << " position keys, " <<
        animChan->mNumRotationKeys << " rotation keys, " << animChan->mNumScalingKeys << " scaling keys" << std::endl;
      for (unsigned key_idx = 0; key_idx < animChan->mNumPositionKeys; ++key_idx)
      {
        // Note, Scaling keys are not supported right now
        // Compute the position into a math pose
        auto posKey = animChan->mPositionKeys[key_idx];
        auto quatKey = animChan->mRotationKeys[key_idx];
        math::Vector3d pos(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
        math::Quaterniond quat(quatKey.mValue.w, quatKey.mValue.x, quatKey.mValue.y, quatKey.mValue.z);
        math::Pose3d pose(pos, quat);
        // Time is in ms after 5.0.1?
        skelAnim->AddKeyFrame(chanName, posKey.mTime / 1000.0, pose);
        igndbg << "Adding animation at time " << posKey.mTime / 1000.0 << " with position (" << pos.X() << "," << pos.Y() << "," <<
          pos.Z() << ")" << std::endl;
      }
    }
    mesh->MeshSkeleton()->AddAnimation(skelAnim);
  }

  if (mesh->HasSkeleton())
    this->dataPtr->ApplyInvBindTransform(mesh->MeshSkeleton());

  return mesh;
}

/////////////////////////////////////////////////
void AssimpLoader::Implementation::ApplyInvBindTransform(SkeletonPtr _skeleton)
{
  std::list<SkeletonNode *> queue;
  queue.push_back(_skeleton->RootNode());

  while (!queue.empty())
  {
    SkeletonNode *node = queue.front();
    queue.pop_front();
    if (nullptr == node)
      continue;

    if (node->HasInvBindTransform())
    {
      node->SetModelTransform(node->InverseBindTransform().Inverse(), false);
      //igndbg << "Node " << node->Name() << " model transform is:" << std::endl << node->ModelTransform() << std::endl;
      //igndbg << "Parent " << node->Parent()->Name() << " transform is:" << std::endl << node->Parent()->ModelTransform() << std::endl;
      //igndbg << "Node " << node->Name() << " transform is:" << std::endl << node->Transform() << std::endl;
    }
    for (unsigned int i = 0; i < node->ChildCount(); i++)
      queue.push_back(node->Child(i));
  }
}

}
}
