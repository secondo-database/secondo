/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[$][\$]

*/
#include "log.h"
#include "str.h"
#include "io.h"
#include "datetime.h"

using namespace std;
using namespace dfs::log;

void Logger::debug(const Str &s) {
  if (!canDebug) return;
  LogEntry e;
  e.prefix = "DEBUG  ";
  e.content = s;
  this->log(e);
}

void Logger::fatal(const Str &s) {
  LogEntry e;
  e.prefix = "FATAL  ";
  e.content = s;
  this->log(e);
}

void Logger::error(const Str &s) {
  LogEntry e;
  e.prefix = "ERROR  ";
  e.content = s;
  this->log(e);
}

void Logger::warn(const Str &s) {
  LogEntry e;
  e.prefix = "WARN   ";
  e.content = s;
  this->log(e);
}

void Logger::info(const Str &s) {
  LogEntry e;
  e.prefix = "INFO   ";
  e.content = s;
  this->log(e);
}

void Logger::success(const Str &s) {
  LogEntry e;
  e.prefix = "SUCCESS";
  e.content = s;
  this->log(e);
}

void DefaultOutLogger::log(const LogEntry &e) {
  cout << e.prefix << " " << e.content << endl;
}

void FileLogger::setFilename(const Str &f) {
  this->filename = f;
}

void FileLogger::log(const LogEntry &e) {
  dfs::io::file::Writer w(filename, true);
  Str s = e.prefix.append(" ").append(e.content).append("\r\n");
  w.append(s);
  w.close();
}

CompositeLogger::CompositeLogger() {
  count = 0;
  arr = 0;
}

void CompositeLogger::add(Logger *l) {
  if (count == 0) {
    arr = new Logger *[1];
    arr[0] = l;
    count++;
  } else {
    Logger **tmp = new Logger *[count + 1];
    for (int i = 0; i < count; i++) tmp[i] = arr[i];
    tmp[count++] = l;
    delete[] arr;
    arr = tmp;
  }
}

CompositeLogger::~CompositeLogger() {
  if (count > 0) {
    count = 0;
    delete[] arr;
  }
}

void CompositeLogger::log(const LogEntry &e) {
  if (count == 0) return;
  for (int i = 0; i < count; i++)
    arr[i]->log(e);
}

void FormattedCompositeLogger::log(const LogEntry &e) {
  LogEntry f;
  f.prefix = dfs::datetime::DateTimeUtils::forLog().append(" ").append(
    e.prefix);
  f.content = e.content;
  CompositeLogger::log(f);
}

void FormattedFileLogger::log(const LogEntry &e) {
  LogEntry f;
  f.prefix = dfs::datetime::DateTimeUtils::forLog().append(" ").append(
    e.prefix);
  f.content = e.content;
  FileLogger::log(f);
}
