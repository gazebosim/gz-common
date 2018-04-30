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
#include "ignition/common/config.hh"
#include "ignition/common/Console.hh"
#include "ignition/common/ffmpeg_inc.hh"
#include "ignition/common/Video.hh"

using namespace ignition;
using namespace common;

// Private data structure for the Video class
class ignition::common::VideoPrivate
{
  /// \brief libav Format I/O context
  public: AVFormatContext *formatCtx = nullptr;

  /// \brief libav main external API structure
  public: AVCodecContext *codecCtx = nullptr;

  /// \brief audio video frame
  public: AVFrame *avFrame = nullptr;

  /// \brief Destination audio video frame
  public: AVFrame *avFrameDst = nullptr;

  /// \brief software scaling context
  public: SwsContext *swsCtx = nullptr;

  /// \brief index of first video stream or -1
  public: int videoStream = -1;
};

/////////////////////////////////////////////////
Video::Video()
: dataPtr(new VideoPrivate)
{
}

/////////////////////////////////////////////////
Video::~Video()
{
  this->Cleanup();
}

/////////////////////////////////////////////////
void Video::Cleanup()
{
  // Free the YUV frame
  av_free(this->dataPtr->avFrame);

  // Close the video file
  avformat_close_input(&this->dataPtr->formatCtx);

  // Close the codec
  avcodec_close(this->dataPtr->codecCtx);

  av_free(this->dataPtr->avFrameDst);
}

/////////////////////////////////////////////////
bool Video::Load(const std::string &_filename)
{
  AVCodec *codec = nullptr;
  this->dataPtr->videoStream = -1;

  if (this->dataPtr->formatCtx || this->dataPtr->avFrame ||
      this->dataPtr->codecCtx)
  {
    this->Cleanup();
  }

  this->dataPtr->avFrame = common::AVFrameAlloc();

  // Open video file
  if (avformat_open_input(&this->dataPtr->formatCtx, _filename.c_str(),
        nullptr, nullptr) < 0)
  {
    ignerr << "Unable to read video file[" << _filename << "]\n";
    return false;
  }

  // Retrieve stream information
  if (avformat_find_stream_info(this->dataPtr->formatCtx, nullptr) < 0)
  {
    ignerr << "Couldn't find stream information\n";
    return false;
  }

  // Find the first video stream
  for (unsigned int i = 0; i < this->dataPtr->formatCtx->nb_streams; ++i)
  {
    enum AVMediaType codec_type;
    // codec parameter deprecated in ffmpeg version 3.1
    // github.com/FFmpeg/FFmpeg/commit/9200514ad8717c
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
    codec_type = this->dataPtr->formatCtx->streams[i]->codecpar->codec_type;
#else
    codec_type = this->dataPtr->formatCtx->streams[i]->codec->codec_type;
#endif
    if (codec_type == AVMEDIA_TYPE_VIDEO)
    {
      this->dataPtr->videoStream = static_cast<int>(i);
      break;
    }
  }

  if (this->dataPtr->videoStream == -1)
  {
    ignerr << "Unable to find a video stream\n";
    return false;
  }

  // Find the decoder for the video stream
  auto stream = this->dataPtr->formatCtx->streams[this->dataPtr->videoStream];
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
  codec = avcodec_find_decoder(stream->codecpar->codec_id);
#else
  codec = avcodec_find_decoder(stream->codec->codec_id);
#endif
  if (codec == nullptr)
  {
    ignerr << "Codec not found\n";
    return false;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
  // AVCodecContext is not included in an AVStream as of ffmpeg 3.1
  // allocate a codec context based on updated example
  // github.com/FFmpeg/FFmpeg/commit/bba6a03b2816d805d44bce4f9701a71f7d3f8dad
  this->dataPtr->codecCtx = avcodec_alloc_context3(codec);
  if (!this->dataPtr->codecCtx)
  {
    ignerr << "Failed to allocate the codec context" << std::endl;
    return false;
  }

  // Copy codec parameters from input stream to output codec context
  if (avcodec_parameters_to_context(this->dataPtr->codecCtx,
                                    stream->codecpar) < 0)
  {
    ignerr << "Failed to copy codec parameters to decoder context"
           << std::endl;
    return false;
  }
#else
  // Get a pointer to the codec context for the video stream
  this->dataPtr->codecCtx = this->dataPtr->formatCtx->streams[
    this->dataPtr->videoStream]->codec;
#endif

  // Inform the codec that we can handle truncated bitstreams -- i.e.,
  // bitstreams where frame boundaries can fall in the middle of packets
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(56, 60, 100)
  if (codec->capabilities & AV_CODEC_CAP_TRUNCATED)
    this->dataPtr->codecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;
#else
  if (codec->capabilities & CODEC_CAP_TRUNCATED)
    this->dataPtr->codecCtx->flags |= CODEC_FLAG_TRUNCATED;
#endif

  // Open codec
  if (avcodec_open2(this->dataPtr->codecCtx, codec, nullptr) < 0)
  {
    ignerr << "Could not open codec\n";
    return false;
  }

  this->dataPtr->swsCtx = sws_getContext(
      this->dataPtr->codecCtx->width,
      this->dataPtr->codecCtx->height,
      this->dataPtr->codecCtx->pix_fmt,
      this->dataPtr->codecCtx->width,
      this->dataPtr->codecCtx->height,
      AV_PIX_FMT_RGB24,
      SWS_BICUBIC, nullptr, nullptr, nullptr);

  if (this->dataPtr->swsCtx == nullptr)
  {
    ignerr << "Error while calling sws_getContext\n";
    return false;
  }

  this->dataPtr->avFrameDst = common::AVFrameAlloc();
  this->dataPtr->avFrameDst->format = this->dataPtr->codecCtx->pix_fmt;
  this->dataPtr->avFrameDst->width = this->dataPtr->codecCtx->width;
  this->dataPtr->avFrameDst->height = this->dataPtr->codecCtx->height;
  av_image_alloc(this->dataPtr->avFrameDst->data,
      this->dataPtr->avFrameDst->linesize,
      this->dataPtr->codecCtx->width, this->dataPtr->codecCtx->height,
      this->dataPtr->codecCtx->pix_fmt, 1);

  // DEBUG: Will save all the frames
  // Image img;
  // char buf[1024];
  // int frame = 0;

  // // the decoding loop, running until EOF
  // while (this->GetNextFrame(img))
  // {
  //   printf("WH[%d %d]\n",this->dataPtr->codecCtx->width,
  //   this->dataPtr->codecCtx->height);
  //   snprintf(buf, sizeof(buf), "/tmp/test_%3d.png", frame++);
  //   img.SavePNG(buf);
  // }
  // printf("Done\n");

  return true;
}

/////////////////////////////////////////////////
bool Video::NextFrame(unsigned char **_buffer)
{
  AVPacket packet, tmpPacket;
  int frameAvailable = 0;

  av_init_packet(&packet);

  // Read a frame.
  if (av_read_frame(this->dataPtr->formatCtx, &packet) < 0)
    return false;

  if (packet.stream_index == this->dataPtr->videoStream)
  {
    tmpPacket.data = packet.data;
    tmpPacket.size = packet.size;

    // Process all the data in the frame
    while (tmpPacket.size > 0)
    {
      // sending data to libavcodec
      int processedLength = AVCodecDecode(this->dataPtr->codecCtx,
          this->dataPtr->avFrame, &frameAvailable, &tmpPacket);

      if (processedLength < 0)
      {
        ignerr << "Error while processing the data\n";
        break;
      }

      tmpPacket.data = tmpPacket.data + processedLength;
      tmpPacket.size = tmpPacket.size - processedLength;

      // processing the image if available
      if (frameAvailable)
      {
        sws_scale(this->dataPtr->swsCtx, this->dataPtr->avFrame->data,
            this->dataPtr->avFrame->linesize, 0,
            this->dataPtr->codecCtx->height, this->dataPtr->avFrameDst->data,
            this->dataPtr->avFrameDst->linesize);

        memcpy(*_buffer, this->dataPtr->avFrameDst->data[0],
            this->dataPtr->codecCtx->height *
            (this->dataPtr->codecCtx->width*3));

        // Debug:
        // pgm_save(this->pic.data[0], this->pic.linesize[0],
        // this->dataPtr->codecCtx->width,
        // this->dataPtr->codecCtx->height, buf);
      }
    }
  }
  AVPacketUnref(&packet);

  return true;
}

/////////////////////////////////////////////////
int Video::Width() const
{
  return this->dataPtr->codecCtx->width;
}

/////////////////////////////////////////////////
int Video::Height() const
{
  return this->dataPtr->codecCtx->height;
}
