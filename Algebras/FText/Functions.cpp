
/*
----
This file is part of SECONDO.

Copyright (C) 2007, 
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


#include "Functions.h"
#include <iostream>

/*

1 Some auxiliary Functions

*/



/*
1.4 ~removeState~

removes state from the set and decrements all elements in the set greater
than state by one.

*/
void removeState(std::set<int>& targets, int state){
  if(state<0){
    return;
  }
  targets.erase(state);
  std::set<int> result;
  std::set<int>::iterator it;
  for(it=targets.begin(); it!=targets.end();it++){
    if(*it>state){
      result.insert(*it -1); 
    }else{
      result.insert(*it);
    }
  }
  targets.clear();
  targets = result;
}

