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
#pragma once

#include "../define.h"
#include "str.h"

#include <iostream>

using namespace dfs;

namespace dfs {

  namespace io {

    namespace file {

      /**
       * allocates a file of given space
       * @param filename
       * @param bytes
       */
      void allocate(const char *filename, unsigned long long bytes);

      /**
       * creates a file with content
       * content is created by given strategy
       * strategy can be
       * ten - writes 01234567890123... to fill until content size is reached
       * single - write single char in args1 until content size is reached
       * alphabet - same as ten but with A...Z as chars
       * @param filename
       * @param bytes
       * @param strategy
       * @param args1
       */
      void createFile(const Str &filename, long bytes, const Str &strategy,
                      const Str &args1 = "");

      /**
       * deletes file
       * @param filename
       */
      void deleteFile(const Str &filename);

      /**
       * combines two path elements
       * @param a
       * @param b
       * @return
       */
      Str combinePath(const Str &a, const Str &b);

      /**
       * determine current dir of current thread
       * @return
       */
      Str currentDir();

      /**
       * creates directory recursively
       * @param dir
       */
      void createDir(const Str &dir);

      /**
       * returns file size of given file
       * @param filename
       * @return
       */
      long fileSize(const Str &filename);

      /**
       * returns file content as heap buffer
       * careful on big file
       * memory needs to be freed by caller
       * @param filename
       * @return
       */
      char *getFileContent(const Str &filename);

      /**
       * util class for writing data to a file
       */
      class Writer {
      private:
        FILE *fp;
      public:
        Writer(const Str &filename);

        Writer(const Str &filename, bool append);

        void append(int i);

        void append(Str &s);

        void append(const char *buffer, UI64 bufferLength);

        void appendWithLengthInfo(short lenlen, Str &s);

        void writeBufferAt(unsigned long long offsetInFile,
                           unsigned long bufferLength, const char *buffer);

        void close();
      };

      /**
       * util class for easy file reading
       */
      class Reader {
      private:
        FILE *fp;
        Str filename;
      public:
        Reader(const Str &filename);

        bool open();

        int readInt();

        Str readStr(int len);

        Str readWithLengthInfo(short lenlen);

        void close();
      };

    }

  };

};
