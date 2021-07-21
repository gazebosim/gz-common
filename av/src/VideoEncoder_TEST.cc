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

#include "ignition/common/Console.hh"
#include "ignition/common/VideoEncoder.hh"
#include "test_config.h"

using namespace ignition;
using namespace common;

class VideoEncoderTest : public common::testing::AutoLogFixture
{
  // Documentation inherited
  protected: void SetUp() override
  {
    Console::SetVerbosity(4);
  }
};

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, StartStop)
{
  auto filePathMp4 = common::joinPaths(common::cwd(), "TMP_RECORDING.mp4");
  auto filePathMpg = common::joinPaths(common::cwd(), "TMP_RECORDING.mpg");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);
    EXPECT_EQ(video.BitRate(), static_cast<unsigned int>(
          VIDEO_ENCODER_BITRATE_DEFAULT));

    video.Start();
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_TRUE(common::exists(filePathMp4)) << filePathMp4;
    EXPECT_EQ(video.BitRate(), 920000u);

    video.Stop();
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_FALSE(common::exists(filePathMpg)) << filePathMpg;

    video.Start("mpg", "", 1024, 768);
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mpg");
    EXPECT_TRUE(common::exists(filePathMpg)) << filePathMpg;

    video.Start("mp4", "", 1024, 768);
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mp4");

    video.Stop();
    EXPECT_FALSE(video.IsEncoding());
  }

  // Check that temp files are removed when video goes out of scope
  EXPECT_FALSE(common::exists(filePathMp4)) << filePathMp4;
  EXPECT_FALSE(common::exists(filePathMpg)) << filePathMpg;
}

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, Exists)
{
  auto filePathMp4 = common::joinPaths(common::cwd(), "TMP_RECORDING.mp4");
  auto filePathMpg = common::joinPaths(common::cwd(), "TMP_RECORDING.mpg");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);

    EXPECT_FALSE(common::exists(filePathMp4)) << filePathMp4;
    EXPECT_FALSE(common::exists(filePathMpg)) << filePathMpg;

    video.Start();
    EXPECT_TRUE(common::exists(filePathMp4));

    video.Reset();
    EXPECT_FALSE(common::exists(filePathMp4));
  }

  // Check that temp files are removed when video goes out of scope
  EXPECT_FALSE(common::exists(filePathMp4)) << filePathMp4;
  EXPECT_FALSE(common::exists(filePathMpg)) << filePathMpg;
}
