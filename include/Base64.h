#ifndef CLASS_BASE64_H
#define CLASS_BASE64_H

/*
December 08, M. Spiekermann: Initial Version

This class provides static member functions used for encoding and decoding
the Base64 data format as specified in RFC 1341. The interface was designed
to interact kindly with the text atoms of nested lists. 

*/

#include <string>

using namespace std;

class Base64{

public:
   Base64();
   ~Base64(){};    

   void encode(char* buffer, string& text, int size);
   /*  encodes size bytes of the buffer into base64 returned as string.  
    *  If finish is true the data will be closed, e.g. padded with "=".
    */ 
   void decode(istream& in, ostream& out);
   void encodeStreams(istream& in, ostream& out);

private:
   /* the characters used in the base64 format */
   static char base64Alphabet[];
   
   bool getNext(char& byte, istream& in);
   int  getIndex(char b);
   bool isAllowed(char b);
 
   bool endReached;
   int  currentPos;
   int  filled;

};

#endif

