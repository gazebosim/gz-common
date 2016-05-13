/*
* Copyright (C) 2014 Open Source Robotics Foundation
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

#include <ignition/common/ffmpeg_inc.hh>
#include <ignition/common/AudioDecoder.hh>
#include <ignition/common/AudioDecoderPrivate.hh>
#include <ignition/common/Console.hh>

#define AUDIO_INBUF_SIZE (20480 * 2)
#define AUDIO_REFILL_THRESH 4096

using namespace ignition;
using namespace common;

class Initializer
{
  public: Initializer()
  {
#ifdef HAVE_FFMPEG
    static bool first = true;
    if (first)
    {
      first = false;
      avcodec_register_all();
      av_register_all();
    }
#endif
  }
};
static Initializer init;

/////////////////////////////////////////////////
AudioDecoder::AudioDecoder()
  : data(new AudioDecoderPrivate)
{
  this->data->formatCtx = NULL;
  this->data->codecCtx = NULL;
  this->data->codec = NULL;
  this->data->audioStream = 0;
}

/////////////////////////////////////////////////
AudioDecoder::~AudioDecoder()
{
  this->Cleanup();
  delete this->data;
  this->data = NULL;
}

/////////////////////////////////////////////////
void AudioDecoder::Cleanup()
{
#ifdef HAVE_FFMPEG
  // Close the codec
  if (this->data->codecCtx)
    avcodec_close(this->data->codecCtx);

  // Close the audio file
  if (this->data->formatCtx)
    avformat_close_input(&this->data->formatCtx);
#endif
}

/////////////////////////////////////////////////
#ifdef HAVE_FFMPEG
bool AudioDecoder::Decode(uint8_t **_outBuffer, unsigned int *_outBufferSize)
{
  AVPacket packet, packet1;
  int bytesDecoded = 0;
  unsigned int maxBufferSize = 0;
  AVFrame *decodedFrame = NULL;

  if (this->data->codec == NULL)
  {
    ignerr << "Set an audio file before decoding.\n";
    return false;
  }

  if (_outBufferSize == NULL)
  {
    ignerr << "outBufferSize is NULL!!\n";
    return false;
  }

  *_outBufferSize = 0;

  if (*_outBuffer)
  {
    delete [] *_outBuffer;
    *_outBuffer = NULL;
  }

  bool result = true;

  if (!decodedFrame)
  {
    if (!(decodedFrame = avcodec_alloc_frame()))
    {
      ignerr << "Audio decoder out of memory\n";
      result = false;
    }
  }
  else
    avcodec_get_frame_defaults(decodedFrame);

  av_init_packet(&packet);
  while (av_read_frame(this->data->formatCtx, &packet) == 0)
  {
    if (packet.stream_index == this->data->audioStream)
    {
      int gotFrame = 0;

      packet1 = packet;
      while (packet1.size)
      {
        // Some frames rely on multiple packets, so we have to make sure
        // the frame is finished before we can use it
        bytesDecoded = avcodec_decode_audio4(this->data->codecCtx, decodedFrame,
            &gotFrame, &packet1);

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
    }
    av_free_packet(&packet);
  }

  av_free_packet(&packet);

  // Seek to the beginning so that it can be decoded again, if necessary.
  av_seek_frame(this->data->formatCtx, this->data->audioStream, 0, 0);

  return result;
}
#else
bool AudioDecoder::Decode(uint8_t ** /*_outBuffer*/,
    unsigned int * /*_outBufferSize*/)
{
  return true;
}
#endif

/////////////////////////////////////////////////
int AudioDecoder::GetSampleRate()
{
#ifdef HAVE_FFMPEG
  return this->data->codecCtx->sample_rate;
#else
  return 0;
#endif
}

/////////////////////////////////////////////////
#ifdef HAVE_FFMPEG
bool AudioDecoder::SetFile(const std::string &_filename)
{
  unsigned int i;

  this->data->formatCtx = avformat_alloc_context();

  // Open file
  if (avformat_open_input(&this->data->formatCtx,
        _filename.c_str(), NULL, NULL) < 0)
  {
    ignerr << "Unable to open audio file[" << _filename << "]\n";
    this->data->formatCtx = NULL;
    return false;
  }

  // Hide av logging
  av_log_set_level(0);

  // Retrieve some information
  if (avformat_find_stream_info(this->data->formatCtx, NULL) < 0)
  {
    ignerr << "Unable to find stream info.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = NULL;

    return false;
  }

  // Dump information about file onto standard error.
  // dump_format(this->data->formatCtx, 0, "dump.txt", false);

  // Find audio stream;
  this->data->audioStream = -1;
  for (i = 0; i < this->data->formatCtx->nb_streams; ++i)
  {
    if (this->data->formatCtx->streams[i]->codec->codec_type ==
        AVMEDIA_TYPE_AUDIO)
    {
      this->data->audioStream = i;
      break;
    }
  }

  if (this->data->audioStream == -1)
  {
    ignerr << "Couldn't find audio stream.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = NULL;

    return false;
  }

  // Get the audio stream codec
  this->data->codecCtx = this->data->formatCtx->streams[audioStream]->codec;

  // Find a decoder
  this->data->codec = avcodec_find_decoder(codecCtx->codec_id);

  if (this->data->codec == NULL)
  {
    ignerr << "Couldn't find codec for audio stream.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = NULL;

    return false;
  }

  if (this->data->codec->capabilities & CODEC_CAP_TRUNCATED)
    this->data->codecCtx->flags |= CODEC_FLAG_TRUNCATED;

  // Open codec
  if (avcodec_open2(this->data->codecCtx, this->data->codec, NULL) < 0)
  {
    ignerr << "Couldn't open audio codec.\n";
    avformat_close_input(&this->data->formatCtx);
    this->data->formatCtx = NULL;

    return false;
  }

  this->data->filename = _filename;

  return true;
}
#else
bool AudioDecoder::SetFile(const std::string & /*_filename*/)
{
  return false;
}
#endif

/////////////////////////////////////////////////
std::string AudioDecoder::GetFile() const
{
  return this->data->filename;
}
