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

#include <algorithm>
#include <list>
#include <map>
#include <string>

#include "ignition/common/Console.hh"
#include "ignition/common/URI.hh"

using namespace ignition;
using namespace common;

static const std::string kSchemeDelim = "://";

/// \brief URIPath private data.
class ignition::common::URIPathPrivate
{
  /// \brief The parts of the path.
  public: std::list<std::string> path;

  /// \brief Whether the path is absolute (starts with slash) or not.
  public: bool isAbsolute = false;

  /// \brief A helper method to determine if the given string represents
  ///        an absolute path starting segment or not.
  public: bool IsStringAbsolute(const std::string &_path)
  {
    return _path.length() > 0 && _path[0] == '/';
  }
};

/// \brief URIQuery private data.
class ignition::common::URIQueryPrivate
{
  /// \brief The key/value tuples that compose the query.
  public: std::map<std::string, std::string> values;
};

/// \brief URIFragment private data.
class ignition::common::URIFragmentPrivate
{
  /// \brief The value of the fragment.
  public: std::string value;
};

/// \brief URI private data.
class ignition::common::URIPrivate
{
  /// \brief The URI scheme.
  public: std::string scheme;

  /// \brief Path component.
  public: URIPath path;

  /// \brief Query component.
  public: URIQuery query;

  /// \brief Fragment component.
  public: URIFragment fragment;
};

/////////////////////////////////////////////////
URIPath::URIPath()
: dataPtr(new URIPathPrivate())
{
}

/////////////////////////////////////////////////
URIPath::~URIPath()
{
}

/////////////////////////////////////////////////
URIPath::URIPath(const std::string &_str)
  : URIPath()
{
  if (!this->Parse(_str))
  {
    ignwarn << "Unable to parse URIPath [" << _str << "]. Ignoring."
           << std::endl;
  }
}

/////////////////////////////////////////////////
URIPath::URIPath(const URIPath &_path)
  : URIPath()
{
  *this = _path;
}

/////////////////////////////////////////////////
bool URIPath::IsAbsolute() const
{
  return this->dataPtr->isAbsolute;
}

/////////////////////////////////////////////////
void URIPath::SetAbsolute(const bool _absolute)
{
  this->dataPtr->isAbsolute = _absolute;
}

/////////////////////////////////////////////////
void URIPath::SetRelative()
{
  this->SetAbsolute(false);
}

/////////////////////////////////////////////////
void URIPath::PushFront(const std::string &_part)
{
  if (_part.empty())
  {
    ignwarn << "Adding empty path segment to URI " << this->Str()
            << " has no effect." << std::endl;
    return;
  }

  auto part = _part;
  if (_part[0] == '/')
  {
    ignwarn << "Instead of pushing a string starting with slash, call "
               "SetAbsolute() instead." << std::endl;
    part = _part.substr(1);
    this->SetAbsolute();
  }

  if (part.find('/') != std::string::npos)
  {
    // TODO: Once URI encoding is implemented, all invalid characters should be
    // encoded, not just slashes.
    ignwarn << "Unencoded slashes in URI part, encoding them." << std::endl;
    part = common::replaceAll(part, "/", "%2F");
  }

  if (!part.empty())
    this->dataPtr->path.push_front(part);
}

/////////////////////////////////////////////////
void URIPath::PushBack(const std::string &_part)
{
  if (_part.empty())
  {
    ignwarn << "Adding empty path segment to URI " << this->Str()
            << " has no effect." << std::endl;
    return;
  }

  auto part = _part;
  if (this->dataPtr->path.size() == 0 && _part[0] == '/')
  {
    ignwarn << "Instead of pushing a string starting with slash, call "
               "SetAbsolute() instead." << std::endl;
    part = _part.substr(1);
    this->SetAbsolute();
  }

  if (part.find('/') != std::string::npos)
  {
    // TODO: Once URI encoding is implemented, all invalid characters should be
    // encoded, not just slashes.
    ignwarn << "Unencoded slashes in URI part, encoding them." << std::endl;
    part = common::replaceAll(part, "/", "%2F");
  }

  this->dataPtr->path.push_back(part);
}

/////////////////////////////////////////////////
const URIPath URIPath::operator/(const std::string &_part) const
{
  URIPath result = *this;
  result /= _part;
  return result;
}

/////////////////////////////////////////////////
const URIPath &URIPath::operator/=(const std::string &_part)
{
  this->PushBack(_part);
  return *this;
}

/////////////////////////////////////////////////
std::string URIPath::Str(const std::string &_delim) const
{
  std::string result(this->dataPtr->isAbsolute ? "/" : "");
  bool firstPart = true;
  for (auto const &part : this->dataPtr->path)
  {
    if (firstPart)
    {
      firstPart = false;
    }
    else
    {
      result += _delim;
    }
    result += part;
  }

  return result;
}

/////////////////////////////////////////////////
URIPath &URIPath::operator=(const URIPath &_path)
{
  this->dataPtr->path = _path.dataPtr->path;
  this->dataPtr->isAbsolute = _path.dataPtr->isAbsolute;
  return *this;
}

/////////////////////////////////////////////////
void URIPath::Clear()
{
  this->dataPtr->path.clear();
  this->dataPtr->isAbsolute = false;
}

/////////////////////////////////////////////////
bool URIPath::operator==(const URIPath &_path) const
{
  return this->dataPtr->path == _path.dataPtr->path &&
         this->dataPtr->isAbsolute == _path.dataPtr->isAbsolute;
}

/////////////////////////////////////////////////
bool URIPath::Valid() const
{
  return this->Valid(this->Str());
}

/////////////////////////////////////////////////
bool URIPath::Valid(const std::string &_str)
{
  auto str = trimmed(_str);
  size_t slashCount = std::count(str.begin(), str.end(), '/');
  if ((str.empty()) ||
      (slashCount == str.size() && str.size() > 1))
  {
    return false;
  }

  // TODO: the space should not be
  // there, but leaving it out breaks
  // other stuff, e.g. ign-fuel-tools
  // now have URIs with unencoded spaces
  const std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                                   "QWERTZUIOPASDFGHJKLYXCVBNM"
                                   "0123456789"
                                   "/"
                                   ":@"
                                   "%"
                                   "-._~"
                                   "!$&'()*+,;="
                                   "[] ";
  if (str.find_first_not_of(allowedChars) != std::string::npos)
    return false;

  const std::string allowedCharsFirst = "qwertzuiopasdfghjklyxcvbnm"
                                        "QWERTZUIOPASDFGHJKLYXCVBNM"
                                        "0123456789"
                                        ":"
                                        "%"
                                        // "-._~"  // is in RFC, weird
                                        // "!$&'()*+,;=" // is in RFC, weird
                                        "[/";
  if (str.substr(0, 1).find_first_not_of(allowedCharsFirst) !=
      std::string::npos)
    return false;

  // Two consecutive slashes are not valid
  if (str.find("//") != std::string::npos)
    return false;

  return true;
}

/////////////////////////////////////////////////
bool URIPath::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  this->Clear();

  for (auto part : common::split(_str, "/"))
    this->PushBack(part);

  // the initial / is removed from _str when splitting, so we need to
  // explicitly check for it
  this->dataPtr->isAbsolute = this->dataPtr->IsStringAbsolute(_str);

  return true;
}

/////////////////////////////////////////////////
URIQuery::URIQuery()
  : dataPtr(new URIQueryPrivate())
{
}

/////////////////////////////////////////////////
URIQuery::URIQuery(const std::string &_str)
  : URIQuery()
{
  if (!this->Parse(_str))
  {
    ignwarn << "Unable to parse URIQuery [" << _str << "]. Ignoring."
           << std::endl;
  }
}

/////////////////////////////////////////////////
URIQuery::URIQuery(const URIQuery &_query)
  : URIQuery()
{
  *this = _query;
}

/////////////////////////////////////////////////
URIQuery::~URIQuery()
{
}

/////////////////////////////////////////////////
void URIQuery::Insert(const std::string &_key, const std::string &_value)
{
  this->dataPtr->values.insert(std::make_pair(_key, _value));
}

/////////////////////////////////////////////////
URIQuery &URIQuery::operator=(const URIQuery &_query)
{
  this->dataPtr->values = _query.dataPtr->values;
  return *this;
}

/////////////////////////////////////////////////
std::string URIQuery::Str(const std::string &_delim) const
{
  if (this->dataPtr->values.empty())
      return "";

  std::string result = "?";
  for (auto const &value : this->dataPtr->values)
  {
    if (result != "?")
      result += _delim;
    result += value.first + "=" + value.second;
  }

  return result;
}

/////////////////////////////////////////////////
void URIQuery::Clear()
{
  this->dataPtr->values.clear();
}

/////////////////////////////////////////////////
bool URIQuery::operator==(const URIQuery &_query) const
{
  return this->Str() == _query.Str();
}

/////////////////////////////////////////////////
bool URIQuery::Valid() const
{
  return this->Valid(this->Str());
}

/////////////////////////////////////////////////
bool URIQuery::Valid(const std::string &_str)
{
  auto str = trimmed(_str);
  if (str.empty())
    return true;

  if (str[0] != '?')
    return false;

  // ABNF for query from RFC3986:
  // query         = *( pchar / "/" / "?" )
  // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
  // pct-encoded   = "%" HEXDIG HEXDIG
  // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
  //                 / "*" / "+" / "," / ";" / "="

  std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                             "QWERTZUIOPASDFGHJKLYXCVBNM"
                             "0123456789"
                             "/?"
                             ":@"
                             "%"
                             "-._~"
                             "!$&'()*+,;=";
  if (str.find_first_not_of(allowedChars, 1) != std::string::npos)
    return false;

  for (auto const &query : common::split(str.substr(1), "&"))
  {
    if (common::split(query, "=").size() != 2u)
      return false;
  }

  return true;
}

/////////////////////////////////////////////////
bool URIQuery::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  this->Clear();

  if (!_str.empty())
  {
    for (auto query : common::split(_str.substr(1), "&"))
    {
      auto values = common::split(query, "=");
      this->Insert(values.at(0), values.at(1));
    }
  }

  return true;
}

/////////////////////////////////////////////////
URIFragment::URIFragment()
  : dataPtr(new URIFragmentPrivate())
{
}

/////////////////////////////////////////////////
URIFragment::URIFragment(const std::string &_str)
  : URIFragment()
{
  if (!this->Parse(_str))
  {
    ignwarn << "Unable to parse URIFragment [" << _str << "]. Ignoring."
           << std::endl;
  }
}

/////////////////////////////////////////////////
URIFragment::URIFragment(const URIFragment &_fragment)
  : URIFragment()
{
  *this = _fragment;
}

/////////////////////////////////////////////////
URIFragment::~URIFragment()
{
}

/////////////////////////////////////////////////
URIFragment &URIFragment::operator=(const URIFragment &_fragment)
{
  this->dataPtr->value = _fragment.dataPtr->value;
  return *this;
}

/////////////////////////////////////////////////
URIFragment &URIFragment::operator=(const std::string &_fragment)
{
  this->Parse(_fragment);
  return *this;
}

/////////////////////////////////////////////////
std::string URIFragment::Str() const
{
  if (this->dataPtr->value.empty())
    return "";
  return "#" + this->dataPtr->value;
}

/////////////////////////////////////////////////
void URIFragment::Clear()
{
  this->dataPtr->value.clear();
}

/////////////////////////////////////////////////
bool URIFragment::operator==(const URIFragment &_fragment) const
{
  return this->Str() == _fragment.Str();
}

/////////////////////////////////////////////////
bool URIFragment::Valid() const
{
  return URIFragment::Valid(this->Str());
}

/////////////////////////////////////////////////
bool URIFragment::Valid(const std::string &_str)
{
  auto str = trimmed(_str);
  if (str.empty())
    return true;

  if (str[0] != '#')
    return false;

  // ABNF for fragment from RFC3986:
  // fragment      = *( pchar / "/" / "?" )
  // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
  // pct-encoded   = "%" HEXDIG HEXDIG
  // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
  //                 / "*" / "+" / "," / ";" / "="

  std::string allowedChars = "qwertzuiopasdfghjklyxcvbnm"
                             "QWERTZUIOPASDFGHJKLYXCVBNM"
                             "0123456789"
                             "/?"
                             ":@"
                             "%"
                             "-._~"
                             "!$&'()*+,;=";
  if (str.find_first_not_of(allowedChars, 1) != std::string::npos)
    return false;

  return true;
}

/////////////////////////////////////////////////
bool URIFragment::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  this->Clear();

  if (!_str.empty())
  {
    this->dataPtr->value = _str.substr(1);
  }

  return true;
}

/////////////////////////////////////////////////
URI::URI()
  : dataPtr(new URIPrivate())
{
}

/////////////////////////////////////////////////
URI::URI(const std::string &_str)
  : URI()
{
  if (!this->Parse(_str))
    ignwarn << "Unable to parse URI [" << _str << "]. Ignoring." << std::endl;
}

/////////////////////////////////////////////////
URI::URI(const URI &_uri)
  : URI()
{
  *this = _uri;
}

/////////////////////////////////////////////////
URI::~URI()
{
}

//////////////////////////////////////////////////
std::string URI::Str() const
{
  std::string result =
    this->dataPtr->scheme.empty() ? "" : this->dataPtr->scheme + "://";
  result += this->dataPtr->path.Str() + this->dataPtr->query.Str() +
      this->dataPtr->fragment.Str();
  return result;
}

/////////////////////////////////////////////////
std::string URI::Scheme() const
{
  return this->dataPtr->scheme;
}

/////////////////////////////////////////////////
void URI::SetScheme(const std::string &_scheme)
{
  this->dataPtr->scheme = _scheme;
}

/////////////////////////////////////////////////
URIPath &URI::Path()
{
  return this->dataPtr->path;
}

/////////////////////////////////////////////////
const URIPath &URI::Path() const
{
  return this->dataPtr->path;
}

/////////////////////////////////////////////////
URIQuery &URI::Query()
{
  return this->dataPtr->query;
}

/////////////////////////////////////////////////
const URIQuery &URI::Query() const
{
  return this->dataPtr->query;
}

/////////////////////////////////////////////////
URIFragment &URI::Fragment()
{
  return this->dataPtr->fragment;
}

/////////////////////////////////////////////////
const URIFragment &URI::Fragment() const
{
  return this->dataPtr->fragment;
}

/////////////////////////////////////////////////
void URI::Clear()
{
  this->dataPtr->scheme.clear();
  this->dataPtr->path.Clear();
  this->dataPtr->query.Clear();
  this->dataPtr->fragment.Clear();
}

/////////////////////////////////////////////////
bool URI::operator==(const URI &_uri) const
{
  return this->dataPtr->scheme == _uri.dataPtr->scheme &&
         this->dataPtr->path == _uri.dataPtr->path &&
         this->dataPtr->query == _uri.dataPtr->query &&
         this->dataPtr->fragment == _uri.dataPtr->fragment;
}

/////////////////////////////////////////////////
URI &URI::operator=(const URI &_uri)
{
  this->dataPtr->scheme = _uri.dataPtr->scheme;
  this->dataPtr->path = _uri.dataPtr->path;
  this->dataPtr->query = _uri.dataPtr->query;
  this->dataPtr->fragment = _uri.dataPtr->fragment;
  return *this;
}

/////////////////////////////////////////////////
bool URI::Valid() const
{
  return this->Valid(this->Str());
}

/////////////////////////////////////////////////
bool URI::Valid(const std::string &_str)
{
  auto str = trimmed(_str);
  // Validate scheme.
  auto schemeDelimPos = str.find(kSchemeDelim);
  if ((str.empty()) ||
      (schemeDelimPos == std::string::npos) ||
      (schemeDelimPos == 0u))
  {
    return false;
  }

  auto from = schemeDelimPos + kSchemeDelim.size();
  std::string localPath = str.substr(from);
  std::string localQuery;
  std::string localFragment;

  auto to = str.find('?', from);
  if (to != std::string::npos)
  {
    // Update path.
    localPath = str.substr(from, to - from);

    // Update the query.
    localQuery = str.substr(to);
  }

  auto to2 = str.find('#', to);
  if (to2 != std::string::npos)
  {
    // Update query.
    localQuery = str.substr(to, to2 - to);

    // Update the fragment.
    localFragment = str.substr(to2);
  }

  // Validate the path and query.
  return URIPath::Valid(localPath) && URIQuery::Valid(localQuery) &&
         URIFragment::Valid(localFragment);
}

/////////////////////////////////////////////////
bool URI::Parse(const std::string &_str)
{
  if (!this->Valid(_str))
    return false;

  auto schemeDelimPos = _str.find(kSchemeDelim);
  auto from = schemeDelimPos + kSchemeDelim.size();
  std::string localScheme = _str.substr(0, schemeDelimPos);
  std::string localPath = _str.substr(from);
  std::string localQuery;
  std::string localFragment;

  auto to = _str.find('?', from);
  if (to != std::string::npos)
  {
    // Update path.
    localPath = _str.substr(from, to - from);

    // Update the query.
    localQuery = _str.substr(to);
  }

  auto to2 = _str.find('#', to);
  if (to2 != std::string::npos)
  {
    // Update query.
    localQuery = _str.substr(to, to2 - to);

    // Update the fragment.
    localFragment = _str.substr(to2);
  }

  this->Clear();
  this->SetScheme(localScheme);

  return this->dataPtr->path.Parse(localPath) &&
         this->dataPtr->query.Parse(localQuery) &&
         this->dataPtr->fragment.Parse(localFragment);
}
