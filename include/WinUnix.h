/*

September 03, M. Spiekermann: Initial Version


This class consists of static functions which are used to hide
calls to different Win32 or Unix/Linux libraries, thus all system
dependent code should be isolated in this class.

*/

class WinUnix {

public:
   WinUnix(){};
   ~WinUnix(){};

   static int getPageSize( void );

};
