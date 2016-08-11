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
#include "ignition/common/win_dirent.h"
#endif


#define LEFT_ROTATE(x, n) (((x) << (n)) ^ ((x) >> (32-(n))))

#ifdef _WIN32
# define IGN_PATH_MAX _MAX_PATH
#elif defined(PATH_MAX)
# define IGN_PATH_MAX PATH_MAX
#elif defined(_XOPEN_PATH_MAX)
# define IGN_PATH_MAX _XOPEN_PATH_MAX
#else
# define IGN_PATH_MAX _POSIX_PATH_MAX
#endif

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
  _hash = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};

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
bool ignition::common::isDirectory(const std::string &_path)
{
  struct stat info;

  if (stat(_path.c_str(), &info) != 0)
    return false;
  else if (info.st_mode & S_IFDIR)
    return true;
  else
    return false;
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
  v = std::getenv(_name.c_str());
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
bool ignition::common::isFile(const std::string &_path)
{
  std::ifstream f(_path);
  return f.good();
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectory(const std::string &_path)
{
  if (ignition::common::isDirectory(_path))
  {
#ifdef _WIN32
    return RemoveDirectory(_path.c_str());
#else
    return rmdir(_path.c_str()) == 0;
#endif
  }

  return false;
}

/////////////////////////////////////////////////
bool ignition::common::removeDirectoryOrFile(const std::string &_path)
{
  if (ignition::common::isDirectory(_path))
  {
    return ignition::common::removeDirectory(_path);
  }
  else if (ignition::common::isFile(_path))
  {
    return std::remove(_path.c_str()) == 0;
  }
  return false;
}

/////////////////////////////////////////////////
bool ignition::common::removeAll(const std::string &_path)
{
  if (ignition::common::isDirectory(_path))
  {
    DIR *dir = opendir(_path.c_str());
    if (dir)
    {
      struct dirent *p;
      while (p=readdir(dir))
      {
        // Skip special files.
        if (!std::strcmp(p->d_name, ".") || !std::strcmp(p->d_name, ".."))
          continue;

        ignition::common::removeAll(_path + "/" + p->d_name);
      }
    }
    closedir(dir);
  }

  return ignition::common::removeDirectoryOrFile(_path);
}

/////////////////////////////////////////////////
bool ignition::common::exists(const std::string &_path)
{
  return ignition::common::isFile(_path) ||
         ignition::common::isDirectory(_path);
}

/////////////////////////////////////////////////
bool ignition::common::moveFile(const std::string &_existingFilename,
                                const std::string &_newFilename)
{
  return copyFile(_existingFilename, _newFilename) &&
         std::remove(_existingFilename.c_str()) == 0;
}

/////////////////////////////////////////////////
std::string ignition::common::absPath(const std::string &_path)
{
  std::string result;

  char path[IGN_PATH_MAX] = "";
#ifdef _WIN32
  if (GetFullPathName(_path.c_str(), IGN_PATH_MAX, &path[0], nullptr) != 0)
#else
  if (realpath(_path.c_str(), &path[0]) != nullptr)
#endif
    result = path;
  else if (!_path.empty())
  {
    // If _path is an absolute path, then return _path.
    // An absoluate path on Windows is a character followed by a colon and a
    // forward-slash.
    if (_path.compare(0, 1, "/") == 0 || _path.compare(1, 3, ":\\") == 0)
      result = _path;
    // Otherwise return the current working directory with _path appended.
    else
      result = ignition::common::cwd() + "/" + _path;
  }

  ignition::common::replaceAll(result, result, "//", "/");

  return result;
}

/////////////////////////////////////////////////
bool ignition::common::copyFile(const std::string &_existingFilename,
                                const std::string &_newFilename)
{
  std::string absExistingFilename =
    ignition::common::absPath(_existingFilename);
  std::string absNewFilename = ignition::common::absPath(_newFilename);

  if (absExistingFilename == absNewFilename)
    return false;

#ifdef _WIN32
  return CopyFile(absExistingFilename.c_str(), absNewFilename.c_str(), false);
#elif defined(__APPLE__)
  bool result = false;
  std::ifstream in(absExistingFilename.c_str(), std::ifstream::binary);

  if (in.good())
  {
    std::ofstream out(absNewFilename.c_str(),
                      std::ifstream::trunc | std::ifstream::binary);
    if (out.good())
    {
      out << in.rdbuf();
      result = ignition::common::isFile(absNewFilename);
    }
    out.close();
  }
  in.close();

  return result;
#else
  int readFd = 0;
  int writeFd = 0;
  struct stat statBuf;
  off_t offset = 0;

  // Open the input file.
  readFd = open(absExistingFilename.c_str(), O_RDONLY);
  if (readFd < 0)
    return false;

  // Stat the input file to obtain its size.
  fstat(readFd, &statBuf);

  // Open the output file for writing, with the same permissions as the
  // source file.
  writeFd = open(absNewFilename.c_str(), O_WRONLY | O_CREAT, statBuf.st_mode);

  while (offset >= 0 && offset < statBuf.st_size)
  {
    // Send the bytes from one file to the other.
    ssize_t written = sendfile(writeFd, readFd, &offset, statBuf.st_size);
    if (written < 0)
      break;
  }

  close(readFd);
  close(writeFd);

  return offset == statBuf.st_size;
#endif
}

/////////////////////////////////////////////////
std::string ignition::common::cwd()
{
  char buf[IGN_PATH_MAX + 1] = {'\0'};
#ifdef _WIN32
  return _getcwd(buf, sizeof(buf)) == nullptr ? "" : buf;
#else
  return getcwd(buf, sizeof(buf)) == nullptr ? "" : buf;
#endif
}

/////////////////////////////////////////////////
bool ignition::common::createDirectories(const std::string &_path)
{
  size_t index = 0;
  while (index < _path.size())
  {
    size_t end = _path.find('/', index+1);
    std::string dir = _path.substr(0, end);
    if (!exists(dir))
    {
#ifdef _WIN32
      _mkdir(dir.c_str());
#else
      mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }
    index = end;
  }

  return true;
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


