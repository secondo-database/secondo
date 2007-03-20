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

//paragraph [1] Title: [{\Large \bf ] [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[->] [$\rightarrow $]


Dec 2003 M. Spiekermann, initial Version 


******************************************************************************/

#include <unistd.h>

#include <string>
#include <iostream>
#include <fstream>

#include "NestedList.h"


static void
usageMsg(char* progName) {

   cout << "Usage: " << progName << " [-b -2] filename" << endl;
   cout << "Converts a nested list from text to binary format "
        << "and vice versa." << endl;
   cout << "  -b: Input file is a binary encoded nested list" << endl;
   cout << "  -2: 2 output files, binary and text" << endl; 
   cout << "  -k: keep format. Output format is the same as in "
        << "the input file." << endl; 
}


int
main(int argc, char *argv[]) {
#ifdef _POSIX_OPT_H
   if (argc < 2 || argc > 5) {
      usageMsg(argv[0]);
      exit(1);
   }

   int c = 0;
   opterr = 0;
   bool keepFormat=false, twoOutputs = false, 
        outBinary=true, outText=false, binary=false, text=true;

   while ((c = getopt(argc,argv, "bk2")) != -1) {
      switch (c) {
      case 'b':
         binary = true;
         text = false;
         outText = true;
         outBinary = false;
         break;
      case '2':
         twoOutputs = true;
         break;
      case 'k':
         keepFormat = true; 
         break;
      case '?':
         cerr << "Unkown option " << (char) optopt << endl;
         usageMsg(argv[0]);
      default:
         abort();
      }  
   }

   if (keepFormat) {
      outBinary = keepFormat && binary;
      outText = keepFormat && text;
   }

   if (twoOutputs) {
      outBinary = true;
      outText = true;
   }

   string inFileStr(argv[optind]);
   string modeStr = "";

   NestedList nl(0);

   modeStr = binary ? "binary" : "text";
   cout << endl << "Reading " << modeStr << "-file " << inFileStr << endl;

   ListExpr list=0;
   if (binary) {
      ifstream inFile(inFileStr.c_str(), ios::binary); 
      nl.ReadBinaryFrom( inFile, list );
      cout << "Reading binary encoded list ..." << endl;
   } else {
      cout << "Reading textual list representation ... " << endl;
      nl.ReadFromFile( inFileStr, list );
   }   

   if (outText) {
      string outFileStr = inFileStr + ".text";
      cout << endl << "Writing " << outFileStr << " ... " << endl;
      nl.WriteToFile( outFileStr, list );
   }

   if (outBinary) {
      string outFileStr = inFileStr + ".bnl";
      cout << endl << "Writing " + outFileStr + " ... " << endl;
      ofstream outFile(outFileStr.c_str(), ios::out|ios::trunc|ios::binary); 
      nl.WriteBinaryTo(list, outFile);   
   }
#else
  cerr << "You will need the posix getopt library!" << endl;
  exit(1);
#endif  
}


