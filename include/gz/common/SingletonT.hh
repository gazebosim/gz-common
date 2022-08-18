/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef IGNITION_COMMON_SINGLETONT_HH_
#define IGNITION_COMMON_SINGLETONT_HH_

namespace ignition
{
  namespace common
  {
    /// \class SingletonT SingletonT.hh common/common.hh
    /// \brief Singleton template class
    template <class T>
    class SingletonT
    {
      /// \brief Get an instance of the singleton
      public: static T *Instance()
      {
        return &GetInstance();
      }

      /// \brief Constructor
      protected: SingletonT() {}

      /// \brief Destructor
      protected: virtual ~SingletonT() {}

      /// \brief Creates and returns a reference to the unique (static) instance
      private: static T &GetInstance()
      {
        static T t;
        return static_cast<T &>(t);
      }

      /// \brief A reference to the unique instance
      private: static T &myself;
    };

    /// \brief Initialization of the singleton instance.
    template <class T>
    T &SingletonT<T>::myself = SingletonT<T>::GetInstance();
  }
}
#endif
