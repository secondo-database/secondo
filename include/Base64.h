#ifndef CLASS_BASE64_H
#define CLASS_BASE64_H

/*
December 08, M. Spiekermann: Initial Version

This class provides static member functions used for encoding and decoding
the Base64 data format as specified in RFC 1341. The interface was designed
to interact kindly with the text atoms of nested lists. 

*/

#include <string>
#include <math.h>
using namespace std;

class Base64{

public:
   Base64();
   ~Base64(){};    

   int sizeDecoded(int text) {
             return (int)ceil( (3*(double)text)/4 );
   }
   int sizeEncoded(int bytes) {
             return ( (int)ceil( 4*(double)bytes/3 ) + (int)ceil( 4*(double)bytes/(3*72) ) );
   }
/*

These functions calculate the decoded or encoded size for a given
number of bytes.

*/


   void encode(char* bytes, int size, string& base64);
/*  

Encodes bytes of a buffer of size ~size~ into base64 format returned as string.  

*/ 
   int decode(string& text, char* bytes);
/*

Returns the number of bytes written in a buffer which has to be allocated
by the caller. The size can be estimated by the function sizedecoded().   

*/

   void decodeStream(istream& in, ostream& out);
   void encodeStream(istream& in, ostream& out);

/*

Encode or decode a stream of bas64 characters or bytes. 

*/


private:
   /* the characters used in the base64 format */
   static char base64Alphabet[];
   
   bool getNext(char& byte, istream& in);
   int  getIndex(char b);
   bool isAllowed(char b);
   void encode2(char* buffer, string& text, int size);
 
   bool endReached;
   int  currentPos;
   int  filled;

};

#endif

