/*

----
This file is part of SECONDO.

Copyright (C) 2020,
Faculty of Mathematics and Computer Science,
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


#include <stdlib.h>
#include <iostream>
#include <time.h>

using namespace std;

int main(int argc, char** argv){

  if(argc!=4){
    cout << "usage crnd <count> <minValue> <maxValue>" << endl;
    return 1;
  }
  int count = atoi(argv[1]);
  int minValue = atoi(argv[2]);
  int maxValue = atoi(argv[3]);
  if(count < 1){
    cerr << "invalid number of numbers" << endl;
    return 1;
  }
  if(minValue < 0){
     cerr << "invalid minimum value " << endl;
     return 1;
  }
  if(minValue > maxValue){
     cerr << "invalid interval " << endl;
     return 1;
  }
  srand(time(NULL));
  int range = (maxValue - minValue)+1;
  for(int i=0;i<count;i++){
     int r = rand() % range + minValue;
     cout << r << endl; 
  }




}

