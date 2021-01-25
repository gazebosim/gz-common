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
#ifndef IGNITION_COMMON_URI_HH_
#define IGNITION_COMMON_URI_HH_

#include <memory>
#include <optional>
#include <string>

#include <ignition/common/Export.hh>
#include <ignition/common/SuppressWarning.hh>

namespace ignition
{
  namespace common
  {
    // Forward declare private data classes.
    class URIAuthorityPrivate;
    class URIPathPrivate;
    class URIQueryPrivate;
    class URIFragmentPrivate;
    class URIPrivate;

    /// A URI authority contains userinfo, host, and port data. The format
    /// of a URI authority is `//userinfo@host:port`. The `userinfo` and
    /// `port` components are optional.
    ///
    /// A URI Authority requires the existence of host information,
    /// except when a Scheme is `file`.  When a scheme is `file`, then the
    /// following are considered valid URIs and equivalent:
    ///
    ///   * file:/abs/path
    ///   * file:///abs/path
    ///
    /// Keep in mind that a URI path must start with a forward slash when an
    /// authority, as indicated by two forward slashes, is present. This
    /// means relative file paths cannot be specified with an empty
    /// authority. For example, `file://abs/path` will result in a host
    /// value of `abs` and the URI path will be `/path`. You can specify
    /// a relative path using `file:abs/path`.
    class IGNITION_COMMON_VISIBLE URIAuthority
    {
      /// \brief Constructor
      public: URIAuthority();

      /// \brief Copy constructor.
      /// \param[in] _path Another URIAuthority.
      public: URIAuthority(const URIAuthority &_path);

      /// \brief Construct a URIAuthority object from a string.
      /// \param[in] _str A string.
      public: explicit URIAuthority(const std::string &_str);

      /// \brief Destructor
      public: virtual ~URIAuthority();

      /// \brief Remove all parts of the authority
      public: void Clear();

      /// \brief Get the user information.
      /// \return User information, or an empty string if the user
      /// information is not set.
      public: std::string UserInfo() const;

      /// \brief Set the user information.
      /// \param[in] _userInfo The user information string.
      public: void SetUserInfo(const std::string &_userInfo) const;

      /// \brief Get the host.
      /// \return The host.
      public: std::string Host() const;

      /// \brief Set the host.
      /// \param[in] _host The host.
      public: void SetHost(const std::string &_host) const;

      /// \brief True if an empty host is considered valid.
      /// \return True if an empty host is valid.
      public: bool EmptyHostValid() const;

      /// \brief Set whether an empty host is considered valid.
      /// This should only be set to true if the corresponding URIScheme
      /// is "file".
      /// \param[in] _valid True if an empty host is valid.
      public: void SetEmptyHostValid(bool _valid) const;

      /// \brief Get the port.
      /// \return The port number, which is optional.
      public: std::optional<int> Port() const;

      /// \brief Set the port number.
      public: void SetPort(int _port) const;

      /// \brief Return true if the two authorities match.
      /// \param[in] _auth Authority.
      /// return True of the authorities match.
      public: bool operator==(const URIAuthority &_auth) const;

      /// \brief Get the complete authoriy as a string.
      /// \return The authority as a string.
      public: std::string Str() const;

      /// \brief Assignment operator.
      /// \param[in] _auth Another URIAuthority.
      /// \return Itself.
      public: URIAuthority &operator=(const URIAuthority &_auth);

      /// \brief Return true if the string is a valid path.
      /// \param[in] _str String to check.
      /// \param[in] _emptyHostValid Set this to true if an empty host is
      /// valid. This should only be set to true if the corresponding URIScheme
      /// is "file".
      /// \return True if _str is a valid URI path.
      public: static bool Valid(const std::string &_str,
                  bool _emptyHostValid = false);

      /// \brief Return true if this is a valid authority.
      /// \return True if this is a valid URI authority.
      public: bool Valid() const;

      /// \brief Parse a string as URIAuthority.
      /// \param[in] _str A string.
      /// \param[in] _emptyHostValid Set this to true if an empty host is
      /// valid. This should only be set to true if the corresponding URIScheme
      /// is "file".
      /// \return True if the string could be parsed as a URIAuthority.
      public: bool Parse(const std::string &_str,
                  bool _emptyHostValid = false);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<URIAuthorityPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };

    /// \brief The path component of a URI
    class IGNITION_COMMON_VISIBLE URIPath
    {
      /// \brief Constructor
      public: URIPath();

      /// \brief Copy constructor.
      /// \param[in] _path Another URIPath.
      public: URIPath(const URIPath &_path);

      /// \brief Construct a URIPath object from a string.
      /// \param[in] _str A string.
      public: explicit URIPath(const std::string &_str);

      /// \brief Destructor
      public: virtual ~URIPath();

      /// \brief Remove all parts of the path
      public: void Clear();

      /// \brief Returns whether the path is absolute or not.
      /// \return Whether the path is absolute or not.
      public: bool IsAbsolute() const;

      /// \brief Set whether the path is to be treated absolute or not.
      /// \param[in] _absolute Whether the path is to be treated absolute or
      ///                      not.
      /// \note If this path starts with a Windows drive specifier (e.g. 'C:'),
      /// it cannot be set non-absolute.
      public: void SetAbsolute(bool _absolute = true);

      /// \brief Set the path to be relative.
      /// \note If this path starts with a Windows drive specifier (e.g. 'C:'),
      /// it cannot be set relative.
      public: void SetRelative();

      /// \brief Push a new part onto the front of this path.
      /// \param[in] _part Path part to push
      /// Empty _part is ignored. If _part starts with /, the path is set to
      /// absolute (though calling SetAbsolute() is the preferred method). All
      /// forward slashes inside the string are URI-encoded to %2F. The path is
      /// also set to absolute if a Windows drive specifier (e.g. 'C:') is
      /// pushed to the front.
      public: void PushFront(const std::string &_part);

      /// \brief Push a new part onto the back of this path.
      /// \param[in] _part Path part to push
      /// \sa operator/
      /// Empty _part is ignored. If _part starts with / and the path is empty,
      /// the path is set to absolute (though calling SetAbsolute() is the
      /// preferred method). All forward slashes inside the string are
      /// URI-encoded to %2F. The path is also set to absolute if it is empty
      /// and a Windows drive specifier (e.g. 'C:') is pushed to it.
      public: void PushBack(const std::string &_part);

      /// \brief Compound assignment operator.
      /// \param[in] _part A new path to append.
      /// \return A new Path that consists of "this / _part"
      public: const URIPath &operator/=(const std::string &_part);

      /// \brief Get the current path with the _part added to the end.
      /// \param[in] _part Path part.
      /// \return A new Path that consists of "this / _part"
      /// \sa PushBack
      public: const URIPath operator/(const std::string &_part) const;

      /// \brief Return true if the two paths match.
      /// \param[in] _part Path part.
      /// return True of the paths match.
      public: bool operator==(const URIPath &_path) const;

      /// \brief Get the path as a string.
      /// \param[in] _delim Delimiter used to separate each part of the path.
      /// \return The path as a string, with each path part separated by _delim.
      public: std::string Str(const std::string &_delim = "/") const;

      /// \brief Assignment operator.
      /// \param[in] _path Another URIPath.
      /// \return Itself.
      public: URIPath &operator=(const URIPath &_path);

      /// \brief Return true if the string is a valid path.
      /// \param[in] _str String to check.
      /// \return True if _str is a valid URI path.
      public: static bool Valid(const std::string &_str);

      /// \brief Return true if this is a valid path.
      /// \return True if this is a valid URI path.
      public: bool Valid() const;

      /// \brief Parse a string as URIPath.
      /// \param[in] _str A string.
      /// \return True if the string could be parsed as a URIPath.
      public: bool Parse(const std::string &_str);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<URIPathPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };

    /// \brief The query component of a URI
    class IGNITION_COMMON_VISIBLE URIQuery
    {
      /// \brief Constructor
      public: URIQuery();

      /// \brief Construct a URIQuery object from a string.
      /// \param[in] _str A string.
      public: explicit URIQuery(const std::string &_str);

      /// \brief Copy constructor
      /// \param[in] _query Another query component
      public: URIQuery(const URIQuery &_query);

      /// \brief Destructor
      public: virtual ~URIQuery();

      /// \brief Remove all values of the query
      public: void Clear();

      /// \brief Get this query with a new _key=_value pair added.
      /// \param[in] _key Key of the query.
      /// \param[in] _value Value of the query.
      public: void Insert(const std::string &_key,
                          const std::string &_value);

      /// \brief Assignment operator.
      /// \param[in] _query another URIQuery.
      /// \return Itself.
      public: URIQuery &operator=(const URIQuery &_query);

      /// \brief Return true if the two queries contain the same values.
      /// \param[in] _query A URI query to compare.
      /// return True if the queries match.
      public: bool operator==(const URIQuery &_query) const;

      /// \brief Get the query as a string.
      /// \param[in] _delim Delimiter used to separate each tuple of the query.
      /// \return The query as a string, with each key,value pair separated by
      /// _delim.
      public: std::string Str(const std::string &_delim = "&") const;

      /// \brief Check if a string is a valid URI query.
      /// \param[in] _str The string to check.
      /// \return True if the string can be parsed as a URI query.
      public: static bool Valid(const std::string &_str);

      /// \brief Check if this is a valid URI query.
      /// \return True if this can be parsed as a URI query.
      public: bool Valid() const;

      /// \brief Parse a string as URIQuery.
      /// \param[in] _str A string.
      /// \return True if the string can be parsed as a URIQuery.
      public: bool Parse(const std::string &_string);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<URIQueryPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };

    /// \brief The fragment component of a URI
    class IGNITION_COMMON_VISIBLE URIFragment
    {
      /// \brief Constructor
      public: URIFragment();

      /// \brief Construct a URIFragment object from a string.
      /// \param[in] _str A string.
      public: explicit URIFragment(const std::string &_str);

      /// \brief Copy constructor
      /// \param[in] _fragment Another fragment component
      public: URIFragment(const URIFragment &_fragment);

      /// \brief Destructor
      public: virtual ~URIFragment();

      /// \brief Remove all values of the fragment
      public: void Clear();

      /// \brief Assignment operator.
      /// \param[in] _fragment another URIFragment.
      /// \return Itself.
      public: URIFragment &operator=(const URIFragment &_fragment);

      /// \brief Assignment operator.
      /// \param[in] _fragment another URIFragment.
      /// \return Itself.
      public: URIFragment &operator=(const std::string &_fragment);

      /// \brief Return true if the two fragments contain the same values.
      /// \param[in] _fragment A URI fragment to compare.
      /// return True if the fragments match.
      public: bool operator==(const URIFragment &_fragment) const;

      /// \brief Get the fragment as a string.
      /// \return The fragment as a string.
      public: std::string Str() const;

      /// \brief Check if a string is a valid URI fragment.
      /// \param[in] _str The string to check.
      /// \return True if the string can be parsed as a URI fragment.
      public: static bool Valid(const std::string &_str);

      /// \brief Check if this is a valid URI fragment.
      /// \return True if this can be parsed as a URI fragment.
      public: bool Valid() const;

      /// \brief Parse a string as URIFragment.
      /// \param[in] _str A string.
      /// \return True if the string can be parsed as a URIFragment.
      public: bool Parse(const std::string &_string);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<URIFragmentPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };

    /// \brief A complete URI which has the following components:
    ///
    /// scheme:[//authority]path[?query][#fragment]
    ///
    // cppcheck-suppress class_X_Y
    class IGNITION_COMMON_VISIBLE URI
    {
      /// \brief Default constructor
      public: URI();

      /// \brief Construct a URI object from a string.
      /// \param[in] _str A string.
      public: explicit URI(const std::string &_str);

      /// \brief Copy constructor
      /// \param[in] _uri Another URI.
      public: URI(const URI &_uri);

      /// \brief Destructor.
      public: ~URI();

      /// \brief Get the URI as a string, which has the form:
      ///
      /// scheme:[//authority]path[?query][#fragment]
      ///
      /// \return The full URI as a string
      public: std::string Str() const;

      /// \brief Remove all components of the URI
      public: void Clear();

      /// \brief Get the URI's scheme
      /// \return The scheme
      public: std::string Scheme() const;

      /// \brief Set the URI's scheme
      /// \param[in] _scheme New scheme.
      public: void SetScheme(const std::string &_scheme);

      /// \brief Get a mutable version of the URI's authority.
      /// \return The authority
      public: URIAuthority &Authority();

      /// \brief Get a const reference of the URI's authority.
      /// \return The authority
      public: const URIAuthority &Authority() const;

      /// \brief Get a mutable version of the path component
      /// \return A reference to the path
      public: URIPath &Path();

      /// \brief Get a const reference of the path component.
      /// \return A const reference of the path.
      public: const URIPath &Path() const;

      /// \brief Get a mutable version of the query component
      /// \return A reference to the query
      public: URIQuery &Query();

      /// \brief Get a const reference of the query component.
      /// \return A const reference of the query.
      public: const URIQuery &Query() const;

      /// \brief Get a mutable version of the fragment component.
      /// \return A reference to the fragment.
      public: URIFragment &Fragment();

      /// \brief Get a const reference of the fragment component.
      /// \return A const reference of the fragment.
      public: const URIFragment &Fragment() const;

      /// \brief Assignment operator.
      /// \param[in] _uri Another URI.
      /// \return Itself.
      public: URI &operator=(const URI &_uri);

      /// \brief Return true if the two URIs match.
      /// \param[in] _uri Another URI to compare.
      /// \return True if the two URIs match.
      public: bool operator==(const URI &_uri) const;

      /// \brief Validate this URI.
      /// \return True if this can be parsed as a URI.
      public: bool Valid() const;

      /// \brief Validate a string as URI.
      /// \param[in] _str The string to validate.
      /// \return True if the string can be parsed as a URI.
      public: static bool Valid(const std::string &_str);

      /// \brief Parse a string as URI.
      /// \param[in] _str A string.
      /// \return True if the string can be parsed as a URI.
      public: bool Parse(const std::string &_str);

      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<URIPrivate> dataPtr;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}
#endif
