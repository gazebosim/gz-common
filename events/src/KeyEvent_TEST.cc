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

#include <gtest/gtest.h>
#include "ignition/common/KeyEvent.hh"

using namespace ignition;
using namespace common;

/////////////////////////////////////////////////
TEST(KeyEventTEST, Constructor)
{
  KeyEvent evt;
  EXPECT_EQ(evt.Type(), KeyEvent::NO_EVENT);
  EXPECT_EQ(evt.Key(), 0);
  EXPECT_TRUE(evt.Text().empty());
  EXPECT_FALSE(evt.Control());
  EXPECT_FALSE(evt.Shift());
  EXPECT_FALSE(evt.Alt());

  evt.SetType(KeyEvent::PRESS);
  evt.SetKey(123);
  evt.SetText("hello");
  evt.SetControl(true);
  evt.SetShift(true);
  evt.SetAlt(true);

  EXPECT_EQ(evt.Type(), KeyEvent::PRESS);
  EXPECT_EQ(evt.Key(), 123);
  EXPECT_EQ(evt.Text(), "hello");
  EXPECT_TRUE(evt.Control());
  EXPECT_TRUE(evt.Shift());
  EXPECT_TRUE(evt.Alt());

  KeyEvent otherEvent2(evt);
  EXPECT_EQ(otherEvent2.Type(), KeyEvent::PRESS);
  EXPECT_EQ(otherEvent2.Key(), 123);
  EXPECT_EQ(otherEvent2.Text(), "hello");
  EXPECT_TRUE(otherEvent2.Control());
  EXPECT_TRUE(otherEvent2.Shift());
  EXPECT_TRUE(otherEvent2.Alt());
}
