#include <iostream>
#include <string>

using namespace std;

static long jval = 1;

extern "C"
{
  long dlcall1()
  {
    cout << "%%% Output from the dynamic library tdynlib1 %%%" << endl;
    jval *= 2;
    return (jval);
  }  
}

