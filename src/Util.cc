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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

#include <iomanip>
#include <array>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include <ignition/common/config.hh>
#include <ignition/common/SystemPaths.hh>
#include <ignition/common/Util.hh>
#include <ignition/common/Uuid.hh>

#ifndef _WIN32
#include <dirent.h>
#include <limits.h>
#include <climits>
#else
#include <io.h>
#include "win_dirent.h"
#endif


#define LEFT_ROTATE(x, n) (((x) << (n)) ^ ((x) >> (32-(n))))


#ifdef _WIN32
  const auto &ignstrtok = strtok_s;
#else
  const auto &ignstrtok = strtok_r;
#endif

static std::unique_ptr<ignition::common::SystemPaths> gSystemPaths(
    new ignition::common::SystemPaths);

/////////////////////////////////////////////////
// Internal class for SHA1 computation
class Sha1
{
  /// \brief Constructor
  public: Sha1() = default;

  /// \brief Get the SHA1 digest
  /// \param[in] _digest Digest types
  public: bool Digest(void const *_buffer, std::size_t _byteCount,
                      std::array<unsigned int, 5> &_digest);

  /// \brief Helper function to process a byte
  /// \param[in] _byte Byte to process
  private: void Byte(std::array<unsigned int, 5> &_hash,
               const unsigned char _byte);

  /// \brief Byte block
  private: std::array<unsigned char, 64> block;

  /// \brief Byte block index
  private: std::size_t blockByteIndex = 0;

  /// \brief Bit low count
  private: std::size_t bitCountLow = 0;

  /// \brief Bit high count
  private: std::size_t bitCountHigh = 0;
};

/////////////////////////////////////////////////
void Sha1::Byte(std::array<unsigned int, 5> &_hash, const unsigned char _byte)
{
  this->block[this->blockByteIndex++] = _byte;

  if (this->blockByteIndex == 64)
  {
    unsigned int w[80];
    this->blockByteIndex = 0;

    for (std::size_t i = 0; i < 16; ++i)
    {
      w[i] = (this->block[i*4 + 0] << 24);
      w[i] |= (this->block[i*4 + 1] << 16);
      w[i] |= (this->block[i*4 + 2] << 8);
      w[i] |= this->block[i*4 + 3];
    }

    for (std::size_t i = 16; i < 80; ++i)
    {
      w[i] = LEFT_ROTATE((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);
    }

    auto a = _hash[0];
    auto b = _hash[1];
    auto c = _hash[2];
    auto d = _hash[3];
    auto e = _hash[4];

    for (std::size_t i = 0; i < 80; ++i)
    {
      unsigned int f;
      unsigned int k;

      if (i < 20)
      {
        f = (b & c) | (~b & d);
        k = 0x5A827999;
      }
      else if (i < 40)
      {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1;
      }
      else if (i < 60)
      {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDC;
      }
      else
      {
        f = b ^ c ^ d;
        k = 0xCA62C1D6;
      }

      auto temp = LEFT_ROTATE(a, 5) + f + e + k + w[i];
      e = d;
      d = c;
      c = LEFT_ROTATE(b, 30);
      b = a;
      a = temp;
    }

    _hash[0] += a;
    _hash[1] += b;
    _hash[2] += c;
    _hash[3] += d;
    _hash[4] += e;
  }
}

/////////////////////////////////////////////////
bool Sha1::Digest(void const *_buffer, std::size_t _byteCount,
    std::array<unsigned int, 5> &_hash)
{
  _hash = {{0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0}};

  auto start = static_cast<unsigned char const*>(_buffer);
  auto end = static_cast<unsigned char const*>(start + _byteCount);

  for (; start != end; ++start)
  {
    this->Byte(_hash, *start);

    // size_t max value = 0xFFFFFFFF
    if (this->bitCountLow < 0xFFFFFFF8)
    {
      this->bitCountLow += 8;
    }
    else
    {
      this->bitCountLow = 0;

      if (this->bitCountHigh <= 0xFFFFFFFE)
      {
        ++this->bitCountHigh;
      }
      else
      {
        // Too many bytes
        return false;
      }
    }
  }

  // append the bit '1' to the message
  this->Byte(_hash, 0x80);

  // append k bits '0', where k is the minimum number >= 0
  // such that the resulting message length is congruent to 56 (mod 64)
  // check if there is enough space for padding and bit_count
  if (this->blockByteIndex > 56)
  {
    // finish this block
    while (this->blockByteIndex != 0)
      this->Byte(_hash, 0);

    // one more block
    while (this->blockByteIndex < 56)
      this->Byte(_hash, 0);
  }
  else
  {
    while (this->blockByteIndex < 56)
      this->Byte(_hash, 0);
  }

  // append length of message (before pre-processing)
  // as a 64-bit big-endian integer
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountHigh>>24) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountHigh>>16) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountHigh>>8) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountHigh) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountLow>>24) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountLow>>16) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountLow>>8) & 0xFF));
  this->Byte(_hash,
      static_cast<unsigned char>((this->bitCountLow) & 0xFF));

  return true;
}

/////////////////////////////////////////////////
std::string ignition::common::systemTimeISO()
{
  char isoStr[25];

  auto epoch = IGN_SYSTEM_TIME().time_since_epoch();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
  auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
      epoch).count() - sec * IGN_SEC_TO_NANO;

  time_t tmSec = static_cast<time_t>(sec);
  std::strftime(isoStr, sizeof(isoStr), "%FT%T", std::localtime(&tmSec));

  return std::string(isoStr) + "." + std::to_string(nano);
}

/////////////////////////////////////////////////
std::string ignition::common::logPath()
{
  return gSystemPaths->LogPath();
}

/////////////////////////////////////////////////
void ignition::common::addSearchPathSuffix(const std::string &_suffix)
{
  gSystemPaths->AddSearchPathSuffix(_suffix);
}

/////////////////////////////////////////////////
std::string ignition::common::findFile(const std::string &_file)
{
  return gSystemPaths->FindFile(_file, true);
}

/////////////////////////////////////////////////
std::string ignition::common::findFile(const std::string &_file,
                                       const bool _searchLocalPath)
{
  return gSystemPaths->FindFile(_file, _searchLocalPath);
}

/////////////////////////////////////////////////
std::string ignition::common::findFilePath(const std::string &_file)
{
  std::string filepath = findFile(_file);

  if (common::isDirectory(filepath))
  {
    return filepath;
  }
  else
  {
    int index = filepath.find_last_of("/");
    return filepath.substr(0, index);
  }
}

/////////////////////////////////////////////////
bool ignition::common::env(const std::string &_name, std::string &_value)
{
  std::string v;
#ifdef _WIN32
  const DWORD buffSize = 32767;
  static char buffer[buffSize];
  if (GetEnvironmentVariable(_name.c_str(), buffer, buffSize))
  {
    v = buffer;
  }
#else
  const char *cvar = std::getenv(_name.c_str());
  if (cvar)
    v = cvar;
#endif
  if (!v.empty())
  {
    _value = v;
    return true;
  }
  return false;
}

/////////////////////////////////////////////////
std::string ignition::common::sha1(void const *_buffer, std::size_t _byteCount)
{
  Sha1 sha1;
  std::stringstream stream;

  std::array<unsigned int, 5> hash;
  if (sha1.Digest(_buffer, _byteCount, hash))
  {
    for (auto const h : hash)
      stream << std::setfill('0') << std::setw(sizeof(h) * 2) << std::hex << h;
  }

  return stream.str();
}

/////////////////////////////////////////////////
std::string ignition::common::uuid()
{
  ignition::common::Uuid uuid;
  return uuid.String();
}

/////////////////////////////////////////////////
std::vector<std::string> ignition::common::split(const std::string &_str,
    const std::string &_delim)
{
  std::vector<std::string> tokens;
  char *saveptr;
  char *str = strdup(_str.c_str());

  auto token = ignstrtok(str, _delim.c_str(), &saveptr);

  while (token)
  {
    tokens.push_back(token);
    token = ignstrtok(NULL, _delim.c_str(), &saveptr);
  }

  free(str);
  return tokens;
}

/////////////////////////////////////////////////
void ignition::common::ltrim(std::string &_s)
{
  _s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(),
        std::not1(std::ptr_fun<int, int>(std::isspace))));
}

/////////////////////////////////////////////////
void ignition::common::rtrim(std::string &_s)
{
  _s.erase(std::find_if(_s.rbegin(), _s.rend(),
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
      _s.end());
}

/////////////////////////////////////////////////
void ignition::common::trim(std::string &_s)
{
  ignition::common::ltrim(_s);
  ignition::common::rtrim(_s);
}

/////////////////////////////////////////////////
std::string ignition::common::ltrimmed(std::string _s)
{
  ignition::common::ltrim(_s);
  return _s;
}

/////////////////////////////////////////////////
std::string ignition::common::rtrimmed(std::string _s)
{
  ignition::common::rtrim(_s);
  return _s;
}

/////////////////////////////////////////////////
std::string ignition::common::trimmed(std::string _s)
{
  ignition::common::trim(_s);
  return _s;
}

/////////////////////////////////////////////////
std::string ignition::common::lowercase(const std::string &_in)
{
  std::string out = _in;
  std::transform(out.begin(), out.end(), out.begin(), ::tolower);
  return out;
}

/////////////////////////////////////////////////
std::string ignition::common::lowercase(const char *_in)
{
  std::string ins = _in;
  return lowercase(ins);
}

/////////////////////////////////////////////////
void ignition::common::replaceAll(std::string &_result,
                                  const std::string &_orig,
                                  const std::string &_key,
                                  const std::string &_replacement)
{
  _result = _orig;
  size_t pos = 0;
  while ((pos = _result.find(_key, pos)) != std::string::npos)
  {
    _result = _result.replace(pos, _key.length(), _replacement);
    pos += _key.length() > _replacement.length() ? 0 : _replacement.length();
  }
}

/////////////////////////////////////////////////
std::string ignition::common::replaceAll(const std::string &_orig,
                                        const std::string &_key,
                                        const std::string &_replacement)
{
  std::string result;
  replaceAll(result, _orig, _key, _replacement);
  return result;
}
