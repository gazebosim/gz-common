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

#include <ignition/common/Console.hh>
#include <ignition/common/config.hh>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace ignition;
using namespace common;

std::string customPrefix;

FileLogger ignition::common::Console::log("");

#ifdef _WIN32
  // These are Windows-based color codes
  // (yellow is not enumerated by Windows)
  const int red = FOREGROUND_RED | FOREGROUND_INTENSITY;
  const int yellow = 0x006 | FOREGROUND_INTENSITY;
  const int green = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  const int blue = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
#else
  // These are ANSI-based color codes
  const int red = 31;
  const int yellow = 33;
  const int green = 32;
  const int blue = 36;
#endif

Logger Console::err("[Err] ", red, Logger::STDERR, 1);
Logger Console::warn("[Wrn] ", yellow, Logger::STDERR, 2);
Logger Console::msg("[Msg] ", green, Logger::STDOUT, 3);
Logger Console::dbg("[Dbg] ", blue, Logger::STDOUT, 4);

int Console::verbosity = 1;

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
  delete this->rdbuf();
}

/////////////////////////////////////////////////
Logger &Logger::operator()()
{
  Console::log << "(" << ignition::common::systemTimeISO() << ") ";
  (*this) << Console::Prefix() << this->prefix;

  return (*this);
}

/////////////////////////////////////////////////
Logger &Logger::operator()(const std::string &_file, int _line)
{
  int index = _file.find_last_of("/") + 1;

  Console::log << "(" << IGN_SYSTEM_TIME_NS() << ") ";
  (*this) << Console::Prefix() << this->prefix
    << "[" << _file.substr(index , _file.size() - index) << ":"
    << _line << "] ";

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
int Logger::Buffer::sync()
{
  std::string outstr = this->str();

  // Log messages to disk
  Console::log << outstr;
  Console::log.flush();

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

    fprintf(outstream, "%s", ss.str().c_str());
#else
    HANDLE hConsole = GetStdHandle(
          this->type == Logger::STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO originalBufferInfo;
    GetConsoleScreenBufferInfo(hConsole, &originalBufferInfo);

    SetConsoleTextAttribute(hConsole, this->color);

    std::ostream &outStream =
        this->type == Logger::STDOUT ? std::cout : std::cerr;

    outStream << outstr;

    SetConsoleTextAttribute(hConsole, originalBufferInfo.wAttributes);
#endif
  }

  this->str("");
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
  delete this->rdbuf();
}

/////////////////////////////////////////////////
void FileLogger::Init(const std::string &_directory,
                      const std::string &_filename)
{
  std::string logPath;

  if (!env(IGN_HOMEDIR, logPath))
  {
    ignerr << "Missing HOME environment variable."
           << "No log file will be generated.";
    return;
  }

  FileLogger::Buffer *buf = static_cast<FileLogger::Buffer*>(
      this->rdbuf());

  logPath = logPath + "/" + _directory;

  // Create the directory if it doesn't exist.
  // \todo: Replace this with c++1y, when it is released.
  if (!exists(logPath))
    createDirectories(logPath);

  logPath = logPath + "/" + _filename;

  // Check if the Init method has been already called, and if so
  // remove current buffer.
  if (buf->stream)
    delete buf->stream;

  buf->stream = new std::ofstream(logPath.c_str(), std::ios::out);
  if (!buf->stream->is_open())
    std::cerr << "Error opening log file: " << logPath << std::endl;

  // Update the log directory name.
  if (isDirectory(logPath))
    this->logDirectory = logPath;
  else
    this->logDirectory = logPath.substr(0, logPath.rfind('/'));

  this->initialized = true;

  /// \todo Reimplement this.
  // Output the version of the project.
  // (*buf->stream) << PROJECT_VERSION_HEADER << std::endl;
}

/////////////////////////////////////////////////
FileLogger &FileLogger::operator()()
{
  if (!this->initialized)
    this->Init(".ignition", "auto_default.log");

  (*this) << "(" << IGN_SYSTEM_TIME_NS() << ") ";
  return (*this);
}

/////////////////////////////////////////////////
FileLogger &FileLogger::operator()(const std::string &_file, int _line)
{
  if (!this->initialized)
    this->Init(".ignition", "auto_default.log");

  int index = _file.find_last_of("/") + 1;
  (*this) << "(" << IGN_SYSTEM_TIME_NS() << ") ["
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
int FileLogger::Buffer::sync()
{
  if (!this->stream)
    return -1;

  *this->stream << this->str();

  this->stream->flush();

  this->str("");
  return !(*this->stream);
}
