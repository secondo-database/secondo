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

Feb. 2006 M. Spiekermann. The TestRunner is now a command option for SecondoTTY and
not an stand alone program. This saves time for linking and saves diskspace, since both
applications hav minimal differences.

*/

/*
Feb 2006, M. Spiekermann
   
1 The Main Routine

Main routine for ~SecondoBDB~ and ~SecondoCS~. This source file will be compiled twice. The
resulting object files will be used to link together ~SecondoBDB~ and ~SecondoCS~. There will
be some shell scripts which invoke the above programs with different options, e.g.

SecondoTTYBDB
SecondoPL
TestRunner

SecondoTTYCS
SecondoPLCS
TestRunnerCS
  
*/   

#include "License.h"
#include "Application.h"

using namespace std;

// forward declarations
extern int SecondoTTYMode(const TTYParameter&);
extern int SecondoTestRunner(const TTYParameter&);
extern int SecondoPLMode(int, char **);

#ifndef SEC_TTYCS
extern int SecondoServerMode( const int, const char**);
#endif

int
main( const int argc, char* argv[] )
{
  TTYParameter tp(argc,argv);
  
#ifndef SEC_TTYCS
  if ( tp.isServerMode() )
    return SecondoServerMode(tp.numArgs, (const char**)tp.argValues);
#endif

#ifdef SECONDO_PL 
  if ( tp.isPLMode() )
    return SecondoPLMode(tp.numArgs,tp.argValues);
#endif

  cout << License::getStr() << endl;
  
  // Testrunner or TTY
  if ( !tp.CheckConfiguration() )
    return 1;
  
  if ( tp.isTestRunnerMode() )
    return SecondoTestRunner(tp);
  
  return SecondoTTYMode(tp);
}
