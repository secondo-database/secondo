/*
22.11.06 M. Spiekermann

*/   

#include <iostream>
#include <fstream>
#include "ExampleReader.h"


using namespace std;

int main(int argc, char* argv[]) {

 assert(argc == 2);

 ExampleReader reader(argv[1]);

 bool ok = reader.parse();

 if (ok)
  return 0;
 return 1;


} 
