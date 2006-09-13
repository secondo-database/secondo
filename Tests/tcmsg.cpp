/*
Sept. 2006, M. Spiekermann

1 How to avoid problems with class ~CMsg~ 

On some systems, e.g. MAC OSX, the code below will crash. The problem is that
class ~X~ uses the global defined object cmsg. C++ does not specify in which order
global instances of classes will be created. Hence it is not guaranteed that cmsg
is properly initialized when the constructor function of ~X~ will be called. 

*/

#include "LogMsg.h"

class X {

 public:
 X() { cmsg.file("mytrace1.txt"); }

};

X x;

int main()
{

  cmsg.file("mytraces.txt");

  cmsg.info() << "hello world" << endl;
  cmsg.send();
}
