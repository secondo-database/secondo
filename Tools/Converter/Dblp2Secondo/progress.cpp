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

1 Progress.cpp

This little prrogram counts the lines coming from stdin.
After a number of lines (given as argument or 1000 as default),
a dot is written to stdout. This can be useful for the user while
processing large text files. The standard application will be in a
pipe together with the tee command.

*/

#include <iostream>
using namespace std;

int main(int argc , char* argv[]){
   long number = 0;
   if(argc==2)
     number = atol(argv[1]);
   else
     number = 1000; // default
   cout << " Progress " << endl;
   cout << "Every dot corresponds to " << number << " lines " << endl;
   long count=1;
   int maxlinelength=1024;
   char cdummy[maxlinelength];
   int idummy=maxlinelength;
   while (!cin.eof()){
      cin.getline(cdummy,idummy);
      count++;
      if( (count % number)==0){
         cout << ".";
         cout.flush();
         count = 1;
      }
   }

}

