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

#ifndef COUNTERPAIR_H
#define COUNTERPAIR_H

#include "Attribute.h"
#include "StandardTypes.h"

namespace eschbach {
    template<class T>
    class counterPair
{
  public:
  counterPair(T item, long frequency, long maxError);
  ~counterPair() {}
  //The actual element
  T item; 
  //The frequency of an element in the stream
  int frequency;
  //The bucket/window index - 1 Value inserted at 
  //an elements counter creation
  int maxError;



  //Getter und Setter
  T getItem(); 
  void setItem(T item);
  int getFrequency(); 
  void setFrequency();
  int getMaxError();
  //Setter not implemented, cause the error is always
  //fixed when a counter is created
  void setMaxError();
  };
}
#endif