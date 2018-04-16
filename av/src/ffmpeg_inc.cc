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
#include "ignition/common/ffmpeg_inc.hh"

using namespace ignition;

//////////////////////////////////////////////////
AVFrame *common::AVFrameAlloc(void)
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 28, 1)
  return av_frame_alloc();
#else
  return avcodec_alloc_frame();
#endif
}

//////////////////////////////////////////////////
void common::AVFrameUnref(AVFrame *_frame)
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 28, 1)
  av_frame_unref(_frame);
#else
  avcodec_get_frame_defaults(_frame);
#endif
}

//////////////////////////////////////////////////
void common::AVPacketUnref(AVPacket *_packet)
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 24, 102)
  av_packet_unref(_packet);
#else
  av_free_packet(_packet);
#endif
}

//////////////////////////////////////////////////
int common::AVCodecDecode(AVCodecContext *_codecCtx,
    AVFrame *_frame, int *_gotFrame, AVPacket *_packet)
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
  // from https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
  int ret;

  *_gotFrame = 0;

  if (_packet)
  {
    ret = avcodec_send_packet(_codecCtx, _packet);
    if (ret < 0)
    {
      return ret == AVERROR_EOF ? 0 : ret;
    }
  }

  ret = avcodec_receive_frame(_codecCtx, _frame);
  if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
  {
    return ret;
  }
  if (ret >= 0)
  {
    *_gotFrame = 1;
  }

  return 0;
#else
  // this was deprecated in ffmpeg version 3.1
  // github.com/FFmpeg/FFmpeg/commit/7fc329e2dd6226dfecaa4a1d7adf353bf2773726
# ifndef _WIN32
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
# endif
  return avcodec_decode_video2(_codecCtx, _frame, _gotFrame, _packet);
# ifndef _WIN32
#  pragma GCC diagnostic pop
# endif
#endif
}
