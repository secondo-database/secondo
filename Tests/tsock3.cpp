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
using namespace std;
#include "SocketIO.h"
#include <time.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

int main( int argc, char* argv[] )
{
  cout << "Socket test program 3 (Rule set test)" << endl;

  SocketRuleSet ruleSet;
  if ( ruleSet.LoadFromFile( "tsock3.dat" ) )
  {
    cout << "RuleSet read from file 'tsock3.dat'." << endl;
  }
  else
  {
    ruleSet.AddRule( "132.176.69.0", "255.255.255.0" );
    cout << "Rule (132.176.69.0/255.255.255.0/ALLOW) added." << endl;
  }
  cout << "Enter IP address which should be checked: ";
  string ipAddr;
  cin >> ipAddr;
  if ( ruleSet.Ok( ipAddr ) )
  {
    cout << "Access for IP address " << ipAddr << " is allowed." << endl;
  }
  else
  {
    cout << "Access for IP address " << ipAddr << " is denied." << endl;
  }
  if ( ruleSet.StoreToFile( "tsock3.dat" ) )
  {
    cout << "RuleSet stored to file 'tsock3.dat'." << endl;
  }
  else
  {
    cout << "RuleSet could NOT be stored to file 'tsock3.dat'." << endl;
  }
  return EXIT_SUCCESS;
}

