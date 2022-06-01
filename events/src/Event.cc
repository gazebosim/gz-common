/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "gz/common/Console.hh"
#include "gz/common/Event.hh"

using namespace gz;
using namespace common;

//////////////////////////////////////////////////
Event::Event()
  : signaled(false)
{
}

//////////////////////////////////////////////////
Event::~Event()
{
  // Clear the Event pointer on all connections so that they are not
  // accessed after this Event is destructed.
  for (auto &conn : this->connections)
  {
    auto publicCon = conn.second->publicConnection.lock();
    if (publicCon)
    {
      publicCon->event = nullptr;
    }
  }
  this->connections.clear();
}

//////////////////////////////////////////////////
bool Event::Signaled() const
{
  return this->signaled;
}

//////////////////////////////////////////////////
void Event::SetSignaled(bool _sig)
{
  this->signaled = _sig;
}

//////////////////////////////////////////////////
void Event::Disconnect(int _id) {
  // Find the connection
  auto const &it = this->connections.find(_id);

  if (it != this->connections.end()) {
    it->second->on = false;
    // The destructor of std::function seems to crashes if the function it
    // points to is in a shared library and has been unloaded by the time
    // the destructor is invoked. It's not clear whether this is a bug in
    // the implementation of std::function or not. To avoid the crash,
    // we call the destructor here by setting `callback = nullptr` because
    // it is likely that EventT::Disconnect is called before the shared
    // library is unloaded via Connection::~Connection.
    // it->second->callback = nullptr;
    this->connectionsToRemove.push_back(it);
  }
}

/////////////////////////////////////////////
void Event::Cleanup()
{
  std::lock_guard<std::mutex> lock(this->mutex);
  // Remove all queue connections.
  for (auto &conn : this->connectionsToRemove)
    this->connections.erase(conn);
  this->connectionsToRemove.clear();
}

//////////////////////////////////////////////////
Connection::Connection(Event *_e, int _i)
: event(_e), id(_i)
{
  this->creationTime = IGN_SYSTEM_TIME();
}

//////////////////////////////////////////////////
Connection::~Connection()
{
  auto diffTime = IGN_SYSTEM_TIME() - this->creationTime;
  if ((this->event && !this->event->Signaled()) &&
      diffTime < std::chrono::nanoseconds(10000))
  {
    gzwarn << "Warning: Deleting a connection right after creation. "
          << "Make sure to save the ConnectionPtr from a Connect call\n";
  }

  if (this->event && this->id >= 0)
  {
    this->event->Disconnect(this->id);
    this->id = -1;
    this->event = nullptr;
  }
}

//////////////////////////////////////////////////
int Connection::Id() const
{
  return this->id;
}

