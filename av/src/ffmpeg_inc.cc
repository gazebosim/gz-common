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
  return av_frame_alloc();
}

//////////////////////////////////////////////////
void common::AVFrameUnref(AVFrame *_frame)
{
  av_frame_unref(_frame);
}

//////////////////////////////////////////////////
void common::AVPacketUnref(AVPacket *_packet)
{
  av_packet_unref(_packet);
}

//////////////////////////////////////////////////
int common::AVCodecDecode(AVCodecContext *_codecCtx,
    AVFrame *_frame, int *_gotFrame, AVPacket *_packet)
{
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
  if (ret < 0 && ret != AVERROR(EAGAIN))
  {
    return ret;
  }
  if (ret >= 0)
  {
    *_gotFrame = 1;
  }

  // new API always consumes the whole packet
  return _packet ? _packet->size : 0;
}
