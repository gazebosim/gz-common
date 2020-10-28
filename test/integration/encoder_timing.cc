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
#include "test_config.h"
#include "test/util.hh"

using namespace ignition;
using namespace common;

const unsigned int kSize = 10;
const unsigned char *kFrame = new unsigned char[kSize*kSize];

// set to 720ms because video duration missing additional 18 frames
//    which may be due to how video encoding works
const int kTol = 720;

void durationTest(VideoEncoder &_vidEncoder, Video &_video,
                  const int &_fps, const int &_seconds)
{
  _vidEncoder.Start("mp4", "", kSize, kSize, _fps, 0);

  int frameCount = 0;
  while (frameCount != _fps*_seconds)
  {
    if (_vidEncoder.AddFrame(kFrame, kSize, kSize))
      ++frameCount;
  }

  _vidEncoder.Stop();
  _video.Load(common::cwd() + "/TMP_RECORDING.mp4");

  EXPECT_NEAR(std::chrono::duration_cast<std::chrono::milliseconds>(
              _video.Duration()).count(),
              _seconds*1000,
              kTol);
}

TEST(EncoderTimingTest, Duration)
{
  VideoEncoder vidEncoder;
  Video video;

  durationTest(vidEncoder, video, 50, 1);
  durationTest(vidEncoder, video, 30, 2);
  durationTest(vidEncoder, video, 25, 5);

  delete [] kFrame;
}
