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
#include <iostream>
#include <string>

void text2list( string& in, string& out, string& err );

extern int xxdebug;

int main()
{
  char intext[1000];
  xxdebug = 0;
//  string in = "query Staedte feed filter[.Bev > 100000] consume";
  string in = "query Staedte select[.pop > 100000].";
  string out = "";
  string err = "";
  cout << "?> " << in << endl;
  while( cout << "?> " << flush, cin.getline(intext, 1000))
  {
    in = intext; out = ""; err = "";
    text2list( in, out, err );
    cout << "=> " << out << "\n";
  }
  return (0);
}

