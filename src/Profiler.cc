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
#include "ignition/common/Profiler.hh" // NOLINT(*)

#include "RemoteryProfilerImpl.hh"

using namespace ignition;
using namespace common;

//////////////////////////////////////////////////
Profiler::Profiler():
  impl(new RemoteryProfilerImpl())
{
}

//////////////////////////////////////////////////
Profiler::~Profiler()
{
  delete this->impl;
  this->impl = nullptr;
}

//////////////////////////////////////////////////
void Profiler::SetThreadName(const char * _name)
{
  this->impl->SetThreadName(_name);
}

//////////////////////////////////////////////////
void Profiler::LogText(const char * _text)
{
  this->impl->LogText(_text);
}

//////////////////////////////////////////////////
void Profiler::BeginSample(const char * _name, uint32_t* _hash)
{
  this->impl->BeginSample(_name, _hash);
}

//////////////////////////////////////////////////
void Profiler::EndSample()
{
  this->impl->EndSample();
}

