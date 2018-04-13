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

#include "ignition/common/SignalHandler.hh"
#include <gtest/gtest.h>
#include <signal.h>
#include <condition_variable>
#include <mutex>
#include "ignition/common/Util.hh"

using namespace ignition;

int gHandler1Sig = -1;
int gHandler2Sig = -1;

/////////////////////////////////////////////////
void handler1Cb(int _sig)
{
  gHandler1Sig = _sig;
}

/////////////////////////////////////////////////
void handler2Cb(int _sig)
{
  gHandler2Sig = _sig;
}

/////////////////////////////////////////////////
TEST(SignalHandler, Single)
{
  gHandler1Sig = -1;
  gHandler2Sig = -1;

  common::SignalHandler handler1;
  EXPECT_TRUE(handler1.AddCallback(handler1Cb));
  raise(SIGTERM);
  EXPECT_EQ(SIGTERM, gHandler1Sig);
}

/////////////////////////////////////////////////
TEST(SignalHandler, Multiple)
{
  gHandler1Sig = -1;
  gHandler2Sig = -1;

  common::SignalHandler handler1;
  common::SignalHandler handler2;

  EXPECT_TRUE(handler1.Initialized());
  EXPECT_TRUE(handler2.Initialized());

  EXPECT_TRUE(handler1.AddCallback(handler1Cb));
  EXPECT_TRUE(handler2.AddCallback(handler2Cb));

  raise(SIGINT);

  EXPECT_EQ(SIGINT, gHandler1Sig);
  EXPECT_EQ(SIGINT, gHandler2Sig);
}


/////////////////////////////////////////////////
TEST(SignalHandler, InitFailure)
{
  gHandler1Sig = -1;
  gHandler2Sig = -1;

  class NotInitializedSignalHandler : public common::SignalHandler
  {
    /// \brief Constructor
    public: NotInitializedSignalHandler()
            : common::SignalHandler()
    {
      this->SetInitialized(false);
    }
  };

  NotInitializedSignalHandler badSigHandler;
  EXPECT_FALSE(badSigHandler.Initialized());
  EXPECT_FALSE(badSigHandler.AddCallback(handler1Cb));

  raise(SIGINT);

  EXPECT_EQ(-1, gHandler1Sig);
  EXPECT_EQ(-1, gHandler2Sig);
}

/////////////////////////////////////////////////
TEST(SignalHandler, Thread)
{
  gHandler1Sig = -1;
  gHandler2Sig = -1;

  std::mutex mutex;
  std::condition_variable cv;

  // Create a lock, which will allows us to wait for the thread to create
  // its signal handler
  std::unique_lock<std::mutex> outerLock(mutex);

  // Create the thread. Inside the thread, a new signal handler is created.
  std::thread thread1([&mutex, &cv] ()
  {
    common::SignalHandler handler;
    EXPECT_TRUE(handler.Initialized());

    // This signal handler will unblock the thread, allowing it to exit.
    EXPECT_TRUE(handler.AddCallback([&cv] (int _sig)
    {
      gHandler1Sig = _sig;
      cv.notify_all();
    }));

    std::unique_lock<std::mutex> threadLock(mutex);
    // Unblock the test.
    cv.notify_all();

    // Wait for this thread's signal handler.
    cv.wait(threadLock);
  });

  // Create a signal handler outside the thread.
  common::SignalHandler handler;
  EXPECT_TRUE(handler.Initialized());
  EXPECT_TRUE(handler.AddCallback([] (int _sig)
  {
    gHandler2Sig = _sig;
  }));

  // Wait for the thread to finish creating its signal handler
  cv.wait(outerLock);

  // Raise the signal and join the thread.
  raise(SIGINT);
  outerLock.unlock();
  thread1.join();

  EXPECT_EQ(SIGINT, gHandler1Sig);
  EXPECT_EQ(SIGINT, gHandler2Sig);
}
