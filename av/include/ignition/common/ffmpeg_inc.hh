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
#ifndef IGNITION_COMMON_FFMPEG_INC_HH_
#define IGNITION_COMMON_FFMPEG_INC_HH_

#include <ignition/common/config.hh>

#ifndef _WIN32
#pragma GCC system_header
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#ifndef AV_ERROR_MAX_STRING_SIZE
#define AV_ERROR_MAX_STRING_SIZE 64
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>

#if defined(__linux__) && defined(HAVE_AVDEVICE)
#include <libavdevice/avdevice.h>
#endif
}

#include <ignition/common/av/Export.hh>

namespace ignition
{
  namespace common
  {
    /// \brief Helper function to avoid deprecation warnings.
    IGNITION_COMMON_AV_VISIBLE
    AVFrame *AVFrameAlloc(void);

    /// \brief Helper function to avoid deprecation warnings.
    IGNITION_COMMON_AV_VISIBLE
    void AVFrameUnref(AVFrame *_frame);

    /// \brief Helper function to avoid deprecation warnings.
    /// \param[in] _packet AVPacket structure that stores compressed data
    IGNITION_COMMON_AV_VISIBLE
    void AVPacketUnref(AVPacket *_packet);

    /// \brief Helper function to avoid deprecation warnings
    /// from av_codec_decode_video2.
    /// \param[in] _codecCtx Codec context.
    /// \param[out] _frame AVFrame in which decoded video frame is stored.
    /// \param[out] _gotFrame Zero if no frame could be decompressed,
    /// otherwise nonzero.
    /// \param[in] _packet AVPacket structure that stores compressed data.
    /// \return On error a negative value is returned, otherwise
    /// the number of bytes used or zero if no frame could be decompressed.
    IGNITION_COMMON_AV_VISIBLE
    int AVCodecDecode(AVCodecContext *_codecCtx,
        AVFrame *_frame, int *_gotFrame, AVPacket *_packet);
  }
}

#endif
