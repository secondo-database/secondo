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

[1] Loadgenerator. This programm generates CSV data and send it 
over a network socket. A sample line:

gEpYm0eUDk,fAgVgUHWPo,bClSVK17HX,ixjXTTW7yh,qdsU8WzP1O,
CcZw52F47W,bpRKKsoq0m,YoNOJWsGtt,c5U92XBHbG,kA5CUO4GE2

*/

/* 
1.0 Includes

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
#include <boost/concept_check.hpp>

/* 
1.1 Defines

*/

#define EOT "\004"
#define ACK "\006"

using namespace std;

/*
2.1 print usage fuction

*/
void printUsageAndExit(char* hostname) {
  cerr << "Usage: " << hostname << " -h <hostname> -p <port> -l <lines> "
               "-d <delay> -c <columns> -s <size per column>" << endl;
      cerr << endl;
      cerr << "Where <hostname> is the hostname to connect to" << endl;
      cerr << "<port> is the port to connect to" << endl;
      cerr << "<lines> is the numer of lines to send" << endl;
      cerr << "<delay> is the pause (in ms) between two lines " << endl;
      cerr << "<columns> is the number of columns to generate" << endl;
      cerr << "<size per column> is the size in byte per column" << endl;
      cerr << endl;
      cerr << "Example: " << hostname
           << " -h 127.0.0.1 -p 10000 -l 10 -d 4 -c 10 -s 10" << endl;
      exit(-1);
}

/*
2.2 fill the send buffer with the specified content

*/
void fillBuffer(string &result, int columns, int sizePerColumn) {
  
   // Allowed chars to send
   static const char charArray[] = 
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijlkmnopqrstuvwxyz";
      
  for(int i = 0; i < columns; i++) {
     
     if(i != 0) {
       result.append(",");
     }
     
     for(int charNum = 0; charNum < sizePerColumn; charNum++) {
       char aChar = charArray[rand() % (sizeof(charArray) - 1)];
       result.push_back(aChar);
     }
   }
   
   result.append("\n");
}

/*
2.3 main function

*/
int main(int argc, char* argv[]) {
   
   int socketfd;        // Our socket
   int port;            // Port to connect to
   string buffer;       // Buffer for writing
   char *hostname;      // Hostname
   
   int lines;           // Lines to send
   int delay;           // Delay in ms
   int columns;         // Number of columns
   int sizePerColumn;   // Size per column
   
   struct hostent *server;
   struct sockaddr_in server_addr;
   
   if(argc != 13) {
      printUsageAndExit(argv[0]);
   }
   
   int option = 0;
   while ((option = getopt(argc, argv,"h:p:l:d:c:s:")) != -1) {
     switch (option) {
      case 'h':
           hostname = optarg;
        break;
      case 'p':
           port = atoi(optarg);
        break;
      case 'l':
          lines = atoi(optarg);
        break;
      case 'd':
          delay = atoi(optarg);
        break;
      case 'c':
          columns = atoi(optarg);
        break;
      case 's':
          sizePerColumn = atoi(optarg);
        break;
      default:
        printUsageAndExit(argv[0]);
     } 
   }
   
   // Initalize Rand
   srand (time(NULL));
  
   // Prepare buffer
   fillBuffer(buffer, columns, sizePerColumn);
   
   cout << "The size of the buffer is: " << buffer.length() 
        << " bytes " << endl;
   cout << "The buffer contains: " << buffer << endl;
   
   socketfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if(socketfd < 0) {
      cerr << "Error opening socket" << endl;
      return -1;
   }
   
   // Resolve hostname
   server = gethostbyname(hostname);
   
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
   int fivePercents = max(((int) ((lines / 100.0) * 5.0)), 1);
   
   // Write lines to server
   for(int i = 0; i < lines; i++) {
      fillBuffer(buffer, columns, sizePerColumn);
      write(socketfd, buffer.c_str(), buffer.length());
   
      // Calculate progess (ii)
      if(i % fivePercents == 0) {
         cout << ".";
         cout << flush;
      }
      
      usleep(delay * 1000);
   }
   
   cout << endl;
   
   // Send EOT (End of Transmission)
   write(socketfd, EOT, sizeof(char));
   
   shutdown(socketfd, 2);
   return EXIT_SUCCESS;
}
