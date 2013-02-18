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

#ifndef RASTER2_DISPLAYGRID2_H
#define RASTER2_DISPLAYGRID2_H

#include "DisplayTTY.h"

namespace raster2
{
  struct Displaygrid2 : DisplayFunction
  {
    virtual void Display(ListExpr type, ListExpr numType, ListExpr value)
    {
      if(nl->IsAtom(value) &&
         nl->AtomType(value) == SymbolType &&
         nl->SymbolValue(value) == Symbol::UNDEFINED())
      {
        cout << Symbol::UNDEFINED();
      }
      
      else
      {
        const string formatError = "Incorrect Data Format.";
        
        if(nl->ListLength(value) == 3)
        {
          bool bError = false;
          
          double x = GetNumeric(nl->First(value), bError);
          
          if(bError)
          {
            throw runtime_error(formatError);
          }
          
          double y = GetNumeric(nl->Second(value), bError);
          
          if(bError)
          {
            throw runtime_error(formatError);
          }
          
          double length = GetNumeric(nl->Third(value), bError);
          
          if(bError)
          {
            throw runtime_error(formatError);
          }
          
          cout << "grid2: (x = " << x << ", y = " << y <<
                  ", length = " << length << ")";
        }
        
        else
        {
          throw runtime_error(formatError);
        }
      }
    }
};

}
#endif /* RASTER2_DISPLAYGRID2_H */
