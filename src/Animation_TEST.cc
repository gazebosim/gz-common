/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include <ignition/math/Vector3.hh>
#include <ignition/math/Quaternion.hh>
#include <ignition/common/KeyFrame.hh>
#include <ignition/common/Animation.hh>
#include "test/util.hh"

using namespace ignition;

class AnimationTest : public ignition::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(AnimationTest, PoseAnimation)
{
  {
    common::PoseAnimation anim("test", 1.0, true);
    anim.Time(-0.5);
    EXPECT_DOUBLE_EQ(0.5, anim.Time());
  }

  {
    common::PoseAnimation anim("test", 1.0, false);
    anim.Time(-0.5);
    EXPECT_DOUBLE_EQ(0.0, anim.Time());

    anim.Time(1.5);
    EXPECT_DOUBLE_EQ(1.0, anim.Time());
  }


  common::PoseAnimation anim("pose_test", 5.0, false);
  common::PoseKeyFrame *key = anim.CreateKeyFrame(0.0);

  EXPECT_DOUBLE_EQ(5.0, anim.Length());
  anim.Length(10.0);
  EXPECT_DOUBLE_EQ(10.0, anim.Length());

  key->Translation(math::Vector3d(0, 0, 0));
  EXPECT_TRUE(key->Translation() == math::Vector3d(0, 0, 0));

  key->Rotation(math::Quaterniond(0, 0, 0));
  EXPECT_TRUE(key->Rotation() == math::Quaterniond(0, 0, 0));

  key = anim.CreateKeyFrame(10.0);
  key->Translation(math::Vector3d(10, 20, 30));
  EXPECT_TRUE(key->Translation() == math::Vector3d(10, 20, 30));

  key->Rotation(math::Quaterniond(0.1, 0.2, 0.3));
  EXPECT_TRUE(key->Rotation() == math::Quaterniond(0.1, 0.2, 0.3));

  anim.AddTime(5.0);
  EXPECT_DOUBLE_EQ(5.0, anim.Time());

  anim.Time(4.0);
  EXPECT_DOUBLE_EQ(4.0, anim.Time());

  common::PoseKeyFrame interpolatedKey(-1.0);
  anim.InterpolatedKeyFrame(interpolatedKey);
  EXPECT_TRUE(interpolatedKey.Translation() ==
      math::Vector3d(3.76, 7.52, 11.28));
  EXPECT_TRUE(interpolatedKey.Rotation() ==
      math::Quaterniond(0.0302776, 0.0785971, 0.109824));
}

/////////////////////////////////////////////////
TEST_F(AnimationTest, NumericAnimation)
{
  common::NumericAnimation anim("numeric_test", 10, false);
  common::NumericKeyFrame *key = anim.CreateKeyFrame(0.0);

  key->Value(0.0);
  EXPECT_DOUBLE_EQ(0.0, key->Value());

  key = anim.CreateKeyFrame(10.0);
  key->Value(30);
  EXPECT_DOUBLE_EQ(30, key->Value());

  anim.AddTime(5.0);
  EXPECT_DOUBLE_EQ(5.0, anim.Time());

  anim.Time(4.0);
  EXPECT_DOUBLE_EQ(4.0, anim.Time());

  common::NumericKeyFrame interpolatedKey(0);
  anim.InterpolatedKeyFrame(interpolatedKey);
  EXPECT_DOUBLE_EQ(12, interpolatedKey.Value());
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
