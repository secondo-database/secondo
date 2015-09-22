/*
----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

[1] TCP blackhole

*/

#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFERSIZE 1048576 // 1 MB Buffersize
#define LISTENPORT 10025

using namespace std;

void handleRead(int connfd) {
   char buffer[BUFFERSIZE];
   size_t bytesRead = 0;
   
   while(bytesRead >= 0) {
      bytesRead = read(connfd, buffer, BUFFERSIZE);
   }
   
   if(connfd != 0) {
      shutdown(connfd, 2);
   }
}

/*
2.7 Main function

The function open a socket, generate requested data and close the socket

*/
int main(int argc, char* argv[]) {
   
   // Our server socket
   struct sockaddr_in serv_addr;   // Server address  
   struct sockaddr_in client_addr; // Client address
   
   int listenfd = socket(AF_INET, SOCK_STREAM, 0);
   int connfd = 0;

   memset(&serv_addr, 0, sizeof(serv_addr));
   memset(&client_addr, 0, sizeof(client_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(LISTENPORT); 

   // Bind our socket
   if ((bind(listenfd, (struct sockaddr *) 
       &serv_addr, sizeof(serv_addr)) ) < 0) {
     
     cerr << "[Error] Bind failed" << endl;
     return false;
   }

   // Listen
   if(( listen(listenfd, 10)) < 0 ){
     cerr << "[Error] Listen failed " << endl;
     return false;
   }
   
   unsigned int clientlen = sizeof(client_addr);

   // Accept connection
   if(! (connfd = accept(listenfd, (struct sockaddr *) &client_addr, 
         &clientlen))) {
           
     cerr << "[Error] Accept failed" << endl;
     return false;
   } else {
      handleRead(connfd);
   }
   
   if(listenfd != 0) {
     shutdown(connfd, 2);
     connfd = 0;
   }

   return EXIT_SUCCESS;  
}