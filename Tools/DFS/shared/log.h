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
#ifndef DFS_LOG_H
#define DFS_LOG_H

#include <iostream>
#include "str.h"

namespace dfs {

  namespace log {

    /**
     * describes entry to be logged
     */
    struct LogEntry {
      Str prefix;
      Str content;
    };

    /**
     * describes base logger methods
     * acts as an interface
     */
    class Logger {
    public:
      bool canDebug;

      void debug(const Str &s);

      void fatal(const Str &s);

      void error(const Str &s);

      void warn(const Str &s);

      void info(const Str &s);

      void success(const Str &s);

      virtual void log(const LogEntry &e) = 0;

      virtual ~Logger(){}

    };

    /**
     * this Logger implementation logs to cout
     */
    class DefaultOutLogger : public Logger {
    public:
      ~DefaultOutLogger(){}
      virtual void log(const LogEntry &e);
    };

    /**
     * this Logger implementation does log anything :)
     */
    class BlackHoleLogger : public Logger {
    public:
      virtual void log(const LogEntry &e) {}
    };

    /**
     * a simple file logger using a file to log
     * file is appended
     */
    class FileLogger : public Logger {
    private:
      Str filename;
    public:
      void setFilename(const Str &filename);

      virtual void log(const LogEntry &e);
    };

    /**
     * a file logger with a pretty date time format
     */
    class FormattedFileLogger : public FileLogger {
    public:
      virtual void log(const LogEntry &e);
    };

    /**
     * a composite logger which calls log of all this children
     */
    class CompositeLogger : public Logger {
    private:
      Logger **arr;
      int count;
    public:
      CompositeLogger();

      virtual ~CompositeLogger();

      void add(Logger *l);

      virtual void log(const LogEntry &e);
    };

    /**
     * a composite logger with pretty format
     * children do not need to format
     */
    class FormattedCompositeLogger : public CompositeLogger {
    public:
      virtual void log(const LogEntry &e);
    };

  };

};

#endif
