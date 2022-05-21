/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <gz/common.hh>

int main(int argc, char **argv)
{
  // Default verbosity is 1, only error messages show
  igndbg << "This is a debug message" << std::endl;
  ignmsg << "This is an informational message" << std::endl;
  ignwarn << "This is a warning" << std::endl;
  ignerr << "This is an error" << std::endl;

  // Change verbosity to level 4, all messages show
  gz::common::Console::SetVerbosity(4);
  igndbg << "This is a debug message" << std::endl;
  ignmsg << "This is an informational message" << std::endl;
  ignwarn << "This is a warning" << std::endl;
  ignerr << "This is an error" << std::endl;

  return 0;
}
