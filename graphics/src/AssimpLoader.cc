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
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

namespace ignition
{
namespace common
{

/// \brief Private data for the AssimpLoader class
class AssimpLoader::Implementation
{
  /// Convert a color from assimp implementation to Ignition common
  public: ignition::math::Color ConvertColor(aiColor4D& _color);

  /// Convert a transformation from assimp implementation to Ignition math
  public: ignition::math::Matrix4d ConvertTransform(const aiMatrix4x4& _matrix);

  public: MaterialPtr CreateMaterial(const aiScene* scene, unsigned mat_idx, const std::string& path);

  public: SubMesh CreateSubMesh(const aiMesh* assimp_mesh, const ignition::math::Matrix4d& transformation);

  public: std::unordered_map<std::string, ignition::math::Matrix4d> PopulateTransformMap(const aiScene* scene);

  public: void RecursiveCreate(const aiScene* scene, const aiNode* node, const ignition::math::Matrix4d& transformation, Mesh* mesh);

  public: void RecursiveSkeletonCreate(const aiNode* node, SkeletonNode* root_node, const ignition::math::Matrix4d& transformation);

  /// \brief Apply the the inv bind transform to the skeleton pose.
  /// \remarks have to set the model transforms starting from the root in
  /// breadth first order. Because setting the model transform also updates
  /// the transform based on the parent's inv model transform. Setting the
  /// child before the parent results in the child's transform being
  /// calculated from the "old" parent model transform.
  /// \param[in] _skeleton the skeleton to work on
  public: void ApplyInvBindTransform(SkeletonPtr _skeleton);
};

ignition::math::Color AssimpLoader::Implementation::ConvertColor(aiColor4D& _color)
{
  ignition::math::Color col(_color.r, _color.g, _color.b, _color.a);
  return col;
}

ignition::math::Matrix4d AssimpLoader::Implementation::ConvertTransform(const aiMatrix4x4& _sm)
{
  return ignition::math::Matrix4d(
      _sm.a1, _sm.a2, _sm.a3, _sm.a4,
      _sm.b1, _sm.b2, _sm.b3, _sm.b4,
      _sm.c1, _sm.c2, _sm.c3, _sm.c4,
      _sm.d1, _sm.d2, _sm.d3, _sm.d4);
}

void AssimpLoader::Implementation::RecursiveCreate(const aiScene* scene, const aiNode* node, const ignition::math::Matrix4d& transformation, Mesh* mesh)
{
  if (!node)
    return;

  // Visit this node, add the submesh
  ignmsg << "Processing node " << node->mName.C_Str() << " with " << node->mNumMeshes << " meshes" << std::endl;
  for (unsigned mesh_idx = 0; mesh_idx < node->mNumMeshes; ++mesh_idx)
  {
    auto assimp_mesh_idx = node->mMeshes[mesh_idx];
    auto assimp_mesh = scene->mMeshes[assimp_mesh_idx];
    auto node_name = std::string(node->mName.C_Str());
    auto subMesh = this->CreateSubMesh(assimp_mesh, transformation);
    subMesh.SetName(node_name);
    // Now add the bones to the skeleton
    if (assimp_mesh->HasBones() && scene->HasAnimations())
    {
      // TODO merging skeletons here
      auto skeleton = mesh->MeshSkeleton();
      auto skel_root_node = skeleton->RootNode();
      // TODO Append to existing skeleton if multiple submeshes?
      skeleton->SetNumVertAttached(subMesh.VertexCount());
      // Now add the bone weights
      for (unsigned bone_idx = 0; bone_idx < scene->mMeshes[mesh_idx]->mNumBones; ++bone_idx)
      {
        auto bone = assimp_mesh->mBones[bone_idx];
        auto bone_name = std::string(bone->mName.C_Str());
        // Apply inverse bind transform to the matching node
        SkeletonNode *skel_node =
            mesh->MeshSkeleton()->NodeByName(bone_name);
        skel_node->SetInverseBindTransform(this->ConvertTransform(bone->mOffsetMatrix));
        igndbg << "Bone " << bone_name << " has " << bone->mNumWeights << " weights" << std::endl;
        for (unsigned weight_idx = 0; weight_idx < bone->mNumWeights; ++weight_idx)
        {
          auto vertex_weight = bone->mWeights[weight_idx];
          // TODO SetNumVertAttached for performance
          skeleton->AddVertNodeWeight(vertex_weight.mVertexId, bone_name, vertex_weight.mWeight);
          //igndbg << "Adding weight at idx " << vertex_weight.mVertexId << " for bone " << bone_name << " of " << vertex_weight.mWeight << std::endl;
        }
      }
      // Add node assignment to mesh
      ignmsg << "submesh has " << subMesh.VertexCount() << " vertices" << std::endl;
      for (unsigned vert_idx = 0; vert_idx < subMesh.VertexCount(); ++vert_idx)
      {
        //ignmsg << "skel at id " << vert_idx << " has " << skel->VertNodeWeightCount(vert_idx) << " indices" << std::endl;
        for (unsigned int i = 0;
            i < skeleton->VertNodeWeightCount(vert_idx); ++i)
        {
          std::pair<std::string, double> node_weight =
            skeleton->VertNodeWeight(vert_idx, i);
          SkeletonNode *node =
              mesh->MeshSkeleton()->NodeByName(node_weight.first);
          if (node == nullptr)
          {
            igndbg << "Not found while Looking for node with name " << node_weight.first << std::endl;
          }
          subMesh.AddNodeAssignment(vert_idx,
                          node->Handle(), node_weight.second);
          //igndbg << "Adding node assignment for vertex " << vert_idx << " to node " << node->Name() << " of weight " << node_weight.second << std::endl;
        }
      }
    }
    mesh->AddSubMesh(std::move(subMesh));
  }

  // Iterate over children
  for (unsigned child_idx = 0; child_idx < node->mNumChildren; ++child_idx)
  {
    // Calculate the transform
    auto child_node = node->mChildren[child_idx];
    auto node_trans = this->ConvertTransform(child_node->mTransformation);
    node_trans = transformation * node_trans;

    // Finally recursive call to explore subnode
    this->RecursiveCreate(scene, child_node, node_trans, mesh);
  }
}

void AssimpLoader::Implementation::RecursiveSkeletonCreate(const aiNode* node, SkeletonNode* root_node, const ignition::math::Matrix4d& transformation)
{
  // First explore this node
  auto node_name = std::string(node->mName.C_Str());
  //ignmsg << "Exploring node " << node_name << std::endl;
  // TODO check if node or joint?
  auto skel_node = new SkeletonNode(root_node, node_name, node_name, SkeletonNode::JOINT);
  // Calculate transform
  auto node_trans = this->ConvertTransform(node->mTransformation);
  igndbg << node_trans << std::endl;
  skel_node->SetTransform(node_trans);
  //skel_node->SetModelTransform(node_trans.Inverse(), false);
  node_trans = transformation * node_trans;

  // TODO Set the vertex weights
  for (unsigned child_idx = 0; child_idx < node->mNumChildren; ++child_idx)
  {
    this->RecursiveSkeletonCreate(node->mChildren[child_idx], skel_node, node_trans);
  }
}

//////////////////////////////////////////////////
MaterialPtr AssimpLoader::Implementation::CreateMaterial(const aiScene* scene, unsigned mat_idx, const std::string& path)
{
  MaterialPtr mat = std::make_shared<Material>();
  aiColor4D color;
  igndbg << "Processing material with name " << scene->mMaterials[mat_idx]->GetName().C_Str() << std::endl;
  auto ret = scene->mMaterials[mat_idx]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetDiffuse(this->ConvertColor(color));
  }
  ret = scene->mMaterials[mat_idx]->Get(AI_MATKEY_COLOR_AMBIENT, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetAmbient(this->ConvertColor(color));
  }
  ret = scene->mMaterials[mat_idx]->Get(AI_MATKEY_COLOR_SPECULAR, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetSpecular(this->ConvertColor(color));
  }
  ret = scene->mMaterials[mat_idx]->Get(AI_MATKEY_COLOR_EMISSIVE, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetEmissive(this->ConvertColor(color));
  }
  double shininess;
  ret = scene->mMaterials[mat_idx]->Get(AI_MATKEY_SHININESS, shininess);
  if (ret == AI_SUCCESS)
  {
    mat->SetShininess(shininess);
  }
  // TODO more than one texture
  aiString texturePath(path.c_str());
  unsigned textureIndex = 0;
  unsigned uvIndex = 10000;
  ret = scene->mMaterials[mat_idx]->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath,
      NULL, // Type of mapping, TODO check that it is UV
      &uvIndex,
      NULL, // Blend mode, TODO implement
      NULL, // Texture operation, unneeded?
      NULL); // Mapping modes, unneeded?
  if (ret == AI_SUCCESS)
  {
    mat->SetTextureImage(std::string(texturePath.C_Str()), path.c_str());
    if (uvIndex < 10000)
    {

    }
  }
  // TODO other properties
  return mat;
}

SubMesh AssimpLoader::Implementation::CreateSubMesh(const aiMesh* assimp_mesh, const ignition::math::Matrix4d& transformation)
{
  SubMesh subMesh;
  ignition::math::Matrix4d rot = transformation;
  rot.SetTranslation(ignition::math::Vector3d::Zero);
  ignmsg << "Mesh has " << assimp_mesh->mNumVertices << " vertices" << std::endl;
  // Now create the submesh
  for (unsigned vertex_idx = 0; vertex_idx < assimp_mesh->mNumVertices; ++vertex_idx)
  {
    // Add the vertex
    ignition::math::Vector3d vertex;
    ignition::math::Vector3d normal;
    vertex.X(assimp_mesh->mVertices[vertex_idx].x);
    vertex.Y(assimp_mesh->mVertices[vertex_idx].y);
    vertex.Z(assimp_mesh->mVertices[vertex_idx].z);
    normal.X(assimp_mesh->mNormals[vertex_idx].x);
    normal.Y(assimp_mesh->mNormals[vertex_idx].y);
    normal.Z(assimp_mesh->mNormals[vertex_idx].z);
    vertex = transformation * vertex;
    normal = rot * normal;
    normal.Normalize();
    subMesh.AddVertex(vertex);
    subMesh.AddNormal(normal);
    // Iterate over sets of texture coordinates
    int i = 0;
    while(assimp_mesh->HasTextureCoords(i))
    {
      ignition::math::Vector3d texcoords;
      texcoords.X(assimp_mesh->mTextureCoords[i][vertex_idx].x);
      texcoords.Y(assimp_mesh->mTextureCoords[i][vertex_idx].y);
      // TODO why do we need 1.0 - Y?
      subMesh.AddTexCoordBySet(texcoords.X(), 1.0 - texcoords.Y(), i);
      ++i;
    }
  }
  for (unsigned face_idx = 0; face_idx < assimp_mesh->mNumFaces; ++face_idx)
  {
    auto face = assimp_mesh->mFaces[face_idx];
    subMesh.AddIndex(face.mIndices[0]);
    subMesh.AddIndex(face.mIndices[1]);
    subMesh.AddIndex(face.mIndices[2]);
  }

  subMesh.SetMaterialIndex(assimp_mesh->mMaterialIndex);
  return subMesh;
}

//////////////////////////////////////////////////
AssimpLoader::AssimpLoader()
: MeshLoader(), dataPtr(ignition::utils::MakeImpl<Implementation>())
{
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
  Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);
  Assimp::Importer importer;
  importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
  // Load the asset, TODO check if we need to do preprocessing
  const aiScene* scene = importer.ReadFile(_filename,
      //aiProcess_JoinIdenticalVertices |
      aiProcess_RemoveRedundantMaterials |
      aiProcess_SortByPType |
      aiProcess_Triangulate |
      0);
  if (scene == nullptr)
  {
    ignerr << "Unable to import mesh [" << _filename << "]" << std::endl;
    return mesh;
  }
  auto root_node = scene->mRootNode;
  auto root_name = std::string(root_node->mName.C_Str());
  auto transform = scene->mRootNode->mTransformation;
  aiVector3D scaling, axis, pos;
  float angle;
  transform.Decompose(scaling, axis, angle, pos);
  // drop rotation, but keep scaling and position
  transform = aiMatrix4x4(scaling, aiQuaternion(), pos);

  auto root_transformation = this->dataPtr->ConvertTransform(transform);

  // TODO remove workaround, it seems imported assets are rotated by 90 degrees
  // as documented here https://github.com/assimp/assimp/issues/849, remove workaround when fixed
  // Add the materials first
  for (unsigned mat_idx = 0; mat_idx < scene->mNumMaterials; ++mat_idx)
  {
    auto mat = this->dataPtr->CreateMaterial(scene, mat_idx, path);
    mesh->AddMaterial(mat);
  }
  auto root_skel_node = new SkeletonNode(nullptr, root_name, root_name, SkeletonNode::NODE);
  root_skel_node->SetTransform(root_transformation);
  root_skel_node->SetModelTransform(root_transformation);
  for (unsigned child_idx = 0; child_idx < root_node->mNumChildren; ++child_idx)
  {
    // First populate the skeleton with the node transforms
    // TODO parse different skeletons and merge them
    this->dataPtr->RecursiveSkeletonCreate(root_node->mChildren[child_idx], root_skel_node, root_transformation);
  }
  SkeletonPtr root_skeleton = std::make_shared<Skeleton>(root_skel_node);
  mesh->SetSkeleton(root_skeleton);

  // Now create the meshes
  // Recursive call to keep track of transforms, mesh is passed by reference and edited throughout
  this->dataPtr->RecursiveCreate(scene, root_node, root_transformation, mesh);

  // Add the animations
  for (unsigned anim_idx = 0; anim_idx < scene->mNumAnimations; ++anim_idx)
  {
    auto anim = scene->mAnimations[anim_idx];
    auto anim_name = std::string(anim->mName.C_Str());
    ignmsg << "Found animation with name " << anim_name << std::endl;
    ignmsg << "Animation has " << anim->mNumMeshChannels << " mesh channels" << std::endl;
    ignmsg << "Animation has " << anim->mNumChannels << " channels" << std::endl;
    ignmsg << "Animation has " << anim->mNumMorphMeshChannels << " morph mesh channels" << std::endl;
    SkeletonAnimation* skel_anim = new SkeletonAnimation(anim_name);
    for (unsigned chan_idx = 0; chan_idx < anim->mNumChannels; ++chan_idx)
    {
      auto anim_chan = anim->mChannels[chan_idx];
      auto chan_name = std::string(anim_chan->mNodeName.C_Str());
      auto skel_ptr = mesh->MeshSkeleton();
      igndbg << "Node " << chan_name << " has " << anim_chan->mNumPositionKeys << " position keys, " <<
        anim_chan->mNumRotationKeys << " rotation keys, " << anim_chan->mNumScalingKeys << " scaling keys" << std::endl;
      for (unsigned key_idx = 0; key_idx < anim_chan->mNumPositionKeys; ++key_idx)
      {
        // Note, Scaling keys are not supported right now
        //void SkeletonAnimation::AddKeyFrame(const std::string &_node,
        //const double _time, const math::Pose3d &_pose)
        // Compute the position into a ignition math pose
        auto pos_key = anim_chan->mPositionKeys[key_idx];
        auto quat_key = anim_chan->mRotationKeys[key_idx];
        ignition::math::Vector3d pos(pos_key.mValue.x, pos_key.mValue.y, pos_key.mValue.z);
        ignition::math::Quaterniond quat(quat_key.mValue.w, quat_key.mValue.x, quat_key.mValue.y, quat_key.mValue.z);
        ignition::math::Pose3d pose(pos, quat);
        // Time is in ms after 5.0.1?
        skel_anim->AddKeyFrame(chan_name, pos_key.mTime / 1000.0, pose);
        igndbg << "Adding animation at time " << pos_key.mTime / 1000.0 << " with position (" << pos.X() << "," << pos.Y() << "," <<
          pos.Z() << ")" << std::endl;
      }
    }
    mesh->MeshSkeleton()->AddAnimation(skel_anim);
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
