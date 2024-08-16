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
#include <filesystem>
#include <gz/common/Console.hh>

int main(int argc, char **argv)
{
  // Default verbosity is 1, only error messages show
  gzdbg  << "This is a debug message";
  gzmsg  << "This is an informational message";
  gzwarn << "This is a warning";
  gzerr  << "This is an error";
  gzcrit << "This is a critical error";

  // Change verbosity to level 4, all messages show
  // gz::common::ConsoleGlobal::SetVerbosity(4);
  // gz::common::ConsoleGlobal::SetPrefix("My prefix. ");
  gzdbg  << "This is a debug message";
  gzmsg  << "This is an informational message";
  gzwarn << "This is a warning";
  gzerr  << "This is an error";
  gzcrit << "This is a critical error";

  std::filesystem::path logDir = std::filesystem::temp_directory_path();
  std::filesystem::path logFile = "my_log.txt";

  gz::common::Console c("gz_tmp");
  c.SetLogDestination(logDir / "tmp2" / logFile);
  auto logger = c.RawLogger();
  logger.log(spdlog::level::err, "Hello");

  gz::common::Console::Init(logDir / "tmp3", logFile);
  gzerr << "Error 1";
  gzerr << "Error 2";
  gzerr << "Directory: " << gz::common::Console::Directory();
  gz::common::Console::Close();
  gzerr << "Error 3";

  return 0;
}
