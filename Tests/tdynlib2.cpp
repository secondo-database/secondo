#include <iostream>
#include <string>

using namespace std;

static long ival = 0;

extern "C" long dlcall1();

extern "C"
{
  long dlcall2()
  {
    cout << "%%% Output from the dynamic library tdynlib2 %%%" << endl;
    ival += dlcall1();
    return (ival);
  }  
}

