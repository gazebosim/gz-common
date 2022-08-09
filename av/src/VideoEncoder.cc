/*
 * Copyright 2016 Open Source Robotics Foundation
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
#include <stdio.h>

#include <mutex>

#include <ignition/common/av/Util.hh>
#include "ignition/common/ffmpeg_inc.hh"
#include "ignition/common/Console.hh"
#include "ignition/common/VideoEncoder.hh"
#include "ignition/common/StringUtils.hh"

#ifdef IGN_COMMON_BUILD_HW_VIDEO
#include "ignition/common/HWEncoder.hh"
#endif

using namespace ignition;
using namespace common;
using namespace std;

// Private data class
// hidden visibility specifier has to be explicitly set to silent a gcc warning
class IGNITION_COMMON_AV_HIDDEN common::VideoEncoderPrivate
{
  /// \brief Name of the file which stores the video while it is being
  ///        recorded.
  public: std::string filename;

  /// \brief libav audio video stream
  public: AVStream *videoStream = nullptr;

  /// \brief libav codec  context
  public: AVCodecContext *codecCtx = nullptr;

  /// \brief libav format I/O context
  public: AVFormatContext *formatCtx = nullptr;

  /// \brief libav output video frame (aligned to 32 bytes)
  public: AVFrame *avOutFrame = nullptr;

  /// \brief libav input image data (aligned to 32 bytes)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
  public: AVPicture *avInFrame = nullptr;
#else
  public: AVFrame *avInFrame = nullptr;
#endif

  /// \brief Pixel format of the input frame. So far it is hardcoded.
  public: AVPixelFormat inPixFormat = AV_PIX_FMT_RGB24;

  /// \brief Software scaling context
  public: SwsContext *swsCtx = nullptr;

  /// \brief Line sizes of an unaligned input frame
  public: int inputLineSizes[4];

  /// \brief True if the encoder is running
  public: bool encoding = false;

  /// \brief Video encoding bit rate
  public: unsigned int bitRate = VIDEO_ENCODER_BITRATE_DEFAULT;

  /// \brief Input frame width
  public: unsigned int inWidth = 0;

  /// \brief Input frame height
  public: unsigned int inHeight = 0;

  /// \brief Encoding format
  public: std::string format = VIDEO_ENCODER_FORMAT_DEFAULT;

  /// \brief Target framerate.
  public: unsigned int fps = VIDEO_ENCODER_FPS_DEFAULT;

  /// \brief Previous time when the frame is added.
  public: std::chrono::steady_clock::time_point timePrev;

  /// \brief Time when the first frame is added.
  public: std::chrono::steady_clock::time_point timeStart;

  /// \brief Number of frames in the video
  public: uint64_t frameCount = 0;

  /// \brief Mutex for thread safety.
  public: std::mutex mutex;

#ifdef IGN_COMMON_BUILD_HW_VIDEO
  /// \brief The HW encoder configuration (optional).
  public: std::unique_ptr<HWEncoder> hwEncoder = nullptr;
#endif

  /// Find a suitable encoder for the given codec ID.
  /// \param[in] _codecId ID of the codec we seek the encoder for.
  /// \return The matched encoder (or nullptr on failure).
  public: const AVCodec* FindEncoder(AVCodecID _codecId);

  /// \brief Get a pointer to the frame that contains the encoder input. This
  /// mainly serves for uploading the frame to GPU buffer if HW acceleration is
  /// used and HW surfaces are configured.
  /// \param[in] _inFrame The input frame to be encoded (in CPU buffer).
  /// \return Either _inFrame of a pointer to the GPU buffer containing the
  /// uploaded frame.
  public: AVFrame* GetFrameForEncoder(AVFrame* _inFrame);

  /// \brief Process an encoder packet and write it to disk.
  /// \param avPacket The packet to process.
  /// \return Non-negative on success, negative on error.
  int ProcessPacket(AVPacket* avPacket);
};

/////////////////////////////////////////////////
const AVCodec* VideoEncoderPrivate::FindEncoder(AVCodecID _codecId)
{
#ifdef IGN_COMMON_BUILD_HW_VIDEO
  if (this->hwEncoder)
    return this->hwEncoder->FindEncoder(_codecId);
#endif
  return avcodec_find_encoder(_codecId);
}

/////////////////////////////////////////////////
AVFrame* VideoEncoderPrivate::GetFrameForEncoder(AVFrame* _inFrame)
{
#ifdef IGN_COMMON_BUILD_HW_VIDEO
  if (this->hwEncoder)
    return this->hwEncoder->GetFrameForEncoder(_inFrame);
#endif
  return _inFrame;
}

/////////////////////////////////////////////////
VideoEncoder::VideoEncoder()
: dataPtr(new VideoEncoderPrivate)
{
  // Make sure libav is loaded.
  load();
}

/////////////////////////////////////////////////
VideoEncoder::~VideoEncoder()
{
  this->Reset();
}

/////////////////////////////////////////////////
std::string VideoEncoder::Format() const
{
  return this->dataPtr->format;
}

/////////////////////////////////////////////////
unsigned int VideoEncoder::BitRate() const
{
  return this->dataPtr->bitRate;
}

/////////////////////////////////////////////////
bool VideoEncoder::Start(const std::string &_format,
                         const std::string &_filename,
                         const unsigned int _width,
                         const unsigned int _height,
                         const unsigned int _fps,
                         const unsigned int _bitRate)
{
  return this->Start(_format, _filename, _width, _height, _fps, _bitRate,
                     true);
}

/////////////////////////////////////////////////
bool VideoEncoder::Start(const std::string &_format,
                         const std::string &_filename,
                         const unsigned int _width,
                         const unsigned int _height,
                         const unsigned int _fps,
                         const unsigned int _bitRate,
                         const bool _allowHwAccel)
{
  FlagSet<HWEncoderType> allowedEncoders = HWEncoderType::NONE;
  std::string device;
  std::optional<bool> useHwSurface;

  if (_allowHwAccel)
  {
    std::string allowedEncodersStr;
    env("IGN_VIDEO_ALLOWED_ENCODERS", allowedEncodersStr);

    if (allowedEncodersStr == "ALL")
    {
      allowedEncoders = FlagSet<HWEncoderType>::AllSet();
    }
    else if (!allowedEncodersStr.empty() && allowedEncodersStr != "NONE")
    {
      for (const auto& encoderStr : Split(allowedEncodersStr, ':'))
      {
        HWEncoderType encoder = HWEncoderType::NONE;
        HWEncoderTypeParser.Set(encoder, encoderStr);
        if (encoder != HWEncoderType::NONE)
          allowedEncoders |= encoder;
      }
    }

#ifndef IGN_COMMON_BUILD_HW_VIDEO
    if (allowedEncoders != HWEncoderType::NONE)
    {
      ignwarn << "Hardware encoding with encoders " << allowedEncodersStr
              << " was requested, but ignition-common is built without HW "
              << "encoding support. A software encoder will be used instead."
              << std::endl;
    }
#endif

    env("IGN_VIDEO_ENCODER_DEVICE", device);

    std::string hwSurfaceStr;
    env("IGN_VIDEO_USE_HW_SURFACE", hwSurfaceStr);

    if (!hwSurfaceStr.empty())
    {
      if (hwSurfaceStr == "0")
        useHwSurface = false;
      else
        useHwSurface = true;
    }
  }

  return this->Start(_format, _filename, _width, _height, _fps, _bitRate,
                     allowedEncoders, device, useHwSurface);
}

/////////////////////////////////////////////////
bool VideoEncoder::Start(
  const std::string &_format, const std::string &_filename,
  const unsigned int _width, const unsigned int _height,
  const unsigned int _fps, const unsigned int _bitRate,
  [[maybe_unused]] const FlagSet<HWEncoderType>& _allowedHwAccel,
  [[maybe_unused]] const std::string& _hwAccelDevice,
  [[maybe_unused]] std::optional<bool> _useHwSurface)
{
  // Do not allow Start to be called more than once without Stop or Reset
  // being called first.
  if (this->dataPtr->encoding)
    return false;

  // This will be true if Stop has been called, but not reset. We will reset
  // automatically to prevent any errors.
  if (this->dataPtr->formatCtx || this->dataPtr->avInFrame ||
      this->dataPtr->avOutFrame || this->dataPtr->swsCtx ||
      this->dataPtr->frameCount > 0u)
  {
    this->Reset();
  }

  // Remove old temp file, if it exists.
  if (exists(this->dataPtr->filename))
  {
    auto success = removeFile(this->dataPtr->filename.c_str());
    if (!success)
    {
      ignerr << "Failed to remove temp file [" << this->dataPtr->filename
             << "]" << std::endl;
    }
  }

  // Calculate a good bitrate if the _bitRate argument is zero
  if (_bitRate == 0)
  {
    // 240p
    if (_width * _height <= 424*240)
      this->dataPtr->bitRate = 100000;
    // 360p
    else if (_width * _height <= 640*360)
      this->dataPtr->bitRate = 230000;
    // 432p
    else if (_width * _height <= 768*432)
      this->dataPtr->bitRate = 330000;
    // 480p (SD or NTSC widescreen)
    else if (_width * _height <= 848*480)
      this->dataPtr->bitRate = 410000;
    // 576p (PAL widescreen)
    else if (_width * _height <= 1024*576)
      this->dataPtr->bitRate = 590000;
    // 720p (HD)
    else if (_width * _height <= 1280*720)
      this->dataPtr->bitRate = 920000;
    // >720P(Full HD)
    else
      this->dataPtr->bitRate = 2070000;
  }
  else
  {
    this->dataPtr->bitRate = _bitRate;
  }

  // Store some info and reset the frame count.
  this->dataPtr->format = _format.compare("v4l") == 0 ? "v4l2" : _format;
  this->dataPtr->fps = _fps;
  this->dataPtr->frameCount = 0;
  this->dataPtr->filename = _filename;

  // Create a default filename if the provided filename is empty.
  if (this->dataPtr->filename.empty())
  {
    if (this->dataPtr->format.compare("v4l2") == 0)
    {
      ignerr << "A video4linux loopback device filename must be specified on "
        << "Start\n";
      this->Reset();
      return false;
    }
    else
    {
      this->dataPtr->filename = joinPaths(cwd(), "TMP_RECORDING." +
                                this->dataPtr->format);
    }
  }

  // The remainder of this function handles FFMPEG initialization of a video
  // stream

  // This 'if' and 'free' are just for safety. We chech the value of formatCtx
  // below.
  if (this->dataPtr->formatCtx)
    avformat_free_context(this->dataPtr->formatCtx);
  this->dataPtr->formatCtx = nullptr;

  // Special case for video4linux2. Here we attempt to find the v4l2 device
  if (this->dataPtr->format.compare("v4l2") == 0)
  {
#if LIBAVDEVICE_VERSION_INT >= AV_VERSION_INT(56, 4, 100)
    AVOutputFormat *outputFormat = nullptr;
    while ((outputFormat = av_output_video_device_next(outputFormat))
           != nullptr)
    {
      // Break when the output device name matches 'v4l2'
      if (this->dataPtr->format.compare(outputFormat->name) == 0)
      {
        // Allocate the context using the correct outputFormat
        auto result = avformat_alloc_output_context2(&this->dataPtr->formatCtx,
            outputFormat, nullptr, this->dataPtr->filename.c_str());
        if (result < 0)
        {
          ignerr << "Failed to allocate AV context [" << av_err2str_cpp(result)
                 << "]" << std::endl;
        }
        break;
      }
    }
#else
    ignerr << "libavdevice version >= 56.4.100 is required for v4l2 recording. "
          << "This version is available on Ubuntu Xenial or greater.\n";
    return false;
#endif
  }
  else
  {
    const AVOutputFormat *outputFormat = av_guess_format(nullptr,
                                   this->dataPtr->filename.c_str(), nullptr);

    if (!outputFormat)
    {
      ignwarn << "Could not deduce output format from file extension."
        << "Using MPEG.\n";
    }

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(56, 40, 1)
        this->dataPtr->formatCtx = avformat_alloc_context();
        if (outputFormat)
        {
          this->dataPtr->formatCtx->oformat = outputFormat;
        }
        else
        {
          this->dataPtr->formatCtx->oformat =
            av_guess_format("mpeg", nullptr, nullptr);
        }
#ifdef WIN32
        _sprintf(this->dataPtr->formatCtx->filename,
                 sizeof(this->dataPtr->formatCtx->filename),
                 "%s", _filename.c_str());
#else
        snprintf(this->dataPtr->formatCtx->filename,
                sizeof(this->dataPtr->formatCtx->filename),
                "%s", _filename.c_str());
#endif

#else
    auto result = avformat_alloc_output_context2(&this->dataPtr->formatCtx,
        nullptr, nullptr, this->dataPtr->filename.c_str());
    if (result < 0)
    {
      ignerr << "Failed to allocate AV context [" << av_err2str_cpp(result)
             << "]" << std::endl;
    }
#endif
  }

  // Make sure allocation occurred.
  if (!this->dataPtr->formatCtx)
  {
    ignerr << "Unable to allocate format context. Video encoding not started\n";
    this->Reset();
    return false;
  }

#ifdef IGN_COMMON_BUILD_HW_VIDEO
  // HW encoder needs to be created before the call to FindEncoder()
  this->dataPtr->hwEncoder = std::make_unique<HWEncoder>(
      _allowedHwAccel, _hwAccelDevice, _useHwSurface);
#endif

  // find the video encoder
  const auto codecId = this->dataPtr->formatCtx->oformat->video_codec;
  auto* encoder = this->dataPtr->FindEncoder(codecId);
  if (!encoder)
  {
    ignerr << "Codec for["
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
          << this->dataPtr->formatCtx->oformat->name
#else
          << avcodec_get_name(codecId)
#endif
          << "] not found. Video encoding is not started.\n";
    this->Reset();
    return false;
  }

  ignmsg << "Using encoder " << encoder->name << std::endl;

  // Create a new video stream
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
  this->dataPtr->videoStream = avformat_new_stream(this->dataPtr->formatCtx,
    encoder);
#else
  this->dataPtr->videoStream = avformat_new_stream(this->dataPtr->formatCtx,
      nullptr);
#endif

  if (!this->dataPtr->videoStream)
  {
    ignerr << "Could not allocate stream. Video encoding is not started\n";
    this->Reset();
    return false;
  }
  this->dataPtr->videoStream->id = this->dataPtr->formatCtx->nb_streams-1;

  // Allocate a new video context
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
  this->dataPtr->codecCtx = this->dataPtr->videoStream->codec;
#else
  this->dataPtr->codecCtx = avcodec_alloc_context3(encoder);
#endif

  if (!this->dataPtr->codecCtx)
  {
    ignerr << "Could not allocate an encoding context."
          << "Video encoding is not started\n";
    this->Reset();
    return false;
  }

  // some formats want stream headers to be separate
  if (this->dataPtr->formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
  {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
    this->dataPtr->codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
#else
    this->dataPtr->codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
#endif
  }

  // Frames per second
  this->dataPtr->codecCtx->time_base.den = this->dataPtr->fps;
  this->dataPtr->codecCtx->time_base.num = 1;

  // The video stream must have the same time base as the context
  this->dataPtr->videoStream->time_base.den = this->dataPtr->fps;
  this->dataPtr->videoStream->time_base.num = 1;

  // Bitrate
  this->dataPtr->codecCtx->bit_rate = this->dataPtr->bitRate;

  // The resolution must be divisible by two
  this->dataPtr->codecCtx->width = _width % 2 == 0 ? _width : _width + 1;
  this->dataPtr->codecCtx->height = _height % 2 == 0 ? _height : _height + 1;

  // Emit one intra-frame every 10 frames
  this->dataPtr->codecCtx->gop_size = 10;
  this->dataPtr->codecCtx->max_b_frames = 1;
  this->dataPtr->codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
  this->dataPtr->codecCtx->thread_count = 5;

  // Set the codec id
  this->dataPtr->codecCtx->codec_id = codecId;

  if (this->dataPtr->codecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
  {
    // Needed to avoid using macroblocks in which some coeffs overflow.
    // This does not happen with normal video, it just happens here as
    // the motion of the chroma plane does not match the luma plane.
    this->dataPtr->codecCtx->mb_decision = 2;
  }

  if (this->dataPtr->codecCtx->codec_id == AV_CODEC_ID_H264)
  {
    av_opt_set(this->dataPtr->codecCtx->priv_data, "preset", "slow", 0);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
    av_opt_set(this->dataPtr->videoStream->codec->priv_data,
        "preset", "slow", 0);
#else
    av_opt_set(this->dataPtr->videoStream->priv_data, "preset", "slow", 0);
#endif
  }

  // we misuse this field a bit, as docs say it is unused in encoders
  // here, it stores the input format of the encoder
  this->dataPtr->codecCtx->sw_pix_fmt = this->dataPtr->codecCtx->pix_fmt;
#ifdef IGN_COMMON_BUILD_HW_VIDEO
  if (this->dataPtr->hwEncoder)
    this->dataPtr->hwEncoder->ConfigHWAccel(this->dataPtr->codecCtx);
#endif

  // Open the video context
  int ret = avcodec_open2(this->dataPtr->codecCtx, encoder, 0);
  if (ret < 0)
  {
    ignerr << "Could not open video codec: " << av_err2str_cpp(ret)
          << ". Video encoding is not started\n";
#ifdef IGN_COMMON_BUILD_HW_VIDEO
    if (AVUNERROR(ret) == ENOMEM &&
      this->dataPtr->hwEncoder->GetEncoderType() == HWEncoderType::NVENC)
    {
      ignwarn << "If this computer has non-server-class GPUs (like GeForce), "
              << "it is possible that you have reached the maximum number of "
              << "simultaneous NVENC sessions (most probably 3). This limit is "
              << "not per GPU, but per the whole computer regardless of the "
              << "number of GPUs installed. You can try to circumvent this "
              << "limit by using the unofficial driver patch at "
              << "https://github.com/keylase/nvidia-patch . If you cannot (or "
              << "do not want) install this patch, do not run more than 3 "
              << "HW-accelerated video encoding tasks on this computer "
              << "simultaneously.\n";
    }
#endif
    this->Reset();
    return false;
  }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
  this->dataPtr->avOutFrame = avcodec_alloc_frame();
#else
  this->dataPtr->avOutFrame = av_frame_alloc();
#endif

  if (!this->dataPtr->avOutFrame)
  {
    ignerr << "Could not allocate video frame. Video encoding is not started\n";
    this->Reset();
    return false;
  }

  // we misuse sw_pix_fmt a bit, as docs say it is unused in encoders
  this->dataPtr->avOutFrame->format = this->dataPtr->codecCtx->sw_pix_fmt;
  this->dataPtr->avOutFrame->width = this->dataPtr->codecCtx->width;
  this->dataPtr->avOutFrame->height = this->dataPtr->codecCtx->height;

  // av_image_alloc() could also allocate the image, but av_frame_get_buffer()
  // allocates a refcounted buffer, which is easier to manage
  if (av_frame_get_buffer(this->dataPtr->avOutFrame, 32) > 0)
  {
    ignerr << "Could not allocate raw picture buffer. "
           << "Video encoding is not started\n";
    this->Reset();
    return false;
  }

  // Copy parameters from the context to the video stream
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 40, 101)
//  ret = avcodec_copy_context(this->dataPtr->videoStream->codec,
//                       this->dataPtr->codecCtx);
#else
  // codecpar was implemented in ffmpeg version 3.1
  ret = avcodec_parameters_from_context(
      this->dataPtr->videoStream->codecpar, this->dataPtr->codecCtx);
#endif
  if (ret < 0)
  {
    ignerr << "Could not copy the stream parameters:" << av_err2str_cpp(ret)
          << ". Video encoding not started\n";
    return false;
  }

  // setting mux preload and max delay avoids buffer underflow when writing to
  // mpeg format
  double muxMaxDelay = 0.7f;
  this->dataPtr->formatCtx->max_delay =
    static_cast<int>(muxMaxDelay * AV_TIME_BASE);

  // Open the video stream
  if (!(this->dataPtr->formatCtx->oformat->flags & AVFMT_NOFILE))
  {
    ret = avio_open(&this->dataPtr->formatCtx->pb,
        this->dataPtr->filename.c_str(), AVIO_FLAG_WRITE);

    if (ret < 0)
    {
      ignerr << "Could not open '" << this->dataPtr->filename << "'. "
            << av_err2str_cpp(ret) << ". Video encoding is not started\n";
      this->Reset();
      return false;
    }
  }

  // Write the stream header, if any.
  ret = avformat_write_header(this->dataPtr->formatCtx, nullptr);
  if (ret < 0)
  {
    ignerr << "Error occured when opening output file: " << av_err2str_cpp(ret)
          << ". Video encoding is not started\n";
    this->Reset();
    return false;
  }

  this->dataPtr->encoding = true;
  return true;
}

////////////////////////////////////////////////
bool VideoEncoder::IsEncoding() const
{
  return this->dataPtr->encoding;
}

/////////////////////////////////////////////////
bool VideoEncoder::AddFrame(const unsigned char *_frame,
                            const unsigned int _width,
                            const unsigned int _height)
{
  return this->AddFrame(_frame, _width, _height,
                        std::chrono::steady_clock::now());
}

/////////////////////////////////////////////////
// This function supports ffmpeg2
bool VideoEncoder::AddFrame(const unsigned char *_frame,
    const unsigned int _width,
    const unsigned int _height,
    const std::chrono::steady_clock::time_point &_timestamp)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  if (!this->dataPtr->encoding)
  {
    ignerr << "Start encoding before adding a frame\n";
    return false;
  }

  auto dt = _timestamp - this->dataPtr->timePrev;

  // Skip frames that arrive faster than the video's fps
  double period = 1.0/this->dataPtr->fps;
  if (this->dataPtr->frameCount > 0u &&
      dt < std::chrono::duration<double>(period))
    return false;

  if (this->dataPtr->frameCount == 0u)
    this->dataPtr->timeStart = _timestamp;

  this->dataPtr->timePrev = _timestamp;

  // Cause the sws to be recreated on image resize
  if (this->dataPtr->swsCtx &&
      (this->dataPtr->inWidth != _width || this->dataPtr->inHeight != _height))
  {
    sws_freeContext(this->dataPtr->swsCtx);
    this->dataPtr->swsCtx = nullptr;

    if (this->dataPtr->avInFrame)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
      av_free(this->dataPtr->avInFrame);
#else
      av_frame_free(&this->dataPtr->avInFrame);
#endif
    this->dataPtr->avInFrame = nullptr;
  }

  if (!this->dataPtr->swsCtx)
  {
    this->dataPtr->inWidth = _width;
    this->dataPtr->inHeight = _height;

    if (!this->dataPtr->avInFrame)
    {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
      this->dataPtr->avInFrame = new AVPicture;
      avpicture_alloc(this->dataPtr->avInFrame,
          this->dataPtr->inPixFormat, this->dataPtr->inWidth,
          this->dataPtr->inHeight);
#else
      this->dataPtr->avInFrame = av_frame_alloc();
      this->dataPtr->avInFrame->width = this->dataPtr->inWidth;
      this->dataPtr->avInFrame->height = this->dataPtr->inHeight;
      this->dataPtr->avInFrame->format = this->dataPtr->inPixFormat;

      av_frame_get_buffer(this->dataPtr->avInFrame, 32);
#endif
    }

    av_image_fill_linesizes(this->dataPtr->inputLineSizes,
                            this->dataPtr->inPixFormat,
                            this->dataPtr->inWidth);

    this->dataPtr->swsCtx = sws_getContext(
        this->dataPtr->inWidth,
        this->dataPtr->inHeight,
        this->dataPtr->inPixFormat,
        this->dataPtr->codecCtx->width,
        this->dataPtr->codecCtx->height,
        // we misuse this field a bit, as docs say it is unused in encoders
        this->dataPtr->codecCtx->sw_pix_fmt,
        0, nullptr, nullptr, nullptr);

    if (this->dataPtr->swsCtx == nullptr)
    {
      ignerr << "Error while calling sws_getContext\n";
      return false;
    }
  }

  // encode

  // copy the unaligned input buffer to the 32-byte-aligned avInFrame
  av_image_copy(
      this->dataPtr->avInFrame->data, this->dataPtr->avInFrame->linesize,
      &_frame, this->dataPtr->inputLineSizes,
      this->dataPtr->inPixFormat,
      this->dataPtr->inWidth, this->dataPtr->inHeight);

  sws_scale(this->dataPtr->swsCtx,
      this->dataPtr->avInFrame->data,
      this->dataPtr->avInFrame->linesize,
      0, this->dataPtr->inHeight,
      this->dataPtr->avOutFrame->data,
      this->dataPtr->avOutFrame->linesize);

  auto* frameToEncode = this->dataPtr->GetFrameForEncoder(
    this->dataPtr->avOutFrame);

  // compute frame number based on timestamp of current image
  auto timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(
      _timestamp - this->dataPtr->timeStart);
  double durationSec = timeSinceStart.count() / 1000.0;
  uint64_t frameNumber = static_cast<uint64_t>(durationSec / period);

  uint64_t frameDiff = frameNumber + 1 - this->dataPtr->frameCount;

  int ret = 0;

  // make sure we have continuous pts (frame number) otherwise some decoders
  // may not be happy. So encode more (duplicate) frames until the current frame
  // number
  for (uint64_t i = 0u;
       i < frameDiff && (ret >= 0 || ret == AVERROR(EAGAIN));
       ++i)
  {
    frameToEncode->pts = this->dataPtr->frameCount++;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 40, 101)
    int gotOutput = 0;
    AVPacket avPacket;
    av_init_packet(&avPacket);
    avPacket.data = nullptr;
    avPacket.size = 0;

    ret = avcodec_encode_video2(this->dataPtr->codecCtx, &avPacket,
        frameToEncode, &gotOutput);

    if (ret >= 0 && gotOutput == 1)
      ret = ProcessPacket(&avPacket);

    av_free_packet(&avPacket);

    // #else for libavcodec version check
#else

    AVPacket* avPacket = av_packet_alloc();

    avPacket->data = nullptr;
    avPacket->size = 0;

    ret = avcodec_send_frame(this->dataPtr->codecCtx,
                                 frameToEncode);

    // This loop will retrieve and write available packets
    while (ret >= 0)
    {
      ret = avcodec_receive_packet(this->dataPtr->codecCtx, avPacket);

      // Potential performance improvement: Queue the packets and write in
      // a separate thread.
      if (ret >= 0)
        ret = this->dataPtr->ProcessPacket(avPacket);
    }

    av_packet_unref(avPacket);
#endif
  }
  return ret >= 0 || ret == AVERROR(EAGAIN);
}

/////////////////////////////////////////////////
int VideoEncoderPrivate::ProcessPacket(AVPacket* avPacket)
{
  avPacket->stream_index = this->videoStream->index;

  // Scale timestamp appropriately.
  if (avPacket->pts != static_cast<int64_t>(AV_NOPTS_VALUE))
  {
    avPacket->pts = av_rescale_q(
      avPacket->pts,
      this->codecCtx->time_base,
      this->videoStream->time_base);
  }

  if (avPacket->dts != static_cast<int64_t>(AV_NOPTS_VALUE))
  {
    avPacket->dts = av_rescale_q(
      avPacket->dts,
      this->codecCtx->time_base,
      this->videoStream->time_base);
  }

  // Write frame to disk
  int ret = av_interleaved_write_frame(this->formatCtx, avPacket);

  if (ret < 0)
    ignerr << "Error writing frame: " << av_err2str_cpp(ret) << std::endl;

  return ret;
}

/////////////////////////////////////////////////
bool VideoEncoder::Stop()
{
  // drain remaining packets from the encoder
  if (this->dataPtr->encoding && this->dataPtr->codecCtx)
  {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 40, 101)
    if ((this->dataPtr->codecCtx->capabilities & AV_CODEC_CAP_DELAY) > 0)
    {
      int gotOutput = 1;
      int ret = 0;
      AVPacket avPacket;
      av_init_packet(&avPacket);
      avPacket.data = nullptr;
      avPacket.size = 0;

      while (ret >= 0 && gotOutput == 1)
      {
        ret = avcodec_encode_video2(this->dataPtr->codecCtx, &avPacket,
            nullptr, &gotOutput);

        if (ret >= 0 && gotOutput == 1)
          ret = ProcessPacket(&avPacket);
      }

      av_free_packet(&avPacket);
  }

// #else for libavcodec version check
#else

    int ret = 0;
    // enter drain state
    ret = avcodec_send_frame(this->dataPtr->codecCtx, nullptr);

    if (ret >= 0)
    {
      AVPacket *avPacket = av_packet_alloc();
      avPacket->data = nullptr;
      avPacket->size = 0;

      // This loop will retrieve and write all remaining packets
      while (ret >= 0)
      {
        ret = avcodec_receive_packet(this->dataPtr->codecCtx, avPacket);

        // Potential performance improvement: Queue the packets and write in
        // a separate thread.
        if (ret >= 0)
          ret = this->dataPtr->ProcessPacket(avPacket);
      }
      av_packet_unref(avPacket);
    }
#endif
  }

  if (this->dataPtr->encoding && this->dataPtr->formatCtx)
    av_write_trailer(this->dataPtr->formatCtx);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 24, 1)
  if (this->dataPtr->codecCtx)
    avcodec_free_context(&this->dataPtr->codecCtx);
#endif
  this->dataPtr->codecCtx = nullptr;

  if (this->dataPtr->avInFrame)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
    av_free(this->dataPtr->avInFrame);
#else
    av_frame_free(&this->dataPtr->avInFrame);
#endif
  this->dataPtr->avInFrame = nullptr;

  if (this->dataPtr->avOutFrame)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 24, 1)
    av_free(this->dataPtr->avOutFrame);
#else
    av_frame_free(&this->dataPtr->avOutFrame);
#endif
  this->dataPtr->avOutFrame = nullptr;

  if (this->dataPtr->swsCtx)
    sws_freeContext(this->dataPtr->swsCtx);
  this->dataPtr->swsCtx = nullptr;

  if (this->dataPtr->formatCtx && this->dataPtr->formatCtx->pb)
    avio_closep(&this->dataPtr->formatCtx->pb);

  // This frees the context and all the streams
  if (this->dataPtr->formatCtx)
    avformat_free_context(this->dataPtr->formatCtx);
  this->dataPtr->formatCtx = nullptr;
  this->dataPtr->videoStream = nullptr;

  this->dataPtr->encoding = false;
  return true;
}

/////////////////////////////////////////////////
bool VideoEncoder::SaveToFile(const std::string &_filename)
{
  // First stop the recording
  this->Stop();

  bool result = true;

  if (this->dataPtr->format != "v4l2")
  {
    result = moveFile(this->dataPtr->filename, _filename);

    if (!result)
    {
      ignerr << "Unable to rename file from[" << this->dataPtr->filename
        << "] to [" << _filename << "]\n";
    }
  }

  this->dataPtr->filename = "";

  this->Reset();

  return result;
}

/////////////////////////////////////////////////
void VideoEncoder::Reset()
{
  // Make sure the video has been stopped.
  this->Stop();

  // Remove old temp file, if it exists.
  if (exists(this->dataPtr->filename))
  {
    auto success = removeFile(this->dataPtr->filename.c_str());
    if (!success)
    {
      ignerr << "Failed to remove temp file [" << this->dataPtr->filename
             << "]" << std::endl;
    }
  }

  // set default values
  this->dataPtr->frameCount = 0;
  this->dataPtr->inWidth = 0;
  this->dataPtr->inHeight = 0;
  this->dataPtr->timePrev = {};
  this->dataPtr->bitRate = VIDEO_ENCODER_BITRATE_DEFAULT;
  this->dataPtr->fps = VIDEO_ENCODER_FPS_DEFAULT;
  this->dataPtr->format = VIDEO_ENCODER_FORMAT_DEFAULT;
  this->dataPtr->timePrev = std::chrono::steady_clock::time_point();
  this->dataPtr->timeStart = std::chrono::steady_clock::time_point();
  this->dataPtr->filename.clear();
#ifdef IGN_COMMON_BUILD_HW_VIDEO
  if (this->dataPtr->hwEncoder)
    this->dataPtr->hwEncoder.reset();
#endif
}
