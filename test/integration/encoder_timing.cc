/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include "ignition/common/Video.hh"
#include "ignition/common/ffmpeg_inc.hh"
#include "test_config.h"
#include "test/util.hh"

using namespace ignition;
using namespace common;

TEST(EncoderTimingTest, Duration)
{
  VideoEncoder vidEncoder;
  vidEncoder.Start();

  unsigned int size = 10;
  unsigned char *frame = new unsigned char[size*size];

  // int milliSec = (1.0/VIDEO_ENCODER_FPS_DEFAULT)*1000; // ms btwn frames
  // for (int i = 0; i < VIDEO_ENCODER_FPS_DEFAULT; ++i)
  // {
  //   ASSERT_TRUE(vidEncoder.AddFrame(frame, size, size));
  //   std::this_thread::sleep_for(std::chrono::milliseconds(milliSec));
  // }

  int frame_count = 0;
  while (frame_count != VIDEO_ENCODER_FPS_DEFAULT)
  {
    if (vidEncoder.AddFrame(frame, size, size))
      ++frame_count;
  }

  vidEncoder.Stop();

  Video video;
  std::string path = common::cwd() + "/TMP_RECORDING.mp4";
  video.Load(path);

  EXPECT_EQ(video.Duration()*1.0/AV_TIME_BASE, 1.0);
  // EXPECT_EQ(video.Duration(), AV_TIME_BASE);
}
