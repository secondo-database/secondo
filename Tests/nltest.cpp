#include <string>
#include <iostream>
#include "CTable.h"

using namespace std;

int
main() {
 
 int testcase = 0;
 NestedList listA;
 NestedList listB;
	
 cout << ++testcase << " reportVectorSizes():" << endl
      << listA.reportVectorSizes() << endl;

 
 String listStr1("(1 2 (3 5) 7 (9 2 3 4 5) (2 (3 4 (5 6)) 4))");
 ListExpr listExp1 =0;
 cout << ++testcase << " ReadfromString():" << endl
      << "Trying String: " << ">>" << listStr1 << "<<" << endl;
 if ( listA.ReadfromString(listStr1, listExp1) ) {
    cout << "Ok!" << endl;
 } else {
    cout << "Failed!" << endl;
 }
    
 String listStr2("");
 listA.WriteToString(listStr2, listExp1);
 cout << ++testcase << " WriteToString(): " << endl
      << "Result: " << ">>" << listStr2 << "<<" << endl;
 

}



