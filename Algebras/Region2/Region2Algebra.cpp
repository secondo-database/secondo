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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Spatial Algebra

May 2013, Oliver Feuer


[TOC]

1 Overview

This implementation file contains the implementation of the classes ~Reg2PreciseHalfSegment~
and ~Region2~. 
The class ~Region2~ correspond to the memory representation for the type constructor ~region~.

2 Defines and Includes

*/

#include "Region2Algebra.h"


/*
1 Helper functions

*/
/*
1.1 ~reverseCycle2~

Changes the direction of a cycle.

*/
void reverseCycle2(vector<Reg2PrecisePoint>& cycle){

  for (unsigned int i=0; i < cycle.size()/2; i++)
  {
     Reg2PrecisePoint tmp = cycle[i];
     cycle[i] = cycle[cycle.size()-(i+1)];
     cycle[cycle.size()-(i+1)] = tmp;
  }
}


/*
1.1 ~getDir2~

Determines the direction of a cycle. If the cycle is in clockwise order, the
return value is true. If the cycle is directed counter clockwise, the result will
be false. 

*/
bool getDir2(const vector<Reg2PrecisePoint>& vp)
{
  // determine the direction of cycle
  int min = 0;
  for (unsigned int i=1; i < vp.size(); i++)
  {
    if (vp[i] < vp[min]) {
       min = i;
    }
  }

  bool cw;
  int s = vp.size();
  if ( vp[0] == vp[vp.size()-1] )
  {
    s--;
  }

  Reg2PrecisePoint a = vp[ (min - 1 + s ) % s ];
  Reg2PrecisePoint p = vp[min];
  Reg2PrecisePoint b = vp[ (min+1) % s];
  if ( cmp(a.x, p.x) == 0 ) // a -> p vertical
  {
    if ( cmp(a.y, p.y) > 0 ) 
    {
       cw = false;
    } 
    else 
    {
       cw = true;
    }
  } 
  else if ( cmp(p.x, b.x) == 0 ) //p -> b vertical
  {
    if ( cmp(p.y, b.y) > 0 ) 
    {
       cw = false;
    } 
    else 
    {
       cw = true;
    }
  } 
  else  // both segments are non-vertical
  {
    mpq_class m_p_a = (a.y-p.y) / (a.x-p.x);
    m_p_a.canonicalize();
    mpq_class m_p_b = (b.y-p.y) / (b.x-p.x);
    m_p_b.canonicalize();
    if ( cmp(m_p_a, m_p_b) > 0 )
    {
        cw = false;
    } 
    else 
    {
        cw = true;
    }
  }
  return cw;
}


/*
1.1 ~buildRegion2~

*/
Region2* buildRegion2(const int scale, 
                      vector< vector<Reg2PrecisePoint> >& cycles)
 {
  if (cycles.size() < 1)
  {
    cerr << "no face found within the cycles" << endl;
    return (new Region2(0));
  }
  
  multiset<Reg2PreciseHalfSegment> multiHSSet;
  list<Reg2PreciseHalfSegment> multiHSList;
  int e = 0;
  
  for (unsigned int i = 0; i < cycles.size(); i++)
  {
    vector<Reg2PrecisePoint> cycle = cycles[i];
    bool cw = getDir2(cycle);
//cout << "Cycle " << i << " mit Richtung " << cw << endl;    
    for (unsigned int j=0; j < cycle.size()-1; j++)
    {
       Reg2PrecisePoint lp,rp;
       bool small = cycle[j] < cycle[j+1];
       if (small)
       {
         lp = cycle[j];
         rp = cycle[j+1];
       } 
       else 
       {
         lp = cycle[j+1];
         rp = cycle[j];
       }
       Reg2PreciseHalfSegment hs(true,lp,rp);
       hs.attr.edgeno = e++;
       hs.attr.insideAbove = (cw && !small) || (!cw && small);
       hs.attr.insideAbove = !small;
       hs.attr.faceno=0;
       hs.attr.cycleno = 0;
       hs.attr.coverageno = 0;
       multiHSSet.insert(hs);
//    cout << "Kennung small " << small << endl ;
//    cout << hs << endl;
    }
  }

  Region2* reg = new Region2(0);
  reg->SetScaleFactor(scale, false);
  reg->StartBulkLoad();

  while (!multiHSSet.empty())
  {
    Reg2PreciseHalfSegment hs(*multiHSSet.begin());
    multiHSSet.erase(multiHSSet.begin());
    
    Reg2PreciseHalfSegment hs2(hs);
    hs2.SetLeftDomPoint(false);
    *reg += hs;
    *reg += hs2;
    multiHSList.push_back(hs);
//cout << "     >>>>>Merke: " << hs << endl;
//    cout << hs << "  " << hs2 << endl;
  }
  
    reg->EndBulkLoad();
  
    if (!reg->validateRegion2() )
    {
      delete reg;
      reg = 0;
    }
  
    return reg;
}


/*
1.1 output operators

*/
ostream& operator<<( ostream& o, const Reg2GridPoint& p )
{
  if( p.IsDefined() )
    o << "(" << p.x << ", " << p.y << ")";
  else
    o << Symbol::UNDEFINED();
  return o;
}

ostream& operator<<( ostream& o, const Reg2PrecisePoint& p )
{
  if( p.IsDefined() )
    o << "(" << p.x << ", " << p.y << ")";
  else
    o << Symbol::UNDEFINED();
  return o;
}

ostream& operator<<(ostream &os, const Reg2GridHalfSegment& hs)
{
  return os << "("
             <<"F("<< hs.attr.faceno
             <<") C("<<  hs.attr.cycleno
             <<") E(" << hs.attr.edgeno<<") DP("
             <<  (hs.IsLeftDomPoint()? "L":"R")
             <<") IA("<< (hs.attr.insideAbove? "A":"U")
             <<") Co("<<hs.attr.coverageno
             <<") PNo("<<hs.attr.partnerno
             <<") (("<< hs.GetLeftPointX() << ", " 
             << hs.GetLeftPointY() 
             << ") ("<< hs.GetRightPointX() << ", " 
             << hs.GetRightPointY() 
             <<")) ";
}

ostream& operator<<(ostream &os, const Reg2PrecHalfSegment& hs)
{
  return os << "lx "   << hs.getlxNumPosition() << " " 
            << hs.getlxDenPosition() 
            << " " << hs.getlxNumInts() << " " << hs.getlxDenInts()
            << ", ly " << hs.getlyNumPosition() << " " 
            << hs.getlyDenPosition() 
            << " " << hs.getlyNumInts() << " " << hs.getlyDenInts()
            << ", rx " << hs.getrxNumPosition() << " " 
            << hs.getrxDenPosition() 
            << " " << hs.getrxNumInts() << " " << hs.getrxDenInts()
            << ", ry " << hs.getryNumPosition() << " " 
            << hs.getryDenPosition() 
            << " " << hs.getryNumInts() << " " << hs.getryDenInts(); 
}

ostream& operator<<(ostream &os, const Reg2PreciseHalfSegment& hs)
{
  return os << "("
             <<"F("<< hs.attr.faceno
             <<") C("<<  hs.attr.cycleno
             <<") E(" << hs.attr.edgeno<<") DP("
             <<  (hs.IsLeftDomPoint()? "L":"R")
             <<") IA("<< (hs.attr.insideAbove? "A":"U")
             <<") Co("<<hs.attr.coverageno
             <<") PNo("<<hs.attr.partnerno
             <<") ("<< hs.GetLeftPoint() << " "
             << hs.GetRightPoint() <<") ";
}


/*
1 Functions of Reg2PreciseHalfSegment

*/
/*
1.1 ~Intersects~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::Intersects( 
                const Reg2PreciseHalfSegment& hs ) const
{
  mpq_class k(0), a(0), K(0), A(0);
  if ( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  Reg2PrecisePoint hs_lp = hs.GetLeftPoint(),
               hs_rp = hs.GetRightPoint();

  if ( cmp(lp.x, rp.x) == 0 &&
       cmp(hs_lp.x, hs_rp.x) == 0 )
    // both segments are vertical
  {
    if ( cmp(lp.x, hs_lp.x) == 0 &&
         (( cmp(hs_lp.y, lp.y)<=0 && cmp(lp.y, hs_rp.y)<=0 ) ||
          ( cmp(hs_lp.y, rp.y)<=0 && cmp(rp.y, hs_rp.y)<=0 ) ||
          ( cmp(lp.y, hs_lp.y)<=0 && cmp(hs_lp.y, rp.y)<=0 ) ||
          ( cmp(lp.y, hs_rp.y)<=0 && cmp(hs_rp.y, rp.y)<=0 )) )
      return true;
    return false;
  }

  if ( cmp(lp.x, rp.x) != 0 )
    // this segment is not vertical
  {
    k = (rp.y - lp.y) / (rp.x - lp.x);
    k.canonicalize();
    a = lp.y - k * lp.x;
    a.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) != 0 )
    // hs is not vertical
  {
    K = (hs_rp.y - hs_lp.y) / (hs_rp.x - hs_lp.x);
    K.canonicalize();
    A = hs_lp.y - K * hs_lp.x;
    A.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) == 0 )
    //only hs is vertical
  {
    mpq_class y0 = k * hs_lp.x + a;
    y0.canonicalize();

    if ( cmp(lp.x, hs_lp.x) <= 0 &&
         cmp(hs_lp.x, rp.x) <= 0 )
    {
      if ( (cmp(hs_lp.y, y0) <= 0 &&
            cmp(y0, hs_rp.y) <= 0 ) ||
           (cmp(hs_rp.y, y0) <= 0 &&
            cmp(y0, hs_lp.y) <= 0 ) )
        // (Xl, y0) is the intersection point
        return true;
    } 
    return false;
  }

  if ( cmp(lp.x, rp.x) == 0 )
    // only this segment is vertical
  {
    mpq_class Y0 = K * lp.x + A;
    Y0.canonicalize();

    if ( cmp(hs_lp.x, lp.x) <= 0 &&
         cmp(lp.x, hs_rp.x) <= 0 )
    {
      if ( (cmp(lp.y, Y0) <= 0 &&
            cmp(Y0, rp.y) <= 0 ) ||
           (cmp(rp.y, Y0) <= 0 &&
            cmp(Y0, lp.y) <= 0 ) )
        // (xl, Y0) is the intersection point
        return true;
    }
    return false;
  }

  // both segments are non-vertical

  if ( cmp(k, K) == 0 )
    // both segments have the same inclination
  {
    if ( cmp(A, a) == 0 &&
        (( cmp(hs_lp.x, lp.x) <= 0 &&
           cmp(lp.x, hs_rp.x) <= 0 ) ||
         ( cmp(lp.x, hs_lp.x) <= 0 &&
           cmp(hs_lp.x, rp.x) <= 0 )) )
      // the segments are in the same straight line
      return true;
  }
  else
  {
    mpq_class x0 = (A - a) / (k - K);
    x0.canonicalize();
    // y0 = x0 * k + a;

    if( cmp(lp.x, x0) <= 0 &&
        cmp(x0, rp.x) <= 0 &&
        cmp(hs_lp.x, x0) <= 0 &&
        cmp(x0, hs_rp.x) <= 0 )
      // the segments intersect at (x0, y0)
      return true;
  }
  return false;
}


/*
1.1 ~InnerIntersects~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::InnerIntersects( 
                const Reg2PreciseHalfSegment& hs ) const
{
  mpq_class k(0), a(0), K(0), A(0);
  if ( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  Reg2PrecisePoint hs_lp = hs.GetLeftPoint(),
               hs_rp = hs.GetRightPoint();

  if ( cmp(lp.x, rp.x) == 0 &&
       cmp(hs_lp.x, hs_rp.x) == 0 )
    // both segments are vertical
  {
    if ( cmp(lp.x, hs_lp.x) != 0 )
      return false;

    mpq_class ylow, yup, hs_ylow, hs_yup;
    if ( cmp(lp.y, rp.y) < 0 )
    {
      ylow = lp.y;
      yup = rp.y;
    }
    else
    {
      ylow = rp.y;
      yup = lp.y;
    }

    if ( cmp(hs_lp.y, hs_rp.y) < 0 )
    {
      hs_ylow = hs_lp.y;
      hs_yup = hs_rp.y;
    }
    else
    {
      hs_ylow = hs_rp.y;
      hs_yup = hs_lp.y;
    }

    if( ylow >= hs_yup || yup <= hs_ylow )
      return false;
    return true;
  }

  if ( cmp(lp.x, rp.x) != 0 )
    // this segment is not vertical
  {
    k = (rp.y - lp.y) / (rp.x - lp.x);
    k.canonicalize();
    a = lp.y - k * lp.x;
    a.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) != 0 )
    // hs is not vertical
  {
    K = (hs_rp.y - hs_lp.y) / (hs_rp.x - hs_lp.x);
    K.canonicalize();
    A = hs_lp.y - K * hs_lp.x;
    A.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) == 0 )
    //only hs is vertical
  {
    mpq_class y0 = k * hs_lp.x + a;
    y0.canonicalize();

    if ( cmp(lp.x, hs_lp.x) <= 0 &&
         cmp(hs_lp.x, rp.x) <= 0 )
    {
      if ( (cmp(hs_lp.y, y0) < 0 &&
            cmp(y0, hs_rp.y) < 0 ) ||
           (cmp(hs_rp.y, y0) < 0 &&
            cmp(y0, hs_lp.y) < 0 ) )
        // (Xl, y0) is the intersection point
        return true;
    } 
    return false;
  }

  if ( cmp(lp.x, rp.x) == 0 )
    // only this segment is vertical
  {
    mpq_class Y0 = K * lp.x + A;
    Y0.canonicalize();

    if ( cmp(hs_lp.x, lp.x) < 0 &&
         cmp(lp.x, hs_rp.x) < 0 )
    {
      if ( (cmp(lp.y, Y0) <= 0 &&
            cmp(Y0, rp.y) <= 0 ) ||
           (cmp(rp.y, Y0) <= 0 &&
            cmp(Y0, lp.y) <= 0 ) )
        // (xl, Y0) is the intersection point
        return true;
    }
    return false;
  }

  // both segments are non-vertical

  if ( cmp(k, K) == 0 )
    // both segments have the same inclination
  {
    if ( cmp(A, a) != 0 ) //Parallel lines
      return false;

    //they are in the same straight line
    if ( cmp(rp.x, hs_lp.x) <= 0 || 
         cmp(hs_rp.x, lp.x) <= 0 )
      return false;
    return true;
  }
  else
  {
    mpq_class x0 = (A - a) / (k - K);
    x0.canonicalize();
    // y0 = x0 * k + a;

    if( cmp(lp.x, x0) <= 0 &&
        cmp(x0, rp.x) <= 0 &&
        cmp(hs_lp.x, x0) <= 0 &&
        cmp(x0, hs_rp.x) <= 0 )
      // the segments intersect at (x0, y0)
      return true;
  }
  return false;
}


/*
1.1 ~Crosses~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::Crosses( 
                const Reg2PreciseHalfSegment& hs ) const
{
  mpq_class k(0), a(0), K(0), A(0);
  if ( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  Reg2PrecisePoint hs_lp = hs.GetLeftPoint(),
               hs_rp = hs.GetRightPoint();

  if ( cmp(lp.x, rp.x) == 0 &&
       cmp(hs_lp.x, hs_rp.x) == 0 )
    // both segments are vertical
    return false;

  if ( cmp(lp.x, rp.x) != 0 )
    // this segment is not vertical
  {
    k = (rp.y - lp.y) / (rp.x - lp.x);
    k.canonicalize();
    a = lp.y - k * lp.x;
    a.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) != 0 )
    // hs is not vertical
  {
    K = (hs_rp.y - hs_lp.y) / (hs_rp.x - hs_lp.x);
    K.canonicalize();
    A = hs_lp.y - K * hs_lp.x;
    A.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) == 0 )
    //only hs is vertical
  {
    mpq_class y0 = k * hs_lp.x + a;
    y0.canonicalize();

    if ( cmp(lp.x, hs_lp.x) < 0 &&
         cmp(hs_lp.x, rp.x) < 0 )
    {
      if ( (cmp(hs_lp.y, y0) < 0 &&
            cmp(y0, hs_rp.y) < 0 ) ||
           (cmp(hs_rp.y, y0) < 0 &&
            cmp(y0, hs_lp.y) < 0 ) )
        // (Xl, y0) is the intersection point
        return true;
    } 
    return false;
  }

  if ( cmp(lp.x, rp.x) == 0 )
    // only this segment is vertical
  {
    mpq_class Y0 = K * lp.x + A;
    Y0.canonicalize();

    if ( cmp(hs_lp.x, lp.x) < 0 &&
         cmp(lp.x, hs_rp.x) < 0 )
    {
      if ( (cmp(lp.y, Y0) < 0 &&
            cmp(Y0, rp.y) < 0 ) ||
           (cmp(rp.y, Y0) < 0 &&
            cmp(Y0, lp.y) < 0 ) )
        // (xl, Y0) is the intersection point
        return true;
    }
    return false;
  }

  // both segments are non-vertical

  if ( cmp(k, K) == 0 )
    // both segments have the same inclination
  {
    return false;
  }
  else
  {
    mpq_class x0 = (A - a) / (k - K);
    x0.canonicalize();
    // y0 = x0 * k + a;

    if( cmp(lp.x, x0) < 0 &&
        cmp(x0, rp.x) < 0 &&
        cmp(hs_lp.x, x0) < 0 &&
        cmp(x0, hs_rp.x) < 0 )
      // the segments intersect at (x0, y0)
      return true;
  }
  return false;
}


/*
1.1 ~Intersection~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::Intersection( 
                const Reg2PreciseHalfSegment& hs, 
                Reg2PrecisePoint& resp ) const
{
  mpq_class k(0), a(0), K(0), A(0);
  if ( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  Reg2PrecisePoint hs_lp = hs.GetLeftPoint(),
               hs_rp = hs.GetRightPoint();

  resp.SetDefined( true );

  // Check for same endpoints
  if ( lp == hs_lp )
  {
    if ( !hs.Contains(rp) && !this->Contains(hs_rp) )
    {
        resp = lp;
        return true;
    } else {
        return false; //overlapping halfsegments
    }
  } 
  else if ( rp == hs_lp )
  {
    if (!hs.Contains(lp) && !this->Contains(hs_rp) ) {
      resp = rp;
      return true;
    } else {
      return false; //overlapping halfsegments
    }
  } 
  else if ( lp == hs_rp )
  {
    if ( !hs.Contains(rp) && !this->Contains(hs_lp) ) {
      resp = lp;
      return true;
    } else {
      return false; //overlapping halfsegments
    }
  } 
  else if ( rp == hs_rp )
  {
    if (!hs.Contains(lp) && !this->Contains(hs_lp)){
      resp = rp;
      return true;
    } else {
      return false; //overlapping halfsegments
    }
  }

  if ( cmp(lp.x, rp.x) == 0 &&
       cmp(hs_lp.x, hs_rp.x) == 0 )
    // both segments are vertical
    {
    if ( cmp(hs_lp.y, rp.y) == 0 ) {
      resp.Set(rp.x, rp.y);
      return true;
    }
    if ( cmp(lp.y, hs_rp.y) == 0 ) {
      resp.Set( lp.x, lp.y );
      return true;
    }
    return false;
  }

  if ( cmp(lp.x, rp.x) != 0 )
    // this segment is not vertical
  {
    k = (rp.y - lp.y) / (rp.x - lp.x);
    k.canonicalize();
    a = lp.y - k * lp.x;
    a.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) != 0 )
    // hs is not vertical
  {
    K = (hs_rp.y - hs_lp.y) / (hs_rp.x - hs_lp.x);
    K.canonicalize();
    A = hs_lp.y - K * hs_lp.x;
    A.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) == 0 )
    //only hs is vertical
  {
    mpq_class y0 = k * hs_lp.x + a;
    y0.canonicalize();

    if ( cmp(lp.x, hs_lp.x) <= 0 &&
         cmp(hs_lp.x, rp.x) <= 0 )
    {
      if ( (cmp(hs_lp.y, y0) <= 0 &&
            cmp(y0, hs_rp.y) <= 0 ) ||
           (cmp(hs_rp.y, y0) <= 0 &&
            cmp(y0, hs_lp.y) <= 0 ) ) {
        // (Xl, y0) is the intersection point
        resp.Set(hs_lp.x, y0);
        return true;
      }
    } 
    return false;
  }

  if ( cmp(lp.x, rp.x) == 0 )
    // only this segment is vertical
  {
    mpq_class Y0 = K * lp.x + A;
    Y0.canonicalize();

    if ( cmp(hs_lp.x, lp.x) <= 0 &&
         cmp(lp.x, hs_rp.x) <= 0 )
    {
      if ( (cmp(lp.y, Y0) <= 0 &&
            cmp(Y0, rp.y) <= 0 ) ||
           (cmp(rp.y, Y0) <= 0 &&
            cmp(Y0, lp.y) <= 0 ) ) {
        // (xl, Y0) is the intersection point
        resp.Set(lp.x, Y0);
        return true;
      }
    }
    return false;
  }

  if ( cmp(k, K) == 0 )
    // both segments have the same inclination
  {
    if ( rp == hs.lp ) {
      resp = rp;
      return true;
    }
    if ( lp == hs.rp ) {
      resp = lp;
      return true;
    }
    return false;
  }
  else
  {
    mpq_class x0 = (A - a) / (k - K);
    x0.canonicalize();
    mpq_class y0 = x0 * k + a;
    y0.canonicalize();

    if ( cmp(lp.x, x0) <= 0 &&
         cmp(x0, rp.x) <= 0 &&
         cmp(hs_lp.x, x0) <= 0 &&
         cmp(x0, hs_rp.x) <= 0 ) {
      // the segments intersect at (x0, y0)
      resp.Set( x0, y0 );
      return true;
    }
  }
  return false;
}


/*
1.1 ~Intersection~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::Intersection( 
        const Reg2PreciseHalfSegment& hs, 
        Reg2PreciseHalfSegment& reshs ) const
{
  mpq_class k(0), a(0), K(0), A(0);
  if ( !BoundingBox().Intersects( hs.BoundingBox() ) )
    return false;

  if ( *this == hs )
  {
    reshs = hs;
    return true;
  }

  Reg2PrecisePoint hs_lp = hs.GetLeftPoint(),
               hs_rp = hs.GetRightPoint();

  if ( cmp(lp.x, rp.x) == 0 &&
       cmp(hs_lp.x, hs_rp.x) == 0 )
    // both segments are vertical
  {
    mpq_class ylow, yup, hs_ylow, hs_yup;
    if ( cmp(lp.y, rp.y) < 0 )
    {
      ylow = lp.y;
      yup = rp.y;
    }
    else
    {
      ylow = rp.y;
      yup = lp.y;
    }

    if ( cmp(hs_lp.y, hs_rp.y) < 0 )
    {
      hs_ylow = hs_lp.y;
      hs_yup = hs_rp.y;
    }
    else
    {
      hs_ylow = hs_rp.y;
      hs_yup = hs_lp.y;
    }

    if ( cmp(hs_ylow, yup) < 0 && cmp(ylow, hs_yup) < 0 )
    {
      Reg2PrecisePoint p1, p2;

      if ( cmp(ylow, hs_ylow) > 0 )
        p1.Set( lp.x, ylow );
      else
        p1.Set( lp.x, hs_ylow );

      if ( cmp(yup, hs_yup) < 0 )
        p2.Set( lp.x, yup );
      else
        p2.Set( lp.x, hs_yup );

      reshs.Set( true, p1, p2 );
      return true;
    }
    else return false;
  }

  if ( cmp(lp.x, rp.x) == 0 ||
       cmp(hs_lp.x, hs_rp.x) == 0 )
    // one of the segments is vertical
    return false;

  if ( cmp(lp.x, rp.x) != 0 )
    // this segment is not vertical
  {
    k = (rp.y - lp.y) / (rp.x - lp.x);
    k.canonicalize();
    a = lp.y - k * lp.x;
    a.canonicalize();
  }

  if ( cmp(hs_lp.x, hs_rp.x) != 0 )
    // hs is not vertical
  {
    K = (hs_rp.y - hs_lp.y) / (hs_rp.x - hs_lp.x);
    K.canonicalize();
    A = hs_lp.y - K * hs_lp.x;
    A.canonicalize();
  }

  if ( cmp(k, K) == 0 && cmp(a, A) == 0 )
  {
    if ( cmp(rp.x, hs_lp.x) > 0 && cmp(lp.x, hs_rp.x) < 0 )
    {
      Reg2PrecisePoint p1, p2;
      if ( cmp(lp.x, hs_lp.x) > 0 )
        p1.Set( lp.x, lp.y );
      else
        p1.Set( hs_lp.x, hs_lp.y );

      if ( cmp(rp.x, hs_rp.x) < 0 )
        p2.Set( rp.x, rp.y );
      else
        p2.Set( hs_rp.x, hs_rp.y );

      reshs.Set( true, p1, p2 );
      return true;
    }
  }
  return false;
}


/*
1.1 ~Inside~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::Inside( 
        const Reg2PreciseHalfSegment& hs ) const
{
  return hs.Contains( GetLeftPoint() ) &&
         hs.Contains( GetRightPoint() );
}


/*
1.1 ~RayAbove~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::RayAbove( 
        const Reg2PrecisePoint& p, mpq_class &yIntersection ) const
{
    assert(p.IsDefined());
    if (this->IsVertical())
          return false;

    mpq_class xl = GetLeftPoint().x,
                yl = GetLeftPoint().y,
                xr = GetRightPoint().x,
                yr = GetRightPoint().y;

    if ( cmp(xl, p.x) == 0 && cmp(p.y, yl) < 0 )
    {
      yIntersection = yl;
      return true;
    }
    else if ( cmp(xl, p.x) < 0 && cmp(p.x, xr) <= 0 )
    {
      mpq_class k = (yr - yl) / (xr - xl);
      k.canonicalize();
      mpq_class a = (yl - k * xl);
      a.canonicalize();
      mpq_class y0 = k * p.x + a;
      y0.canonicalize();

      if (cmp(y0, p.y) > 0)
      {
        yIntersection = y0;
        return true;
      }
    }
    return false;
}


/*
1.1 ~RayDown~ of Reg2PreciseHalfSegment

*/
bool Reg2PreciseHalfSegment::RayDown( 
        const Reg2PrecisePoint& p, mpq_class &yIntersection ) const
{
    assert(p.IsDefined());
    if (this->IsVertical())
          return false;

    mpq_class xl = GetLeftPoint().x,
              yl = GetLeftPoint().y,
              xr = GetRightPoint().x,
              yr = GetRightPoint().y;

    // between is true, iff xl <= x <= xr.
    const bool between = cmp(xl, p.x) <= 0 && cmp(p.x, xr) <= 0;

    if (!between)
        return false;

    mpq_class k = (yr - yl) / (xr - xl);
    k.canonicalize();
    mpq_class a = (yl - k * xl);
    a.canonicalize();
    mpq_class y0 = k * p.x + a;
    y0.canonicalize();

    if (cmp(y0, p.y) > 0) // y0 > y: this is above p.
        return false;

    // y0 <= p: p is above or on this.

    yIntersection = y0;

    return true;
}



/*
1 Region2 class

*/
/*
1.1 Constructor ~Region2~ 

*/
Region2::Region2( const Region2& cr, bool onlyLeft ) :
StandardSpatialAttribute<2>( cr.IsDefined() ),
scaleFactor( cr.scaleFactor ),
gridCoordinates( cr.Size() ),
precCoordinates( cr.Size() ),
preciseCoordinates( 0 ),
bbox( cr.bbox ),
noComponents( cr.noComponents ),
ordered( true )
{ 
//  cout << "Region2(Region2, Bool)... " << endl;
//cout << "Constructor called..." << endl;
//cout << "onlyLeft is " << onlyLeft << endl;
  if( IsDefined() && cr.Size() > 0 ) 
  {
    assert( cr.IsOrdered() );

    if( !onlyLeft )
    {
      gridCoordinates.copyFrom(cr.gridCoordinates);
      precCoordinates.copyFrom(cr.precCoordinates);
      preciseCoordinates.copyFrom(cr.preciseCoordinates);
      precHSvector = cr.precHSvector;
    }
    else 
    {
      StartBulkLoad();
      Reg2PreciseHalfSegment hs;
      for( int i = 0; i < cr.Size(); i++ ) 
      {
        cr.Get( i, hs );
//cout << "i: " << i << " " << hs << endl;
        if ( hs.IsLeftDomPoint() )
          precHSvector.push_back(hs);
      }
      EndBulkLoad( false, false, false, false, false );
    } 
  }
}


/*
1.1 Constructor ~Region2~ 

*/
Region2::Region2( const Region& r ) :
StandardSpatialAttribute<2>(r.IsDefined()),
scaleFactor( 0 ),
gridCoordinates(0),
precCoordinates(0),
preciseCoordinates(0)
{
//  cout << "Region2(Region)... " << endl;
//cout << "Constructor with region started" << endl;
    Clear();
    if(  r.IsDefined() )
    {
      SetDefined( true);
      HalfSegment hs;
      Reg2PreciseHalfSegment phs;
      StartBulkLoad();
      int e = 0;
      for( int i = 0; i < r.Size(); i++ ) 
      {
        r.Get( i, hs );
        if (hs.IsLeftDomPoint())
        {
//cout << i << ": " << hs << endl;
          phs = Reg2PreciseHalfSegment(hs);
//cout << phs << endl;
          phs.attr.edgeno = e++;
          *this += phs;
//          precHSvector.push_back(phs);
          phs.SetLeftDomPoint(false);
          *this += phs;
//          precHSvector.push_back(phs);
        }
      }
      EndBulkLoad();
    }
    else {
      SetDefined( false );
    } 
//cout << "Constructor with region finished" << endl;
}


/*
1.1 Constructor ~Region2~ 

*/
Region2::Region2( const Rectangle<2>& r ) :
StandardSpatialAttribute<2>(r.IsDefined()),
scaleFactor( 0 ),
gridCoordinates(0),
precCoordinates(0),
preciseCoordinates(0)
{
//  cout << "Region2(Rect<2>)... " << endl;
    Clear();
    if(  r.IsDefined() )
    {
      SetDefined( true);
      Reg2PreciseHalfSegment hs;
      int partnerno = 0;
      mpq_class min0(r.MinD(0));
      mpq_class max0(r.MaxD(0));
      mpq_class min1(r.MinD(1));
      mpq_class max1(r.MaxD(1));

      Reg2PrecisePoint v1(max0, min1);
      Reg2PrecisePoint v2(max0, max1);
      Reg2PrecisePoint v3(min0, max1);
      Reg2PrecisePoint v4(min0, min1);

      if ( v1 == v2 || v2 == v3 || v3 == v4 || v4 == v1 )
      { // one interval is (almost) empty, so will be the region
        SetDefined( true );
        return;
      }

      SetDefined( true );
      StartBulkLoad();

      hs = Reg2PreciseHalfSegment(true, v1, v2);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v1);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      hs = Reg2PreciseHalfSegment(true, v2, v3);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v2);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      hs = Reg2PreciseHalfSegment(true, v3, v4);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v3);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      hs = Reg2PreciseHalfSegment(true, v4, v1);
      hs.attr.faceno = 0;         // only one face
      hs.attr.cycleno = 0;        // only one cycle
      hs.attr.edgeno = partnerno;
      hs.attr.partnerno = partnerno++;
      hs.attr.insideAbove = (hs.GetLeftPoint() == v4);
      *this += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      *this += hs;

      EndBulkLoad();
    }
    else {
      SetDefined( false );
    } 
}


/*
1.1 ~StartBulkLoad~ of Region2

*/
void Region2::StartBulkLoad()
{
//  cout << "Region2::StartBulkLoad..." << endl;
  ordered = false;
}


/*
1.1 ~EndBulkLoad~ of Region2

*/
void Region2::EndBulkLoad( bool sortit, bool setCoverageNo,
                           bool setPartnerNo, bool computeRegion, 
                           bool buildDb )
{
//  cout << "Region2::EndBulkLoad..." << sortit << " " 
//       << setCoverageNo 
//       << " " << setPartnerNo << " " << computeRegion 
//       << " " << buildDb 
//       << endl;
  if( !IsDefined() ) {
    Clear();
    return;
  }

  if ( sortit ) 
    sort(precHSvector.begin(), precHSvector.end());

  if ( setCoverageNo )
  {
    int currCoverageNo = 0;
    Reg2PreciseHalfSegment hs;
    
    for(unsigned int i= 0; i< precHSvector.size(); i++ )
    {
      hs = precHSvector[i];
      if ( hs.IsLeftDomPoint() )
        currCoverageNo++;
      else
        currCoverageNo--;

      precHSvector[i].attr.coverageno = currCoverageNo;
    }
  }

  if ( setPartnerNo )
    SetPartnerNo();

  if ( computeRegion )
    ComputeRegion();

  if ( buildDb )
    buildDbArrays();

  ordered = true; 
}


/*
1.1 ~Contains~ of Region2

*/
bool Region2::Contains( const Reg2PrecisePoint& p) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );

  if ( IsEmpty() || !p.IsDefined() )
  {
    return false;
  }
  if ( !p.Inside(bbox) )
  {
    return false;
  }
  assert( IsOrdered() );
  map<int, int> faceISN;
  Reg2PreciseHalfSegment hs;

  int coverno=0;
  unsigned int startpos=0;
  mpq_class y0;

  //1. find the right place
  for (startpos = 0; startpos < precHSvector.size() 
          && p >= precHSvector[startpos].GetDomPoint() ; startpos++)
    if ( precHSvector[startpos].GetDomPoint() == p )
      return true;

  if ( startpos == 0 ){   //p is smallest
    return false;
  } else if ( startpos == precHSvector.size() ){  //p is largest
    return false;
  }

  int i = startpos - 1;

  //2. deal with equal-x hs's
  hs = precHSvector[i];
  while( i > 0 &&
         cmp(hs.GetDomPoint().x, p.x) == 0 ){
    if( hs.Contains(p) )
    {
      return true;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    hs = precHSvector[--i];
  }

  // at this point, i is pointing to the last hs whose dp.x != p.x

  //3. get the coverage value
  coverno = hs.attr.coverageno;

  //4. search the region value for coverageno steps
  int touchedNo = 0;
  while( i >= 0 && touchedNo < coverno ){
    hs = precHSvector[i];
    if( hs.Contains(p) ){
      return true;
    }
    if( hs.IsLeftDomPoint() &&
        cmp(hs.GetLeftPoint().x, p.x) <= 0 &&
        cmp(p.x, hs.GetRightPoint().x) <= 0 )
    {
      touchedNo++;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    i--;  //the iterator
  }

  for( map<int, int>::iterator iter = faceISN.begin();
       iter != faceISN.end();
       iter++ ){
    if( iter->second % 2 != 0 ){
      return true;
    }
  } 
  return false;
}


/*
1.1 ~InnerContains~ of Region2

*/
bool Region2::InnerContains( const Reg2PrecisePoint& p) const
{ 
  assert( IsDefined() );
  assert( p.IsDefined() );

  if ( IsEmpty() || !p.IsDefined() )
  {
    return false;
  }
  if ( !p.Inside(bbox) )
  {
    return false;
  }
  assert( IsOrdered() );
  map<int, int> faceISN;
  Reg2PreciseHalfSegment hs;

  int coverno=0;
  unsigned int startpos=0;
  mpq_class y0;

  //1. find the right place
  for (startpos = 0; startpos < precHSvector.size() 
          && p >= precHSvector[startpos].GetDomPoint() ; startpos++)
    if ( precHSvector[startpos].GetDomPoint() == p )
      return false;

  if ( startpos == 0 ){   //p is smallest
    return false;
  } else if ( startpos == precHSvector.size() ){  //p is largest
    return false;
  }

  int i = startpos - 1;

  //2. deal with equal-x hs's
  hs = precHSvector[i];
  while( i > 0 &&
         cmp(hs.GetDomPoint().x, p.x) == 0 ){
    if( hs.Contains(p) )
    {
      return false;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    hs = precHSvector[--i];
  }

  // at this point, i is pointing to the last hs whose dp.x != p.x

  //3. get the coverage value
  coverno = hs.attr.coverageno;

  //4. search the region value for coverageno steps
  int touchedNo = 0;
  while( i >= 0 && touchedNo < coverno ){
    hs = precHSvector[i];
    if( hs.Contains(p) ){
      return false;
    }
    if( hs.IsLeftDomPoint() &&
        cmp(hs.GetLeftPoint().x, p.x) <= 0 &&
        cmp(p.x, hs.GetRightPoint().x) <= 0 )
    {
      touchedNo++;
    }
    if( hs.IsLeftDomPoint() && hs.RayAbove( p, y0 ) ){
      if( faceISN.find( hs.attr.faceno ) != faceISN.end() ){
        faceISN[ hs.attr.faceno ]++;
      } else {
        faceISN[ hs.attr.faceno ] = 1;
      }
    }
    i--;  //the iterator
  }

  for( map<int, int>::iterator iter = faceISN.begin();
       iter != faceISN.end();
       iter++ ){
    if( iter->second % 2 != 0 ){
      return true;
    }
  } 
  return false;
}


/*
1.1 ~Contains~ of Region2

*/
bool Region2::Contains( const Reg2PreciseHalfSegment& hs) const
{
  assert( IsDefined() );

  if ( IsEmpty() )
  {
    return false;
  }
  if ( !hs.GetLeftPoint().Inside(bbox) ||
       !hs.GetRightPoint().Inside(bbox) )
  {
    return false;
  }
  if ( !Contains(hs.GetLeftPoint()) ||
       !Contains(hs.GetRightPoint()) )
  {
    return false;
  }

  Reg2PreciseHalfSegment auxhs;

  //now we know that both endpoints of hs is inside region
  for( unsigned int i = 0; i < precHSvector.size(); i++ )
  {
    auxhs = precHSvector[i];
    if( auxhs.IsLeftDomPoint() ){
      if( hs.Crosses(auxhs) ){
        return false;
      } else if( hs.Inside(auxhs) ) { //hs is part of the border
        return true;
      }
    }
  }
  return true;
}


/*
1.1 ~InnerContains~ of Region2

*/
bool Region2::InnerContains( const Reg2PreciseHalfSegment& hs) const
{
  assert( IsDefined() );
  if( IsEmpty() ){
    return false;
  }
  if( !hs.GetLeftPoint().Inside(bbox) ||
      !hs.GetRightPoint().Inside(bbox) ){
    return false;
  }
  if( !InnerContains(hs.GetLeftPoint()) ||
      !InnerContains(hs.GetRightPoint()) ){
    return false;
  }
  Reg2PreciseHalfSegment auxhs;

//now we know that both endpoints of hs are 
//completely inside the region
  for( int i = 0; i < Size(); i++ ){
    auxhs = precHSvector[i];
    if( auxhs.IsLeftDomPoint() ){
      if( hs.Intersects( auxhs ) ){
        return false;
      }
    }
  }
  return true;
}


/*
1.1 ~HoleEdgeContain~ of Region2

*/
bool Region2::HoleEdgeContain( 
         const Reg2PreciseHalfSegment& hs ) const
{
  assert( IsDefined() );
  if( !IsEmpty() ) {
    return false;
  }
  Reg2PreciseHalfSegment auxhs;
  for( int i = 0; i < Size(); i++ ){
    Get( i, auxhs );
    if( auxhs.IsLeftDomPoint() &&
        auxhs.attr.cycleno > 0 &&
        hs.Inside( auxhs ) ){
      return true;
    }
  }
  return false;
}


/*
1.1 ~operator=~ of Region2

*/
Region2& Region2::operator=( const Region2& r )
{
//  cout << "Region2::operator=..." << endl;
  assert( r.IsOrdered() );
  ordered = true;
  scaleFactor = r.scaleFactor;
  gridCoordinates.copyFrom(r.gridCoordinates);
  precCoordinates.copyFrom(r.precCoordinates);
  preciseCoordinates.copyFrom(r.preciseCoordinates);
  precHSvector = r.precHSvector;
  bbox = r.bbox;
  noComponents = r.noComponents;
  return *this;
}


/*
1.1 ~operator+=~ of Region2

*/
Region2& Region2::operator+=( const Reg2PreciseHalfSegment& hs )
{
  assert(IsDefined());

  if( IsEmpty() )
    bbox = hs.BoundingBox();
  else
    bbox = bbox.Union( hs.BoundingBox() );

  if( !IsOrdered() )
  {
    precHSvector.push_back(hs);
  }
  else
  {
    if ( hs < precHSvector.front() )
      precHSvector.insert(precHSvector.begin(), hs);
    else if ( hs > precHSvector.back() )
      precHSvector.push_back(hs);
    else
    {
      for( vector<Reg2PreciseHalfSegment>::iterator 
           it=precHSvector.begin(); 
           it!=precHSvector.end(); it++ )
      if ( hs < *it)
      {
        precHSvector.insert(it, hs);
        break;
      }
    }
  }
  return *this;
}


/*
1.1 ~AddPreciseHalfsegment~ of Region2

*/
Region2& Region2::AddPreciseHalfsegment( const bool ldp, 
                                         const Reg2PrecisePoint& pl, 
                                         const Reg2PrecisePoint& pr)
{
  Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(ldp, pl, pr);
  return *this += hs;
}


/*
1.1 ~AddPreciseHalfsegment~ of Region2

*/
Region2& Region2::AddPreciseHalfsegment( const bool ldp, 
                                         const int xl, 
                                         const mpq_class xlp, 
                                         const int yl, 
                                         const mpq_class ylp, 
                                         const int xr, 
                                         const mpq_class xrp, 
                                         const int yr, 
                                         const mpq_class yrp, 
                                         const int scale)
{
  Reg2PrecisePoint pl = Reg2PrecisePoint(xlp, xl, ylp, yl);
  Reg2PrecisePoint pr = Reg2PrecisePoint(xrp, xr, yrp, yr);
  return AddPreciseHalfsegment(ldp, pl, pr);
}


/*
1.1 ~Intersects~ of Region2

*/
bool Region2::Intersects( const Region2 &r ) const
{ 
  assert( IsDefined() );
  assert( r.IsDefined() );
  if( IsEmpty() || r.IsEmpty() ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox() ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( Inside( r ) || r.Inside( *this ) ){
    return true;
  }
  Reg2PreciseHalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() &&
            hs1.Intersects( hs2 ) ){
          return true;
        }
      }
    }
  } 
  return false;
}


/*
1.1 ~Inside~ of Region2

*/
bool Region2::Inside( const Region2& r ) const
{
  assert( IsDefined() );
  assert( r.IsDefined() );

  if(!IsDefined() || !r.IsDefined() ){
//    cout << "Not defined!" << endl;
    return false;
  }
  if( IsEmpty() ){
//    cout << "is empty" << endl;
    return true;
  }
  if(r.IsEmpty()){
//    cout << "r is empty" << endl;
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( !r.BoundingBox().Contains( bbox ) ){
//    cout << "Boundingbox don't contains!" << endl;
    return false;
  }
  Reg2PreciseHalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      if( !r.Contains( hs1 ) ){
//        cout << "HS not in r" << endl;
        return false;
      }
    }
  }
  bool existhole = false,
       allholeedgeinside = true;
  for( int j = 0; j < r.Size(); j++ ){
    r.Get( j, hs2 );
    if( hs2.IsLeftDomPoint() &&
        hs2.attr.cycleno > 0 ){
    //hs2 is not masked by another face of region2
      if( !HoleEdgeContain( hs2 ) ){
        existhole=true;
        if( !Contains( hs2 ) ){
          allholeedgeinside=false;
        }
      }
    }
  } 
  if( existhole && allholeedgeinside ){
//    cout << "Hole not inside" << endl;
    return false;
  } 
  return true;
}


/*
1.1 ~Adjacent~ of Region2

*/
bool Region2::Adjacent( const Region2& r ) const
{ 
  assert( IsDefined() );
  assert( r.IsDefined() );
  if( IsEmpty() || r.IsEmpty() ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox() ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( Inside( r ) || r.Inside( *this ) )
    return false;

  Reg2PreciseHalfSegment hs1, hs2;
  bool found = false;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() && hs1.Intersects( hs2 ) ){
          if( hs1.Crosses( hs2 ) ){
            return false;
          }
          found = true;
        }
      }
    }
  }
  return found;
}


/*
1.1 ~Overlaps~ of Region2

*/
bool Region2::Overlaps( const Region2& r ) const
{ 
  assert( IsDefined() );
  assert( r.IsDefined() );
  if( IsEmpty() || r.IsEmpty() ){
    return false;
  }
  if( !BoundingBox().Intersects( r.BoundingBox() ) ){
    return false;
  }
  assert( IsOrdered() );
  assert( r.IsOrdered() );
  if( Inside( r ) || r.Inside( *this ) )
    return true;

  Reg2PreciseHalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ ){
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() ){
      for( int j = 0; j < r.Size(); j++ ){
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() ){
          if( hs2.Crosses( hs1 ) ){
            return true;
          }
        }
      }
    }
  } 
  return false;
}


/*
1.1 ~Area~ of Region2

*/
double Region2::Area() const
{
  assert( IsDefined() );
  int n = Size();
  double area = 0.0;
  mpq_class precArea(0);
  mpq_class a(0);
  mpq_class x0(0);
  mpq_class y0(0);
  mpq_class x1(0);
  mpq_class y1(0);

  // get minimum with respect to Y-dimension
  mpq_class minY (MIN(BoundingBox().MinD(1), +0.0));

  Reg2PreciseHalfSegment hs;
  for(int i=0; i < n; i++)
  {
    Get( i, hs );
    if( hs.IsLeftDomPoint() ){ // use only one halfsegment
      x0 = hs.GetLeftPoint().x;
      x0.canonicalize();
      x1 = hs.GetRightPoint().x;
      x1.canonicalize();
      // y0, y1 must be >= 0, so we correct them
      y0 = hs.GetLeftPoint().y - minY;
      y0.canonicalize();
      y1 = hs.GetRightPoint().y - minY;
      y1.canonicalize();
      a = (x1-x0) * ((y1+y0) / 2);
      a.canonicalize();
      if ( hs.attr.insideAbove ){
        a = -a;
      }
      precArea += a;
    }
  }

  area = precArea.get_d();
  return area;
}


/*
1.1 ~Translate~ of Region2

*/
void Region2::Translate( const double& x, 
                         const double& y, Region2& result ) const
{ 
  result.Clear();
  if( !IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  result.SetDefined( true );
  assert( IsOrdered() );
  Reg2PreciseHalfSegment hs;
  result.StartBulkLoad();
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs );
    hs.Translate( x, y );
    result += hs;
  }
  result.SetNoComponents( NoComponents() );
  result.EndBulkLoad( false, false, false, false, false );
}


/*
1.1 ~Compare~ of Region2

*/
int Region2::Compare( const Attribute* arg ) const
{
//  cout << "Region2::Compare..." << endl;
  Region2* cr = (Region2* )(arg);
  if ( !cr )
    return -2;

  if (!IsDefined() && (!cr->IsDefined())){
    return 0;
  }

  if(!IsDefined()){
    return -1;
  }
  if(!cr->IsDefined()){
    return 1;
  }
  if(Size()<cr->Size()){
    return -1;
  }
  if(Size()>cr->Size()){
    return 1;
  }
  if(Size()==0){ // two empty regions
    return 0;
  }

  int bboxCmp = bbox.Compare( &cr->bbox );
  if(bboxCmp!=0){
   return bboxCmp;
  }

  Reg2PreciseHalfSegment hs1, hs2;
  for( unsigned int i = 0; i < precHSvector.size(); i++ ) {
     hs1 = precHSvector[i];
     cr->Get( i, hs2 );
     int hsCmp = hs1.Compare(hs2);
     if(hsCmp!=0){
       return hsCmp;
     }
  }
  return 0;
}


/*
1.1 ~Print~ of Region2

*/
ostream& Region2::Print( ostream &os ) const
{
  os << "<";
  if( !IsDefined() ) {
    os << " undefined ";
  } else {
    Reg2PreciseHalfSegment hs;
    for( unsigned int i = 0; i < precHSvector.size(); i++ )
    {
      hs = precHSvector[i];
      os << " " << hs;
    }
  } 
  os << ">";
  return os;
}


/*
1.1 ~Clone~ of Region2

*/
Region2 *Region2::Clone() const
{
//  cout << "Region2::Clone..." << endl;
  return new Region2( *this );
}


/*
1.1 ~HashValue~ of Region2

*/
size_t Region2::HashValue() const
{
//  cout << "Region2::HashValue..." << endl;
  if(IsEmpty()) // subsumes !IsDefined()
    return 0;

  unsigned long h=0;
  Reg2PreciseHalfSegment hs;
  mpq_class x1, y1;
  mpq_class x2, y2;

  for( int i = 0; ((i < Size())&&(i<5)); i++ )
  {
    Get( i, hs );
    x1=hs.GetLeftPoint().x;
    y1=hs.GetLeftPoint().y;
    x2=hs.GetRightPoint().x;
    y2=hs.GetRightPoint().y;
    h=h+(unsigned long)mpq_class((5*x1 + y1)+ (5*x2 + y2)).get_d();
  }
  return size_t(h);
}


/*
1.1 ~CopyFrom~ of Region2

*/
void Region2::CopyFrom( const Attribute* right )
{
//  cout << "Region2::CopyFrom..." << endl;
  *this = *(const Region2 *)right;
}


/*
1.1 ~LogicSort~ of Region2

*/
struct LogicSortReg2PreciseHalfSegment {
    bool operator()( const  Reg2PreciseHalfSegment& hs1, 
                     const  Reg2PreciseHalfSegment& hs2 ) {
      return (hs1.LogicCompare(hs2) == -1);
    }
};

void Region2::LogicSort()
{
  if ( !IsDefined() ) {
   return;
  }

  sort(precHSvector.begin(), precHSvector.end(),  
                LogicSortReg2PreciseHalfSegment() );
  buildDbArrays();
  
  ordered = true;
}


/*
1.1 ~Clear~ of Region2

*/
void Region2::Clear()
{
//  cout << "Region2::Clear..." << endl;
  gridCoordinates.clean();
  precCoordinates.clean();
  preciseCoordinates.clean();
  precHSvector.clear();
  ordered = true;
  bbox.SetDefined(false);
}


/*
1.1 ~SetEmpty~ of Region2

*/
void Region2::SetEmpty()
{
//  cout << "Region2::SetEmpty..." << endl;
  Clear();
  SetDefined(true);
}

/*
1.1 ~SetPartnerNo~ of Region2

*/
//old version: 
void Region2::SetPartnerNo()
{
  if( !IsDefined() )
    return;

  int size = precHSvector.size();
  int* tmp = new int[size/2];
  memset(tmp,0,size*sizeof(int) / 2);
  
  Reg2PreciseHalfSegment hs;
  for( int i = 0; i < size; i++ )
  {
    Get( i, hs );
    if( hs.IsLeftDomPoint() )
    {
      tmp[hs.attr.edgeno] = i;
    }
    else
    {
      int p = tmp[hs.attr.edgeno];
      precHSvector[i].attr.partnerno = p;
      precHSvector[p].attr.partnerno = i;
    }
  }
  delete[] tmp;
}


/*
1.1 ~GetCycleDirection~ of Region2

*/
bool Region2::GetCycleDirection() const
{
/*
Preconditions:
* The region must represent just one cycle!!!!
* It is need that the edgeno stores the order that the half segments were
 typed, and the half segments must be sorted in the half segment order. In
 other words if hs1.attr.edgeno is less than hs2.attr.edgeno then hs1 was
 typed first than hs2.

This function has the purpose of choosing the A, P, and B points in order
to call the function that really computes the cycle direction.
As the point P is leftmost point then it is the left point of hs1 or
 the left point of hs2 because in the half segment order these two points
are equal.  Now the problem is to decide which of the right points are A
and B. At the first sight we could say that the point A is the right point
of the half segment with lowest partner number. However it is not true ever
because the APB connected points may be go over the
bound of the pointlist. This will be the case if the cycle is in the form
P,B,..,A and B,...,A,P. Nevertheless the segments are ordered in the half
segment order, and when the last half segment is been considered for choosing
 the APB connected points, the point A will be always the right point of the
last segment.

*/
  Reg2PrecisePoint pA, pP, pB;
  Reg2PreciseHalfSegment hs1, hs2;
  this->Get(0,hs1);
  this->Get(1,hs2);

  assert( hs1.GetLeftPoint()==hs2.GetLeftPoint() );
  pP = hs1.GetLeftPoint();
  // If we have the last half segment connected to the first half
  // segment, the difference //between their partner numbers is
  // more than one.
  if (abs(hs1.attr.edgeno - hs2.attr.edgeno)>1)
  {
    if (hs1.attr.edgeno > hs2.attr.edgeno)
    {
      pA = hs1.GetRightPoint();
      pB = hs2.GetRightPoint();
    }
    else
    {
      pA = hs2.GetRightPoint();
      pB = hs1.GetRightPoint();
    }
  }
  else
  {
    if (hs1.attr.edgeno < hs2.attr.edgeno)
    {
      pA = hs1.GetRightPoint();
      pB = hs2.GetRightPoint();
    }
    else
    {
      pA = hs2.GetRightPoint();
      pB = hs1.GetRightPoint();
    }
  }
  
  if ( cmp(pA.x, pP.x) == 0 ) {//A --> P is a vertical segment
    if ( cmp(pA.y, pP.y) > 0 ) {//A --> P directed downwards (case 1)
      return false; //Counterclockwise
    } else {//upwards (case 2)
      return true; // Clockwise
    }
  }
  if ( cmp(pB.x, pP.x) == 0 ) {//P --> B is a vertical segment
    if ( cmp(pP.y, pB.y) > 0 ){ //downwords (case 3)
      return false; //Conterclockwise
    } else {//upwards (case 4)
      return true; //Clockwise
    }
  }

  //compute the slopes of P-->A and P-->B
  mpq_class m_p_a = ( pA.y - pP.y ) / ( pA.x - pP.x );
  m_p_a.canonicalize();
  mpq_class m_p_b = ( pB.y - pP.y ) / ( pB.x - pP.x );
  m_p_b.canonicalize();
  if ( cmp(m_p_a, m_p_b) > 0 ) //case 5
    return false;//counterclockwise
  else  //case 6
    return true; //clockwise

}
  //cycleDirection: true (cycle is clockwise) / false
  // (cycle is counterclockwise)
  //It is need that the attribute insideAbove of the 
  //half segments represents the order that  their points 
  //were typed: true (left point, right point) /
  //false (right point, left point).

  
/*
1.1 ~IsCriticalPoint~ of Region2

*/
bool Region2::IsCriticalPoint( const Reg2PrecisePoint &adjacentPoint,
                               const int hsPosition ) const
{
  int adjPosition = hsPosition,
      adjacencyNo = 0,
      step = 1;   
  do
  {
    Reg2PreciseHalfSegment adjCHS;
    adjPosition+=step;
    if ( adjPosition<0 || adjPosition>=this->Size())
      break;
    adjCHS = precHSvector[adjPosition];
    if (!adjCHS.IsLeftDomPoint())
      continue;
    AttrType attr = adjCHS.GetAttr();
    //When looking for critical points, the partner of
    //the adjacent half segment found
    //cannot be consired.
    if (attr.partnerno == hsPosition)
      continue;
    if ( ( adjacentPoint==adjCHS.GetLeftPoint() ) ||
         ( adjacentPoint==adjCHS.GetRightPoint() ) )
      adjacencyNo++;
    else
    {
      if (step==-1)
        return false;
      step=-1;
      adjPosition=hsPosition;
    }
  }
  while (adjacencyNo<2);

  return (adjacencyNo>1);
}


/*
1.1 ~GetAdjacentHS~ of Region2

*/
bool Region2::GetAdjacentHS( const Reg2PreciseHalfSegment &hs,
                            const int hsPosition,
                            int &position,
                            const int partnerno,
                            const int partnernoP,
                            Reg2PreciseHalfSegment& adjacentCHS,
                            const Reg2PrecisePoint &adjacentPoint,
                            Reg2PrecisePoint &newAdjacentPoint,
                            bool *cycle,
                            int step) const
{
  bool adjacencyFound=false; 
  do
  {
    position+=step;
    if ( position<0 || position>=(int)precHSvector.size())
      break;

    adjacentCHS = precHSvector[position];
    if (partnernoP == position)
      continue;

    if ( adjacentPoint==adjacentCHS.GetLeftPoint() ){
        if (!cycle[position]){
          newAdjacentPoint = adjacentCHS.GetRightPoint();
          adjacencyFound = true;
        }
    }
    else if  ( adjacentPoint==adjacentCHS.GetRightPoint() ){
            if (!cycle[position]){
              newAdjacentPoint = adjacentCHS.GetLeftPoint();
              adjacencyFound = true;
            }
      }
      else
        break;
  }
  while (!adjacencyFound);

//  cout<<"adjacencyFound "<<adjacencyFound<<endl;

  return adjacencyFound;
}


/*
1.1 ~ComputeCycle~ of Region2

*/
void Region2::ComputeCycle( Reg2PreciseHalfSegment &hs,
                           int faceno,
                           int cycleno,
                           int &edgeno,
                           bool *cycle )
{

  Reg2PrecisePoint  nextPoint = hs.GetLeftPoint(),
                lastPoint = hs.GetRightPoint(),
                previousPoint, *currentCriticalPoint=NULL;
  AttrType attr, attrP;
  Reg2PreciseHalfSegment hsP;
  vector<SCycle2> sCycleVector;
  SCycle2 *s=NULL;
  
//cout << "F(" << faceno << ") Cy(" << cycleno << ") E(" 
//     << edgeno << ")" << endl << endl;     
  do
  {
     if (s==NULL)
     {
       //Update attributes
       attr = hs.GetAttr();
       attr.faceno=faceno;
       attr.cycleno=cycleno;
       attr.edgeno=edgeno;
       hs.SetAttr(attr);

       hsP = precHSvector[attr.partnerno];
       attrP = hsP.GetAttr();
       attrP.faceno=faceno;
       attrP.cycleno=cycleno;
       attrP.edgeno=edgeno;
       hsP.SetAttr(attrP);
       precHSvector[attrP.partnerno].SetAttr(attr);
       precHSvector[attr.partnerno].SetAttr(attrP);

       edgeno++;

       cycle[attr.partnerno]=true;
       cycle[attrP.partnerno]=true;

       if (this->IsCriticalPoint(nextPoint,attrP.partnerno))
         currentCriticalPoint = new Reg2PrecisePoint(nextPoint);

       s = new SCycle2(hs,attr.partnerno,hsP,attrP.partnerno,
                       currentCriticalPoint,nextPoint);
     }
     Reg2PreciseHalfSegment adjacentCHS;
     Reg2PrecisePoint adjacentPoint;
     bool adjacentPointFound=false;
     previousPoint = nextPoint;
     if (s->goToCHS1Right)
     {
       s->goToCHS1Right=GetAdjacentHS(s->hs1,
                                      s->hs2Partnerno,
                                      s->hs1PosRight,
                                      s->hs1Partnerno,
                                      s->hs2Partnerno,adjacentCHS,
                                      previousPoint,
                                      nextPoint,
                                      cycle,
                                      1);
       adjacentPointFound=s->goToCHS1Right;
// cout<<"flag 1 "<<adjacentPointFound<<" p1 "<<previousPoint<<endl;
     }
     if ( !adjacentPointFound && s->goToCHS1Left )
     {
       s->goToCHS1Left=GetAdjacentHS(s->hs1,
                                     s->hs2Partnerno,
                                     s->hs1PosLeft,
                                     s->hs1Partnerno,
                                     s->hs2Partnerno,
                                     adjacentCHS,
                                     previousPoint,
                                     nextPoint,
                                     cycle,
                                     -1);
       adjacentPointFound=s->goToCHS1Left;
// cout<<"flag 2 "<<adjacentPointFound<<" p2 "<<previousPoint<<endl;
     }
     if (!adjacentPointFound && s->goToCHS2Right)
     {
       s->goToCHS2Right=GetAdjacentHS(s->hs2,
                                      s->hs1Partnerno,
                                      s->hs2PosRight,
                                      s->hs2Partnerno,
                                      s->hs1Partnerno,
                                      adjacentCHS,
                                      previousPoint,
                                      nextPoint,
                                      cycle,
                                      1);
       adjacentPointFound=s->goToCHS2Right;
// cout<<"flag 3 "<<adjacentPointFound<<" p3 "<<previousPoint<<endl;
     }
     if (!adjacentPointFound && s->goToCHS2Left)
     {
       s->goToCHS2Left=GetAdjacentHS(s->hs2,
                                     s->hs1Partnerno,
                                     s->hs2PosLeft,
                                     s->hs2Partnerno,
                                     s->hs1Partnerno,
                                     adjacentCHS,
                                     previousPoint,
                                     nextPoint,
                                     cycle,
                                     -1);
       adjacentPointFound = s->goToCHS2Left;

// cout<<"flag 4 "<<adjacentPointFound<<" p4 "<<previousPoint<<endl;

     }

// cout << "F(" << faceno << ") Cy(" << cycleno 
// << ") E(" << edgeno << ")" << endl << endl;     
     if(!adjacentPointFound){
         cerr<<"previousPoint "<<previousPoint<<endl;
         cerr << "Problem in rebuilding cycle in a region " << endl;
         cerr << "no adjacent point found" << endl;
         cerr << "Halfsegments : ---------------     " << endl;

         for( unsigned int i = 0; i < precHSvector.size(); i++){
            cerr << i << " : " << (precHSvector[i]) << endl;
         }
         assert(adjacentPointFound); // assert(false)
     }
     sCycleVector.push_back(*s);

     if ( (currentCriticalPoint!=NULL) 
       && (*currentCriticalPoint==nextPoint) )
     {
       //The critical point defines a cycle, so it is need to
       //remove the segments
       //from the vector, and set the segment as not visited 
       //in the cycle array.
       //FirsAux is the first half segment with the critical 
       //point equals to criticalPoint.
       SCycle2 sAux,firstSCycle;

       do
       {
          sAux=sCycleVector.back();
          sCycleVector.pop_back();
          firstSCycle=sCycleVector.back();
          if (firstSCycle.criticalPoint==NULL)
            break;
          if (*firstSCycle.criticalPoint!=*currentCriticalPoint)
            break;
          cycle[sAux.hs1Partnerno]=false;
          cycle[sAux.hs2Partnerno]=false;
          edgeno--;
       }while(sCycleVector.size()>1);
       delete s; 
//when s is deleted, the critical point is also deleted.
       s = 0;
       if (sCycleVector.size()==1)
       {
         sCycleVector.pop_back();
         if(s){
           delete s;
         }
         s = new SCycle2(firstSCycle);
       }
       else{
         if(s){
           delete s;
         }
         s= new SCycle2(sAux);
       }
       hs = s->hs1;
       currentCriticalPoint=s->criticalPoint;
       nextPoint=s->nextPoint;
       continue;
     }

     if ( nextPoint==lastPoint )
     {
       //Update attributes
       attr = adjacentCHS.GetAttr();
       attr.faceno=faceno;
       attr.cycleno=cycleno;
       attr.edgeno=edgeno;
       hs.SetAttr(attr);

       hsP = precHSvector[attr.partnerno];
       attrP = hsP.GetAttr();
       attrP.faceno=faceno;
       attrP.cycleno=cycleno;
       attrP.edgeno=edgeno;
       hsP.SetAttr(attrP);
       precHSvector[attrP.partnerno].SetAttr(attr);
       precHSvector[attr.partnerno].SetAttr(attrP);

       edgeno++;

       cycle[attr.partnerno]=true;
       cycle[attrP.partnerno]=true;

       break;
     }
     hs = adjacentCHS;
     delete s;
     s=NULL;
  }
  while(1);
  if(s){
    delete s;
    s = 0;
  }
} 


/*
1.1 ~GetNewFaceNo~ of Region2

*/
int Region2::GetNewFaceNo(const Reg2PreciseHalfSegment& hsIn, 
                          const int startpos) const {

    // Precondition:
    // hsIn is the smallest (in halfsegment-order)
    // segment of a cycle.
    // startpos is the index of hsIn in the DBArray.

    if (hsIn.GetAttr().insideAbove) 
    {
        // hsIn belongs to a new face:
        return -1;
    }

    // Now we know hsIn belongs to a new hole and we
    // have to encounter the enclosing face.
    // This is done by searching the next halfsegment
    // maxHS 'under' hsIn.
    // Since we go downwards, the facenumber of maxHS
    // must be already known
    // and is equal to the facenumber of hsIn.

    mpq_class y0;
    mpq_class maxY0;
    Reg2PreciseHalfSegment hs;
    Reg2PreciseHalfSegment maxHS;
    bool hasMax = false;
    const Reg2PrecisePoint& p = hsIn.GetLeftPoint();
    const int coverno = hsIn.GetAttr().coverageno;
    int touchedNo = 0;
    int i = startpos -1;
    bool first = true;

    while (i >= 0 && touchedNo < coverno) 
    {

        hs = precHSvector[i];

        if (!hs.IsLeftDomPoint()) 
        {
          i--;
          continue;
        }

        if ( cmp(hs.GetLeftPoint().x, p.x) <= 0 &&
             cmp(p.x, hs.GetRightPoint().x) <= 0 ) 
        {
          touchedNo++;
        }

        if ( cmp(hs.GetRightPoint().x, p.x) != 0 &&
             hs.RayDown(p, y0)) {

            if (first ||
                 cmp(y0, maxY0) > 0 ||
                ( cmp(y0, maxY0) == 0 && hs > maxHS)) {

                // To find the first halfsegment 
                // 'under' hsIn
                // we compare them as follows:
                // (1) y-value of the intersection 
                //     point between a ray down from 
                //     the left point of hsIn and hs.
                // (2) halfsegment order.

                maxY0 = y0;
                maxHS = hs;
                first = false;
                hasMax = true;
            }
        }

        i--;
    }

    if (!hasMax) {
        cerr << "Problem in rebuilding cycle in a region " << endl;
        cerr << "No outer cycle found" << endl;
        cerr << "hsIn: " << hsIn << endl;
        cerr << "Halfsegments : ---------------     " << endl;
        Reg2PreciseHalfSegment hs;

        for ( unsigned int i = 0; i < precHSvector.size(); i++) 
        {
            cerr << i << " : " << (precHSvector[i]) << endl;
        }

        assert(false);
    }

    //the new cycle is a holecycle of the face ~maxHS.attr.faceno~
    return maxHS.GetAttr().faceno;  
}


/*
1.1 ~ComputeRegion~ of Region2

*/
void Region2::ComputeRegion()
{
  if( !IsDefined() )
    return;
  //array that stores in position i the 
  //last cycle number of the face i
  vector<int> face;
  //array that stores in the position ~i~
  //if the half segment hi had 
  //already the face number, the cycle
  //number and the edge number
  //attributes set properly, in other
  //words, it means that hi is already 
  //part of a cycle
  bool *cycle;
  int lastfaceno=0,
      faceno=0,
      cycleno = 0,
      edgeno = 0;
  bool isFirstCHS=true;

  int size = precHSvector.size();
  if (size==0)
    return;
   //Insert in the vector the first cycle of the first face
  face.push_back(0);
  cycle = new bool[size];
#ifdef SECONDO_MAC_OSX
  // something goes wrong at mac osx and the memset function
  for(int i=0;i<size;i++){
    cycle[i] = false;
  }
#else
  memset( cycle, 0, size*sizeof(bool) );
#endif
  for (int i=0; i<size; i++)
  {
    Reg2PreciseHalfSegment aux(precHSvector[i]);
    if ( aux.IsLeftDomPoint() && !cycle[i])
    {
      if(!isFirstCHS)
      {
        int facenoAux = GetNewFaceNo(aux,i);
//cout << "GetNewFaceNo = " << facenoAux << endl;
        if (facenoAux==-1)
        {
        //The lhs half segment will start a new face
          lastfaceno++;
          faceno = lastfaceno;
        //to store the first cycle number of the face lastFace
          face.push_back(0);
          cycleno = 0;
          edgeno = 0;
        }
        else
        {
        //The half segment ~hs~ belongs to an existing face
          faceno = facenoAux;
          face[faceno]++;
          cycleno = face[faceno];
          edgeno = 0;
        }
      }
      else
      {
        isFirstCHS = false;
      }
      ComputeCycle(aux, faceno,cycleno, edgeno, cycle); 
    }
  }
  delete [] cycle;
  noComponents = lastfaceno + 1;
}


/*
1.1 ~buildDbArrays~ of Region2

*/
void Region2::buildDbArrays()
{
//  cout << "Region2::buildDbArrays..." << endl;
  
  if (precHSvector.size() < 1) return;
  
  gridCoordinates.clean();
  precCoordinates.clean();
  preciseCoordinates.clean();
  
  Reg2PreciseHalfSegment hs;
  Reg2GridHalfSegment gHS;
  Reg2PrecHalfSegment pHS;
  
  for (unsigned int i=0; i < precHSvector.size(); i++)
  {
    hs = precHSvector[i];
    Reg2PrecisePoint lp = hs.GetLeftPoint();
    Reg2PrecisePoint rp = hs.GetRightPoint();

    mpz_t sFactor;
    mpz_init(sFactor);
    mpq_class sFac(0);
    uint sfactor;
    
    if (scaleFactor < 0)
    {
      sfactor = -scaleFactor;
      mpz_ui_pow_ui(sFactor, 10, sfactor);
      sFac = mpq_class(mpz_class(1), mpz_class(sFactor));
    }
    else
    {
      sfactor = scaleFactor;
      mpz_ui_pow_ui(sFactor, 10, sfactor);
      sFac = mpq_class(mpz_class(sFactor), mpz_class(1));
    }
    sFac.canonicalize();
    mpz_clear(sFactor);
    
    mpq_class lxs = lp.x * sFac;
    lxs.canonicalize();
    mpq_class lys = lp.y * sFac;
    lys.canonicalize();            
    mpq_class rxs = rp.x * sFac;
    rxs.canonicalize();
    mpq_class rys = rp.y * sFac;
    rys.canonicalize();            
    
    int lxi = (int)floor(lxs.get_d());
    int lyi = (int)floor(lys.get_d());
    Reg2GridPoint lgp(lxi, lyi);

    int rxi = (int)floor(rxs.get_d());
    int ryi = (int)floor(rys.get_d());
    Reg2GridPoint rgp(rxi, ryi);
//cout << lxi << " " << lyi << " " << rxi << " " << ryi << " ";
//cout <<  lgp << " " << rgp << endl;
    
    mpq_class lxr = lxs - lxi;
    lxr.canonicalize();
    mpq_class lyr = lys - lyi;
    lyr.canonicalize();
    mpq_class rxr = rxs - rxi;
    rxr.canonicalize();
    mpq_class ryr = rys - ryi;
    ryr.canonicalize();
//cout << lxr << " " << lyr << " " << rxr << " " << ryr << endl;
    
    gHS = Reg2GridHalfSegment(hs.IsLeftDomPoint(), lxi, lyi, 
                                                   rxi, ryi);
    gHS.SetAttr(hs.attr);
//cout << gHS << endl;
    gridCoordinates.Append(gHS);
    
    pHS = Reg2PrecHalfSegment(-1);
    pHS.SetlPointx(lxr, &preciseCoordinates);
    pHS.SetlPointy(lyr, &preciseCoordinates);
    pHS.SetrPointx(rxr, &preciseCoordinates);
    pHS.SetrPointy(ryr, &preciseCoordinates);
//cout << pHS << endl;
    precCoordinates.Append(pHS);

    gridCoordinates.Get(i, gHS);
//cout << gHS << endl;
    precCoordinates.Get(i, pHS);
//cout << pHS << endl;
//cout << pHS.GetlPointx(&preciseCoordinates) << " " 
//    << pHS.GetlPointy(&preciseCoordinates) << " " 
//    << pHS.GetrPointx(&preciseCoordinates) << " " 
//    << pHS.GetrPointy(&preciseCoordinates) << " " << endl;

    gridCoordinates.TrimToSize();
    precCoordinates.TrimToSize();
    preciseCoordinates.TrimToSize();
  }
}


/*
1.1 ~buildHSvector~ of Region2

*/
void Region2::buildHSvector()
{ 
//  cout << "Region2::buildHSvector..." << endl;
  Reg2PreciseHalfSegment hs;
  Reg2GridHalfSegment gHS;
  Reg2PrecHalfSegment pHS;
  
  precHSvector.clear();

//cout << "Build HS vector: " << endl;  
  for (int i=0; i < gridCoordinates.Size(); i++)
  {
    gridCoordinates.Get(i, gHS);
//cout << gHS << endl;
    precCoordinates.Get(i, pHS);
//cout << pHS << endl;
//cout << pHS.GetlPointx(&preciseCoordinates) << " " 
//     << pHS.GetlPointy(&preciseCoordinates) << " " 
//     << pHS.GetrPointx(&preciseCoordinates) << " " 
//     << pHS.GetrPointy(&preciseCoordinates) << " " << endl;

    mpz_t sFactor;
    mpz_init(sFactor);
    mpq_class sFac(0);
    uint sfactor;
    
    if (scaleFactor < 0)
    {
      sfactor = -scaleFactor;
      mpz_ui_pow_ui(sFactor, 10, sfactor);
      sFac = mpq_class(mpz_class(sFactor), mpz_class(1));
    }
    else
    {
      sfactor = scaleFactor;
      mpz_ui_pow_ui(sFactor, 10, sfactor);
      sFac = mpq_class(mpz_class(1), mpz_class(sFactor));
    }
    sFac.canonicalize();
    mpz_clear(sFactor);
        
    mpq_class lx = (pHS.GetlPointx(&preciseCoordinates) 
                        + gHS.GetLeftPointX()) * sFac;
    lx.canonicalize();
    mpq_class ly = (pHS.GetlPointy(&preciseCoordinates) 
                        + gHS.GetLeftPointY()) * sFac;
    ly.canonicalize();
    mpq_class rx = (pHS.GetrPointx(&preciseCoordinates) 
                        + gHS.GetRightPointX()) * sFac;
    rx.canonicalize();
    mpq_class ry = (pHS.GetrPointy(&preciseCoordinates) 
                        + gHS.GetRightPointY()) * sFac;
    ry.canonicalize();

    Reg2PrecisePoint pl = Reg2PrecisePoint(lx, ly);
    Reg2PrecisePoint pr = Reg2PrecisePoint(rx, ry);

    hs = Reg2PreciseHalfSegment(gHS.IsLeftDomPoint(), pl, pr);
    AttrType attr = AttrType(gHS.GetAttr());
    hs.SetAttr(attr);
    precHSvector.push_back(hs); 
  } 
}


/*
1.1 ~validateRegion2~ of Region2

*/
bool Region2::validateRegion2()
{
//  return true;
  
  Reg2PreciseHalfSegment hsi, hsj;
//cout << "Start validateRegion2: "<< endl;  
  for( unsigned int i = 0; i < precHSvector.size()-1; i++ )
  {
    Get( i, hsi );

    if (hsi.IsLeftDomPoint())
    {
      for( unsigned int j = i+1; j < precHSvector.size(); j++ )
      {
        Get( j, hsj );
      
        if (hsj.IsLeftDomPoint())
        {
          if (hsi.Intersects(hsj))
          {
//cout << "Intersection von " << i << " mit " << j << endl;
            if ((hsi.attr.faceno!=hsj.attr.faceno) ||
                (hsi.attr.cycleno!=hsj.attr.cycleno))
            {
              cout << "two cycles intersect with the ";
              cout << "following edges:";
              cout << hsj << " :: " << hsi << endl;
              return false;
            }
            else
            {
              if ((hsj.GetLeftPoint()==hsi.GetLeftPoint()) &&
                  (hsj.GetRightPoint()==hsi.GetRightPoint()))
              {
                cout << "two edges: " << hsj << " :: " << hsi
                     << " are the same!"
                     << endl;
                return false;
              }
              if ((hsj.GetLeftPoint()!=hsi.GetLeftPoint()) &&
                  (hsj.GetLeftPoint()!=hsi.GetRightPoint()) &&
                  (hsj.GetRightPoint()!=hsi.GetLeftPoint()) &&
                  (hsj.GetRightPoint()!=hsi.GetRightPoint()))
              {
                cout << "two edges: " << hsj << " :: " << hsi
                     << " of the same cycle intersect in middle!"
                     << endl;
                return false;
              }
            }
          }
        }
      }
    }
  }

  vector<Region2*> comps;
  Components(comps);
  
  Region2 reg2(0);
  vector<Region2*> regparts;
  vector<vector<Region2*> > faces;
  vector<vector<Region2*> > faceholes;
  for (unsigned int i = 0; i < comps.size(); i++)
  {
    comps[i]->getFaces(reg2);
    reg2.Components(regparts);
    faces.push_back(regparts);
    comps[i]->getHoles(reg2);
    reg2.Components(regparts);
    faceholes.push_back(regparts);
  }

  for (unsigned int i = 0; i < comps.size()-1; i++)
  {
    for (unsigned int j = i+1; j < comps.size(); j++)
    { 
      if ( faces[j][0]->Inside(*faces[i][0]) )
      {
        bool insidefound = false;
        for (unsigned int k = 0; k < faceholes[i].size() 
                                 && !insidefound; k++)
        {
          if ( faces[j][0]->Inside(*faceholes[i][k]) )
            insidefound = true;
        }
        if ( !insidefound )
        {
          cout << "one face is inside another face!" << endl;
          return false;
        }
      }
      if ( faces[i][0]->Inside(*faces[j][0]) )
      {
        bool insidefound = false;
        for (unsigned int k = 0; k < faceholes[j].size() 
                                  && !insidefound; k++)
        {
          if ( faces[i][0]->Inside(*faceholes[j][k]) )
            insidefound = true;
        }
        if ( !insidefound )
        {
          cout << "one face is inside another face!" << endl;
          return false;
        }
      }
    }
    for (unsigned int j = 0; j < faceholes[i].size(); j++)
    {
      if ( !faceholes[i][j]->Inside(*faces[i][0]) )
      {
        cout << "one hole is not inside the face!" << endl;
        return false;
      }
      for (unsigned int k = j+1; 
             k < faceholes[j].size(); k++)
        if ( faceholes[i][j]->Inside(*faceholes[i][k]) 
          || faceholes[i][k]->Inside(*faceholes[i][j]) )
        {
          cout << "one hole is inside another hole!" << endl;
          return false;
        }
    }
    
  }

//  cout << "Beende validateRegion2 mit OK = TRUE" << endl;
  return true;
}


/*
1.1 ~Components~ of Region2

*/
void Region2::Components( vector<Region2*>& components )
{
//cout << " Start Components..." << endl;
  vector<int> edgeno;
  components.clear();
  if ( IsEmpty() ) // subsumes IsDefined()
  { 
    return;
  }
  for (int i=0; i < noComponents; i++)
  {
   components.push_back(new Region2(1));
   components[i]->StartBulkLoad();
   edgeno.push_back(0);
  }
  Reg2PreciseHalfSegment hs;
  for (int i=0; i < Size(); i++)
  {
    Get(i,hs);
    if ( hs.IsLeftDomPoint() )
    {
      int face = hs.attr.faceno;
      hs.attr.faceno = 0;
      hs.attr.edgeno = edgeno[face]++; 
      (*components[face]) += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      (*components[face]) += hs;
    }
  }

  for (int i=0; i < noComponents; i++)
  {
    components[i]->SetNoComponents( 1 );
    components[i]->SetScaleFactor(scaleFactor, false);
    components[i]->EndBulkLoad();
  }
//cout << " Ende Components..." << endl;
}


/*
1.1 ~getFaces~ of Region2

*/
void Region2::getFaces(Region2& result) const
{
//cout << " Start getFaces..." << endl;
   if (!IsDefined())
   {
      result.SetDefined(false);
   }
   result.Clear();
   result.SetDefined(true);
   result.StartBulkLoad();
   int edgeno = 0;
   for (int i=0; i < Size(); i++)
   {
       Reg2PreciseHalfSegment hs;
       Get(i,hs);
       if (hs.IsLeftDomPoint())
       {
         if (hs.attr.cycleno==0) // only the outer cycles
         {
            hs.attr.edgeno = edgeno;
            result += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            result += hs;
            edgeno++;
         }
      }
   }

   result.SetScaleFactor(scaleFactor, false);
   result.EndBulkLoad();
//cout << " Ende getFaces..." << endl;
}


/*
1.1 ~getHoles~ of Region2

*/
void Region2::getHoles(Region2& result) const
{
//cout << " Start getHoles..." << endl;
   if (!IsDefined())
   {
      result.SetDefined(false);
   }
   result.Clear();
   result.SetDefined(true);
   result.StartBulkLoad();
   int edgeno = 0;
   for (int i=0; i < Size(); i++)
   {
       Reg2PreciseHalfSegment hs;
       Get(i,hs);
       if (hs.IsLeftDomPoint())
       {
         if (hs.attr.cycleno!=0) // not the outer cycle
         {
            hs.attr.edgeno = edgeno;
            hs.attr.insideAbove = ! hs.attr.insideAbove;
            result += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            result += hs;
            edgeno++;
         }
      }
   }

   result.SetScaleFactor(scaleFactor, false);
   result.EndBulkLoad();
//cout << " Ende getHoles..." << endl;
}


/*
1.1 ~SetOutType~ of Region2

*/
int Region2::outType = 0;

bool Region2::SetOutType(int i) {
  if ( i == 0 || i == 1 ) {
    outType = i;
    return true;
  }
  return false;
}


/*
1 Definition of the Region2-Algebra

*/
/*
1.1 ~Region2Property~

*/
ListExpr Region2Property()
{
  ListExpr listreplist = nl->TextAtom();
  nl->AppendText(listreplist,
   "(<int> (<face>*)), where <int> represents the "
   "scaleFactor, <face> is (<outercycle>"
   "<holecycle>*), <outercycle> and <holecycle> are"
   " (<precisePoint>*). Each <precisePoint>  "
   "consists of (<int> <int> <precisePart>), with "
   "<precisePart> as (<text> <text>) or empty list, "
   "representing X and Y values "
   "separated in an integer part and a text with a "
   "rational number representing the precise "
   "remainder (value between 0 and 1). Each precise "
   "coordinate is calculated by "
   "( integer-value + precisePart ) * "
   "10 ^(-scaleFactor)");
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
   "(2 ((((3 0 ())(10 1 ())(3 1 ()))((3 0 ('1/10' "
   "'1/10'))(3 0 ('1/10' '9/10'))(6 0 ('0' '8/10'))))))");
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
    "all <holecycle> must be completely "
    "within <outercycle>.");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
           nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(Region2::BasicType()),
           listreplist,
           examplelist,
           remarkslist)));
}

ListExpr Region2PropertyV1()
{
  ListExpr listreplist = nl->TextAtom();
  nl->AppendText(listreplist,
   "(<face>*), where <face> is (<outercycle>"
   "<holecycle>*), <outercycle> and <holecycle> are"
   " (<precisePoint>*). Each <precisePoint> consists"
   " of (<text> <text>) representing X and Y as real"
   " values with arbitrary precision. "
   "The predefined scaleFactor is 0.");
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
   "(((('1.298262036' '1.888212877')('1.11177987207'"
   " '10.112877')('5.29826225622263094' "
   "'10.22666432466989809822'))))");
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
   "all <holecycle> must be completely "
   "within <outercycle>.");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
           nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(Region2::BasicType()),
           listreplist,
           examplelist,
           remarkslist)));
}

ListExpr Region2PropertyV2()
{
  ListExpr listreplist = nl->TextAtom();
  nl->AppendText(listreplist,
    "(<face>*), where <face> is (<outercycle>"
    "<holecycle>*), <outercycle> and <holecycle> are "
    "<points>*. This is the same definition like the "
    "region data type, with a predefined scaleFactor "
    "of 0.");
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "(((3 0)(10 1)(3 1))((3.1 0.1)(3.1 0.9)"
    "(6 0.8)))");
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
    "all <holecycle> must be completely "
    "within <outercycle>.");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
           nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(Region2::BasicType()),
           listreplist,
           examplelist,
           remarkslist)));
}


/*
1.1 ~OutPoint~

*/
ListExpr OutPoint( Region2* r, int i, bool left )
{
  ListExpr precX, precY;
  
  Reg2GridHalfSegment gHS;
  Reg2PrecHalfSegment pHS;

  r->getgridCoordinates()->Get(i, gHS);
  r->getprecCoordinates()->Get(i, pHS);

  if ( left )
  {
    mpq_class x = pHS.GetlPointx(r->getpreciseCoordinates());
    mpq_class y = pHS.GetlPointy(r->getpreciseCoordinates());
    if ( cmp(x, 0) != 0 || cmp(y, 0) != 0 )
    {
      gmpTypeToTextType1( x, precX );
      gmpTypeToTextType1( y, precY );
      return nl->ThreeElemList(  nl->IntAtom( gHS.GetLeftPointX() ), 
                                 nl->IntAtom( gHS.GetLeftPointY() ), 
                                 nl->TwoElemList( precX, precY ) );
    }
    else
      return nl->ThreeElemList(  nl->IntAtom( gHS.GetLeftPointX() ), 
                                 nl->IntAtom( gHS.GetLeftPointY() ), 
                                 nl->TheEmptyList() );
  }
  else
  {
    mpq_class x = pHS.GetrPointx(r->getpreciseCoordinates());
    mpq_class y = pHS.GetrPointy(r->getpreciseCoordinates());
    if ( cmp(x, 0) != 0 || cmp(y, 0) != 0 )
    {
      gmpTypeToTextType1( x, precX );
      gmpTypeToTextType1( y, precY );
      return nl->ThreeElemList( nl->IntAtom( gHS.GetRightPointX() ), 
                                nl->IntAtom( gHS.GetRightPointY() ), 
                                nl->TwoElemList( precX, precY ) );
    }
    else
      return nl->ThreeElemList( nl->IntAtom( gHS.GetRightPointX() ), 
                                nl->IntAtom( gHS.GetRightPointY() ), 
                                nl->TheEmptyList() );
  } 
}


/*
1.1 ~OutPoint2~

*/
ListExpr OutPoint2( Region2* r, int i, bool left )
{
  ListExpr precX, precY;
  
  Reg2PreciseHalfSegment pHS;
  Reg2PrecisePoint pP;
  
  r->Get(i, pHS);
  if ( left )
  {
    pP = pHS.GetLeftPoint();
  }
  else
  {
    pP = pHS.GetRightPoint();
  }
    
  gmpTypeToTextType2( pP.x, precX );
  gmpTypeToTextType2( pP.y, precY );

  return nl->TwoElemList( precX, precY );
}


/*
1.1 ~OutRegion2~

*/
ListExpr OutRegion2( ListExpr typeInfo, Word value )
{
//  cout << "OutRegion2... " << endl;
//  return (nl->TheEmptyList());

  Region2* r = (Region2*)(value.addr);
  if ( !r->IsDefined() )
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }

  if (r->precHSvectorIsEmpty()) r->buildHSvector();
  
  if( r->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    Region2 *RCopy=new Region2(*r, true); // in memory
    RCopy->LogicSort();

    Reg2PreciseHalfSegment hs, hsnext;

    ListExpr regionNL = nl->TheEmptyList();
    ListExpr regionNLLast = regionNL;

    ListExpr faceNL = nl->TheEmptyList();
    ListExpr faceNLLast = faceNL;

    ListExpr cycleNL = nl->TheEmptyList();
    ListExpr cycleNLLast = cycleNL;

    ListExpr pointNL = nl->TheEmptyList();

// avoid uninitialized use
    int currFace = -999999, currCycle= -999999; 
    Reg2PrecisePoint outputP, leftoverP;
    bool left;

    for ( int i = 0; i < RCopy->Size(); i++ )
    {
      RCopy->Get( i, hs);
      if (i==0)
      {
        currFace = hs.attr.faceno;
        currCycle = hs.attr.cycleno;
        RCopy->Get( i+1, hsnext);

        if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
            ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
        {
          outputP = hs.GetRightPoint();
          left = false;
          leftoverP = hs.GetLeftPoint();
        }
        else 
        if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
           ((hs.GetRightPoint() == hsnext.GetRightPoint())) )
        {
          outputP = hs.GetLeftPoint();
          left = true;
          leftoverP = hs.GetRightPoint();
        }
        else
        {
          cerr << "\n" << __PRETTY_FUNCTION__ 
               << ": Wrong data format --- "
               << "discontiguous segments!" << endl
               << "\ths     = " << hs     << endl
               << "\thsnext = " << hsnext << endl;
          return nl->SymbolAtom(Symbol::UNDEFINED());
        }

        if (Region2::outType == 0) 
           pointNL = OutPoint( RCopy, i, left );
        if (Region2::outType == 1) 
           pointNL = OutPoint2( RCopy, i, left );
        
        if (cycleNL == nl->TheEmptyList())
        {
          cycleNL = nl->OneElemList(pointNL);
          cycleNLLast = cycleNL;
        }
        else
        {
          cycleNLLast = nl->Append( cycleNLLast, pointNL );
        }
      }
      else
      {
        if (hs.attr.faceno == currFace)
        {
          if (hs.attr.cycleno == currCycle)
          {
            outputP=leftoverP;

            if (hs.GetLeftPoint() == leftoverP)
            {
              left = true;
              leftoverP = hs.GetRightPoint();
            }
            else if (hs.GetRightPoint() == leftoverP)
            {
              left = false;
              leftoverP = hs.GetLeftPoint();
            }
            else
            {
              cerr << "\n" << __PRETTY_FUNCTION__ 
                   << ": Wrong data format --- "
                   << "discontiguous segment in cycle!" << endl
                   << "\thh        = " << hs << endl
                   << "\tleftoverP = " << leftoverP << endl;
              return nl->SymbolAtom(Symbol::UNDEFINED());
            }

            if (Region2::outType == 0) 
               pointNL=OutPoint( RCopy, i, left );
            if (Region2::outType == 1) 
               pointNL=OutPoint2( RCopy, i, left );

            if (cycleNL == nl->TheEmptyList())
            {
              cycleNL=nl->OneElemList(pointNL);
              cycleNLLast = cycleNL;
            }
            else
            {
              cycleNLLast = nl->Append(cycleNLLast, pointNL);
            }
          }
          else
          {
            if (faceNL == nl->TheEmptyList())
            {
              faceNL = nl->OneElemList(cycleNL);
              faceNLLast = faceNL;
            }
            else
            {
              faceNLLast = nl->Append(faceNLLast, cycleNL);
            }
            cycleNL = nl->TheEmptyList();
            currCycle = hs.attr.cycleno;

            RCopy->Get( i+1, hsnext);
            if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
                ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetRightPoint();
              left = false;
              leftoverP = hs.GetLeftPoint();
            }
            else 
            if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
               ((hs.GetRightPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetLeftPoint();
              left = true;
              leftoverP = hs.GetRightPoint();
            }
            else
            {
              cerr << "\n" << __PRETTY_FUNCTION__ 
                   << ": Wrong data format --- "
                   << "discontiguous segments in cycle!" << endl
                   << "\ths     = " << hs     << endl
                   << "\thsnext = " << hsnext << endl;
              return nl->SymbolAtom(Symbol::UNDEFINED());
            }

            if (Region2::outType == 0) 
               pointNL = OutPoint( RCopy, i, left );
            if (Region2::outType == 1) 
               pointNL = OutPoint2( RCopy, i, left );
            
            if (cycleNL == nl->TheEmptyList())
            {
              cycleNL = nl->OneElemList(pointNL);
              cycleNLLast = cycleNL;
            }
            else
            {
              cycleNLLast = nl->Append(cycleNLLast, pointNL);
            }
          }
        }
        else
        {
          if (faceNL == nl->TheEmptyList())
          {
            faceNL = nl->OneElemList(cycleNL);
            faceNLLast = faceNL;
          }
          else
          {
            faceNLLast = nl->Append(faceNLLast, cycleNL);
          }
          cycleNL = nl->TheEmptyList();

          if (regionNL == nl->TheEmptyList())
          {
            regionNL = nl->OneElemList(faceNL);
            regionNLLast = regionNL;
          }
          else
          {
            regionNLLast = nl->Append(regionNLLast, faceNL);
          }
          faceNL = nl->TheEmptyList();

          currFace = hs.attr.faceno;
          currCycle = hs.attr.cycleno;

          RCopy->Get( i+1, hsnext);
          if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
             ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetRightPoint();
            left = false;
            leftoverP = hs.GetLeftPoint();
          }
          else 
          if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
             ((hs.GetRightPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetLeftPoint();
            left = true;
            leftoverP = hs.GetRightPoint();
          }
          else
          {
            cerr << "\n" << __PRETTY_FUNCTION__ 
                 << ": Wrong data format --- "
                 << "discontiguous segments in cycle!" << endl
                 << "\ths     = " << hs     << endl
                 << "\thsnext = " << hsnext << endl;
            return nl->SymbolAtom(Symbol::UNDEFINED());
          }

          if (Region2::outType == 0) 
             pointNL = OutPoint( RCopy, i, left );
          if (Region2::outType == 1) 
             pointNL = OutPoint2( RCopy, i, left );

          if (cycleNL == nl->TheEmptyList())
          {
            cycleNL = nl->OneElemList(pointNL);
            cycleNLLast = cycleNL;
          }
          else
          {
            cycleNLLast = nl->Append(cycleNLLast, pointNL);
          }
        }
      }
    }

    if (faceNL == nl->TheEmptyList())
    {
      faceNL = nl->OneElemList(cycleNL);
      faceNLLast = faceNL;
    }
    else
    {
      faceNLLast = nl->Append(faceNLLast, cycleNL);
    }
    cycleNL = nl->TheEmptyList();

    if (regionNL == nl->TheEmptyList())
    {
      regionNL = nl->OneElemList(faceNL);
      regionNLLast = regionNL;
    }
    else
    {
      regionNLLast = nl->Append(regionNLLast, faceNL);
    }
    faceNL = nl->TheEmptyList();

    ListExpr factorNL = nl->IntAtom(RCopy->GetScaleFactor());
    
    RCopy->DeleteIfAllowed();
    
    if (Region2::outType == 0) 
       return nl->TwoElemList(factorNL, regionNL);
    if (Region2::outType == 1) 
       return regionNL;
    return nl->TheEmptyList();
  } 
}


/*
1.1 ~InRegion2~

*/
Word
InRegion2(const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
//  cout << "InRegion2... " << endl;
  if(listutils::isSymbol(instance,Symbol::UNDEFINED()))
  {
    Region2* r = new Region2(0);
    r->SetDefined(false);
    correct = true;
    return SetWord(Address(r));
  }

//cerr << "Schritt 1" << endl;
  int syntaxtyp = 0;
  int scale = 0;
  ListExpr regNL;
  
  if (nl->ListLength(instance) == 2 &&
           nl->AtomType(nl->First(instance)) == IntType)
  {
    syntaxtyp = 1;
    scale = nl->IntValue(nl->First(instance));
    regNL = nl->Second(instance);
//cout << "Syntaxtyp 1" << endl;
  }
  else if ( nl->AtomType(instance) == NoAtom )
  {
    syntaxtyp = 2;
    regNL = instance;
//cout << "Syntaxtyp 2" << endl;
  }
  else
  {
    correct = false;
//cout << "Syntaxtyp fehlgeschlagen" << endl;
    return SetWord(Address(0));
  }

//cerr << "Schritt 2" << endl;
  vector<vector<Reg2PrecisePoint> > cycles;

//cerr << "Schritt 3" << endl;
  while (!nl->IsEmpty(regNL))
  {
    ListExpr faceNL = nl->First(regNL);
    regNL = nl->Rest(regNL);
    if (nl->AtomType(faceNL)!=NoAtom)
    {
      correct = false;
      return SetWord(Address(0));
    }
// cerr << "Schritt 4" << endl;
   bool firstCycle = true;
    Rectangle<2> faceRect;
    while (!nl->IsEmpty(faceNL))
    {
      vector<Reg2PrecisePoint> cycle;
      ListExpr cycleNL = nl->First(faceNL);
      faceNL = nl->Rest(faceNL);
//cerr << "Schritt 5" << endl;
      if (nl->AtomType(cycleNL)!=NoAtom)
      {
         correct=false;
         return SetWord(Address(0));
      }
//cerr << "Schritt 6" << endl;

      Reg2PrecisePoint firstPoint;
      bool fp = true;
      Rectangle<2> currentBB;
      while (!nl->IsEmpty(cycleNL))
      {
//cerr << "Schritt 7: " << nl->ToString(nl->First(cycleNL)) << endl;
        ListExpr pointNL = nl->First(cycleNL);
         cycleNL = nl->Rest(cycleNL);

//cerr << "Schritt 8" << endl;
         mpq_class preciseX(0);
         mpq_class preciseY(0);
         Reg2PrecisePoint p;

         if ( syntaxtyp == 1 )
         {         
           if (nl->ListLength(pointNL) != 3
                    || !nl->IsAtom(nl->First(pointNL))
                    || !nl->IsAtom(nl->Second(pointNL))
                    || nl->AtomType(nl->First(pointNL)) != IntType
                    || nl->AtomType(nl->Second(pointNL)) != IntType)
           {
             correct = false;
             return SetWord(Address(0));
           }

//cerr << "Schritt 8a" << endl;
           if (nl->ListLength(nl->Third(pointNL)) == 2
             && nl->IsAtom(nl->First(nl->Third(pointNL)))
             && nl->AtomType(nl->First(nl->Third(pointNL)))
                 == TextType
             && nl->IsAtom(nl->Second(nl->Third(pointNL)))
             && nl->AtomType(nl->Second(nl->Third(pointNL)))
                 == TextType )
           {
             textTypeToGmpType1(nl->First(nl->Third(pointNL)),
                                preciseX);
             textTypeToGmpType1(nl->Second(nl->Third(pointNL)),
                                preciseY);
           }

//cerr << "Schritt 8b" << endl;
           p = Reg2PrecisePoint(
                     preciseX, 
                     nl->IntValue(nl->First(pointNL)),
                     preciseY, 
                     nl->IntValue(nl->Second(pointNL)), scale);
         }
         else if ( syntaxtyp == 2 )
         {
           if(nl->ListLength(pointNL)!=2){
             correct = false;
             return SetWord(Address(0));
           }
           if ( listutils::isNumeric(nl->First(pointNL)) &&
                listutils::isNumeric(nl->Second(pointNL)) )
           {
             syntaxtyp = 3;
             preciseX = D2MPQ(listutils::getNumValue(
                                nl->First(pointNL)));
             preciseY = D2MPQ(listutils::getNumValue(
                                nl->Second(pointNL)));
             p = Reg2PrecisePoint(preciseX, preciseY);
           }
           else if ( nl->AtomType(nl->First(pointNL)) == TextType &&
                     nl->AtomType(nl->Second(pointNL)) == TextType )
           {
             syntaxtyp = 4;
             textTypeToGmpType2(nl->First(pointNL), preciseX);
             textTypeToGmpType2(nl->Second(pointNL), preciseY);
             p = Reg2PrecisePoint(preciseX, preciseY);
           }
           else
           {
             correct = false;
             return SetWord(Address(0));
           }
         }
         else if ( syntaxtyp == 3 )
         {
           if(nl->ListLength(pointNL)!=2){
             correct = false;
             return SetWord(Address(0));
           }
           if ( listutils::isNumeric(nl->First(pointNL)) &&
                listutils::isNumeric(nl->Second(pointNL)) )
           {
             preciseX = D2MPQ(listutils::getNumValue(
                                      nl->First(pointNL)));
             preciseY = D2MPQ(listutils::getNumValue(
                                      nl->Second(pointNL)));
           }
           p = Reg2PrecisePoint(preciseX, preciseY);
         }
         else if ( syntaxtyp == 4 )
         {
           if(nl->ListLength(pointNL)!=2){
             correct = false;
             return SetWord(Address(0));
           }
           if ( nl->AtomType(nl->First(pointNL)) == TextType &&
                nl->AtomType(nl->Second(pointNL)) == TextType )
           {
             textTypeToGmpType2(nl->First(pointNL), preciseX);
             textTypeToGmpType2(nl->Second(pointNL), preciseY);
           }
           p = Reg2PrecisePoint(preciseX, preciseY);
         }
         
//cerr << "Schritt 8c" << endl;
         cycle.push_back(p);
//cerr << "Schritt 8d" << endl;
         if (fp) 
         {
            fp = false;
            firstPoint = p;
            currentBB = p.BoundingBox();
         } 
         else 
         {
            currentBB = currentBB.Union(p.BoundingBox());
         }
      }
//cout << "Cycle closed!" << endl;
      if ( firstPoint != cycle[cycle.size()-1] ) 
      {
        cycle.push_back(firstPoint);
      }
      if (firstCycle || !faceRect.Contains(currentBB))
      {
//cout << "FirstCycle or New Face" << endl;
        if ( !getDir2(cycle) ) 
        {
           reverseCycle2(cycle);
//cout << "Richtung umgekehrt" << endl;
        }
        firstCycle = false;
        faceRect = currentBB;
      } 
      else 
      {
//cout << "Inside Cycle" << endl;
        if (getDir2(cycle) )
        {
           reverseCycle2(cycle);
//cout << "Richtung umgekehrt" << endl;
        }
      }
//cout << "Richtung: " << getDir2(cycle) << endl;
      cycles.push_back(cycle); 
    }
  }
//cerr << "Schritt 9" << endl;
//cout << "In-Funktion erledigt!" << endl;

  Region2* res = buildRegion2(scale, cycles);
  correct = res!=0;
  return SetWord(res);
}


/*
1.1 ~CreateRegion2~

*/
Word
CreateRegion2( const ListExpr typeInfo )
{
//  cout << "CreateRegion2... " << endl;
  return (SetWord( new Region2( 0 ) ));
}


/*
1.1 ~DeleteRegion2~

*/
void
DeleteRegion2( const ListExpr typeInfo, Word& w )
{
//  cout << "DeleteRegion2... " << endl;
  Region2 *r = (Region2 *)w.addr;
  r->Destroy();
  r->DeleteIfAllowed(false);
//  delete r;
  w.addr = 0;
}


/*
1.1 ~OpenRegion2~

*/
bool
OpenRegion2( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
//  cout << "OpenRegion2... " << endl;
  Region2 *r = (Region2*)Attribute::Open( valueRecord, 
                                          offset, 
                                          typeInfo );
  r->buildHSvector();
  value = SetWord( r );
  return true;
}


/*
1.1 ~SaveRegion2~

*/
bool
SaveRegion2( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
//  cout << "SaveRegion2... " << endl;
  Region2 *r = (Region2 *)value.addr;
  r->buildDbArrays();
  Attribute::Save( valueRecord, offset, typeInfo, r );
  return true;
}


/*
1.1 ~CloseRegion2~

*/
void
CloseRegion2( const ListExpr typeInfo, Word& w )
{
//  cout << "CloseRegion2... " << endl;
  ((Region2 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}


/*
1.1 ~CloneRegion2~

*/
Word
CloneRegion2( const ListExpr typeInfo, const Word& w )
{
//  cout << "CloneRegion2... " << endl;
  Region2 *r = new Region2( *((Region2 *)w.addr) );
  return SetWord( r );
}


/*
1.1 ~SizeOfRegion2~

*/
static int SizeOfRegion2()
{
//  cout << "SizeOfRegion2... " << endl;
  return sizeof(Region2);
}


/*
1.1 ~CheckRegion2~

*/
bool
CheckRegion2( ListExpr type, ListExpr& errorInfo )
{
//  cout << "CheckRegion2... " << endl;
  return (nl->IsEqual( type, Region2::BasicType() ));
}


/*
1.1 TypeConstructor ~region2~

*/
TypeConstructor region2(
        Region2::BasicType(),
        Region2Property,
        OutRegion2,     InRegion2,
        0,              0, 
        CreateRegion2,  DeleteRegion2,
        OpenRegion2,    SaveRegion2,
        CloseRegion2,   CloneRegion2,
        Region2::Cast,
        SizeOfRegion2,
        CheckRegion2 );


TypeConstructor region2V1(
        Region2::BasicType(),
        Region2PropertyV1,
        OutRegion2,     InRegion2,
        0,              0,
        CreateRegion2,  DeleteRegion2,
        OpenRegion2,    SaveRegion2,
        CloseRegion2,   CloneRegion2,
        Region2::Cast,
        SizeOfRegion2,
        CheckRegion2 );


TypeConstructor region2V2(
        Region2::BasicType(),
        Region2PropertyV2,
        OutRegion2,     InRegion2,
        0,              0,
        CreateRegion2,  DeleteRegion2,
        OpenRegion2,    SaveRegion2,
        CloseRegion2,   CloneRegion2,
        Region2::Cast,
        SizeOfRegion2,
        CheckRegion2 );


/*
1.1 ~simpleSelect~

*/
static int simpleSelect(ListExpr args) 
{
  return 0;
}


/*
1 Operators of Region2-Algebra

*/
/*
1.1 setscalefactor

*/
static ListExpr SetScaleTypeMap(ListExpr args)
{
        if (nl->ListLength(args) != 2){
                ErrorReporter::ReportError(
                  "Invalid number of arguments: "
                  "operator expects 2 arguments");
                return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        if (!nl->IsEqual(nl->First(args),Region2::BasicType())){
                ErrorReporter::ReportError(
                        "Region2 as first argument required");
                return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        if (!nl->IsEqual(nl->Second(args),CcInt::BasicType())){
                ErrorReporter::ReportError(
                        "Type Integer as second argument required");
                return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        return nl->SymbolAtom(Region2::BasicType());
}

static int SetScaleValueMap(Word* args, Word& result, int message, 
                            Word& local, Supplier s) 
{
        result = qp->ResultStorage(s);

        Region2* reg = (Region2*) args[0].addr;
        int newScale = ((CcInt*)args[1].addr)->GetIntval();

        Region2 reg2(*reg);
        reg2.SetScaleFactor(newScale);

        ((Region2*)result.addr)->CopyFrom(&reg2);
        return 0;
}

static const string setscalespec =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        " ( <text>region2 x int -> region2</text--->"
        "   <text>setscalefactor( _, _)</text--->"
        "   <text>Changes the scale factor for a given region2,"
        " the integer value is the new scale factor.</text--->"
        "   <text>query setscalefactor(reg2, 1248)</text--->) )";

static Operator setscale("setscalefactor",
                setscalespec,
                SetScaleValueMap,
                simpleSelect,
                SetScaleTypeMap);


/*
1.1 setregion2outstlye

*/
static ListExpr SetOutStyleTypeMap(ListExpr args)
{
        if (nl->ListLength(args) != 1){
                ErrorReporter::ReportError(
                  "Invalid number of arguments: "
                  "operator expects 1 argument");
                return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        ListExpr second = nl->First(args);
        if(!nl->IsEqual(second, CcInt::BasicType())){
              return listutils::typeError(
                    "The argument must be of type int.");
        }
        return (nl->SymbolAtom(CcBool::BasicType()));
}

static int SetOutStyleValueMap(Word* args, Word& result, int message, 
                               Word& local, Supplier s) 
{
        result = qp->ResultStorage(s);

        unsigned style = ((CcInt*)args[0].addr)->GetIntval();
        CcBool *res = (CcBool*) result.addr;

        bool sc = Region2::SetOutType(style);
        res->Set( true, sc );
        
        return 0;
}

static const string setoutstylespec =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        " ( <text>int -> bool</text--->"
        "   <text>setregion2outstyle( _)</text--->"
        "   <text>Changes the style of the out function,"
        " how the points are printed. "
        " 0 (default) is <int int <precisePart> > like"
        " the standard constructor and 1 is <text text> "
        "representing real values with arbitrary precision."
        "</text--->   <text>query setregion2outstyle(1)"
        "</text--->) )";

static Operator setoutstyle("setregion2outstyle",
                setoutstylespec,
                SetOutStyleValueMap,
                simpleSelect,
                SetOutStyleTypeMap);


/*
1.1 isempty

*/
ListExpr isemptyTypeMap(ListExpr args)
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()))
      return (nl->SymbolAtom( CcBool::BasicType() ));
  ErrorReporter::ReportError("Region2 as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int isemptyValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Region2* r = static_cast<const Region2*>(args[0].addr);
  ((CcBool *)result.addr)->Set(true, r->IsEmpty());
  return 0;
}

const string isemptyspec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>region2 -> bool</text--->"
       "<text>isempty ( _ )</text--->"
       "<text>Returns TRUE if the value is undefined or empty."
       " The result is always defined!</text--->"
       "<text>query isempty ( region2 )</text--->"
       ") )";

static Operator isempty ("isempty",
                isemptyspec,
                isemptyValueMap,
                simpleSelect,
                isemptyTypeMap );


/*
1.1 intersects

*/
ListExpr intersectsTypeMap( ListExpr args )
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()) 
    && nl->IsEqual(nl->Second(args),Region2::BasicType()))
      return (nl->SymbolAtom( CcBool::BasicType() ));
  ErrorReporter::ReportError(
               "Region2 as first and second argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int intersectsValueMap( Word* args, Word& result, int message, 
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region2* r1 = static_cast<Region2*>(args[0].addr);
  Region2* r2 = static_cast<Region2*>(args[1].addr);
  if( r1->IsDefined() && r2->IsDefined() )
    ((CcBool *)result.addr)->Set( true, r1->Intersects( *r2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

const string intersectsspec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>region2 x region2 -> bool </text--->"
  "<text>_ intersects _</text--->"
  "<text>TRUE, iff both region2s intersect.</text--->"
  "<text>query reg21 intersects reg26</text--->"
  ") )";

static Operator intersects ( "intersects",
                intersectsspec,
                intersectsValueMap,
                simpleSelect,
                intersectsTypeMap );


/*
1.1 inside

*/
int insideValueMap( Word* args, Word& result, int message, 
                    Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region2* r1 = static_cast<Region2*>(args[0].addr);
  Region2* r2 = static_cast<Region2*>(args[1].addr);
  if( r1->IsDefined() && r2->IsDefined() )
    ((CcBool *)result.addr)->Set( true, r1->Inside( *r2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

const string insidespec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>region2 x region2 -> bool</text--->"
  "<text>_ inside _</text--->"
  "<text>TRUE iff the first region2 is inside the second region2."
  "</text---><text>query reg23 inside reg25</text--->"
  ") )";

static Operator inside ( "inside",
                insidespec,
                insideValueMap,
                simpleSelect,
                intersectsTypeMap );


/*
1.1 adjacent

*/
int adjacentValueMap( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region2* r1 = static_cast<Region2*>(args[0].addr);
  Region2* r2 = static_cast<Region2*>(args[1].addr);
  if( r1->IsDefined() && r2->IsDefined() )
    ((CcBool *)result.addr)->Set( true, r1->Adjacent( *r2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

const string adjacentspec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>region2 x region2 -> bool</text--->"
  "<text>_ adjacent _</text--->"
  "<text>TRUE, iff both region2s are adjacent.</text--->"
  "<text>query reg21 adjacent reg22</text--->"
  ") )";

static Operator adjacent ( "adjacent",
                adjacentspec,
                adjacentValueMap,
                simpleSelect,
                intersectsTypeMap );


/*
1.1 overlaps

*/
int overlapsValueMap( Word* args, Word& result, int message, 
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region2* r1 = static_cast<Region2*>(args[0].addr);
  Region2* r2 = static_cast<Region2*>(args[1].addr);
  if( r1->IsDefined() && r2->IsDefined() )
    ((CcBool *)result.addr)->Set( true,r1->Overlaps( *r2 ) );
  else
    ((CcBool *)result.addr)->SetDefined( false );
  return 0;
}

const string overlapsspec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>region2 x region2 -> bool</text--->"
  "<text>_ overlaps _</text--->"
  "<text>TRUE, iff both region2 objects overlap each other."
  "</text---><text>query reg22 overlap reg24</text--->"
  ") )";

static Operator overlaps ( "overlaps",
                overlapsspec,
                overlapsValueMap,
                simpleSelect,
                intersectsTypeMap );


/*
1.1 no[_]components

*/
ListExpr nocomponentsTypeMap(ListExpr args)
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()))
      return (nl->SymbolAtom( CcInt::BasicType() ));
  ErrorReporter::ReportError("Region2 as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int nocomponentsValueMap( Word* args, Word& result, int message, 
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Region2* r = static_cast<const Region2*>(args[0].addr);
  if( r->IsDefined() )
    ((CcInt *)result.addr)->Set( true, r->NoComponents() );
  else
    ((CcInt *)result.addr)->Set( false, 0 );
  return 0;
}

const string nocomponentsspec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region2 -> int</text--->"
  "<text> no_components( _ )</text--->"
  "<text>return the number of components, "
  "here: the number of faces of a region2 object.</text--->"
  "<text>query no_components(region)</text--->"
  ") )";

static Operator nocomponents ( "no_components",
                nocomponentsspec,
                nocomponentsValueMap,
                simpleSelect,
                nocomponentsTypeMap );


/*
1.1 no[_]segments

*/
int nosegmentsValueMap( Word* args, Word& result, int message, 
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const Region2* r = static_cast<const Region2*>(args[0].addr);
  if( r->IsDefined() )
    ((CcInt *)result.addr)->Set( true, r->Size() );
  else
    ((CcInt *)result.addr)->Set( false, 0 );
  return 0;
}

const string nosegmentsspec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region2 -> int</text--->"
  "<text> no_segments( _ )</text--->"
  "<text>return the number of half segments of a region2 object."
  "</text---><text>query no_segments(region)</text--->"
  ") )";

static Operator nosegments ( "no_segments",
                nosegmentsspec,
                nosegmentsValueMap,
                simpleSelect,
                nocomponentsTypeMap );


/*
1.1 bbox

*/
ListExpr bboxTypeMap( ListExpr args )
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()))
      return (nl->SymbolAtom( Rectangle<2>::BasicType() ));
  ErrorReporter::ReportError("Region2 as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int bboxValueMap(Word* args, Word& result, int message, 
                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<2>* box = static_cast<Rectangle<2>* >(result.addr);
  const Region2* r = static_cast<const Region2*>(args[0].addr);

  if ( !r->IsDefined() )
  {
    box->SetDefined(false);
  } else {
    (*box) = r->BoundingBox();
  }
  return 0;
}

const string bboxspec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region2 -> rect</text--->"
  "<text> bbox( _ )</text--->"
  "<text>Returns the bounding box of a region2 object.</text--->"
  "<text>query bbox(reg22)</text--->"
  ") )";

static Operator bbox ( "bbox",
                bboxspec,
                bboxValueMap,
                simpleSelect,
                bboxTypeMap );

  
/*
1.1 translate

*/
ListExpr translateTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual(arg1, Region2::BasicType()) &&
        nl->IsEqual(nl->First( arg2 ), CcReal::BasicType()) &&
        nl->IsEqual(nl->Second( arg2 ), CcReal::BasicType()))
      return (nl->SymbolAtom( Region2::BasicType() ));
  }
  ErrorReporter::ReportError("First argument as Region2 required, "
                             "second and third as Real!");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int translateValueMap( Word* args, Word& result, int message, 
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region2 *cr = (Region2 *)args[0].addr,
  *pResult = (Region2 *)result.addr;
  pResult->Clear();

  Supplier son = qp->GetSupplier( args[1].addr, 0 );

  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if(  cr->IsDefined() && tx->IsDefined() && ty->IsDefined() ) {
      const double txval = (double)(tx->GetRealval()),
                   tyval = (double)(ty->GetRealval());
      cr->Translate( txval, tyval, *pResult );
  }
  else
    ((Region2*)result.addr)->SetDefined( false );

  return 0;
}

const string translatespec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region2 x real x real -> region2</text--->"
  "<text> _ translate [ _, _ ]</text--->"
  "<text> move the object parallely for some distance.</text--->"
  "<text> query region21 translate[3.5, 15.1]</text--->"
  ") )";

static Operator reg2translate ( "translate",
                translatespec,
                translateValueMap,
                simpleSelect,
                translateTypeMap );

  
/*
1.1 scale

*/
ListExpr scaleTypeMap(ListExpr args)
{
   if(nl->ListLength(args)!=2){
      return listutils::typeError(
                 "operator scale requires two arguments");
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(!(nl->IsEqual(arg2 , CcReal::BasicType()))){
      return listutils::typeError("expectes real as 2nd");
   }
   if(nl->IsEqual(arg1,Region2::BasicType()))
     return nl->SymbolAtom(Region2::BasicType());
   return listutils::typeError(
                  "Expected first argument to be of region2");
}

int scaleValueMap( Word* args, Word& result, int message, 
                   Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Region2 *R = (Region2*) args[0].addr;
  CcReal *factor = (CcReal*) args[1].addr;
  Region2 *res = (Region2*) result.addr;
  if( !R->IsDefined() || !factor->IsDefined() )
  {
    res->SetDefined(false);
  }
  else
  {
    res->Clear();
    res->SetDefined(true);
    double f = factor->GetRealval();
    if(!R->IsEmpty()){
       res->StartBulkLoad();
       int size = R->Size();
       Reg2PreciseHalfSegment hs;
       for(int i=0;i<size;i++){
         R->Get(i,hs);
         hs.Scale(f);
         (*res) += hs;
       }
      res->SetNoComponents( R->NoComponents() );
      res->SetScaleFactor( R->GetScaleFactor() );
      res->EndBulkLoad( false, false, false, false, false );
    }
  }
  return 0;
}

const string scalespec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region2 x real -> region2</text--->"
  "<text> _ scale [ _ ] </text--->"
  "<text> scales a region2 object by the given factor.</text--->"
  "<text> query region21 scale[1000.0]</text--->"
  ") )";

static Operator scale ( "scale",
                scalespec,
                scaleValueMap,
                simpleSelect,
                scaleTypeMap );


/*
1.1 components

*/
ListExpr componentsTypeMap( ListExpr args )
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()))
      return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()), 
                              nl->SymbolAtom(Region2::BasicType()) );
  ErrorReporter::ReportError("Region2 as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

struct ComponentsLocalInfo
{
  vector<Region2*> components;
  vector<Region2*>::iterator iter;
};

int componentsValueMap( Word* args, Word& result, int message, 
                        Word& local, Supplier s )
{
  ComponentsLocalInfo *localInfo;

  switch( message )
  {
    case OPEN:
    // IsEmpty() subsumes undef
      if( !((Region2*)args[0].addr)->IsEmpty() ){
        localInfo = new ComponentsLocalInfo();
        ((Region2*)args[0].addr)->Components(
                                   localInfo->components );
        localInfo->iter = localInfo->components.begin();
        local.setAddr( localInfo );
      } else {
        local.setAddr( 0 );
      }
      return 0;

    case REQUEST:
      if( !local.addr ) {
        return CANCEL;
      }
      localInfo = (ComponentsLocalInfo*)local.addr;
      if( localInfo->iter == localInfo->components.end() )
        return CANCEL;
      result.setAddr( *localInfo->iter++ );
      return YIELD;

    case CLOSE:

      if(local.addr)
      {
        localInfo = (ComponentsLocalInfo*)local.addr;
        while( localInfo->iter != localInfo->components.end() )
        {
          delete *localInfo->iter++;
        }
        delete localInfo;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

const string componentsspec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>region2 -> stream(region2)</text--->"
  "<text>components( _ )</text--->"
  "<text>Returns the components of a region object "
  "(the contained faces) as a stream."
  "Both, empty and undefined objects result in empty stream."
  "</text---><text>query components(reg21) count</text--->"
  ") )";

static Operator components ( "components",
                componentsspec,
                componentsValueMap,
                simpleSelect,
                componentsTypeMap );


/*
1.1 getHoles

*/
ListExpr getholesTypeMap(ListExpr args)
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()))
      return nl->SymbolAtom(Region2::BasicType());
  ErrorReporter::ReportError("Region2 as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int getholesValueMap(Word* args, Word& result, int message, 
                     Word& local, Supplier s )
{
   Region2* arg = (Region2*) args[0].addr;
   result = qp->ResultStorage(s);
   Region2* res = (Region2*) result.addr;
   arg->getHoles(*res);
   return 0;
}

const string getholesspec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>region2 -> region2</text--->"
  "<text> getHoles(_) </text--->"
  "<text> Returns the holes of a region.</text--->"
  "<text> query getHoles(reg22) </text--->"
  ") )";

static Operator getholes( "getHoles",
                getholesspec,
                getholesValueMap,
                simpleSelect,
                getholesTypeMap
);


/*
1.1 region2region2

*/
ListExpr region2region2TypeMap( ListExpr args )
{
  if ( nl->IsEqual(nl->First(args),Region::BasicType()) )
    return (nl->SymbolAtom( Region2::BasicType() ));
  ErrorReporter::ReportError("region as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int region2region2ValueMap( Word* args, Word& result, int message, 
                            Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region *r = (Region *)args[0].addr;
  Region2 *res = (Region2*)result.addr;
  *res = Region2( *r );
  return 0;
}

const string region2region2spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>region -> region2</text--->"
    "<text>_ region2region2</text--->"
    "<text>Converts a region object to a region2 object.</text--->"
    "<text> query [const region value ( (1 1 2 2)(3 3 4 4) )] "
    "region2region2 </text--->) )";

static Operator region2region2 ( "region2region2",
                region2region2spec,
                region2region2ValueMap,
                simpleSelect,
                region2region2TypeMap );


/*
1.1 rect2region2

*/
ListExpr rect2region2TypeMap( ListExpr args )
{
  if ( nl->IsEqual(nl->First(args),Rectangle<2>::BasicType()) )
    return (nl->SymbolAtom( Region2::BasicType() ));
  ErrorReporter::ReportError("Rectangle<2> as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int rect2region2ValueMap( Word* args, Word& result, int message, 
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Rectangle<2> *rect = (Rectangle<2> *)args[0].addr;
  Region2 *res = (Region2*)result.addr;
  *res = Region2( *rect );
  return 0;
}

const string rect2region2spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>rect -> region2</text--->"
    "<text>_ rect2region2</text--->"
    "<text>Converts a rect object to a region2 object.</text--->"
    "<text> query [const rect value (-100.0 200.0 -50.0 500.0)] "
    "rect2region2 </text--->) )";

static Operator rect2region2 ( "rect2region2",
                rect2region2spec,
                rect2region2ValueMap,
                simpleSelect,
                rect2region2TypeMap );


/*
1.1 area and size

*/
ListExpr areaTypeMap( ListExpr args )
{
  if ( nl->IsEqual(nl->First(args),Region2::BasicType()) )
    return (nl->SymbolAtom( CcReal::BasicType() ));
  ErrorReporter::ReportError("Region2 as argument required");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

int areaValueMap( Word* args, Word& result, int message, 
                  Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Region2 *reg = (Region2 *)args[0].addr;
  CcReal *res = (CcReal *)result.addr;
  if(  reg->IsDefined() )
    res->Set( true, reg->Area() );
  else
    res->Set( false, 0.0 );
  return 0;
}

const string areaspec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>region2 -> real</text--->"
    "<text>area( _ )</text--->"
    "<text>Returns the area of a region2 object as a real value."
    "</text---><text> query area( reg22 )</text--->"
    ") )";

static Operator area ( "area",
                areaspec,
                areaValueMap,
                simpleSelect,
                areaTypeMap );


const string sizespec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>region2 -> real</text--->"
    "<text>size( _ )</text--->"
    "<text>Returns the size of a region2 object as a real value."
    "</text---><text> query size( reg22 )</text--->"
    ") )";

static Operator size ( "size",
                sizespec,
                areaValueMap,
                simpleSelect,
                areaTypeMap );


/*
1.1 class definition

*/
class Region2Algebra : public Algebra
{
 public:
  Region2Algebra() : Algebra()
  {
    AddTypeConstructor( &region2 );
    AddTypeConstructor( &region2V1 );
    AddTypeConstructor( &region2V2 );

    region2.AssociateKind(Kind::DATA());

    AddOperator(&setscale);
    AddOperator(&setoutstyle);

    AddOperator( &isempty );
    AddOperator( &intersects );
    AddOperator( &inside );
    AddOperator( &adjacent );
    AddOperator( &overlaps );
    AddOperator( &nocomponents );
    AddOperator( &nosegments );
    AddOperator( &bbox);
    AddOperator( &reg2translate );
    AddOperator( &scale );
    AddOperator( &components );
    AddOperator( &getholes );
    AddOperator( &region2region2 );
    AddOperator( &rect2region2 );
    AddOperator( &size );
    AddOperator( &area );

  }
  ~Region2Algebra() {};
};

extern "C"
Algebra*
InitializeRegion2Algebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new Region2Algebra());
}
