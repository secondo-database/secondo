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

May, 2007 Leonardo Azevedo, Rafael Brand

*/
//---------------------------------------------------------------------------

#include "TurningPoint.h"
bool TurningPoint::getDirectionTurningPoint(const RealCoordinate &p,
                                            const RealCoordinate &p2,
                                            bool insideAbove,
                                            const RealCoordinate &v)

{
  //window's vertical edge
  if (v.x==p.x)
  {
    if (insideAbove)
      return false; //UP
    else
      return  true; //DOWN
  }
  else  //Horizontal edge
  {
    if (insideAbove)
    {
      if ( (p.x-p2.x)>0 ) //p2.x is located to the left of p.x
        return false; //RIGHT
      else
        return true; //LEFT
    }
    else
    {
      if ( (p.x-p2.x)>0 )//p2.x is located to the right of p.x
        return true; //LEFT
      else
        return  false; //RIGHT
    }
  }
}

//---------------------------------------------------------------------------
