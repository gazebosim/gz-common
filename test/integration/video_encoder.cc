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

#include <cmath>

#include "ignition/common/VideoEncoder.hh"
#include "ignition/common/Video.hh"
#include "test_config.h"
#include "test/util.hh"
#include "ignition/common/ffmpeg_inc.hh"

using namespace ignition;
using namespace common;
using namespace std::chrono;
auto Now = steady_clock::now;
using TimePoint = steady_clock::time_point;

class EncoderDecoderTest : public ignition::testing::AutoLogFixture { };

double
computeAverageIntensity(const size_t bufferSize, const unsigned char* buf)
{
  double avgIntensity = 0.0;
  for (size_t i = 0; i < bufferSize; ++i)
  {
    avgIntensity += static_cast<double>(buf[i]);
  }
  avgIntensity = avgIntensity / bufferSize;
  return avgIntensity;
}

/////////////////////////////////////////////////
TEST_F(EncoderDecoderTest, DecodeEncodeDecode)
{
//  av_log_set_level(AV_LOG_ERROR);

  const unsigned int fps = 25u;

  const char* testVideoInName = "test_video.mp4";
  const char* testVideoOutName = "test_video_out.mp4";
  const auto testVideoInPath = joinPaths(TEST_PATH, "data", testVideoInName);
  const auto testVideoOutPath = joinPaths(cwd(), testVideoOutName);

  Video decoder;
  decoder.Load(testVideoInPath);

  VideoEncoder encoder;
  ASSERT_TRUE(encoder.Start("mp4", "",
                            decoder.Width(), decoder.Height(), fps));

  const size_t bufferSize = decoder.Width() * decoder.Height() * 3;
  auto* buf = new unsigned char[bufferSize];
  size_t numInFrames = 0;
  size_t numOutFrames = 0;
  double avgIntensity = -1.0;

  auto fps_start = Now();
  size_t fps_frames = 0;

  decoder.NextFrame(&buf);
  while (decoder.NextFrame(&buf))
  {
    TimePoint stamp { milliseconds(1000) / fps * numInFrames };
    if (encoder.AddFrame(buf, decoder.Width(), decoder.Height(), stamp))
      numOutFrames++;
    numInFrames++;

    // compute average intensity of frame number 10
    if (numInFrames == 10)
      avgIntensity = computeAverageIntensity(bufferSize, buf);

    fps_frames++;
    if ((Now() - fps_start) > seconds(1)) {
      double dur = duration<double>(Now() - fps_start).count();
      double currentFps = fps_frames / dur;
      ignmsg << "FPS: " << currentFps << std::endl;
      fps_start = Now();
      fps_frames = 0;
    }
  }

  EXPECT_GE(numInFrames, 89u);
  EXPECT_GE(numOutFrames, 88u);

  // check that the intensity got computed and that the image is not all black
  // or all white
  ASSERT_GT(std::abs(avgIntensity - (-1.0)), 1e-6);
  EXPECT_GT(avgIntensity, 1e-6);
  EXPECT_LT(avgIntensity, 255.0 - 1e-6);

  encoder.SaveToFile(testVideoOutPath);
  ASSERT_TRUE(common::exists(testVideoOutPath));

  Video decoder2;
  decoder2.Load(testVideoOutPath);

  ASSERT_EQ(decoder2.Width(), decoder.Width());
  ASSERT_EQ(decoder2.Height(), decoder.Height());

  size_t numFrames2 = 0;
  double avgIntensity2 = -1.0;
  while (decoder2.NextFrame(&buf))
  {
    numFrames2++;
    if (numFrames2 == 10)
      avgIntensity2 = computeAverageIntensity(bufferSize, buf);
  }

  // output can differ by up to 1 sec
  EXPECT_GT(numFrames2, numInFrames - fps);
  // average color intensities should be pretty close
  EXPECT_NEAR(avgIntensity2, avgIntensity, 1.0);

  delete[] buf;
}

