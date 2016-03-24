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

#ifndef RASTER2_DISPLAYMSTYPE_H
#define RASTER2_DISPLAYMSTYPE_H

#include "DisplayTTY.h"

namespace raster2
{
  class Displaymstype: public DisplayFunction
  {
    virtual void Display(ListExpr type, ListExpr list)
    {
      ListExpr first = nl->First(list);
      std::cout << "Origin: (" << nl->RealValue(nl->First(first))
                << ", " << nl->RealValue(nl->Second(first))<< ")\n";
      std::cout << "Length: " << nl->RealValue(nl->Third(first)) 
                << ", " << "\n";
      std::cout << "Duration: " << nl->RealValue(nl->Fourth(first)) 
                << "\n";
      
      list = nl->Rest(list);
      first = nl->First(list);
      int rows = nl->IntValue(nl->First(first));
      int cols = nl->IntValue(nl->Second(first));
      int times = nl->IntValue(nl->Third(first));
      list = nl->Rest(list);
      
      std::cout << "Values: ";
      bool has_values = false;
      
      while(!nl->IsEmpty(list)) {
        ListExpr head = nl->First(list);
        list = nl->Rest(list);
        int row = nl->IntValue(nl->First(head));
        int col = nl->IntValue(nl->Second(head));
        int time = nl->IntValue(nl->Third(head));
       
        ListExpr values = nl->Fourth(head);
 
        for(int t = 0; t < times; ++t)
        {
            for(int r = 0; r < rows; ++r)
            {
                for(int c = 0; c < cols; ++c)
                {
                    int p = 1 + Cardinal(c + (r*cols) + (t*rows*cols));
                    ListExpr element = nl->Nth(p, values);
                    if(!nl->IsEqual(element,Symbol::UNDEFINED())) {
                      has_values = true;
                      std::cout << "\n        (" << (row + r) << ", "
                                                 << (col + c) << ", "
                                                 << (time + t) << "): "
                                << nl->ToString(element);
                    }
                }

            }
        }
      }
      
      if (!has_values)
      {
        std::cout << "none";
      }
    }
  };
}
#endif /* RASTER2_DISPLAYMSTYPE_H */
