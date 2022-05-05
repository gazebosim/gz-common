#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <memory>

#include "ignition/math/Helpers.hh"
#include "ignition/common/Filesystem.hh"
#include "ignition/common/Console.hh"
#include "ignition/common/Mesh.hh"
#include "ignition/common/Material.hh"
#include "ignition/common/SubMesh.hh"
#include "ignition/common/AssimpLoader.hh"

using namespace ignition;
using namespace common;

//TODO: Conventions

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
Mesh *AssimpLoader::Load(const std::string &resource_path)
{
  /*
    Check whether a mesh with same path already loaded.
    TO-DO: I believe this is not a good idea.
  */

  // Create an instance of the Assimp Importer class
  Assimp::Importer importer;
  /*
    TO-DO: Do we need to create our own IOLoader?
  */

  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll
  // probably to request more postprocessing than we do in this example.
  // TO-DO: Should we add more post-procesing: https://learnopengl.com/Model-Loading/Model
  const aiScene* scene = importer.ReadFile(resource_path,
    aiProcess_FlipUVs |
    aiProcess_GenUVCoords |
    aiProcess_Triangulate |
    aiProcess_GenNormals |
    aiProcess_FindInvalidData |
    aiProcess_SortByPType);


  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || !scene->HasMeshes())
  {
    ignerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return new Mesh();
  }

  std::cout << "Succesfully loaded the scene." << std::endl ;

  return MeshFromAssimpScene(resource_path, scene);

}

//////////////////////////////////////////////////
Mesh *AssimpLoader::MeshFromAssimpScene(const std::string& name, const aiScene *scene)
{
  
  Mesh *mesh = new Mesh();

  //We are not going to load the materials here.

  float radius = 0.0f; //TO-DO: Why?
  // buildMesh(scene, scene->mRootNode, mesh, aabb, radius, material_table);
  /*
  TO-DO: We dont need aabb, and radius
  */
  buildMesh(scene, scene->mRootNode, mesh);
  return mesh;
}



void buildMesh(const aiScene* scene,
               const aiNode* node,
               Mesh *mesh,
               aiMatrix4x4 transform = aiMatrix4x4())
{
  if (!node)
    return;

  if (node->mParent == nullptr)
  {
    // Use root node's transform
    transform = node->mTransformation;
    // but don't rotate to y-up orientation, which is *sometimes* done in assimp's root node
    aiVector3D scaling, axis, pos;
    float angle;
    transform.Decompose(scaling, axis, angle, pos);
    // drop rotation, but keep scaling and position
    transform = aiMatrix4x4(scaling, aiQuaternion(), pos);
  }
  else
    transform *= node->mTransformation;

  // rotation (+scaling) part of transform applied to normals
  aiMatrix3x3 rotation(transform);

  for (uint32_t i = 0; i < node->mNumMeshes; i++)
  {

    aiMesh* input_mesh = scene->mMeshes[node->mMeshes[i]];
    std::unique_ptr<SubMesh> subMesh(new SubMesh());
    Material *mat = nullptr;

    /*
    TO-DO: Do we need to create a vertex buffer
    as Rviz did? STL and OBJ doesnt use them.
    */

    /*
    OBJLoader creates new submesh for each face. Because
    each face has a different submesh. This requires additional 
    checking.

    However, Assimp, has one material for each mesh, so we
    dont need that, we are just going to add vertices, normals
    texture coordinates.  And that is basically it.
    */
    for (uint32_t j = 0; j < input_mesh->mNumVertices; j++)
    {
      //Process vertex positions
      ignition::math::Vector3d vertex;
      vertex.X(input_mesh->mVertices[j].x);
      vertex.Y(input_mesh->mVertices[j].y);
      vertex.Z(input_mesh->mVertices[j].z);

      //Process vertex normals
      ignition::math::Vector3d normal;
      normal.X(input_mesh->mNormals[j].x);
      normal.Y(input_mesh->mNormals[j].y);
      normal.Z(input_mesh->mNormals[j].z);

      //Process textures
      ignition::math::Vector2d texcoords(0,0);
      if (input_mesh->HasTextureCoords(0)) //TO-DO: Why just first texture?
      {
        texcoords.Set(input_mesh->mTextureCoords[0][i].x, input_mesh->mTextureCoords[0][i].y);
      }
      else
      {
        texcoords.Set(0, 0);
      }

      subMesh->AddTexCoord(texcoords);
      subMesh->AddVertex(vertex);
      subMesh->AddNormal(normal);
      subMesh->AddIndex(subMesh->IndexCount());
    }
    if(input_mesh->mMaterialIndex >= 0)
    {

      math::Color diffuse(0, 0, 0);
      math::Color specular(0, 0, 0);
      math::Color ambient(0, 0, 0);
      math::Color emissive(0, 0, 0);

      mat = new Material();
      aiMaterial *input_material = scene->mMaterials[input_mesh->mMaterialIndex];
      for (uint32_t j = 0; j < input_material->mNumProperties; j++)
      {
        aiMaterialProperty* prop = input_material->mProperties[j];
        std::string propKey = prop->mKey.data;
        if (propKey == "$tex.file")
        {
          //TO-DO: Look from Rviz source code.
          //What does it actually do?
          //What is OGRE::Pass
        }
        else if(propKey == "$clr.diffuse")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_DIFFUSE, clr);
          diffuse.Set(clr.r, clr.g, clr.b);
        }
        else if(propKey == "$clr.ambient")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_AMBIENT, clr);
          ambient.Set(clr.r, clr.g, clr.b);
        }
        else if(propKey == "$clr.specular")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_SPECULAR, clr);
          specular.Set(clr.r, clr.g, clr.b);
        }
        else if (propKey == "$clr.emissive")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_EMISSIVE, clr);
          emissive.Set(clr.r, clr.g, clr.b);
          mat->SetEmissive(emissive);
        }
        else if(propKey == "$clr.opacity")
        {
          float o;
          input_material->Get(AI_MATKEY_OPACITY, o);
          diffuse.A(o);
        }
        else if(propKey == "$mat.shininess")
        {
          float s;
          input_material->Get(AI_MATKEY_SHININESS, s);
          mat->SetShininess(s);
        }
        else if(propKey == "$mat.shadingm")
        {
          int model;
          input_material->Get(AI_MATKEY_SHADING_MODEL, model)
          switch(model)
          {
            case aiShadingMode_Flat:
            mat->setShadingMode(FLAT);
            break;
          case aiShadingMode_Phong:
            mat->setShadingMode(PHONG);
            break;
          case aiShadingMode_Gouraud:
          default:
            mat->setShadingMode(GOURAUD);
            break;
          }
        }
      }
      int mode = aiBlendMode_Default;
      input_material->Get(AI_MATKEY_BLEND_FUNC, mode);
      switch (mode)
      {
      case aiBlendMode_Additive:
        mat->SetBlend(ADD);
        break;
      case aiBlendMode_Default:
      default:
      {
        if (diffuse.A() < 0.99)
        {
          //TO-DO What is this?
          //pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
          //TO-DO: What is this?
        }
        else
        {
          //TO-DO What is this?
          //pass->setSceneBlending(REPLACE);
        }
      }
      break;
      }

      mat->setAmbient(ambient * 0.5);
      mat->setDiffuse(diffuse);
      specular.A(diffuse.A());
      mat->setSpecular(specular);
      //TODO: Add this material to main mesh material index.
      //TODO: Refer the index in the submesh.
    }
    mesh->AddSubMesh(subMesh);
  }
  for (uint32_t i = 0; i < node->mNumChildren; ++i),
  {
    buildMesh(scene, node->mChildren[i], mesh, aabb, radius, material_table, transform);
  }
}


