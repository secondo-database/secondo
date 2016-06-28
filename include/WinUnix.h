/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

September 03, M. Spiekermann: Initial Version


This class consists of static functions which are used to hide
calls to different Win32 or Unix/Linux libraries, thus all system
dependent code should be isolated in this class.

*/

#ifndef CLASS_WINUNIX_H
#define CLASS_WINUNIX_H

#include <string>
#include <fstream>
#include <inttypes.h>
#include <cstdlib>


class WinUnix {

  static const bool win32;

public:
   WinUnix(){};
   ~WinUnix(){};

   static int getPageSize( void );
   static int getpid( void );

   static void setenv(const char *name, const char *value);

   static inline bool isLittleEndian() { return *(char *)&endian_detect == 1;}

   static inline int rand(void) { return ::rand(); }
   static inline int rand(int n)
   {
     return 1 + (int) (n * 1.0 * (rand() / (RAND_MAX + 1.0)));
   }

   static void srand(unsigned int seed) { return std::srand(seed); }

   static void string2stdout(const char* string);
   static void stacktrace(const char* appName, const char* stacktraceOutput);

   static inline bool WindowsHost() { return isWin32(); }
   static inline bool isWin32() { return win32; }
   static inline bool isUnix() { return !win32; }

   static void sleep( const int seconds );
   static std::string getPlatformStr();

   static void writeBigEndian(std::ostream& o, const uint32_t number);

   static void writeLittleEndian(std::ostream& o, const uint32_t number);

   static void writeLittleEndian(std::ostream& o, const uint16_t number);

   static void writeBigEndian(std::ostream& o, const uint16_t number);

   static void writeLittle64(std::ostream& o, const double number);

   static void writeBig64(std::ostream& o, const double number);

   static void writeLittleEndian(std::ostream& o, const unsigned char b);

   static void writeBigEndian(std::ostream& o, const unsigned char b);

   static uint32_t convertEndian(const uint32_t n);
   static uint16_t convertEndian(const uint16_t n);
   static uint64_t convertEndian(const uint64_t n);

private:
   static const int endian_detect;
};

/*
A Class for handling text files

*/

class CFile {

   std::fstream object;

  public:
   CFile(const std::string& name) : fileName(MakePath(name))
   { object.clear(); }
   ~CFile() {}
   bool exists();    // check whether file exists
   bool open();      // open existing file in reading-mode
   bool overwrite(); // open (possibly existing) file in overwrite-mode
   bool append();    // open (possibly existing) file in append-mode
   bool close();     // close an open file
   bool eof() { return object.eof(); }
   bool fail() { return object.fail(); }
   bool good() { return object.good(); }
   std::fstream& ios() { return object; }

   const std::string fileName;
   std::string getPath() const;
   std::string getName() const;

   static const char* pathSepWin32;
   static const char* pathSepUnix;
   static const char* pathSep;

   static std::string MakePath(const std::string& s);
};

  /*
  template<class T>
  CFile& operator<<(CFile& f, const T& t) {
    f.ios() << t;
    return f;
  }

  template <class T>
  CFile& operator>>(CFile& f, const T& t) {
    f.ios() >> t;
    return f;
  }
  */


#endif
