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

#ifndef IGNITION_COMMON_FILESYSTEM_HH_
#define IGNITION_COMMON_FILESYSTEM_HH_

#include <memory>
#include <string>

#include <ignition/common/Export.hh>
#include <ignition/common/SuppressWarning.hh>

namespace ignition
{
  namespace common
  {
    /// \brief Options for how to handle errors that occur in functions that
    /// manipulate the filesystem.
    enum FilesystemWarningOp
    {
      /// \brief Errors that occur during filesystem manipulation should be
      /// logged as warnings using ignwarn. (Recommended)
      FSWO_LOG_WARNINGS = 0,

      /// \brief Errors that occur during filesystem manipulation should
      /// not be logged. The user will be responsible for checking the
      /// system's error flags.
      FSWO_SUPPRESS_WARNINGS
    };

    /// \brief Determine whether the given path exists on the filesystem.
    /// \param[in] _path  The path to check for existence
    /// \return True if the path exists on the filesystem, false otherwise.
    bool IGNITION_COMMON_VISIBLE exists(const std::string &_path);

    /// \brief Determine whether the given path is a directory.
    /// \param[in] _path  The path to check
    /// \return True if given path exists and is a directory, false otherwise.
    bool IGNITION_COMMON_VISIBLE isDirectory(const std::string &_path);

    /// \brief Check if the given path is a file.
    /// \param[in] _path Path to a file.
    /// \return True if _path is a file.
    bool IGNITION_COMMON_VISIBLE isFile(const std::string &_path);

    /// \brief Create a new directory on the filesystem.  Intermediate
    ///        directories must already exist.
    /// \param[in] _path  The new directory path to create
    /// \return True if directory creation was successful, false otherwise.
    bool IGNITION_COMMON_VISIBLE createDirectory(const std::string &_path);

    /// \brief Create directories for the given path
    /// \param[in] _path Path to create directories from
    /// \return true on success
    bool IGNITION_COMMON_VISIBLE createDirectories(const std::string &_path);

    /// \brief Append the preferred path separator character for this platform
    ///        onto the passed-in string.
    /// \param[in] _s  The path to start with.
    /// \return The original path with the platform path separator appended.
    std::string IGNITION_COMMON_VISIBLE const separator(
        std::string const &_s);

    /// \brief Replace forward-slashes '/' with the preferred directory
    /// separator of the current operating system. On Windows, this will turn
    /// forward-slashes into backslashes. If forward-slash is the preferred
    /// separator of the current operating system, this will do nothing.
    ///
    /// Note that this will NOT convert backslashes (or any other separator)
    /// into forward slashes, even on operating systems that use
    /// forward-slashes as separators.
    ///
    /// \param[out] _path This string will be directly modified by
    /// replacing its
    /// forward-slashes with the preferred directory separator of the current
    /// operating system.
    void IGNITION_COMMON_VISIBLE changeFromUnixPath(std::string &_path);

    /// \brief Returns a copy of _path which has been passed through
    /// changeFromUnixPath.
    ///
    /// \param[in] _path The path to start with
    /// \return A modified path that uses the preferred directory separator of
    /// the current operating system.
    std::string IGNITION_COMMON_VISIBLE copyFromUnixPath(
        const std::string &_path);

    /// \brief Replace the preferred directory separator of the current
    /// operating system with a forward-slash '/'. On Windows, this will turn
    /// backslashes into forward-slashes.
    ///
    /// \param[out] _path This string will be directly modified to use forward
    /// slashes to separate its directory names.
    void IGNITION_COMMON_VISIBLE changeToUnixPath(std::string &_path);

    /// \brief Returns a copy of _path which has been passed through
    /// changeToUnixPath.
    ///
    /// \param[in] _path The path to start with
    /// \return A modified path that uses forward slashes to separate
    /// directory names.
    std::string IGNITION_COMMON_VISIBLE copyToUnixPath(
        const std::string &_path);

    /// \brief Get the absolute path of a provided path.
    /// \param[in] _path Relative or absolute path.
    /// \return Absolute path
    std::string IGNITION_COMMON_VISIBLE absPath(const std::string &_path);

    /// \brief Join two strings together to form a path
    /// \param[in] _path1 the left portion of the path
    /// \param[in] _path2 the right portion of the path
    /// \return Joined path
    std::string IGNITION_COMMON_VISIBLE joinPaths(const std::string &_path1,
                                                  const std::string &_path2);

    /// \brief base case for joinPaths(...) below
    inline std::string joinPaths(const std::string &_path)
    {
      return _path;
    }

    // The below is C++ variadic template magic to allow a joinPaths
    // method that takes 1-n number of arguments to append together.

    /// \brief Append one or more additional path elements to the first
    ///        passed in argument.
    /// \param[in] args  The paths to append together
    /// \return A new string with the paths appended together.
    template<typename... Args>
    inline std::string joinPaths(const std::string &_path1,
                                 const std::string &_path2,
                                 Args const &..._args)
    {
      return joinPaths(joinPaths(_path1, _path2),
                       joinPaths(_args...));
    }

    /// \brief Get the current working directory
    /// \return Name of the current directory
    std::string IGNITION_COMMON_VISIBLE cwd();

    /// \brief Given a path, get just the basename portion.
    /// \param[in] _path  The full path.
    /// \return A new string with just the basename portion of the path.
    std::string IGNITION_COMMON_VISIBLE basename(
        const std::string &_path);

    /// \brief Copy a file.
    /// \param[in] _existingFilename Path to an existing file.
    /// \param[in] _newFilename Path of the new file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True on success.
    bool IGNITION_COMMON_VISIBLE copyFile(
        const std::string &_existingFilename,
        const std::string &_newFilename,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Move a file.
    /// \param[in] _existingFilename Full path to an existing file.
    /// \param[in] _newFilename Full path of the new file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True on success.
    bool IGNITION_COMMON_VISIBLE moveFile(
        const std::string &_existingFilename,
        const std::string &_newFilename,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove an empty directory
    /// \remarks the directory must be empty to be removed
    /// \param[in] _path Path to a directory.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True if _path is a directory and was removed.
    bool IGNITION_COMMON_VISIBLE removeDirectory(
        const std::string &_path,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove a file.
    /// \param[in] _existingFilename Full path to an existing file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True on success.
    bool IGNITION_COMMON_VISIBLE removeFile(
        const std::string &_existingFilename,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove an empty directory or file.
    /// \param[in] _path Path to a directory or file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True if _path was removed.
    bool IGNITION_COMMON_VISIBLE removeDirectoryOrFile(
        const std::string &_path,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove a file or a directory and all its contents.
    /// \param[in] _path Path to a directory or file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True if _path was removed.
    bool IGNITION_COMMON_VISIBLE removeAll(
        const std::string &_path,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \internal
    class DirIterPrivate;

    /// \class DirIter Filesystem.hh
    /// \brief A class for iterating over all items in a directory.
    class IGNITION_COMMON_VISIBLE DirIter
    {
      /// \brief Constructor.
      /// \param[in] _in  Directory to iterate over.
      public: explicit DirIter(const std::string &_in);

      /// \brief Constructor for end element.
      public: DirIter();

      /// \brief Dereference operator; returns current directory record.
      /// \return A string representing the entire path of the directory
      /// record.
      public: std::string operator*() const;

      /// \brief Pre-increment operator; moves to next directory record.
      /// \return This iterator.
      public: const DirIter &operator++();

      /// \brief Comparison operator to see if this iterator is at the
      ///        same point as another iterator.
      /// \param[in] _other  The other iterator to compare against.
      /// \return true if the iterators are equal, false otherwise.
      public: bool operator!=(const DirIter &_other) const;

      /// \brief Destructor
      public: ~DirIter();

      /// \brief Move to the next directory record, skipping . and .. records.
      private: void Next();

      /// \brief Set the internal variable to the empty string.
      private: void SetInternalEmpty();

      /// \brief Close an open directory handle.
      private: void CloseHandle();

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Private data.
      private: std::unique_ptr<DirIterPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#endif
