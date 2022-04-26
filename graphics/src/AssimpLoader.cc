#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <memory>

#include "ignition/math/Helpers.hh"
#include "ignition/common/Console.hh"
#include "ignition/common/Mesh.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/AssimpLoader.hh"

using namespace ignition;
using namespace common;


//////////////////////////////////////////////////
class ignition::common::AssimpLoader::Implementation
{
};

//////////////////////////////////////////////////
AssimpLoader::AssimpLoader()
: MeshLoader(),
  dataPtr(ignition::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
AssimpLoader::~AssimpLoader()
{
}

//////////////////////////////////////////////////
Mesh *AssimpLoader::Load(const std::string &pFile)
{

  Mesh *mesh = new Mesh();

  // Create an instance of the Assimp Importer class
  Assimp::Importer importer;

  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll
  // probably to request more postprocessing than we do in this example.
  const aiScene* scene = importer.ReadFile( pFile,
    aiProcess_CalcTangentSpace       |
    aiProcess_Triangulate            |
    aiProcess_JoinIdenticalVertices  |
    aiProcess_SortByPType);


  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
  {
    ignerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return nullptr;
  }

  std::cout << "Succesfully loaded the scene." << std::endl ;

  ProcessNode(scene->mRootNode, scene, mesh);
  // We're done. Everything will be cleaned up by the importer destructor
  
  return mesh;
}

//////////////////////////////////////////////////
bool AssimpLoader::ProcessNode(aiNode *node, const aiScene *scene, Mesh *gazeboMesh)
{
  // process all the node's meshes (if any)
  for(unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    SubMesh subMesh = Assimp2GazeboMesh(mesh);
    gazeboMesh->AddSubMesh(subMesh);
  }
  // then do the same for each of its children
  /*
  for(unsigned int i = 0; i < node->mNumChildren; i++)
  {
    processNode(node->mChildren[i], scene, mesh);
  }
  */
  return true;
}

SubMesh AssimpLoader::Assimp2GazeboMesh(aiMesh *mesh)
{
  SubMesh subMesh;

  // Retrieve all the vertex data;
  for(unsigned int i = 0; i < mesh->mNumVertices; i++)
  {

    //Process vertex positions
    ignition::math::Vector3d vertex;
    vertex.X(mesh->mVertices[i].x);
    vertex.Y(mesh->mVertices[i].y);
    vertex.Z(mesh->mVertices[i].z);

    //Process vertex normals
    ignition::math::Vector3d normal;
    normal.X(mesh->mNormals[i].x);
    normal.Y(mesh->mNormals[i].y);
    normal.Z(mesh->mNormals[i].z);

    //Process texture coordinates
      //TO-DO: Implement this later...

    subMesh.AddVertex(vertex);
    subMesh.AddNormal(normal);
  }

  // Retrieve all the mesh's indices
  for(unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    for(unsigned int j = 0; j < face.mNumIndices; j++)
      subMesh.AddIndex(face.mIndices[j]);
  }
  // Retrieve material data
    //TO-DO: Implement this later...
  return subMesh;
}

int main(void)
{
  printf("Hello world!\n");
  return 0;
}
