/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include "ignition/common/VideoEncoder.hh"
#include "test_config.h"
#include "test/util.hh"

using namespace ignition;
using namespace common;

class VideoEncoderTest : public ignition::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, StartStop)
{
  VideoEncoder video;
  EXPECT_FALSE(video.IsEncoding());
  EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);
  EXPECT_EQ(video.BitRate(), static_cast<unsigned int>(
        VIDEO_ENCODER_BITRATE_DEFAULT));

  video.Start();
  EXPECT_TRUE(video.IsEncoding());
  EXPECT_TRUE(common::exists(common::cwd() + "/TMP_RECORDING.mp4"));
  EXPECT_EQ(video.BitRate(), 920000u);

  video.Stop();
  EXPECT_FALSE(video.IsEncoding());
  EXPECT_FALSE(common::exists(common::cwd() + "/TMP_RECORDING.mpg"));

  video.Start("mpg", "", 1024, 768);
  EXPECT_TRUE(video.IsEncoding());
  EXPECT_STREQ(video.Format().c_str(), "mpg");
  EXPECT_TRUE(common::exists(common::cwd() + "/TMP_RECORDING.mpg"));

  video.Start("mp4", "", 1024, 768);
  EXPECT_TRUE(video.IsEncoding());
  EXPECT_STREQ(video.Format().c_str(), "mpg");

  video.Stop();
  EXPECT_FALSE(video.IsEncoding());
}

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, Exists)
{
  VideoEncoder video;
  EXPECT_FALSE(video.IsEncoding());
  EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);

  EXPECT_FALSE(common::exists(common::cwd() + "/TMP_RECORDING.mpg"));
  EXPECT_FALSE(common::exists(common::cwd() + "/TMP_RECORDING.mp4"));

  video.Start();
  EXPECT_TRUE(common::exists(common::cwd() + "/TMP_RECORDING.mp4"));

  video.Reset();
  EXPECT_FALSE(common::exists(common::cwd() + "/TMP_RECORDING.mp4"));
}
