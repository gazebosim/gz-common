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

#include <queue>
#include <unordered_set>

#include "gz/common/graphics/Types.hh"
#include "gz/common/AssimpLoader.hh"
#include "gz/common/Console.hh"
#include "gz/common/Image.hh"
#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/common/Util.hh"

#ifndef GZ_ASSIMP_PRE_5_2_0
  #include <assimp/GltfMaterial.h>    // GLTF specific material properties
#endif
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>           // Output data structure

// Disable warning for converting double to unsigned char
#ifdef _WIN32
  #pragma warning( disable : 4244 )
#endif

namespace gz
{
namespace common
{

using ImagePtr = std::shared_ptr<Image>;

/// \brief Private data for the AssimpLoader class
class AssimpLoader::Implementation
{
  /// \brief the Assimp importer used to parse meshes
  public: Assimp::Importer importer;

  /// \brief Convert a color from assimp implementation to Ignition common
  /// \param[in] _color the assimp color to convert
  /// \return the matching math::Color
  public: math::Color ConvertColor(aiColor4D& _color) const;

  /// \brief Convert a matrix from assimp implementation to gz::Math
  /// \param[in] _matrix the assimp matrix to convert
  /// \return the converted math::Matrix4d
  public: math::Matrix4d ConvertTransform(const aiMatrix4x4& _matrix) const;

  /// \brief Convert from assimp to gz::common::Material
  /// \param[in] _scene the assimp scene
  /// \param[in] _matIdx index of the material in the scene
  /// \param[in] _path path where the mesh is located
  /// \return pointer to the converted common::Material
  public: MaterialPtr CreateMaterial(const aiScene* _scene,
                                     unsigned _matIdx,
                                     const std::string& _path) const;

  /// \brief Load a texture embedded in a mesh (i.e. for GLB format)
  /// into a gz::common::Image
  /// \param[in] _texture the assimp texture object
  /// \return Pointer to a common::Image containing the texture
  public: ImagePtr LoadEmbeddedTexture(const aiTexture* _texture) const;

  /// \brief Utility function to generate a texture name for both embedded
  /// and external textures
  /// \param[in] _scene the assimp scene
  /// \param[in] _mat the assimp material
  /// \param[in] _type the type of texture (i.e. Diffuse, Metal)
  /// \return the generated texture name
  public: std::string GenerateTextureName(const aiScene* _scene,
                                          aiMaterial*  _mat,
                                          const std::string& _type) const;

  /// \brief Function to parse texture information and load it if embedded
  /// \param[in] _scene the assimp scene
  /// \param[in] _texturePath the path where the texture is located
  /// \param[in] _textureName the name of the texture
  /// \return a pair containing the name of the texture and a pointer to the
  /// image data, if the texture was loaded in memory
  public: std::pair<std::string, ImagePtr>
          LoadTexture(const aiScene* _scene,
                      const aiString& _texturePath,
                      const std::string& _textureName) const;

  /// \brief Function to split a gltf metallicroughness map into
  /// a metalness and roughness map
  /// \param[in] _img the image to split
  /// \return a pair of image pointers with the first being the metalness
  /// map and the second being the roughness map
  public: std::pair<ImagePtr, ImagePtr>
          SplitMetallicRoughnessMap(const Image& _img) const;

  /// \brief Convert an assimp mesh into a gz::common::SubMesh
  /// \param[in] _assimpMesh the assimp mesh to load
  /// \param[in] _transform the node transform for the mesh
  /// \return the converted common::Submesh
  public: SubMesh CreateSubMesh(const aiMesh* _assimpMesh,
                                const math::Matrix4d& _transform) const;

  /// \brief Recursively create submeshes scene starting from the root node
  /// \param[in] _scene the assimp scene
  /// \param[in] _node the node being processed
  /// \param[in] _transform the transform of the node being processed
  /// \param[out] _mesh the common::Mesh to edit
  public: void RecursiveCreate(const aiScene* _scene,
                               const aiNode* _node,
                               const math::Matrix4d& _transform,
                               Mesh* _mesh) const;

  /// \brief Recursively create the skeleton starting from the root node
  /// \param[in] _node the node being processed
  /// \param[in] _parent the parent skeleton node
  /// \param[in] _transform the transform of the current node
  /// \param[in] _boneNames set of bone names, used to skip nodes without a bone
  public: void RecursiveSkeletonCreate(
          const aiNode* _node,
          SkeletonNode* _parent,
          const math::Matrix4d& _transform,
          const std::unordered_set<std::string> &_boneNames) const;

  /// \brief Recursively store the bone names starting from the root node
  /// to make sure that only nodes that map to a bone are added to the skeleton
  /// \param[in] _scene the assimp scene
  /// \param[in] _node the node being processed
  /// \param[out] _boneNames set of bone names populated while recursing
  public: void RecursiveStoreBoneNames(
          const aiScene *_scene,
          const aiNode* _node,
          std::unordered_set<std::string>& _boneNames) const;

  /// \brief Apply the the inv bind transform to the skeleton pose.
  /// \remarks have to set the model transforms starting from the root in
  /// breadth first order. Because setting the model transform also updates
  /// the transform based on the parent's inv model transform. Setting the
  /// child before the parent results in the child's transform being
  /// calculated from the "old" parent model transform.
  /// \param[in] _skeleton the skeleton to work on
  public: void ApplyInvBindTransform(SkeletonPtr _skeleton) const;
};

//////////////////////////////////////////////////
// Utility function to convert to std::string from aiString
static std::string ToString(const aiString& str)
{
  return std::string(str.C_Str());
}

//////////////////////////////////////////////////
math::Color AssimpLoader::Implementation::ConvertColor(aiColor4D& _color) const
{
  math::Color col(_color.r, _color.g, _color.b, _color.a);
  return col;
}

//////////////////////////////////////////////////
math::Matrix4d AssimpLoader::Implementation::ConvertTransform(
    const aiMatrix4x4& _sm) const
{
  return math::Matrix4d(
      _sm.a1, _sm.a2, _sm.a3, _sm.a4,
      _sm.b1, _sm.b2, _sm.b3, _sm.b4,
      _sm.c1, _sm.c2, _sm.c3, _sm.c4,
      _sm.d1, _sm.d2, _sm.d3, _sm.d4);
}

//////////////////////////////////////////////////
void AssimpLoader::Implementation::RecursiveCreate(const aiScene* _scene,
    const aiNode* _node, const math::Matrix4d& _transform, Mesh* _mesh) const
{
  if (!_node)
    return;
  // Visit this node, add the submesh
  for (unsigned meshIdx = 0; meshIdx < _node->mNumMeshes; ++meshIdx)
  {
    auto assimpMeshIdx = _node->mMeshes[meshIdx];
    auto& assimpMesh = _scene->mMeshes[assimpMeshIdx];
    auto nodeName = ToString(_node->mName);
    auto subMesh = this->CreateSubMesh(assimpMesh, _transform);
    subMesh.SetName(nodeName);
    // Now add the bones to the skeleton
    if (assimpMesh->HasBones() && _scene->HasAnimations())
    {
      // TODO(luca) merging skeletons here
      auto skeleton = _mesh->MeshSkeleton();
      // TODO(luca) Append to existing skeleton if multiple submeshes?
      skeleton->SetNumVertAttached(subMesh.VertexCount());
      // Now add the bone weights
      for (unsigned boneIdx = 0; boneIdx < assimpMesh->mNumBones; ++boneIdx)
      {
        auto& bone = assimpMesh->mBones[boneIdx];
        auto boneNodeName = ToString(bone->mName);
        // Apply inverse bind transform to the matching node
        SkeletonNode *skelNode =
            skeleton->NodeByName(boneNodeName);
        if (skelNode == nullptr)
          continue;
        skelNode->SetInverseBindTransform(
            this->ConvertTransform(bone->mOffsetMatrix));
        for (unsigned weightIdx = 0; weightIdx < bone->mNumWeights; ++weightIdx)
        {
          auto vertexWeight = bone->mWeights[weightIdx];
          skeleton->AddVertNodeWeight(
              vertexWeight.mVertexId, boneNodeName, vertexWeight.mWeight);
        }
      }
      // Add node assignment to mesh
      for (unsigned vertexIdx = 0; vertexIdx < subMesh.VertexCount();
          ++vertexIdx)
      {
        for (unsigned i = 0; i < skeleton->VertNodeWeightCount(vertexIdx); ++i)
        {
          std::pair<std::string, double> nodeWeight =
            skeleton->VertNodeWeight(vertexIdx, i);
          SkeletonNode *node =
              skeleton->NodeByName(nodeWeight.first);
          subMesh.AddNodeAssignment(vertexIdx,
                          node->Handle(), nodeWeight.second);
        }
      }
    }
    _mesh->AddSubMesh(std::move(subMesh));
  }

  // Iterate over children
  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    // Calculate the transform
    auto& child_node = _node->mChildren[childIdx];
    auto nodeTrans = this->ConvertTransform(child_node->mTransformation);
    nodeTrans = _transform * nodeTrans;

    // Finally recursive call to explore subnode
    this->RecursiveCreate(_scene, child_node, nodeTrans, _mesh);
  }
}

void AssimpLoader::Implementation::RecursiveStoreBoneNames(
    const aiScene *_scene, const aiNode *_node,
    std::unordered_set<std::string>& _boneNames) const
{
  if (!_node)
    return;

  for (unsigned meshIdx = 0; meshIdx < _node->mNumMeshes; ++meshIdx)
  {
    auto assimpMeshIdx = _node->mMeshes[meshIdx];
    auto assimpMesh = _scene->mMeshes[assimpMeshIdx];
    for (unsigned boneIdx = 0; boneIdx < assimpMesh->mNumBones; ++boneIdx)
    {
      auto bone = assimpMesh->mBones[boneIdx];
      _boneNames.insert(ToString(bone->mName));
    }
  }

  // Iterate over children
  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    auto child_node = _node->mChildren[childIdx];
    // Finally recursive call to explore subnode
    this->RecursiveStoreBoneNames(_scene, child_node, _boneNames);
  }
}

//////////////////////////////////////////////////
void AssimpLoader::Implementation::RecursiveSkeletonCreate(const aiNode* _node,
    SkeletonNode* _parent, const math::Matrix4d& _transform,
    const std::unordered_set<std::string> &_boneNames) const
{
  if (_node == nullptr || _parent == nullptr)
    return;
  // First explore this node
  auto nodeName = ToString(_node->mName);
  auto boneExist = _boneNames.find(nodeName) != _boneNames.end();
  auto nodeTrans = this->ConvertTransform(_node->mTransformation);
  auto skelNode = _parent;

  if (boneExist)
  {
    skelNode = new SkeletonNode(
        _parent, nodeName, nodeName, SkeletonNode::JOINT);
    skelNode->SetTransform(nodeTrans);
  }

  nodeTrans = _transform * nodeTrans;

  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    this->RecursiveSkeletonCreate(
        _node->mChildren[childIdx], skelNode, nodeTrans, _boneNames);
  }
}

//////////////////////////////////////////////////
MaterialPtr AssimpLoader::Implementation::CreateMaterial(
    const aiScene* _scene, unsigned _matIdx, const std::string& _path) const
{
  MaterialPtr mat = std::make_shared<Material>();
  aiColor4D color;
  auto& assimpMat = _scene->mMaterials[_matIdx];
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
  float shininess;
  ret = assimpMat->Get(AI_MATKEY_SHININESS, shininess);
  if (ret == AI_SUCCESS)
  {
    mat->SetShininess(shininess);
  }
  float opacity = 1.0;
  ret = assimpMat->Get(AI_MATKEY_OPACITY, opacity);
  mat->SetTransparency(1.0 - opacity);
  mat->SetBlendFactors(opacity, 1.0 - opacity);
  // TODO(luca) more than one texture, Gazebo assumes UV index 0
  Pbr pbr;
  aiString texturePath(_path.c_str());
  ret = assimpMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
  // TODO(luca) check other arguments,
  // type of mappings to be UV, uv index, blend mode
  if (ret == AI_SUCCESS)
  {
    // Check if the texture is embedded or not
    auto [texName, texData] = this->LoadTexture(_scene,
        texturePath, this->GenerateTextureName(_scene, assimpMat, "Diffuse"));
    if (texData != nullptr)
      mat->SetTextureImage(texName, texData);
    else
      mat->SetTextureImage(texName, _path);
#ifndef GZ_ASSIMP_PRE_5_2_0
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
#endif
  }
#ifndef GZ_ASSIMP_PRE_5_2_0
  // Edge case for GLTF, Metal and Rough texture are embedded in a
  // MetallicRoughness texture with metalness in B and roughness in G
  // Open, preprocess and split into metal and roughness map
  ret = assimpMat->GetTexture(
      AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
      &texturePath);
  if (ret == AI_SUCCESS)
  {
    auto [texName, texData] = this->LoadTexture(_scene, texturePath,
        this->GenerateTextureName(_scene, assimpMat, "MetallicRoughness"));
    // Load it into a common::Image then split it
    auto texImg = texData != nullptr ? texData :
      std::make_shared<Image>(joinPaths(_path, texName));
    auto [metalTexture, roughTexture] =
      this->SplitMetallicRoughnessMap(*texImg);
    pbr.SetMetalnessMap(
        this->GenerateTextureName(_scene, assimpMat, "Metalness"),
        metalTexture);
    pbr.SetRoughnessMap(
        this->GenerateTextureName(_scene, assimpMat, "Roughness"),
        roughTexture);
  }
  else
  {
    // Load the textures separately
    ret = assimpMat->GetTexture(aiTextureType_METALNESS, 0, &texturePath);
    if (ret == AI_SUCCESS)
    {
      auto [texName, texData] = this->LoadTexture(_scene, texturePath,
          this->GenerateTextureName(_scene, assimpMat, "Metalness"));
      pbr.SetMetalnessMap(texName, texData);
    }
    ret = assimpMat->GetTexture(
        aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath);
    if (ret == AI_SUCCESS)
    {
      auto [texName, texData] = this->LoadTexture(_scene, texturePath,
          this->GenerateTextureName(_scene, assimpMat, "Roughness"));
      pbr.SetRoughnessMap(texName, texData);
    }
  }
#endif
  ret = assimpMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath);
  if (ret == AI_SUCCESS)
  {
    auto [texName, texData] = this->LoadTexture(_scene, texturePath,
        this->GenerateTextureName(_scene, assimpMat, "Normal"));
    // TODO(luca) different normal map spaces
    pbr.SetNormalMap(texName, NormalMapSpace::TANGENT, texData);
  }
  ret = assimpMat->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath);
  if (ret == AI_SUCCESS)
  {
    auto [texName, texData] = this->LoadTexture(_scene, texturePath,
        this->GenerateTextureName(_scene, assimpMat, "Emissive"));
    pbr.SetEmissiveMap(texName, texData);
  }
  unsigned int uvIdx = 0;
  ret = assimpMat->GetTexture(
      aiTextureType_LIGHTMAP, 0, &texturePath, NULL, &uvIdx);
  if (ret == AI_SUCCESS)
  {
    auto [texName, texData] = this->LoadTexture(_scene, texturePath,
        this->GenerateTextureName(_scene, assimpMat, "Lightmap"));
    pbr.SetLightMap(texName, uvIdx, texData);
  }
#ifndef GZ_ASSIMP_PRE_5_2_0
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
#endif
  mat->SetPbrMaterial(pbr);
  return mat;
}

//////////////////////////////////////////////////
std::pair<std::string, ImagePtr> AssimpLoader::Implementation::LoadTexture(
    const aiScene* _scene,
    const aiString& _texturePath,
    const std::string& _textureName) const
{
  std::pair<std::string, ImagePtr> ret;
  // Check if the texture is embedded or not
  auto embeddedTexture = _scene->GetEmbeddedTexture(_texturePath.C_Str());
  if (embeddedTexture)
  {
    // Load embedded texture
    ret.first = _textureName;
    ret.second = this->LoadEmbeddedTexture(embeddedTexture);
  }
  else
  {
    ret.first = ToString(_texturePath);
  }
  return ret;
}

std::pair<ImagePtr, ImagePtr>
    AssimpLoader::Implementation::SplitMetallicRoughnessMap(
    const Image& _img) const
{
  std::pair<ImagePtr, ImagePtr> ret;
  // Metalness in B roughness in G
  const auto width = _img.Width();
  const auto height = _img.Height();
  const auto bytesPerPixel = 4;

  std::vector<unsigned char> metalnessData(width * height * bytesPerPixel);
  std::vector<unsigned char> roughnessData(width * height * bytesPerPixel);

  for (unsigned int x = 0; x < width; ++x)
  {
    for (unsigned int y = 0; y < height; ++y)
    {
      // RGBA so 4 bytes per pixel, alpha fully opaque
      auto baseIndex = bytesPerPixel * (x * height + y);
      auto color = _img.Pixel(x, y);
      metalnessData[baseIndex] = color.B() * 255.0;
      metalnessData[baseIndex + 1] = color.B() * 255.0;
      metalnessData[baseIndex + 2] = color.B() * 255.0;
      metalnessData[baseIndex + 3] = 255;
      roughnessData[baseIndex] = color.G() * 255.0;
      roughnessData[baseIndex + 1] = color.G() * 255.0;
      roughnessData[baseIndex + 2] = color.G() * 255.0;
      roughnessData[baseIndex + 3] = 255;
    }
  }
  // First is metal, second is rough
  ret.first = std::make_shared<Image>();
  ret.first->SetFromData(&metalnessData[0], width, height, Image::RGBA_INT8);
  ret.second = std::make_shared<Image>();
  ret.second->SetFromData(&roughnessData[0], width, height, Image::RGBA_INT8);
  return ret;
}

//////////////////////////////////////////////////
ImagePtr AssimpLoader::Implementation::LoadEmbeddedTexture(
    const aiTexture* _texture) const
{
  auto img = std::make_shared<Image>();
  if (_texture->mHeight == 0)
  {
    if (_texture->CheckFormat("png"))
    {
      img->SetFromCompressedData((unsigned char*)_texture->pcData,
          _texture->mWidth, Image::PixelFormatType::COMPRESSED_PNG);
    }
  }
  return img;
}

//////////////////////////////////////////////////
std::string AssimpLoader::Implementation::GenerateTextureName(
    const aiScene* _scene, aiMaterial* _mat, const std::string& _type) const
{
  return ToString(_scene->mRootNode->mName) + "_" + ToString(_mat->GetName()) +
    "_" + _type;
}

SubMesh AssimpLoader::Implementation::CreateSubMesh(
    const aiMesh* _assimpMesh, const math::Matrix4d& _transform) const
{
  SubMesh subMesh;
  math::Matrix4d rot = _transform;
  rot.SetTranslation(math::Vector3d::Zero);
  // Now create the submesh
  for (unsigned vertexIdx = 0; vertexIdx < _assimpMesh->mNumVertices;
      ++vertexIdx)
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
      subMesh.AddTexCoordBySet(texcoords.X(), texcoords.Y(), uvIdx);
      ++uvIdx;
    }
  }
  for (unsigned faceIdx = 0; faceIdx < _assimpMesh->mNumFaces; ++faceIdx)
  {
    auto& face = _assimpMesh->mFaces[faceIdx];
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
  this->dataPtr->importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
  this->dataPtr->importer.SetPropertyBool(
      AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);
}

//////////////////////////////////////////////////
AssimpLoader::~AssimpLoader()
{
}

//////////////////////////////////////////////////
Mesh *AssimpLoader::Load(const std::string &_filename)
{
  Mesh *mesh = new Mesh();
  std::string path = common::parentPath(_filename);
  const aiScene* scene = this->dataPtr->importer.ReadFile(_filename,
      aiProcess_JoinIdenticalVertices |
      aiProcess_RemoveRedundantMaterials |
      aiProcess_SortByPType |
      aiProcess_FlipUVs |
#ifndef GZ_ASSIMP_PRE_5_2_0
      aiProcess_PopulateArmatureData |
#endif
      aiProcess_Triangulate |
      aiProcess_GenNormals |
      0);
  if (scene == nullptr)
  {
    gzerr << "Unable to import mesh [" << _filename << "]" << std::endl;
    return mesh;
  }
  auto& rootNode = scene->mRootNode;
  auto rootName = ToString(rootNode->mName);
  auto transform = scene->mRootNode->mTransformation;
  aiVector3D rootScaling, rootAxis, rootPos;
  float angle;
  transform.Decompose(rootScaling, rootAxis, angle, rootPos);
  // drop rotation, but keep scaling and position
  // TODO(luca) it seems imported assets are rotated by 90 degrees
  // as documented here https://github.com/assimp/assimp/issues/849
  // remove workaround when fixed
  transform = aiMatrix4x4(rootScaling, aiQuaternion(), rootPos);

  auto rootTransform = this->dataPtr->ConvertTransform(transform);

  // Add the materials first
  for (unsigned _matIdx = 0; _matIdx < scene->mNumMaterials; ++_matIdx)
  {
    auto mat = this->dataPtr->CreateMaterial(scene, _matIdx, path);
    mesh->AddMaterial(mat);
  }
  // Create the skeleton
  {
    std::unordered_set<std::string> boneNames;
    this->dataPtr->RecursiveStoreBoneNames(scene, rootNode, boneNames);
    auto rootSkelNode = new SkeletonNode(
        nullptr, rootName, rootName, SkeletonNode::NODE);
    rootSkelNode->SetTransform(rootTransform);
    rootSkelNode->SetModelTransform(rootTransform);
    for (unsigned childIdx = 0; childIdx < rootNode->mNumChildren; ++childIdx)
    {
      // First populate the skeleton with the node transforms
      this->dataPtr->RecursiveSkeletonCreate(
          rootNode->mChildren[childIdx], rootSkelNode,
          rootTransform, boneNames);
    }
    rootSkelNode->SetParent(nullptr);

    SkeletonPtr rootSkeleton = std::make_shared<Skeleton>(rootSkelNode);
    mesh->SetSkeleton(rootSkeleton);
  }
  // Now create the meshes
  // Recursive call to keep track of transforms,
  // mesh is passed by reference and edited throughout
  this->dataPtr->RecursiveCreate(scene, rootNode, rootTransform, mesh);
  auto rootSkeleton = mesh->MeshSkeleton();
  // Add the animations
  for (unsigned animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
  {
    auto& anim = scene->mAnimations[animIdx];
    auto animName = ToString(anim->mName);
    if (animName.empty())
    {
      animName = "animation" +
                 std::to_string(rootSkeleton->AnimationCount() + 1);
    }
    SkeletonAnimation* skelAnim = new SkeletonAnimation(animName);
    for (unsigned chanIdx = 0; chanIdx < anim->mNumChannels; ++chanIdx)
    {
      auto& animChan = anim->mChannels[chanIdx];
      auto chanName = ToString(animChan->mNodeName);
      for (unsigned keyIdx = 0; keyIdx < animChan->mNumPositionKeys; ++keyIdx)
      {
        // Note, Scaling keys are not supported right now
        // Compute the position into a math pose
        auto& posKey = animChan->mPositionKeys[keyIdx];
        auto& quatKey = animChan->mRotationKeys[keyIdx];
        math::Vector3d pos(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
        math::Quaterniond quat(quatKey.mValue.w, quatKey.mValue.x,
            quatKey.mValue.y, quatKey.mValue.z);
        math::Pose3d pose(pos, quat);
        // Time is in ms
        skelAnim->AddKeyFrame(chanName, posKey.mTime / 1000.0, pose);
      }
    }
    mesh->MeshSkeleton()->AddAnimation(skelAnim);
  }

  this->dataPtr->ApplyInvBindTransform(mesh->MeshSkeleton());

  return mesh;
}

/////////////////////////////////////////////////
void AssimpLoader::Implementation::ApplyInvBindTransform(
    SkeletonPtr _skeleton) const

{
  std::queue<SkeletonNode *> queue;
  queue.push(_skeleton->RootNode());

  while (!queue.empty())
  {
    SkeletonNode *node = queue.front();
    queue.pop();
    if (nullptr == node)
      continue;

    if (node->HasInvBindTransform())
    {
      node->SetModelTransform(node->InverseBindTransform().Inverse(), false);
    }
    for (unsigned int i = 0; i < node->ChildCount(); i++)
      queue.push(node->Child(i));
  }
}

}
}
