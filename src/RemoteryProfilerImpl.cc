/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "RemoteryProfilerImpl.hh"

#include "ignition/common/Console.hh"
#include "ignition/common/Util.hh"

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
RemoteryProfilerImpl::RemoteryProfilerImpl()
{
  this->settings = rmt_Settings();

  // Always attempt to reuse the port
  this->settings->reuse_open_port = RMT_TRUE;

  std::string port;
  if (env("RMT_PORT", port))
  {
    this->settings->port = std::stoul(port);
  }

  std::string queue_size;
  if (env("RMT_QUEUE_SIZE", queue_size))
  {
    this->settings->messageQueueSizeInBytes = std::stoul(queue_size);
  }
  else
  {
    this->settings->messageQueueSizeInBytes = 2048 * 2048;
  }

  std::string messages_per_update;
  if (env("RMT_MSGS_PER_UPDATE", messages_per_update))
  {
    this->settings->maxNbMessagesPerUpdate = std::stoul(messages_per_update);
  }
  else
  {
    this->settings->maxNbMessagesPerUpdate = 10;
  }

  std::string sleep_between_updates;
  if (env("RMT_SLEEP_BETWEEN_UPDATES", sleep_between_updates))
  {
    this->settings->msSleepBetweenServerUpdates = std::stoul(sleep_between_updates);
  }
  else
  {
    this->settings->msSleepBetweenServerUpdates = 10;
  }

  rmtError error;
  error = rmt_CreateGlobalInstance(&this->rmt);

  if(RMT_ERROR_NONE != error)
  {
    ignerr << "Error launching Remotery: " << error;
  }
}

//////////////////////////////////////////////////
RemoteryProfilerImpl::~RemoteryProfilerImpl()
{
  rmt_DestroyGlobalInstance(rmt);
}

//////////////////////////////////////////////////
void RemoteryProfilerImpl::SetThreadName(const char * _name)
{
  rmt_SetCurrentThreadName(_name);
}

//////////////////////////////////////////////////
void RemoteryProfilerImpl::LogText(const char * _text)
{
  rmt_LogText(_text);
}

//////////////////////////////////////////////////
void RemoteryProfilerImpl::BeginSample(const char * _name, uint32_t* _hash)
{
  _rmt_BeginCPUSample(_name, RMTSF_None, _hash);
}

//////////////////////////////////////////////////
void RemoteryProfilerImpl::EndSample()
{
  rmt_EndCPUSample();
}
