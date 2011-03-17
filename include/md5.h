
/* 

MD5.H - header file for MD5C.C


*/

/* 


Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.



These notices must be retained in any copies of any part of this
documentation and/or software.

2009: changed to be usable as c++ classes


*/

#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>



/* 
MD5 context. 

*/
typedef struct {
  uint32_t state[4];                                   /* state (ABCD) */
  uint32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

typedef unsigned char *POINTER;

class MD5{
  public:
     static void MD5Init(MD5_CTX* context);
     static void MD5Update(MD5_CTX *context, 
                           const unsigned char *input, 
                           const unsigned int len);
     static void MD5Final(unsigned char digest[16] , 
                          MD5_CTX *context);

     static void md5(const char* s, unsigned char digest[16]){
         MD5_CTX context;
         MD5Init(&context);
         unsigned int len = strlen(s);
         unsigned char* s2 = (unsigned char*) s;
         MD5Update(&context, s2, len);
         MD5Final(digest,&context);
     }

     static std::string toString(const unsigned char digest[16]){
        std::ostringstream oss;
        oss << std::hex; 
        for(int i=0; i<16;i++){
          if(digest[i] <16){
            oss<<'0';
          }
          oss << (short)digest[i];
        }
        return oss.str();
     }    

     static char * unix_encode(const char *pw, const char *salt); 


  private:

     static void MD5Transform(uint32_t state[4], 
                              const unsigned char block[64]);
     static void Encode(unsigned char *output, 
                        const uint32_t *input, 
                        const unsigned int len);
     static void Decode(uint32_t * output, 
                        const unsigned char *input, 
                        const unsigned int len);
     static void MD5_memcpy(POINTER output, 
                            const POINTER input, 
                            const unsigned int len);
     static void MD5_memset(POINTER output, 
                            const int value, 
                            const unsigned int len);

     static unsigned char PADDING[64];


};

#endif

