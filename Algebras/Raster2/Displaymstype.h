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
    virtual void Display(ListExpr type,  ListExpr numType, ListExpr value)
    {
      NList list(value);
      
      std::cout << "Origin: (" << list.first().first().realval()
                << ", " << list.first().second().realval() << ")\n";
      std::cout << "Length: " << list.first().third().realval() << ", " << "\n";
      std::cout << "Duration: " << list.first().fourth().realval() << "\n";
      
      list.rest();
      int rows = list.first().first().intval();
      int cols = list.first().second().intval();
      int times = list.first().third().intval();
      list.rest();
      
      std::cout << "Values: ";
      bool has_values = false;
      
      while (!list.isEmpty())
      {
        NList head = list.first();
        list.rest();
        int row = head.first().intval();
        int col = head.second().intval();
        int time = head.third().intval();
        
        NList values = head.fourth();
        
        for(int t = 0; t < times; ++t)
        {
            for(int r = 0; r < rows; ++r)
            {
                for(int c = 0; c < cols; ++c)
                {
                    NList element = values.elem(
                            1 + Cardinal(c + (r*cols) + (t*rows*cols)));

                    if(!element.isSymbol(Symbol::UNDEFINED()))
                    {
                      has_values = true;
                      std::cout << "\n        (" << (row + r) << ", "
                                                 << (col + c) << ", "
                                                 << (time + t) << "): "
                                << element.convertToString();
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
