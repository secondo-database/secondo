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

Last change: Nov. 2004, M. Spiekermann 

*/

#include <string>
#include <iostream>

#include "CTable.h"

using namespace std;

int main()
{
  CTable<string> ct( 5 );
  cout << ct.Size() << endl;
  ct[2] = "Anton";
  string& strRef = ct[2];
  cout << strRef << endl;
  strRef = "Antonia";
  ct[4] = "Berta";
  cout << ct[2] << endl;
  cout << ct.Size() << endl;
  cout << ct[4] << endl;
  cout << "1: " << ct.IsValid( 1 ) << endl;
  cout << "2: " << ct.IsValid( 2 ) << endl;
  cout << "3: " << ct.IsValid( 3 ) << endl;
  cout << "4: " << ct.IsValid( 4 ) << endl;
  cout << "5: " << ct.IsValid( 5 ) << endl;
  ct.Add( *new string("Dora") );
  ct.Add( *new string("Emil") );
  ct.Add( *new string("Floriane") );
  ct.Add( *new string("Gerda") );
  ct.Add( *new string("Hugo") );
  cout << "Size=" << ct.Size() << endl;
  
  // test Get and Put 
  cout << endl << "###### test Get and Put #######" << endl;
  string s1;
  ct.Get(5, s1);
  cout << "5: " << s1 << endl;
  string s2 = "Felix";
  ct.Put(5, s2);
  ct.Put(6, s2);
  
  // test iterators
  cout << endl << "###### test iterators #######" << endl;
  
  CTable<string>::Iterator it, it2;
  it2 = ct.Begin();
  it = it2++;
  cout << "it  " << *it  << ", i= " << it.GetIndex()  << endl;
  *it = "Kasimir";
  cout << "it2 " << *it2 << ", i2=" << it2.GetIndex() << endl;
  ct.Remove( 5 );
  for ( it = ct.Begin(); it != ct.End(); ++it )
  {
    cout << "it " << *it << ", i=" << it.GetIndex() << endl;
  }
  cout << "eos? " << it.EndOfScan() << endl;
  ++it;
  cout << "eos? " << it.EndOfScan() << endl;
  return 0;
}

