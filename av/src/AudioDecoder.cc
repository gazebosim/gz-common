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

#include <gz/common/av/Util.hh>
#include <gz/common/ffmpeg_inc.hh>
#include <gz/common/AudioDecoder.hh>
#include <gz/common/Console.hh>

#define AUDIO_INBUF_SIZE (20480 * 2)
#define AUDIO_REFILL_THRESH 4096

using namespace ignition;
using namespace common;

class gz::common::AudioDecoderPrivate
{
  /// \brief libav Format I/O context.
  public: AVFormatContext *formatCtx;

  /// \brief libav main external API structure.
  public: AVCodecContext *codecCtx;

  /// \brief libavcodec audio codec.
  public: const AVCodec *codec;

  /// \brief Index of the audio stream.
  public: int audioStream;

  /// \brief Audio file to decode.
  public: std::string filename;
};

/////////////////////////////////////////////////
AudioDecoder::AudioDecoder()
  : data(new AudioDecoderPrivate)
{
  load();

  this->data->formatCtx = nullptr;
  this->data->codecCtx = nullptr;
  this->data->codec = nullptr;
  this->data->audioStream = 0;
}

/////////////////////////////////////////////////
AudioDecoder::~AudioDecoder()
{
  this->Cleanup();
}

/////////////////////////////////////////////////
void AudioDecoder::Cleanup()
{
  // Close the codec
  if (this->data->codecCtx)
  {
#if LIBAVFORMAT_VERSION_MAJOR < 59
    avcodec_close(this->data->codecCtx);
#else
    avcodec_free_context(&this->data->codecCtx);
#endif
  }

  // Close the audio file
  if (this->data->formatCtx)
    avformat_close_input(&this->data->formatCtx);
}

/////////////////////////////////////////////////
bool AudioDecoder::Decode(uint8_t **_outBuffer, unsigned int *_outBufferSize)
{
#if LIBAVFORMAT_VERSION_MAJOR < 59
  AVPacket *packet, packet1;
  int bytesDecoded = 0;
#else
  AVPacket *packet;
#endif
  unsigned int maxBufferSize = 0;
  AVFrame *decodedFrame = nullptr;

  if (this->data->codec == nullptr)
  {
    ignerr << "Set an audio file before decoding.\n";
    return false;
  }

  if (_outBufferSize == nullptr)
  {
    ignerr << "outBufferSize is null!!\n";
    return false;
  }

  *_outBufferSize = 0;

  if (*_outBuffer)
  {
    free(*_outBuffer);
    *_outBuffer = nullptr;
  }

  bool result = true;

  if (!(decodedFrame = AVFrameAlloc()))
  {
    ignerr << "Audio decoder out of memory\n";
    result = false;
  }

  packet = av_packet_alloc();
  if (!packet)
  {
    ignerr << "Failed to allocate AVPacket" << std::endl;
    return false;
  }

  while (av_read_frame(this->data->formatCtx, packet) == 0)
  {
    if (packet->stream_index == this->data->audioStream)
    {
#if LIBAVFORMAT_VERSION_MAJOR >= 59
      // Inspired from
      // https://github.com/FFmpeg/FFmpeg/blob/n5.0/doc/examples/decode_audio.c#L71

      // send the packet with the compressed data to the decoder
      int ret = avcodec_send_packet(this->data->codecCtx, packet);
      if (ret < 0)
      {
        ignerr << "Error submitting the packet to the decoder" << std::endl;
        return false;
      }

      // read all the output frames
      // (in general there may be any number of them)
      while (ret >= 0)
      {
        ret = avcodec_receive_frame(this->data->codecCtx, decodedFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
          break;
        }
        else if (ret < 0)
        {
            ignerr << "Error during decoding" << std::endl;
            return false;
        }

        // Total size of the data. Some padding can be added to
        // decodedFrame->data[0], which is why we can't use
        // decodedFrame->linesize[0].
        int size = decodedFrame->nb_samples *
          av_get_bytes_per_sample(this->data->codecCtx->sample_fmt) *
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 24, 100)
          this->data->codecCtx->ch_layout.nb_channels;
#else
          this->data->codecCtx->channels;
#endif
        // Resize the audio buffer as necessary
        if (*_outBufferSize + size > maxBufferSize)
        {
          maxBufferSize += size * 5;
          *_outBuffer = reinterpret_cast<uint8_t*>(realloc(*_outBuffer,
                maxBufferSize * sizeof(*_outBuffer[0])));
        }

        memcpy(*_outBuffer + *_outBufferSize, decodedFrame->data[0],
            size);
        *_outBufferSize += size;
    }
#else
      int gotFrame = 0;

      packet1 = *packet;
      while (packet1.size)
      {
        // Some frames rely on multiple packets, so we have to make sure
        // the frame is finished before we can use it
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        bytesDecoded = avcodec_decode_audio4(this->data->codecCtx, decodedFrame,
            &gotFrame, &packet1);
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

        if (gotFrame)
        {
          // Total size of the data. Some padding can be added to
          // decodedFrame->data[0], which is why we can't use
          // decodedFrame->linesize[0].
          int size = decodedFrame->nb_samples *
            av_get_bytes_per_sample(this->data->codecCtx->sample_fmt) *
            this->data->codecCtx->channels;

          // Resize the audio buffer as necessary
          if (*_outBufferSize + size > maxBufferSize)
          {
            maxBufferSize += size * 5;
            *_outBuffer = reinterpret_cast<uint8_t*>(realloc(*_outBuffer,
                  maxBufferSize * sizeof(*_outBuffer[0])));
          }

          memcpy(*_outBuffer + *_outBufferSize, decodedFrame->data[0],
              size);
          *_outBufferSize += size;
        }

        packet1.data += bytesDecoded;
        packet1.size -= bytesDecoded;
      }
#endif
    }
    av_packet_unref(packet);
  }

  av_packet_unref(packet);

  // Seek to the beginning so that it can be decoded again, if necessary.
  av_seek_frame(this->data->formatCtx, this->data->audioStream, 0, 0);

  return result;
}

/////////////////////////////////////////////////
int AudioDecoder::SampleRate()
{
  if (this->data->codecCtx)
    return this->data->codecCtx->sample_rate;

  return -1;
}

/////////////////////////////////////////////////
bool AudioDecoder::SetFile(const std::string &_filename)
{
  unsigned int i;

  this->data->formatCtx = avformat_alloc_context();

  // Open file
  if (avformat_open_input(&this->data->formatCtx,
        _filename.c_str(), nullptr, nullptr) < 0)
  {
    ignerr << "Unable to open audio file[" << _filename << "]\n";
    this->data->formatCtx = nullptr;
    return false;
  }

  // Hide av logging
  av_log_set_level(0);

  // Retrieve some information
  if (avformat_find_stream_info(this->data->formatCtx, nullptr) < 0)
  {
    ignerr << "Unable to find stream info.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = nullptr;

    return false;
  }

  // Dump information about file onto standard error.
  // dump_format(this->data->formatCtx, 0, "dump.txt", false);

  // Find audio stream;
  this->data->audioStream = -1;
  for (i = 0; i < this->data->formatCtx->nb_streams; ++i)
  {
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if LIBAVFORMAT_VERSION_MAJOR >= 59
    if (this->data->formatCtx->streams[i]->codecpar->codec_type == // NOLINT(*)
        AVMEDIA_TYPE_AUDIO)
#else
    if (this->data->formatCtx->streams[i]->codec->codec_type == // NOLINT(*)
        AVMEDIA_TYPE_AUDIO)
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
    {
      this->data->audioStream = i;
      break;
    }
  }

  if (this->data->audioStream == -1)
  {
    ignerr << "Couldn't find audio stream.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = nullptr;

    return false;
  }

  // Get the audio stream codec
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if LIBAVFORMAT_VERSION_MAJOR < 59
  this->data->codecCtx = this->data->formatCtx->streams[
    this->data->audioStream]->codec;
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

  // Find a decoder
#if LIBAVFORMAT_VERSION_MAJOR >= 59
  this->data->codec = avcodec_find_decoder(this->data->formatCtx->streams[
    this->data->audioStream]->codecpar->codec_id);
  if (!this->data->codec)
  {
    ignerr << "Failed to find the codec" << std::endl;
    return false;
  }
  this->data->codecCtx = avcodec_alloc_context3(this->data->codec);
  if (!this->data->codecCtx)
  {
    ignerr << "Failed to allocate the codec context" << std::endl;
    return false;
  }
  // Copy all relevant parameters from codepar to codecCtx
  avcodec_parameters_to_context(this->data->codecCtx,
    this->data->formatCtx->streams[this->data->audioStream]->codecpar);
#else
  this->data->codec = avcodec_find_decoder(this->data->codecCtx->codec_id);
#endif

  if (this->data->codec == nullptr)
  {
    ignerr << "Couldn't find codec for audio stream.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = nullptr;

    return false;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(56, 60, 100)
#if LIBAVCODEC_VERSION_MAJOR < 60
  if (this->data->codec->capabilities & AV_CODEC_CAP_TRUNCATED)
    this->data->codecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;
#endif
#else
  if (this->data->codec->capabilities & CODEC_CAP_TRUNCATED)
    this->data->codecCtx->flags |= CODEC_FLAG_TRUNCATED;
#endif

  // Open codec
  if (avcodec_open2(this->data->codecCtx, this->data->codec, nullptr) < 0)
  {
    ignerr << "Couldn't open audio codec.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = nullptr;

    return false;
  }

  this->data->filename = _filename;

  return true;
}

/////////////////////////////////////////////////
std::string AudioDecoder::File() const
{
  return this->data->filename;
}
