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

September 2003, M. Spiekermann: Implementation of getpagesize()

*/


#include "SecondoConfig.h"

#ifdef SECONDO_WIN32
#include <io.h>
#include <windows.h>

#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>

// Supplied by makefile.env as -DSECONDO_PLATFORM_NAME="<platform>".
#ifndef SECONDO_PLATFORM_NAME
#error "SECONDO_PLATFORM_NAME is not defined -- build via makefile.env, \
which derives it from $(platform)."
#endif


#ifndef SECONDO_ANDROID
#if defined(SECONDO_LINUX) || defined(SECONDO_MAC_OSX)
#include <execinfo.h>
#endif
#endif

#ifdef SECONDO_HAVE_LIBBACKTRACE
#include <backtrace.h>
#endif

#include <iostream>
#include <string>

#include "CharTransform.h"
#include "WinUnix.h"
#include "LogMsg.h"

using namespace std;

const int
WinUnix::endian_detect = 1;


#ifdef SECONDO_WIN32
const bool
WinUnix::win32 = true;
#else
const bool
WinUnix::win32 = false; 
#endif  

int
WinUnix::getPageSize() {

   return WinUnix::SECONDO_PAGE_SIZE;
}


int
WinUnix::getpid() { 

#ifdef SECONDO_WIN32
  return ::GetCurrentProcessId();
#else
  return ::getpid();
#endif
}

void
WinUnix::setenv(const char *name, const char *value) 
{
#ifdef SECONDO_WIN32
#ifdef HAVE__PUTENV_S
   _putenv_s(name, value);
#endif
#else
   // set or overwrite
   ::setenv(name, value, 1);
#endif
}

void
WinUnix::sleep( const int seconds )
{
#ifdef SECONDO_WIN32
  ::Sleep( (DWORD) (seconds*1000) );
#else
  ::sleep( seconds );
#endif
}



string
WinUnix::getPlatformStr() {
  // Compiled in by makefile.env from $(platform); see -DSECONDO_PLATFORM_NAME.
  return string(SECONDO_PLATFORM_NAME);
}

void WinUnix::writeBigEndian(ostream& o, const uint32_t number){
  uint32_t x = number;
  if(isLittleEndian()){
    x = (x>>24) | ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
  }
  o.write(reinterpret_cast<char*>(&x),4);
}

void WinUnix::writeLittleEndian(ostream& o, const uint32_t number){
  uint32_t x = number;
  if(!isLittleEndian()){
    x = (x>>24) | ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
  }
  o.write(reinterpret_cast<char*>(&x),4);
}


void WinUnix::writeLittleEndian(ostream& o, 
  const uint16_t number){
  uint16_t x = number;
  if(!isLittleEndian()){
    x =  (( x & 0x00FF) << 8) | ( ( x & 0xFF00) >> 8);
  }
  o.write(reinterpret_cast<char*>(&x),2);
}


void WinUnix::writeBigEndian(ostream& o, const uint16_t number){
  uint16_t x = number;
  if(isLittleEndian()){
    x =  (( x & 0x00FF) << 8) | ( ( x & 0xFF00) >> 8);
  }
  o.write(reinterpret_cast<char*>(&x),2);
}


void WinUnix::writeLittle64(ostream& o, const double number){
   double number2 = number;
   uint64_t x = *(reinterpret_cast<uint64_t*>(&number2));
   if(!isLittleEndian()){
       x = convertEndian(x);
   }
   o.write(reinterpret_cast<char*>(&x),8);
}

void WinUnix::writeBig64(ostream& o, const double number){
   double number2 = number;
   uint64_t x = *(reinterpret_cast<uint64_t*>(&number2));
   if(isLittleEndian()){
       x = convertEndian(x);
   }
   o.write(reinterpret_cast<char*>(&x),8);
}


void WinUnix::writeLittleEndian(ostream& o, const unsigned char b){
   unsigned char b1 = b;
   o.write(reinterpret_cast<char*>(&b1),1);
}

void WinUnix::writeBigEndian(ostream& o, const unsigned char b){
   unsigned char b1 = b;
   o.write(reinterpret_cast<char*>(&b1),1);
}



uint32_t WinUnix::convertEndian(const uint32_t n){
  uint32_t x = n;
  return (x>>24) | ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}


uint16_t WinUnix::convertEndian(const uint16_t number){
  uint16_t x = number;
  return  (( x & 0x00FF) << 8) | ( ( x & 0xFF00) >> 8);
}


uint64_t WinUnix::convertEndian(const uint64_t n){
   uint64_t x = n;
   return (x>>56) | 
          ((x<<40) & 0x00FF000000000000ull) |
          ((x<<24) & 0x0000FF0000000000ull) |
          ((x<<8)  & 0x000000FF00000000ull) |
          ((x>>8)  & 0x00000000FF000000ull) |
          ((x>>24) & 0x0000000000FF0000ull) |
          ((x>>40) & 0x000000000000FF00ull) |
          (x<<56);
}

/* Write a complete buffer to a file descriptor. Partial writes are continued
   and interrupted writes are repeated. */
bool
WinUnix::writeAll(int fd, const char* buffer, size_t size) {
   size_t written = 0;

   while(written < size) {
      // 'long' instead of 'ssize_t', the latter is not available on Windows
      long result = (long) write(fd, buffer + written, size - written);

      if(result < 0) {
         if(errno == EINTR) {
            continue;
         }
         return false;
      }

      written = written + (size_t) result;
   }

   return true;
}

/* Write a string to stdout */
void
WinUnix::string2stdout(const char* string) {
   writeAll(fileno(stdout), string, strlen(string));
}
    
#ifndef SECONDO_ANDROID
#if defined(SECONDO_LINUX) || defined(SECONDO_MAC_OSX)

#ifdef SECONDO_HAVE_LIBBACKTRACE

namespace {

// Created once at startup, never freed: libbacktrace has no counterpart to
// backtrace\_create\_state, and having the state ready before the crash is
// what makes backtrace\_full() usable from a signal handler at all.
struct backtrace_state* btState = 0;

/*
Writes VALUE in the given base into the caller's stack buffer and returns the
number of characters written. snprintf() must not be used in a signal handler:
a signal arriving while the main program is inside it would deadlock on the
lock it already holds. Application's number2stdout() does not fit either, it
writes to stdout only.

*/
size_t
writeNum(char* buffer, uintptr_t value, unsigned int base)
{
   char digits[sizeof(uintptr_t) * 8];
   size_t len = 0;

   do {
      digits[len++] = "0123456789abcdef"[value % base];
      value /= base;
   } while(value != 0);

   for(size_t pos = 0; pos < len; pos++) {
      buffer[pos] = digits[len - 1 - pos];
   }

   return len;
}

void
writeStr(int fd, const char* string)
{
   WinUnix::writeAll(fd, string, strlen(string));
}

/*
Error callback for backtrace\_create\_state() as well as backtrace\_full().
points to the file descriptor to report on, and is NULL during setup, where
there is nowhere useful to write and a NULL state is the only signal needed.
Reporting matters: when backtrace\_full() fails outright no frame is printed at
all, and a silent empty trace would be the worst possible output.

*/
void
btError(void* data, const char* msg, int errnum)
{
   if(data == 0) {
      return;
   }

   int fd = *(int*) data;

   writeStr(fd, "  [libbacktrace: ");
   writeStr(fd, (msg != 0) ? msg : "unknown error");
   writeStr(fd, "]\n");
   (void) errnum;
}

/*
Prints one frame as "0xPC function at file:line". No iostream, no printf and
no std::string: each of them either allocates or takes a lock the interrupted
code may already hold. FILENAME and FUNCTION are NUL-terminated buffers owned
by libbacktrace and valid only for the duration of this call, so they are
written straight through. The names are DWARF linkage names and stay mangled;
the wrapper script pipes the trace through c++filt.

*/
int
btFrame(void* data, uintptr_t pc, const char* filename, int lineno,
        const char* function)
{
   int fd = *(int*) data;

   // "0x" + at most 16 hex digits + ' ' = 19, and the ":lineno" written
   // further down needs 11.
   char buffer[32];
   size_t len = 0;

   buffer[len++] = '0';
   buffer[len++] = 'x';
   len += writeNum(buffer + len, pc, 16);
   buffer[len++] = ' ';
   WinUnix::writeAll(fd, buffer, len);

   writeStr(fd, (function != 0) ? function : "??");

   if(filename != 0) {
      writeStr(fd, " at ");
      writeStr(fd, filename);
      buffer[0] = ':';
      len = 1 + writeNum(buffer + 1, (uintptr_t) (lineno > 0 ? lineno : 0), 10);
      WinUnix::writeAll(fd, buffer, len);
   }

   writeStr(fd, "\n");

   return 0;   // keep unwinding
}

} // anonymous namespace
#endif

/* Prepare the symbolizing stack tracer. */
void
WinUnix::initStacktrace()
{
#ifdef SECONDO_HAVE_LIBBACKTRACE
   // NULL rather than the application name: Application only knows
   // basename(argv[0]), which libbacktrace would look up relative to the
   // current directory. Given NULL it locates the running image itself.
   //
   // threaded = 1 because SECONDO is multi-threaded.
   //
   // This call is cheap, no debug info is read before the first
   // backtrace_full(), but it is the part that allocates, which is why it
   // cannot wait for the crash. Should a crash ever hang in the tracer,
   // reading the debug info in advance (backtrace_pcinfo() on a known
   // address) is the way out; it is not done by default because it costs
   // startup time and memory in every run.
   btState = backtrace_create_state(NULL, 1, btError, NULL);
#endif
}

/* Obtain a backtrace and print it. */
void
WinUnix::stacktrace(const char* stacktraceOutput)
{
     string2stdout(" Generating stack trace ... \n");
     string2stdout(" ************ BEGIN STACKTRACE ************\n");

     int fd = fileno(stdout); // File descriptor for stacktrace output
     int outputfd = -1;

     if(stacktraceOutput != NULL) {
         string2stdout("Writing stacktrace to: ");
         string2stdout(stacktraceOutput);
         string2stdout("\n");

         outputfd = open (stacktraceOutput,
                 O_TRUNC | O_WRONLY | O_CREAT, 0666);

         if (outputfd != -1) {
             fd = outputfd;
         }
     }

#ifdef SECONDO_HAVE_LIBBACKTRACE
     if(btState != 0) {
         // skip 1 drops this function, the signal handler frame above it is
         // kept as a marker. libbacktrace walks every loaded object through
         // dl_iterate_phdr, so a frame in an algebra shared library or in
         // libc is resolved against that object's own debug info: the case
         // one relocation offset for the whole process could not get right.
         backtrace_full(btState, 1, btFrame, btError, &fd);
     } else {
         // Setting up the symbolizer failed. Raw addresses beat nothing.
         void* frames[256];
         backtrace_symbols_fd(frames, backtrace(frames, 256), fd);
     }
#else
     // No libbacktrace on this platform, so the addresses stay unsymbolized.
     void* frames[256];
     backtrace_symbols_fd(frames, backtrace(frames, 256), fd);
#endif

     if (outputfd != -1) {
        close(outputfd);
     }

     string2stdout("\n *********** END STACKTRACE **********************\n\n");
}
#else
void
WinUnix::initStacktrace()
{
}

void
WinUnix::stacktrace(const char* stacktraceOutput)
{
  cerr << "Sorry - stack trace not supported." << endl;
}


#endif
#else
void
WinUnix::initStacktrace()
{
}

void
WinUnix::stacktrace(const char* stacktraceOutput)
{
  cerr << "Sorry - stack trace under Android not supported." << endl;
}


#endif

char* WinUnix::getAbsolutePath(const char* relPath){
#ifndef SECONDO_WIN32
  return realpath(relPath,0);
#else
  return _fullpath(0,relPath,1024);
#endif

}



/*
Implementation of class ~CFile~

*/   

const char*
CFile::pathSepWin32 = "\\";

const char*
CFile::pathSepUnix = "/";

#ifdef SECONDO_WIN32
const char*
CFile::pathSep = pathSepWin32;
#else
const char*
CFile::pathSep = pathSepUnix;
#endif  

string
CFile::MakePath(const string& s)
{ 
  string t=s;
  if ( WinUnix::isWin32() )
    t = translate(t, pathSepUnix, pathSepWin32);
  else
    t = translate(t, pathSepWin32, pathSepUnix);
  return t;  
}

bool CFile::exists() 
{
  bool rc = access( fileName.c_str(), F_OK ) != -1;
  return rc;
} 

bool CFile::open() 
{
  object.open(fileName.c_str(), ios::in);
  return object.good();
} 
   
bool CFile::overwrite() 
{
  object.open(fileName.c_str(), ios::out|ios::trunc);
  return object.good();
} 

bool CFile::append() 
{
  object.open(fileName.c_str(), ios::out|ios::app);
  return object.good();
} 


bool CFile::close() 
{
  object.close();
  return object.good(); 
}


string CFile::getPath() const 
{
  size_t pos = fileName.find_last_of(pathSep);
  if (pos != string::npos)
    return fileName.substr(0,pos+1);
  else
    return "";
}


string CFile::getName() const
{
  string name = fileName;
  string path = getPath();
  if (path != "")
    removePrefix(path, name);
  return name;
}






