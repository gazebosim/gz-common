/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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


#ifndef IGNITION_COMMON_TEST_UTIL_DUMMY_PLUGINS_HH_
#define IGNITION_COMMON_TEST_UTIL_DUMMY_PLUGINS_HH_

#include <string>

#include <ignition/common/System.hh>
#include <ignition/common/PluginMacros.hh>

namespace test
{
namespace util
{

class IGNITION_COMMON_VISIBLE DummyPluginBase
{
  public: virtual std::string MyNameIs() = 0;
};


class DummyPlugin : public DummyPluginBase
{
  public: virtual std::string MyNameIs() override;
};


class IGNITION_COMMON_VISIBLE DummyOtherBase
{
  public: virtual double MyValueIs() = 0;
};

class DummyFooBase
{
  public: virtual int MyIntegerValueIs() = 0;
  IGN_COMMON_SPECIALIZE_INTERFACE(test::util::DummyFooBase)
};

class DummySetterBase
{
  public: virtual void SetName(const std::string &_name) = 0;
  public: virtual void SetValue(const double _val) = 0;
  public: virtual void SetIntegerValue(const int _val) = 0;
  IGN_COMMON_SPECIALIZE_INTERFACE(test::util::DummyFooBase)
};

class DummyMultiPlugin
    : public DummyPluginBase,
      public DummyOtherBase,
      public DummyFooBase,
      public DummySetterBase
{
  public: virtual std::string MyNameIs() override;
  public: virtual double MyValueIs() override;
  public: virtual int MyIntegerValueIs() override;

  public: virtual void SetName(const std::string &_name) override;
  public: virtual void SetValue(const double _val) override;
  public: virtual void SetIntegerValue(const int _val) override;

  public: DummyMultiPlugin();

  private: std::string name;
  private: double val;
  private: int intVal;
};

}
}


#endif
