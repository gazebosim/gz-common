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
#include <cstring>
#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdio>

#include "ignition/common/Util.hh"
#include "ignition/common/SystemPaths.hh"

#ifdef _WIN32
  #define snprintf _snprintf
#endif

using namespace ignition;

class SystemPathsFixture : public ::testing::Test
{
  public: virtual void SetUp()
    {
      this->backupPluginPath = "IGN_PLUGIN_PATH=";

      if (getenv("IGN_PLUGIN_PATH"))
        this->backupPluginPath += getenv("IGN_PLUGIN_PATH");

      putenv(const_cast<char*>("IGN_PLUGIN_PATH="));

#ifdef _WIN32
      this->filesystemRoot = "C:\\";
#else
      this->filesystemRoot = "/";
#endif
    }

  public: virtual void TearDown()
    {
      putenv(const_cast<char*>(this->backupPluginPath.c_str()));
    }

  public: std::string backupPluginPath;
  public: std::string filesystemRoot;
};

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SystemPaths)
{
  common::SystemPaths paths;
  std::string env_str("IGN_PLUGIN_PATH=/tmp/plugin");
  env_str += ignition::common::SystemPaths::Delimiter();
  env_str += "/test/plugin/now";

  // The char* passed to putenv will continue to be stored after the lifetime
  // of this function, so we should not pass it a pointer which has an automatic
  // lifetime.
  static char env[1024];
  snprintf(env, sizeof(env), "%s", env_str.c_str());
  putenv(env);

  const std::list<std::string> pathList3 = paths.PluginPaths();
  EXPECT_EQ(static_cast<unsigned int>(2), pathList3.size());
  EXPECT_STREQ("/tmp/plugin/", pathList3.front().c_str());
  EXPECT_STREQ("/test/plugin/now/", pathList3.back().c_str());

  paths.ClearPluginPaths();

  EXPECT_EQ(static_cast<unsigned int>(2), paths.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, InitialNoSearchPaths)
{
  common::SystemPaths sp;
  EXPECT_EQ(0u, sp.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, AddOneSearchPath)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("./");
  auto paths = sp.PluginPaths();
  EXPECT_EQ(1, std::count(paths.begin(), paths.end(), "./"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, ClearSearchPaths)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("./");
  EXPECT_EQ(1u, sp.PluginPaths().size());
  sp.ClearPluginPaths();
  EXPECT_EQ(0u, sp.PluginPaths().size());
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SearchPathGetsTrailingSlash)
{
  common::SystemPaths sp;
  sp.AddPluginPaths("/usr/local/lib");
  auto paths = sp.PluginPaths();
  EXPECT_EQ(1, std::count(paths.begin(), paths.end(), "/usr/local/lib/"));
}

/////////////////////////////////////////////////
TEST_F(SystemPathsFixture, SearchPathUsesForwardSlashes)
{
#ifdef _WIN32
  std::string before = "C:\\user\\alice\\gazebolibs\\";
  std::string after = "C:/user/alice/gazebolibs/";
#else
  std::string before = "\\usr\\lib\\";
  std::string after = "/usr/lib/";
#endif
  common::SystemPaths sp;
  sp.AddPluginPaths(before);
  auto paths = sp.PluginPaths();
  EXPECT_EQ(after, *paths.begin());
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, FindFileURI)
{
  auto dir1 = ignition::common::absPath("test_dir1");
  auto dir2 = ignition::common::absPath("test_dir2");
  ignition::common::createDirectories(dir1);
  ignition::common::createDirectories(dir2);
  auto file1 = ignition::common::absPath(
      ignition::common::joinPaths(dir1, "test_f1"));
  auto file2 = ignition::common::absPath(
      ignition::common::joinPaths(dir2, "test_f2"));

  std::ofstream fout;
  fout.open(file1, std::ofstream::out);
  fout << "asdf";
  fout.close();
  fout.open(file2, std::ofstream::out);
  fout << "asdf";
  fout.close();

  common::SystemPaths sp;
  auto filesystemRootUnix = ignition::common::copyToUnixPath(
          this->filesystemRoot);

  EXPECT_EQ("", sp.FindFileURI("file://no_such_file"));
  EXPECT_EQ(file1, sp.FindFileURI("file://test_dir1/test_f1"));
  EXPECT_EQ(file1, sp.FindFileURI("file://" +
                                  ignition::common::copyToUnixPath(file1)));
  EXPECT_EQ("", sp.FindFileURI("osrf://unknown.protocol"));
  EXPECT_EQ("", sp.FindFileURI(this->filesystemRoot + "no_such_file"));
  EXPECT_EQ("", sp.FindFileURI("file://" + filesystemRootUnix +
                               "no_such_file"));
  EXPECT_EQ("", sp.FindFileURI("not_an_uri"));

#ifdef _WIN32
  EXPECT_EQ(this->filesystemRoot + "Windows",
            sp.FindFileURI("file://" + filesystemRootUnix + "Windows"));
  // TODO: This test should not work, because 'file://C:\\Windows' is not a
  // valid URI. However, until the URI class is upgraded to resolve this as
  // invalid URI, this test has to be commented out.
  // EXPECT_EQ("",
  //     sp.FindFileURI("file://" + this->filesystemRoot + "Windows"));
#endif

  auto robotCb = [dir1](const std::string &_s)
  {
    return _s.find("robot://", 0) != std::string::npos ?
           ignition::common::joinPaths(dir1, _s.substr(8)) : "";
  };
  auto osrfCb = [dir2](const ignition::common::URI &_uri)
  {
    return _uri.Scheme() == "osrf" ?
           ignition::common::joinPaths(dir2, _uri.Path().Str()) : "";
  };
  auto robot2Cb = [dir2](const ignition::common::URI &_uri)
  {
    return _uri.Scheme() == "robot" ?
           ignition::common::joinPaths(dir2, _uri.Path().Str()) : "";
  };

  EXPECT_EQ("", sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ("", sp.FindFileURI("osrf://test_f2"));

  // We still want to test the deprecated function until it is removed.
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
  sp.SetFindFileURICallback(robotCb);
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

  EXPECT_EQ(file1, sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ("", sp.FindFileURI("osrf://test_f2"));

  sp.AddFindFileURICallback(osrfCb);
  EXPECT_EQ(file1, sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ(file2, sp.FindFileURI("osrf://test_f2"));

  // Test that th CB from SetFindFileURICallback is called first even when a
  // second handler for the same protocol is available
  sp.AddFindFileURICallback(robot2Cb);
  EXPECT_EQ(file1, sp.FindFileURI("robot://test_f1"));
  EXPECT_EQ(file2, sp.FindFileURI("osrf://test_f2"));
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, FindFile)
{
  auto dir1 = ignition::common::absPath("test_dir1");
  auto dir2 = ignition::common::absPath("test_dir2");
  ignition::common::createDirectories(dir1);
  ignition::common::createDirectories(dir2);
  auto file1 = ignition::common::absPath(
      ignition::common::joinPaths(dir1, "test_f1"));
  auto file2 = ignition::common::absPath(
      ignition::common::joinPaths(dir2, "test_f2"));

  std::ofstream fout;
  fout.open(file1, std::ofstream::out);
  fout << "asdf";
  fout.close();
  fout.open(file2, std::ofstream::out);
  fout << "asdf";
  fout.close();

  // LocateLocalFile
  common::SystemPaths sp;
  EXPECT_EQ(ignition::common::copyToUnixPath(file1),
            sp.LocateLocalFile("test_f1", {dir1, dir2}));

  EXPECT_EQ(ignition::common::copyToUnixPath(file2),
            sp.LocateLocalFile("test_f2", {dir1, dir2}));

  EXPECT_EQ("", sp.LocateLocalFile("test_f3", {dir1, dir2}));

  // FindFile
  EXPECT_EQ("", sp.FindFile(this->filesystemRoot + "no_such_file"));
  EXPECT_EQ("", sp.FindFile("no_such_file"));
  EXPECT_EQ(file1, sp.FindFile(common::joinPaths("test_dir1", "test_f1")));
  EXPECT_EQ(file1, sp.FindFile("file://test_dir1/test_f1"));

  // Existing absolute paths
#ifndef _WIN32
  const auto tmpDir = "/tmp";
  const auto homeDir = "/home";
  const auto badDir = "/bad";
#else
  const auto tmpDir = "C:\\Windows";
  const auto homeDir = "C:\\Users";
  const auto badDir = "C:\\bad";
#endif
  {
    EXPECT_EQ(tmpDir, sp.FindFile(tmpDir));
    EXPECT_EQ(homeDir, sp.FindFile(homeDir));
    EXPECT_EQ("", sp.FindFile(badDir));
  }

  // Custom callback
  {
    auto tmpCb = [tmpDir](const std::string &_s)
    {
      return _s == "tmp" ? tmpDir : "";
    };
    auto homeCb = [homeDir](const std::string &_s)
    {
      return _s == "home" ? homeDir : "";
    };
    auto badCb = [badDir](const std::string &_s)
    {
      return _s == "bad" ? badDir : "";
    };

    // We still want to test the deprecated function until it is removed.
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    sp.SetFindFileCallback(tmpCb);
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

    EXPECT_EQ(tmpDir, sp.FindFile("tmp"));
    EXPECT_EQ("", sp.FindFile("home"));
    EXPECT_EQ("", sp.FindFile("bad"));
    EXPECT_EQ("", sp.FindFile("banana"));

    sp.AddFindFileCallback(homeCb);

    EXPECT_EQ(tmpDir, sp.FindFile("tmp"));
    EXPECT_EQ(homeDir, sp.FindFile("home"));
    EXPECT_EQ("", sp.FindFile("bad"));
    EXPECT_EQ("", sp.FindFile("banana"));

    sp.AddFindFileCallback(badCb);

    EXPECT_EQ(tmpDir, sp.FindFile("tmp"));
    EXPECT_EQ(homeDir, sp.FindFile("home"));
    EXPECT_EQ("", sp.FindFile("bad"));
    EXPECT_EQ("", sp.FindFile("banana"));
  }
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, NormalizeDirectoryPath)
{
  EXPECT_EQ(ignition::common::SystemPaths::NormalizeDirectoryPath("a/b/c/"),
                                                                  "a/b/c/");
  EXPECT_EQ(ignition::common::SystemPaths::NormalizeDirectoryPath("a/b/c"),
                                                                  "a/b/c/");
  EXPECT_EQ(ignition::common::SystemPaths::NormalizeDirectoryPath("/a/b/c/"),
                                                                  "/a/b/c/");
  EXPECT_EQ(ignition::common::SystemPaths::NormalizeDirectoryPath("/a/b/c"),
                                                                  "/a/b/c/");
  EXPECT_EQ(ignition::common::SystemPaths::NormalizeDirectoryPath("a\\b\\c"),
                                                                  "a/b/c/");
  EXPECT_EQ(ignition::common::SystemPaths::NormalizeDirectoryPath("a\\b\\c\\"),
                                                                  "a/b/c/");
}

//////////////////////////////////////////////////
TEST_F(SystemPathsFixture, PathsFromEnv)
{
  std::string env_str = "IGN_PLUGIN_PATH=/tmp/plugin";
  env_str += ignition::common::SystemPaths::Delimiter();
  env_str += "/test/plugin/now/";
  env_str += ignition::common::SystemPaths::Delimiter();
  env_str += "/tmp/plugin";

  // The char* passed to putenv will continue to be stored after the lifetime
  // of this function, so we should not pass it a pointer which has an automatic
  // lifetime.
  static char env[1024];
  snprintf(env, sizeof(env), "%s", env_str.c_str());
  putenv(env);

  auto paths = ignition::common::SystemPaths::PathsFromEnv("IGN_PLUGIN_PATH");

  EXPECT_EQ(paths.size(), 2u);

  unsigned int count = 0;
  for (auto const &path : paths)
  {
    if (count == 0)
    {
      EXPECT_EQ(path, "/tmp/plugin/");
    }
    if (count == 1)
    {
      EXPECT_EQ(path, "/test/plugin/now/");
    }
    if (count == 2)
    {
      FAIL();
    }

    ++count;
  }
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
