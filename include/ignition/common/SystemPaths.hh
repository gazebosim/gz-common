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
#ifndef IGNITION_COMMON_SYSTEMPATHS_HH_
#define IGNITION_COMMON_SYSTEMPATHS_HH_

#include <stdio.h>

#ifdef _WIN32
  #include <direct.h>
  #define GetCurrentDir _getcwd
#else
  #include <unistd.h>
  #define GetCurrentDir getcwd
#endif

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <ignition/common/System.hh>

namespace ignition
{
  namespace common
  {
    // Forward declare private data class
    class SystemPathsPrivate;

    /// \class SystemPaths SystemPaths.hh ignition/common/SystemPaths.hh
    /// \brief Functions to handle getting system paths, keeps track of:
    ///        \li SystemPaths#pluginPaths - plugin library paths
    ///            for common::WorldPlugin
    class IGNITION_COMMON_VISIBLE  SystemPaths
    {
      /// \brief Constructor for SystemPaths
      public: SystemPaths();

      /// \brief Destructor
      public: virtual ~SystemPaths();

      /// \brief Get the log path
      /// \return the path
      public: std::string LogPath() const;

      /// \brief Get the plugin paths
      /// \return a list of paths
      public: const std::list<std::string> &PluginPaths();

      /// \brief Find a file or path using a URI
      /// \param[in] _uri the uniform resource identifier
      /// \return Returns full path name to file
      public: std::string FindFileURI(const std::string &_uri);

      /// \brief Set the plugin path environment variable to use
      /// \param [in] _env name of the environment variable
      public: void SetPluginPathEnv(const std::string &_env);

      /// \brief Find a file in the set search paths (not recursive)
      /// \param[in] _filename Name of the file to find.
      /// \param[in] _searchLocalPath True to search in the current working
      /// directory.
      /// \return Returns full path name to file
      public: std::string FindFile(const std::string &_filename,
                                   bool _searchLocalPath = true);

      /// \brief look for a file in a set of search paths (not recursive)
      /// \description This method checks if a file exists in given directories.
      ///              It does so by joining each path with the filename and
      ///              checking if the file exists. If the file exists in
      ///              multiple paths the first one is found.
      /// \param[in] _filename Name of the file to find
      /// \param[in] _paths paths to look for the file
      /// \return Returns a path that will work from the current directory
      //          or an empty string if the file was not found
      public: std::string LocateLocalFile(const std::string &_filename,
                                   const std::vector<std::string> &_paths);

      /// \brief Find a shared library by name in the plugin paths
      ///
      /// This will try different combinations of library names on different
      /// platforms. For example searching for "MyLibName" may try finding
      /// "MyLibName", "libMyLibName.so", "MyLibName.dll", etc...
      /// \param[in] _libName Name of shared libary to look for
      /// \return path to file or empty string on error
      public: std::string FindSharedLibrary(const std::string &_libName);

      /// \brief Add colon (semicolon on windows) delimited paths to plugins
      /// \param[in] _path the directory to add
      public: void AddPluginPaths(const std::string &_path);

      /// \brief clear out SystemPaths#pluginPaths
      public: void ClearPluginPaths();

      /// \brief add _suffix to the list of path search suffixes
      /// \param[in] _suffix The suffix to add
      public: void AddSearchPathSuffix(const std::string &_suffix);

      /// \brief Set the callback to use when ignition can't find a file.
      /// The callback should return a complete path to the requested file, or
      /// and empty string if the file was not found in the callback.
      /// \param[in] _cb The callback function.
      public: void SetFindFileCallback(
                  std::function<std::string (const std::string &)> _cb);

      /// \brief Set the callback to use when ignition can't find a file uri.
      /// The callback should return a complete path to the requested file, or
      /// and empty string if the file was not found in the callback.
      /// \param[in] _cb The callback function.
      public: void SetFindFileURICallback(
                  std::function<std::string (const std::string &)> _cb);

      /// \brief Format the directory path to use "/" as a separator with "/"
      /// at the end.
      /// \param[in] _path Path to normalize
      /// \return Normalized path
      public: static std::string NormalizeDirectoryPath(
                const std::string &_path);

      /// \brief Return all paths given by an environment variable.
      /// \param[in] _env Environment variable.
      /// \return A list of paths listed by the environment variable.
      public: static std::list<std::string> PathsFromEnv(
                const std::string &_env);

      /// \brief Get the delimiter that the current operating system
      /// uses to separate different paths from each other.
      public: static const char Delimiter();

#ifdef _WIN32
// Disable warning C4251
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
      /// \brief Private data pointer
      private: std::unique_ptr<SystemPathsPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };
  }
}
#endif
