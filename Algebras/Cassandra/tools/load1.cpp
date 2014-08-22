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
to a network socket. A sample line looks like:

gEpYm0eUDk,fAgVgUHWPo,bClSVK17HX,ixjXTTW7yh,qdsU8WzP1O,
CcZw52F47W,bpRKKsoq0m,YoNOJWsGtt,c5U92XBHbG,kA5CUO4GE2

You can specify the number of lines to be send and parameter
like delay, number of columns or the size of a column.
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
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <boost/concept_check.hpp>

#include "timer.h"

/* 
1.1 Defines

*/

#define EOT             "\004"
#define ACK             "\006"

#define CMDLINE_HOST    1<<0
#define CMDLINE_PORT    1<<1
#define CMDLINE_LINES   1<<2
#define CMDLINE_DELAY   1<<3
#define CMDLINE_COLUMNS 1<<4
#define CMDLINE_SIZE    1<<5
#define CMDLINE_ACK     1<<6

using namespace std;

/*
1.2 Structs

*/

struct commandline_args_t {
   char *hostname;      // Hostname
   int port;            // Port to connect to
   int lines;           // Lines to send
   int delay;           // Delay in ms
   int columns;         // Number of columns
   int sizePerColumn;   // Size per column
   int acknowledgeAfter;// Wait for ack after n lines
};

/*
2.1 print usage fuction

*/
void printUsageAndExit(char* progname) {
      cerr << "Usage: " << progname << " -h <hostname> -p <port> -l <lines> "
          "-d <delay> -c <columns> -s <size per column> -a <ack after>" 
       << endl;
      cerr << endl;
      cerr << "Where <hostname> is the hostname to connect to" << endl;
      cerr << "<port> is the port to connect to" << endl;
      cerr << "<lines> is the numer of lines to send" << endl;
      cerr << "<delay> is the pause (in ms) between two lines " << endl;
      cerr << "<columns> is the number of columns to generate" << endl;
      cerr << "<size per column> is the size in byte per column" << endl;
      cerr << "<ack after> wait fror a ACK from server after n lines" << endl;
      cerr << endl;
      cerr << "Example: " << progname
           << " -h 127.0.0.1 -p 10000 -l 10 -d 4 -c 10 -s 10 -a 10" << endl;
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
     
  result.clear();
 
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
2.3 Wait for an ACK char from server

*/
void waitForAck(int socketfd) {
  
  char buffer[255];
  read(socketfd, buffer, sizeof(buffer));
  
  /*if(strncmp(buffer,ACK,1) == 0) {
    cout << "[Info] Got ack";
  } else {
    cout << "[Error] Got something else";
  }*/
}

/*
2.4 Open the network socket

*/
bool openSocket(int &socketfd, char* hostname, int port) {
  
   struct hostent *server;
   struct sockaddr_in server_addr;
   
   socketfd = socket(AF_INET, SOCK_STREAM, 0);

   if(socketfd < 0) {
      cerr << "Error opening socket" << endl;
      return false;
   }
   
   // Resolve hostname
   server = gethostbyname(hostname);
   
   if(server == NULL) {
      cerr << "Error resolving hostname: " << hostname << endl;
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
      return false;
   }
   
   return true;
}

/*
2.5 parse commandline args

*/
int parseCommandline(int argc, char* argv[], 
                     commandline_args_t &commandline_args) {
  
   unsigned int flags = 0;
   
   int option = 0;
   while ((option = getopt(argc, argv,"h:p:l:d:c:s:a:")) != -1) {
     switch (option) {
      case 'h':
           commandline_args.hostname = optarg;
           flags |= CMDLINE_HOST;
        break;
      case 'p':
           commandline_args.port = atoi(optarg);
           flags |= CMDLINE_PORT;
        break;
      case 'l':
          commandline_args.lines = atoi(optarg);
          flags |= CMDLINE_LINES;
        break;
      case 'd':
          commandline_args.delay = atoi(optarg);
          flags |= CMDLINE_DELAY;
        break;
      case 'c':
          commandline_args.columns = atoi(optarg);
          flags |= CMDLINE_COLUMNS;
        break;
      case 's':
          commandline_args.sizePerColumn = atoi(optarg);
          flags |= CMDLINE_SIZE;
      case 'a':
          commandline_args.acknowledgeAfter = atoi(optarg);
          flags |= CMDLINE_ACK;
        break;
      default:
        printUsageAndExit(argv[0]);
     } 
   }
   
   unsigned int requriedFlags = CMDLINE_HOST | CMDLINE_PORT | CMDLINE_LINES 
                                | CMDLINE_DELAY | CMDLINE_COLUMNS 
                                | CMDLINE_SIZE | CMDLINE_ACK; 
                                
   if(flags != requriedFlags) {
     printUsageAndExit(argv[0]);
   }
}

/*
2.6 main function

*/
int main(int argc, char* argv[]) {
   
   // Our output socket
   int socketfd;       
   
   // Buffer for writing
   string buffer;      
   
   // Commandline args
   commandline_args_t commandline_args;
   parseCommandline(argc, argv, commandline_args);
 
   // Initalize Rand
   srand (time(NULL));
  
   // Open socket
   if(! openSocket(socketfd, commandline_args.hostname, 
                   commandline_args.port) ) {
     
     cerr << "[Error] Unable to open socket" << endl;
   
     return EXIT_FAILURE;
   }
   
   // Prepare buffer
   fillBuffer(buffer, commandline_args.columns, 
              commandline_args.sizePerColumn);
   
   cout << "The size of the buffer is: " << buffer.length() 
        << " bytes " << endl;
   cout << "The buffer contains: " << buffer << endl;

   // Start timer
   Timer timer;
   timer.start();
   
   // Calculate progess (i)
   cout << "Writing: ";
   int fivePercents = 
      max(((int) ((commandline_args.lines / 100.0) * 5.0)), 1);
   
   // Write lines to server
   for(int i = 0; i < commandline_args.lines; ++i) {
     
      fillBuffer(buffer, commandline_args.columns, 
                 commandline_args.sizePerColumn);
      
      write(socketfd, buffer.c_str(), buffer.length());
   
      // Calculate progess (ii)
      if(i % fivePercents == 0) {
         cout << ".";
         cout << flush;
      }
      
      // Wait for ack
      if(commandline_args.acknowledgeAfter > 0 &&
        ((i + 1) % commandline_args.acknowledgeAfter == 0)) {
        waitForAck(socketfd);
      }
      
      if(commandline_args.delay != 0) {
        usleep(commandline_args.delay * 1000);
      }
   }
   
   cout << endl;
   cout << "Total execution time (ms): " << timer.getDiff() / 1000 << endl;
   
   // Send EOT (End of Transmission)
   write(socketfd, EOT, sizeof(char));
   shutdown(socketfd, 2);
   return EXIT_SUCCESS;
}
