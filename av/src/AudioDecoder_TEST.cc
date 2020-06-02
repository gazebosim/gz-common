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

#include <ignition/common/AudioDecoder.hh>

#include "test_util.hh"
#include "test_config.h"

using namespace ignition;

/////////////////////////////////////////////////
TEST(AudioDecoder, FileNotSet)
{
  common::AudioDecoder audio;
  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;
  EXPECT_FALSE(audio.Decode(&dataBuffer, &dataBufferSize));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, MissingFile)
{
  common::AudioDecoder audio;
  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;
  EXPECT_FALSE(audio.Decode(&dataBuffer, &dataBufferSize));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, BufferSizeInvalid)
{
  common::AudioDecoder audio;
  std::string path = TEST_PATH;
  path += "/data/cheer.wav";
  EXPECT_TRUE(audio.SetFile(path));

  unsigned int *dataBufferSize = NULL;
  uint8_t *dataBuffer = NULL;
  EXPECT_FALSE(audio.Decode(&dataBuffer, dataBufferSize));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, DataBuffer)
{
  common::AudioDecoder audio;

  std::string path = PROJECT_SOURCE_PATH;
  path += "/data/cheer.wav";
  EXPECT_TRUE(audio.SetFile(path));

  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;
  EXPECT_TRUE(audio.Decode(&dataBuffer, &dataBufferSize));

  unsigned int dataBufferSize2;
  uint8_t *dataBuffer2 = new uint8_t[5];
  EXPECT_TRUE(audio.Decode(&dataBuffer2, &dataBufferSize2));

  EXPECT_EQ(dataBufferSize2, dataBufferSize);
  EXPECT_EQ(sizeof(dataBuffer), sizeof(dataBuffer2));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, NoCodec)
{
  common::AudioDecoder audio;
  std::string path = TEST_PATH;
  path += "/data/audio_bad_codec.mp4";
  EXPECT_FALSE(audio.SetFile(path));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, CheerFile)
{
  common::AudioDecoder audio;

  // Test a bad filename
  EXPECT_FALSE(audio.SetFile("_bad_audio_filename_.wav"));

  // Test no stream info
  std::string path = TEST_PATH;
  path += "/data/audio_bad_codec.grf";
  EXPECT_FALSE(audio.SetFile(path));

  // Test a valid file without an audio stream
  path = TEST_PATH;
  path += "/data/empty_audio.mp4";
  EXPECT_FALSE(audio.SetFile(path));

  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;

  // WAV
  {
    path = TEST_PATH;
    path += "/data/cheer.wav";
    EXPECT_TRUE(audio.SetFile(path));
    EXPECT_EQ(audio.GetFile(), path);
    EXPECT_EQ(audio.GetSampleRate(), 48000);

    audio.Decode(&dataBuffer, &dataBufferSize);
    EXPECT_EQ(dataBufferSize, 5428692u);
  }

  // OGG
  {
    path = TEST_PATH;
    path += "/data/cheer.ogg";
    EXPECT_TRUE(audio.SetFile(path));
    EXPECT_EQ(audio.GetFile(), path);
    EXPECT_EQ(audio.GetSampleRate(), 44100);

    audio.Decode(&dataBuffer, &dataBufferSize);
    // In Ubuntu trusty the buffer size double for ogg decoding.
    // This check is suitable for both older and newer versions of Ubuntu.
    EXPECT_TRUE(dataBufferSize == 4989184u ||
                dataBufferSize == 4989184u * 2u);
  }

  // MP3
  {
    path = TEST_PATH;
    path += "/data/cheer.mp3";
    EXPECT_TRUE(audio.SetFile(path));
    EXPECT_EQ(audio.GetFile(), path);
    EXPECT_EQ(audio.GetSampleRate(), 44100);

    audio.Decode(&dataBuffer, &dataBufferSize);
    EXPECT_EQ(dataBufferSize, 4995072u);
  }
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
