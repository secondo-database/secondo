/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/
#include <string>
#include <iostream>
#include "CTable.h"
#include "NestedList.h"

using namespace std;

int
main() {
 
 int testcase = 0;
 NestedList listA;
 NestedList listB;

 cout << "NodeRecord Size: " << sizeof(NodeRecord) << endl
      << "ConstRecord Size: " << sizeof(Constant) << endl 
      << endl;
 
 cout << ++testcase << " reportVectorSizes():" << endl
      << "listA: " << listA.reportVectorSizes() << endl;

 string listStr1("(create fifty : (rel(tuple((n int)))))");
 //string listStr1("(0 (1 2) (3 4))"); 
 //string listStr1("(1 2 (3 5) 7 (9 2 3 4 5) (2 (3 4 (5 6)) 4))");
 //string listStr1("(1 \"jhgjhg\" 6 <text> hallo dies inst ein recht langer und langweiliger Text, hier gibt es nichts zu erfahren. Wir wollen nur testen ob Text-Atome korrekt behandelt werden. </text---> \"anbsdfklsd sksdjf sdksdf sdfj asdkjf sdkjssd\" 7 (9 10))");
 ListExpr listExp1 =0;
 cout << ++testcase << " ReadFromString():" << endl
      << "Trying String: " << ">>" << listStr1 << "<<" << endl;
 if ( listA.ReadFromString(listStr1, listExp1) ) {
    cout << "Ok!" << endl;
 } else {
    cout << "Failed!" << endl;
 }

 cout << ++testcase << " reportVectorSizes():" << endl
      << "listA: " << listA.reportVectorSizes() << endl;
 
    
 string listStr2("");
 listA.WriteToString(listStr2, listExp1);
 cout << ++testcase << " WriteToString(): " << endl
      << "Result: " << ">>" << listStr2 << "<<" << endl;
 

 cout << ++testcase << " reportVectorSizes():" << endl
      << "listA: " << listA.reportVectorSizes() << endl;
 
 string listStr3("");
 ListExpr newExp1 = listA.CopyList(listExp1, &listB);
 listB.WriteToString(listStr3, newExp1);
 cout << ++testcase << " CopyList(): " << endl
      << "Result: " << ">>" << listStr3 << "<<" << endl;

 cout << ++testcase << " reportVectorSizes(): " << endl
      << "listA: " << listA.reportVectorSizes() << endl
      << "listB: " << listB.reportVectorSizes() << endl;
}



