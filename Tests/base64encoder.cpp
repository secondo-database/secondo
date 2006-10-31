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
#include <Base64.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <sstream>

using namespace std;



static void
usageMsg(char* progName) {

   cout << "Usage: " << progName << " [-d -n<size>] filename" << endl;
   cout << "Converts a file into base64 format and vice versa." << endl;
   cout << "Options   : " << endl;
   cout << "  -d      : decode" << endl;
   cout << "  -n<size>: Don't use the stream based methods "
        <<              "of class Base64. " << endl;
   cout << "            Read the file into a buffer of size <size>" << endl; 
}


int
main(int argc, char *argv[]) {


 if (argc < 2 || argc > 4) {
      usageMsg(argv[0]);
      exit(1);
   }


 int size=0;
 bool nostream=false, decode=false;

#ifdef _POSIX_OPT_H 
 int c = 0;
 opterr = 0;

   while ((c = getopt(argc,argv, "dn:")) != -1) {
      switch (c) {
      case 'd':
         decode = true;
         break;
      case 'n':
         nostream = true;
         size = atoi(optarg);
         break;
      case '?':
         cerr << "Unkown option " << (char) optopt << endl;
         usageMsg(argv[0]);
      default:
         exit(1);
      }  
   }

 string fileName(argv[optind]);
 string extension="";
 cout << "file: " << fileName << ", size: " << size << endl;
 decode ?  extension=".bin" : extension=".base64"; 


 ifstream inFile(fileName.c_str(), ios::binary);
 ofstream outFile((fileName+extension).c_str(), ios::binary);

 Base64* encoder = new Base64();

 if (decode) {

   if (nostream) {
     string text="";
     int bufSize = 0;
     char* buffer=0;
     int readFaults=0;
     const int inLength=1024;
     char textPart[inLength+1];

     do {

       inFile.read(textPart, inLength);
       int noBytes = inFile.gcount();
       if (noBytes != inLength) {
	 readFaults++;
       }
       assert( readFaults <= 1 );
       
       textPart[noBytes]=0; 
       text += textPart;

     } while (inFile.good());
     
     bufSize = text.length();
     bufSize=encoder->sizeDecoded(text.length());
     cout << "Text: " << text.length() << ", bufSize: " << bufSize << endl;
     buffer = new char[bufSize];    

     bufSize = encoder->decode(text, buffer);
     outFile.write(buffer, bufSize);
     delete [] buffer;

   } else {

     encoder->decodeStream(inFile,outFile);
   }

 } else {

    char* buffer = 0;
    if (nostream) {
      cout << "nostreams ..." << endl;
      buffer = new char[size+1];
      inFile.read(buffer,size);
      assert (inFile.gcount() == size);
      string text = "";
      encoder->encode(buffer, size, text);
      outFile << text;
      delete [] buffer;
 
    } else {
      encoder->encodeStream(inFile,outFile);
    }
 }

 delete encoder; 
 
#else
  cerr << "You will need the posix getopt library!" << endl;
  exit(1);
#endif   

}


