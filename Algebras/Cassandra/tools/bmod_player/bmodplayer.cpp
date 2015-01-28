/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the Systems of the GNU General Public License as published by
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


1 Realtime player for BerlinMod GPS Data


1.1 Includes

*/

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#define CMDLINE_INPUTFILE   1<<0
#define CMDLINE_OUTPUTFILE  1<<1
#define CMDLINE_RESOLUTION  1<<2

using namespace std;

struct Configuration {
   string inputfile;
   string outputfile;
   size_t resolution;
};

struct Statistics {
   size_t trips;
   size_t send;
   size_t discarded; 
};


void printHelpAndExit(char *progName) {
   cerr << "Usage: " << progName << " -i <inputfile>";
   cerr << " -o <outputfile> -r <resolution>" << endl;
   exit(-1);
}

void parseParameter(int argc, char *argv[], Configuration *configuration) {
   
   unsigned int flags = 0;
   int option = 0;
   
   while ((option = getopt(argc, argv,"i:o:r:")) != -1) {
       switch (option) {
          case 'i':
             flags |= CMDLINE_INPUTFILE;
             configuration->inputfile = string(optarg);
          break;
          
          case 'o':
             flags |= CMDLINE_OUTPUTFILE;
             configuration->outputfile = string(optarg);
          break;
          
          case 'r':
             flags |= CMDLINE_RESOLUTION;
             configuration->resolution = atoi(optarg);
          break;
          
          default:
            printHelpAndExit(argv[0]);
       }
   }
   
   
   unsigned int requiredFalgs = CMDLINE_RESOLUTION | 
                                CMDLINE_INPUTFILE|
                                CMDLINE_OUTPUTFILE;
   
   if(flags != requiredFalgs) {
      printHelpAndExit(argv[0]);
   }
   
}

int main(int argc, char *argv[]) {
   
   Configuration *configuration = new Configuration();
   Statistics *statistics = new Statistics(); 
   
   parseParameter(argc, argv, configuration);
   
   for(size_t i = 0; i < 100; i++) {
      cout << "\r\033[2K" << "Sec: " << i;
      cout << " \033[1m Trips:\033[0m " << statistics -> trips;
      cout << " \033[1m Send:\033[0m " << statistics -> send;
      cout << " \033[1m Discarded:\033[0m " << statistics -> discarded;
      cout.flush();
      usleep(1000 * 1000);
   }

   if(statistics != NULL) {
      delete statistics;
      statistics = NULL;
   }
   
   if(configuration != NULL) {
      delete configuration;
      configuration = NULL;
   }
   
   return EXIT_SUCCESS;
}