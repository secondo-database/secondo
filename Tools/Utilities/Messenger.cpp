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

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
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
  
  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    answer = "Connect error";
    return false;
  }

  // Write command to registry
  write(fd, message.c_str(), message.length());
  write(fd, "\n", 1);

  // Read answer from registry
  char buf[255];
  memset(buf, '\0', sizeof(buf));

  size_t buf_used = 0;
  size_t buf_remain = sizeof(buf) - buf_used;
  size_t retries = 0;
  bool error = false;

  // Read until newline
  for(;;) {

    int result = read(fd, buf + buf_used, buf_remain);

    if(result < 0) {
      retries++;

      if(retries > 10) {
        answer = "Unable to read data";
        error = true;
        break;
      }
    }

    buf_used = buf_used + result;
    buf_remain = sizeof(buf) - buf_used;

    if(buf[buf_used - 1] == '\n') {
      break;
    }
  }

  if(! error) {
    answer = string(buf);
  }
  
  close(fd);
  return true;
}
