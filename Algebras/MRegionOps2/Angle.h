/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[pow] [\verb+^+]

[1] Headerfile 

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#ifndef ANGLE_H_
#define ANGLE_H_

#include <gmpxx.h>
#include "Vector2D.h"

namespace mregionops2 {

class Angle
{
public:

  Angle()
  {
    angle = Infinite;
  }

  Angle(mpq_class a)
  {
    angle = a;
    Normalize();
  }
  
  Angle(Vector2D v)
  {

// use sin2 = y2 / (x2+y2)
// sin is a reelle number, not exact
    mpq_class xSquare = v.GetX() * v.GetX();
    mpq_class ySquare = v.GetY() * v.GetY();

    if (v.GetX() >= 0)
    {
      if (v.GetY() >= 0)
      {
//      Quadrant 1
        angle = ySquare / (xSquare + ySquare);
      }
      else
      {
//      Quadrant 4
        angle = 3 + xSquare / (xSquare + ySquare);
      }
    }
    else
    {
      if (v.GetY() >= 0)
      {
//      Quadrant 2
        angle = 1 + xSquare / (xSquare + ySquare);
      }
      else
      {
//      Quadrant 3
        angle = 2 + ySquare / (xSquare + ySquare);
      }
    }
  }

  Angle GetOpposite();
  
  inline bool operator <(Angle& a) {

        if (angle < a.angle) return true;
        return false;
    }
  
  inline Angle operator -(Angle& a) {

        mpq_class diff =  angle - a.angle;
        if (diff < 0) diff = diff + 4;
        return Angle(diff);
    }

  inline mpq_class GetAngle()
  {
    return angle;
  }

  inline bool IsZero()
  {
    return angle == 0;
  }
  

  inline bool IsInfinite()
  {
    return angle == Infinite;
  }  

private:

  const static int Maxvalue = 4;

  // is infinite
  const static int Infinite = Maxvalue + 1;
  void Normalize();

  mpq_class angle;

};

ostream& operator <<(ostream& o, Angle& a);

}
#endif
