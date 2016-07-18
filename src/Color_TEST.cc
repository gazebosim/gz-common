/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.A()pache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <gtest/gtest.h>

#include <ignition/common/Color.hh>

using namespace ignition;

/////////////////////////////////////////////////
TEST(Color, Color)
{
  common::Color clr(.1f, .2f, .3f, 1.0f);
  EXPECT_FLOAT_EQ(0.1f, clr.R());
  EXPECT_FLOAT_EQ(0.2f, clr.G());
  EXPECT_FLOAT_EQ(0.3f, clr.B());
  EXPECT_FLOAT_EQ(1.0f, clr.A());

  clr.Reset();
  EXPECT_FLOAT_EQ(0.0f, clr.R());
  EXPECT_FLOAT_EQ(0.0f, clr.G());
  EXPECT_FLOAT_EQ(0.0f, clr.B());
  EXPECT_FLOAT_EQ(0.0f, clr.A());

  clr.SetFromHSV(0, 0.5, 1.0);
  EXPECT_FLOAT_EQ(1.0f, clr.R());
  EXPECT_FLOAT_EQ(0.5f, clr.G());
  EXPECT_FLOAT_EQ(0.5f, clr.B());
  EXPECT_FLOAT_EQ(0.0f, clr.A());

  EXPECT_TRUE(clr.HSV() == math::Vector3f(6, 0.5, 1));

  clr.SetFromHSV(60, 0.0, 1.0);
  EXPECT_FLOAT_EQ(1.0f, clr.R());
  EXPECT_FLOAT_EQ(1.0f, clr.G());
  EXPECT_FLOAT_EQ(1.0f, clr.B());
  EXPECT_FLOAT_EQ(0.0f, clr.A());

  clr.SetFromHSV(120, 0.5, 1.0);
  EXPECT_FLOAT_EQ(0.5f, clr.R());
  EXPECT_FLOAT_EQ(1.0f, clr.G());
  EXPECT_FLOAT_EQ(0.5f, clr.B());
  EXPECT_FLOAT_EQ(0.0f, clr.A());

  clr.SetFromHSV(180, 0.5, 1.0);
  EXPECT_FLOAT_EQ(0.5f, clr.R());
  EXPECT_FLOAT_EQ(1.0f, clr.G());
  EXPECT_FLOAT_EQ(1.0f, clr.B());
  EXPECT_FLOAT_EQ(0.0f, clr.A());

  clr.SetFromHSV(240, 0.5, 1.0);
  EXPECT_FLOAT_EQ(0.5f, clr.R());
  EXPECT_FLOAT_EQ(0.5f, clr.G());
  EXPECT_FLOAT_EQ(1.0f, clr.B());
  EXPECT_FLOAT_EQ(0.0f, clr.A());

  clr.SetFromHSV(300, 0.5, 1.0);
  EXPECT_FLOAT_EQ(1.0f, clr[0]);
  EXPECT_FLOAT_EQ(0.5f, clr[1]);
  EXPECT_FLOAT_EQ(1.0f, clr[2]);
  EXPECT_FLOAT_EQ(0.0f, clr[3]);
  EXPECT_FLOAT_EQ(0.0f, clr[4]);

  clr.R() = 0.1f;
  clr.G() = 0.2f;
  clr.B() = 0.3f;
  clr.A() = 0.4f;
  EXPECT_FLOAT_EQ(0.1f, clr[0]);
  EXPECT_FLOAT_EQ(0.2f, clr[1]);
  EXPECT_FLOAT_EQ(0.3f, clr[2]);
  EXPECT_FLOAT_EQ(0.4f, clr[3]);

  clr.Set(0.1f, 0.2f, 0.3f, 0.4f);
  clr = clr + 0.2f;
  EXPECT_TRUE(clr == common::Color(0.3f, 0.4f, 0.5f, 0.6f));

  clr.Set(0.1f, 0.2f, 0.3f, 0.4f);
  clr += common::Color(0.2f, 0.2f, 0.2f, 0.2f);
  EXPECT_TRUE(clr == common::Color(0.3f, 0.4f, 0.5f, 0.6f));


  clr.Set(0.1f, 0.2f, 0.3f, 0.4f);
  clr = clr - 0.1f;
  EXPECT_TRUE(clr == common::Color(0.0f, 0.1f, 0.2f, 0.3f));

  clr.Set(0.1f, 0.2f, 0.3f, 0.4f);
  clr -= common::Color(0.1f, 0.1f, 0.1f, 0.1f);
  EXPECT_TRUE(clr == common::Color(0.0f, 0.1f, 0.2f, 0.3f));


  clr.Set(1.f, 1.f, 1.f, 1.f);
  clr = clr / 1.6f;
  EXPECT_TRUE(clr == common::Color(0.625f, 0.625f, 0.625f, 0.625f));

  clr.Set(1.f, 1.f, 1.f, 1.f);
  clr /= common::Color(1.f, 1.f, 1.f, 1.f);
  EXPECT_TRUE(clr == common::Color(1.f, 1.f, 1.f, 1.f));


  clr.Set(.1f, .2f, .3f, .4f);
  clr = clr * .1f;
  EXPECT_TRUE(clr == common::Color(0.01f, 0.02f, 0.03f, 0.04f));

  clr.Set(.1f, .2f, .3f, .4f);
  clr *= common::Color(0.1f, 0.1f, 0.1f, 0.1f);
  EXPECT_TRUE(clr == common::Color(0.01f, 0.02f, 0.03f, 0.04f));


  clr.SetFromYUV(0.5f, 0.2f, 0.8f);
  EXPECT_TRUE(math::equal(0.00553f, clr.R(), 1e-3f));
  EXPECT_TRUE(math::equal(0.0f, clr.G()));
  EXPECT_TRUE(math::equal(0.9064f, clr.B(), 1e-3f));
  EXPECT_TRUE(math::equal(0.04f, clr.A()));

  EXPECT_TRUE(clr.YUV() == math::Vector3f(0.104985f, 0.95227f, 0.429305f));

  clr = common::Color(1.0f, 0.0f, 0.5f, 1.0f) +
    common::Color(0.1f, 0.3f, 0.4f, 1.0f);
  EXPECT_TRUE(math::equal(0.00431373f, clr.R()));
  EXPECT_TRUE(math::equal(0.3f, clr.G()));
  EXPECT_TRUE(math::equal(0.9f, clr.B()));
  EXPECT_TRUE(math::equal(2.0f, clr.A()));

  clr = common::Color(1.0f, 0.0f, 0.5f, 1.0f) -
    common::Color(0.1f, 0.3f, 0.4f, 1.0f);
  EXPECT_TRUE(math::equal(0.9f, clr.R()));
  EXPECT_TRUE(math::equal(0.0f, clr.G()));
  EXPECT_TRUE(math::equal(0.1f, clr.B()));
  EXPECT_TRUE(math::equal(0.0f, clr.A()));

  clr = common::Color(0.5f, 0.2f, 0.4f, 0.6f) / 2.0f;
  EXPECT_TRUE(math::equal(0.25f, clr.R()));
  EXPECT_TRUE(math::equal(0.1f, clr.G()));
  EXPECT_TRUE(math::equal(0.2f, clr.B()));
  EXPECT_TRUE(math::equal(0.3f, clr.A()));
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
