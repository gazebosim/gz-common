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
// comments when upgrading to ign-cmake's "make codecheck"
#include "ignition/common/SignalHandler.hh" // NOLINT(*)
#include <signal.h> // NOLINT(*)
#include <functional> // NOLINT(*)
#include <map> // NOLINT(*)
#include <vector> // NOLINT(*)
#include "ignition/common/Console.hh" // NOLINT(*)

using namespace ignition;
using namespace common;

// A wrapper for the sigaction sa_handler.
std::map<int, std::function<void(int)>> gOnSignalWrappers;
void onSignal(int _value)
{
  // Send the signal to each wrapper
  for (std::pair<int, std::function<void(int)>> func : gOnSignalWrappers)
  {
    func.second(_value);
  }
}

/////////////////////////////////////////////////
class ignition::common::SignalHandlerPrivate
{
  /// \brief Handle a signal.
  /// \param[in] _sig Signal number
  public: void OnSignal(int _sig);

  /// \brief the callbacks to execute when a signal is received.
  public: std::vector<std::function<void(int)>> callbacks;

  /// \brief True if signal handlers were successfully initialized.
  public: bool initialized = false;

  /// \brief Index in the global gOnSignalWrappers map.
  public: int wrapperIndex = -1;
};

/////////////////////////////////////////////////
SignalHandler::SignalHandler()
  : dataPtr(new SignalHandlerPrivate)
{
  static int counter = 0;

#ifndef _WIN32
  // Only need to setup the system sighandler once.
  if (counter == 0)
  {
    /// \todo(nkoenig) Add a mock interface to test failure of signal
    /// creation.
    struct sigaction sigact;
    sigact.sa_flags = 0;
    sigact.sa_handler = onSignal;
    if (sigemptyset(&sigact.sa_mask) != 0)
      std::cerr << "sigemptyset failed while setting up for SIGINT\n";

    if (sigaction(SIGINT, &sigact, nullptr))
    {
      ignerr << "Unable to catch SIGINT.\n"
        << " Please visit http://community.gazebosim.org for help.\n";
      return;
    }
    if (sigaction(SIGTERM, &sigact, nullptr))
    {
      ignerr << "Unable to catch SIGTERM.\n";
      return;
    }
  }
#endif

  gOnSignalWrappers[counter] = std::bind(&SignalHandlerPrivate::OnSignal,
      this->dataPtr, std::placeholders::_1);
  this->dataPtr->wrapperIndex = counter;

  ++counter;

  this->dataPtr->initialized = true;
}

/////////////////////////////////////////////////
SignalHandler::~SignalHandler()
{
  if (this->dataPtr->wrapperIndex >= 0)
  {
    gOnSignalWrappers.erase(
        gOnSignalWrappers.find(this->dataPtr->wrapperIndex));
  }
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
  bool result = false;
  if (this->dataPtr->initialized)
  {
    this->dataPtr->callbacks.push_back(_cb);
    result = true;
  }
  else
  {
    ignerr << "The SignalHandler was not initialized. Adding a callback will "
      << "have no effect.\n";
  }
  return result;
}

//////////////////////////////////////////////////
void SignalHandlerPrivate::OnSignal(int _sig)
{
  igndbg << "Received signal[" << _sig << "].\n";
  for (std::function<void(int)> cb : this->callbacks)
    cb(_sig);
}
