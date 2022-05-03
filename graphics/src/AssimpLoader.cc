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
Assimp::Mesh *AssimpLoader::Load(const std::string &resource_path)
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


  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || !scene->HasMeshes())
  {
    ignerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return Assimp::MeshPtr();
  }

  std::cout << "Succesfully loaded the scene." << std::endl ;

  return MeshFromAssimpScene(resource_path, scene);

}

//////////////////////////////////////////////////
Assimp::Mesh * AssimpLoader::MeshFromAssimpScene(const std::string& name, const aiScene *scene)
{
  if (!scene->HasMeshes())
  {
    ignerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return Assimp::MeshPtr();
  }

  //We are not going to load the materials here.

  Assimp::Mesh *mesh = new Assimp::Mesh();
  float radius = 0.0f; //TO-DO: Why?
  // buildMesh(scene, scene->mRootNode, mesh, aabb, radius, material_table);
  /*
  TO-DO: We dont need aabb, and radius
  */
  return buildMesh(scene, scene->mRootNode, mesh) 
}


void buildMesh(const aiScene* scene,
               const aiNode* node,
               const Assimp::MeshPtr& mesh,
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
    SubMesh subMesh;

    /*
    TO-DO: Do we need to create a vertex buffer
    as Rviz did? STL and OBJ doesnt use them.
    */

    // Add the vertices
    for (uint32_t j = 0; j < input_mesh->mNumVertices; j++)
    {
      //Process vertex positions
      ignition::math::Vector3d vertex;
      vertex.X(mesh->mVertices[j].x);
      vertex.Y(mesh->mVertices[j].y);
      vertex.Z(mesh->mVertices[j].z);

      //Process vertex normals
      ignition::math::Vector3d normal;
      normal.X(mesh->mNormals[j].x);
      normal.Y(mesh->mNormals[j].y);
      normal.Z(mesh->mNormals[j].z);

      //Process textures
      ignition::math::Vector2d texcoords;
      if (input_mesh->HasTextureCoords(0)) //TO-DO: Why just first texture?
      {
        texcoords.x = mesh->mTextureCoords[0][i].x; 
        texcoords.y = mesh->mTextureCoords[0][i].y;
      }
      else
      {
        texcoords.x = 0.0f;
        texcoords.y = 0.0f;
      }

      subMesh.AddTexCoord(texcoords);
      subMesh.AddVertex(vertex);
      subMesh.AddNormal(normal);
    }
    if(input_mesh->mMaterialIndex >= 0)
    {
      mat = new Material();
      aiMaterial *input_material = scene->mMaterials[input_mesh->mMaterialIndex];
      for (uint32_t j = 0; j < input_material->mNumProperties; j++)
      {
        aiMaterialProperty* prob = input_material->mProperties[j];
        std::string propKey = prop->mKey.data;
        if (propKey == "$tex.file")
        {
          aiString texName;
          aiTextureMapping mapping;
          uint32_t uvIndex;
          input_material->GetTexture(aiTextureType_DIFFUSE, 0, &texName, &mapping, &uvIndex);

          // Assume textures are in paths relative to the mesh
          std::string texture_path = fs::path(resource_path).parent_path().string() + "/" + texName.data;
          loadTexture(texture_path); //TODO: Image loader
          Ogre::TextureUnitState* tu = pass->createTextureUnitState();
          tu->setTextureName(texture_path);
        }
        else if(propKey == "$clr.diffuse")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_DIFFUSE, clr);
          diffuse = Ogre::ColourValue(clr.r, clr.g, clr.b)
        }
        else if(propKey == "$clr.ambient")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_AMBIENT, clr);
          ambient = Ogre::ColourValue(clr.r, clr.g, clr.b)
        }
        else if(propKey == "$clr.specular")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_SPECULAR, clr);
          specular = Ogre::ColourValue(clr.r, clr.g, clr.b)
        }
        else if (propKey == "$clr.emissive")
        {
          aiColor3D clr;
          input_material->Get(AI_MATKEY_COLOR_EMISSIVE, clr);
          mat->setSelfIllumination(clr.r, clr.g, clr.b);
          //TO-DO: This might be set emissive
        }
        else if(propKey == "$clr.opacity")
        {
          float o;
          input_material->Get(AI_MATKEY_OPACITY, o);
          difuse.a = o;
        }
        else if(propKey == "$mat.shininess")
        {
          float s;
          amat->Get(AI_MATKEY_SHININESS, s);
          mat->setShininess(s);
        }
        else if(propKey == "$mat.shadingm")
        {
          int model;
          amat->Get(AI_MATKEY_SHADING_MODEL, model)
          switch(model)
          {
            case aiShadingMode_Flat:
            mat->setShadingMode(Ogre::SO_FLAT);
            break;
          case aiShadingMode_Phong:
            mat->setShadingMode(Ogre::SO_PHONG);
            break;
          case aiShadingMode_Gouraud:
          default:
            mat->setShadingMode(Ogre::SO_GOURAUD);
            break;
          }
        }
      }
      int mode = aiBlendMode_Default;
      amat->Get(AI_MATKEY_BLEND_FUNC, mode);
      switch (mode)
      {
      case aiBlendMode_Additive:
        mat->setSceneBlending(Ogre::SBT_ADD);
        break;
      case aiBlendMode_Default:
      default:
      {
        if (diffuse.a < 0.99)
        {
          pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        }
        else
        {
          pass->setSceneBlending(Ogre::SBT_REPLACE);
        }
      }
      break;
      }

      mat->setAmbient(ambient * 0.5);
      mat->setDiffuse(diffuse);
      specular.a = diffuse.a;
      mat->setSpecular(specular);

    }
  mesh->AddSubMesh(subMesh);
}






















      Ogre::Vector3 v(p.x, p.y, p.z);
      aabb.merge(v);
      float dist = v.length();
      if (dist > radius)
      {
        radius = dist;
      }

      if (input_mesh->HasNormals())
      {
        aiVector3D n = input_mesh->mNormals[j];
        n *= rotation;
        n.Normalize();
        *vertices++ = n.x;
        *vertices++ = n.y;
        *vertices++ = n.z;
      }

      if (input_mesh->HasTextureCoords(0))
      {
        *vertices++ = input_mesh->mTextureCoords[0][j].x;
        *vertices++ = input_mesh->mTextureCoords[0][j].y;
      }
    }

    // calculate index count
    submesh->indexData->indexCount = 0;
    for (uint32_t j = 0; j < input_mesh->mNumFaces; j++)
    {
      aiFace& face = input_mesh->mFaces[j];
      submesh->indexData->indexCount += face.mNumIndices;
    }

    // If we have less than 65536 (2^16) vertices, we can use a 16-bit index buffer.
    if (vertex_data->vertexCount < (1 << 16))
    {
      // allocate index buffer
      submesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
          Ogre::HardwareIndexBuffer::IT_16BIT, submesh->indexData->indexCount,
          Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

      Ogre::HardwareIndexBufferSharedPtr ibuf = submesh->indexData->indexBuffer;
      uint16_t* indices = static_cast<uint16_t*>(ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

      // add the indices
      for (uint32_t j = 0; j < input_mesh->mNumFaces; j++)
      {
        aiFace& face = input_mesh->mFaces[j];
        for (uint32_t k = 0; k < face.mNumIndices; ++k)
        {
          *indices++ = face.mIndices[k];
        }
      }

      ibuf->unlock();
    }
    else
    {
      // Else we have more than 65536 (2^16) vertices, so we must
      // use a 32-bit index buffer (or subdivide the mesh, which
      // I'm too impatient to do right now)

      // allocate index buffer
      submesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
          Ogre::HardwareIndexBuffer::IT_32BIT, submesh->indexData->indexCount,
          Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

      Ogre::HardwareIndexBufferSharedPtr ibuf = submesh->indexData->indexBuffer;
      uint32_t* indices = static_cast<uint32_t*>(ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

      // add the indices
      for (uint32_t j = 0; j < input_mesh->mNumFaces; j++)
      {
        aiFace& face = input_mesh->mFaces[j];
        for (uint32_t k = 0; k < face.mNumIndices; ++k)
        {
          *indices++ = face.mIndices[k];
        }
      }

      ibuf->unlock();
    }
    vbuf->unlock();

    Ogre::MaterialPtr const& material = material_table[input_mesh->mMaterialIndex];
    submesh->setMaterialName(material->getName(), material->getGroup());
  }

  for (uint32_t i = 0; i < node->mNumChildren; ++i)
  {
    buildMesh(scene, node->mChildren[i], mesh, aabb, radius, material_table, transform);
  }
}

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
