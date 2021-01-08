/*
 * MIT License
 *
 * Copyright (c) 2019 Arnaud Kapp (Xaqq), Barry Revzin, Mart Sõmermaa
 * Copyright (c) 2020 Martin Pecka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ignition::common::FlagSet is a type-safe class for using enums as flags in
// C++14 with an underlying std::bitset.
// See https://github.com/mrts/flag-set-cpp
// Licence: MIT

#include <ignition/common/FlagSet.hh>

#include <cstdint>
#include <unordered_set>

#include "gtest/gtest.h"

enum class Options : uint8_t
{
  FULLSCREEN,
  INVERT_MOUSE,
  FLASH,
  RED_BACKGROUND,
  RED_FOREGROUND,
  _
};

TEST(FlagSet, TestAnd)
{
  ignition::common::FlagSet<Options> red(
      Options::RED_FOREGROUND | Options::RED_BACKGROUND);

  auto result = red & Options::RED_BACKGROUND;
  EXPECT_TRUE(result);
  EXPECT_EQ(result.Count(), 1u);

  result = red & Options::RED_FOREGROUND;
  EXPECT_TRUE(result);
  EXPECT_EQ(result.Count(), 1u);

  result = red & (Options::RED_FOREGROUND | Options::RED_BACKGROUND);
  EXPECT_TRUE(result);
  EXPECT_EQ(result.Count(), 2u);

  result = ~red & Options::RED_BACKGROUND;
  EXPECT_FALSE(result);
  EXPECT_EQ(result.Count(), 0u);

  ignition::common::FlagSet<Options> red_foreground(Options::RED_FOREGROUND);
  red &= Options::RED_FOREGROUND;
  EXPECT_TRUE(red == red_foreground);
}

TEST(FlagSet, TestOr)
{
  ignition::common::FlagSet<Options> red;
  red |= Options::RED_FOREGROUND | Options::RED_BACKGROUND;
  EXPECT_TRUE(red);
  EXPECT_EQ(red.Count(), 2u);

  ignition::common::FlagSet<Options> options;
  options |= (Options::FULLSCREEN | Options::FLASH);

  auto result = options & (Options::FULLSCREEN | Options::FLASH |
                           Options::RED_FOREGROUND);
  EXPECT_TRUE(result);

  ignition::common::FlagSet<Options> expected;
  expected |= Options::FULLSCREEN;
  EXPECT_EQ(options & expected, expected);

  result = options & (Options::RED_FOREGROUND | Options::RED_BACKGROUND);
  EXPECT_FALSE(result);

  EXPECT_FALSE(options & red);

  EXPECT_TRUE(!(options & Options::INVERT_MOUSE));

  options |= ~red;

  EXPECT_TRUE(options & Options::INVERT_MOUSE);
}

TEST(FlagSet, TestSetReset)
{
  ignition::common::FlagSet<Options> options;
  EXPECT_EQ(options.Count(), 0u);

  options.Set();
  EXPECT_EQ(options.Count(), 5u);
  EXPECT_EQ(options.Size(), 5u);
  EXPECT_EQ(options.String(), "11111");

  options.Reset();
  EXPECT_EQ(options.Count(), 0u);

  options.Set(Options::FLASH);
  EXPECT_EQ(options.Count(), 1u);
  EXPECT_TRUE(options[Options::FLASH]);

  options.Set(Options::FLASH, false);
  EXPECT_EQ(options.Count(), 0u);

  options.Set(Options::FLASH);
  options.Set(Options::INVERT_MOUSE);
  EXPECT_EQ(options.Count(), 2u);
}

TEST(FlagSet, TestAnyAllNone)
{
  ignition::common::FlagSet<Options> options;
  EXPECT_TRUE(options.None());
  EXPECT_FALSE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set();
  EXPECT_FALSE(options.None());
  EXPECT_TRUE(options.Any());
  EXPECT_TRUE(options.All());

  options.Reset();
  EXPECT_TRUE(options.None());
  EXPECT_FALSE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set(Options::FLASH);
  EXPECT_FALSE(options.None());
  EXPECT_TRUE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set(Options::FLASH, false);
  EXPECT_TRUE(options.None());
  EXPECT_FALSE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set(Options::FLASH);
  options.Set(Options::INVERT_MOUSE);
  EXPECT_FALSE(options.None());
  EXPECT_TRUE(options.Any());
  EXPECT_FALSE(options.All());
}

TEST(FlagSet, TestStaticConstructors)
{
  EXPECT_TRUE(ignition::common::FlagSet<Options>::AllSet().All());
  EXPECT_FALSE(ignition::common::FlagSet<Options>::AllSet().None());
  EXPECT_TRUE(ignition::common::FlagSet<Options>::AllSet().Any());

  EXPECT_FALSE(ignition::common::FlagSet<Options>::NoneSet().All());
  EXPECT_TRUE(ignition::common::FlagSet<Options>::NoneSet().None());
  EXPECT_FALSE(ignition::common::FlagSet<Options>::NoneSet().Any());
}


TEST(FlagSet, TestHash)
{
  using namespace ignition::common;

  const auto options1 = FlagSet<Options>::AllSet();
  const auto options2 = FlagSet<Options>::NoneSet();

  std::hash<FlagSet<Options>> hash {};

  ASSERT_NE(hash(options1), hash(options2));
  ASSERT_EQ(hash(options1), hash(options1));
  ASSERT_EQ(hash(options2), hash(options2));

  std::unordered_set<FlagSet<Options>> valid;
  ASSERT_EQ(valid.find(FlagSet<Options>::AllSet()), valid.end());
  valid.insert(FlagSet<Options>::AllSet());
  ASSERT_NE(valid.find(FlagSet<Options>::AllSet()), valid.end());
}
