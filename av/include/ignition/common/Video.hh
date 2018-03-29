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
#ifndef IGNITION_COMMON_VIDEO_HH_
#define IGNITION_COMMON_VIDEO_HH_

#include <string>
#include <memory>

#include <ignition/common/av/Export.hh>
#include <ignition/common/SuppressWarning.hh>

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPicture;
struct SwsContext;

namespace ignition
{
  namespace common
  {
    // Forward declare private data class
    class VideoPrivate;

    /// \brief Handle video encoding and decoding using libavcodec
    class IGNITION_COMMON_AV_VISIBLE Video
    {
      /// \brief Constructor
      public: Video();

      /// \brief Destructor
      public: virtual ~Video();

      /// \brief Load a video file
      /// \param[in] _filename Full path of the video file
      /// \return false if  a video stream can't be found
      public: bool Load(const std::string &_filename);

      /// \brief Get the width of the video in pixels
      /// \return the width
      public: int Width() const;

      /// \brief Get the height of the video in pixels
      /// \return the height
      public: int Height() const;

      /// \brief Get the next frame of the video.
      /// \param[out] _img Image in which the frame is stored
      /// \return  false on error
      public: bool NextFrame(unsigned char **_buffer);

      /// \brief free up open Video object, close files, streams
      private: void Cleanup();

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Private data pointer
      private: std::unique_ptr<VideoPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}
#endif
