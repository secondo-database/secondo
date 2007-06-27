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
#ifndef SegmentH
#define SegmentH
#include "../RTree/coord.h"
class Segment
{
  public:
    RealCoordinate p1,p2;
    bool insideAbove;
    unsigned id;

    Segment(){}

    /*
    Segment(RealCoordinate c1,RealCoordinate c2, bool InsideAbove)
    {
      p1 = c1;
      p2 = c2;
      insideAbove = InsideAbove;
    }
    */

    Segment(RealCoordinate c1,RealCoordinate c2, int clockwise,
            unsigned segmentId)
    {

      //pre-condition: segment.p1 comes before segment.p2 accordingly with the
      //         cycle direction
    
      if ( ( ( c1 < c2 ) && ( clockwise) ) ||
           ( ( c1 > c2 ) && ( !clockwise) ) )
        set(c1,c2,false,segmentId);
      else
        set(c1,c2,true,segmentId);
    }

    Segment(RealCoordinate c1,RealCoordinate c2, bool insAbove,
             unsigned segmentId)
    {
      set(c1,c2,insAbove,segmentId);
    }

    void set(RealCoordinate c1,RealCoordinate c2, bool insAbove,
              unsigned segmentId)
    {
      this->p1 = c1;
      this->p2 = c2;

      this->insideAbove = insAbove;
      this->id=segmentId;
    }

    RealCoordinate getLP()
    {
      if (p1<=p2)
        return p1;
      else
        return p2;
    }

    RealCoordinate getRP()
    {
      if (p1>p2)
        return p1;
      else
        return p2;
    }

};
//---------------------------------------------------------------------------
#endif
