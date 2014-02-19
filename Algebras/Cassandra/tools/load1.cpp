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

[1] Load1

*/

#include <iostream>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


using namespace std;

int main(int argc, char* argv[]) {
   
   int socketfd;        // Our socket
   int port;            // Port to connect to
   string buffer;       // Buffer for writing
   
   int lines;           // Lines to send
   int delay;           // Delay in ms
   int columns;         // Number of columns
   int sizePerColumn;   // Size per column
   
   struct hostent *server;
   struct sockaddr_in server_addr;

   // Allowed chars to send
   static const char charArray[] = 
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijlkmnopqrstuvwxyz";
   
   if(argc != 7) {
      cerr << "Usage: " << argv[0] << " <hostname> <port> <lines> "
               "<delay> <columns> <size per column>" << endl;
      cerr << endl;
      cerr << "Where <hostname> is a hostname to connect to" << endl;
      cerr << "<port> is the port to connect to" << endl;
      cerr << "<lines> is the numer of lines to send" << endl;
      cerr << "<delay> is the pause (in ms) between two lines " << endl;
      cerr << "<columns> is the number of columns to generate" << endl;
      cerr << "<size per column> is the size per column" << endl;
      cerr << endl;
      cerr << "Example: " << argv[0] << " 127.0.0.1 10000 10 4 10" << endl;
      return -1;
   }
   
   // Parameter
   lines = atoi(argv[3]);
   delay = atoi(argv[4]);
   columns = atoi(argv[5]);
   sizePerColumn = atoi(argv[6]);
 
   // Initalize Rand
   srand (time(NULL));
  
   // Prepare buffer
   for(int i = 0; i < columns; i++) {
     
     if(i != 0) {
       buffer.append(";");
     }
     
     for(int charNum = 0; charNum < sizePerColumn; charNum++) {
       char aChar = charArray[rand() % (sizeof(charArray) - 1)];
       buffer.push_back(aChar);
     }
     
   }
   
   buffer.append("\n");
   
   cout << "The size of the buffer is " << buffer.length() << endl;
   cout << "The buffer contains: " << buffer << endl;
   
   // Create socket
   port = atoi(argv[2]);
   socketfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if(socketfd < 0) {
      cerr << "Error opening socket" << endl;
      return -1;
   }
   
   // Resolve hostname
   server = gethostbyname(argv[1]);
   
   if(server == NULL) {
      cerr << "Error resolving hostname: " << argv[1] << endl;
      return -1;
   }
   
   // Connect
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(port);
   
   server_addr.sin_addr.s_addr = 
     ((struct in_addr *)server->h_addr_list[0])->s_addr;
  
   if(connect(socketfd, (struct sockaddr*) &server_addr, 
         sizeof(struct sockaddr)) < 0) {

      cerr << "Error in connect() " << endl;
      return -1;
   }
 
   // Calculate progess (i)
   cout << "Writing: ";
   int fifePercents = max(((int) ((lines / 100.0) * 5.0)), 1);
   
   // Write lines to server
   for(int i = 0; i < lines; i++) {
      write(socketfd, buffer.c_str(), buffer.length());
   
      // Calculate progess (ii)
      if(i % fifePercents == 0) {
         cout << ".";
         cout << flush;
      }
      
      usleep(delay * 1000);
   }
   
   cout << endl;
   
   // Send EOT (End of Transmission)
   write(socketfd, "\004", sizeof(char));
   
   shutdown(socketfd, 2);

   return EXIT_SUCCESS;
}
