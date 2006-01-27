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

//characters    [1]    verbatim:       [\verb@]                                [@]

1 Base64, a class for encoding binary data to text and vice versa.

December 08, M. Spiekermann: Initial Version

This class provides member functions used for encoding and decoding the ~base64~
data format as specified in RFC 1341. The interface was designed to interact
kindly with the text atoms of nested lists. The nested list parser recognizes a
file name enclosed in the tag-pair "<file>"[1],"</file--->" and translates it to
a text atom containing the data of the file encoded into base64 format. The
base64 data maps 24 bits of input to 4 letters into an alphabet of 64
characters. Thus the encoded data grows to 4/3 (not exactly, since newlines are
added every 72 bytes) of the original size and the decoded Size shrinks to less
than 3/4 since newlines and unknown characters are ignored. 


*/

#ifndef CLASS_BASE64_H
#define CLASS_BASE64_H

#include <string>
#include <math.h>
using namespace std;

class Base64{

public:
   Base64();
   ~Base64(){};    

   int sizeDecoded(int letters) {
             return (int)ceil( (3*(double)letters)/4 );
   }
   int sizeEncoded(int bytes) {
             return ( (int)ceil( 4*(double)bytes/3 ) + (int)ceil( 4*(double)bytes/(3*72) ) );
   }
/*
These functions calculate the decoded or encoded size for a given
number of bytes. This is useful for allocating buffers.

1.1 Encoding and Decoding

*/


   void encode(const char* bytes, int size, string& base64);
/*  
Encodes the bytes contained in the buffer ~bytes~ of size ~size~ into base64
format written into the string ~base64~.  

*/ 
   int decode(const string& text, char* bytes);
/*
Decodes the base64 data stored in ~text~ and returns the number of bytes written
into the buffer ~bytes~ which has to be allocated by the caller. The size of the
buffer can be estimated by the function sizeDecoded().   

*/
   void decodeStream(istream& in, ostream& out);
   void encodeStream(istream& in, ostream& out);

/*
Encode or decode a stream of base64 characters or bytes. 

*/


private:
   /* the characters used in the base64 format */
   static char base64Alphabet[];
   
   /* internal helper functions */
   int  getIndex(char b);
   bool isAllowed(char b);

   /* these functions implement the encoding logic */
   void encode2(const char* buffer, string& text, int size);
   bool getNext(char& byte, istream& in);

   /* status and position information*/ 
   bool endReached;
   int  currentPos;
   int  filled;

};

#endif

