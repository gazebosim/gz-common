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
TEST(EncoderDecoderTest, DecodeEncodeDecode)
{
  const unsigned int fps = 25u;

  const char* testVideoInName = "test_video.mp4";
  const char* testVideoOutName = "test_video_out.mp4";
  const auto testVideoInPath = common::joinPaths(TEST_PATH, "data",
                                                 testVideoInName);
  const auto testVideoOutPath = common::joinPaths(common::cwd(),
                                                  testVideoOutName);

  Video decoder;
  decoder.Load(testVideoInPath);

  VideoEncoder encoder;
  encoder.Start("mp4", "", decoder.Width(), decoder.Height(), fps);

  const size_t bufferSize = decoder.Width() * decoder.Height() * 3;
  auto* buf = new unsigned char[bufferSize];
  size_t numInFrames = 0;
  size_t numOutFrames = 0;
  double avgIntensity = -1.0;
  while (decoder.NextFrame(&buf))
  {
    std::chrono::steady_clock::time_point stamp{
        std::chrono::milliseconds(1000) / fps * numInFrames};
    if (encoder.AddFrame(buf, decoder.Width(), decoder.Height(), stamp))
      numOutFrames++;
    numInFrames++;

    // compute average intensity of frame number 10
    if (numInFrames == 10)
      avgIntensity = computeAverageIntensity(bufferSize, buf);
  }

  EXPECT_EQ(numInFrames, 90u);
  EXPECT_GE(numOutFrames, 89u);

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

