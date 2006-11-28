/*
22.11.06 M. Spiekermann

*/   

#include <iostream>
#include <fstream>
#include "ExampleReader.h"



void test_rms(string suf, string s)
{ 
  string s1="x.y";
  cout << s << " - ";
  removeSuffix(suf, s);
  cout << s << endl;
} 


using namespace std;

int main(int argc, char* argv[]) {

 test_rms("y", "x.y");
 test_rms("y", "x.yy");
 test_rms("Algebra", "StandardAlgebra");

 assert(argc == 2);

 ExampleReader reader(argv[1]);

 bool ok = reader.parse();

 ExampleInfo ex;
 ex.opName = "test1";
 ex.number = 1;
 ex.signature = "bool x int -> bool";
 ex.example = "query test1(TRUE, 5)";
 ex.result = "TRUE";

 for (int i=0; i < 10; i++) {
   reader.add("test1", i, ex);
 }

 reader.write();

 if (ok)
  return 0;
 return 1;


} 
