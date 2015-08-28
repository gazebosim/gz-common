/*
 * Copyright (C) 2012-2015 Open Source Robotics Foundation
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

#include "ignition/common/Material.hh"

using namespace ignition;

TEST(MaterialTest, Material)
{
  common::Material mat(common::Color(1.0, 0.5, 0.2, 1.0));
  EXPECT_TRUE(mat.Ambient() == common::Color(1.0, 0.5, 0.2, 1.0));
  EXPECT_TRUE(mat.Diffuse() == common::Color(1.0, 0.5, 0.2, 1.0));
  EXPECT_STREQ("ign-common_material_0", mat.Name().c_str());

  mat.SetTextureImage("texture_image");
  EXPECT_STREQ("texture_image", mat.TextureImage().c_str());

  mat.SetTextureImage("texture_image", "/path");
  EXPECT_STREQ("/path/../materials/textures/texture_image",
               mat.TextureImage().c_str());

  mat.SetAmbient(common::Color(0.1, 0.2, 0.3, 0.4));
  EXPECT_TRUE(mat.Ambient() == common::Color(0.1, 0.2, 0.3, 0.4));

  mat.SetDiffuse(common::Color(0.1, 0.2, 0.3, 0.4));
  EXPECT_TRUE(mat.Diffuse() == common::Color(0.1, 0.2, 0.3, 0.4));

  mat.SetSpecular(common::Color(0.1, 0.2, 0.3, 0.4));
  EXPECT_TRUE(mat.Specular() == common::Color(0.1, 0.2, 0.3, 0.4));

  mat.SetEmissive(common::Color(0.1, 0.2, 0.3, 0.4));
  EXPECT_TRUE(mat.Emissive() == common::Color(0.1, 0.2, 0.3, 0.4));

  mat.SetTransparency(0.2);
  EXPECT_DOUBLE_EQ(0.2, mat.Transparency());

  mat.SetShininess(0.2);
  EXPECT_DOUBLE_EQ(0.2, mat.Shininess());

  mat.SetBlendFactors(.1, .5);
  double a, b;
  mat.BlendFactors(a, b);
  EXPECT_DOUBLE_EQ(.1, a);
  EXPECT_DOUBLE_EQ(0.5, b);

  mat.SetBlendMode(common::Material::MODULATE);
  EXPECT_EQ(common::Material::MODULATE, mat.BlendMode());

  mat.SetShadeMode(common::Material::BLINN);
  EXPECT_EQ(common::Material::BLINN, mat.ShadeMode());

  mat.SetPointSize(0.2);
  EXPECT_DOUBLE_EQ(0.2, mat.PointSize());

  mat.SetDepthWrite(false);
  EXPECT_FALSE(mat.DepthWrite());

  mat.SetLighting(true);
  EXPECT_TRUE(mat.Lighting());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
