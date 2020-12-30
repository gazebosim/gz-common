/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <gtest/gtest.h>

#include <ignition/math/Color.hh>
#include "ignition/common/Pbr.hh"

/////////////////////////////////////////////////
TEST(Pbr, BasicAPI)
{
  // initial state
  ignition::common::Pbr pbr;
  EXPECT_EQ(ignition::common::PbrType::NONE, pbr.Type());
  EXPECT_EQ(std::string(), pbr.AlbedoMap());
  EXPECT_EQ(std::string(), pbr.NormalMap());
  EXPECT_EQ(ignition::common::NormalMapSpace::TANGENT, pbr.NormalMapType());
  EXPECT_EQ(std::string(), pbr.RoughnessMap());
  EXPECT_EQ(std::string(), pbr.MetalnessMap());
  EXPECT_EQ(std::string(), pbr.EmissiveMap());
  EXPECT_EQ(std::string(), pbr.LightMap());
  EXPECT_EQ(0u, pbr.LightMapTexCoordSet());
  EXPECT_DOUBLE_EQ(0.5, pbr.Roughness());
  EXPECT_DOUBLE_EQ(0.0, pbr.Metalness());
  EXPECT_EQ(std::string(), pbr.SpecularMap());
  EXPECT_EQ(std::string(), pbr.GlossinessMap());
  EXPECT_DOUBLE_EQ(0.0, pbr.Glossiness());
  EXPECT_EQ(std::string(), pbr.EnvironmentMap());
  EXPECT_EQ(std::string(), pbr.AmbientOcclusionMap());

  // set / get
  pbr.SetType(ignition::common::PbrType::METAL);
  EXPECT_EQ(ignition::common::PbrType::METAL, pbr.Type());

  pbr.SetAlbedoMap("metal_albedo_map.png");
  EXPECT_EQ("metal_albedo_map.png", pbr.AlbedoMap());

  pbr.SetNormalMap("metal_normal_map.png");
  EXPECT_EQ("metal_normal_map.png", pbr.NormalMap());

  pbr.SetEnvironmentMap("metal_env_map.png");
  EXPECT_EQ("metal_env_map.png", pbr.EnvironmentMap());

  pbr.SetAmbientOcclusionMap("metal_ambient_occlusion_map.png");
  EXPECT_EQ("metal_ambient_occlusion_map.png",
      pbr.AmbientOcclusionMap());

  pbr.SetEmissiveMap("metal_emissive_map.png");
  EXPECT_EQ("metal_emissive_map.png", pbr.EmissiveMap());

  pbr.SetLightMap("metal_light_map.png", 1u);
  EXPECT_EQ("metal_light_map.png", pbr.LightMap());
  EXPECT_EQ(1u, pbr.LightMapTexCoordSet());

  pbr.SetRoughnessMap("roughness_map.png");
  EXPECT_EQ("roughness_map.png", pbr.RoughnessMap());

  pbr.SetMetalnessMap("metalness_map.png");
  EXPECT_EQ("metalness_map.png", pbr.MetalnessMap());

  pbr.SetRoughness(0.8);
  EXPECT_DOUBLE_EQ(0.8, pbr.Roughness());

  pbr.SetMetalness(0.3);
  EXPECT_DOUBLE_EQ(0.3, pbr.Metalness());

  EXPECT_EQ(std::string(), pbr.GlossinessMap());
  EXPECT_EQ(std::string(), pbr.SpecularMap());
  EXPECT_DOUBLE_EQ(0.0, pbr.Glossiness());
}

/////////////////////////////////////////////////
TEST(Pbr, MoveCopy)
{
  // copy constructor
  {
    ignition::common::Pbr pbr;
    pbr.SetType(ignition::common::PbrType::SPECULAR);
    pbr.SetAlbedoMap("specular_albedo_map.png");
    pbr.SetNormalMap("specular_normal_map.png");
    pbr.SetEnvironmentMap("specular_env_map.png");
    pbr.SetAmbientOcclusionMap("specular_ambient_occlusion_map.png");
    pbr.SetEmissiveMap("specular_emissive_map.png");
    pbr.SetLightMap("specular_light_map.png", 2u);
    pbr.SetGlossinessMap("glossiness_map.png");
    pbr.SetSpecularMap("specular_map.png");
    pbr.SetGlossiness(0.1);

    ignition::common::Pbr pbr2(pbr);
    EXPECT_EQ(ignition::common::PbrType::SPECULAR, pbr2.Type());
    EXPECT_EQ("specular_albedo_map.png", pbr2.AlbedoMap());
    EXPECT_EQ("specular_normal_map.png", pbr2.NormalMap());
    EXPECT_EQ(ignition::common::NormalMapSpace::TANGENT, pbr2.NormalMapType());
    EXPECT_EQ("specular_env_map.png", pbr2.EnvironmentMap());
    EXPECT_EQ("specular_ambient_occlusion_map.png",
        pbr2.AmbientOcclusionMap());
    EXPECT_EQ("specular_emissive_map.png", pbr2.EmissiveMap());
    EXPECT_EQ("specular_light_map.png", pbr2.LightMap());
    EXPECT_EQ(2u, pbr2.LightMapTexCoordSet());
    EXPECT_EQ("specular_map.png", pbr2.SpecularMap());
    EXPECT_EQ("glossiness_map.png", pbr2.GlossinessMap());
    EXPECT_DOUBLE_EQ(0.1, pbr2.Glossiness());

    EXPECT_EQ(std::string(), pbr2.RoughnessMap());
    EXPECT_EQ(std::string(), pbr2.MetalnessMap());
    EXPECT_DOUBLE_EQ(0.5, pbr2.Roughness());
    EXPECT_DOUBLE_EQ(0.0, pbr2.Metalness());
  }

  // move
  {
    ignition::common::Pbr pbr;
    pbr.SetType(ignition::common::PbrType::METAL);
    pbr.SetAlbedoMap("metal_albedo_map.png");
    pbr.SetNormalMap("metal_normal_map.png",
        ignition::common::NormalMapSpace::TANGENT);
    pbr.SetEnvironmentMap("metal_env_map.png");
    pbr.SetAmbientOcclusionMap("metal_ambient_occlusion_map.png");
    pbr.SetEmissiveMap("metal_emissive_map.png");
    pbr.SetLightMap("metal_light_map.png", 3u);
    pbr.SetRoughnessMap("roughness_map.png");
    pbr.SetMetalnessMap("metalness_map.png");
    pbr.SetRoughness(0.8);
    pbr.SetMetalness(0.3);

    ignition::common::Pbr pbr2(std::move(pbr));
    EXPECT_EQ(ignition::common::PbrType::METAL, pbr2.Type());
    EXPECT_EQ("metal_albedo_map.png", pbr2.AlbedoMap());
    EXPECT_EQ("metal_normal_map.png", pbr2.NormalMap());
    EXPECT_EQ(ignition::common::NormalMapSpace::TANGENT, pbr2.NormalMapType());
    EXPECT_EQ("metal_env_map.png", pbr2.EnvironmentMap());
    EXPECT_EQ("metal_ambient_occlusion_map.png",
        pbr2.AmbientOcclusionMap());
    EXPECT_EQ("metal_emissive_map.png", pbr2.EmissiveMap());
    EXPECT_EQ("metal_light_map.png", pbr2.LightMap());
    EXPECT_EQ(3u, pbr2.LightMapTexCoordSet());
    EXPECT_EQ("roughness_map.png", pbr2.RoughnessMap());
    EXPECT_EQ("metalness_map.png", pbr2.MetalnessMap());
    EXPECT_DOUBLE_EQ(0.8, pbr2.Roughness());
    EXPECT_DOUBLE_EQ(0.3, pbr2.Metalness());

    EXPECT_EQ(std::string(), pbr2.GlossinessMap());
    EXPECT_EQ(std::string(), pbr2.SpecularMap());
    EXPECT_DOUBLE_EQ(0.0, pbr2.Glossiness());
  }

  // move assignment
  {
    ignition::common::Pbr pbr;
    pbr.SetType(ignition::common::PbrType::METAL);
    pbr.SetAlbedoMap("metal_albedo_map.png");
    pbr.SetNormalMap("metal_normal_map.png",
        ignition::common::NormalMapSpace::TANGENT);
    pbr.SetEnvironmentMap("metal_env_map.png");
    pbr.SetAmbientOcclusionMap("metal_ambient_occlusion_map.png");
    pbr.SetEmissiveMap("metal_emissive_map.png");
    pbr.SetLightMap("metal_light_map.png", 1u);
    pbr.SetRoughnessMap("roughness_map.png");
    pbr.SetMetalnessMap("metalness_map.png");
    pbr.SetRoughness(0.8);
    pbr.SetMetalness(0.3);

    ignition::common::Pbr pbr2;
    pbr2 = std::move(pbr);
    EXPECT_EQ(ignition::common::PbrType::METAL, pbr2.Type());
    EXPECT_EQ("metal_albedo_map.png", pbr2.AlbedoMap());
    EXPECT_EQ("metal_normal_map.png", pbr2.NormalMap());
    EXPECT_EQ(ignition::common::NormalMapSpace::TANGENT, pbr2.NormalMapType());
    EXPECT_EQ("metal_env_map.png", pbr2.EnvironmentMap());
    EXPECT_EQ("metal_ambient_occlusion_map.png",
        pbr2.AmbientOcclusionMap());
    EXPECT_EQ("metal_emissive_map.png", pbr2.EmissiveMap());
    EXPECT_EQ("metal_light_map.png", pbr2.LightMap());
    EXPECT_EQ(1u, pbr2.LightMapTexCoordSet());
    EXPECT_EQ("roughness_map.png", pbr2.RoughnessMap());
    EXPECT_EQ("metalness_map.png", pbr2.MetalnessMap());
    EXPECT_DOUBLE_EQ(0.8, pbr2.Roughness());
    EXPECT_DOUBLE_EQ(0.3, pbr2.Metalness());

    EXPECT_EQ(std::string(), pbr2.GlossinessMap());
    EXPECT_EQ(std::string(), pbr2.SpecularMap());
    EXPECT_DOUBLE_EQ(0.0, pbr2.Glossiness());
  }


  // assignment
  {
    ignition::common::Pbr pbr;
    pbr.SetType(ignition::common::PbrType::METAL);
    pbr.SetAlbedoMap("metal_albedo_map.png");
    pbr.SetNormalMap("metal_normal_map.png");
    pbr.SetEnvironmentMap("metal_env_map.png");
    pbr.SetAmbientOcclusionMap("metal_ambient_occlusion_map.png");
    pbr.SetEmissiveMap("metal_emissive_map.png");
    pbr.SetLightMap("metal_light_map.png", 2u);
    pbr.SetRoughnessMap("roughness_map.png");
    pbr.SetMetalnessMap("metalness_map.png");
    pbr.SetRoughness(0.18);
    pbr.SetMetalness(0.13);

    ignition::common::Pbr pbr2;
    pbr2 = pbr;
    EXPECT_EQ(ignition::common::PbrType::METAL, pbr2.Type());
    EXPECT_EQ("metal_albedo_map.png", pbr2.AlbedoMap());
    EXPECT_EQ("metal_normal_map.png", pbr2.NormalMap());
    EXPECT_EQ("metal_env_map.png", pbr2.EnvironmentMap());
    EXPECT_EQ("metal_ambient_occlusion_map.png",
        pbr2.AmbientOcclusionMap());
    EXPECT_EQ("metal_emissive_map.png", pbr2.EmissiveMap());
    EXPECT_EQ("metal_light_map.png", pbr2.LightMap());
    EXPECT_EQ(2u, pbr2.LightMapTexCoordSet());
    EXPECT_EQ("roughness_map.png", pbr2.RoughnessMap());
    EXPECT_EQ("metalness_map.png", pbr2.MetalnessMap());
    EXPECT_DOUBLE_EQ(0.18, pbr2.Roughness());
    EXPECT_DOUBLE_EQ(0.13, pbr2.Metalness());

    EXPECT_EQ(std::string(), pbr2.GlossinessMap());
    EXPECT_EQ(std::string(), pbr2.SpecularMap());
    EXPECT_DOUBLE_EQ(0.0, pbr2.Glossiness());
  }

  // copy assignment after move
  {
    ignition::common::Pbr pbr1;
    pbr1.SetType(ignition::common::PbrType::METAL);

    ignition::common::Pbr pbr2;
    pbr2.SetType(ignition::common::PbrType::SPECULAR);

    // This is similar to what std::swap does except it uses std::move for each
    // assignment
    ignition::common::Pbr tmp = std::move(pbr1);
    pbr1 = pbr2;
    pbr2 = tmp;

    EXPECT_EQ(ignition::common::PbrType::SPECULAR, pbr1.Type());
    EXPECT_EQ(ignition::common::PbrType::METAL, pbr2.Type());
  }
}
