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
#include "DynamicLibrary.h"

int main()
{
  cout << "*** Test of loading dynamic libraries" << endl;
  DynamicLibrary dl1, dl2;
  cout << "--- Load should fail due to missing name." << endl;
  if ( dl1.Load( "" ) )
  {
    cout << "A miracle: an unnamed dynamic library was loaded!" << endl;
    dl1.Unload();
  }
  else
  {
    cout << "Error: " << dl1.GetLastErrorMessage() << endl;
  }
  cout << "--- Load should fail due to invalid library name." << endl;
  if ( dl1.Load( "Invalid Name" ) )
  {
    cout << "A miracle: dynamic library 'Invalid Name' was loaded!" << endl;
    dl1.Unload();
  }
  else
  {
    cout << "Error: " << dl1.GetLastErrorMessage() << endl;
  }
  cout << "--- Load of dynamic library 'libtdynlib1' should succeed." << endl;
  if ( dl1.Load( "libtdynlib1" ) )
  {
    cout << "Success: dynamic library 'tdynlib1' was loaded!" << endl;
  }
  else
  {
    cout << "Error: " << dl1.GetLastErrorMessage() << endl;
  }
  if ( dl1.IsLoaded() )
  {
    cout << "Dynamic library '" << dl1.GetLibraryName() << "' loaded." << endl;
    cout << "--- Get address of function 'dlcall1'" << endl;
    typedef long (*DynLibFunc)();
    DynLibFunc dlf1 = (DynLibFunc) dl1.GetFunctionAddress( "dlcall1" );
    if ( dlf1 != 0 )
    {
      cout << "Call the function in the dynamic library 1" << endl;
      cout << "Result 1a: " << (*dlf1)() << " (should be 2)" << endl;
      if ( dl2.Load( "libtdynlib2" ) )
      {
        DynLibFunc dlf2 = (DynLibFunc) dl2.GetFunctionAddress( "dlcall2" );
        if ( dlf2 != 0 )
        {
          cout << "Call the function in the dynamic library 2" << endl;
          cout << "Result 2a: " << (*dlf2)() << " (should be 4)" << endl;
          cout << "Result 2b: " << (*dlf2)() << " (should be 12)" << endl;
        }
        else
        {
          cout << "Error GetFunctionAddr dlcall2: " << dl2.GetLastErrorMessage() << endl;
        }
      }
      else
      {
        cout << "Load 2 Error: " << dl2.GetLastErrorMessage() << endl;
      }
      cout << "Result 1b: " << (*dlf1)() << " (should be 16)" << endl;
      if ( dl2.Unload() )
      {
        cout << "Success: dynamic library 'libtdynlib2' was unloaded!" << endl;
      }
      else
      {
        cout << "Error: " << dl1.GetLastErrorMessage() << endl;
      }
    }
    else
    {
      cout << "Error: " << dl1.GetLastErrorMessage() << endl;
    }
  }
  if ( dl1.Unload() )
  {
    cout << "Success: dynamic library 'libtdynlib1' was unloaded!" << endl;
  }
  else
  {
    cout << "Error: " << dl1.GetLastErrorMessage() << endl;
  }
  cout << "*** Test finished." << endl;
  return 0;
}

