/*
 * Copyright 2017 Open Source Robotics Foundation
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

#ifndef _WIN32
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// The symlink tests should always work on UNIX systems
#define BUILD_SYMLINK_TESTS

/////////////////////////////////////////////////
bool create_and_switch_to_temp_dir(std::string &_new_temp_path)
{
  std::string tmppath;
  const char *tmp = getenv("TMPDIR");
  if (tmp)
  {
    tmppath = std::string(tmp);
  }
  else
  {
    tmppath = std::string("/tmp");
  }

  tmppath += "/XXXXXX";

  char *dtemp = mkdtemp(const_cast<char *>(tmppath.c_str()));
  if (dtemp == nullptr)
  {
    return false;
  }
  if (chdir(dtemp) < 0)
  {
    return false;
  }

  char resolved[PATH_MAX];
  if (realpath(dtemp, resolved) == nullptr)
  {
    return false;
  }

  _new_temp_path = std::string(resolved);

  return true;
}

/////////////////////////////////////////////////
bool create_new_empty_file(const std::string &_filename)
{
  int fd = open(_filename.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd < 0)
  {
    return false;
  }

  close(fd);

  return true;
}

/////////////////////////////////////////////////
bool create_new_file_symlink(const std::string &_symlink,
                             const std::string &_target)
{
  return symlink(_target.c_str(), _symlink.c_str()) == 0;
}

/////////////////////////////////////////////////
bool create_new_dir_symlink(const std::string &_symlink,
                            const std::string &_target)
{
  return symlink(_target.c_str(), _symlink.c_str()) == 0;
}

/////////////////////////////////////////////////
bool create_new_file_hardlink(const std::string &_hardlink,
                              const std::string &_target)
{
  return link(_target.c_str(), _hardlink.c_str()) == 0;
}

#else
#include <windows.h>
#include <winnt.h>
#include <cstdint>
#include "PrintWindowsSystemWarning.hh"

#ifdef IGN_BUILD_SYMLINK_TESTS_ON_WINDOWS
// The symlink tests require special permissions to work on Windows,
// so they will be disabled by default. For more information, see:
// https://bitbucket.org/ignitionrobotics/ign-common/issues/21
#define BUILD_SYMLINK_TESTS
#endif

/////////////////////////////////////////////////
bool create_and_switch_to_temp_dir(std::string &_new_temp_path)
{
  char temp_path[MAX_PATH + 1];
  DWORD path_len = ::GetTempPathA(MAX_PATH, temp_path);
  if (path_len >= MAX_PATH || path_len <= 0)
  {
    return false;
  }
  std::string path_to_create(temp_path);
  srand(static_cast<uint32_t>(time(nullptr)));

  for (int count = 0; count < 50; ++count)
  {
    // Try creating a new temporary directory with a randomly generated name.
    // If the one we chose exists, keep trying another path name until we reach
    // some limit.
    std::string new_dir_name;
    new_dir_name.append(std::to_string(::GetCurrentProcessId()));
    new_dir_name.push_back('_');
    // On Windows, rand_r() doesn't exist as an alternative to rand(), so the
    // cpplint warning is spurious.  This program is not multi-threaded, so
    // it is safe to suppress the threadsafe_fn warning here.
    new_dir_name.append(
       std::to_string(rand()    // NOLINT(runtime/threadsafe_fn)
                      % ((int16_t)0x7fff)));

    path_to_create += new_dir_name;
    if (::CreateDirectoryA(path_to_create.c_str(), nullptr))
    {
      _new_temp_path = path_to_create;
      return ::SetCurrentDirectoryA(_new_temp_path.c_str()) != 0;
    }
  }

  return false;
}

/////////////////////////////////////////////////
bool create_new_empty_file(const std::string &_filename)
{
  return ::CreateFileA(_filename.c_str(),
                       FILE_READ_DATA,
                       FILE_SHARE_READ,
                       nullptr,
                       OPEN_ALWAYS,
                       0,
                       nullptr) != INVALID_HANDLE_VALUE;
}

/////////////////////////////////////////////////
bool create_new_file_symlink(const std::string &_symlink,
                             const std::string &_target)
{
  const bool linked = ::CreateSymbolicLinkA(
        _symlink.c_str(), _target.c_str(), 0);

  if (!linked)
  {
    ignition::common::PrintWindowsSystemWarning(
          "Failed to create file symlink from [" + _target
          + "] to [" + _symlink + "]");
  }

  return linked;
}

/////////////////////////////////////////////////
bool create_new_dir_symlink(const std::string &_symlink,
                            const std::string &_target)
{
  const bool linked = ::CreateSymbolicLinkA(_symlink.c_str(), _target.c_str(),
                                            SYMBOLIC_LINK_FLAG_DIRECTORY);
  if (!linked)
  {
    ignition::common::PrintWindowsSystemWarning(
          "Failed to create directory symlink from [" + _target
          + "] to [" + _symlink + "]");
  }

  return linked;
}

/////////////////////////////////////////////////
bool create_new_file_hardlink(const std::string &_hardlink,
                              const std::string &_target)
{
  return ::CreateHardLinkA(_hardlink.c_str(), _target.c_str(), nullptr) == TRUE;
}

#endif

#include <fstream> // NOLINT
#include "ignition/common/Filesystem.hh"

namespace igncmn = ignition::common;
using namespace ignition;
using namespace igncmn;

/////////////////////////////////////////////////
/// \brief Test file operations
TEST(Util_TEST, fileOps)
{
  EXPECT_FALSE(common::cwd().empty());
  EXPECT_TRUE(common::exists(common::cwd()));
  EXPECT_TRUE(common::isDirectory(common::cwd()));
  EXPECT_FALSE(common::isFile(common::cwd()));

  EXPECT_TRUE(common::isFile(__FILE__));
  EXPECT_FALSE(common::isDirectory(__FILE__));

  std::ofstream tmpOut("test.tmp");
  tmpOut << "Output" << std::endl;
  tmpOut.close();

  EXPECT_TRUE(common::copyFile("test.tmp", "test2.tmp"));
  EXPECT_TRUE(common::exists("test.tmp"));
  EXPECT_TRUE(common::exists("test2.tmp"));

  std::ifstream testIn("test.tmp");
  std::string testInContent((std::istreambuf_iterator<char>(testIn)),
                            (std::istreambuf_iterator<char>()));

  std::ifstream test2In("test2.tmp");
  std::string test2InContent((std::istreambuf_iterator<char>(test2In)),
                             (std::istreambuf_iterator<char>()));
  // We must close the file stream for test2.tmp, or else Windows
  // will refuse to remove the file from the filesystem in the
  // upcoming test.
  test2In.close();

  EXPECT_EQ(testInContent, test2InContent);

  EXPECT_TRUE(common::moveFile("test2.tmp", "test3.tmp"));

  EXPECT_FALSE(common::exists("test2.tmp"));
  EXPECT_TRUE(common::exists("test3.tmp"));

  std::ifstream test3In("test3.tmp");
  std::string test3InContent((std::istreambuf_iterator<char>(test3In)),
                             (std::istreambuf_iterator<char>()));
  // We must close this file stream for test3.tmp or else Windows
  // will refuse to remove the file from the filesystem in the
  // upcoming test.
  test3In.close();

  EXPECT_EQ(testInContent, test3InContent);

  EXPECT_FALSE(common::copyFile("test3.tmp", "test3.tmp"));
  EXPECT_FALSE(common::copyFile("test3.tmp", "./test3.tmp"));

  std::remove("test.tmp");

  // This file shouldn't exist, but we'll try to remove just in case the
  // test failed.
  EXPECT_FALSE(common::removeFile("test2.tmp"));

  EXPECT_TRUE(common::removeFile("test3.tmp"));

  EXPECT_FALSE(common::exists("test3.tmp"));
}

/////////////////////////////////////////////////
/// \brief Test file operations
TEST(Util_TEST, moreFileOps)
{
  EXPECT_FALSE(common::copyFile("__wrong__.tmp", "test2.tmp"));
  EXPECT_TRUE(!common::exists("test2.tmp"));
  EXPECT_FALSE(common::copyFile("test.tmp", "__wrong_dir__/__wrong__.tmp"));
  EXPECT_TRUE(!common::exists("__wrong_dir__"));

  EXPECT_FALSE(common::moveFile("__wrong__.tmp", "test3.tmp"));
  EXPECT_TRUE(!common::exists("test3.tmp"));
  EXPECT_FALSE(common::moveFile("test2.tmp", "__wrong_dir__/__wrong__.tmp"));
  EXPECT_TRUE(!common::exists("__wrong_dir__"));
}

/////////////////////////////////////////////////
TEST(Filesystem, exists)
{
  std::string new_temp_dir;
  ASSERT_TRUE(create_and_switch_to_temp_dir(new_temp_dir));
  ASSERT_TRUE(create_new_empty_file("newfile"));
  ASSERT_TRUE(createDirectory("fstestexists"));

  const auto &relativeSubdir = joinPaths("fstestexists", "newfile2");
  const auto &absoluteSubdir = joinPaths(new_temp_dir, relativeSubdir);
  ASSERT_TRUE(create_new_empty_file(relativeSubdir));

  EXPECT_TRUE(exists("fstestexists"));
  EXPECT_TRUE(isDirectory("fstestexists"));

  EXPECT_FALSE(exists("notcreated"));
  EXPECT_FALSE(isDirectory("notcreated"));

  EXPECT_TRUE(exists("newfile"));
  EXPECT_FALSE(isDirectory("newfile"));

  // file in relative subdirectory
  EXPECT_TRUE(exists(relativeSubdir));
  EXPECT_TRUE(isFile(relativeSubdir));
  EXPECT_FALSE(isDirectory(relativeSubdir));

  // file in absolute subdirectory
  EXPECT_TRUE(exists(absoluteSubdir));
  EXPECT_TRUE(isFile(absoluteSubdir));
  EXPECT_FALSE(isDirectory(absoluteSubdir));
}

// The symlink tests require special permissions to work on Windows,
// so they will be disabled by default. For more information, see:
// https://bitbucket.org/ignitionrobotics/ign-common/issues/21
#ifdef BUILD_SYMLINK_TESTS
/////////////////////////////////////////////////
TEST(Filesystem, symlink_exists)
{
  // There are 5 cases we want to test for links (Unix doesn't allow hard links
  // to directories or to non-existent files):
  // 1. symbolic link to existing file
  // 2. symbolic link to non-existent file
  // 3. symbolic link to existing directory
  // 4. symbolic link to non-existent directory
  // 5. hard link to existing file
  std::string new_temp_dir;
  ASSERT_TRUE(create_and_switch_to_temp_dir(new_temp_dir));
  ASSERT_TRUE(create_new_empty_file("newfile"));
  ASSERT_TRUE(createDirectory("newdir"));

  // Case 1
  ASSERT_TRUE(create_new_file_symlink("symlink-file", "newfile"));
  EXPECT_TRUE(exists("symlink-file"));
  EXPECT_TRUE(removeFile("symlink-file"));

  // Case 2
  ASSERT_TRUE(create_new_file_symlink("symlink-file-broken", "nonexistent"));
  EXPECT_FALSE(exists("symlink-file-broken"));

  // Case 3
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir", "newdir"));
  EXPECT_TRUE(exists("symlink-dir"));
  EXPECT_TRUE(removeDirectory("symlink-dir"));

  // Case 4
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir-broken", "nonexistent-dir"));
  EXPECT_FALSE(exists("symlink-dir-broken"));

  // Case 5
  ASSERT_TRUE(create_new_file_hardlink("hardlink-file", "newfile"));
  EXPECT_TRUE(exists("hardlink-file"));
  EXPECT_TRUE(removeFile("hardlink-file"));
}
#endif

/////////////////////////////////////////////////
TEST(Filesystem, cwd)
{
  std::string new_temp_dir;
  ASSERT_TRUE(create_and_switch_to_temp_dir(new_temp_dir));

  std::string path = cwd();
  EXPECT_EQ(path, new_temp_dir);
}

/////////////////////////////////////////////////
TEST(Filesystem, append)
{
  std::string path = "tmp";
  path = joinPaths(path, "hello");

  ASSERT_EQ(path, separator("tmp") + "hello");

  path = joinPaths(path, "there", "again");

  EXPECT_EQ(path, separator("tmp") +
            separator("hello") +
            separator("there") + "again");
}

/////////////////////////////////////////////////
TEST(Filesystem, cwd_error)
{
  // This test intentionally creates a directory, switches to it, removes
  // the directory, and then tries to call cwd() on it to cause
  // cwd() to fail.  Windows does not allow you to remove an
  // in-use directory, so this test is restricted to Unix.
#ifndef _WIN32
  std::string new_temp_dir;
  ASSERT_TRUE(create_and_switch_to_temp_dir(new_temp_dir));

  ASSERT_EQ(rmdir(new_temp_dir.c_str()), 0);

  EXPECT_EQ(cwd(), "");
#endif
}

/////////////////////////////////////////////////
TEST(Filesystem, basename)
{
  std::string absolute = joinPaths("", "home", "bob", "foo");
  EXPECT_EQ(basename(absolute), "foo");

  std::string relative = joinPaths("baz", "foobar");
  EXPECT_EQ(basename(relative), "foobar");

  std::string bname = "bzzz";
  EXPECT_EQ(basename(bname), "bzzz");

  std::string nobase = joinPaths("baz", "");
  EXPECT_EQ(basename(nobase), "baz");

  std::string multiple_slash = separator("baz") + separator("") + separator("")
    + separator("");
  EXPECT_EQ(basename(multiple_slash), "baz");

  std::string multiple_slash_middle = separator("") + separator("home")
    + separator("") + separator("") + separator("bob") + separator("foo");
  EXPECT_EQ(basename(multiple_slash_middle), "foo");

  std::string multiple_slash_start = separator("") + separator("")
    + separator("") + separator("home") + separator("bob") + separator("foo");
  EXPECT_EQ(basename(multiple_slash_start), "foo");

  std::string slash_only = separator("") + separator("");
  EXPECT_EQ(basename(slash_only), separator(""));

  std::string multiple_slash_only = separator("") + separator("")
    + separator("") + separator("");
  EXPECT_EQ(basename(multiple_slash_only), separator(""));
}

/////////////////////////////////////////////////
TEST(Filesystem, directory_iterator)
{
  std::string new_temp_dir;
  ASSERT_TRUE(create_and_switch_to_temp_dir(new_temp_dir));
  ASSERT_TRUE(create_new_empty_file("newfile"));
  ASSERT_TRUE(createDirectory("newdir"));
#ifdef BUILD_SYMLINK_TESTS
  ASSERT_TRUE(create_new_file_symlink("symlink-file", "newfile"));
  ASSERT_TRUE(create_new_file_symlink("symlink-file-broken", "nonexistent"));
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir", "newdir"));
  ASSERT_TRUE(create_new_dir_symlink("symlink-dir-broken", "nonexistent-dir"));
  ASSERT_TRUE(create_new_file_hardlink("hardlink-file", "newfile"));
#endif

  std::set<std::string> found_items;

  DirIter endIter;
  for (DirIter dirIter("."); dirIter != endIter; ++dirIter)
  {
    found_items.insert(basename(*dirIter));
  }

  EXPECT_FALSE(found_items.find("newfile") == found_items.end());
  EXPECT_FALSE(found_items.find("newdir") == found_items.end());
#ifdef BUILD_SYMLINK_TESTS
  EXPECT_FALSE(found_items.find("symlink-file") == found_items.end());
  EXPECT_FALSE(found_items.find("symlink-file-broken") == found_items.end());
  EXPECT_FALSE(found_items.find("symlink-dir") == found_items.end());
  EXPECT_FALSE(found_items.find("symlink-dir-broken") == found_items.end());
  EXPECT_FALSE(found_items.find("hardlink-file") == found_items.end());
#endif

  found_items.clear();
  for (DirIter dirIter(""); dirIter != endIter; ++dirIter)
  {
    found_items.insert(basename(*dirIter));
  }

  EXPECT_EQ(found_items.size(), 0UL);
}

/////////////////////////////////////////////////
TEST(Filesystem, createDirectories)
{
  std::string new_temp_dir;
  ASSERT_TRUE(create_and_switch_to_temp_dir(new_temp_dir));

  // test creating directories using relative path
  std::string rel_dir = separator("rel_dir") + "subdir";
  ASSERT_TRUE(createDirectories(rel_dir));
  EXPECT_TRUE(exists(rel_dir));
  EXPECT_TRUE(isDirectory(rel_dir));

  // test creating directories using abs path
  std::string path = cwd();
  EXPECT_EQ(path, new_temp_dir);

  std::string abs_dir = separator(cwd()) + separator("abs_dir") + "subdir";
  ASSERT_TRUE(createDirectories(abs_dir));
  EXPECT_TRUE(exists(abs_dir));
  EXPECT_TRUE(isDirectory(abs_dir));
}

/////////////////////////////////////////////////
/// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
