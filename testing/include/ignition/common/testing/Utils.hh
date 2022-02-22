/*
* Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef IGNITION_COMMON_TESTING_UTILS_HH_
#define IGNITION_COMMON_TESTING_UTILS_HH_

#include <climits>
#include <cstdint>
#include <fstream>
#include <random>
#include <string>

namespace ignition::common::testing
{

/// \brief Get a random number based on an integer converted to string.
/// \return A random integer converted to string.
std::string getRandomNumber(int32_t _min = 0, int32_t _max = INT_MAX)
{
  // Initialize random number generator.
  uint32_t seed = std::random_device {}();
  std::mt19937 randGenerator(seed);

  // Create a random number based on an integer converted to string.
  std::uniform_int_distribution<int32_t> d(_min, _max);

  return std::to_string(d(randGenerator));
}

/////////////////////////////////////////////////
bool create_new_empty_file(const std::string &_filename)
{
  try
  {
    std::fstream fs(_filename, std::ios::out);
  }
  catch(...)
  {
    return false;
  }
  return true;
}

}  // namespace ignition::common::testing

#endif  // IGNITION_COMMON_TESTING_TESTPATHS_HH_
