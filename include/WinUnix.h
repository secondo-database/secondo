/*

September 03, M. Spiekermann: Initial Version


This class consists of static functions which are used to hide
calls to different Win32 or Unix/Linux libraries, thus all system
dependent code should be isolated in this class.

*/

#ifndef CLASS_WINUNIX_H
#define CLASS_WINUNIX_H


class WinUnix {

public:
   WinUnix(){};
   ~WinUnix(){};

   static int getPageSize( void );
 
   static inline bool isLittleEndian() { return *(char *)&endian_detect == 1;}
   
private:
   static const int endian_detect;

};

#endif
