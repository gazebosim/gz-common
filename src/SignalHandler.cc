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

// Suppressing cpplint.py because tools/cpplint.py is old. Remove the NOLINT
// comments when upgrading to gz-cmake's "make codecheck"
#include "gz/common/SignalHandler.hh" // NOLINT(*)
#include <atomic>
#include <cstring>
#include <csignal> // NOLINT(*)
#include <functional> // NOLINT(*)
#include <fcntl.h>
#include <unistd.h>
#include <map> // NOLINT(*)
#include <mutex> // NOLINT(*)
#include <utility> // NOLINT(*)
#include <vector> // NOLINT(*)
#include "gz/common/Console.hh" // NOLINT(*)

using namespace gz;
using namespace common;

// A wrapper for the sigaction sa_handler.
// TODO(azeey) We should avoid using objects with non-trivial destructors as globals.
GZ_COMMON_VISIBLE std::map<int, std::function<void(int)>> gOnSignalWrappers;
std::mutex gWrapperMutex;

namespace
{

class SelfPipe
{
  public:
  static int pipeFd[2];

  public:
  static void Initialize();

  public:
  ~SelfPipe();

  private:
  SelfPipe();

  private:
  void CheckPipe();

  private:
  std::thread checkPipeThread;

  private:
  std::atomic<bool> running{false};
};

int SelfPipe::pipeFd[2];

void onSignalTopHalf(int _value)
{
  auto valueByte = static_cast<std::uint8_t>(_value);
  if (write(SelfPipe::pipeFd[1], &valueByte, 1) == -1)
  {
    // TODO (azeey) Not clear what to do here.
  }
}

/////////////////////////////////////////////////
/// \brief Callback to execute when a signal is received.
/// \param[in] _value Signal number.
void onSignalBottomHalf(int _value)
{
  std::lock_guard<std::mutex> lock(gWrapperMutex);
  // Send the signal to each wrapper
  for (std::pair<int, std::function<void(int)>> func : gOnSignalWrappers)
  {
    func.second(_value);
  }
}

SelfPipe::SelfPipe()
{
  if (pipe(this->pipeFd))
  {
    gzerr << "Unable to create pipe\n";
  }

  int flags = fcntl(this->pipeFd[1], F_GETFL);
  fcntl(this->pipeFd[1], F_SETFL, flags | O_NONBLOCK);
  // TODO(azeey) Handle errors
  this->running = true;
  std::cout << "Initialized self pipe " << running << std::endl;
  this->checkPipeThread = std::thread(&SelfPipe::CheckPipe, this);
}

SelfPipe::~SelfPipe()
{
  this->running = false;
  onSignalTopHalf(127);
  this->checkPipeThread.join();
}
void SelfPipe::Initialize()
{
  // We actually need this object to be destructed in order to join the thread,
  // so we can't use gz::utils::NeverDestroyed here.
  static SelfPipe selfPipe;
}

void SelfPipe::CheckPipe()
{
  while (this->running)
  {
    std::uint8_t signal;
    if (read(SelfPipe::pipeFd[0], &signal, 1) != -1)
    {
      if (this->running)
      {
        onSignalBottomHalf(signal);
      }
    }
    else
    {
      gzerr << errno << " " << std::strerror(errno) << std::endl;
    }
  }
}

}  // namespace

/////////////////////////////////////////////////
class common::SignalHandlerPrivate
{
  /// \brief Handle a signal.
  /// \param[in] _sig Signal number
  public: void OnSignal(int _sig);

  /// \brief the callbacks to execute when a signal is received.
  public: std::vector<std::function<void(int)>> callbacks;

  /// \brief Callback mutex.
  public: std::mutex cbMutex;

  /// \brief True if signal handlers were successfully initialized.
  public: std::atomic<bool> initialized = {false};

  /// \brief Index in the global gOnSignalWrappers map.
  public: int wrapperIndex = -1;
};

/////////////////////////////////////////////////
SignalHandler::SignalHandler()
  : dataPtr(new SignalHandlerPrivate)
{
  static int counter = 0;
  std::lock_guard<std::mutex> lock(gWrapperMutex);

  SelfPipe::Initialize();
  if (std::signal(SIGINT, onSignalTopHalf) == SIG_ERR)
  {
    gzerr << "Unable to catch SIGINT.\n"
           << " Please visit http://community.gazebosim.org for help.\n";
    return;
  }

  if (std::signal(SIGTERM, onSignalTopHalf) == SIG_ERR)
  {
    gzerr << "Unable to catch SIGTERM.\n"
           << " Please visit http://community.gazebosim.org for help.\n";
    return;
  }

  gOnSignalWrappers[counter] = std::bind(&SignalHandlerPrivate::OnSignal,
      this->dataPtr, std::placeholders::_1);
  this->dataPtr->wrapperIndex = counter;

  ++counter;

  this->dataPtr->initialized = true;
}

/////////////////////////////////////////////////
SignalHandler::~SignalHandler()
{
  std::lock_guard<std::mutex> lock(gWrapperMutex);
  gOnSignalWrappers.erase(this->dataPtr->wrapperIndex);
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

//////////////////////////////////////////////////
bool SignalHandler::Initialized() const
{
  return this->dataPtr->initialized;
}

//////////////////////////////////////////////////
void SignalHandler::SetInitialized(const bool _init)
{
  this->dataPtr->initialized = _init;
}

/////////////////////////////////////////////////
bool SignalHandler::AddCallback(std::function<void(int)> _cb)
{
  bool result = this->dataPtr->initialized;
  if (result)
  {
    std::lock_guard<std::mutex> lock(this->dataPtr->cbMutex);
    this->dataPtr->callbacks.push_back(_cb);
  }
  else
  {
    gzerr << "The SignalHandler was not initialized. Adding a callback will "
      << "have no effect.\n";
  }
  return result;
}

//////////////////////////////////////////////////
void SignalHandlerPrivate::OnSignal(int _sig)
{
  std::lock_guard<std::mutex> lock(this->cbMutex);
  gzdbg << "Received signal[" << _sig << "].\n";
  for (std::function<void(int)> cb : this->callbacks)
    cb(_sig);
}
