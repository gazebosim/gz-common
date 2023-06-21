/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include "ImageLoader.hh"

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Util.hh>

namespace gz::common
{

ImageLoader::~ImageLoader() = default;

std::shared_ptr<ImageData> ImageLoader::Load(const std::string &_filename) const
{
  std::string fullName = _filename;
  if (!gz::common::exists(fullName))
  {
    fullName = gz::common::findFile(fullName);
  }

  if (!gz::common::exists(fullName))
  {
    gzerr << "Unable to open image file[" << fullName
          << "], check your GZ_RESOURCE_PATH settings.\n";
    return {nullptr};
  }

  return this->LoadImpl(fullName);
}


}  //  namespace gz::common
