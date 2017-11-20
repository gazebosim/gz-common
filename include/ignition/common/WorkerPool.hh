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

#ifndef IGNITION_COMMON_WORKER_POOL_HH_
#define IGNITION_COMMON_WORKER_POOL_HH_

#include <functional>
#include <memory>

#include <ignition/common/Export.hh>
#include <ignition/common/Time.hh>
#include <ignition/common/SuppressWarning.hh>

namespace ignition
{
  namespace common
  {
    /// \brief forward declaration
    class WorkerPoolPrivate;

    /// \brief A pool of worker threads that do stuff in parallel
    class IGNITION_COMMON_VISIBLE WorkerPool
    {
      /// \brief creates worker threads
      public: WorkerPool();

      /// \brief closes worker threads
      public: ~WorkerPool();

      /// \brief Adds work to the worker pool with optional callback
      /// \param[in] _work function to do one piece of work
      /// \param[in] _cb optional callback when the work is done
      /// \remark Typical work is a function bound with arguments. It must
      //               return within a finite amount of time.
      public: void AddWork(std::function<void()> _work,
                  std::function<void()> _cb = std::function<void()>());

      /// \brief Waits until all work is done and threads are idle
      /// \param[in] _timeout How long to wait, default to forever
      /// \returns true if all work was finished
      /// \remarks The return value can be false even when waiting forever if
      //           the WorkerPool is destructed before all work is completed
      public: bool WaitForResults(const Time &_timeout = Time::Zero);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief private implementation pointer
      private: std::unique_ptr<WorkerPoolPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#endif
