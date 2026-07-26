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

*/

#include <iostream>
#include <string>
#include "Messenger.h"
#include "WinUnix.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

bool
Messenger::Send( const string& message, string& answer )
{
  answer = "";

  struct sockaddr_un addr;
  memset(&addr, '\0', sizeof(addr));

  addr.sun_family = AF_UNIX;
  string fullpath = string("/tmp/") + msgQueue;
  fullpath.copy(addr.sun_path, 100);

  int fd;

  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    answer = "Socket error";
    return false;
  }

  // Bound every step so a registrar that accepts a connection but never answers
  // cannot stall the caller.
  static const int kTimeoutSecs = 5;

  // A blocking connect() does not honour SO_SNDTIMEO, so connect non-blocking
  // and wait for writability with a bounded poll(), then restore blocking mode.
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags != -1) {
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  }
  int crc = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
  if (crc == -1 && errno == EINPROGRESS) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT;
    int pr = poll(&pfd, 1, kTimeoutSecs * 1000);
    if (pr <= 0) {
      answer = (pr == 0) ? "Connect timed out" : "Connect error";
      close(fd);
      return false;
    }
    int soerr = 0;
    socklen_t len = sizeof(soerr);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&soerr, &len) == -1
        || soerr != 0) {
      answer = "Connect error";
      close(fd);
      return false;
    }
  } else if (crc == -1) {
    answer = "Connect error";
    close(fd);
    return false;
  }

  // Back to blocking, now bounded by the timeouts
  if (flags != -1) {
    fcntl(fd, F_SETFL, flags);   
  }

  // Set timeouts
  struct timeval tv;
  tv.tv_sec  = kTimeoutSecs;
  tv.tv_usec = 0;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

  // Write command to registry. Sending less than the whole command would leave
  // the registrar waiting for the rest of a line that never arrives.
  if( !WinUnix::writeAll(fd, message.c_str(), message.length())
      || !WinUnix::writeAll(fd, "\n", 1) ) {
    answer = "Unable to send the command";
    close(fd);
    return false;
  }

  // Read the answer from the registrar: a single newline-terminated line.
  char buf[255];
  memset(buf, '\0', sizeof(buf));

  size_t buf_used = 0;
  bool ok = false;

  for(;;) {

    // A reply that fills the buffer without a newline is malformed; stop rather
    // than call read() with a zero-length count, which would look like EOF.
    if(buf_used >= sizeof(buf)) {
      answer = "Reply too long";
      break;
    }

    ssize_t result = read(fd, buf + buf_used, sizeof(buf) - buf_used);

    if(result < 0) {
      // EINTR is worth retrying. A receive timeout (EAGAIN/EWOULDBLOCK from
      // SO_RCVTIMEO) means the registrar accepted the connection but went
      // silent; report that rather than retry, and any other error likewise.
      if(errno == EINTR) {
        continue;
      }
      answer = (errno == EAGAIN || errno == EWOULDBLOCK)
               ? "Registrar timed out" : "Unable to read data";
      break;
    }

    if(result == 0) {
      // EOF: the registrar closed the connection without answering, which
      // normally means it died. Report it instead of reading again forever.
      answer = "Registrar closed the connection";
      break;
    }

    buf_used = buf_used + (size_t) result;

    // Stop at the first newline anywhere in the buffer. Checking only the last
    // byte read would be equivalent for every well-formed reply, including one
    // split across several reads, since the newline ends the final chunk. It
    // differs only for malformed replies where a read returns bytes *after*
    // the newline, and there the last-byte check loops forever waiting for a
    // newline that has already arrived.
    const char* newline = (const char*) memchr(buf, '\n', buf_used);

    if(newline != 0) {
      // The protocol is one command per connection answered by exactly one
      // line, so anything after that newline means the sender is out of step
      // with us.
      if((size_t) (newline - buf) + 1 != buf_used) {
        answer = "Unexpected data after the reply line";
        break;
      }
      ok = true;
      break;
    }
  }

  if(ok) {
    answer = string(buf, buf_used);
  }

  close(fd);
  return ok;
}
