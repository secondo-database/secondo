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
#include <vector>
#include <fstream>
#include <sstream>
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

class InputData {
   
public:   
    size_t moid;
    size_t tripid;
    tm time;
    float x;
    float y;
};


void handleCSVLine(vector<InputData*> &data, vector<std::string> &lineData) {
   InputData *inputdata1 = new InputData;
   InputData *inputdata2 = new InputData;
   
   data.push_back(inputdata1);
   data.push_back(inputdata2);
   
   inputdata1 -> moid = atoi(lineData[0].c_str());
   inputdata2 -> moid = atoi(lineData[0].c_str());
   
   inputdata1 -> tripid = atoi(lineData[1].c_str());
   inputdata2 -> tripid = atoi(lineData[1].c_str());
   
   cout << lineData[2] << endl;
   
   // 2007-06-08 08:32:26.781
   struct tm tm1;
   struct tm tm2;
   
   if (! strptime(lineData[2].c_str(), "%Y-%m-%d %H:%M:%S.", &tm1)) {
      cerr << "Unable to parse start date: " << lineData[2] << endl;
   }
   
   if (! strptime(lineData[3].c_str(), "%Y-%m-%d %H:%M:%S.", &tm2)) {
      cerr << "Unable to parse end date: " << lineData[3] << endl;
   }
   
   inputdata1 -> time = tm1;
   inputdata2 -> time = tm2;
   
   inputdata1 -> x = atof(lineData[4].c_str());
   inputdata1 -> y = atof(lineData[5].c_str());
   
   inputdata2 -> x = atof(lineData[6].c_str());
   inputdata2 -> y = atof(lineData[7].c_str());
}

bool parseInputData(string &filename, vector<InputData*> &data) {
   
   if( access( filename.c_str(), F_OK ) == -1 ) {
      cerr << "Unable to open Input file: " << filename << endl;
      return false;
   }
   
   string line;
   ifstream myfile(filename.c_str());
   
   if (! myfile.is_open()) {
      cerr << "Unable to open file: " << filename << endl;
      return false;
   }
   
   while ( getline (myfile,line) ) {
          
      vector<std::string> lineData;
      stringstream lineStream(line);
      string cell;

      while(getline(lineStream,cell,',')) {
         lineData.push_back(cell);
      }
                    
      if(lineData.size() != 8) {
         cerr << "Invalid line: " << line << " skipping" << endl;
         continue;
      }
          
      handleCSVLine(data, lineData);
   }
   
   myfile.close();
   return true;
}

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
   
   vector<InputData*> inputData;
   
   bool result = parseInputData(configuration->inputfile, inputData);
   
   if(! result) {
      cerr << "Unable to parse input data" << endl;
      exit(-1);
   }
   
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
   
   while(! inputData.empty()) {
      InputData *entry = inputData.back();
      inputData.pop_back();
      
      if(entry != NULL) {
         delete entry;
         entry = NULL;
      }
   }
   
   return EXIT_SUCCESS;
}