/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string>
#include <sstream>

#include <gz/common/Console.hh>
#include <gz/common/config.hh>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace gz;
using namespace common;


FileLogger common::Console::log("");

// On UNIX, these are ANSI-based color codes. On Windows, these are colors from
// docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences .
// They happen to overlap, but there might be differences if more colors are
// added.
const int red = 31;
const int yellow = 33;
const int green = 32;
const int blue = 36;

Logger Console::err("[Err] ", red, Logger::STDERR, 1);
Logger Console::warn("[Wrn] ", yellow, Logger::STDERR, 2);
Logger Console::msg("[Msg] ", green, Logger::STDOUT, 3);
Logger Console::dbg("[Dbg] ", blue, Logger::STDOUT, 4);

int Console::verbosity = 1;
std::string Console::customPrefix = ""; // NOLINT(*)

//////////////////////////////////////////////////
void Console::SetVerbosity(const int _level)
{
  verbosity = _level;
}

//////////////////////////////////////////////////
int Console::Verbosity()
{
  return verbosity;
}

//////////////////////////////////////////////////
void Console::SetPrefix(const std::string &_prefix)
{
  customPrefix = _prefix;
}

//////////////////////////////////////////////////
std::string Console::Prefix()
{
  return customPrefix;
}

/////////////////////////////////////////////////
Logger::Logger(const std::string &_prefix, const int _color,
               const LogType _type, const int _verbosity)
: std::ostream(new Buffer(_type, _color, _verbosity)), prefix(_prefix)
{
  this->setf(std::ios_base::unitbuf);
}

/////////////////////////////////////////////////
Logger::~Logger()
{
}

/////////////////////////////////////////////////
Logger &Logger::operator()()
{
  Console::log() << "(" << common::systemTimeIso() << ") ";
  (*this) << Console::Prefix() << this->prefix;

  return (*this);
}

/////////////////////////////////////////////////
Logger &Logger::operator()(const std::string &_file, int _line)
{
  int index = _file.find_last_of("/") + 1;

  Console::log() << "(" << common::systemTimeIso() << ") ";
  std::stringstream prefixString;
  prefixString << Console::Prefix() << this->prefix
    << "[" << _file.substr(index , _file.size() - index) << ":"
    << _line << "] ";
  (*this) << prefixString.str();

  return (*this);
}

/////////////////////////////////////////////////
Logger::Buffer::Buffer(LogType _type, const int _color, const int _verbosity)
  :  type(_type), color(_color), verbosity(_verbosity)
{
}

/////////////////////////////////////////////////
Logger::Buffer::~Buffer()
{
  this->pubsync();
}

/////////////////////////////////////////////////
std::streamsize Logger::Buffer::xsputn(const char *_char,
                                       std::streamsize _count)
{
  std::lock_guard<std::mutex> lk(this->syncMutex);
  return std::stringbuf::xsputn(_char, _count);
}

/////////////////////////////////////////////////
int Logger::Buffer::sync()
{
  std::string outstr;
  {
    std::lock_guard<std::mutex> lk(this->syncMutex);
    outstr = this->str();
  }

  // Log messages to disk
  {
    std::lock_guard<std::mutex> lk(this->syncMutex);
    Console::log << outstr;
    Console::log.flush();
  }

  // Output to terminal
  if (Console::Verbosity() >= this->verbosity && !outstr.empty())
  {
#ifndef _WIN32
    bool lastNewLine = outstr.back() == '\n';
    FILE *outstream = this->type == Logger::STDOUT ? stdout : stderr;

    if (lastNewLine)
      outstr.pop_back();

    std::stringstream ss;
    ss << "\033[1;" << this->color << "m" << outstr << "\033[0m";
    if (lastNewLine)
      ss << std::endl;

    {
      std::lock_guard<std::mutex> lk(this->syncMutex);
      fprintf(outstream, "%s", ss.str().c_str());
    }
#else
    HANDLE hConsole = CreateFileW(
      L"CONOUT$", GENERIC_WRITE|GENERIC_READ, 0, nullptr, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, nullptr);

    DWORD dwMode = 0;
    bool vtProcessing = false;
    if (GetConsoleMode(hConsole, &dwMode))
    {
      if ((dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) > 0)
      {
        vtProcessing = true;
      }
      else
      {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (SetConsoleMode(hConsole, dwMode))
          vtProcessing = true;
      }
    }

    std::ostream &outStream =
        this->type == Logger::STDOUT ? std::cout : std::cerr;

    {
      std::lock_guard<std::mutex> lk(this->syncMutex);
      if (vtProcessing)
        outStream << "\x1b[" << this->color << "m" << outstr << "\x1b[m";
      else
        outStream << outstr;
    }
#endif
  }

  {
    std::lock_guard<std::mutex> lk(this->syncMutex);
    this->str("");
  }
  return 0;
}

/////////////////////////////////////////////////
FileLogger::FileLogger(const std::string &_filename)
  : std::ostream(new Buffer(_filename)),
    logDirectory("")
{
  this->initialized = false;
  this->setf(std::ios_base::unitbuf);
}

/////////////////////////////////////////////////
FileLogger::~FileLogger()
{
  if (this->initialized && this->rdbuf())
  {
    auto *buf = dynamic_cast<FileLogger::Buffer*>(this->rdbuf());
    if (buf->stream)
    {
      delete buf->stream;
      buf->stream = nullptr;
    }
  }
}

/////////////////////////////////////////////////
void FileLogger::Init(const std::string &_directory,
                      const std::string &_filename)
{
  std::string logPath;

  if (_directory.empty() ||
#ifndef _WIN32
    _directory[0] != '/'
#else
    _directory.length() < 2 || _directory[1] != ':'
#endif
    )
  {
    if (!env(GZ_HOMEDIR, logPath))
    {
      // Use stderr here to prevent infinite recursion
      // trying to get the log initialized
      std::cerr << "Missing HOME environment variable."
        << "No log file will be generated." << std::endl;
      return;
    }
    logPath = joinPaths(logPath, _directory);
  }
  else
  {
    logPath = _directory;
  }

  auto* buf = dynamic_cast<FileLogger::Buffer*>(this->rdbuf());

  // Create the directory if it doesn't exist.
  createDirectories(logPath);

  logPath = joinPaths(logPath, _filename);

  // Check if the Init method has been already called, and if so
  // remove current buffer.
  if (buf->stream)
  {
    delete buf->stream;
    buf->stream = nullptr;
  }

  buf->stream = new std::ofstream(logPath.c_str(), std::ios::out);
  if (!buf->stream->is_open())
    std::cerr << "Error opening log file: " << logPath << std::endl;

  // Update the log directory name.
  if (isDirectory(logPath))
    this->logDirectory = logPath;
  else
    this->logDirectory = common::parentPath(logPath);

  this->initialized = true;

  /// \todo(anyone) Reimplement this.
  // Output the version of the project.
  // (*buf->stream) << PROJECT_VERSION_HEADER << std::endl;
}

/////////////////////////////////////////////////
void FileLogger::Close()
{
  auto* buf = dynamic_cast<FileLogger::Buffer*>(this->rdbuf());
  if (buf && buf->stream && buf->stream->is_open())
  {
    buf->stream->close();
    delete buf->stream;
    buf->stream = nullptr;
  }
}

/////////////////////////////////////////////////
FileLogger &FileLogger::operator()()
{
  if (!this->initialized)
    this->Init(".gz", "auto_default.log");

  (*this) << "(" << common::systemTimeIso() << ") ";
  return (*this);
}

/////////////////////////////////////////////////
FileLogger &FileLogger::operator()(const std::string &_file, int _line)
{
  if (!this->initialized)
    this->Init(".gz", "auto_default.log");

  int index = _file.find_last_of("/") + 1;
  (*this) << "(" << common::systemTimeIso() << ") ["
    << _file.substr(index , _file.size() - index) << ":" << _line << "]";

  return (*this);
}

/////////////////////////////////////////////////
std::string FileLogger::LogDirectory() const
{
  return this->logDirectory;
}

/////////////////////////////////////////////////
FileLogger::Buffer::Buffer(const std::string &_filename)
  : stream(NULL)
{
  if (!_filename.empty())
  {
    this->stream = new std::ofstream(_filename.c_str(), std::ios::out);
  }
}

/////////////////////////////////////////////////
FileLogger::Buffer::~Buffer()
{
  if (this->stream)
    static_cast<std::ofstream*>(this->stream)->close();
}

/////////////////////////////////////////////////
std::streamsize FileLogger::Buffer::xsputn(const char *_char,
                                           std::streamsize _count)
{
  std::lock_guard<std::mutex> lk(this->syncMutex);
  return std::stringbuf::xsputn(_char, _count);
}

/////////////////////////////////////////////////
int FileLogger::Buffer::sync()
{
  if (!this->stream)
    return -1;

  {
    std::lock_guard<std::mutex> lk(this->syncMutex);
    *this->stream << this->str();
  }

  {
    std::lock_guard<std::mutex> lk(this->syncMutex);
    this->stream->flush();
  }
  {
    std::lock_guard<std::mutex> lk(this->syncMutex);
    this->str("");
  }
  return !(*this->stream);
}
