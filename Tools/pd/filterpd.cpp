/*

02.07.2004 M. Spiekermann 

filterpd reads in a list of filenames and checks if the file 
ends with ".cpp" or ".h".

*/

#include <string>
#include <iostream>

using namespace std;

int main( int argc, char* argv[] ) {

  if ( argc < 2 ) {
    cout << "no files specified" << endl;
    return 1;
  }

  for (int i=1; i < argc; i++) {
  
    string file(argv[i]);
    int strLen = file.length();
    int p1 = file.rfind(".cpp");
    int p2 = file.rfind(".h");

    if (  p1 == abs(strLen-4)  ||  p2 == abs(strLen-2) ) {
      cout << file << endl;
    }
  }

  return 1;
}


