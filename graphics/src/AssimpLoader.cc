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
  public: ignition::math::Matrix4d ConvertTransform(aiMatrix4x4& _matrix);

  public: MaterialPtr CreateMaterial(const aiScene* scene, unsigned mat_idx, const std::string& path);

  public: SubMesh CreateSubMesh(const aiScene* scene, unsigned mesh_idx, const ignition::math::Matrix4d& transformation);
};

ignition::math::Color AssimpLoader::Implementation::ConvertColor(aiColor4D& _color)
{
  ignition::math::Color col(_color.r, _color.g, _color.b, _color.a);
  return col;
}

ignition::math::Matrix4d AssimpLoader::Implementation::ConvertTransform(aiMatrix4x4& _sm)
{
  return ignition::math::Matrix4d(
      _sm.a1, _sm.a2, _sm.a3, _sm.a4,
      _sm.b1, _sm.b2, _sm.b3, _sm.b4,
      _sm.c1, _sm.c2, _sm.c3, _sm.c4,
      _sm.d1, _sm.d2, _sm.d3, _sm.d4);
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

SubMesh AssimpLoader::Implementation::CreateSubMesh(const aiScene* scene, unsigned mesh_idx, const ignition::math::Matrix4d& transformation)
{
  SubMesh subMesh;
  auto assimp_mesh = scene->mMeshes[mesh_idx];
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
  auto root_transformation = this->dataPtr->ConvertTransform(scene->mRootNode->mTransformation);
  root_transformation = ignition::math::Matrix4d::Identity;
  // TODO remove workaround, it seems imported assets are rotated by 90 degrees
  // as documented here https://github.com/assimp/assimp/issues/849, remove workaround when fixed
  // TODO find actual workaround to remove rotation
  // root_transformation = root_transformation * root_transformation.Rotation();
  // TODO recursive call for children?
  // Add the materials first
  for (unsigned mat_idx = 0; mat_idx < scene->mNumMaterials; ++mat_idx)
  {
    auto mat = this->dataPtr->CreateMaterial(scene, mat_idx, path);
    mesh->AddMaterial(mat);
  }
  for (unsigned node_idx = 0; node_idx < root_node->mNumChildren; ++node_idx)
  {
    auto node = root_node->mChildren[node_idx];
    auto trans = this->dataPtr->ConvertTransform(node->mTransformation);
    trans = root_transformation * trans;
    // TODO 90 degree rotation issue with collada
    // TODO node name
    ignmsg << "Processing mesh with " << node->mNumMeshes << " meshes" << std::endl;
    for (unsigned mesh_idx = 0; mesh_idx < node->mNumMeshes; ++mesh_idx)
    {
      auto assimp_mesh_idx = node->mMeshes[mesh_idx];
      auto subMesh = this->dataPtr->CreateSubMesh(scene, assimp_mesh_idx, trans);
      subMesh.SetName(std::string(node->mName.C_Str()));
      mesh->AddSubMesh(std::move(subMesh));
    }
  }
  // Process animations
  ignmsg << "Processing " << scene->mNumAnimations << " animations" << std::endl;
  ignmsg << "Scene has " << scene->mNumMeshes << " meshes" << std::endl;
  // Iterate over meshes in scene not contained in root node
  // this is a strict superset of the above that also contains animation meshes
  for (unsigned mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx)
  {
    // Skip if the mesh was found in the previous step
    auto mesh_name = std::string(scene->mMeshes[mesh_idx]->mName.C_Str());
    if (!mesh->SubMeshByName(mesh_name).expired())
      continue;
    auto assimp_mesh = scene->mMeshes[mesh_idx];
    ignmsg << "New mesh found with name " << scene->mMeshes[mesh_idx]->mName.C_Str() << std::endl;
    ignmsg << "Mesh has " << scene->mMeshes[mesh_idx]->mNumAnimMeshes << " anim meshes" << std::endl;
    ignmsg << "Mesh has " << scene->mMeshes[mesh_idx]->mNumBones << " bones" << std::endl;
    // Add the bones
    if (scene->mMeshes[mesh_idx]->HasBones() && scene->HasAnimations())
    {
      // First find the root node, guaranteed to have at least one animation
      // TODO check for multiple root nodes and merge them if needed
      std::unordered_set<std::string> bone_names;
      for (unsigned bone_idx = 0; bone_idx < scene->mMeshes[mesh_idx]->mNumBones; ++bone_idx)
      {
        bone_names.insert(std::string(scene->mMeshes[mesh_idx]->mBones[bone_idx]->mName.C_Str()));
      }
      SkeletonNode* skel_root_node = nullptr;
      for (unsigned chan_idx = 0; chan_idx < scene->mAnimations[0]->mNumChannels; ++chan_idx)
      {
        auto chan_name = std::string(scene->mAnimations[0]->mChannels[chan_idx]->mNodeName.C_Str());
        if (bone_names.find(chan_name) ==
            bone_names.end())
        {
          // This is a joint and root node
          skel_root_node = new SkeletonNode(nullptr, chan_name, chan_name, SkeletonNode::JOINT);
          skel_root_node->SetTransform(ignition::math::Matrix4d::Identity);
          skel_root_node->AddRawTransform(ignition::math::Matrix4d::Identity);
          // TODO root transform for this node
          ignmsg << "Added new root node " << chan_name << std::endl;
        }
      }
      for (unsigned bone_idx = 0; bone_idx < scene->mMeshes[mesh_idx]->mNumBones; ++bone_idx)
      {
        auto bone = scene->mMeshes[mesh_idx]->mBones[bone_idx];
        igndbg << "Found bone with name " << bone->mName.C_Str() << std::endl;
        // Now add to the skeleton
        // TODO name vs id?
        auto skel_node = new SkeletonNode(skel_root_node,
            std::string(bone->mName.C_Str()),
            std::string(bone->mName.C_Str()),
            SkeletonNode::NODE);
        auto trans = this->dataPtr->ConvertTransform(bone->mOffsetMatrix);
        skel_node->SetTransform(trans);
        skel_node->AddRawTransform(trans);
      }
      SkeletonPtr skel(new Skeleton(skel_root_node));
      // Set vertex weights, TODO check if we can put above
      for (unsigned bone_idx = 0; bone_idx < scene->mMeshes[mesh_idx]->mNumBones; ++bone_idx)
      {
        auto bone = scene->mMeshes[mesh_idx]->mBones[bone_idx];
        auto bone_name = std::string(bone->mName.C_Str());
        ignmsg << "Bone " << bone_name << " has " << bone->mNumWeights << " weights" << std::endl;
        for (unsigned weight_idx = 0; weight_idx < bone->mNumWeights; ++weight_idx)
        {
          auto vertex_weight = bone->mWeights[weight_idx];
          // TODO SetNumVertAttached for performance
          skel->AddVertNodeWeight(vertex_weight.mVertexId, bone_name, vertex_weight.mWeight);
        }
      }
      // Now add the skeleton to the mesh
      mesh->SetSkeleton(skel);
    }
    // This mesh doesn't have a node transform
    auto subMesh = this->dataPtr->CreateSubMesh(scene, mesh_idx, root_transformation);
    subMesh.SetName(std::string(assimp_mesh->mName.C_Str()));
    mesh->AddSubMesh(std::move(subMesh));
  }
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
        skel_anim->AddKeyFrame(chan_name, pos_key.mTime, pose);
        //igndbg << "Adding animation at time " << pos_key.mTime << " with position (" << pos.X() << "," << pos.Y() << "," <<
        //  pos.Z() << ")" << std::endl;
      }
    }
    mesh->MeshSkeleton()->AddAnimation(skel_anim);
  }
  // Iterate over nodes and add a submesh for each
  /*
  mesh->SetPath(this->dataPtr->path);

  this->dataPtr->LoadScene(mesh);

  if (mesh->HasSkeleton())
    this->dataPtr->ApplyInvBindTransform(mesh->MeshSkeleton());

  // This will make the model the correct size.
  mesh->Scale(ignition::math::Vector3d(
      this->dataPtr->meter, this->dataPtr->meter, this->dataPtr->meter));
  if (mesh->HasSkeleton())
    mesh->MeshSkeleton()->Scale(this->dataPtr->meter);
  */

  return mesh;
}

}
}
