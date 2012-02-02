/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1 Implementation of Algebra Network


March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

February 2008 -  Simone Jandt

October 2008 - Jianqiu Xu

1.1 Defines, includes, and constants

*/

#include <sstream>
#include <time.h>
#include <map>
#include <iterator>
#include <algorithm>
#include <limits>
#include "../Relation-C++/RelationAlgebra.h"
#include "../BTree/BTreeAlgebra.h"
#include "../RTree/RTreeAlgebra.h"
#include "../Spatial/SpatialAlgebra.h"
#include "NetworkAlgebra.h"
#include "../Rectangle/RectangleAlgebra.h"
#include "NetworkManager.h"
#include "../TupleIdentifier/TupleIdentifier.h"
#include "Symbols.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "StandardTypes.h"
#include "Algebra.h"
#include "Messages.h"
#include "ListUtils.h"
#include "ConstructorTemplates.h"
#include "TypeMapUtils.h"
#include "Operator.h"
#include "Attribute.h"



extern NestedList* nl;
extern QueryProcessor* qp;
static map<int,string> *netList;

/*
1 Helping structs, methods and classes

Sending a message via the message-center

*/
void sendMessage ( string in_strMessage )
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append ( NList ( "error" ) );
  xMessage.append ( NList().textAtom ( in_strMessage ) );
  xMessageCenter->Send ( xMessage );
}


/*
Computes a spatial BoundingBox of a RouteInterval

*/

Rectangle<2> RouteInterval::BoundingBox (const Network* pNetwork ) const
{
  if ( AlmostEqualAbsolute ( m_dStart , m_dEnd, pNetwork->GetScalefactor()))
  {
    Point *p = ( GPoint ( true, pNetwork->GetId(), m_iRouteId,
                          m_dStart ) ).ToPoint ( pNetwork );
    Rectangle<2> bbox = Rectangle<2> ( true,
                                       p->GetX(), p->GetX(),
                                       p->GetY(), p->GetY() );
    p->DeleteIfAllowed();
    return bbox;
  }
  else
  {
    SimpleLine *line = new SimpleLine ( 0 );
    pNetwork->GetLineValueOfRouteInterval ( this, line );
    if ( !line->IsEmpty() )
    {
      Rectangle<2> res = line->BoundingBox();
      line->DeleteIfAllowed();
      return res;
    }
    else
    {
      line->DeleteIfAllowed();
      Point *p1 = ( GPoint ( true, pNetwork->GetId(), m_iRouteId,
                             m_dStart ) ).ToPoint ( pNetwork );
      Point *p2 = ( GPoint ( true, pNetwork->GetId(), m_iRouteId,
                             m_dEnd ) ).ToPoint ( pNetwork );
      Rectangle<2> bbox = Rectangle<2> ( true,
                                         min ( p1->GetX(), p2->GetX() ),
                                         max ( p1->GetX(), p2->GetX() ),
                                         min ( p1->GetY(), p2->GetY() ),
                                         max ( p1->GetY(), p2->GetY() ) );
      p1->DeleteIfAllowed();
      p2->DeleteIfAllowed();
      return bbox;
    }
  }
}

/*
~searchRouteInterval~

Method for binary search after a route interval in a sorted ~GLine~. O(log n).
Used for example by operator ~inside~.

*/
bool RouteInterval::Contains(const RouteInterval *ri,
                             const double tolerance)const
{
  double tstart = min(GetStartPos(),GetEndPos());
  double tend = max(GetStartPos(),GetEndPos());
  double start = min(ri->GetStartPos(),ri->GetEndPos());
  double end = max(ri->GetStartPos(),ri->GetEndPos());
  if (ri->GetRouteId() == GetRouteId() &&
    ((tstart <= start || AlmostEqualAbsolute( tstart , start,tolerance )) &&
     (end  <= tend ||AlmostEqualAbsolute(tend, end,tolerance ))))
    return true;
  else
    return false;
}

bool RouteInterval::Contains(const GPoint *gp)const
{
  double tstart = min(GetStartPos(),GetEndPos());
  double tend = max(GetStartPos(),GetEndPos());
  if (GetRouteId() == gp->GetRouteId() &&
    tstart <= gp->GetPosition() && tend >= gp->GetPosition())
    return true;
  else
    return false;
}

bool RouteInterval::Intersects(const RouteInterval *ri,
                               const double tolerance)const
{
  double tstart = min(GetStartPos(),GetEndPos());
  double tend = max(GetStartPos(),GetEndPos());
  double start = min(ri->GetStartPos(),ri->GetEndPos());
  double end = max(ri->GetStartPos(),ri->GetEndPos());
  if(ri->Contains(this, tolerance) || Contains(ri, tolerance) ||
      (ri->GetRouteId() == GetRouteId() &&
      ((start <= tstart && tstart <= end) || (start <= tend && tend <= end))))
    return true;
  else
    return false;
}

bool searchRouteInterval (const RouteInterval *ri, const GLine *pGLine,
                          const double tolerance,
                          const int low, const int high )
{
  RouteInterval rigl;
  if ( low <= high )
  {
    int mid = ( high + low ) / 2;
    if ( ( mid < 0 ) || ( mid >= pGLine->NoOfComponents() ) )
    {
      return false;
    }
    else
    {
      pGLine->Get ( mid, rigl );
      if ( rigl.GetRouteId() < ri->GetRouteId() )
      {
        return searchRouteInterval ( ri, pGLine, tolerance, mid+1, high );
      }
      else
      {
        if ( rigl.GetRouteId() > ri->GetRouteId() )
        {
          return searchRouteInterval ( ri, pGLine, tolerance ,low, mid-1 );
        }
        else
        {
          if (rigl.Contains(ri, tolerance))
          {
            return true;
          }
          else
          {
            if ( rigl.GetStartPos() > ri->GetEndPos())
            {
              return searchRouteInterval ( ri, pGLine, tolerance,low, mid-1 );
            }
            else
            {
              if ( rigl.GetEndPos() < ri->GetStartPos() )
              {
               return searchRouteInterval ( ri, pGLine,tolerance, mid+1, high );
              }
              else
              {
                return false;
              }
            }
          }
        }
      }
    }
  }
  return false;
}
/*
~searchRouteInterval~

Method for binary search after a route interval in a sorted ~GLine~. O(log n).
Used for example by operator ~inside~.

*/

bool searchRouteInterval ( const GPoint *pGPoint, const GLine *&pGLine,
                           const double tolerance,
                           const size_t low, const size_t high )
{
  RouteInterval rI;
  if ( low <= high )
  {
    size_t mid = ( high + low ) / 2;
    int imid = mid;
    if ( ( imid < 0 ) || ( imid >= pGLine->NoOfComponents() ) )
    {
      return false;
    }
    else
    {
      pGLine->Get ( mid, rI );
      if ( rI.GetRouteId() < pGPoint->GetRouteId() )
      {
        return searchRouteInterval ( pGPoint, pGLine, tolerance, mid+1, high );
      }
      else
      {
        if ( rI.GetRouteId() > pGPoint->GetRouteId() )
        {
          return searchRouteInterval ( pGPoint, pGLine, tolerance, low, mid-1 );
        }
        else
        {
          if (AlmostEqualAbsolute(pGPoint->GetPosition(),
                                  rI.GetStartPos(), tolerance ) ||
              AlmostEqualAbsolute ( pGPoint->GetPosition() ,
                                    rI.GetEndPos(),tolerance ))
          {
            return true;
          }
          else
          {
            if ( rI.GetStartPos() > pGPoint->GetPosition() )
            {
              return searchRouteInterval ( pGPoint, pGLine, tolerance, low,
                                           mid-1 );
            }
            else
            {
              if ( rI.GetEndPos() < pGPoint->GetPosition() )
              {
                return searchRouteInterval ( pGPoint, pGLine, tolerance, mid+1,
                                             high );
              }
              else
              {
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

/*
Helper struct to store sections which have been visited before. Used in
shortest path implementation of Dijkstras Algorithm.

*/
struct SectTree
{

  SectTree() {};

  SectTree ( const SectTreeEntry *nEntry, SectTree *l = 0, SectTree *r = 0 )
  {
    value = *nEntry;
    left = l;
    right = r;
  };

  ~SectTree() {};


/*
Inserts a section into the tree. If the section is already in the insert is
ignored.

*/
  void Insert (const SectTreeEntry *nEntry )
  {
    if ( nEntry->secttid < value.secttid )
    {
      if ( right != 0 ) right->Insert ( nEntry );
      else right = new SectTree ( nEntry,0,0 );
    }
    else
    {
      if ( nEntry->secttid > value.secttid )
      {
        if ( left != 0 ) left->Insert ( nEntry );
        else left = new SectTree ( nEntry,0,0 );
      }
    }
  };

/*
Returns a pointer to the node with the given id or 0 if not found.

*/
  SectTree* Find (const TupleId n )
  {
    if ( n < value.secttid )
    {
      if ( right != 0 ) return right->Find ( n);
      else return 0;
    }
    else
    {
      if ( n > value.secttid )
      {
        if ( left != 0 ) return left->Find ( n);
        else return 0;
      }
      else return this;
    }
  };

  void CheckSection (const  Network *pNetwork, const SectTreeEntry n,
                     GPoints &result )
  {

    vector<DirectedSection> sectList;
    sectList.clear();
    if ( n.startbool || n.endbool )
    {
      if ( n.startbool )
      {
        pNetwork->GetAdjacentSections ( n.secttid, false, sectList );
        size_t j = 0;
        SectTree* test= 0;
        while ( j < sectList.size() && test == 0 )
        {
          DirectedSection actSection = sectList[j];
          test = Find ( actSection.GetSectionTid());
        }
        if ( test != 0 )
          result += GPoint ( true, pNetwork->GetId(), n.rid, n.start, None );
      }
      sectList.clear();
      if ( n.endbool )
      {
        pNetwork->GetAdjacentSections ( n.secttid, true, sectList );
        size_t j = 0;
        SectTree* test = 0;
        while ( j < sectList.size() && test == 0 )
        {
          DirectedSection actSection = sectList[j];
          test = Find ( actSection.GetSectionTid());
        }
        if ( test != 0 )
          result+= GPoint ( true, pNetwork->GetId(), n.rid, n.end, None );
      }
      sectList.clear();
    }
    else result += GPoint ( true, pNetwork->GetId(), n.rid, n.end, None );
  }

  void WriteResult ( const Network* pNetwork, GPoints &result,
                     SectTree &secTr ) const
  {
    if ( left != 0 ) left->WriteResult ( pNetwork, result, secTr );
    if ( right != 0 ) right ->WriteResult ( pNetwork, result, secTr );
    secTr.CheckSection ( pNetwork, value, result );
  };

  void Remove()
  {
    if ( left != 0 ) left->Remove();
    if ( right != 0 ) right->Remove();
    delete this;
  };

  SectTreeEntry value;
  SectTree *left, *right;
};

/*
Almost similar to operator ~checkPoint~ but additional returning a difference
value if the point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint ( SimpleLine *&route, const Point point,
                const bool startSmaller,const double tolerance,
                double &pos, double &difference )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ( ( AlmostEqualAbsolute ( x,xr,tolerance) &&
           AlmostEqualAbsolute ( y,yr,tolerance)) ||
         ( AlmostEqualAbsolute ( x,xl,tolerance) &&
           AlmostEqualAbsolute ( y,yl,tolerance)))
    {
      difference = 0.0;
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( fabs ( k1-k2 ) < 0.004*tolerance/0.01 ) &&
             ( ( xl < xr &&
               ( x > xl || AlmostEqualAbsolute ( x,xl,tolerance )) &&
               ( x < xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ||
               ( xl > xr &&
               ( x < xl || AlmostEqualAbsolute ( x,xl,tolerance ))  &&
               ( x > xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ) &&
             ((( yl <= yr  &&
                   ( y > yl || AlmostEqualAbsolute ( y,yl, tolerance )) &&
                   ( y < yr || AlmostEqualAbsolute ( y,yr, tolerance ) ) ) ||
               ( yl > yr &&
                   ( y < yl || AlmostEqualAbsolute ( y,yl,tolerance ) ) &&
                   ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ) ))
        {
          difference = fabs ( k1-k2 );
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( AlmostEqualAbsolute ( xl, xr,tolerance ) &&
               AlmostEqualAbsolute ( xl,x,tolerance ) ) &&
            ( ( ( yl <= yr &&
                  ( yl < y || AlmostEqualAbsolute (yl,y,tolerance))&&
                  ( y < yr ||AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ||
                ( yl > yr &&
                  ( yl > y || AlmostEqualAbsolute ( yl,y,tolerance) ) &&
                  ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ) ))
        {
          difference = 0.0;
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( AlmostEqualAbsolute ( pos,0.0,tolerance ))
        pos = 0.0;
      else if ( AlmostEqualAbsolute( pos, route->Length(),tolerance))
        pos = route->Length();
      return result;
    }
  }
  return result;
}

/*
Almost similar to operator ~chkPoint~ but allowing a greater difference if the
point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint03 ( SimpleLine *&route, const Point point,
                  const bool startSmaller, const double tolerance,
                  double &pos, double &difference )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
          if ( ( AlmostEqualAbsolute ( x,xl,tolerance ) &&
                 AlmostEqualAbsolute ( y,yl,tolerance )) ||
               ( AlmostEqualAbsolute ( x,xr,tolerance ) &&
                 AlmostEqualAbsolute ( y,yr,tolerance ) ) )
    {
      difference = 0.0;
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( fabs ( k1-k2 ) < 1.2*tolerance ) &&
             ( ( xl < xr &&
               ( x > xl || AlmostEqualAbsolute ( x,xl,tolerance ) ) &&
               ( x < xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ||
               ( xl > xr &&
               ( x < xl || AlmostEqualAbsolute ( x,xl,tolerance ))  &&
               ( x > xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ) &&
             ( ( ( yl < yr || AlmostEqualAbsolute ( yl,yr,tolerance )) &&
                 ( y > yl || AlmostEqualAbsolute (y,yl,tolerance )) &&
                 ( y < yr || AlmostEqualAbsolute ( y,yr,tolerance )) ) ||
               ( yl > yr &&
                 ( y < yl || AlmostEqualAbsolute ( y,yl,tolerance )) &&
                 ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ) )
        {
          difference = fabs ( k1-k2 );
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( AlmostEqualAbsolute ( xl, xr,tolerance ) &&
               AlmostEqualAbsolute ( xl,x,tolerance )) &&
             ( ( ( yl < yr|| AlmostEqualAbsolute ( yl,yr,tolerance )) &&
                 ( yl < y || AlmostEqualAbsolute (yl,y ,tolerance) ) &&
                 ( y < yr || AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ||
                 ( yl > yr  &&
                 ( yl > y || AlmostEqualAbsolute ( yl,y,tolerance )) &&
                 ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance )) ) ) )
        {
          difference = 0.0;
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( AlmostEqualAbsolute ( pos,0.0,tolerance ))
        pos = 0.0;
      else if ( AlmostEqualAbsolute ( pos,route->Length(),tolerance ))
        pos = route->Length();
      return result;
    }
  }
  return result;
}

/*
Almost similar to operator ~chkPoint~ but not checking the difference if the
point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/
bool lastchkPoint03 ( SimpleLine *&route, const Point point,
                      const bool startSmaller, const double tolerance,
                      double &pos, double &difference )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
          if ( ( AlmostEqualAbsolute ( x,xl,tolerance ) &&
                 AlmostEqualAbsolute ( y,yl,tolerance )) ||
               ( AlmostEqualAbsolute ( x,xr,tolerance ) &&
                 AlmostEqualAbsolute ( y,yr,tolerance ) ) )
    {
      difference = 0.0;
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ((( xl < xr &&
             ( x > xl || AlmostEqualAbsolute ( x,xl,tolerance ) ) &&
             ( x < xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ||
             ( xl > xr &&
             ( x < xl || AlmostEqualAbsolute ( x,xl,tolerance ))  &&
             ( x > xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ) &&
           ( ( ( yl < yr || AlmostEqualAbsolute ( yl,yr,tolerance )) &&
               ( y > yl || AlmostEqualAbsolute (y,yl,tolerance )) &&
               ( y < yr || AlmostEqualAbsolute ( y,yr,tolerance )) ) ||
             ( yl > yr &&
             ( y < yl || AlmostEqualAbsolute ( y,yl,tolerance )) &&
             ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ) )
        {
          difference = fabs ( k1-k2 );
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( AlmostEqualAbsolute ( xl, xr,tolerance ) &&
               AlmostEqualAbsolute ( xl,x,tolerance )) &&
             ( ( ( yl < yr|| AlmostEqualAbsolute ( yl,yr,tolerance )) &&
                 ( yl < y || AlmostEqualAbsolute (yl,y ,tolerance) ) &&
                 ( y < yr || AlmostEqualAbsolute ( y,yr,tolerance ) ) ) ||
                 ( yl > yr  &&
                 ( yl > y || AlmostEqualAbsolute ( yl,y,tolerance )) &&
                 ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance )) ) ) )
        {
          difference = 0.0;
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( !startSmaller)
        pos = route->Length() - pos;
      if ( pos < 0.0 || AlmostEqualAbsolute ( pos, 0.0, tolerance ) )
        pos = 0.0;
      else if ( pos > route->Length() ||
                AlmostEqualAbsolute( pos,route->Length(),tolerance))
        pos = route->Length();
      return result;
    }
  }
  return result;
}

/*
Returns true if a ~point~ is part of a ~sline~, false elsewhere. If the point
is part of the sline his distance from the start is computed also. Used by
operator ~line2gline~.

*/
bool checkPoint ( SimpleLine *&route, const Point point,
                  const bool startSmaller, const double tolerance,
                  double &pos )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ( ( AlmostEqualAbsolute ( x,xr,tolerance ) &&
           AlmostEqualAbsolute ( y,yr,tolerance )) ||
         ( AlmostEqualAbsolute ( x,xl,tolerance ) &&
           AlmostEqualAbsolute ( y,yl,tolerance )) )
    {
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( fabs ( k1-k2 ) < 0.004*tolerance ) &&
             ( ( xl < xr &&
                  ( x > xl || AlmostEqualAbsolute ( x,xl,tolerance )) &&
                  ( x < xr || AlmostEqualAbsolute ( x,xr,tolerance ) ) ) ||
               ( xl > xr &&
                  ( x < xl || AlmostEqualAbsolute ( x,xl,tolerance ))  &&
                  ( x > xr || AlmostEqualAbsolute ( x,xr,tolerance )) ) ) &&
             ( ( ( yl < yr || AlmostEqualAbsolute ( yl,yr,tolerance )) &&
                  ( y > yl || AlmostEqualAbsolute ( y,yl,tolerance )) &&
                  ( y < yr || AlmostEqualAbsolute ( y,yr,tolerance )) ) ||
               ( yl > yr &&
                  ( y < yl || AlmostEqualAbsolute ( y,yl,tolerance )) &&
                  ( y > yr || AlmostEqualAbsolute ( y,yr,tolerance )) ) ) )
        {
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( AlmostEqualAbsolute ( xl, xr,tolerance ) &&
               AlmostEqualAbsolute ( xl,x, tolerance)) &&
             ( ( ( yl < yr|| AlmostEqualAbsolute ( yl,yr, tolerance )) &&
                    ( yl < y || AlmostEqualAbsolute ( yl,y, tolerance ) ) &&
                    ( y < yr ||AlmostEqualAbsolute ( y,yr, tolerance ) ) ) ||
                 ( yl > yr &&
                    ( yl > y || AlmostEqualAbsolute ( yl,y,tolerance )) &&
                    ( y > yr || AlmostEqualAbsolute ( y,yr, tolerance ) ) ) ) )
        {
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( AlmostEqualAbsolute( pos,0.0, tolerance ))
        pos = 0.0;
      else if ( AlmostEqualAbsolute ( pos,route->Length(), tolerance ))
        pos = route->Length();
      return result;

    }
  }
  return result;
};

/*
Precondition: ~GLine~ is sorted.

Returns true if there is a ~RouteInterval~ in the sorted ~GLine~ which
intersects with the given ~RouteInterval~ false elsewhere.

Used by operator ~intersects~

*/

bool searchUnit ( const GLine *pGLine, const int low, const int high,
                  const RouteInterval pRi )
{

  assert ( pGLine->IsSorted() );
  RouteInterval rI;
  if ( low <= high )
  {
    int mid = ( high + low ) / 2;
    if ( ( mid < 0 ) || ( mid >= pGLine->NoOfComponents() ) )
    {
      return false;
    }
    else
    {
      pGLine->Get ( mid, rI );

      if ( rI.GetRouteId() < pRi.GetRouteId() )
      {
        return searchUnit ( pGLine, mid+1, high, pRi );
      }
      else
      {
        if ( rI.GetRouteId() > pRi.GetRouteId() )
        {
          return searchUnit ( pGLine, low, mid-1, pRi );
        }
        else
        {
          if ( rI.GetStartPos() > pRi.GetEndPos() )
          {
            return searchUnit ( pGLine, low, mid-1, pRi );
          }
          else
          {
            if ( rI.GetEndPos() < pRi.GetStartPos() )
            {
              return searchUnit ( pGLine, mid+1, high, pRi );
            }
            else
            {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}


/*
class ~GPointList~

Used by the operator ~polygpoint~. Computes and stores the resulting ~GPoints~
for the resulting ~stream~ of ~GPoint~.

*/

class GPointList
{
  public:
    /*
    The constructor creates a GPointList from a given gpoint.

    */
    GPointList ( const GPoint *gp, const Network *pNetwork ) :
        aliasGP ( 1 )
    {
      lastPos = 0;
      aliasGP.clean();
      aliasGP.Append ( *gp );
      vector<JunctionSortEntry> xJunctions;
      xJunctions.clear();
      if ( pNetwork != 0 )
      {
        CcInt iRouteId ( true, gp->GetRouteId() );
        pNetwork->GetJunctionsOnRoute ( &iRouteId, xJunctions );
        bool found = false;
        JunctionSortEntry pCurrJunction;
        size_t i = 0;
        while ( !found && i < xJunctions.size() )
        {
          pCurrJunction = xJunctions[i];
          if (AlmostEqualAbsolute( pCurrJunction.GetRouteMeas(),
                                   gp->GetPosition(),
                                   pNetwork->GetScalefactor()*0-01))
          {
            found = true;
            aliasGP.Append ( GPoint( true, gp->GetNetworkId(),
                          pCurrJunction.GetOtherRouteId(),
                          pCurrJunction.GetOtherRouteMeas(),
                          None) );
          }
          i++;
        }
        while ( found && i < xJunctions.size() )
        {
          pCurrJunction = xJunctions[i];
          if ( AlmostEqualAbsolute ( pCurrJunction.GetRouteMeas(),
                                     gp->GetPosition(),
                                     pNetwork->GetScalefactor()*0.01))
          {
            aliasGP.Append ( GPoint(true, gp->GetNetworkId(),
                          pCurrJunction.GetOtherRouteId(),
                          pCurrJunction.GetOtherRouteMeas(),
                          None) );
          }
          else
          {
            found = false;
          }
          i++;
        }
        for ( size_t i=0;i<xJunctions.size();i++ )
        {
          xJunctions[i].m_pJunction->DeleteIfAllowed();
        }
        xJunctions.clear();
      }
    }

    ~GPointList() {}

/*
~NextGPoint~

This function returns the next GPoint from the GPointList.
If there is no more GPoint in the  List the result will be
0. This function creates a new GPoint instance via the new
operator. The caller of this function has to ensure the
deletion of this object.

*/
    const GPoint NextGPoint()
    {
      GPoint pAktGPoint(true);
      if ( lastPos < aliasGP.Size() && lastPos >= 0 )
      {
        aliasGP.Get ( lastPos, pAktGPoint );
        lastPos++;
        return pAktGPoint;
      }
      else
      {
        pAktGPoint.SetDefined(false);
        return pAktGPoint;
      }
    }

    void Destroy()
    {
      aliasGP.Destroy();
    }

  private:

    DbArray<GPoint> aliasGP;
    int lastPos;

};

/*
class ~RectangleList~

Almost similar to ~GPointList~. But storing rectangles representing route
intervals of a ~gline~. Used by operator ~routeintervals~ to create a ~stream~
of ~rectangles~.

*/

class RectangleList
{

  public:

    /*
    ~Constructor~

    Creates a RectangleList from a given gline.

    */

    RectangleList ( const GLine *gl ) :
        aliasRectangleList ( 0 )
    {
      RouteInterval ri;
      lastPos = 0;
      aliasRectangleList.clean();
      for ( int i = 0 ; i < gl->NoOfComponents(); i++ )
      {
        gl->Get ( i, ri );
        Rectangle<2> elem ( true,
                            ( double ) ri.GetRouteId(),
                            ( double ) ri.GetRouteId(),
                            min ( ri.GetStartPos(),ri.GetEndPos() ),
                            max ( ri.GetStartPos(), ri.GetEndPos() ) );
        aliasRectangleList.Append ( elem );
      }
    }

    ~RectangleList() {}

/*
~NextRectangle~

This function returns the next rectangle from the RectangleList.
If there is no more route interval in the  List the result will be
0. This function creates a new Rectangle instance via the new
operator. The caller of this function has to ensure the
deletion of this object.

*/
    const Rectangle<2> NextRectangle()
    {
      Rectangle<2> pAktRectangle;
      if ( lastPos >= aliasRectangleList.Size() || lastPos < 0 )
      {
        pAktRectangle.SetDefined ( false );
      }
      else
      {
        aliasRectangleList.Get ( lastPos, pAktRectangle );
        lastPos++;
      }
      return pAktRectangle;
    }

  private:

    DbArray<Rectangle<2> > aliasRectangleList;
    int lastPos;

};

/*
~struct RIStack~

Used to build compressed shortestpath ~gline~ values.
The ~RouteIntervals~ of the shortest path computation are pushed to a stack to
first the end section. Last the first section of the path. So they can be
returned in the sequence they are needed in the path.

When the stack is returned to the resulting ~gline~ it is checked if the
~RouteInterval~s on the stack can be connected to bigger parts. This is always
possible if the shortest path follows the same route for more than one section.

*/

struct RIStack
{

  RIStack() {};

  RIStack ( const int ri, const double pos1, const double pos2,
            RIStack* next = 0 )
  {
    m_iRouteId = ri;
    m_dStart = pos1;
    m_dEnd = pos2;
    m_next = next;
  };

  ~RIStack() {};

  void Push ( const int rid, const double pos1, const double pos2,
              RIStack *&first )
  {
    RIStack *actElem = new RIStack ( rid, pos1, pos2, this );
    first = actElem;
  };

  void StackToGLine ( GLine *gline, const double tolerance ) const
  {
    int actRId = m_iRouteId;
    double actStartPos = m_dStart;
    double actEndPos = m_dEnd;
    RIStack *actElem = m_next;
    while ( actElem != 0 )
    {
      if ( actRId == actElem->m_iRouteId &&
           AlmostEqualAbsolute ( actEndPos,actElem->m_dStart, tolerance ) )
      {

        actEndPos = actElem->m_dEnd;
      }
      else
      {
        gline->AddRouteInterval ( actRId, actStartPos, actEndPos );
        actRId = actElem->m_iRouteId;
        actStartPos = actElem->m_dStart;
        actEndPos = actElem->m_dEnd;
      }
      actElem = actElem->m_next;
    }
    gline->AddRouteInterval ( actRId, actStartPos, actEndPos );
  };

  void RemoveStack()
  {
    if ( m_next != 0 ) m_next->RemoveStack();
    delete this;
  };

  int m_iRouteId;
  double m_dStart, m_dEnd;
  RIStack *m_next;
};

/*
Analogous to RIStack but not limited by main memory.

*/

struct RIStackP
{

  RIStackP():ristack(0)
  {
    lastEntry = -1;
  };

  RIStackP(const int n):ristack(n)
  {
    lastEntry = -1;
  };

  ~RIStackP() {};

  void Destroy()
  {
    ristack.Destroy();
  }

  void Push ( const int rid, const double pos1, const double pos2)
  {
    ristack.Put(lastEntry+1,RouteInterval(rid,pos1,pos2));
    lastEntry++;
  };

  void Push (const RouteInterval ri)
  {
    ristack.Put(lastEntry+1,ri);
    lastEntry++;
  }

  void StackToGLine ( GLine *gline, const int netid ) const
  {
    gline->Clear();
    gline->SetNetworkId(netid);
    RouteInterval ri1, ri2;
    int i = lastEntry;
    if (i > -1 )
    {
      ristack.Get(i,ri1);
      gline->SetDefined(true);
    }
    else
      gline->SetDefined(false);
    i--;
    while (i >= 0)
    {
      ristack.Get(i,ri2);
      if (ri2.GetRouteId() == ri1.GetRouteId() &&
          AlmostEqual(ri1.GetEndPos(),ri2.GetStartPos()))
      {
        ri1.SetEndPos(ri2.GetEndPos());
      }
      else
      {
        gline->AddRouteInterval(ri1);
        ri1 = ri2;
      }
      i--;
    }
    gline->AddRouteInterval ( ri1);
  };

  DbArray<RouteInterval> ristack;
  int lastEntry;
};

/*
Class PQEntry used for priority Queue in Dijkstras Algorithm for shortest path
computing between two gpoint.

*/

class PQEntry
{
  public:
    PQEntry() {}

    PQEntry ( const TupleId aktID, const double distance, const bool upDown,
              const TupleId beforeID )
    {
      sectID = aktID;
      distFromStart = distance;
      upDownFlag = upDown;
      beforeSectID = beforeID;
    }

    PQEntry ( const PQEntry &e )
    {
      sectID = e.sectID;
      distFromStart = e.distFromStart;
      upDownFlag = e.upDownFlag;
      beforeSectID = e.beforeSectID;
    }

  ostream& Print ( ostream& os ) const
  {
    os << "Section TupleId: " << sectID << endl;
    os << "Distance from Start: " << distFromStart << endl;
    os << "upDownFlag: " << upDownFlag << endl;
    os << "Before Section TupleId: " << beforeSectID << endl;
    os << endl;
    return os;
  }

  ostream& csvout (ostream& os) const
  {
    os << sectID << ";" << upDownFlag << endl;
    return os;
  }

    ~PQEntry() {}

    TupleId sectID;
    double distFromStart;
    bool upDownFlag;
    TupleId beforeSectID;
};

/*
Almost similar to PQEntry but supporting priority values needed by
A-Star-Algorithm.

*/

class PQEntryA {
  public:

    PQEntryA(){};

    PQEntryA(const TupleId aktID, const double weight, const double value,
             const bool upDown, const TupleId beforeID,
             const bool beforeUp )
    {
      sectID = aktID;
      prioval = weight;
      valFromStart = value;
      upDownFlag = upDown;
      beforeSectID = beforeID;
      beforeUpDown = beforeUp;
    };

    PQEntryA (const PQEntryA &e )
    {
      sectID = e.sectID;
      prioval = e.prioval;
      valFromStart = e.valFromStart;
      upDownFlag = e.upDownFlag;
      beforeSectID = e.beforeSectID;
      beforeUpDown = e.beforeUpDown;
    }

    int Compare(const PQEntryA& nE) const
    {
      if (sectID < nE.sectID) return -1;
      if (sectID > nE.sectID) return 1;
      if (prioval < nE.prioval) return -1;
      if (prioval > nE.prioval) return 1;
      if (valFromStart < nE.valFromStart) return -1;
      if (valFromStart > nE.valFromStart) return 1;
      return 0;
    }

    ostream& Print ( ostream& os ) const
    {
      os << "Section TupleId: " << sectID << endl;
      os << "Weight: " << prioval << endl;
      os << "Distance from Start: " << valFromStart << endl;
      os << "upDownFlag: " << upDownFlag << endl;
      os << "Before Section TupleId: " << beforeSectID << endl;
      os << "Before Section UpDownFlag: " << beforeUpDown << endl;
      return os;
    }

    ~PQEntryA() {};

    TupleId sectID;
    double prioval;
    double valFromStart;
    bool upDownFlag;
    TupleId beforeSectID;
    bool beforeUpDown;

};

/*
Section Entries for data structure used in shortest path search.

*/

struct SectEntry
{
  SectEntry(){}

  SectEntry(const SectEntry& nE)
    : sectID(nE.GetSectId()),
      beforeSectId(nE.GetBeforeSectId()),
      upDownFlag(nE.GetUpDownFlag()),
      beforeSectUpDown(nE.GetBeforeSectUpDown()),
      index(nE.GetIndex()),
      distFromStart(nE.GetDistFromStart())
  {}

  SectEntry(const TupleId sid, const TupleId befSID, const bool updown,
            const bool befUpDown, const int i, const double dist)
    : sectID(sid), beforeSectId(befSID), upDownFlag(updown),
      beforeSectUpDown(befUpDown), index(i),
      distFromStart(dist)
  {}

  ~SectEntry(){}

  inline TupleId GetSectId() const
  {
    return sectID;
  }

  inline TupleId GetBeforeSectId() const
  {
    return beforeSectId;
  }

  inline bool GetUpDownFlag() const
  {
    return upDownFlag;
  }

  inline int GetIndex() const
  {
    return index;
  }

  inline double GetDistFromStart() const
  {
    return distFromStart;
  }

  inline bool GetBeforeSectUpDown() const
  {
    return beforeSectUpDown;
  }

  int Compare (const SectEntry& se) const
  {
    return Compare(se.GetSectId(),se.GetUpDownFlag());
  }

  inline int Compare (const TupleId sid, const bool upDown) const
  {
    if (sectID < sid ) return -1;
    if (sectID > sid ) return 1;
    if (upDownFlag < upDown) return -1;
    if (upDownFlag > upDown) return 1;
    return 0;
  }

  void operator=(const SectEntry& se)
  {
    sectID = se.GetSectId();
    beforeSectId = se.GetBeforeSectId();
    upDownFlag = se.GetUpDownFlag();
    index = se.GetIndex();
    distFromStart = se.GetDistFromStart();
    beforeSectUpDown = se.GetBeforeSectUpDown();
  }

  ostream& Print(ostream& os) const
  {
    os << "SectEntry: ";
    os << "SectionID: " << sectID;
    os << ", upDownFlag: " << upDownFlag;
    os << ", Before Section Id: " << beforeSectId;
    os << ", Before Section UpDownFlag: " << beforeSectUpDown;
    os << ", index: " << index;
    os << ", distFromStart: " << distFromStart << endl;
    return os;
  }

  inline void SetSectID(const TupleId tid)
  {
    sectID = tid;
  }

  inline void SetBeforeSectId(const TupleId tid)
  {
    beforeSectId = tid;
  }

  inline void SetUpDownFlag(const bool b)
  {
    upDownFlag = b;
  }

  inline void SetIndex(const int i)
  {
    index = i;
  }

  inline void SetDistFromStart(const double dist)
  {
    distFromStart = dist;
  }

  inline void SetBeforeSectUpDown(const bool upDown)
  {
    beforeSectUpDown = upDown;
  }

  TupleId sectID;
  TupleId beforeSectId;
  bool upDownFlag;
  bool beforeSectUpDown;
  int index;
  double distFromStart;
};

/*
Helpful datastructure for shortest path search.

*/

struct SectIDTreeEntry
{
  SectIDTreeEntry(){}

  SectIDTreeEntry(const SectEntry ne, const int l = -1, const int r = -1)
    : sE(ne), left(l), right(r)
  {}

  SectIDTreeEntry(const SectIDTreeEntry& nSE)
  {
    sE = nSE.GetEntry();
    left = nSE.GetLeft();
    right = nSE.GetRight();
  }

  ~SectIDTreeEntry(){}

  inline SectEntry GetEntry() const
  {
    return sE;
  }

  inline int GetLeft() const
  {
    return left;
  }

  inline int GetRight() const
  {
    return right;
  }

  inline void SetEntry(const SectEntry nse)
  {
    sE = nse;
  }

  inline void SetLeft(const int l)
  {
    left = l;
  }

  inline void SetRight(const int r)
  {
    right = r;
  }

  void operator=(const SectIDTreeEntry siE)
  {
    sE = siE.GetEntry();
    left = siE.GetLeft();
    right = siE.GetRight();
  }

  inline int Compare(const SectIDTreeEntry& siE) const
  {
    return Compare(siE.GetEntry());
  }

  inline int Compare(const SectEntry& cse) const
  {
    return GetEntry().Compare(cse);
  }

  inline int Compare(const int stid, const bool upDown) const
  {
    return GetEntry().Compare(stid,upDown);
  }

  ostream& Print(ostream& os) const
  {
    os << "SectIDTreeEntry: ";
    sE.Print(os);
    os << ", left: " << left;
    os << ", right: " << right << endl;
    return os;
  }

  SectEntry sE;
  int left, right;
};

/*
Not main memory limited data structure supporting shortest path search.

*/

struct SectIDTreeP
{
  SectIDTreeP()
    : tree(0)
  {
    fFree = 0;
  }

  SectIDTreeP(const int n)
    : tree(n)
  {
    fFree = 0;
  }

  ~SectIDTreeP(){};

  inline void Destroy()
  {
    tree.Destroy();
  }

  int Find(const SectIDTreeEntry& te) const
  {
    int i = 0;
    if (tree.Size() < 1) return -1;
    while (i < fFree)
    {
      SectIDTreeEntry test = GetTreeEntry(i);
      switch(test.Compare(te))
      {
        case 0:
        {
          return i;
          break;
        }
        case 1:
        {
          if (test.GetLeft() != -1) i = test.GetLeft();
          else return i;
          break;
        }
        case -1:
        {
          if (test.GetRight() != -1) i = test.GetRight();
          else return i;
          break;
        }
        default: // should never been reached
        {
          return -1;
          break;
        }
      }
    }
    return -1; // should never been reached.
  }

  int Find(const TupleId tid, const bool upDown) const
  {
    int i = 0;
    if (tree.Size() < 1) return -1;
    while (i < fFree)
    {
      SectIDTreeEntry test = GetTreeEntry(i);
      switch(test.Compare(tid,upDown))
      {
        case 0:
        {
          return i;
          break;
        }
        case 1:
        {
          if (test.GetLeft() > -1) i = test.GetLeft();
          else return i;
          break;
        }
        case -1:
        {
          if (test.GetRight() > -1) i = test.GetRight();
          else return i;
          break;
        }
        default: // should never been reached
        {
          return -1;
        }
      }
    }
    return -1; //Should never been reached
  }

  ostream& Print(ostream& os) const
  {
    for (int i = 0; i < tree.Size(); i++)
    {
      SectIDTreeEntry te = GetTreeEntry(i);
      te.Print(os);
    }
    return os;
  }

  bool Insert(const SectIDTreeEntry nse, int& newPos)
  {
    newPos = Find(nse);
    if (newPos < 0)
    {
      newPos = fFree;
      fFree++;
      tree.Put(newPos, nse);
      return true;
    }
    else
    {
      if (newPos >= 0 && newPos < fFree)
      {
        SectIDTreeEntry test = GetTreeEntry(newPos);
        switch(test.Compare(nse))
        {
          case 1:
          {
            test.SetLeft(fFree);
            tree.Put(newPos,test);
            tree.Put(fFree,nse);
            fFree++;
            return true;
            break;
          }
          case -1:
          {
            test.SetRight(fFree);
            tree.Put(newPos,test);
            tree.Put(fFree,nse);
            fFree++;
            return true;
            break;
          }
          case 0:
          {
            if (nse.GetEntry().GetDistFromStart() <
                  test.GetEntry().GetDistFromStart())
            {
              tree.Put(newPos,nse);
              return true;
            }
            else
            {
              return false;
            }
            break;
          }
          default: // should never been reached
          {
            return false;
            break;
          }
        }
      }
      else
        return false;
    }
    return false; //should never been reached
  }

  void SetIndex (const int pos, const int index)
  {
    assert(pos > -1 && pos < fFree);
    SectIDTreeEntry te = GetTreeEntry(pos);
    SectEntry nE = te.GetEntry();
    nE.SetIndex(index);
    te.SetEntry(nE);
    tree.Put(pos,te);
  }

  inline int GetIndex (const int pos) const
  {
    assert(pos > -1 && pos < fFree);
    return GetTreeEntry(pos).GetEntry().GetIndex();
  }

  void SetBeforeSectId(const int pos, const TupleId before)
  {
    assert (pos > -1 && pos < fFree);
    SectIDTreeEntry te = GetTreeEntry(pos);
    SectEntry ne = te.GetEntry();
    ne.SetBeforeSectId(before);
    te.SetEntry(ne);
    tree.Put(pos,te);
  }

  void SetBeforeUpDownFlag(const int pos, const bool upDown)
  {
    assert (pos > -1 && pos < fFree);
    SectIDTreeEntry te = GetTreeEntry(pos);
    SectEntry ne = te.GetEntry();
    ne.SetBeforeSectUpDown(upDown);
    te.SetEntry(ne);
    tree.Put(pos,te);
  }

  SectIDTreeEntry GetTreeEntry(const int pos) const
  {
    assert(pos > -1 && pos < fFree);
    SectIDTreeEntry te;
    tree.Get(pos,te);
    return te;
  }

  bool IsNode(const int pos, const TupleId sectId, const bool upDown) const
  {
    assert (pos > -1 && pos < fFree);
    SectEntry se = GetTreeEntry(pos).GetEntry();
    if (se.Compare(sectId,upDown) == 0)
      return true;
    else
      return false;
  }

  void SetDistance(const int pos, const double dist)
  {
    assert (pos > -1 && pos < fFree);
    SectIDTreeEntry te = GetTreeEntry(pos);
    SectEntry ne = te.GetEntry();
    ne.SetDistFromStart(dist);
    te.SetEntry(ne);
    tree.Put(pos,te);
  }

  void SetFlag(const int pos, const bool upDown)
  {
    assert (pos > -1 && pos < fFree);
    SectIDTreeEntry te = GetTreeEntry(pos);
    SectEntry ne = te.GetEntry();
    ne.SetUpDownFlag(upDown);
    te.SetEntry(ne);
    tree.Put(pos,te);
  }

  DbArray<SectIDTreeEntry> tree;
  int fFree;
};

/*
struct sectIDTree stores the PQEntrys by section ID with identification flag
for the PrioQueue-Array.
An arrayIndex from max integer means not longer in PrioQ and already visited.

*/

struct SectIDTree
{
  SectIDTree() {};

  SectIDTree ( const TupleId sectIdent, const TupleId beforeSectIdent,
               const bool upDown, const int arrayIndex,
               const double dist = numeric_limits<double>::max(),
               SectIDTree *l = 0, SectIDTree *r = 0 )
  {
    sectID = sectIdent;
    beforeSectId = beforeSectIdent;
    upDownFlag = upDown;
    index = arrayIndex;
    left = l;
    right = r;
    distFromStart = dist;

  };

  ~SectIDTree() {};

  SectIDTree* Find (const  TupleId sectIdent )
  {
    if ( sectID > sectIdent )
    {
      if ( left != 0 ) return left->Find ( sectIdent );
      else
      {
        return this;
      }
    }
    else
    {
      if ( sectID < sectIdent )
      {
        if ( right != 0 ) return right->Find ( sectIdent );
        else
        {
          return this;
        }
      }
      else
      {
        return this;
      }
    }
  };

  ostream& Print(ostream& os) const
  {
    if (left != 0)
    {
      os << "left son:";
      left->Print(os);
    }
    os << "This: ";
    os << "sectID: " << sectID;
    os << ", before sectId: " << beforeSectId;
    os << ", upDownFlag: " << upDownFlag;
    os << ", DistFromStart: " << distFromStart;
    os << ", Index: " << index << endl;
    if (right != 0)
    {
      os << "right son: ";
      right->Print(os);
    }
    return os;
  }

  void Remove()
  {
    if ( left != 0 ) left->Remove();
    if ( right != 0 ) right->Remove();
    delete this;
  };

  bool Insert ( const TupleId sectIdent, const TupleId beforeSectIdent,
                const bool upDownFlag, const int arrayIndex,
                SectIDTree *&pointer,
                const double dist = numeric_limits<double>::max())
  {
    pointer = Find ( sectIdent );
    if ( pointer->sectID > sectIdent )
    {
      pointer->left = new SectIDTree ( sectIdent, beforeSectIdent, upDownFlag,
                                       arrayIndex, dist );
      pointer = pointer->left;
      return true;
    }
    else
    {
      if ( pointer->sectID < sectIdent )
      {
        pointer->right = new SectIDTree ( sectIdent, beforeSectIdent,
                                          upDownFlag, arrayIndex, dist);
        pointer = pointer->right;
        return true;
      }
      else
      {
        if (dist < pointer->distFromStart)
        {
          pointer->beforeSectId = beforeSectIdent;
          pointer->distFromStart = dist;
          pointer->index = arrayIndex;
          pointer->upDownFlag = upDownFlag;
          return true;
        }
        else
        {
          return false;
        }
      }
    }
  };

  void SetIndex ( const TupleId sectIdent, const int arrayIndex )
  {
    (Find( sectIdent ))->index = arrayIndex;
  };

  void SetIndex ( const int arrayIndex )
  {
    index = arrayIndex;
  };

  void SetFlag(const bool upDown)
  {
    upDownFlag = upDown;
  };

  void SetDistance(const double dist)
  {
    distFromStart = dist;
  };

  int GetIndex ( const int sectIdent )
  {
    return (Find ( sectIdent ))->index;
  };

  void SetBeforeSectId (const  TupleId sectIdent, const TupleId before )
  {
    (Find ( sectIdent ))->beforeSectId = before;
  };

  void SetBeforeSectId ( const TupleId before )
  {
    beforeSectId = before;
  };

  TupleId sectID;
  TupleId beforeSectId;
  bool upDownFlag;
  int index;
  double distFromStart;
  SectIDTree *left, *right;
};


/*
struct Priority Queue for Dijkstras Algorithm of shortest path computing between
two gpoint.

*/

struct PrioQueue
{

  PrioQueue():prioQ(0) {firstFree = 0;};

  PrioQueue ( const int n ) : prioQ ( 0 ) {firstFree = 0;};

  ~PrioQueue() {};

  /*
  If a point is reached second time and the distance of the second way is
  smaller than on the path he has been found before. Its distance value and its
  position in the  priority queue must be corrected.

  */
  void CorrectPosition ( const int checkX, const PQEntry nElem,
                         SectIDTree* pSection,
                         SectIDTree *sectTree )
  {
    int act = checkX;
    int n  = checkX;
    PQEntry test;
    bool found = false;
    while ( n >= 0 && !found )
    {
      if ( ( act % 2 ) == 0 ) n = ( act-2 ) / 2;
      else n = ( act -1 ) / 2;
      if ( n >= 0 )
      {
        prioQ.Get ( n, test );
        if ( test.distFromStart > nElem.distFromStart )
        {
          PQEntry help = test;
          prioQ.Put ( n, nElem );
          pSection->SetIndex ( n );
          prioQ.Put ( act, help );
          SectIDTree *thelp = sectTree->Find ( help.sectID );
          thelp->SetIndex ( act );
          act = n;
        }
        else
        {
          found = true;
        }
      }
      else
      {
        found = true;
      }
    }
  };

  void Insert ( const PQEntry nElem, SectIDTree *sectTree,
                DbArray<TupleId>* touchedSects)
  {
    SectIDTree *pSection = sectTree->Find ( nElem.sectID );
    PQEntry old;
    if ( pSection->sectID == nElem.sectID )
    {
      if ( pSection->index != numeric_limits<int>::max() )
      {
        prioQ.Get ( pSection->index, old );
        if ( nElem.distFromStart < old.distFromStart )
        {
          prioQ.Put ( pSection->index, nElem );
          pSection->SetBeforeSectId ( nElem.beforeSectID );
          CorrectPosition ( pSection->index, nElem, pSection, sectTree );
        }
      }
    }
    else
    {
      prioQ.Put ( firstFree, nElem );
      sectTree->Insert ( nElem.sectID, nElem.beforeSectID, nElem.upDownFlag,
                         firstFree, pSection/*, nElem.distFromStart */);
      CorrectPosition ( firstFree, nElem, pSection ,sectTree );
      if(touchedSects != 0) touchedSects->Append(nElem.sectID);
      firstFree++;
    }
  }

  PQEntry* GetAndDeleteMin ( SectIDTree *sectTree )
  {
    if ( firstFree > 0 )
    {
      PQEntry result, last, test1, test2;
      prioQ.Get ( 0,result );
      PQEntry *retValue = new PQEntry ( result.sectID,
                                        result.distFromStart,
                                        result.upDownFlag,
                                        result.beforeSectID );
      SectIDTree *tRet = sectTree->Find ( result.sectID );
      tRet->SetIndex ( numeric_limits<int>::max() );
      prioQ.Get ( firstFree-1, last );
      prioQ.Put ( 0, last );
      firstFree--;
      SectIDTree *pSection = sectTree->Find ( last.sectID );
      pSection->SetIndex ( 0 );
      int act = 0;
      int checkX = 0;
      bool found = false;
      while ( checkX < firstFree && !found )
      {
        checkX = 2*act + 1;
        if ( checkX < firstFree-1 )
        {
          prioQ.Get ( checkX, test1 );
          prioQ.Get ( checkX+1, test2 );
          if ( test1.distFromStart < last.distFromStart ||
                  test2.distFromStart < last.distFromStart )
          {
            if ( test1.distFromStart <= test2.distFromStart )
            {
              PQEntry help = test1;
              prioQ.Put ( checkX, last );
              pSection->SetIndex ( checkX );
              prioQ.Put ( act, help );
              SectIDTree *thelp = sectTree->Find ( help.sectID );
              thelp->SetIndex ( act );
              act = checkX;
            }
            else
            {
              PQEntry help = test2;
              prioQ.Put ( checkX+1, last );
              pSection->SetIndex ( checkX+1 );
              prioQ.Put ( act, help );
              SectIDTree *thelp = sectTree->Find ( help.sectID );
              thelp->SetIndex ( act );
              act = checkX+1;
            }
          }
          else
          {
            found = true;
          }
        }
        else
        {
          if ( checkX != 0 && checkX == firstFree-1 )
          {
            prioQ.Get ( checkX, test1 );
            if ( test1.distFromStart < last.distFromStart )
            {
              PQEntry help = test1;
              prioQ.Put ( checkX, last );
              pSection->SetIndex ( checkX );
              prioQ.Put ( act, help );
              SectIDTree *thelp = sectTree->Find ( help.sectID );
              thelp->SetIndex ( act );
              act = checkX;
            }
            else
            {
              found = true;
            }
          }
        }
      }
      return retValue;
    }
    else
    {
      return 0;
    }
  }

  void Clear()
  {
    prioQ.clean();
    firstFree = 0;
  }

  bool IsEmpty() const
  {
    if ( firstFree == 0 ) return true;
    else return false;
  }

  void Destroy()
  {
    prioQ.Destroy();
  }

  DbArray<PQEntry> prioQ;
  int firstFree;

};

struct PrioQueueA
{
   PrioQueueA()
    : prioQ(0)
  {
    firstFree=0;
  }

  PrioQueueA ( const int n )
    : prioQ ( n )
  {
    firstFree = 0;
  }

  ~PrioQueueA() {};

/*
If a point is reached second time and the prioval of the second way is
smaller than on the path found before. The prioval, the valFromStart and the
position in the  priority queue must be corrected.

*/
  void CorrectPosition ( const int checkX, const PQEntryA& nElem,
                         SectIDTreeP *sectTree )
  {
    int act = checkX;
    int n = checkX;
    PQEntryA test;
    bool found = false;
    while ( act > 0 && !found )
    {
      if ( ( act % 2 ) == 0 ) n = ( act-2 ) / 2;
      else n = ( act -1 ) / 2;
      if ( n >= 0 )
      {
        prioQ.Get ( n, test );
        if ( test.prioval > nElem.prioval ||
             (test.prioval == nElem.prioval &&
              test.valFromStart > nElem.valFromStart))
        {
          Swap( n, nElem , act, test, sectTree );
        }
        else
        {
          found = true;
        }
      }
      else
      {
        found = true;
      }
    }
  };

  void Append ( const PQEntryA nE, int pNElemPos,
                SectIDTreeP *pNodeTree)
  {
    int actPos = firstFree;
    prioQ.Put(actPos, nE );
    pNodeTree->Insert(SectIDTreeEntry(SectEntry(nE.sectID,
                                                nE.beforeSectID,
                                                nE.upDownFlag,
                                                nE.beforeUpDown,
                                                actPos,
                                                nE.valFromStart),
                                -1,-1),
                      pNElemPos);
    CorrectPosition(actPos, nE, pNodeTree );
    firstFree++;
  }

  void Insert ( const PQEntryA nElem, SectIDTreeP *sectTree,
                DbArray<TupleId>* touchedSects )
  {
    int pSection = sectTree->Find ( nElem.sectID,nElem.upDownFlag );
    if (pSection < 0)
    {
      Append(nElem, pSection,sectTree);
      if (touchedSects != 0) touchedSects->Append(nElem.sectID);
    }
    else
    {
      if (pSection > -1 && pSection < sectTree->fFree)
      {
        if (!sectTree->IsNode(pSection,nElem.sectID,nElem.upDownFlag))
        {
          Append(nElem, pSection,sectTree);
          if (touchedSects != 0) touchedSects->Append(nElem.sectID);
        }
        else
        {
          int index = sectTree->GetIndex(pSection);
          if (index > -1 && index < firstFree)
          {
            PQEntryA test;
            prioQ.Get(index, test);
            if (test.prioval > nElem.prioval)
            {
              prioQ.Put(index,nElem);
              sectTree->SetBeforeSectId(pSection,nElem.beforeSectID);
              sectTree->SetBeforeUpDownFlag(pSection,nElem.beforeUpDown);
              sectTree->SetDistance(pSection, nElem.valFromStart);
              sectTree->SetFlag(pSection, nElem.upDownFlag);
              CorrectPosition(index, nElem, sectTree);
            }
          }
          else
          {
            if (nElem.valFromStart <
                sectTree->GetTreeEntry(pSection).GetEntry().GetDistFromStart())
            {
              int  actPos = firstFree;
              prioQ.Put(actPos,nElem);
              sectTree->SetIndex(pSection, actPos);
              firstFree++;
              sectTree->SetBeforeSectId(pSection,nElem.beforeSectID);
              sectTree->SetBeforeUpDownFlag(pSection,nElem.beforeUpDown);
              sectTree->SetDistance(pSection, nElem.valFromStart);
              sectTree->SetFlag(pSection, nElem.upDownFlag);
              CorrectPosition (actPos, nElem, sectTree );
            }
          }
        }
      }
    }
  }

  void Swap (const int index1, const PQEntryA& entry1,
             int& index2, const PQEntryA& entry2,
             SectIDTreeP* pSectIdTree)
  {
    prioQ.Put(index1, entry1);
    pSectIdTree->SetIndex(pSectIdTree->Find(entry1.sectID,entry1.upDownFlag),
                          index1);
    prioQ.Put(index2, entry2);
    pSectIdTree->SetIndex(pSectIdTree->Find(entry2.sectID,entry2.upDownFlag),
                          index2);
    index2 = index1;
  }

  PQEntryA* GetAndDeleteMin ( SectIDTreeP *sectTree )
  {
    if ( firstFree <= 0 ) return 0;
    PQEntryA result, last, test1, test2;
    prioQ.Get ( 0,result );
    PQEntryA *retValue = new PQEntryA ( result);
    int tRet = sectTree->Find ( result.sectID, result.upDownFlag );
    prioQ.Get (firstFree-1, last );
    prioQ.Put ( 0, last );
    prioQ.Put(firstFree-1, PQEntryA((TupleId)numeric_limits<long>::max(),
                                  numeric_limits<double>::max(),
                                  numeric_limits<double>::max(),
                                  false,
                                  (TupleId)numeric_limits<long>::max(),
                                  false));

    firstFree--;
    int pSection = sectTree->Find ( last.sectID,last.upDownFlag );
    sectTree->SetIndex (pSection,0);
    sectTree->SetIndex(tRet, -1);
    int act = 0;
    int checkX = 0;
    bool found = false;
    while ( checkX < firstFree && !found )
    {
      checkX = 2*act + 1;
      if ( checkX < firstFree-1 )
      {
        prioQ.Get ( checkX, test1 );
        prioQ.Get ( checkX+1, test2 );
        if ( test1.prioval < last.prioval ||
                test2.prioval < last.prioval )
        {
          if ( test1.prioval <= test2.prioval )
          {
            Swap(checkX, last, act, test1, sectTree );
          }
          else
          {
            Swap( checkX+1, last, act, test2, sectTree);
          }
        }
        else
        {
          if (test1.prioval == last.prioval &&
              test1.valFromStart < last.valFromStart)
          {
            Swap( checkX, last, act, test1, sectTree);
          }
          else
          {
            if (test2.prioval == last.prioval &&
                test2.valFromStart < last.valFromStart)
            {
              Swap( checkX+1, last, act, test2, sectTree );
            }
            else
            {
              found = true;
            }
          }
        }
      }
      else
      {
        if ( checkX != 0 && checkX == firstFree-1 )
        {
          prioQ.Get ( checkX, test1 );
          if ( test1.prioval < last.prioval ||
              (test1.prioval == last.prioval &&
                test1.valFromStart < last.valFromStart))
          {
            Swap( checkX, last, act, test1, sectTree );
          }
          else
          {
            found = true;
          }
        }
        else
        {
          found = true;
        }
      }
    }
    return retValue;
  }

  void Clear()
  {
    prioQ.clean();
    firstFree = 0;
  }

  bool IsEmpty() const
  {
    if (firstFree == 0 ) return true;
    else return false;
  }

  void Destroy()
  {
    prioQ.Destroy();
  }

  ostream& Print(ostream& os) const
  {
    os << "PriorityQueue Start: " << endl;
    PQEntryA pE;
    if (firstFree > 0)
    {
      for (int i = 0; i < prioQ.Size(); i++)
      {
        prioQ.Get(i,pE);
        pE.Print(os);
      }
    }
    os << "PriorityQueue Ende" << endl;
    return os;
  }

  DbArray<PQEntryA> prioQ;
  int firstFree;

};

/*
Some helpful methods for shortest path computing

*/

bool IsLastSection(const TupleId source,
                   DbArray<GPointsSections>* endSections, int& pos)
{
  GPointsSections target;
  for (int j = 0; j < endSections->Size(); j++)
  {
    endSections->Get(j,target);
    if (source == target.GetTid())
    {
      pos = j;
      return true;
    }
  }
  return false;
}

bool IsFirstSection(const TupleId pElem,
                    DbArray<GPointsSections>* startSections,
                    int& pos)
{
  GPointsSections source;
  for (int k = 0; k < startSections->Size(); k++)
  {
    startSections->Get(k,source);
    if (pElem == source.GetTid())
    {
      pos = k;
      return true;
    }
  }
  return false;
}

void InsertAdjacentSections(const TupleId tid, const Side direction,
                            const bool init, double dist,
                            const Points* endPoints,
                            const Network* pNetwork, PrioQueueA* prioQ,
                            SectIDTreeP* visitedSect,
                            DbArray<TupleId>* touchedSects)
{
  Tuple* sourceTuple = pNetwork->GetSection(tid);
  double sectMeas1 =
    ((CcReal*) sourceTuple->GetAttribute(SECTION_MEAS1))->GetRealval();
  double sectMeas2 =
    ((CcReal*)sourceTuple->GetAttribute(SECTION_MEAS2))->GetRealval();
  int actRouteId =
    ((CcInt*)sourceTuple->GetAttribute(SECTION_RID))->GetIntval();
  Point* sPoint = 0;
  bool upDown = false;
  if (direction == Down)
  {
    if (init)
      dist = fabs(dist - sectMeas1);
    else
      dist += fabs(sectMeas2 - sectMeas1);
    sPoint = (GPoint( true,
                      pNetwork->GetId(),
                      actRouteId,
                      sectMeas1,
                      Down)).ToPoint(pNetwork);
    upDown = false;
  }
  else
  {
    if (init)
      dist = fabs(sectMeas2 - dist);
    else
      dist += fabs(sectMeas2 - sectMeas1);
    sPoint = (GPoint( true,
                      pNetwork->GetId(),
                      actRouteId,
                      sectMeas2,
                      Up)).ToPoint(pNetwork);
    upDown = true;
  }
  double weight = dist + endPoints->Distance(*sPoint);
  sPoint->DeleteIfAllowed();
  sPoint = 0;
  sourceTuple->DeleteIfAllowed();
  sourceTuple = 0;
  vector<DirectedSection> adjSectionList;
  adjSectionList.clear();
  pNetwork->GetAdjacentSections ( tid, upDown, adjSectionList );
  for (size_t k = 0;  k < adjSectionList.size(); k++ )
  {
    DirectedSection actNextSect = adjSectionList[k];
    prioQ->Insert(PQEntryA ( actNextSect.GetSectionTid(),
                             weight,
                             dist,
                             actNextSect.GetUpDownFlag(),
                             tid,
                             upDown),
                    visitedSect, touchedSects ) ;
  }
  adjSectionList.clear();
}

/*
2 Class Definitions

2.1 class ~Network~

2.1.2 Network relations

*/
string Network::routesTypeInfo =
    "(rel (tuple ((Id int) (Length real) (Curve sline) "
    "(Dual bool) (StartsSmaller bool))))";
string Network::routesBTreeTypeInfo =
    "(btree (tuple ((Id int) (Length real) (Curve sline) "
    "(Dual bool) (StartsSmaller bool))) int)";

string Network::routesRTreeTypeInfo =
    "(rtree (tuple((Id int)(Length real)(Curve sline)(Dual bool)"
    "(StartsSmaller bool))) sline FALSE)";

string Network::junctionsTypeInfo =
    "(rel (tuple ((R1id int) (Meas1 real) (R2id int) "
    "(Meas2 real) (Cc int))))";

/*string Network::junctionsInternalTypeInfo =
    "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
    "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
    "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))))";*/
string Network::junctionsInternalTypeInfo =
    "(rel (tuple ((R1id int) (Meas1 real) (R2id int) "
    "(Meas2 real) (Cc int) (Loc point) (R1rc tid) (R2rc tid) "
    "(Sauprc tid) (Sadownrc tid)(Sbuprc tid) (Sbdownrc tid))))";


string Network::junctionsBTreeTypeInfo =
    "(btree (tuple ((R1id int) (Meas1 real) (R2id int) "
    "(Meas2 real) (Cc int) (Loc point) (R1rc tid) (R2rc tid) "
    "(Sauprc tid) (Sadownrc tid)(Sbuprc tid) (Sbdownrc tid))) int)";
string Network::sectionsInternalTypeInfo =
    "(rel (tuple ((Rid int) (Meas1 real) (Meas2 real) (Dual bool)"
    "(Curve sline)(CurveStartsSmaller bool) (Rrc tid) (Sid int))))";
string Network::sectionsBTreeTypeInfo =
    "(btree (tuple ((Rid int) (Meas1 real) (Meas2 real) (Dual bool)"
    "(Curve sline)(CurveStartsSmaller bool) (Rrc tid) (Sid int))) int)";
string Network::distancestorageTypeInfo =
    "(rel (tuple((J1 tid)(J2 tid)(Dist real)(Sp gline))))";

/*
2.1.3 Constructors and destructors class ~Network~

*/

Network::Network() :
    m_iId ( 0 ),
    m_scalefactor(1.0),
    m_bDefined ( false ),
    m_pRoutes ( 0 ),
    m_pJunctions ( 0 ),
    m_pSections ( 0 ),
    m_pBTreeRoutes ( 0 ),
    m_pRTreeRoutes ( 0 ),
    m_pBTreeJunctionsByRoute1 ( 0 ),
    m_pBTreeJunctionsByRoute2 ( 0 ),
    m_xAdjacencyList ( 0 ),
    m_xSubAdjacencyList ( 0 ),
    m_reverseAdjacencyList(0),
    m_reverseSubAdjancencyList(0),
    m_pBTreeSectionsByRoute ( 0 )
    /*alldistance(0)*/  //only for experimental use with network distances
{
}

Network::Network ( SmiRecord& in_xValueRecord,
                   size_t& inout_iOffset,
                   const ListExpr in_xTypeInfo ):
m_xAdjacencyList(0),
m_xSubAdjacencyList(0),
m_reverseAdjacencyList(0),
m_reverseSubAdjancencyList(0)
{

  // Read network id
  in_xValueRecord.Read ( &m_iId, sizeof ( int ), inout_iOffset );
  inout_iOffset += sizeof ( int );

  in_xValueRecord.Read ( &m_scalefactor, sizeof ( double ), inout_iOffset );
  inout_iOffset += sizeof ( double );
  // Open routes
  ListExpr xType;
  nl->ReadFromString ( routesTypeInfo, xType );
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pRoutes = Relation::Open ( in_xValueRecord,
                               inout_iOffset,
                               xNumericType );
  if ( !m_pRoutes )
  {
    return;
  }

  // Open junctions
  nl->ReadFromString ( junctionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pJunctions = Relation::Open ( in_xValueRecord,
                                  inout_iOffset,
                                  xNumericType );
  if ( !m_pJunctions )
  {
    m_pRoutes->Delete();
    return;
  }

  // Open sections
  nl->ReadFromString ( sectionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pSections = Relation::Open ( in_xValueRecord,
                                 inout_iOffset,
                                 xNumericType );
  if ( !m_pSections )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    return;
  }

  // Open btree for routes
  nl->ReadFromString ( routesBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeRoutes = BTree::Open ( in_xValueRecord,
                                 inout_iOffset,
                                 xNumericType );

  if ( !m_pBTreeRoutes )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    return;
  }
  //Open  rtree for routes
  Word xValue;

  if ( ! ( m_pRTreeRoutes->Open ( in_xValueRecord,
                                  inout_iOffset,
                                  routesRTreeTypeInfo,
                                  xValue ) ) )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    return;
  }

  m_pRTreeRoutes = ( R_Tree<2,TupleId>* ) xValue.addr;

  // Open first btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeJunctionsByRoute1 = BTree::Open ( in_xValueRecord,
                              inout_iOffset,
                              xNumericType );
  if ( !m_pBTreeJunctionsByRoute1 )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    return;
  }

  // Open second btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeJunctionsByRoute2 = BTree::Open ( in_xValueRecord,
                              inout_iOffset,
                              xNumericType );
  if ( !m_pBTreeJunctionsByRoute2 )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    return;
  }

   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   m_xAdjacencyList.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   m_xSubAdjacencyList.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

// The same for reverse adjacency lists.
   bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   m_reverseAdjacencyList.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   m_reverseSubAdjancencyList.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);
  // Open btree for sections
  nl->ReadFromString ( sectionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeSectionsByRoute = BTree::Open ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType );
  if ( !m_pBTreeSectionsByRoute )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    return;
  }

  //Open distance storage
  /*
  nl->ReadFromString(distancestorageTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  alldistance = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);

  if(!alldistance){
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    return;
  }
  */
  m_bDefined = true;
}

Network::Network ( ListExpr in_xValue,
                   int in_iErrorPos,
                   ListExpr& inout_xErrorInfo,
                   bool& inout_bCorrect ) :
    m_iId ( 0 ),
    m_scalefactor(1.0),
    m_bDefined ( false ),
    m_pRoutes ( 0 ),
    m_pJunctions ( 0 ),
    m_pSections ( 0 ),
    m_pBTreeRoutes ( 0 ),
    m_pRTreeRoutes ( 0 ),
    m_pBTreeJunctionsByRoute1 ( 0 ),
    m_pBTreeJunctionsByRoute2 ( 0 ),
    m_xAdjacencyList ( 0 ),
    m_xSubAdjacencyList ( 0 ),
    m_reverseAdjacencyList(0),
    m_reverseSubAdjancencyList(0),
    m_pBTreeSectionsByRoute ( 0 )
    /*alldistance(0)*/
{

  // Check the list
  if ( ! ( nl->ListLength ( in_xValue ) == 4 ) )
  {
    string strErrorMessage = "Network(): List length must be 4.";
    inout_xErrorInfo =
        nl->Append ( inout_xErrorInfo, nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    return;
  }

  // Get type-info for temporary table
  ListExpr xType;
  nl->ReadFromString ( routesTypeInfo, xType );
  ListExpr xRoutesNumType = SecondoSystem::GetCatalog()->NumericType ( xType );
  nl->ReadFromString ( junctionsTypeInfo, xType );
  ListExpr xJunctionsNumType = SecondoSystem::GetCatalog()->NumericType ( xType
);

  // Split into the three parts
  ListExpr xIdList = nl->First ( in_xValue );
  ListExpr xScalList = nl->Second(in_xValue);
  ListExpr xRouteList = nl->Third ( in_xValue );
  ListExpr xJunctionList = nl->Fourth ( in_xValue );
  // Sections will be calculated in the load-method

  // Read Id
  if ( !nl->IsAtom ( xIdList ) ||
          nl->AtomType ( xIdList ) != IntType )
  {
    string strErrorMessage = "Network(): Id is missing.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    return;
  }
  m_iId = nl->IntValue ( xIdList );

  // Read Scalefactor
  if ( !nl->IsAtom ( xScalList ) ||
    nl->AtomType ( xScalList ) != RealType )
  {
    string strErrorMessage = "Network(): Scalefactor is missing.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    return;
  }
    m_scalefactor = nl->RealValue ( xScalList );

  // Create new temporary relations.
  Relation* pRoutes = new Relation ( xRoutesNumType, false );
  Relation* pJunctions = new Relation ( xJunctionsNumType, false );

  // Iterate over all routes
  while ( !nl->IsEmpty ( xRouteList ) )
  {
    ListExpr xCurrentRoute = nl->First ( xRouteList );
    xRouteList = nl->Rest ( xRouteList );

    // Create tuple for internal table
    Tuple* pNewRoute = new Tuple ( nl->Second ( xRoutesNumType ) );

    // Check this part of the list
    if ( nl->ListLength ( xCurrentRoute ) != 5 ||
            ( !nl->IsAtom ( nl->First ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->First ( xCurrentRoute ) ) != IntType ||
            ( !nl->IsAtom ( nl->Second ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->Second ( xCurrentRoute ) ) != RealType ||
            ( nl->IsAtom ( nl->Third ( xCurrentRoute ) ) ) ||
            ( !nl->IsAtom ( nl->Fourth ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->Fourth ( xCurrentRoute ) ) != BoolType ||
            ( !nl->IsAtom ( nl->Fifth ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->Fifth ( xCurrentRoute ) ) != BoolType )
    {
      delete pRoutes;
      delete pRoutes;

      string strErrorMessage = "Network(): Error while reading out routes.";
      inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                      nl->StringAtom ( strErrorMessage ) );
      inout_bCorrect = false;
      return;
    }

    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue ( nl->First ( xCurrentRoute ) );
    double dLength  = nl->RealValue ( nl->Second ( xCurrentRoute ) );
    Word xLineWord = InSimpleLine ( nl->TheEmptyList(),
                                    nl->Third ( xCurrentRoute ),
                                    in_iErrorPos,
                                    inout_xErrorInfo,
                                    inout_bCorrect );
    SimpleLine* pLine = ( SimpleLine* ) ( xLineWord.addr );
    bool bDual= nl->BoolValue ( nl->Fourth ( xCurrentRoute ) );
    bool bStartsSmaller  = nl->BoolValue ( nl->Fifth ( xCurrentRoute ) );

    // Set all necessary attributes
    pNewRoute->PutAttribute ( ROUTE_ID, new CcInt ( true, iRouteId ) );
    pNewRoute->PutAttribute ( ROUTE_LENGTH, new CcReal ( true, dLength ) );
    pNewRoute->PutAttribute ( ROUTE_CURVE, pLine );
    pNewRoute->PutAttribute ( ROUTE_DUAL, new CcBool ( true, bDual ) );
    pNewRoute->PutAttribute ( ROUTE_STARTSSMALLER, new CcBool ( true,
                              bStartsSmaller ) );

    // Append new junction
    pRoutes->AppendTuple ( pNewRoute );
    if ( pNewRoute )
    {
      pNewRoute->DeleteIfAllowed();
      pNewRoute=0;
    }
  }

  // Iterate over all junctions
  while ( !nl->IsEmpty ( xJunctionList ) )
  {
    ListExpr xCurrentJunction = nl->First ( xJunctionList );
    xJunctionList = nl->Rest ( xJunctionList );

    // Create tuple for internal table
    Tuple* pNewJunction = new Tuple ( nl->Second ( xJunctionsNumType ) );

    // Check this part of the list
    if ( nl->ListLength ( xCurrentJunction ) != 6 ||
            ( !nl->IsAtom ( nl->First ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->First ( xCurrentJunction ) ) != IntType ||
            ( !nl->IsAtom ( nl->Second ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Second ( xCurrentJunction ) ) != RealType ||
            ( !nl->IsAtom ( nl->Third ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Third ( xCurrentJunction ) ) != IntType ||
            ( !nl->IsAtom ( nl->Fourth ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Fourth ( xCurrentJunction ) ) != RealType ||
            ( !nl->IsAtom ( nl->Fifth ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Fifth ( xCurrentJunction ) ) != IntType )
    {
      delete pRoutes;
      delete pJunctions;

      string strErrorMessage = "Network(): Error while reading out junctions.";
      inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                      nl->StringAtom ( strErrorMessage ) );
      inout_bCorrect = false;
      return;
    }

    // Read attributes from list
    int iRoute1Id = nl->IntValue ( nl->First ( xCurrentJunction ) );
    double dMeas1 = nl->RealValue ( nl->Second ( xCurrentJunction ) );
    int iRoute2Id = nl->IntValue ( nl->Third ( xCurrentJunction ) );
    double dMeas2 = nl->RealValue ( nl->Fourth ( xCurrentJunction ) );
    int iConnectivityCode= nl->IntValue ( nl->Fifth ( xCurrentJunction ) );
    // The location of the junction "Point" is calculated in the load-method

    // Set all necessary attributes
    pNewJunction->PutAttribute ( JUNCTION_ROUTE1_ID,
                                 new CcInt ( true, iRoute1Id ) );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE1_MEAS,
                                 new CcReal ( true, dMeas1 ) );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE2_ID,
                                 new CcInt ( true, iRoute2Id ) );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE2_MEAS,
                                 new CcReal ( true, dMeas2 ) );
    pNewJunction->PutAttribute ( JUNCTION_CC,
                                 new CcInt ( true, iConnectivityCode ) );

    // Append new junction
    pJunctions->AppendTuple ( pNewJunction );
    if ( pNewJunction )
    {
      pNewJunction->DeleteIfAllowed();
      pNewJunction=0;
    }
  }

  Load ( m_iId,
         m_scalefactor,
         pRoutes,
         pJunctions );

  delete pRoutes;
  delete pJunctions;


  m_bDefined = true;
}

/*
Destructor

*/
Network::~Network()
{
  delete m_pRoutes;

  delete m_pJunctions;

  delete m_pSections;

  delete m_pBTreeRoutes;

  delete m_pRTreeRoutes;

  delete m_pBTreeJunctionsByRoute1;

  delete m_pBTreeJunctionsByRoute2;

  delete m_pBTreeSectionsByRoute;

//  delete alldistance;
}

/*
1.3.1.3 Methods class ~network~

Destroy -- Removing a network from the database

*/
void Network::Destroy()
{
  assert ( m_pRoutes != 0 );
  m_pRoutes->Delete(); m_pRoutes = 0;

  assert ( m_pJunctions != 0 );
  m_pJunctions->Delete(); m_pJunctions = 0;

  assert ( m_pSections != 0 );
  m_pSections->Delete(); m_pSections = 0;

  assert ( m_pBTreeRoutes != 0 );
  m_pBTreeRoutes->DeleteFile();
  delete m_pBTreeRoutes; m_pBTreeRoutes = 0;

  assert ( m_pRTreeRoutes != 0 );
  //m_pRTreeRoutes->DeleteFile();
  delete m_pRTreeRoutes; m_pRTreeRoutes = 0;

  m_pBTreeJunctionsByRoute1->DeleteFile();
  delete m_pBTreeJunctionsByRoute1; m_pBTreeJunctionsByRoute1 = 0;

  m_pBTreeJunctionsByRoute2->DeleteFile();
  delete m_pBTreeJunctionsByRoute2; m_pBTreeJunctionsByRoute2 = 0;

//  m_xAdjacencyList.Destroy();
//  m_xSubAdjacencyList.Destroy();
//  m_reverseAdjacencyList.Destroy();
//  m_reverseSubAdjancencyList.Destroy();
  assert ( m_pBTreeSectionsByRoute != 0 );
  m_pBTreeSectionsByRoute->DeleteFile();
  delete m_pBTreeSectionsByRoute;
  m_pBTreeSectionsByRoute = 0;
/*
assert(alldistance != 0);
delete alldistance;

*/
}

/*
Load -- Create a network from two external relations

*/

void Network::Load ( int in_iId, double scale,
                     const Relation* in_pRoutes,
                     const Relation* in_pJunctions )
{
  m_iId = in_iId;
  m_scalefactor = scale;
  FillRoutes ( in_pRoutes );
  FillJunctions ( in_pJunctions );
  FillSections();
  FillAdjacencyLists();
//FillDistanceStorage();//store distance
  m_bDefined = true;
}

/*
Fill routes relation of network

*/
void Network::FillRoutes ( const Relation *routes )
{

  ListExpr ptrList = listutils::getPtrList(routes);

  string strQuery = "(consume (sort (feed (" + routesTypeInfo +
                    " (ptr " + nl->ToString(ptrList)  + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pRoutes = ( Relation * ) xResult.addr;
  // Create B-Tree for the routes
  ptrList = listutils::getPtrList(m_pRoutes);

  strQuery = "(createbtree (" + routesTypeInfo +
             " (ptr " + nl->ToString(ptrList) + "))" + " Id)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted ); // no query with side effects, please!
  m_pBTreeRoutes = ( BTree* ) xResult.addr;
  //Create R-Tree for the routes

  strQuery = "(bulkloadrtree(sortby(addid(feed (" + routesTypeInfo +
         " (ptr " + nl->ToString(ptrList) + "))))((Curve asc))) Curve)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pRTreeRoutes = ( R_Tree<2,TupleId>* ) xResult.addr;

}


/*
Fill junctions relation of network

*/
void Network::FillJunctions ( const Relation *in_pJunctions )
{
  /////////////////////////////////////////////////////////////////////
  //
  // Create new table for the junctions
  //
  ListExpr xTypeInfo;
  nl->ReadFromString ( junctionsInternalTypeInfo, xTypeInfo );
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xTypeInfo );
  Relation *pIntJunctions = new Relation ( xNumType, true );

  /////////////////////////////////////////////////////////////////////
  //
  // Iterator for the input-table with junctions
  //
  GenericRelationIterator* pJunctionsIter = in_pJunctions->MakeScan();
  Tuple* pCurrentJunction;
  while ( ( pCurrentJunction = pJunctionsIter->GetNextTuple() ) != 0 )
  {
    /////////////////////////////////////////////////////////////////////
    //
    // Create tuple for internal table and copy all attributes from input
    //
    Tuple* pNewJunction = new Tuple ( nl->Second ( xNumType ) );
    for ( int i = 0; i < pCurrentJunction->GetNoAttributes(); i++ )
    {
      pNewJunction->CopyAttribute ( i, pCurrentJunction, i );
    }


    /////////////////////////////////////////////////////////////////////
    //
    // Fill other fields of the table
    //

    // Store Pointer to the first route in the new relation.
    CcInt* pR1Id = ( CcInt* ) pCurrentJunction->GetAttribute (
JUNCTION_ROUTE1_ID );
    BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch ( pR1Id );
    int NextIter = pRoutesIter->Next();
    assert ( NextIter );
    TupleIdentifier *pR1RC = new TupleIdentifier ( true, pRoutesIter->GetId() );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE1_RC, pR1RC );

    // Calculate and store the exakt location of the junction.
    Tuple* pRoute = m_pRoutes->GetTuple ( pRoutesIter->GetId(), false );
    assert ( pRoute != 0 );
    SimpleLine* pLine = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    assert ( pLine != 0 );
    bool startSmaller =
      ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    CcReal* pMeas = ( CcReal* ) pNewJunction->GetAttribute (
JUNCTION_ROUTE1_MEAS );
    Point* pPoint = new Point ( true );
    pLine->AtPosition ( pMeas->GetRealval(), startSmaller, *pPoint );
    pNewJunction->PutAttribute ( JUNCTION_POS, pPoint );

    pRoute->DeleteIfAllowed();
    delete pRoutesIter;

    // Store Pointer to the second route in the new relation.
    CcInt* pR2Id = ( CcInt* ) pCurrentJunction->GetAttribute (
JUNCTION_ROUTE2_ID );
    pRoutesIter = m_pBTreeRoutes->ExactMatch ( pR2Id );
    NextIter = pRoutesIter->Next();
    assert ( NextIter ); // no query with side effects, please!
    TupleIdentifier *pR2RC = new TupleIdentifier ( true, pRoutesIter->GetId() );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE2_RC, pR2RC );
    delete pRoutesIter;

    // Pointers to sections are filled in FillSections
    pNewJunction->PutAttribute ( JUNCTION_SECTION_AUP_RC,
                                 new TupleIdentifier ( false ) );
    pNewJunction->PutAttribute ( JUNCTION_SECTION_ADOWN_RC,
                                 new TupleIdentifier ( false ) );
    pNewJunction->PutAttribute ( JUNCTION_SECTION_BUP_RC,
                                 new TupleIdentifier ( false ) );
    pNewJunction->PutAttribute ( JUNCTION_SECTION_BDOWN_RC,
                                 new TupleIdentifier ( false ) );

    /////////////////////////////////////////////////////////////////////
    //
    // Append new junction
    //
    pIntJunctions->AppendTuple ( pNewJunction );

    pCurrentJunction->DeleteIfAllowed();
    pNewJunction->DeleteIfAllowed();
  }
  delete pJunctionsIter;

  /////////////////////////////////////////////////////////////////////
  //
  // Sort the table which is now containing all junctions
  //
  ListExpr ptrList = listutils::getPtrList(pIntJunctions);

  string strQuery = "(consume (sortby (feed (" + junctionsInternalTypeInfo +
                    " (ptr " + nl->ToString(ptrList) +
                    "))) ((R1id asc)(Meas1 asc))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pJunctions = ( Relation * ) xResult.addr;

  // Delete internal table
  pIntJunctions->Delete();

  /////////////////////////////////////////////////////////////////////
  //
  // Create two b-trees for the junctions sorted by first and second id
  //

  ptrList = listutils::getPtrList(m_pJunctions);

  strQuery = "(createbtree (" + junctionsInternalTypeInfo +
             " (ptr " + nl->ToString(ptrList) + "))" + " R1id)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pBTreeJunctionsByRoute1 = ( BTree* ) xResult.addr;

  strQuery = "(createbtree (" + junctionsInternalTypeInfo +
             " (ptr " + nl->ToString(ptrList) + "))" + " R2id)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pBTreeJunctionsByRoute2 = ( BTree* ) xResult.addr;
}

/*
Fill routes relation of network

*/
void Network::FillSections()
{
  // The method will iterate over routes
  GenericRelationIterator* pRoutesIt = m_pRoutes->MakeScan();

  // Create relation for sections
  ListExpr xType;
  nl->ReadFromString ( sectionsInternalTypeInfo, xType );
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pSections = new Relation ( xNumType );
  /////////////////////////////////////////////////////////////////////
  //
  // Iterate over all Routes
  //
  Tuple* pRoute = 0;
  TupleId iSectionTid = 0;
  while ( ( pRoute = pRoutesIt->GetNextTuple() ) != 0 )
  {
    // Current position on route - starting at the beginning of the route
    double dCurrentPosOnRoute = 0.0;
    SimpleLine* pRouteCurve =
      ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE);
    TupleId iTupleId = pRoute->GetTupleId();
    CcInt* xRouteId = ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID );
    int iRouteId = xRouteId->GetIntval();
    bool bDual = ((CcBool*) pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
    /////////////////////////////////////////////////////////////////////
    //
    // We need to find all junctions belonging to this route
    //
    vector<JunctionSortEntry> xJunctions;
    xJunctions.clear();
    GetJunctionsOnRoute ( xRouteId,
                          xJunctions );
    /////////////////////////////////////////////////////////////////////
    //
    // Now that we found all relevant junctions we can iterate over them.
    //
    JunctionSortEntry xCurrentEntry;
    xCurrentEntry.m_pJunction = 0;
    xCurrentEntry.m_bFirstRoute = false;
    for ( size_t i = 0; i < xJunctions.size(); i++ )
    {
      // Get next junction
      xCurrentEntry = xJunctions[i];

      // Find values for the new section
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = xCurrentEntry.GetRouteMeas();

      // If the first junction is at the very start of the route, no
      // section will be added
      if ( xCurrentEntry.GetRouteMeas() == 0.0 )
      {
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        if ( xCurrentEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_ADOWN_RC );
          xAttrs.push_back ( new TupleIdentifier ( true,
                                       0));
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BDOWN_RC );
          xAttrs.push_back ( new TupleIdentifier ( true,
                                         0) );
        }
        m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                    xIndices,
                                    xAttrs );
        continue;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Create a new section
      //
      // Section will only be created if the length is > 0. Otherwise the
      // one before remains valid.
      if ( !AlmostEqualAbsolute(dEndPos, dStartPos,m_scalefactor*0.01))
      {
        // A sline for the section
        SimpleLine* pLine = new SimpleLine ( 0 );

        // Take start from the route
        bool bStartSmaller = ( ( CcBool* ) pRoute->GetAttribute (
                                   ROUTE_STARTSSMALLER ) )->GetBoolval();

        pRouteCurve->SubLine ( dStartPos,
                               dEndPos,
                               bStartSmaller,
                               *pLine );

        // Find out, if the orientation of the subline differs from the position
        // of the line. If so, the direction has to be changed.
        /*bool bLineStartsSmaller;
        Point pStartPoint ( true );
        pRouteCurve->AtPosition ( dStartPos, bStartSmaller, pStartPoint );
        Point pEndPoint ( true );
        pRouteCurve->AtPosition ( dEndPos, bStartSmaller, pEndPoint );
        if ( pStartPoint.GetX() < pEndPoint.GetX() ||
                (
                    pStartPoint.GetX() == pEndPoint.GetX() &&
                    pStartPoint.GetY() < pEndPoint.GetY()
                )
           )
        {
          // Normal orientation
          bLineStartsSmaller = true;
        }
        else
        {
          // Opposite orientation
          bLineStartsSmaller = false;
        }*/

        // The new section
        Tuple* pNewSection = new Tuple ( nl->Second ( xNumType ) );
        pNewSection->PutAttribute ( SECTION_RID, new CcInt ( true, iRouteId ) );
        pNewSection->PutAttribute ( SECTION_DUAL, new CcBool ( true, bDual ) );
        pNewSection->PutAttribute ( SECTION_MEAS1, new CcReal ( true,
                                                                dStartPos) );
        pNewSection->PutAttribute ( SECTION_MEAS2, new CcReal ( true,
                                                                dEndPos ));
        pNewSection->PutAttribute ( SECTION_RRC, new TupleIdentifier ( true,
                                    iTupleId ) );
        pNewSection->PutAttribute ( SECTION_CURVE, pLine );
        pNewSection->PutAttribute ( SECTION_CURVE_STARTS_SMALLER,
                                    new CcBool ( true,
                                                 pLine->GetStartSmaller()));
        pNewSection->PutAttribute ( SECTION_SID,
                                    new CcInt ( true,
                                                m_pSections->GetNoTuples()+1 ));
        m_pSections->AppendTuple ( pNewSection );
        iSectionTid++;
        pNewSection->DeleteIfAllowed();
        // Update position for next loop
        dCurrentPosOnRoute = dEndPos;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Store ID of new section in junction behind that section.
      //
      vector<int> xIndices;
      vector<Attribute*> xAttrs;
      if ( xCurrentEntry.m_bFirstRoute )
      {
        xIndices.push_back ( JUNCTION_SECTION_ADOWN_RC );
        xAttrs.push_back ( new TupleIdentifier ( true, iSectionTid ) );
      }
      else
      {
        xIndices.push_back ( JUNCTION_SECTION_BDOWN_RC );
        xAttrs.push_back ( new TupleIdentifier ( true, iSectionTid ) );
      }
      m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                  xIndices,
                                  xAttrs );
      if ( AlmostEqual(pRouteCurve->Length(), xCurrentEntry.GetRouteMeas()))
      {
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        if ( xCurrentEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_AUP_RC );
          xAttrs.push_back ( new TupleIdentifier ( true,
                                   0 ) );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BUP_RC );
          xAttrs.push_back ( new TupleIdentifier ( true,
                                   0));
        }
        m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                    xIndices,
                                    xAttrs );
      }

    } // End junctions-loop



    /////////////////////////////////////////////////////////////////////
    //
    // The last section of the route is still missing, if the last
    // junction is not at the end of the route.
    //
    if ( !AlmostEqual(pRouteCurve->Length(), dCurrentPosOnRoute) ||
            dCurrentPosOnRoute == 0.0 )
    {
      // Find values for the new section
      int iRouteId =
        ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ))->GetIntval();
      bool bDual =
        ( ( CcBool* ) pRoute->GetAttribute ( ROUTE_DUAL ))->GetBoolval();
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = pRouteCurve->Length();
      TupleId iTupleId = pRoute->GetTupleId();

      // Calculate line
      SimpleLine* pLine = new SimpleLine ( 0 );
      bool bStartSmaller = ( ( CcBool* ) pRoute->GetAttribute (
                                 ROUTE_STARTSSMALLER ) )->GetBoolval();
      pRouteCurve->SubLine ( dStartPos,
                             dEndPos,
                             bStartSmaller,
                             *pLine );

      // Find out, if the orientation of the subline differs from the position
      // of the sline. If so, the direction has to be changed.
      /*bool bLineStartsSmaller;
      Point pStartPoint ( true );
      pRouteCurve->AtPosition ( dStartPos, bStartSmaller, pStartPoint );
      Point pEndPoint ( true );
      pRouteCurve->AtPosition ( dEndPos, bStartSmaller, pEndPoint );
      if ( pStartPoint.GetX() < pEndPoint.GetX() ||
              (
                  pStartPoint.GetX() == pEndPoint.GetX() &&
                  pStartPoint.GetY() < pEndPoint.GetY()
              )
         )
      {
        // Normal orientation
        bLineStartsSmaller = true;
      }
      else
      {
        // Opposite orientation
        bLineStartsSmaller = false;
      }*/

      // Create a new Section
      Tuple* pNewSection = new Tuple ( nl->Second ( xNumType ) );
      pNewSection->PutAttribute ( SECTION_RID, new CcInt ( true, iRouteId ) );
      pNewSection->PutAttribute ( SECTION_DUAL, new CcBool ( true, bDual ) );
      pNewSection->PutAttribute ( SECTION_MEAS1,new CcReal ( true, dStartPos ));
      pNewSection->PutAttribute ( SECTION_MEAS2, new CcReal ( true, dEndPos ) );
      pNewSection->PutAttribute ( SECTION_RRC,
                                  new TupleIdentifier ( true, iTupleId ) );
      pNewSection->PutAttribute ( SECTION_CURVE, pLine );
      pNewSection->PutAttribute ( SECTION_CURVE_STARTS_SMALLER,
                                  new CcBool ( true, pLine->GetStartSmaller()));
      pNewSection->PutAttribute ( SECTION_SID,
                          new CcInt ( true, m_pSections->GetNoTuples() + 1 ) );
      m_pSections->AppendTuple ( pNewSection );
      iSectionTid++;
      pNewSection->DeleteIfAllowed();
      // Store ID of new section in Junction
      if ( xCurrentEntry.m_pJunction != 0 )
      {
        vector<int> xIndicesLast;
        vector<Attribute*> xAttrsLast;
        if ( xCurrentEntry.m_bFirstRoute )
        {
          xIndicesLast.push_back ( JUNCTION_SECTION_AUP_RC );
          xAttrsLast.push_back ( new TupleIdentifier ( true,
                                 iSectionTid ) );
        }
        else
        {
          xIndicesLast.push_back ( JUNCTION_SECTION_BUP_RC );
          xAttrsLast.push_back ( new TupleIdentifier ( true,
                                 iSectionTid ) );
        }
        m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                    xIndicesLast,
                                    xAttrsLast );
      }
    } // end if
    ////////////////////////////////////////////////////////////////////
    //
    // Fill Up-Pointers of all sections but the last
    //
    vector<JunctionSortEntry> yJunctions;
    yJunctions.clear();
    GetJunctionsOnRoute ( xRouteId,
                          yJunctions );
    if ( yJunctions.size() > 2 )
    {
      for ( int i = yJunctions.size()-2; i >= 0; i-- )
      {
        // Get next junction
        JunctionSortEntry xEntry = yJunctions[i];
        JunctionSortEntry xEntryBehind = yJunctions[i + 1];

        vector<int> xIndices;
        if ( xEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_AUP_RC );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BUP_RC );
        }
        vector<Attribute*> xAttrs;
        if ( AlmostEqual(xEntryBehind.GetRouteMeas(), xEntry.GetRouteMeas()))
        {
          // Two junctions at the same place. In this case they do have
          // the same up-pointers
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
        }
        else
        {
          // Junctions not on the same place. The down-pointer of the second is
          // the up-pointer of the first.
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
        }
        m_pJunctions->UpdateTuple ( xEntry.m_pJunction,
                                    xIndices,
                                    xAttrs );
      }
    }
    else
    {
      if ( yJunctions.size() == 2 )
      {
        JunctionSortEntry xEntry = yJunctions[0];
        JunctionSortEntry xEntryBehind = yJunctions[1];
        vector<int> xIndices;
        if ( xEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_AUP_RC );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BUP_RC );
        }
        vector<Attribute*> xAttrs;
        if ( AlmostEqual( xEntry.GetRouteMeas(),xEntryBehind.GetRouteMeas()))
        {
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          m_pJunctions->UpdateTuple ( xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs );
        }
        else
        {
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          m_pJunctions->UpdateTuple ( xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs );
        }
      }
    }
    pRoute->DeleteIfAllowed();

    // delete Tuples from xJunctions
    for ( unsigned int i=0;i<xJunctions.size();i++ )
    {
      if ( xJunctions[i].m_pJunction )
      {
        xJunctions[i].m_pJunction->DeleteIfAllowed();
      }
    }
    xJunctions.clear();
    // delete Tuples from yJunctions
    for ( unsigned int i=0;i<yJunctions.size();i++ )
    {
      if ( yJunctions[i].m_pJunction )
      {
        yJunctions[i].m_pJunction->DeleteIfAllowed();
      }
    }
    yJunctions.clear();

  } // End while Routes
  delete pRoutesIt;

  // Create B-Tree for the sections

  ListExpr ptrList = listutils::getPtrList(m_pSections);
  string strQuery = "(createbtree (" + sectionsInternalTypeInfo +
                    " (ptr " + nl->ToString(ptrList) + "))" + " Rid)";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pBTreeSectionsByRoute = ( BTree* ) xResult.addr;
}

/*
Fill adjacency list of network

*/
void Network::FillAdjacencyLists()
{
  // Adjust the adjacenzy list to the correct size. From each
  // section four directions are possible - including u-turns
  m_xAdjacencyList.resize ( m_pSections->GetNoTuples() * 2 );
  m_reverseAdjacencyList.resize(m_pSections->GetNoTuples()*2);
  for ( int i = 0; i < m_pSections->GetNoTuples() * 2; i++ )
  {
    m_xAdjacencyList.Put ( i, AdjacencyListEntry ( -1, -1 ) );
    m_reverseAdjacencyList.Put(i, AdjacencyListEntry(-1,-1));
  }

  GenericRelationIterator* pJunctionsIt = m_pJunctions->MakeScan();
  Tuple* pCurrentJunction;

  /////////////////////////////////////////////////////////////////////////
  //
  // In a first step all pairs of adjacent sections will be collected
  //
  vector<DirectedSectionPair> xList;
  vector<DirectedSectionPair> xReverseList;

  while ( ( pCurrentJunction = pJunctionsIt->GetNextTuple() ) != 0 )
  {
    //////////////////////////////////
    //
    // Retrieve the connectivity code
    //
    int iCc =
       ((CcInt*) pCurrentJunction->GetAttribute ( JUNCTION_CC ))->GetIntval();
    ConnectivityCode xCc ( iCc );

    //////////////////////////////////
    //
    // Retrieve the four sections - if they exist
    //
    // (This should also be possible without loading the Section itself)
    //

    TupleId tidpAUp, tidpADown, tidpBUp, tidpBDown;

    tidpAUp = ( ( TupleIdentifier* )
         pCurrentJunction->GetAttribute ( JUNCTION_SECTION_AUP_RC ))->GetTid();
    tidpADown = ( ( TupleIdentifier* )
        pCurrentJunction->GetAttribute ( JUNCTION_SECTION_ADOWN_RC ))->GetTid();
    tidpBUp = ( ( TupleIdentifier* )
       pCurrentJunction->GetAttribute ( JUNCTION_SECTION_BUP_RC ))->GetTid();
    tidpBDown = ( ( TupleIdentifier* )
        pCurrentJunction->GetAttribute ( JUNCTION_SECTION_BDOWN_RC ))->GetTid();

    //////////////////////////////////
    //
    // If a section is existing and the transition is possible
    // it will be added to the adjacency list.
    //
///////////////////////////////////////////////////////////////////////////////
// Implementationversion
///////////////////////////////////////////////////////////////////////////////
//     FillAdjacencyPair ( tidpAUp, false, tidpAUp, true, xCc, AUP_AUP, xList );
//     FillAdjacencyPair ( tidpAUp, false, tidpADown, false, xCc, AUP_ADOWN,
//                         xList);
//     FillAdjacencyPair ( tidpAUp, false, tidpBUp, true, xCc, AUP_BUP, xList );
//     FillAdjacencyPair ( tidpAUp, false, tidpBDown, false, xCc, AUP_BDOWN,
//                         xList);
//
//  FillAdjacencyPair ( tidpADown, true, tidpAUp, true, xCc, ADOWN_AUP, xList );
//     FillAdjacencyPair ( tidpADown, true, tidpADown, false, xCc, ADOWN_ADOWN,
//                         xList);
//  FillAdjacencyPair ( tidpADown, true, tidpBUp, true, xCc, ADOWN_BUP, xList );
//     FillAdjacencyPair ( tidpADown, true, tidpBDown, false, xCc, ADOWN_BDOWN,
//                         xList);
//
//     FillAdjacencyPair ( tidpBUp, false, tidpAUp, true, xCc, BUP_AUP, xList );
//     FillAdjacencyPair ( tidpBUp, false, tidpADown, false, xCc, BUP_ADOWN,
//                         xList);
//     FillAdjacencyPair ( tidpBUp, false, tidpBUp, true, xCc, BUP_BUP, xList );
//     FillAdjacencyPair ( tidpBUp, false, tidpBDown, false, xCc, BUP_BDOWN,
//                         xList);
//
//  FillAdjacencyPair ( tidpBDown, true, tidpAUp, true, xCc, BDOWN_AUP, xList );
//     FillAdjacencyPair ( tidpBDown, true, tidpADown, false, xCc,BDOWN_ADOWN,
//                         xList);
//  FillAdjacencyPair ( tidpBDown, true, tidpBUp, true, xCc, BDOWN_BUP, xList );
//     FillAdjacencyPair ( tidpBDown, true, tidpBDown, false, xCc, BDOWN_BDOWN,
//                         xList);
////////////////////////////////////////////////////////////////////////////////
// Paperversion
///////////////////
    FillAdjacencyPair ( tidpAUp, false, tidpAUp, true, xCc, ADOWN_AUP, xList );
    FillAdjacencyPair ( tidpAUp, false, tidpADown, false, xCc, ADOWN_ADOWN,
                        xList);
    FillAdjacencyPair ( tidpAUp, false, tidpBUp, true, xCc, ADOWN_BUP, xList );
    FillAdjacencyPair ( tidpAUp, false, tidpBDown, false, xCc, ADOWN_BDOWN,
                        xList);

    FillAdjacencyPair ( tidpADown, true, tidpAUp, true, xCc, AUP_AUP, xList );
    FillAdjacencyPair ( tidpADown, true, tidpADown, false, xCc, AUP_ADOWN,
                        xList);
    FillAdjacencyPair ( tidpADown, true, tidpBUp, true, xCc, AUP_BUP, xList );
    FillAdjacencyPair ( tidpADown, true, tidpBDown, false, xCc, AUP_BDOWN,
                        xList);

    FillAdjacencyPair ( tidpBUp, false, tidpAUp, true, xCc, BDOWN_AUP, xList );
    FillAdjacencyPair ( tidpBUp, false, tidpADown, false, xCc, BDOWN_ADOWN,
                        xList);
    FillAdjacencyPair ( tidpBUp, false, tidpBUp, true, xCc, BDOWN_BUP, xList );
    FillAdjacencyPair ( tidpBUp, false, tidpBDown, false, xCc, BDOWN_BDOWN,
                        xList);

    FillAdjacencyPair ( tidpBDown, true, tidpAUp, true, xCc, BUP_AUP, xList );
    FillAdjacencyPair ( tidpBDown, true, tidpADown, false, xCc,BUP_ADOWN,
                        xList);
    FillAdjacencyPair ( tidpBDown, true, tidpBUp, true, xCc, BUP_BUP, xList );
    FillAdjacencyPair ( tidpBDown, true, tidpBDown, false, xCc, BUP_BDOWN,
                        xList);
//////////////////////////////////////////////////////////////////////////
// And the reverse adjacency list.
//////////////////////////////////////////////////////////////////////////////
// Implementationversion
///////////////////////////////////////////////////////////////////////////////

//     FillReverseAdjacencyPair ( tidpAUp, false, tidpAUp, true, xCc, AUP_AUP,
//                                xReverseList );
// FillReverseAdjacencyPair ( tidpAUp, false, tidpADown, false, xCc, AUP_ADOWN,
//                                xReverseList);
//     FillReverseAdjacencyPair ( tidpAUp, false, tidpBUp, true, xCc, AUP_BUP,
//                                xReverseList );
//  FillReverseAdjacencyPair ( tidpAUp, false, tidpBDown, false, xCc, AUP_BDOWN,
//                                xReverseList);
//
//    FillReverseAdjacencyPair ( tidpADown, true, tidpAUp, true, xCc, ADOWN_AUP,
//                                xReverseList );
//     FillReverseAdjacencyPair ( tidpADown, true, tidpADown, false, xCc,
//                                ADOWN_ADOWN, xReverseList);
//    FillReverseAdjacencyPair ( tidpADown, true, tidpBUp, true, xCc, ADOWN_BUP,
//                                xReverseList );
//     FillReverseAdjacencyPair ( tidpADown, true, tidpBDown, false, xCc,
//                                ADOWN_BDOWN, xReverseList);
//
//     FillReverseAdjacencyPair ( tidpBUp, false, tidpAUp, true, xCc, BUP_AUP,
//                                xReverseList );
//  FillReverseAdjacencyPair ( tidpBUp, false, tidpADown, false, xCc, BUP_ADOWN,
//                                xReverseList);
//     FillReverseAdjacencyPair ( tidpBUp, false, tidpBUp, true, xCc, BUP_BUP,
//                                xReverseList );
//  FillReverseAdjacencyPair ( tidpBUp, false, tidpBDown, false, xCc, BUP_BDOWN,
//                                xReverseList);
//
//    FillReverseAdjacencyPair ( tidpBDown, true, tidpAUp, true, xCc, BDOWN_AUP,
//                                xReverseList );
//     FillReverseAdjacencyPair ( tidpBDown, true, tidpADown, false, xCc,
//                                BDOWN_ADOWN, xReverseList);
//   FillReverseAdjacencyPair ( tidpBDown, true, tidpBUp, true, xCc, BDOWN_BUP,
//                                xReverseList );
//     FillReverseAdjacencyPair ( tidpBDown, true, tidpBDown, false, xCc,
//                                BDOWN_BDOWN, xReverseList);
////////////////////////////////////////////////////////////////////////////////
// Paperversion
////////////////////////////////
  FillReverseAdjacencyPair ( tidpAUp, false, tidpAUp, true, xCc, ADOWN_AUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpAUp, false, tidpADown, false, xCc, ADOWN_ADOWN,
                             xReverseList);
  FillReverseAdjacencyPair ( tidpAUp, false, tidpBUp, true, xCc, ADOWN_BUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpAUp, false, tidpBDown, false, xCc, ADOWN_BDOWN,
                             xReverseList);

  FillReverseAdjacencyPair ( tidpADown, true, tidpAUp, true, xCc, AUP_AUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpADown, true, tidpADown, false, xCc,
                             AUP_ADOWN, xReverseList);
  FillReverseAdjacencyPair ( tidpADown, true, tidpBUp, true, xCc, AUP_BUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpADown, true, tidpBDown, false, xCc,
                             AUP_BDOWN, xReverseList);

  FillReverseAdjacencyPair ( tidpBUp, false, tidpAUp, true, xCc, BDOWN_AUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpBUp, false, tidpADown, false, xCc, BDOWN_ADOWN,
                             xReverseList);
  FillReverseAdjacencyPair ( tidpBUp, false, tidpBUp, true, xCc, BDOWN_BUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpBUp, false, tidpBDown, false, xCc, BDOWN_BDOWN,
                             xReverseList);

  FillReverseAdjacencyPair ( tidpBDown, true, tidpAUp, true, xCc, BUP_AUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpBDown, true, tidpADown, false, xCc,
                             BUP_ADOWN, xReverseList);
  FillReverseAdjacencyPair ( tidpBDown, true, tidpBUp, true, xCc, BUP_BUP,
                             xReverseList );
  FillReverseAdjacencyPair ( tidpBDown, true, tidpBDown, false, xCc,
                             BUP_BDOWN, xReverseList);


    pCurrentJunction->DeleteIfAllowed();
  }
  delete pJunctionsIt;


  /////////////////////////////////////////////////////////////////////////
  //
  // Now - as the second step the adjacency lists are filled.
  //
  // Sort the lists by the first directed section

  stable_sort ( xList.begin(), xList.end() );

  DirectedSectionPair xLastPair;
  int iLow = 0;
  int iHigh = 0;
  int iIndex = 0;
  for ( size_t i = 0; i < xList.size(); i++ )
  {
    // Get next
    DirectedSectionPair xPair = xList[i];
    if ( i == 0 )
    {
      // Append new entry to sub-list
      m_xSubAdjacencyList.Append (
                DirectedSection ( xPair.m_iSecondSectionTid,
                                  xPair.m_bSecondUpDown ) );
      xLastPair = xPair;
    }
    else
    {
      // Check if entry allready exists in list. As the list is sorted it
      // has to be the entry before.
      if ( xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
           xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown ||
           xLastPair.m_iSecondSectionTid != xPair.m_iSecondSectionTid ||
           xLastPair.m_bSecondUpDown != xPair.m_bSecondUpDown )
      {
        // Append new entry to sub-list
        m_xSubAdjacencyList.Append(DirectedSection ( xPair.m_iSecondSectionTid,
                                                     xPair.m_bSecondUpDown ) );
      }
    }

    // Entry in adjacency list if all sections adjacent to one section have
    // been found. This is the case every time the first section changes. Never
    // at the first entry and always at the last.
    if ( i == xList.size() -1 ||
            (
                i != 0 &&
                (
                    xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
                    xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown
                )
            )
       )
    {
      if (i == xList.size() -1 ) iHigh = m_xSubAdjacencyList.Size()-1;
      else iHigh = m_xSubAdjacencyList.Size()-2;
      Tuple* sectTup =
        (Tuple*) m_pSections->GetTuple(xLastPair.m_iFirstSectionTid,false);
      int sid = ((CcInt*)sectTup->GetAttribute(SECTION_SID))->GetIntval();
      sectTup->DeleteIfAllowed();
      iIndex = 2 * ( sid-1 );
      iIndex += xLastPair.m_bFirstUpDown ? 1 : 0;
      m_xAdjacencyList.Put ( iIndex, AdjacencyListEntry ( iLow, iHigh ) );
      iLow = iHigh + 1;
    }
    xLastPair = xPair;
  }
  m_xAdjacencyList.TrimToSize();
  m_xSubAdjacencyList.TrimToSize();

  /////////////////////////////////////////////////////////////////////////
  //
  // Now - as the third step the reverse adjacency lists are filled.
  //
  // Sort the lists by the first directed section
  stable_sort ( xReverseList.begin(), xReverseList.end() );

  DirectedSectionPair xLastReversePair;
  iLow = 0;
  iHigh = 0;
  iIndex = 0;
  for ( size_t i = 0; i < xReverseList.size(); i++ )
  {
    // Get next
    DirectedSectionPair xReversePair = xReverseList[i];
    if ( i == 0 )
    {
      // Append new entry to sub-list
      m_reverseSubAdjancencyList.Append (
        DirectedSection ( xReversePair.m_iSecondSectionTid,
                          xReversePair.m_bSecondUpDown ) );
      xLastReversePair = xReversePair;
    }
    else
    {
      // Check if entry allready exists in list. As the list is sorted it
      // has to be the entry before.
      if(xLastReversePair.m_iFirstSectionTid !=
                                            xReversePair.m_iFirstSectionTid ||
         xLastReversePair.m_bFirstUpDown != xReversePair.m_bFirstUpDown ||
         xLastReversePair.m_iSecondSectionTid !=
                                           xReversePair.m_iSecondSectionTid ||
         xLastReversePair.m_bSecondUpDown != xReversePair.m_bSecondUpDown )
      {
        // Append new entry to sub-list
        m_reverseSubAdjancencyList.Append (
        DirectedSection ( xReversePair.m_iSecondSectionTid,
                          xReversePair.m_bSecondUpDown ) );
      }
    }
    // Entry in adjacency list if all sections adjacent to one section have
    // been found. This is the case every time the first section changes. Never
    // at the first entry and always at the last.
    if ( i == xReverseList.size() -1 ||
      (
        i != 0 &&
        (
          xLastReversePair.m_iFirstSectionTid !=
              xReversePair.m_iFirstSectionTid
          ||
          xLastReversePair.m_bFirstUpDown !=
            xReversePair.m_bFirstUpDown
        )
      )
    )
    {
      if (i == xReverseList.size() -1 )
        iHigh = m_reverseSubAdjancencyList.Size()-1;
      else
        iHigh = m_reverseSubAdjancencyList.Size()-2;
      Tuple* sectTup =
        (Tuple*)m_pSections->GetTuple(xLastReversePair.m_iFirstSectionTid,
                                      false);
      int sid = ((CcInt*)sectTup->GetAttribute(SECTION_SID))->GetIntval();
      sectTup->DeleteIfAllowed();
      iIndex = 2 * ( sid-1 );
      iIndex += xLastReversePair.m_bFirstUpDown ? 1 : 0;
      m_reverseAdjacencyList.Put ( iIndex, AdjacencyListEntry ( iLow, iHigh ) );
      iLow = iHigh + 1;
    }
    xLastReversePair = xReversePair;
  }
  m_reverseAdjacencyList.TrimToSize();
  m_reverseSubAdjancencyList.TrimToSize();
}

/*
Build vector of directed section pairs.

*/

void Network::FillAdjacencyPair ( const TupleId in_pFirstSection,
                                  const bool in_bFirstUp,
                                  const TupleId in_pSecondSection,
                                  const bool in_bSecondUp,
                                  const ConnectivityCode in_xCc,
                                  const Transition in_xTransition,
                                  vector<DirectedSectionPair> &inout_xPairs )
{
  if ( in_pFirstSection != 0 &&
       in_pSecondSection != 0 &&
       in_xCc.IsPossible ( in_xTransition ) )
  {
    inout_xPairs.push_back ( DirectedSectionPair ( in_pFirstSection,
                             in_bFirstUp,
                             in_pSecondSection,
                             in_bSecondUp ) );
  }
}

void Network::FillReverseAdjacencyPair ( const TupleId in_pFirstSection,
                                         const bool in_bFirstUp,
                                         const TupleId in_pSecondSection,
                                         const bool in_bSecondUp,
                                         const ConnectivityCode in_xCc,
                                         const Transition in_xTransition,
                                  vector<DirectedSectionPair> &inout_xPairs )
{
  if ( in_pFirstSection != 0 &&
       in_pSecondSection != 0 &&
       in_xCc.IsPossible ( in_xTransition ) )
  {
    inout_xPairs.push_back ( DirectedSectionPair ( in_pSecondSection,
                                                   in_bSecondUp,
                                                   in_pFirstSection,
                                                   in_bFirstUp ) );
  }
}

/*
c.

*/
bool Network::InShortestPath ( GPoint*start, GPoint *to,
                               GLine *result ) const
{
  GPoint* end = new GPoint ( *to );//copy the gpoint
  result->Clear();
  if ( start == 0 || end == 0 || !start->IsDefined() ||
          !end->IsDefined() )
  {
    sendMessage ( "Both gpoints must exist and be defined." );
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }
  // Check wether both points belong to the same network
  if ( start->GetNetworkId() != end->GetNetworkId() )
  {
    sendMessage ( "Both gpoints belong to different networks." );
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }

  result->SetNetworkId ( GetId() );

  // Get sections where the path should start or end
  Tuple* startSection = GetSectionOnRoute ( start );
  if ( startSection == 0 )
  {
    sendMessage ( "Starting GPoint not found in network." );
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }
  Tuple* endSection = GetSectionOnRoute ( end );
  if ( endSection == 0 )
  {
    sendMessage ( "End GPoint not found in network." );
    startSection->DeleteIfAllowed();
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }
////////////////////////////////////////////////////
  bool junctionpoint = false;
  Point* endp = new Point();
  GetPointOnRoute ( end,endp ); //end point
  vector<JunctionSortEntry> juns;
  CcInt* routeid = new CcInt ( true,end->GetRouteId() );
  GetJunctionsOnRoute ( routeid,juns );
  for ( unsigned int i = 0;i < juns.size();i++ )
  {
    Tuple* t = juns[i].m_pJunction;
    if ( ( ( CcInt* ) t->GetAttribute ( JUNCTION_ROUTE1_ID ) )->GetIntval() ==
            end->GetRouteId() &&
            AlmostEqualAbsolute (
              ((CcReal*)t->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval(),
              end->GetPosition(), 0.1*m_scalefactor))
      junctionpoint = true;
    if ( ( ( CcInt* ) t->GetAttribute ( JUNCTION_ROUTE2_ID ) )->GetIntval() ==
            end->GetRouteId() &&
            AlmostEqualAbsolute (
              ((CcReal*)t->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval(),
              end->GetPosition(), 0.1*m_scalefactor))
      junctionpoint = true;
  }
  vector<TupleId> secjunid;
  if ( junctionpoint )  //it is a junction point
  {
    vector<DirectedSection> sectionlist;
    if ( AlmostEqualAbsolute( end->GetPosition(),
             ((CcReal*)endSection->GetAttribute(SECTION_MEAS1))->GetRealval(),
             0.1*m_scalefactor))
      GetAdjacentSections ( endSection->GetTupleId(),false,sectionlist );
    else
      GetAdjacentSections ( endSection->GetTupleId(),true,sectionlist );
    for ( unsigned int i = 0;i < sectionlist.size();i++ )
    {
      if ( sectionlist[i].GetSectionTid() != endSection->GetTupleId() )
        secjunid.push_back ( sectionlist[i].GetSectionTid() );
    }
  }
  endp->DeleteIfAllowed();
  routeid->DeleteIfAllowed();
/////////////////////////////////////////////////////
// Calculate the shortest path using dijkstras algorithm.



  TupleId startSectTID = startSection->GetTupleId();
  TupleId lastSectTID = endSection->GetTupleId();

  if ( startSectTID == lastSectTID )
  {
    result->AddRouteInterval ( start->GetRouteId(), start->GetPosition(),
                               end->GetPosition() );
  }
  else
  {

//Initialize PriorityQueue

    PrioQueue *prioQ = new PrioQueue ( 0 );
    SectIDTree *visitedSect = 0;
    double sectMeas1 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    double dist = 0.0;
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    if ( start->GetSide() == 0 )
    {
      dist = start->GetPosition() - sectMeas1;
      GetAdjacentSections ( startSectTID, false, adjSectionList );
      SectIDTree *startTree = new SectIDTree ( startSectTID,
              ( TupleId ) numeric_limits<long>::max(),
              false,
              numeric_limits<int>::max() );
      visitedSect = startTree;
      for ( size_t i = 0;  i < adjSectionList.size(); i++ )
      {
        DirectedSection actNextSect = adjSectionList[i];
        if ( actNextSect.GetSectionTid() != startSectTID )
        {
          PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID );
          prioQ->Insert ( *actEntry, visitedSect, 0 ) ;
          delete actEntry;
        }
      }
      adjSectionList.clear();
    }
    else
    {
      if ( start->GetSide() == 1 )
      {
        dist = sectMeas2 - start->GetPosition();
        SectIDTree *startTree = new SectIDTree ( startSectTID,
                ( TupleId ) numeric_limits<long>::max(),
                true,
                numeric_limits<int>::max() );
        visitedSect = startTree;
        GetAdjacentSections ( startSectTID, true, adjSectionList );
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID );
            prioQ->Insert ( *actEntry, visitedSect ,0);
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
      else
      {
        dist = start->GetPosition() - sectMeas1;
        GetAdjacentSections ( startSectTID, false, adjSectionList );
        bool first = true;
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            if ( first )
            {
              first = false;
              SectIDTree *startTree = new SectIDTree ( startSectTID,
                      ( TupleId ) numeric_limits<long>::max(),
                      false,
                      numeric_limits<int>::max() );
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID );
            prioQ->Insert ( *actEntry, visitedSect,0 );
            delete actEntry;
          }
        }
        adjSectionList.clear();
        dist = sectMeas2 -start->GetPosition();
        GetAdjacentSections ( startSectTID, true, adjSectionList );
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            if ( first )
            {
              first = false;
              SectIDTree *startTree = new SectIDTree ( startSectTID,
                      ( TupleId ) numeric_limits<long>::max(),
                      true,
                      numeric_limits<int>::max() );
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID );
            prioQ->Insert ( *actEntry, visitedSect,0 );
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
    }
// Use priorityQueue to find shortestPath.

    PQEntry *actPQEntry;
    bool found = false;
    while ( !prioQ->IsEmpty() && !found )
    {
      actPQEntry = prioQ->GetAndDeleteMin ( visitedSect );
      Tuple *actSection = GetSection ( actPQEntry->sectID );
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      dist = actPQEntry->distFromStart + fabs ( sectMeas2 - sectMeas1 );

//////////////////////////////////////
      if ( junctionpoint )   //end point is a junction point
      {
        for ( unsigned int i = 0;i < secjunid.size();i++ )
        {
          if ( secjunid[i] == actPQEntry->sectID )
          {
            lastSectTID = actPQEntry->sectID;
            Tuple* sect = GetSection ( lastSectTID );
            double m1 =
              ((CcReal*)sect->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
              ((CcReal*)sect->GetAttribute(SECTION_MEAS2))->GetRealval();
            if ( actPQEntry->upDownFlag )
            {
              GPoint* temp = new GPoint ( true,end->GetNetworkId(),
                                          end->GetRouteId(),m2,None );
              *end = *temp;
              temp->DeleteIfAllowed();
            }
            else
            {
              GPoint* temp = new GPoint ( true,end->GetNetworkId(),
                                          end->GetRouteId(),m1,None );
              *end = *temp;
              temp->DeleteIfAllowed();
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
      }
////////////////////////////////////

      if ( actPQEntry->sectID != lastSectTID )
      {
//Search further in the network unitl reached last section.
//Get adjacent sections and insert into priority Queue.

        adjSectionList.clear();
        GetAdjacentSections ( actPQEntry->sectID,
                              actPQEntry->upDownFlag,
                              adjSectionList );
        for ( size_t i = 0; i <adjSectionList.size();i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != actPQEntry->sectID )
          {
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(),
                                              dist,
                                              actNextSect.GetUpDownFlag(),
                                              actPQEntry->sectID );
            prioQ->Insert ( *actEntry, visitedSect,0 );
            delete actEntry;
          }
        }
        delete actPQEntry;
        actSection->DeleteIfAllowed();
      }
      else
      {

// Shortest Path found.
// Compute last route interval and resulting gline.

        found = true;
        double startRI, endRI;
        int actRouteId =
            ( ( CcInt* ) actSection->GetAttribute ( SECTION_RID )
)->GetIntval();
        if ( actPQEntry->upDownFlag == true )
        {
          startRI = sectMeas1;
          endRI = end->GetPosition();
        }
        else
        {
          startRI = sectMeas2;
          endRI = end->GetPosition();
        }

        actSection->DeleteIfAllowed();

//Get the sections used for shortest path and write them in right
//order (from start to end ) in the resulting gline. Because dijkstra gives
//the sections from end to start we first have to put the result sections on a
//stack to turn in right order.

        RIStackP *riStack = new RIStackP(0);
        riStack->Push(actRouteId, startRI, endRI);
        TupleId lastSectId = actPQEntry->sectID;
        SectIDTree *pElem = visitedSect->Find ( actPQEntry->beforeSectID );
        bool end = false;
        bool upDown;
        //   if (startRI >= endRI) upDown = false;
        if ( startRI > endRI || fabs ( startRI-endRI ) < 0.1 ) upDown = false;
        else upDown = true;
        while ( !end )
        {
          //GetSection
          actSection = GetSection ( pElem->sectID );
          actRouteId =
              ( ( CcInt* ) actSection->GetAttribute ( SECTION_RID )
)->GetIntval();
          sectMeas1 =
              ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
          sectMeas2 =
              ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();
          upDown = pElem->upDownFlag;
          if ( pElem->sectID != startSectTID )
          {
            if ( upDown )
              riStack->Push ( actRouteId, sectMeas1, sectMeas2);
            else
              riStack->Push ( actRouteId, sectMeas2, sectMeas1);
            lastSectId = pElem->sectID;
            pElem = visitedSect->Find ( pElem->beforeSectId );
          }
          else
          {
            end = true;
            GetAdjacentSections ( startSectTID, true, adjSectionList );
            size_t i = 0;
            bool stsectfound = false;
            while ( i < adjSectionList.size() && !stsectfound )
            {
              DirectedSection adjSection = adjSectionList[i];
              if ( adjSection.GetSectionTid() == lastSectId )
              {
                if ( fabs ( start->GetPosition()-sectMeas2 ) > 0.1 )
                {
                  stsectfound = true;
                  riStack->Push ( actRouteId, start->GetPosition(), sectMeas2);
                  end = true;
                }
              }
              i++;
            }
            adjSectionList.clear();
            if ( !stsectfound )
            {
              GetAdjacentSections ( startSectTID, false,
                                    adjSectionList );
              i = 0;
              while ( i < adjSectionList.size() && !stsectfound )
              {
                DirectedSection adjSection = adjSectionList[i];
                if ( adjSection.GetSectionTid() == lastSectId )
                {
                  if ( fabs ( start->GetPosition() - sectMeas1 ) > 0.1 )
                  {
                    stsectfound = true;
                    riStack->Push( actRouteId, start->GetPosition(), sectMeas1);
                    end = true;
                  }
                }
                i++;
              }
              adjSectionList.clear();
            }
          }
        }
        // Cleanup and return result
        riStack->StackToGLine ( result, GetId());
        riStack->Destroy();
        delete riStack;
        delete actPQEntry;
      }
    }
    visitedSect->Remove();
    prioQ->Destroy();
    delete prioQ;
  }
  startSection->DeleteIfAllowed();
  endSection->DeleteIfAllowed();
  result->SetSorted ( false );
  result->SetDefined ( true );
  result->TrimToSize();
  end->DeleteIfAllowed();
  return true;
};

/*
.

*/
void Network::FindSP ( TupleId j1, TupleId j2, double& length,
                       GLine* res ) const
{
  res->SetNetworkId ( GetId() );
  for ( int i = 1; i <= alldistance->GetNoTuples();i++ )
  {
    Tuple* tuple = alldistance->GetTuple ( i, false );
    TupleId jun1 = ( ( CcInt* ) tuple->GetAttribute ( 0 ) )->GetIntval();
    TupleId jun2 = ( ( CcInt* ) tuple->GetAttribute ( 1 ) )->GetIntval();
    if ( ( jun1 == j1 && jun2 == j2 ) ||
            ( jun1 ==j2 && jun2 == j1 ) )
    {
      length = ( ( CcReal* ) tuple->GetAttribute ( 2 ) )->GetRealval();
      GLine* gline = ( GLine* ) tuple->GetAttribute ( 3 );
      *res = *gline;
      tuple->DeleteIfAllowed();
      break;
    }
    tuple->DeleteIfAllowed();
  }
  res->TrimToSize();
}

/*
.

*/

void Network::FillDistanceStorage()
{
  ListExpr xType;
  nl->ReadFromString ( distancestorageTypeInfo,xType );
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xType );
  alldistance = new Relation ( xNumType );
  //store the distance between each two junction points

  for ( int i = 1;i <= m_pJunctions->GetNoTuples();i++ )
  {
    for ( int j = i+1; j <= m_pJunctions->GetNoTuples();j++ )
    {
      Tuple* jun1 = m_pJunctions->GetTuple ( i, false );
      Tuple* jun2 = m_pJunctions->GetTuple ( j , false);
      int rid1 = ( ( CcInt* ) jun1->GetAttribute ( JUNCTION_ROUTE1_ID )
)->GetIntval();
      int rid2 = ( ( CcInt* ) jun2->GetAttribute ( JUNCTION_ROUTE1_ID )
)->GetIntval();
      float pos1 =
          ( ( CcReal* ) jun1->GetAttribute ( JUNCTION_ROUTE1_MEAS )
)->GetRealval();
      float pos2 =
          ( ( CcReal* ) jun2->GetAttribute ( JUNCTION_ROUTE1_MEAS )
)->GetRealval();
      Side side = None;
      Point* p1 = ( Point* ) jun1->GetAttribute ( JUNCTION_POS );
      Point* p2 = ( Point* ) jun2->GetAttribute ( JUNCTION_POS );
      if ( AlmostEqualAbsolute( p1->GetX(),p2->GetX(),0.1*m_scalefactor) &&
           AlmostEqualAbsolute( p1->GetY(),p2->GetY(),0.1*m_scalefactor))
            //same junction point
        continue;
      GPoint* gp1 = new GPoint ( true,GetId(),rid1,pos1,side );
      GPoint* gp2 = new GPoint ( true,GetId(),rid2,pos2,side );
      Tuple* tuple = new Tuple ( nl->Second ( xNumType ) );
      tuple->PutAttribute ( 0,new TupleIdentifier ( true,i ) );
      tuple->PutAttribute ( 1,new TupleIdentifier ( true,j ) );
      GLine* gline = new GLine ( 0 );
      assert ( InShortestPath ( gp1,gp2,gline ) );
      tuple->PutAttribute ( 2,new CcReal ( true,gline->GetLength() ) );
      GLine* temp = new GLine ( 0 );
      temp->SetNetworkId ( gline->GetNetworkId() );
      RouteInterval ri;
      gline->Get ( 0,ri );
      temp->AddRouteInterval ( ri );//head
      gline->Get ( gline->Size()-1,ri );
      temp->AddRouteInterval ( ri );//tail
      tuple->PutAttribute ( 3,new GLine ( temp ) );
      temp->DeleteIfAllowed();
      gline->DeleteIfAllowed();
      alldistance->AppendTuple ( tuple );
      tuple->DeleteIfAllowed();
      gp1->DeleteIfAllowed();
      gp2->DeleteIfAllowed();
    }
  }
}

/*
Returning network parameters.

*/

int Network::GetId() const
{
  return m_iId;
}

Relation *Network::GetRoutes() const
{
  return m_pRoutes;
}


Relation *Network::GetJunctions() const
{

  ListExpr ptrList = listutils::getPtrList(m_pJunctions);

  string querystring = "(consume (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + nl->ToString(ptrList) + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted );
  return ( Relation * ) resultWord.addr;
}


void Network::GetJunctionsOnRoute ( CcInt* in_pRouteId,
                                    vector<JunctionSortEntry>& inout_xJunctions)
                                    const
{
  BTreeIterator* pJunctionsIt;
  pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch ( in_pRouteId );
  inout_xJunctions.clear();
  while(pJunctionsIt->Next())
  {
    Tuple* pCurrentJunction = m_pJunctions->GetTuple ( pJunctionsIt->GetId(),
                                                       false );
    inout_xJunctions.push_back(JunctionSortEntry(true, pCurrentJunction ));
  }
  delete pJunctionsIt;
  pJunctionsIt = m_pBTreeJunctionsByRoute2->ExactMatch ( in_pRouteId );
  while ( pJunctionsIt->Next() )
  {
    Tuple* pCurrentJunction = m_pJunctions->GetTuple ( pJunctionsIt->GetId(),
                                                       false );
    inout_xJunctions.push_back(JunctionSortEntry(false, pCurrentJunction));
  }
  delete pJunctionsIt;
  stable_sort(inout_xJunctions.begin(), inout_xJunctions.end());
}

Tuple* Network::GetSection ( const TupleId n )const
{
  return m_pSections->GetTuple ( n, false );
}


TupleId Network::GetTupleIdSectionOnRoute ( const GPoint* in_xGPoint )const
{
  CcInt *ciRouteId = new CcInt ( true, in_xGPoint->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect = 0;
  TupleId result;
  while ( pSectionIter->Next() )
  {
    result = pSectionIter->GetId();
    actSect =
        m_pSections->GetTuple ( pSectionIter->GetId(), false );
    if ( actSect != 0 )
    {
      double start =
          ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS1 ) )->GetRealval();
      double end =
          ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS2 ) )->GetRealval();
      if ( in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end)
      {
        delete pSectionIter;
        actSect->DeleteIfAllowed();
        return result;
      }
      else
      {
        if ( AlmostEqualAbsolute( in_xGPoint->GetPosition(), start,
                                  m_scalefactor*0.01))
        {
          delete pSectionIter;
          actSect->DeleteIfAllowed();
          return result;
        }
        else
        {
          if ( AlmostEqualAbsolute( in_xGPoint->GetPosition(), end,
                                    m_scalefactor*0.01 ))
          {
            Tuple *pRoute =
             GetRoute(((TupleIdentifier*)
              actSect->GetAttribute(SECTION_RRC))->GetTid());
            if (AlmostEqualAbsolute(
                  ((CcReal*)pRoute->GetAttribute(ROUTE_LENGTH))->GetRealval(),
                    end, m_scalefactor*0.01 ))
            {
              pRoute->DeleteIfAllowed();
              delete pSectionIter;
              actSect->DeleteIfAllowed();
              return result;
            }
            else
            {
              pRoute->DeleteIfAllowed();
            }
          }
        }
      }
      actSect->DeleteIfAllowed();
    }
  }
  delete pSectionIter;
  return 0;
}

Tuple* Network::GetSectionOnRoute (const  GPoint* in_xGPoint )const
{
  return GetSection ( GetTupleIdSectionOnRoute ( in_xGPoint ) );
}

/*
Returns the tuple from routes relation for the given route id.

*/

Tuple* Network::GetRoute ( const int in_RouteId )const
{
  CcInt* pRouteId = new CcInt ( true, in_RouteId );
  BTreeIterator *pRoutesIter = m_pBTreeRoutes->ExactMatch ( pRouteId );
  pRouteId->DeleteIfAllowed();
  Tuple *pRoute = 0;
  if ( pRoutesIter->Next() )
    pRoute = m_pRoutes->GetTuple ( pRoutesIter->GetId() , false);
  assert ( pRoute != 0 );
  delete pRoutesIter;
  return pRoute;

}

Tuple* Network::GetRoute (const  TupleId in_routeTID )const
{
  return m_pRoutes->GetTuple ( in_routeTID, false );
}

void Network::GetSectionsOfRouteInterval ( const RouteInterval *ri,
        vector<SectTreeEntry>& io_SectionIds )const
{
  double ristart = min ( ri->GetStartPos(), ri->GetEndPos() );
  double riend = max ( ri->GetStartPos(), ri->GetEndPos() );
  CcInt* ciRouteId = new CcInt ( true, ri->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect;
  TupleId actSectTID;
  while ( pSectionIter->Next() )
  {
    bool bsectstart = true;
    bool bsectend = true;
    actSectTID = pSectionIter->GetId();
    actSect = m_pSections->GetTuple ( actSectTID, false );
    assert ( actSect != 0 );
    double start =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS1 ) )->GetRealval();
    double end =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS2 ) )->GetRealval();
    if (( ristart <= start && riend >= end ) ||
          ( start <= ristart && end >= ristart ) ||
          ( start <= riend && end >= riend ))
    {
      if (start < ristart)
      {
        start = ristart;
        bsectstart = false;
      }
      if (riend < end)
      {
        end = riend;
        bsectend = false;
      }
      SectTreeEntry *sect =
        new SectTreeEntry ( actSect->GetTupleId(), ri->GetRouteId(), start,
                            end, bsectstart, bsectend );
      io_SectionIds.push_back(*sect);
      delete sect;
    }
    actSect->DeleteIfAllowed();
    if ( riend <= end ) break;
  }
  delete pSectionIter;
};

void Network::GetSectionsOfRoutInterval ( const RouteInterval *ri,
        vector<TupleId> &res )const
{
  res.clear();
  double ristart = min ( ri->GetStartPos(), ri->GetEndPos() );
  double riend = max ( ri->GetStartPos(), ri->GetEndPos() );
  CcInt* ciRouteId = new CcInt ( true, ri->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect;
  TupleId actSectTID;
  //bool bsectstart = true;
  //bool bsectend = true;
  while ( pSectionIter->Next() )
  {
    actSectTID = pSectionIter->GetId();
    actSect = m_pSections->GetTuple ( actSectTID, false );
    assert ( actSect != 0 );
    double start =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS1 ) )->GetRealval();
    double end =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS2 ) )->GetRealval();
    if ( AlmostEqualAbsolute ( ristart, riend, m_scalefactor*0.01 ) &&
      ( AlmostEqualAbsolute ( ristart, start, m_scalefactor*0.01 ) ||
              AlmostEqualAbsolute ( ristart, end, m_scalefactor*0.01)))
    {
      res.push_back ( actSectTID );
      actSect->DeleteIfAllowed();
      break;
    }
    else
    {
      if ( ( ( ristart <= start && end <= riend ) ||
              ( start <= ristart && end > ristart ) ||
              ( start < riend && riend <= end ) ) &&
            (!( AlmostEqualAbsolute ( ristart, end, m_scalefactor*0.01 ) ||
                AlmostEqualAbsolute (start, riend, m_scalefactor*0.01 ))))
        res.push_back ( actSectTID );
    }
    actSect->DeleteIfAllowed();
  }
  delete pSectionIter;
}

/*
Returns the spatial position of the gpoint.

*/
void Network::GetPointOnRoute ( const GPoint* in_pGPoint, Point*& res )const
{
  /*Point *res = new Point(false);*/
  CcInt* pRouteId = new CcInt ( true, in_pGPoint->GetRouteId() );
  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch ( pRouteId );
  pRouteId->DeleteIfAllowed();
  Tuple *pRoute = 0;
  if ( pRoutesIter->Next() )
    pRoute = m_pRoutes->GetTuple ( pRoutesIter->GetId(), false );
  assert ( pRoute != 0 );
  SimpleLine* pLine = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
  bool startSmaller =
    ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
  assert ( pLine != 0 );
  pLine->AtPosition ( in_pGPoint->GetPosition(), startSmaller, *res );
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
  /*return res;*/
}

Relation* Network::GetSectionsInternal()const
{
  return m_pSections;
}

Relation* Network::GetSections()const
{
  ListExpr ptrList = listutils::getPtrList(m_pSections);

  string querystring = "(consume (feed (" + sectionsInternalTypeInfo +
                       " (ptr " + nl->ToString(ptrList) + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted );
  return ( Relation * ) resultWord.addr;
}

/*
Returns adjacent sections of iniSectionTId.

*/

void Network::GetAdjacentSections ( const TupleId in_iSectionTId,
                                    const bool in_bUpDown,
                                    vector<DirectedSection> &inout_xSections )
                                    const
{
  inout_xSections.clear();
  Tuple *pSect = GetSection ( in_iSectionTId );
  if ( pSect != 0 )
  {
    //cout << "found section" << endl;
    int iSectionId =
      ( ( CcInt* ) pSect->GetAttribute ( SECTION_SID ) )->GetIntval();
    pSect->DeleteIfAllowed();
    int iIndex = 2 * ( iSectionId-1 )  + ( in_bUpDown ? 1 : 0 );
    AdjacencyListEntry xEntry;
    m_xAdjacencyList.Get ( iIndex, xEntry );
    if ( xEntry.m_iHigh != -1 )
    {
      int iLow = xEntry.m_iLow;
      int iHigh = xEntry.m_iHigh;

      for ( int i = iLow; i <= iHigh; i++ )
      {
        DirectedSection xSection;
        m_xSubAdjacencyList.Get ( i, xSection );

        bool bUpDownFlag = xSection.GetUpDownFlag();
        TupleId iSectionTid = xSection.GetSectionTid();
        inout_xSections.push_back ( DirectedSection ( iSectionTid,
                                    bUpDownFlag ) );

      }
    }
  }
}


void Network::GetAdjacentSections(const int sectId, const bool upDown,
                                  DbArray<SectionValue> *resArray)const
{
  int iIndex = 2 * ( sectId-1 )  + ( upDown ? 1 : 0 );
  AdjacencyListEntry xEntry;
  m_xAdjacencyList.Get ( iIndex, xEntry );
  if ( xEntry.m_iHigh != -1 )
  {
    int iLow = xEntry.m_iLow;
    int iHigh = xEntry.m_iHigh;

    for ( int i = iLow; i <= iHigh; i++ )
    {
      DirectedSection xSection;
      m_xSubAdjacencyList.Get ( i, xSection );

      bool bUpDownFlag = xSection.GetUpDownFlag();
      Tuple* sectTuple = GetSection(xSection.GetSectionTid());
      int sectionID =
        ((CcInt*)sectTuple->GetAttribute(SECTION_SID))->GetIntval();
      sectTuple->DeleteIfAllowed();
      resArray->Append(SectionValue(sectionID,bUpDownFlag));
    }
  }
}

/*

Returns reverse adjacent sections of iniSectionTId.

*/

void Network::GetReverseAdjacentSections ( const TupleId in_iSectionTId,
                                           const bool in_bUpDown,
                                    vector<DirectedSection> &inout_xSections )
  const
{
  inout_xSections.clear();
  Tuple *pSect = GetSection ( in_iSectionTId );
  if ( pSect != 0 )
  {
    int iSectionId =
    ( ( CcInt* ) pSect->GetAttribute ( SECTION_SID ) )->GetIntval();
    pSect->DeleteIfAllowed();
    int iIndex = 2 * ( iSectionId-1 )  + ( in_bUpDown ? 1 : 0 );
    AdjacencyListEntry xEntry;
    m_reverseAdjacencyList.Get ( iIndex, xEntry );
    if ( xEntry.m_iHigh != -1 )
    {
      int iLow = xEntry.m_iLow;
      int iHigh = xEntry.m_iHigh;

      for ( int i = iLow; i <= iHigh; i++ )
      {
        DirectedSection xSection;
        m_reverseSubAdjancencyList.Get ( i, xSection );

        bool bUpDownFlag = xSection.GetUpDownFlag();
        TupleId iSectionTid = xSection.GetSectionTid();
        inout_xSections.push_back ( DirectedSection ( iSectionTid,
                                                      bUpDownFlag ) );

      }
    }
  }
}


void Network::GetReverseAdjacentSections(const int sectId,
                                         const bool upDown,
                                  DbArray<SectionValue> *resArray)
  const
{
  int iIndex = 2 * ( sectId-1 )  + ( upDown ? 1 : 0 );
  AdjacencyListEntry xEntry;
  m_reverseAdjacencyList.Get ( iIndex, xEntry );
  if ( xEntry.m_iHigh != -1 )
  {
    int iLow = xEntry.m_iLow;
    int iHigh = xEntry.m_iHigh;

    for ( int i = iLow; i <= iHigh; i++ )
    {
      DirectedSection xSection;
      m_reverseSubAdjancencyList.Get ( i, xSection );

      bool bUpDownFlag = xSection.GetUpDownFlag();
      Tuple* sectTuple = GetSection(xSection.GetSectionTid());
      int sectionID =
      ((CcInt*)sectTuple->GetAttribute(SECTION_SID))->GetIntval();
      sectTuple->DeleteIfAllowed();
      resArray->Append(SectionValue(sectionID, bUpDownFlag));
    }
  }
}

/*
Returns the route Interval between the two points

*/

void chkStartEndA ( double &StartPos, double &EndPos )
{
  double help;
  if ( StartPos > EndPos )
  {
    help = StartPos;
    StartPos = EndPos;
    EndPos = help;
  }
};

bool Network::ShorterConnection ( Tuple *route, double &start,
                                  double &end, double &dpos, double &dpos2,
                                  int &rid, int &ridt, Point p1,
                                  Point p2 )const
{
  if ( AlmostEqual ( p1.Distance ( p2 ), fabs ( end-start ) ) ) return false;
  GPoint *gp = new GPoint ( true, GetId(), route->GetTupleId(),
                            end - m_scalefactor*0.01 );
  TupleId pSection1 = GetTupleIdSectionOnRoute ( gp );
  gp->DeleteIfAllowed();
  vector<DirectedSection> pAdjSect1;
  vector<DirectedSection> pAdjSect2;
  pAdjSect1.clear();
  pAdjSect2.clear();
  GetAdjacentSections ( pSection1,true, pAdjSect1 );
  GetAdjacentSections ( pSection1,false, pAdjSect2 );
  if ( pAdjSect1.size() == 0 || pAdjSect2.size() == 0 )
  {
    pAdjSect1.clear();
    pAdjSect2.clear();
    return false;
  }
  else
  {
    size_t j = 0;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        bool startSmaller =
          ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
        /*
        if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
        {*/
        if (pCurve->AtPoint(p1, startSmaller, m_scalefactor, dpos, 0) &&
          pCurve->AtPoint(p2, startSmaller, m_scalefactor, dpos2, 0))
        {
          pAdjSect1.clear();
          pAdjSect2.clear();
          chkStartEndA ( dpos, dpos2 );
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2 - dpos ) < fabs ( end - start ) ) return  true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    pAdjSect1.clear();
    j = 0;
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        bool startSmaller =
            ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
        /*
        if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
        if (pCurve->AtPoint(p1, startSmaller, m_scalefactor,dpos, 0) &&
          pCurve->AtPoint(p2, startSmaller, m_scalefactor,dpos2, 0))
        {
          pAdjSect2.clear();
          chkStartEndA ( dpos, dpos2 );
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2-dpos ) < fabs ( end - start ) ) return true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    return false;
  }
}

bool Network::ShorterConnection2 ( Tuple *route, double &start,
                                   double &end, double &dpos, double &dpos2,
                                   int &rid, int &ridt, Point p1,
                                   Point p2 )const
{
  if ( AlmostEqualAbsolute ( p1.Distance ( p2 ), fabs ( end-start),
                             m_scalefactor) )
    return false;
  double difference = 0.0;
  if ( start < end && end > m_scalefactor*0.01 )
    difference = end -m_scalefactor*0.01;
  else
    if ( start < end && end <= m_scalefactor*0.01 )
      difference = m_scalefactor*0.01;
    else
      if ( start > end ) difference = end + m_scalefactor*0.01;
      else difference = end; //start == end
  GPoint *gp = new GPoint ( true, GetId(), route->GetTupleId(), difference );
  TupleId pSection1 = GetTupleIdSectionOnRoute ( gp );
  gp->DeleteIfAllowed();
  vector<DirectedSection> pAdjSect1;
  vector<DirectedSection> pAdjSect2;
  pAdjSect1.clear();
  pAdjSect2.clear();
  GetAdjacentSections ( pSection1,true, pAdjSect1 );
  GetAdjacentSections ( pSection1,false, pAdjSect2 );
  if ( pAdjSect1.size() == 0 || pAdjSect2.size() == 0 )
  {
    pAdjSect1.clear();
    pAdjSect2.clear();
    return false;
  }
  else
  {
    size_t j = 0;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        bool startSmaller =
          ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
        /*if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
        if (pCurve->AtPoint(p1, startSmaller,m_scalefactor, dpos) &&
          pCurve->AtPoint(p2, startSmaller, m_scalefactor,dpos2))
        {
          pAdjSect1.clear();
          pAdjSect2.clear();
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2 - dpos ) < fabs ( end - start ) ) return  true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    pAdjSect1.clear();
    j = 0;
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        bool startSmaller =
          ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
        /*if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
        if (pCurve->AtPoint(p1, startSmaller,m_scalefactor, dpos) &&
          pCurve->AtPoint(p2, startSmaller, m_scalefactor,dpos2))
        {
          pAdjSect2.clear();
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2-dpos ) < fabs ( end - start ) ) return true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    return false;
  }
}

/*

Returns the route curve for the given route id.

*/

SimpleLine Network::GetRouteCurve ( const int in_iRouteId )const
{
  Tuple *pRoute = GetRoute ( in_iRouteId );
  SimpleLine sl = * ( ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE ) );
  pRoute->DeleteIfAllowed();
  return sl;
}

/*
  GetDual

  Returns the dual value of the given route id.

*/

bool Network::GetDual (const  int in_iRouteId )const
{
  Tuple *pRoute = GetRoute ( in_iRouteId );
  bool dual = ( ( CcBool* ) pRoute->GetAttribute ( ROUTE_DUAL ) )->GetBoolval();
  pRoute->DeleteIfAllowed();
  return dual;
}

/*
Searches the route interval between the two given point values.

*/

RouteInterval* Network::Find ( const Point p1, const Point p2 )const
{
  GPoint *gpp1 = GetNetworkPosOfPoint ( p1 );
  GPoint *gpp2 = GetNetworkPosOfPoint ( p2 );
  assert ( gpp1->IsDefined() && gpp2->IsDefined() );
  int rid, ridt;
  double start, end, dpos, dpos2;
  if ( gpp1->GetRouteId() == gpp2->GetRouteId() )
  {
    rid = gpp1->GetRouteId();
    start = gpp1->GetPosition();
    end = gpp2->GetPosition();
    chkStartEndA ( start,end );
    Tuple *pRoute = GetRoute ( rid );
    if ( ShorterConnection ( pRoute, start, end, dpos, dpos2, rid, ridt,
                             p1, p2) )
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( ridt, dpos, dpos2 );
    }
    else
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( rid, start, end );
    }
  }
  else   // different RouteIds
  {
    Tuple *pRoute = GetRoute ( gpp1->GetRouteId() );
    SimpleLine *pCurve =
        ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    bool startSmaller =
      ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    //if ( chkPoint ( pCurve, p2, true, dpos, difference ) )
    if (pCurve->AtPoint(p2, startSmaller, m_scalefactor,dpos))
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = gpp1->GetPosition();
      end = dpos;
      chkStartEndA ( start, end );
      if ( ShorterConnection ( pRoute, start, end, dpos, dpos2, rid, ridt, p1,
                               p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    pRoute = GetRoute ( gpp2->GetRouteId() );
    pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    startSmaller =
      ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    //if ( chkPoint ( pCurve, p1, true, dpos, difference ) )
      if (pCurve->AtPoint(p1, startSmaller, m_scalefactor,dpos))
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = gpp2->GetPosition();
      end = dpos;
      chkStartEndA ( start, end );
      if ( ShorterConnection ( pRoute, start, end, dpos, dpos2, rid, ridt, p1,
                               p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    TupleId pSection = GetTupleIdSectionOnRoute ( gpp1 );
    vector<DirectedSection> pAdjSect1;
    vector<DirectedSection> pAdjSect2;
    pAdjSect1.clear();
    pAdjSect2.clear();
    GetAdjacentSections ( pSection,true, pAdjSect1 );
    GetAdjacentSections ( pSection,false, pAdjSect2 );
    size_t j = 0;
    Tuple *pCurrSect;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                          false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      startSmaller =
        ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
      /*if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
      if (pCurve->AtPoint(p1, startSmaller,m_scalefactor, dpos) &&
        pCurve->AtPoint(p2, startSmaller,m_scalefactor, dpos2))
      {
        start = dpos;
        end = dpos2;
        chkStartEndA ( start, end );
        pAdjSect1.clear();
        pAdjSect2.clear();
        if ( ShorterConnection ( pRoute, start, end, dpos, dpos2,
                                 rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }
    j = 0;
    pAdjSect1.clear();
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                          false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      startSmaller =
            ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
      /*if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
      if (pCurve->AtPoint(p1, startSmaller,m_scalefactor, dpos) &&
        pCurve->AtPoint(p2, startSmaller,m_scalefactor, dpos2))
      {
        start = dpos;
        end = dpos2;
        chkStartEndA ( start, end );
        pAdjSect2.clear();
        if ( ShorterConnection ( pRoute, start, end, dpos, dpos2,
                                 rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }//should never be reached
    pAdjSect2.clear();
  }//should never be reached
  gpp1->DeleteIfAllowed();
  gpp2->DeleteIfAllowed();
  return 0;
}


/*
Returns the route interval for the connection from p1 to p2

*/

RouteInterval* Network::FindInterval ( const Point p1, const Point p2 )const
{
  GPoint *gpp1 = GetNetworkPosOfPoint ( p1 );
  GPoint *gpp2 = GetNetworkPosOfPoint ( p2 );
  assert ( gpp1->IsDefined() && gpp2->IsDefined() );
  int rid, ridt;
  double start, end, dpos, dpos2;
  if ( gpp1->GetRouteId() == gpp2->GetRouteId() )
  {
    rid = gpp1->GetRouteId();
    start = gpp1->GetPosition();
    end = gpp2->GetPosition();
    Tuple *pRoute = GetRoute ( rid );
    if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2, rid, ridt,
         p1, p2) )
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( ridt, dpos, dpos2 );
    }
    else
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( rid, start, end );
    }
  }
  else
  {
    Tuple *pRoute = GetRoute ( gpp1->GetRouteId() );
    SimpleLine *pCurve =
        ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    bool startSmaller =
      ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    //if ( chkPoint ( pCurve, p2, true, dpos, difference ) )
      if (pCurve->AtPoint(p2, startSmaller,m_scalefactor, dpos))
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = gpp1->GetPosition();
      end = dpos;
      if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                rid, ridt, p1, p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    pRoute = GetRoute ( gpp2->GetRouteId() );
    pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    startSmaller =
        ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    //if ( chkPoint ( pCurve, p1, true, dpos, difference ) )
        if (pCurve->AtPoint(p1, startSmaller,m_scalefactor, dpos))
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = dpos;
      end = gpp2->GetPosition();
      if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                rid, ridt, p1, p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    TupleId pSection = GetTupleIdSectionOnRoute ( gpp1 );
    vector<DirectedSection> pAdjSect1;
    vector<DirectedSection> pAdjSect2;
    pAdjSect1.clear();
    pAdjSect2.clear();
    GetAdjacentSections ( pSection,true, pAdjSect1 );
    GetAdjacentSections ( pSection,false, pAdjSect2 );
    size_t j = 0;
    Tuple *pCurrSect;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(), false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      startSmaller =
        ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
      /*if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
      if (pCurve->AtPoint(p1, startSmaller,m_scalefactor, dpos) &&
        pCurve->AtPoint(p2, startSmaller,m_scalefactor, dpos2))
      {
        start = dpos;
        end = dpos2;
        pAdjSect1.clear();
        pAdjSect2.clear();
        if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                  rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }
    j = 0;
    pAdjSect1.clear();
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(), false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      startSmaller =
        ((CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
      /*if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )*/
      if (pCurve->AtPoint(p1, startSmaller, m_scalefactor,dpos) &&
        pCurve->AtPoint(p2, startSmaller, m_scalefactor,dpos2))
      {
        start = dpos;
        end = dpos2;
        pAdjSect2.clear();
        if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                  rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }//should never be reached
    pAdjSect2.clear();
  }//should never be reached
  gpp1->DeleteIfAllowed();
  gpp2->DeleteIfAllowed();
  return 0;
}


void Network::GetTupleIdSectionOnRouteJun(const GPoint* in_xGPoint,
                                          vector<TupleId>& res)const
{

  CcInt *ciRouteId = new CcInt ( true, in_xGPoint->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  delete ciRouteId;
  Tuple *actSect = 0;
  TupleId result;
  while ( pSectionIter->Next() )
  {
    result = pSectionIter->GetId();
    actSect = m_pSections->GetTuple ( pSectionIter->GetId(), false );
    if ( actSect != 0 )
    {
      double start =
          ((CcReal*)actSect->GetAttribute(SECTION_MEAS1))->GetRealval();
      double end =
          ((CcReal*)actSect->GetAttribute(SECTION_MEAS2))->GetRealval();
      if(in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end)
      {
//        delete pSectionIter;
//        actSect->DeleteIfAllowed();
//        return result;
        res.push_back(result);
      }
      else
      {
        if ( AlmostEqualAbsolute ( in_xGPoint->GetPosition(), start,
                                    m_scalefactor*0.01 ))
        {
//          delete pSectionIter;
//          actSect->DeleteIfAllowed();
//          return result;
          res.push_back(result);
        }
        else
        {
          if ( AlmostEqualAbsolute ( in_xGPoint->GetPosition(), end,
                                      m_scalefactor*0.01))
          {
           Tuple *pRoute = GetRoute(((TupleIdentifier* )
                          actSect->GetAttribute ( SECTION_RRC ))->GetTid() );
            if ( AlmostEqualAbsolute(
                ((CcReal*)pRoute->GetAttribute ( ROUTE_LENGTH ) )->GetRealval(),
                                     end, m_scalefactor*0.01 ))
            {
 //             pRoute->DeleteIfAllowed();
//              delete pSectionIter;
//              actSect->DeleteIfAllowed();
                res.push_back(result);
//              return result;
            }
            pRoute->DeleteIfAllowed();
          }
        }
      }
//      actSect->DeleteIfAllowed();
    }
    actSect->DeleteIfAllowed();
  }
  delete pSectionIter;

}


/*
~Out~-function of type constructor ~network~

*/
ListExpr Network::Out ( ListExpr typeInfo )
{
//cout << "NetworkOut" << endl;
  ///////////////////////
  // Output of all routes
  GenericRelationIterator *pRoutesIter = m_pRoutes->MakeScan();
  Tuple *pCurrentRoute = 0;
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();
  ListExpr xRoutes = nl->TheEmptyList();
  bool bFirst = true;

  while ( ( pCurrentRoute = pRoutesIter->GetNextTuple() ) != 0 )
  {
    // Read values from table
    CcInt* pRouteId = ( CcInt* ) pCurrentRoute->GetAttribute ( ROUTE_ID );
    int iRouteId = pRouteId->GetIntval();
    CcReal* pLength = ( CcReal* ) pCurrentRoute->GetAttribute ( ROUTE_LENGTH );
    double dLength  = pLength->GetRealval();
    SimpleLine *pCurve = ( SimpleLine* ) pCurrentRoute->GetAttribute (
ROUTE_CURVE );
    // The list for the curve contains all segments of the curve.
    ListExpr xCurve = OutSimpleLine ( nl->TheEmptyList(), SetWord ( pCurve ) );
    CcBool* pDual = ( CcBool* ) pCurrentRoute->GetAttribute ( ROUTE_DUAL );
    bool bDual= pDual->GetBoolval();
    CcBool* pStartsSmaller;
    pStartsSmaller = ( CcBool* ) pCurrentRoute->GetAttribute (
ROUTE_STARTSSMALLER );
    bool bStartsSmaller = pStartsSmaller->GetBoolval();

    // Build list
    xNext = nl->FiveElemList ( nl->IntAtom ( iRouteId ),
                               nl->RealAtom ( dLength ),
                               xCurve,
                               nl->BoolAtom ( bDual ),
                               nl->BoolAtom ( bStartsSmaller ) );

    // Create new list or append element to existing list
    if ( bFirst )
    {
      xRoutes = nl->OneElemList ( xNext );
      xLast = xRoutes;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append ( xLast, xNext );
    }
    pCurrentRoute->DeleteIfAllowed();
  }
  delete pRoutesIter;

  ///////////////////////
  // Output of all junctions
  GenericRelationIterator *pJunctionsIter = m_pJunctions->MakeScan();
  Tuple *pCurrentJunction;
  ListExpr xJunctions = nl->TheEmptyList();
  bFirst = true;

  while ( ( pCurrentJunction = pJunctionsIter->GetNextTuple() ) != 0 )
  {
    // Read values from table
    CcInt* pRoute1Id;
    pRoute1Id = ( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE1_ID
);
    int iRoute1Id = pRoute1Id->GetIntval();
    CcReal* pMeas1;
    pMeas1 = ( CcReal* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE1_MEAS
);
    double dMeas1 = pMeas1->GetRealval();
    CcInt* pRoute2Id;
    pRoute2Id = ( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE2_ID
);
    int iRoute2Id = pRoute2Id->GetIntval();
    CcReal* pMeas2;
    pMeas2 = ( CcReal* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE2_MEAS
);
    double dMeas2 = pMeas2->GetRealval();
    CcInt* pConnectivityCode;
    pConnectivityCode = ( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_CC
);
    int iConnectivityCode= pConnectivityCode->GetIntval();
    Point* pPoint = ( Point* ) pCurrentJunction->GetAttribute ( JUNCTION_POS );
    ListExpr xPoint = OutPoint ( nl->TheEmptyList(), SetWord ( pPoint ) );

    // Build list
    xNext = nl->SixElemList ( nl->IntAtom ( iRoute1Id ),
                              nl->RealAtom ( dMeas1 ),
                              nl->IntAtom ( iRoute2Id ),
                              nl->RealAtom ( dMeas2 ),
                              nl->IntAtom ( iConnectivityCode ),
                              xPoint );

    // Create new list or append element to existing list
    if ( bFirst )
    {
      xJunctions= nl->OneElemList ( xNext );
      xLast = xJunctions;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append ( xLast, xNext );
    }
    pCurrentJunction->DeleteIfAllowed();
  }

  delete pJunctionsIter;

  return nl->FourElemList ( nl->IntAtom ( m_iId ),
                             nl->RealAtom(m_scalefactor),
                             xRoutes,
                             xJunctions );
}



/*
~Save~-function of type constructor ~network~

*/
ListExpr Network::Save ( SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset,
                         const ListExpr in_xTypeInfo )
{
  // Save id of the network
  int iId = m_iId;
  in_xValueRecord.Write ( &iId,
                          sizeof ( int ),
                          inout_iOffset );
  inout_iOffset += sizeof ( int );

  // Save scalefactor of the network
  double iScale = m_scalefactor;
  in_xValueRecord.Write ( &iScale,
                          sizeof ( double ),
                          inout_iOffset );
  inout_iOffset += sizeof ( double );
  // Save routes
  ListExpr xType;
  nl->ReadFromString ( routesTypeInfo, xType );
  ListExpr xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pRoutes->Save ( in_xValueRecord,
                          inout_iOffset,
                          xNumericType ) )
  {
    return false;
  }
  // Save junctions
  nl->ReadFromString ( junctionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pJunctions->Save ( in_xValueRecord,
                             inout_iOffset,
                             xNumericType ) )
  {
    return false;
  }
  // Save sections
  nl->ReadFromString ( sectionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pSections->Save ( in_xValueRecord,
                            inout_iOffset,
                            xNumericType ) )
  {
    return false;
  }
  // Save btree for routes
  nl->ReadFromString ( routesBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeRoutes->Save ( in_xValueRecord,
                               inout_iOffset,
                               xNumericType ) )
  {
    return false;
  }
  // Save rtree for routes

  if ( !m_pRTreeRoutes->Save ( in_xValueRecord,
                               inout_iOffset ) )
  {
    return false;
  }
  // Save first btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeJunctionsByRoute1->Save ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType ) )
  {
    return false;
  }
  // Save second btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeJunctionsByRoute2->Save ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType ) )
  {
    return false;
  }

   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();
   m_xAdjacencyList.saveToFile(rf, m_xAdjacencyList);
   SmiSize offset = 0;
   size_t bufsize = m_xAdjacencyList.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   m_xAdjacencyList.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);
    //save m_xSubAdjacencyList
   //Flob *tmpSubAdjList = &m_xSubAdjacencyList;
   // SmiRecordFile *rf1 = ctlg->GetFlobFile();
   m_xSubAdjacencyList.saveToFile(rf, m_xSubAdjacencyList);
   offset = 0;
   buf = (char*) malloc(bufsize);
   m_xSubAdjacencyList.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;
   //save m_reverseAdjacencyList
   m_reverseAdjacencyList.saveToFile(rf, m_reverseAdjacencyList);
   offset = 0;
   bufsize = m_reverseAdjacencyList.headerSize()+ 2*sizeof(int);
   buf = (char*) malloc(bufsize);
   m_reverseAdjacencyList.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);
   //save m_reverseSubAdjacencyList
   m_reverseSubAdjancencyList.saveToFile(rf, m_reverseSubAdjancencyList);
   offset = 0;
   buf = (char*) malloc(bufsize);
   m_reverseSubAdjancencyList.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

  // Save btree for sections
  nl->ReadFromString ( sectionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeSectionsByRoute->Save ( in_xValueRecord,
                                        inout_iOffset,
                                        xNumericType ) )
  {

    //cout << "cannot store BTree_sections" << endl;

    return false;
  }

  //save distance storage
  /*
  nl->ReadFromString(distancestorageTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!alldistance->Save(in_xValueRecord,
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
  */

  return true;
}


/*
~Open~-function of type constructor ~network~

*/
Network *Network::Open ( SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset,
                         const ListExpr in_xTypeInfo )
{
//cout << "NetworkOpen" << endl;
  // Create network
  return new Network ( in_xValueRecord,
                       inout_iOffset,
                       in_xTypeInfo );
}


ListExpr Network::OutNetwork ( ListExpr typeInfo, Word value )
{
  Network *n = ( Network* ) value.addr;
  return n->Out ( typeInfo );
}

Word Network::InNetwork ( ListExpr in_xTypeInfo,
                          ListExpr in_xValue,
                          int in_iErrorPos,
                          ListExpr& inout_xErrorInfo,
                          bool& inout_bCorrect )
{
//cout << "inNetwork" << endl;
  Network* pNetwork = new Network ( in_xValue,
                                    in_iErrorPos,
                                    inout_xErrorInfo,
                                    inout_bCorrect );

  if ( inout_bCorrect )
  {
    return SetWord ( pNetwork );
  }
  else
  {
    delete pNetwork;
    return SetWord ( Address ( 0 ) );
  }
}

Word Network::CreateNetwork ( const ListExpr typeInfo )
{
  return SetWord ( new Network() );
}

void Network::CloseNetwork ( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Network*> ( w.addr );
  w.addr = 0;

}

/*
~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word Network::CloneNetwork ( const ListExpr typeInfo, const Word& w )
{
  return SetWord ( Address ( 0 ) );
}

void Network::DeleteNetwork ( const ListExpr typeInfo, Word& w )
{
  Network* n = ( Network* ) w.addr;
  //n->Destroy();
  delete n;
  w.addr = 0;
}

bool Network::CheckNetwork ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, "network" ) );
}

void* Network::CastNetwork ( void* addr )
{
  return new (addr)Network;
}

bool Network::SaveNetwork ( SmiRecord& valueRecord,
                            size_t& offset,
                            const ListExpr typeInfo,
                            Word& value )
{
// cout << "Save Network" << endl;
  Network *n = ( Network* ) value.addr;
  return n->Save ( valueRecord, offset, typeInfo );
}

bool Network::OpenNetwork ( SmiRecord& valueRecord,
                            size_t& offset,
                            const ListExpr typeInfo,
                            Word& value )
{
  value.addr = Network::Open ( valueRecord, offset, typeInfo );
  return value.addr != 0;
}

int Network::SizeOfNetwork()
{
  return 0;
}

int Network::IsDefined() const
{
  return m_bDefined;
}

GPoint* Network::GetNetworkPosOfPoint(const Point p) const
{
  const Rectangle<2> orig = p.BoundingBox();
  const Rectangle<2> bbox = Rectangle<2> ( true,
                            orig.MinD ( 0 ) - 1.0*m_scalefactor,
                            orig.MaxD ( 0 ) + 1.0*m_scalefactor,
                            orig.MinD ( 1 ) - 1.0*m_scalefactor,
                            orig.MaxD ( 1 ) + 1.0*m_scalefactor);
  R_TreeLeafEntry<2,TupleId> res;
  Tuple *pCurrRoute = 0;
  if ( m_pRTreeRoutes->First ( bbox, res ) )
  {
    pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
    // pCurrRoute->PutAttribute(0, new TupleIdentifier(true, res.info));
  }
  else
  {
    GPoint *result = new GPoint ( false );
    return result;
  }
  double dpos, mindiff, difference;
  mindiff = numeric_limits<double>::max();
  int minrid;
  SimpleLine* pRouteCurve =
    ( SimpleLine* ) pCurrRoute->GetAttribute (ROUTE_CURVE );
  bool startSmaller =
    ((CcBool*)pCurrRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
  int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
  /*if ( chkPoint ( pRouteCurve, p, true, dpos, difference ) )*/
  if (pRouteCurve->AtPoint(p ,startSmaller,m_scalefactor*0.01, dpos))
  {
    GPoint *result = new GPoint ( true, GetId(), rid, dpos, None );
    pCurrRoute->DeleteIfAllowed();
    return result;
  }
  else
  {
    minrid = rid;
    mindiff = pRouteCurve->Distance(p,0);
    pCurrRoute->DeleteIfAllowed();
    pCurrRoute = 0;
    while ( m_pRTreeRoutes->Next ( res ) )
    {
      pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
      pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
      startSmaller =
        ((CcBool*)pCurrRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
      rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      //if ( chkPoint ( pRouteCurve, p, true, dpos, difference ) )
      if (pRouteCurve->AtPoint(p,startSmaller,m_scalefactor*0.01, dpos))
      {
        GPoint *result = new GPoint ( true, GetId(),
                                      rid,
                                      dpos, None );
        pCurrRoute->DeleteIfAllowed();
        return result;
      }
      difference = pRouteCurve->Distance(p,0);
      if (difference < mindiff)
      {
        mindiff = difference;
        minrid = rid;
      }
      pCurrRoute->DeleteIfAllowed();
      pCurrRoute = 0;
    }
/*
If no exact match route has been found map to the route nearest to the point.
At the place nearest to the point.

*/
    pCurrRoute = GetRoute(minrid);
    pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
    startSmaller =
        ((CcBool*)pCurrRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    if (lastchkPoint03(pRouteCurve, p ,startSmaller, m_scalefactor*0.01,
                           dpos, difference))
    {
      GPoint *result = new GPoint ( true, GetId(),
                                    minrid,
                                    dpos, None );
      pCurrRoute->DeleteIfAllowed();
      return result;
    }
    GPoint *result = new GPoint ( false );
    if (pCurrRoute != 0)
    {
      pCurrRoute->DeleteIfAllowed();
      pCurrRoute = 0;
    }
    return result;
  }
}

GPoint* Network::GetNetworkPosOfPointOnRoute(const Point p, const int rid) const
{
  Tuple *pCurrRoute = GetRoute(rid);
  if (pCurrRoute != 0)
  {
    double dpos;
    SimpleLine* pRouteCurve =
        ( SimpleLine* ) pCurrRoute->GetAttribute (ROUTE_CURVE );
    bool startSmaller =
        ((CcBool*)pCurrRoute->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
    if (pRouteCurve->AtPoint(p, startSmaller,m_scalefactor*0.01, dpos))
    {
      pCurrRoute->DeleteIfAllowed();
      return new GPoint ( true, GetId(), rid, dpos, None );
    }
  }
  if (pCurrRoute != 0) pCurrRoute->DeleteIfAllowed();
  return new GPoint(false);
}

/*
~GetJunctionsMeasForRoutes~

Returns the position of a junction on both routes building the junction.

*/

void Network::GetJunctionMeasForRoutes ( CcInt *pRoute1Id, CcInt *pRoute2Id,
        double &rid1meas, double &rid2meas )const
{
  CcInt *pCurrJuncR2id, *pCurrJuncR1id;
  int iCurrJuncTupleR2id, iCurrJuncR1id, iRoute1Id, iRoute2Id;
  CcReal *pRid1Meas, *pRid2Meas;
  bool r1smallerr2, found;
  BTreeIterator *pJunctionsIt;
  if ( pRoute1Id->GetIntval() <= pRoute2Id->GetIntval() )
  {
    pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch ( pRoute1Id );
    iRoute1Id = pRoute1Id->GetIntval();
    iRoute2Id = pRoute2Id->GetIntval();
    r1smallerr2 = true;
  }
  else
  {
    pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch ( pRoute2Id );
    iRoute1Id = pRoute2Id->GetIntval();
    iRoute2Id = pRoute1Id->GetIntval();
    r1smallerr2 = false;
  }
  found = false;
  while ( !found && pJunctionsIt->Next() )
  {
    Tuple *pCurrJuncTuple = m_pJunctions->GetTuple ( pJunctionsIt->GetId(),
                                                     false );
    pCurrJuncR2id = ( CcInt* ) pCurrJuncTuple->GetAttribute ( JUNCTION_ROUTE2_ID
);
    iCurrJuncTupleR2id = pCurrJuncR2id->GetIntval();
    pCurrJuncR1id = ( CcInt* ) pCurrJuncTuple->GetAttribute ( JUNCTION_ROUTE1_ID
);
    iCurrJuncR1id = pCurrJuncR1id->GetIntval();
    if ( iCurrJuncTupleR2id == iRoute2Id && iCurrJuncR1id == iRoute1Id )
    {
      found = true;
      if ( r1smallerr2 )
      {
        pRid1Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE1_MEAS );
        rid1meas = pRid1Meas->GetRealval();
        pRid2Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE2_MEAS );
        rid2meas = pRid2Meas->GetRealval();
      }
      else
      {
        pRid1Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE2_MEAS );
        rid1meas = pRid1Meas->GetRealval();
        pRid2Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE1_MEAS );
        rid2meas = pRid2Meas->GetRealval();
      }
    }
    pCurrJuncTuple->DeleteIfAllowed();
  }
  delete pJunctionsIt;
  if ( !found )
  {
    rid1meas = numeric_limits<double>::max();
    rid2meas = numeric_limits<double>::max();
  }
}

/*
Return sLine Value from RouteId

*/

void Network::GetLineValueOfRouteInterval ( const RouteInterval *in_ri,
        SimpleLine *out_Line )const
{
  CcInt* pRouteId = new CcInt ( true, in_ri->GetRouteId() );
  BTreeIterator *pRoutesIter = m_pBTreeRoutes->ExactMatch ( pRouteId );
  pRouteId->DeleteIfAllowed();
  Tuple *pRoute = 0;
  if ( pRoutesIter->Next() )
    pRoute = m_pRoutes->GetTuple (pRoutesIter->GetId(), false);
  assert ( pRoute != 0 );
  SimpleLine* pLine = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
  assert ( pLine != 0 );
  bool startSmaller =(( CcBool*)pRoute->GetAttribute(ROUTE_STARTSSMALLER))
                  ->GetBoolval();
  pLine->SubLine ( min ( in_ri->GetStartPos(), in_ri->GetEndPos() ),
                   max ( in_ri->GetStartPos(), in_ri->GetEndPos() ),
                   startSmaller, *out_Line );
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
}

/*
Secondo TypeConstructor for class ~Network~

*/

struct networkInfo:ConstructorInfo{
  networkInfo():ConstructorInfo(){
    name = Network::BasicType();
    signature = "-> NETWORK";
    typeExample = Network::BasicType();
    listRep = "(<id> <routes-relation><junctions-relation>)";
    valueExample = "(1 (rel()) (rel()))";
    remarks = "Datatype containing all network information.";
  }
};

struct networkFunctions:ConstructorFunctions<Network>{
  networkFunctions(){
    in = Network::InNetwork;
    out = Network::OutNetwork;
    create = Network::CreateNetwork;
    deletion = Network::DeleteNetwork;
    open = Network::OpenNetwork;
    save = Network::SaveNetwork;
    close = Network::CloseNetwork;
    clone = Network::CloneNetwork;
    cast = Network::CastNetwork;
    sizeOf = Network::SizeOfNetwork;
    kindCheck = Network::CheckNetwork;
  }
};

networkInfo neti;
networkFunctions nf;
TypeConstructor networkTC(neti,nf);

/*
3 class ~GLine~

3.1 Constructors

The simple constructor. Should not be used.

*/
GLine::GLine():Attribute()
{}

GLine::GLine(const bool def) : Attribute(def), m_xRouteIntervals(0)
{
  SetDefined(def);
  m_bSorted = false;
  m_dLength = 0.0;
}

GLine::GLine (const int in_iSize ) :
    Attribute(true),
    m_xRouteIntervals ( in_iSize )
{
  SetDefined(true);
  m_bSorted = false;
  m_dLength = 0.0;
}

GLine::GLine(const int networkId,
             const bool sort,
             const DbArray<RouteInterval> *rilist):
  Attribute(sort), m_xRouteIntervals(rilist->Size())
{
  m_iNetworkId = networkId;
  m_bSorted = sort;
  m_xRouteIntervals.copyFrom(*rilist);
  m_dLength = 0.0;
  RouteInterval ri;
  for (int i = 0; i < rilist->Size(); i++)
  {
    rilist->Get(i,ri);
    m_dLength = m_dLength + ri.Length();
  }
}

GLine::GLine ( const GLine* in_xOther ) :
    Attribute(in_xOther->IsDefined()),
    m_xRouteIntervals ( 0 )
{
  SetDefined(in_xOther->IsDefined());
  m_bSorted = in_xOther->m_bSorted;
  m_iNetworkId = in_xOther->m_iNetworkId;
  m_dLength = 0.0;
  // Iterate over all RouteIntervalls
  for ( int i = 0; i < in_xOther->m_xRouteIntervals.Size(); i++ )
  {
    // Get next Interval
    RouteInterval pCurrentInterval;
    in_xOther->m_xRouteIntervals.Get ( i, pCurrentInterval );

    int iRouteId = pCurrentInterval.GetRouteId();
    double dStart = pCurrentInterval.GetStartPos();
    double dEnd = pCurrentInterval.GetEndPos();
    AddRouteInterval ( iRouteId,
                       dStart,
                       dEnd );
  }
  TrimToSize();
}

GLine::GLine ( ListExpr in_xValue,
               int in_iErrorPos,
               ListExpr& inout_xErrorInfo,
               bool& inout_bCorrect ):
    Attribute(true)
{
  // Check the list
  if ( ! ( nl->ListLength ( in_xValue ) == 2 ) )
  {
    string strErrorMessage = "GLine(): List length must be 2.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    SetDefined(false);
    m_bSorted = false;
    return;
  }

  // Split into the two parts
  ListExpr xNetworkIdList = nl->First ( in_xValue );
  ListExpr xRouteIntervalList = nl->Second ( in_xValue );

  // Check the parts
  if ( !nl->IsAtom ( xNetworkIdList ) ||
          nl->AtomType ( xNetworkIdList ) != IntType )
  {
    string strErrorMessage = "GLine(): Error while reading network-id.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    SetDefined(false);
    m_bSorted = false;
    inout_bCorrect = false;
    return;
  }

  m_iNetworkId = nl->IntValue ( xNetworkIdList );
  m_dLength = 0.0;
  if ( !nl->IsEmpty ( xRouteIntervalList ) )
  {
    // Iterate over all routes

    while ( !nl->IsEmpty ( xRouteIntervalList ) )
    {
      ListExpr xCurrentRouteInterval = nl->First ( xRouteIntervalList );
      xRouteIntervalList = nl->Rest ( xRouteIntervalList );

      if ( nl->ListLength ( xCurrentRouteInterval ) != 3 ||
              ( !nl->IsAtom ( nl->First ( xCurrentRouteInterval ) ) ) ||
              nl->AtomType ( nl->First ( xCurrentRouteInterval ) ) != IntType ||
              ( !nl->IsAtom ( nl->Second ( xCurrentRouteInterval ) ) ) ||
              nl->AtomType ( nl->Second ( xCurrentRouteInterval ) ) != RealType
||
              ( !nl->IsAtom ( nl->Third ( xCurrentRouteInterval ) ) ) ||
              nl->AtomType ( nl->Third ( xCurrentRouteInterval ) ) != RealType )
      {
        string strErrorMessage = "GLine(): Error while reading route-interval.";
        inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                        nl->StringAtom ( strErrorMessage ) );
        inout_bCorrect = false;
        SetDefined(false);
        m_bSorted = false;
        return;
      }

      // Read attributes from list
      // Read values from table
      int iRouteId = nl->IntValue ( nl->First ( xCurrentRouteInterval ) );
      double dStart = nl->RealValue ( nl->Second ( xCurrentRouteInterval ) );
      double dEnd  = nl->RealValue ( nl->Third ( xCurrentRouteInterval ) );

      AddRouteInterval ( iRouteId,
                         dStart,
                         dEnd );

    }
    inout_bCorrect = true;
    SetDefined(true);
    m_bSorted = false;
  }
  else
  {
    SetDefined(false);
    m_bSorted = false;
    inout_bCorrect = true;
  }
  TrimToSize();
  return;
}

/*
3.2 Methods of class ~GLine~

*/
void GLine::SetNetworkId (const  int in_iNetworkId )
{
  m_iNetworkId = in_iNetworkId;
  SetDefined(true);
}

void GLine::AddRouteInterval ( const RouteInterval ri )
{
  m_xRouteIntervals.Append ( ri );
  m_dLength = m_dLength + fabs ( ri.GetEndPos() - ri.GetStartPos() );

}

void GLine::AddRouteInterval ( const int in_iRouteId,
                               const double in_dStart,
                               const double in_dEnd )
{
  RouteInterval *ri = new RouteInterval ( in_iRouteId,
                                          in_dStart,
                                          in_dEnd );
  AddRouteInterval ( *ri );
  delete ri;
}


bool GLine::IsSorted() const
{
  return m_bSorted;
}

void GLine::SetSorted (const bool in_bSorted )
{
  m_bSorted = in_bSorted;
}

/*
Secondo Integration

*/
Word GLine::In ( const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct )
{
  GLine* pGline = new GLine ( 0 );
  if ( nl->ListLength ( instance ) == 0 )
  {
    correct = true;
    pGline->SetDefined ( false );
    return SetWord ( pGline );
  }
  if (nl->ListLength(instance) == 1 &&
    listutils::isSymbol(instance,"undef"))
  {
    correct = true;
    pGline->SetDefined ( false );
    return SetWord ( pGline );
  }
  if ( nl->ListLength ( instance ) != 2 )
  {
    correct = false;
    pGline->DeleteIfAllowed();
    cmsg.inFunError ( "Expecting (networkid (list of routeintervals))" );
    return SetWord ( Address ( 0 ) );
  }
  ListExpr FirstElem = nl->First ( instance );
  ListExpr SecondElem = nl->Second ( instance );
  if ( !nl->IsAtom ( FirstElem ) || !nl->AtomType ( FirstElem ) == IntType )
  {
    correct = false;
    pGline->DeleteIfAllowed();
    cmsg.inFunError ( "Networkadress is not evaluable" );
    return SetWord ( Address ( 0 ) );
  }
  pGline->SetNetworkId ( nl->IntValue ( FirstElem ) );
  if ( nl->IsEmpty ( SecondElem ) )
  {
    correct = false;
    pGline->DeleteIfAllowed();
    return SetWord ( Address ( 0 ) );
  }
  while ( !nl->IsEmpty ( SecondElem ) )
  {
    ListExpr start = nl->First ( SecondElem );
    SecondElem = nl->Rest ( SecondElem );
    if ( nl->ListLength ( start ) != 3 )
    {
      correct = false;
      pGline->DeleteIfAllowed();
      cmsg.inFunError ( "Routeinterval incorrect.Expected list of 3 Elements."
);
      return SetWord ( Address ( 0 ) );
    }
    ListExpr lrid = nl->First ( start );
    ListExpr lpos1 = nl->Second ( start );
    ListExpr lpos2 = nl->Third ( start );
    if ( !nl->IsAtom ( lrid ) || !nl->AtomType ( lrid ) == IntType ||
            !nl->IsAtom ( lpos1 ) || !nl->AtomType ( lpos1 ) == RealType ||
            !nl->IsAtom ( lpos2 ) || !nl->AtomType ( lpos2 ) == RealType )
    {
      correct = false;
      pGline->DeleteIfAllowed();
      cmsg.inFunError ( "Routeinterval should be list int, real, real." );
      return SetWord ( Address ( 0 ) );
    }
    pGline->AddRouteInterval ( nl->IntValue ( lrid ),
                               nl->RealValue ( lpos1 ),
                               nl->RealValue ( lpos2 ) );
  }
  correct = true;
  pGline->SetDefined(true);
  pGline->TrimToSize();
  return SetWord ( pGline );
}

ListExpr GLine::Out ( ListExpr in_xTypeInfo,
                      Word in_xValue )
{
  GLine *pGline = ( GLine* ) in_xValue.addr;
  if ( pGline == 0 || !pGline->IsDefined() )
  {
    return nl->SymbolAtom ( "undef" );
  }

  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();
  bool bFirst = true;
  ListExpr xNetworkId = nl->IntAtom ( pGline->m_iNetworkId );
  ListExpr xRouteIntervals = nl->TheEmptyList();
  if (pGline->m_xRouteIntervals.Size() > 0)
  {

  // Iterate over all RouteIntervalls
    for ( int i = 0; i < pGline->m_xRouteIntervals.Size(); i++ )
    {
      // Get next Interval
      RouteInterval pCurrentInterval;
      pGline->m_xRouteIntervals.Get ( i, pCurrentInterval );

      // Build list
      xNext = nl->ThreeElemList ( nl->IntAtom (pCurrentInterval.GetRouteId()),
                                  nl->RealAtom (pCurrentInterval.GetStartPos()),
                                  nl->RealAtom (pCurrentInterval.GetEndPos()));

      // Create new list or append element to existing list
      if ( bFirst )
      {
        xRouteIntervals = nl->OneElemList ( xNext );
        xLast = xRouteIntervals;
        bFirst = false;
      }
      else
      {
        xLast = nl->Append ( xLast, xNext );
      }
    }
  }
  return nl->TwoElemList ( xNetworkId,
                           xRouteIntervals );
}

Word GLine::Create ( const ListExpr typeInfo )
{
  return SetWord ( new GLine ( true ) );
}

void GLine::Clear()
{
  m_xRouteIntervals.clean();
  SetSorted ( false );
  m_dLength = 0.0;
}

void GLine::Delete ( const ListExpr typeInfo,
                     Word& w )
{
  GLine *l = ( GLine* ) w.addr;
  //if (l->del.refs == 1) l->m_xRouteIntervals.Destroy();
  l->DeleteIfAllowed();
  w.addr = 0;
}

void GLine::Close ( const ListExpr typeInfo,
                    Word& w )
{
  ( ( GLine* ) w.addr )->DeleteIfAllowed();
  w.addr = 0;
}

Word GLine::CloneGLine ( const ListExpr typeInfo,
                         const Word& w )
{
  return SetWord ( ( ( GLine* ) w.addr )->Clone() );
}

void GLine::SetLength(const double l)
{
  m_dLength = l;
}

GLine* GLine::Clone() const
{
  GLine *xOther = new GLine ( Size() );
  xOther->SetDefined ( IsDefined() );
  xOther->SetSorted ( m_bSorted );
  xOther->SetNetworkId ( m_iNetworkId );
  xOther->SetLength(m_dLength);
  /*RouteInterval ri;
  for ( int i = 0; i < Size(); i++ )
  {
    Get ( i, ri );
    int rid = ri.GetRouteId();
    double start = ri.GetStartPos();
    double end = ri.GetEndPos();
    xOther->AddRouteInterval ( rid, start, end );
  }*/
  xOther->m_xRouteIntervals.copyFrom(m_xRouteIntervals);
  return xOther;
}

void* GLine::Cast ( void* addr )
{
  return new ( addr ) GLine;
}

int GLine::Size() const
{
  return m_xRouteIntervals.Size();
}

int GLine::SizeOf()
{
  return sizeof ( GLine );
}

size_t GLine::Sizeof() const
{
  return sizeof ( *this );
}

ostream& RouteInterval::Print(ostream& os) const
{
    os << "RouteInterval: rid: " << m_iRouteId;
    os << " from: " << m_dStart << " to: " << m_dEnd;
    os << endl;
    return os;
}

ostream& GLine::Print ( ostream& os ) const
{
  if (IsDefined())
  {
    os << "GLine: NetworkId: " << m_iNetworkId << endl;
    for ( int i = 0; i < m_xRouteIntervals.Size() ; i++ )
    {
      RouteInterval ri;
      Get ( i, ri );
      os << i <<". ";
      ri.Print(os);
    }
    os << " end gline";
  }
  else
    os <<"GLine: undef" << endl;
  return os;
};

bool GLine::Adjacent ( const Attribute* arg ) const
{
  return false;
}

/*
Compare

*/
int GLine::Compare ( const Attribute* arg ) const
{
  GLine *gl2 = ( GLine* ) arg;
  if ( IsDefined() && !gl2->IsDefined() ) return 1;
  else
    if ( !IsDefined() && gl2->IsDefined() ) return -1;
    else
      if ( !IsDefined() && !gl2->IsDefined() ) return 0;
      else
        if ( m_dLength < gl2->m_dLength ) return -1;
        else
          if ( m_dLength > gl2->m_dLength ) return 1;
          else
            if ( m_xRouteIntervals.Size() < gl2->m_xRouteIntervals.Size() )
              return -1;
            else
              if ( m_xRouteIntervals.Size() > gl2->m_xRouteIntervals.Size() )
                return 1;
              else
                if ( *this == *gl2 ) return 0;
                else
                {
                  RouteInterval ri1, ri2;
                  int i = 0;
                  while ( i < m_xRouteIntervals.Size() )
                  {
                    Get ( i,ri1 );
                    gl2->Get ( i,ri2 );
                    if ( ri1.GetRouteId() < ri2.GetRouteId() ) return -1;
                    else
                      if ( ri1.GetRouteId() > ri2.GetRouteId() ) return 1;
                      else
                        if ( ri1.GetStartPos() < ri2.GetStartPos() ) return -1;
                        else
                          if ( ri1.GetStartPos() > ri2.GetStartPos() ) return 1;
                          else
                            if ( ri1.GetEndPos() < ri2.GetEndPos() ) return -1;
                            else
                              if ( ri1.GetEndPos() > ri2.GetEndPos() ) return 1;
                    i++;
                  }
                }
  return 0;
}

GLine& GLine::operator= ( const GLine& l )
{
  SetDefined(l.IsDefined());
  if (l.IsDefined())
  {
    m_xRouteIntervals.copyFrom(l.m_xRouteIntervals);
    m_bSorted = l.m_bSorted;
    m_iNetworkId = l.m_iNetworkId;
    m_dLength=l.m_dLength;
  }
  return *this;
}

bool GLine::operator== ( const GLine& l ) const
{
  if ( !IsDefined() || !l.IsDefined())
  {
    return false;
  }
  else
  {
    RouteInterval rIt, rIl;
    if ( m_xRouteIntervals.Size() == l.m_xRouteIntervals.Size() &&
      AlmostEqual ( m_dLength, l.m_dLength ) )
    {
      if ( m_bSorted && l.m_bSorted )
      {
        for ( int i=0; i < m_xRouteIntervals.Size(); i++ )
        {
          Get ( i,rIt );
          l.Get ( i,rIl );
          if ( ! ( rIt.GetRouteId() == rIl.GetRouteId() &&
                   rIt.GetStartPos() == rIl.GetStartPos() &&
                   rIt.GetEndPos() == rIl.GetEndPos() ) ) return false;
        }
        return true;
      }
      else
      {
        for ( int i=0; i < m_xRouteIntervals.Size(); i++ )
        {
          Get ( i,rIt );
          for ( int j = 0; j < m_xRouteIntervals.Size(); j++ )
          {
            l.Get ( i,rIl );
            if ( ! ( rIt.GetRouteId() == rIl.GetRouteId() &&
                     rIt.GetStartPos() == rIl.GetStartPos() &&
                     rIt.GetEndPos() == rIl.GetEndPos() ) ) return false;
          }
        }
        return true;
      }
    }
    else return false;
  }
}

size_t GLine::HashValue() const
{
  size_t xHash = m_iNetworkId;

  // Iterate over all RouteIntervalls
  for ( int i = 0; i < m_xRouteIntervals.Size(); ++i )
  {
    // Get next Interval
    RouteInterval pCurrentInterval;
    m_xRouteIntervals.Get ( i, pCurrentInterval );

    // Add something for each entry
    int iRouteId = pCurrentInterval.GetRouteId();
    double dStart = pCurrentInterval.GetStartPos();
    double dEnd = pCurrentInterval.GetEndPos();
    xHash += iRouteId + ( size_t ) dStart + ( size_t ) dEnd;
  }
  return xHash;
}

int GLine::NumOfFLOBs() const
{
  return 1;
}

Flob* GLine::GetFLOB ( const int i )
{
  if ( i == 0 ) return &m_xRouteIntervals;
  return 0;
}

DbArray<RouteInterval>* GLine::GetRouteIntervals() const
{
  DbArray<RouteInterval> * res = new DbArray<RouteInterval>(0);
  if ( IsDefined() )
  {
    res->copyFrom(m_xRouteIntervals);
    return res;
  }
  else return 0;
};

void GLine::CopyFrom ( const Attribute* right )
{
  *this = * ( ( const GLine * ) right );
}

double GLine::GetLength() const
{
  return m_dLength;
}

int GLine::GetNetworkId() const
{
  return  m_iNetworkId;
};

/*
~Get~ returns the route interval at position i in the route intervals ~DbArray~.

*/

void GLine::Get ( const int i, RouteInterval &ri ) const
{
  m_xRouteIntervals.Get ( i, ri );
};

int GLine::NoOfComponents() const
{
  return m_xRouteIntervals.Size();
};


bool GLine::Check ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, GLine::BasicType() ) );
}


bool GLine::ShortestPathAStar(const GLine *to, GLine *result,
                              DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !to->IsDefined())
  {
    result->SetDefined(false);
    return false;
  }
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  bool bres = ShortestPathAStar(to,result,pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return bres;
}

bool GLine::ShortestPathAStar (const GLine *pgl2, GLine *result,
                               const Network *pNetwork,
                               DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if ( GetNetworkId() != pgl2->GetNetworkId() )
  {
    cmsg.inFunError ( "Both glines must belong to the network." );
    return false;
  }
  if (Intersects(pgl2))
  {
    result->SetDefined(true);
    result->SetNetworkId(pNetwork->GetId());
    return true;
  }
  GPoints *bGPgl1 = new GPoints(true);
  GetBGP(pNetwork, bGPgl1);
  GPoints *bGPgl2 = new GPoints(true);
  pgl2->GetBGP(pNetwork,bGPgl2);
  bool bres = bGPgl1->ShortestPathAStar(bGPgl2,result,pNetwork, touchedSects);
  bGPgl1->DeleteIfAllowed();
  bGPgl2->DeleteIfAllowed();
  return bres;
}

bool GLine::ShortestPathAStar(const GPoint *to, GLine *result,
                         DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !to->IsDefined())
  {
    result->SetDefined(false);
    return false;
  }
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  bool bres = ShortestPathAStar(to, result, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return bres;
}

bool GLine::ShortestPathAStar(const GPoint *to, GLine *result,
                              const Network* pNetwork,
                              DbArray<TupleId>* touchedSects) const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if ( GetNetworkId() != to->GetNetworkId() )
  {
    return false;
  }
  result->SetNetworkId(pNetwork->GetId());
  if (to->Inside(this, pNetwork->GetScalefactor()*0.01))
  {
    result->SetDefined(true);
    return true;
  }
  GPoints *bGPgl1 = new GPoints(true);
  GetBGP(pNetwork, bGPgl1);
  GPoints *bGPgl2 = new GPoints(true);
  bGPgl2->MergeAdd(*to,pNetwork);
  bool bres = bGPgl1->ShortestPathAStar(bGPgl2,result,pNetwork, touchedSects);
  bGPgl1->DeleteIfAllowed();
  bGPgl2->DeleteIfAllowed();
  return bres;
}

bool GLine::ShortestPathAStar(const GPoints *to, GLine *result,
                         DbArray<TupleId>* touchedSects) const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !to->IsDefined())
  {
    result->SetDefined(false);
    return false;
  }
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  bool bres = ShortestPathAStar(to, result, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return bres;
}

bool GLine::ShortestPathAStar(const GPoints *to, GLine *result,
                              const Network* pNetwork,
                              DbArray<TupleId>* touchedSects) const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if ( GetNetworkId() != to->GetNetworkId() )
  {
    return false;
  }
  result->SetNetworkId(pNetwork->GetId());
  if (Includes(to,pNetwork)) return true;
  GPoints *bGPgl1 = new GPoints(true);
  GetBGP(pNetwork, bGPgl1);
  bool bres = bGPgl1->ShortestPathAStar(to,result,pNetwork, touchedSects);
  bGPgl1->DeleteIfAllowed();
  return bres;
}

/*
NetdistanceNew method computes the network distance between two glines.
Uses GPoints Netdistance

*/
double GLine::NetdistanceNew (const GLine* pgl2) const
{
  double res = -1.0;
  if (!IsDefined() || !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->NoOfComponents() < 1)
    return -1.0;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(pgl2,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GLine::NetdistanceNew (const GLine* pgl2, const Network* pNetwork)const
{
  double res = -1.0;
  if (!IsDefined() || !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->NoOfComponents() < 1 ||
      GetNetworkId() != pgl2->GetNetworkId() )
    return res;
  if (Intersects(pgl2)) return 0.0;
  GPoints *bGPgl1 = new GPoints(true);
  GetBGP(pNetwork,bGPgl1);
  GPoints *bGPgl2 = new GPoints(true);
  pgl2->GetBGP(pNetwork,bGPgl2);
  res = bGPgl1->NetdistanceNew(bGPgl2,pNetwork);
  bGPgl1->DeleteIfAllowed();
  bGPgl2->DeleteIfAllowed();
  return res;
}

double GLine::NetdistanceNew(const GPoint* pgl2)const
{
  double res = -1.0;
  if (!IsDefined() || !pgl2->IsDefined() ||
      NoOfComponents() < 1)
    return res;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(pgl2,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GLine::NetdistanceNew(const GPoint* pgl2, const Network* pNetwork) const
{
  double res = -1.0;
  if (!IsDefined() || !pgl2->IsDefined() ||
      NoOfComponents() < 1 || GetNetworkId() != pgl2->GetNetworkId() )
    return res;
  if (pgl2->Inside(this,pNetwork->GetScalefactor()*0.01)) return 0.0;
  GPoints *bGPgl1 = new GPoints(true);
  GetBGP(pNetwork,bGPgl1);
  GPoints *bGPgl2 = new GPoints(true);
  bGPgl2->MergeAdd(*pgl2,pNetwork);
  res = bGPgl1->NetdistanceNew(bGPgl2,pNetwork);
  bGPgl1->DeleteIfAllowed();
  bGPgl2->DeleteIfAllowed();
  return res;
}

double GLine::NetdistanceNew(const GPoints* pgl2) const
{
  double res = -1.0;
  if (!IsDefined() || !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->Size() < 1)
    return res;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(pgl2,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GLine::NetdistanceNew(const GPoints* pgl2, const Network* pNetwork)const
{
  double res = -1.0;
  if (!IsDefined() || !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->Size() < 1 ||
      GetNetworkId() != pgl2->GetNetworkId() )
    return res;
  if (Includes(pgl2, pNetwork)) return 0.0;
  GPoints *bGPgl1 = new GPoints(true);
  GetBGP(pNetwork,bGPgl1);
  res = bGPgl1->NetdistanceNew(pgl2,pNetwork);
  bGPgl1->DeleteIfAllowed();
  return res;
}

double GLine::Netdistance(const GLine* pgl2)const
{
  double res = -1.0;
  if (!IsDefined()|| !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->NoOfComponents() < 1)
    return res;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = Netdistance(pgl2,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

/*
Netdistance method computes the network distance between two glines. Uses
astar network distance method of ~GPoint~.

*/

double GLine::Netdistance ( const GLine* pgl2, const Network* pNetwork )const
{
  double res = -1.0;
  if ( GetNetworkId() != pgl2->GetNetworkId() )
  {
    cmsg.inFunError ( "Both glines must belong to the network." );
    return res;
  }
  DbArray<TupleId>* touchedSects = 0;
  GLine *path = new GLine(false);
  if (ShortestPathBF(pgl2,path,pNetwork,touchedSects))
    res = path->GetLength();
  path->DeleteIfAllowed();
  path = 0;
  return res;
}

bool GLine::ShortestPathBF(const GLine *pgl2, GLine *result,
                           const Network *pNetwork,
                           DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined()|| !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->NoOfComponents() < 1)
    return false;
  if ( GetNetworkId() != pgl2->GetNetworkId() )
    return false;
  result->SetNetworkId(GetNetworkId());
  if (Intersects(pgl2))
    return true;
  GPoints *bGPgl1 = new GPoints(0);
  GetBGP(pNetwork,bGPgl1);
  GPoints *bGPgl2 = new GPoints(0);
  pgl2->GetBGP(pNetwork,bGPgl2);
  GPoint gp1, gp2;
  bool first = true;
  for ( int i = 0; i < bGPgl1->Size(); i++ )
  {
    bGPgl1->Get ( i,gp1 );
    for ( int j = 0; j < bGPgl2->Size(); j++ )
    {
      bGPgl2->Get ( j, gp2 );
      GLine *test = new GLine(false);
      DbArray<TupleId> *testTouchedSects = new DbArray<TupleId> (0);
      if (gp1.ShortestPathAStar(&gp2,test,pNetwork,testTouchedSects))
      {
        if (first)
          *result = *test;
        else
          if (test->GetLength() < result->GetLength())
            *result = *test;
        test->DeleteIfAllowed();
        test = 0;
        if (result->GetLength() <= 0)
        {
          bGPgl1->DeleteIfAllowed();
          bGPgl2->DeleteIfAllowed();
          if (touchedSects != 0) touchedSects->Append(*testTouchedSects);
          testTouchedSects->Destroy();
          delete testTouchedSects;
          return true;
        }
      }
      if (touchedSects != 0) touchedSects->Append(*testTouchedSects);
      testTouchedSects->Destroy();
      delete testTouchedSects;
    }
  }
  bGPgl1->DeleteIfAllowed();
  bGPgl2->DeleteIfAllowed();
  return true;
}

bool GLine::ShortestPathBF(const GLine *pgl2, GLine *result,
                           DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined()|| !pgl2->IsDefined() ||
      NoOfComponents() < 1 || pgl2->NoOfComponents() < 1)
    return false;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  bool res = ShortestPathBF(pgl2, result, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

/*
Distance method computes the Euclidean Distance between two glines. Uses
distance method of ~GPoint~.

*/

double GLine::Distance ( const GLine* pgl2 )const
{
  Line *l1 = new Line ( 0 );
  Line *l2 = new Line ( 0 );
  Gline2line ( l1 );
  pgl2->Gline2line ( l2 );
  if ( l1->IsDefined() && l2->IsDefined() )
  {
    double res = l1->Distance ( *l2 );
    l1->DeleteIfAllowed();
    l2->DeleteIfAllowed();
    return res;
  }
  else return numeric_limits<double>::max();
}

/*
 Computes the union of 2 glines.

*/

void GLine::Uniongl (const GLine *pgl2, GLine *res )const
{
  RouteInterval pRi1, pRi2;
  if ( !IsDefined() || NoOfComponents() == 0 )
  {
    if ( pgl2->IsDefined() && pgl2->NoOfComponents() > 0 )
    {
      if ( pgl2->IsSorted() )
      {
        for ( int j = 0; j < pgl2->NoOfComponents(); j++ )
        {
          pgl2->Get ( j,pRi2 );
          res->AddRouteInterval ( pRi2.GetRouteId(),
                                  pRi2.GetStartPos(),
                                  pRi2.GetEndPos() );
        }
      }
      else
      {
        pgl2->Get ( 0,pRi2 );
        RITree *ritree = new RITree ( pRi2.GetRouteId(),
                                      pRi2.GetStartPos(), pRi2.GetEndPos(),0,0
        );
        for ( int j = 1; j < pgl2->NoOfComponents(); j++ )
        {
          pgl2->Get ( j,pRi2 );
          ritree->Insert ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                           pRi2.GetEndPos() );
        }
        ritree->TreeToGLine ( res );
        ritree->RemoveTree();
      }
      res->SetDefined ( true );
      res->SetSorted ( true );
      res->SetNetworkId ( pgl2->GetNetworkId() );
    }
    else
    {
      res->SetDefined ( false );
      res->SetSorted ( false );
    }
  }
  else
  {
    if ( !pgl2->IsDefined() || pgl2->NoOfComponents() == 0 )
    {
      if ( IsDefined() && NoOfComponents() >0 )
      {
        if ( IsSorted() )
        {
          for ( int i = 0; i < NoOfComponents(); i++ )
          {
            Get ( i,pRi1 );
            res->AddRouteInterval ( pRi1.GetRouteId(),
                                    pRi1.GetStartPos(),
                                    pRi1.GetEndPos() );
          }
        }
        else
        {
          Get ( 0,pRi1 );
          RITree *ritree = new RITree ( pRi1.GetRouteId(),
                                        pRi1.GetStartPos(), pRi1.GetEndPos(),0,0
          );
          for ( int i = 1; i < NoOfComponents(); i++ )
          {
            Get ( i,pRi1 );
            ritree->Insert ( pRi1.GetRouteId(), pRi1.GetStartPos(),
                             pRi1.GetEndPos() );
          }
          ritree->TreeToGLine ( res );
          ritree->RemoveTree();
        }
        res->SetDefined ( true );
        res->SetSorted ( true );
        res->SetNetworkId ( GetNetworkId() );
      }
      else
      {
        res->SetDefined ( false );
        res->SetSorted ( false );
      }
    }
    else
    {
      if ( GetNetworkId() != pgl2->GetNetworkId() )
      {
        res->SetDefined ( false );
        res->SetSorted ( false );
      }
      else
      {
        res->SetNetworkId ( GetNetworkId() );
        if ( IsSorted() && pgl2->IsSorted() )
        {
          int i=0;
          int j=0;
          bool newroute = false;
          int iRouteId;
          double start, end;
          while ( i < NoOfComponents() && j < pgl2->NoOfComponents() )
          {
            Get ( i, pRi1 );
            pgl2->Get ( j, pRi2 );
            if ( pRi1.GetRouteId() < pRi2.GetRouteId() )
            {
              res->AddRouteInterval ( pRi1.GetRouteId(),
                                      pRi1.GetStartPos(),
                                      pRi1.GetEndPos() );
              i++;
            }
            else
            {
              if ( pRi1.GetRouteId() > pRi2.GetRouteId() )
              {
                res->AddRouteInterval ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                                        pRi2.GetEndPos() );
                j++;
              }
              else
              {
                if ( pRi1.GetEndPos() < pRi2.GetStartPos() )
                {
                  res->AddRouteInterval ( pRi1.GetRouteId(), pRi1.GetStartPos(),
                                          pRi1.GetEndPos() );
                  i++;
                }
                else
                {
                  if ( pRi2.GetEndPos() < pRi1.GetStartPos() )
                  {
                    res->AddRouteInterval ( pRi2.GetRouteId(),
                                            pRi2.GetStartPos(),
                                            pRi2.GetEndPos() );
                    j++;
                  }
                  else
                  {
                    iRouteId = pRi1.GetRouteId();
                    start = min ( pRi1.GetStartPos(), pRi2.GetStartPos() ),
                    end = max ( pRi1.GetEndPos(), pRi2.GetEndPos() );
                    i++;
                    j++;
                    newroute = false;
                    while ( i < NoOfComponents() && !newroute )
                    {
                      Get ( i,pRi1 );
                      if ( pRi1.GetRouteId() == iRouteId )
                      {
                        if ( pRi1.GetStartPos() <= end )
                        {
                          end = max ( pRi1.GetEndPos(), end );
                          i++;
                        }
                        else newroute = true;
                      }
                      else newroute = true;
                    }
                    newroute = false;
                    while ( j < pgl2->NoOfComponents() && !newroute )
                    {
                      pgl2->Get ( j,pRi2 );
                      if ( pRi2.GetRouteId() == iRouteId )
                      {
                        if ( pRi2.GetStartPos() <= end )
                        {
                          end = max ( pRi2.GetEndPos(), end );
                          j++;
                        }
                        else newroute = true;
                      }
                      else newroute = true;
                    }
                    res->AddRouteInterval ( iRouteId, start, end );
                  }
                }
              }
            }
          }
          while ( i < NoOfComponents() )
          {
            Get ( i,pRi1 );
            res->AddRouteInterval ( pRi1.GetRouteId(),
                                    pRi1.GetStartPos(),
                                    pRi1.GetEndPos() );
            i++;
          }
          while ( j < pgl2->NoOfComponents() )
          {
            pgl2->Get ( j,pRi2 );
            res->AddRouteInterval ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                                    pRi2.GetEndPos() );
            j++;
          }
        res->SetDefined ( true );
        res->SetSorted ( true );
        }
        else
        {
          RITree *ritree;
          Get ( 0,pRi1 );
          ritree = new RITree ( pRi1.GetRouteId(),
                                pRi1.GetStartPos(), pRi1.GetEndPos(),0,0 );
          for ( int i = 1; i < NoOfComponents(); i++ )
          {
            Get ( i,pRi1 );
            ritree->Insert ( pRi1.GetRouteId(), pRi1.GetStartPos(),
                             pRi1.GetEndPos() );
          }
          for ( int j = 0; j < pgl2->NoOfComponents(); j++ )
          {
            pgl2->Get ( j,pRi2 );
            ritree->Insert ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                             pRi2.GetEndPos() );
          }
          ritree->TreeToGLine ( res );
          ritree->RemoveTree();
          res->SetDefined ( true );
          res->SetSorted ( true );
        }
      }
    }
  }
  res->TrimToSize();
}


void GLine::Gline2line ( Line* res )const
{
  res->Clear();
  if ( IsDefined() && NoOfComponents() > 0 )
  {
    //Network* pNetwork = NetworkManager::GetNetwork(GetNetworkId());
    Network* pNetwork =
      NetworkManager::GetNetworkNew ( GetNetworkId(), netList);
    RouteInterval rI;
    Line l(0);
    Line x = l;
    for ( int i=0; i < this->NoOfComponents(); i++ )
    {
      this->Get ( i,rI );
      SimpleLine *pSubline = new SimpleLine ( 0 );
      pNetwork->GetLineValueOfRouteInterval ( &rI, pSubline );
      if ( pSubline->IsDefined() )
      {
        Line partLine(0);
        pSubline->toLine ( partLine );
        pSubline->DeleteIfAllowed();
        l.Union(partLine,x);
        l = x;
      }
    }
    NetworkManager::CloseNetwork ( pNetwork );
    ( *res ) = x;
    res->SetDefined ( true );
  }
  else
  {
    if ( IsDefined() && NoOfComponents() == 0 )
    {
      res->SetDefined ( true );
    }
    else
    {
      res->SetDefined ( false );
    }
  }
  res->TrimToSize();
}

bool GLine::Intersects (const RouteInterval* ri, const double tolerance)const
{
  if (IsSorted())
  {
    return searchUnit(this, 0, NoOfComponents()-1, *ri);
  }
  else
  {
    RouteInterval rigl;
    for (int i = 0; i < NoOfComponents(); i++)
    {
      Get(i,rigl);
      if (ri->Intersects(&rigl, tolerance)) return true;
    }
  }
  return false;
}

bool GLine::Intersects ( const GLine *pgl )const
{
  RouteInterval pRi1, pRi2;
  if ( !IsSorted() )
  {
    for ( int i = 0; i < NoOfComponents(); i++ )
    {
      Get ( i, pRi1 );
      if ( pgl->IsSorted() )
      {
        if ( searchUnit ( pgl, 0, pgl->NoOfComponents()-1, pRi1 ) )
        {
          return true;
        };
      }
      else
      {
        for ( int j = 0 ; j < pgl->NoOfComponents(); j ++ )
        {
          pgl->Get ( j,pRi2 );
          if ( pRi1.GetRouteId() == pRi2.GetRouteId() &&
                  ((!( pRi1.GetEndPos() < pRi2.GetStartPos() ||
                        pRi2.GetStartPos() > pRi1.GetEndPos() )) ||
                   (!(pRi2.GetEndPos() < pRi1.GetStartPos() ||
                      pRi2.GetStartPos() > pRi1.GetEndPos()  ))))
          {
            return true;
          }
        }
      }
    }
    return false;
  }
  else
  {
    if ( pgl->IsSorted() )
    {
      int i = 0;
      int j = 0;
      while ( i<NoOfComponents() && j < pgl->NoOfComponents() )
      {
        Get ( i,pRi1 );
        pgl->Get ( j,pRi2 );
        if ( pRi1.GetRouteId() < pRi2.GetRouteId() ) i++;
        else
          if ( pRi1.GetRouteId() > pRi2.GetRouteId() ) j++;
          else
            if ( pRi1.GetStartPos() > pRi2.GetEndPos() ) j++;
            else
              if ( pRi1.GetEndPos() < pRi2.GetStartPos() ) i++;
              else return true;
      }
      return false;
    }
    else
    {
      for ( int i = 0; i < pgl->NoOfComponents(); i++ )
      {
        pgl->Get ( i, pRi2 );
        if ( searchUnit ( this, 0, NoOfComponents()-1, pRi2 ) ) return true;
      }
      return false;
    }
  }
}

bool GLine::Includes(const GPoints* gps) const
{
  bool res = false;
  if (!IsDefined() || !gps->IsDefined() ||
       NoOfComponents() < 1 || gps->Size() < 1)
  return res;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = Includes(gps,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

bool GLine::Includes(const GPoints* gps, const Network* pNetwork) const
{
  bool res = false;
  if (!IsDefined() || !gps->IsDefined() ||
       NoOfComponents() < 1 || gps->Size() < 1)
  return res;
  for (int i = 0; i < gps->Size() ; i++)
  {
    GPoint gp;
    gps->Get(i,gp);
    GPointList* aliasGPoints = new GPointList(&gp,pNetwork);
    GPoint test = aliasGPoints->NextGPoint();
    while (test.IsDefined() && !res)
    {
      if (test.Inside(this, pNetwork->GetScalefactor()*0.01)) res = true;
      test = aliasGPoints->NextGPoint();
    }
    aliasGPoints->Destroy();
    delete aliasGPoints;
    if (res) break;
  }
  return res;
}

/*
Secondo Type Constructor for class ~GLine~

*/

struct glineInfo:ConstructorInfo{
  glineInfo(){
    name = GLine::BasicType();
    signature = "-> DATA";
    typeExample = GLine::BasicType();
    listRep = "(<netId> <list of routeintervals>)";
    valueExample = "(1 ((23 34.8 435.3)(...)))";
    remarks = "Route interval: (<routeid><startpos><endpos>)";
  }
};

struct glineFunctions:ConstructorFunctions<GLine>{
  glineFunctions(){
    in = GLine::In;
    out = GLine::Out;
    create = GLine::Create;
    deletion = GLine::Delete;
    open = OpenAttribute<GLine>;
    save = SaveAttribute<GLine>;
    close = GLine::Close;
    clone = GLine::CloneGLine;
    cast = GLine::Cast;
    sizeOf = GLine::SizeOf;
    kindCheck = GLine::Check;
  }
};

glineInfo gli;
glineFunctions glf;
TypeConstructor glineTC(gli,glf);


/*
4 class ~GPoint~

4.1 Constructors

See ~network.h~ class definition of ~GPoint~

4.2 Methods of class ~GPoint~

*/
Word GPoint::InGPoint ( const ListExpr typeInfo,
                        const ListExpr instance,
                        const int errorPos,
                        ListExpr& errorInfo,
                        bool& correct )
{
  if ( nl->ListLength ( instance ) == 4 )
  {
    if ( nl->IsAtom ( nl->First ( instance ) ) &&
            nl->AtomType ( nl->First ( instance ) ) == IntType &&
            nl->IsAtom ( nl->Second ( instance ) ) &&
            nl->AtomType ( nl->Second ( instance ) ) == IntType &&
            nl->IsAtom ( nl->Third ( instance ) ) &&
            nl->AtomType ( nl->Third ( instance ) ) == RealType &&
            nl->IsAtom ( nl->Fourth ( instance ) ) &&
            nl->AtomType ( nl->Fourth ( instance ) ) == IntType )
    {
      GPoint *gp = new GPoint (
          true,
          nl->IntValue ( nl->First ( instance ) ),
          nl->IntValue ( nl->Second ( instance ) ),
          nl->RealValue ( nl->Third ( instance ) ),
          ( Side ) nl->IntValue ( nl->Fourth ( instance ) ) );
      correct = true;
      return SetWord ( gp );
    }
  }
  if (nl->IsEqual(instance, Symbol::UNDEFINED() ))
  {
    correct = true;
    return SetWord ( new GPoint(false));
  }

  correct = false;
  return SetWord ( Address ( 0 ) );
}

ListExpr GPoint::OutGPoint ( ListExpr typeInfo, Word value )
{
  GPoint *gp = ( GPoint* ) value.addr;

  if ( gp->IsDefined() )
  {
    return nl->FourElemList (
               nl->IntAtom ( gp->GetNetworkId() ),
               nl->IntAtom ( gp->GetRouteId() ),
               nl->RealAtom ( gp->GetPosition() ),
               nl->IntAtom ( gp->GetSide() ) );
  }
  return nl->SymbolAtom ( Symbol::UNDEFINED());
}

Word GPoint::CreateGPoint ( const ListExpr typeInfo )
{
  return SetWord ( new GPoint ( true ) );
}

void GPoint::DeleteGPoint ( const ListExpr typeInfo, Word& w )
{
  GPoint *gp = ( GPoint* ) w.addr;
  if (gp->DeleteIfAllowed()) w.addr = 0;
}

void GPoint::CloseGPoint ( const ListExpr typeInfo, Word& w )
{
  GPoint *gp = ( GPoint* ) w.addr;
  if(gp->DeleteIfAllowed()) w.addr = 0;
}

Word GPoint::CloneGPoint ( const ListExpr typeInfo, const Word& w )
{
  return SetWord ( ( ( GPoint* ) w.addr )->Clone() );
}

void* GPoint::CastGPoint ( void* addr )
{
  return new ( addr ) GPoint;
}

int GPoint::SizeOfGPoint()
{
  return sizeof ( GPoint );
}

bool GPoint::CheckGPoint ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, GPoint::BasicType() ) );
}

/*
Netdistance function computes the network distance between two ~GPoint~s.
Using Dijkstras-Algorithm for shortest path computing

*/

double GPoint::Netdistance ( const GPoint* pToGPoint)const
{
  if (!IsDefined() || !pToGPoint->IsDefined())
    return -1.0;
  GLine* pGLine = new GLine ( 0 );
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  double res = -1.0;
  if ( ShortestPath ( pToGPoint, pGLine, pNetwork, 0 ) )
    res = pGLine->GetLength();
  pGLine->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GPoint::NetdistanceNew(const GPoint* pToGPoint, const Network* pNetwork)
                            const
{
  GLine* pGLine = new GLine ( 0 );
  double res = -1.0;
  if ( ShortestPathAStar ( pToGPoint, pGLine, pNetwork,0 ) )
    res = pGLine->GetLength();
  pGLine->DeleteIfAllowed();
  return res;
}

double GPoint::NetdistanceNew ( const GPoint* pToGPoint )const
{
  double result = -1.0;
  if (!IsDefined() || !pToGPoint->IsDefined())
    return result;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  result = NetdistanceNew(pToGPoint,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}

double GPoint::NewNetdistance ( const GPoint* pToGPoint, GLine* gline ) const
{
  GPoint* gp1 = new GPoint ( true,GetNetworkId(),GetRouteId(),GetPosition() );
  GPoint* gp2 = new GPoint ( true,pToGPoint->GetNetworkId(),
                             pToGPoint->GetRouteId(),pToGPoint->GetPosition() );
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  double res = 0.0;
  if ( gp1->ShortestPath ( gp2, gline, pNetwork, 0 ) )
    res = gline->GetLength();
  gp1->DeleteIfAllowed();
  gp2->DeleteIfAllowed();
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GPoint::NetdistanceNew (const GLine* toGLine)const
{
  double res = -1.0;
  if (!IsDefined() || !toGLine->IsDefined() || toGLine->NoOfComponents() < 1 ||
      GetNetworkId() != toGLine->GetNetworkId())
    return res;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(toGLine,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GPoint::NetdistanceNew (const GLine* toGLine, const Network* pNetwork)
              const
{
  double res = -1.0;
  if (!IsDefined() || !toGLine->IsDefined() || toGLine->NoOfComponents() < 1 ||
      GetNetworkId() != toGLine->GetNetworkId())
    return res;
  if (Inside(toGLine, pNetwork->GetScalefactor()*0.01)) return 0.0;
  GLine *path = new GLine(0);
  if (ShortestPathAStar(toGLine,path,pNetwork,0))
    res = path->GetLength();
  path->DeleteIfAllowed();
  return res;
}

double GPoint::NetdistanceNew (const GPoints* toGPoints)const
{
  double res = -1.0;
  if (!IsDefined() || !toGPoints->IsDefined() ||
       toGPoints->Size() < 1 ||
       GetNetworkId() != toGPoints->GetNetworkId())
    return res;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(toGPoints,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GPoint::NetdistanceNew (const GPoints* toGPoints,
                               const Network* pNetwork) const
{
  double res = -1.0;
  if (!IsDefined() || !toGPoints->IsDefined() ||
       toGPoints->Size() < 1 ||
       GetNetworkId() != toGPoints->GetNetworkId())
    return res;
  GLine *path = new GLine(0);
  if (ShortestPathAStar(toGPoints,path,pNetwork,0))
    res = path->GetLength();
  path->DeleteIfAllowed();
  return res;
}

/*
Distance function computes the Euclidean Distance between two ~GPoint~s.

*/

double GPoint::Distance ( const GPoint* pToGPoint ) const
{
  map<int,string>::iterator it = netList->begin();
  if ( IsDefined() && pToGPoint->IsDefined() &&
          GetNetworkId() == pToGPoint->GetNetworkId() )
  {
    //Network* pNetwork=NetworkManager::GetNetwork(GetNetworkId());
    Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(),
                        netList );
    Point *from = new Point ( true );
    pNetwork->GetPointOnRoute ( this, from );
    Point *to = new Point ( true );
    pNetwork->GetPointOnRoute ( pToGPoint,to );
    double res = from->Distance ( *to );
    from->DeleteIfAllowed();
    to->DeleteIfAllowed();
    NetworkManager::CloseNetwork ( pNetwork );
    return res;
  }
  else return numeric_limits<double>::max();
}

/*
Returns true if the gpoint is inside the gline false elsewhere.

*/
bool GPoint::Inside (const  GLine *gl, const double tolerance )const
{
  if ( ! ( gl->IsDefined() ) || !IsDefined() ||
          gl->NoOfComponents() < 1 ) return false;
  if ( GetNetworkId() != gl->GetNetworkId() ) return false;
  RouteInterval pCurrRInter;
  if ( gl->IsSorted() )
    return ( searchRouteInterval ( this, gl, tolerance, 0,
                                   gl->NoOfComponents()-1 ) );
  else
  {
    int i = 0;
    while ( i < gl->NoOfComponents() )
    {
      gl->Get ( i, pCurrRInter );
      if ( pCurrRInter.GetRouteId() == GetRouteId() )
      {
        if ( pCurrRInter.GetStartPos() <= GetPosition() &&
                GetPosition() <= pCurrRInter.GetEndPos() )
          return true;
        if ( pCurrRInter.GetStartPos() >= GetPosition() &&
                GetPosition() >= pCurrRInter.GetEndPos() )
          return true;
        if ( AlmostEqualAbsolute(pCurrRInter.GetStartPos(), GetPosition(),
                                 tolerance))
          return true;
        if ( AlmostEqualAbsolute( pCurrRInter.GetEndPos(), GetPosition(),
                                  tolerance ))
          return true;
      }
      i++;
    }
    return false;
  }
  return false;
}

bool GPoint::operator== ( const GPoint& p ) const
{
  if ( !IsDefined() || !p.IsDefined() ) return false;
  else
  {
    if ( m_iNetworkId == p.GetNetworkId() &&
            m_xRouteLocation.rid == p.GetRouteId() &&
            m_xRouteLocation.d == p.GetPosition() &&
            ( m_xRouteLocation.side == p.GetSide() || m_xRouteLocation.side == 2
||
              p.GetSide() == 2 ) )
    {
      return true;
    }
    else return false;
  }
}

bool GPoint::operator!= ( const GPoint& p ) const
{
  return ! ( *this == p );
}

/*
Computes the shortest path between start and end in the network. Using
Dijkstras Algorithm. The path is returned as gline value.

*/

bool GPoint::ShortestPathAStar(const GPoint *to, GLine *result,
                               DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !to->IsDefined())
  {
    result->SetDefined(false);
    return false;
  }
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  double bres = ShortestPathAStar(to, result, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return bres;
}

bool GPoint::ShortestPathAStar (const GPoint *to, GLine *result,
                                const Network *pNetwork,
                                DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !to->IsDefined())
      return false;
  if (GetNetworkId() != to->GetNetworkId()) return false;
  result->SetNetworkId(GetNetworkId());
  result->SetDefined(true);
  if (Compare(*to) == 0) return true;
  GPoints* start = new GPoints(1);
  start->MergeAdd(*this,pNetwork);
  GPoints* target = new GPoints(1);
  target->MergeAdd(*to,pNetwork);
  bool res = start->ShortestPathAStar(target, result, pNetwork, touchedSects);
  result->SetDefined(res);
  start->DeleteIfAllowed();
  target->DeleteIfAllowed();
  return res;
}

bool GPoint::ShortestPath ( const GPoint *to, GLine *result,
                            DbArray<TupleId>* touchedSects) const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !to->IsDefined())
  {
    result->SetDefined(false);
    return false;
  }
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  double bres = ShortestPath(to, result, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return bres;
}

bool GPoint::ShortestPath ( const GPoint *to, GLine *result,
                            const Network* pNetwork,
                            DbArray<TupleId>* touchedSects )const
{
  result->Clear();
  result->SetDefined(false);
  if (touchedSects != 0) touchedSects->clean();
  if ( !IsDefined() || !to->IsDefined() || to == 0)
  {
    sendMessage ( "Both gpoints must exist and be defined." );
    return false;
  }
  // Check wether both points belong to the same network
  if ( GetNetworkId() != to->GetNetworkId() || pNetwork == 0 ||
       pNetwork->GetId() != GetNetworkId())
  {
    sendMessage ( "Both gpoints belong to network." );
    return false;
  }
  // Get sections where the path should start or end
  TupleId startSectTID = pNetwork->GetTupleIdSectionOnRoute ( this );
  if (touchedSects != 0) touchedSects->Append(startSectTID);
  Tuple* startSection = pNetwork->GetSection ( startSectTID );
  if ( startSection == 0 )
  {
    sendMessage ( "Starting GPoint not found in network." );
    return false;
  }
  TupleId lastSectTID = pNetwork->GetTupleIdSectionOnRoute ( to );
  Tuple* endSection = pNetwork->GetSection ( lastSectTID );
  if ( endSection == 0 )
  {
    sendMessage ( "End GPoint not found in network." );
    startSection->DeleteIfAllowed();
    return false;
  }
  if ( startSectTID == lastSectTID  ||
       GetRouteId() == to->GetRouteId() )
  {
    result->SetDefined(true);
    result->SetNetworkId(pNetwork->GetId());
    result->AddRouteInterval ( GetRouteId(), GetPosition(), to->GetPosition());
    startSection->DeleteIfAllowed();
    endSection->DeleteIfAllowed();
    return true;
  }

/*
Calculate the shortest path using dijkstras algorithm.

Initialize PriorityQueue

*/
  PrioQueueA *prioQ = new PrioQueueA ( 0 );
  SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
  int pHelp = -1;
  visitedSect->Insert(SectIDTreeEntry(
                        SectEntry(startSectTID,
                                  (TupleId) numeric_limits<long>::max(),
                                  false,
                                  false,
                                  -1,
                                  0.0),
                        -1,-1),
                        pHelp);
  visitedSect->Insert(SectIDTreeEntry(SectEntry(startSectTID,
                                  (TupleId) numeric_limits<long>::max(),
                                  true,
                                  true,
                                  -1,
                                  0.0),
                        -1,-1),
                        pHelp);
  double sectMeas1 =
    ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
  double sectMeas2 =
    ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
  double dist = 0.0;
  vector<DirectedSection> adjSectionList;
  adjSectionList.clear();
  if ( GetSide() == 0 || GetSide() ==2)
  {
    dist = GetPosition() - sectMeas1;
    pNetwork->GetAdjacentSections ( startSectTID, false, adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ )
    {
      DirectedSection actNextSect = adjSectionList[i];
      if ( actNextSect.GetSectionTid() != startSectTID )
          prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist, dist,
                                    actNextSect.GetUpDownFlag(),
                                    startSectTID, false ),
                          visitedSect,
                          touchedSects);
    }
    adjSectionList.clear();
  }

  if ( GetSide() == 1 || GetSide()==2 )
  {
    dist = sectMeas2 - GetPosition();
    pNetwork->GetAdjacentSections ( startSectTID, true, adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ )
    {
      DirectedSection actNextSect = adjSectionList[i];
      if ( actNextSect.GetSectionTid() != startSectTID )
        prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist,dist,
                                  actNextSect.GetUpDownFlag(),
                                  startSectTID,true ),
                        visitedSect, touchedSects);
    }
    adjSectionList.clear();
  }
/*
Use priorityQueue to find shortestPath.

*/
  PQEntryA *actPQEntry;
  bool found = false;
  vector<PQEntryA> candidate;
  while ( !prioQ->IsEmpty() && !found )
  {
    actPQEntry = prioQ->GetAndDeleteMin ( visitedSect );
    Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
    sectMeas1 =
      ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    sectMeas2 =
      ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    dist = actPQEntry->valFromStart + fabs ( sectMeas2 - sectMeas1 );
    if ( actPQEntry->sectID != lastSectTID )
    {
/*
Search in the network until reached last section.
Get adjacent sections and insert into priority Queue.

*/
      adjSectionList.clear();
      pNetwork->GetAdjacentSections ( actPQEntry->sectID,
                                      actPQEntry->upDownFlag,
                                      adjSectionList );
      for ( size_t i = 0; i <adjSectionList.size();i++ )
      {
        DirectedSection actNextSect = adjSectionList[i];
        if ( actNextSect.GetSectionTid() != actPQEntry->sectID )
          prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                    dist,dist,
                                    actNextSect.GetUpDownFlag(),
                                    actPQEntry->sectID,
                                    actPQEntry->upDownFlag ),
                          visitedSect, touchedSects);
      }
      if (actPQEntry != 0){
        delete actPQEntry;
        actPQEntry=0;
      }
      if (actSection != 0){
        actSection->DeleteIfAllowed();
        actSection = 0;
      }
    }
    else
    {
/*
Shortest Path found. Compute last route interval and resulting gline.

*/

      found = true;
      double startRI, endRI;
      int actRouteId =
        ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      if ( actPQEntry->upDownFlag)
        startRI = sectMeas1;
      else
        startRI = sectMeas2;
      endRI = to->GetPosition();
      double minPathLength = actPQEntry->valFromStart + fabs(endRI-startRI);
      bool btestend = false;
      while (!btestend && !prioQ->IsEmpty())
      {
        PQEntryA* testEntry = prioQ->GetAndDeleteMin(visitedSect);
        if (testEntry->valFromStart < minPathLength)
        {
          Tuple *testSection = pNetwork->GetSection ( testEntry->sectID );
          double testMeas1 =
            ((CcReal*)testSection->GetAttribute(SECTION_MEAS1))->GetRealval();
          double testMeas2 =
            ((CcReal*)testSection->GetAttribute(SECTION_MEAS2))->GetRealval();
          int testRouteId =
            ((CcInt*)testSection->GetAttribute(SECTION_RID))->GetIntval();
          if (testEntry->sectID != lastSectTID)
          {
            adjSectionList.clear();
            pNetwork->GetAdjacentSections ( testEntry->sectID,
                                            testEntry->upDownFlag,
                                            adjSectionList );
            dist = testEntry->valFromStart + fabs(testMeas2 - testMeas1);
            for ( size_t i = 0; i <adjSectionList.size();i++ )
            {
              DirectedSection actNextSect = adjSectionList[i];
              if ( actNextSect.GetSectionTid() != testEntry->sectID )
                prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                    dist, dist,
                                    actNextSect.GetUpDownFlag(),
                                    testEntry->sectID,
                                    testEntry->upDownFlag),
                          visitedSect, touchedSects);
            }
            if (testEntry != 0)
            {
              delete testEntry;
              testEntry=0;
            }
            if (testSection != 0)
            {
              testSection->DeleteIfAllowed();
              testSection = 0;
            }
          }
          else
          {
            double endTestRI = to->GetPosition();
            double startTestRI = 0.0;
            if ( testEntry->upDownFlag)
              startTestRI = testMeas1;
            else
              startTestRI = testMeas2;
            double actPathLength =
              testEntry->valFromStart + fabs(endTestRI-startTestRI);
            if (actPathLength < minPathLength)
            {
              minPathLength = actPathLength;
              endRI = endTestRI;
              startRI = startTestRI;
              actRouteId = testRouteId;

              if (actPQEntry != 0)
              {
                delete actPQEntry;
              }
              actPQEntry = testEntry;
              actSection->DeleteIfAllowed();
              actSection = testSection;
            }
          }
          if (testSection != 0 && actSection != testSection)
          {
            testSection->DeleteIfAllowed();
            testSection = 0;
          }
        }
        else
          btestend = true;
        if (testEntry != 0 && testEntry != actPQEntry)
        {
          delete testEntry;
          testEntry = 0;
        }
      }
      if (actSection != 0)
      {
        actSection->DeleteIfAllowed();
        actSection = 0;
      }

/*
Get the sections used for shortest path and write them in right order (from
start to end gpoint) in the resulting gline. Because Dijkstra gives the
sections from end to start we first have to put the result sections on a
stack to turn in right order.

*/
      RIStackP *riStack = new RIStackP (0);
      riStack->Push( actRouteId, startRI, endRI );
      TupleId lastSectId = actPQEntry->sectID;
      int pElem = visitedSect->Find ( actPQEntry->beforeSectID,
                                      actPQEntry->beforeUpDown );
      bool end = false;
      bool upDown;
      //   if (startRI >= endRI) upDown = false;
      if ( startRI > endRI ||
           AlmostEqualAbsolute( startRI,endRI, pNetwork->GetScalefactor()*0.01))
        upDown = false;
      else upDown = true;
      while ( !end && pElem > -1 && pElem < visitedSect->fFree)
      {
        //GetSection
        TupleId actTupleId =
          visitedSect->GetTreeEntry(pElem).GetEntry().GetSectId();
        if (actSection != 0) actSection->DeleteIfAllowed();
        actSection = pNetwork->GetSection ( actTupleId );
        if (actSection != 0)
        {
          actRouteId =
            ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
          sectMeas1 =
            ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
          sectMeas2 =
            ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
          upDown =
            visitedSect->GetTreeEntry(pElem).GetEntry().GetUpDownFlag();
          if (actTupleId != startSectTID)
          {
            if (upDown)
              riStack->Push(actRouteId, sectMeas1,sectMeas2);
            else
              riStack->Push(actRouteId, sectMeas2,sectMeas1);
            lastSectId =
              visitedSect->GetTreeEntry(pElem).GetEntry().GetSectId();
            pElem =
              visitedSect->Find(visitedSect->GetTreeEntry(pElem).GetEntry().
                                  GetBeforeSectId(),
                                visitedSect->GetTreeEntry(pElem).GetEntry().
                                  GetBeforeSectUpDown());
          }
          else
          {
            end = true;
            if (upDown)
              riStack->Push ( actRouteId, GetPosition(), sectMeas2);
            else
              riStack->Push ( actRouteId, GetPosition(), sectMeas1);
          }
        }
        if ( actSection )
        {
          actSection->DeleteIfAllowed();
          actSection = 0;
        }
      }
      // Cleanup and return result
      riStack->StackToGLine ( result, GetNetworkId() );
      riStack->Destroy();
      delete riStack;
      result->SetSorted ( false );
    }
    if ( actPQEntry )
    {
      delete actPQEntry;
      actPQEntry = 0;
    }
    if ( actSection )
    {
      actSection->DeleteIfAllowed();
      actSection = 0;
    }
  }
  if ( visitedSect != 0 )
  {
    visitedSect->Destroy();
    delete visitedSect;
  }
  prioQ->Destroy();
  delete prioQ;
  startSection->DeleteIfAllowed();
  endSection->DeleteIfAllowed();
  result->SetDefined ( found );
  result->TrimToSize();
  return found;
};

 bool GPoint::ShortestPathAStar(const GLine *ziel, GLine *result,
                                DbArray<TupleId>* touchedSects)const
 {
   result->Clear();
   if (touchedSects != 0) touchedSects->clean();
   if (!IsDefined() || !ziel->IsDefined() || ziel->NoOfComponents() < 1)
    return false;
   Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
   bool res = ShortestPathAStar(ziel,result,pNetwork,touchedSects);
   NetworkManager::CloseNetwork(pNetwork);
   return res;
 }

 bool GPoint::ShortestPathAStar(const GLine* ziel, GLine* result,
                                const Network *pNetwork,
                         DbArray<TupleId>* touchedSects)const
 {
    result->Clear();
    if (touchedSects != 0) touchedSects->clean();
    if (!IsDefined() || !ziel->IsDefined() || ziel->NoOfComponents() < 1)
      return false;
    if (GetNetworkId() != ziel->GetNetworkId()) return false;
    result->SetNetworkId(GetNetworkId());
    if (Inside(ziel,pNetwork->GetScalefactor()*0.01))return true;
    GPoints* start = new GPoints(1);
    start->MergeAdd(*this,pNetwork);
    GPoints* target = new GPoints(0);
    ziel->GetBGP(pNetwork,target);
    bool res = start->ShortestPathAStar(target, result, pNetwork, touchedSects);
    result->SetDefined(res);
    start->DeleteIfAllowed();
    target->DeleteIfAllowed();
    return res;
 }

bool GPoint::ShortestPathAStar(const GPoints *ziel, GLine *result,
                       DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !ziel->IsDefined() || ziel->Size() < 1)
    return false;
  Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  bool res = ShortestPathAStar(ziel,result,pNetwork,touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

bool GPoint::ShortestPathAStar(const GPoints* ziel, GLine* result,
                               const Network *pNetwork,
                       DbArray<TupleId>* touchedSects)const
{
  result->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if (!IsDefined() || !ziel->IsDefined() || ziel->Size() < 1)
    return false;
  if (GetNetworkId() != ziel->GetNetworkId()) return false;
  result->SetNetworkId(GetNetworkId());
  GPoints* start = new GPoints(0);
  start->MergeAdd(*this,pNetwork);
  bool res = start->ShortestPathAStar(ziel, result, pNetwork, touchedSects);
  result->SetDefined(res);
  start->DeleteIfAllowed();
  return res;
}


/*
Returns the shortest path tree from the gpoint to all sections of the
network. The distances are stored in an DbArray<double>, where the index
of the Array-Field is two times the section number for up sections and
two times the section number plus one for down sections.

*/


void GPoint::ShortestPathTree(const Network* pNetwork,
                               DbArray<ShortestPathTreeEntry> *res) const
{
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
      GetNetworkId() == pNetwork->GetId() && res != 0)
  {
    TupleId startSectionTID = pNetwork->GetTupleIdSectionOnRoute(this);
    PrioQueueA *prioQ = new PrioQueueA ( 0 );
    SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
    int pHelp = -1;
    int sectPos = -1;
    visitedSect->Insert(SectIDTreeEntry(
                         SectEntry(startSectionTID,
                                   startSectionTID,
                                    true, true,
                                    -1,
                                    0.0),
                          -1,-1),
                          pHelp);
    visitedSect->Insert(SectIDTreeEntry(
                         SectEntry(startSectionTID,
                                   startSectionTID,
                                    false, false,
                                    -1,
                                    0.0),
                          -1,-1),
                          pHelp);
    Tuple* startSection = pNetwork->GetSection(startSectionTID);
    double sectMeas1 =
      ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    int actRouteId =
      ((CcInt*)startSection->GetAttribute(SECTION_RID))->GetIntval();
    int sectID =
      ((CcInt*)startSection->GetAttribute(SECTION_SID))->GetIntval();
    res->Put(2*sectID, ShortestPathTreeEntry(0.0,true));
    res->Put(2*sectID + 1, ShortestPathTreeEntry(0.0,false));
    double dist = GetPosition() - sectMeas1;
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    pNetwork->GetAdjacentSections ( startSectionTID, false,
                                    adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ )
    {
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist, dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectionTID, false ),
                           visitedSect, 0);
    }
    adjSectionList.clear();
    dist = sectMeas2 - GetPosition();
    pNetwork->GetAdjacentSections ( startSectionTID, true,
                                    adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ )
    {
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist,dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectionTID, true ),
                          visitedSect, 0);
    }
    adjSectionList.clear();
    PQEntryA* actPQEntry = 0;
    while (!prioQ->IsEmpty())
    {
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      actRouteId =
        ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      sectID =
        ((CcInt*)actSection->GetAttribute(SECTION_SID))->GetIntval();
      sectPos = 2*sectID;
      if (!actPQEntry->upDownFlag) sectPos++;
      res->Put(sectPos, ShortestPathTreeEntry(actPQEntry->valFromStart,
                                              actPQEntry->upDownFlag));
      dist = actPQEntry->valFromStart + fabs ( sectMeas2 - sectMeas1 );
      pNetwork->GetAdjacentSections ( actPQEntry->sectID,
                                      actPQEntry->upDownFlag,
                                      adjSectionList );
      if (adjSectionList.size() != 0)
      {
        for ( size_t i = 0; i <adjSectionList.size();i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                     dist,dist,
                                     actNextSect.GetUpDownFlag(),
                                     actPQEntry->sectID,
                                     actPQEntry->upDownFlag),
                             visitedSect, 0);
        }
      }
      else
      {
        if (!actPQEntry->upDownFlag)
          sectPos--;
        else
          sectPos++;
        res->Put(sectPos, ShortestPathTreeEntry(actPQEntry->valFromStart +
                                                fabs(sectMeas2 - sectMeas1),
                                                !actPQEntry->upDownFlag));
      }
      adjSectionList.clear();
      if (actSection != 0)
      {
        actSection->DeleteIfAllowed();
        actSection = 0;
      }
       if (actPQEntry != 0)
      {
        delete actPQEntry;
        actPQEntry=0;
      }
    }
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    startSection->DeleteIfAllowed();
  }
}

/*
Almost analogous to shortest path tree but stops computation if all sections
of ~toReach~ are inserted.

*/

void GPoint::ShortestPathTree(const Network* pNetwork,
                              DbArray<ShortestPathTreeEntry> *res,
                              SortedTree<Entry<SectionValue> > *toReach) const{
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
      GetNetworkId() == pNetwork->GetId() && res != 0) {
    TupleId startSectionTID = pNetwork->GetTupleIdSectionOnRoute(this);
    PrioQueueA *prioQ = new PrioQueueA ( 0 );
    SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
    int pHelp = -1;
    int sectPos = -1;
    visitedSect->Insert(SectIDTreeEntry(
      SectEntry(startSectionTID,
                startSectionTID,
                true, true,
                -1,
                0.0),
                -1,-1),
                pHelp);
    visitedSect->Insert(SectIDTreeEntry(
      SectEntry(startSectionTID,
                startSectionTID,
                false, false,
                -1,
                0.0),
                -1,-1),
                pHelp);
    Tuple* startSection = pNetwork->GetSection(startSectionTID);
    double sectMeas1 =
      ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    int actRouteId =
      ((CcInt*)startSection->GetAttribute(SECTION_RID))->GetIntval();
    int sectID =
      ((CcInt*)startSection->GetAttribute(SECTION_SID))->GetIntval();
    res->Put(2*sectID, ShortestPathTreeEntry(0.0,true));
    res->Put(2*sectID + 1, ShortestPathTreeEntry(0.0,false));
    toReach->Remove(Entry<SectionValue>(SectionValue(sectID, true)));
    toReach->Remove(Entry<SectionValue>(SectionValue(sectID, false)));
    double dist = GetPosition() - sectMeas1;
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    pNetwork->GetAdjacentSections ( startSectionTID, false, adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ ) {
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist, dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectionTID, false ),
                      visitedSect, 0);
    }
    adjSectionList.clear();
    dist = sectMeas2 - GetPosition();
    pNetwork->GetAdjacentSections ( startSectionTID, true,
                                  adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ ) {
    DirectedSection actNextSect = adjSectionList[i];
    prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist,dist,
                               actNextSect.GetUpDownFlag(),
                               startSectionTID, true ),
                    visitedSect, 0);
    }
    adjSectionList.clear();
    PQEntryA* actPQEntry = 0;
    while (!prioQ->IsEmpty() && !toReach->isEmpty()) {
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      actRouteId =
        ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      sectID =
        ((CcInt*)actSection->GetAttribute(SECTION_SID))->GetIntval();
      sectPos = 2*sectID;
      if (!actPQEntry->upDownFlag) sectPos++;
      res->Put(sectPos, ShortestPathTreeEntry(actPQEntry->valFromStart,
                                              actPQEntry->upDownFlag));
      toReach->Remove(Entry<SectionValue>(SectionValue(sectID,
                                                      actPQEntry->upDownFlag)));
      dist = actPQEntry->valFromStart + fabs ( sectMeas2 - sectMeas1 );
      pNetwork->GetAdjacentSections ( actPQEntry->sectID,
                                      actPQEntry->upDownFlag,
                                      adjSectionList );
      if (adjSectionList.size() != 0) {
        for ( size_t i = 0; i <adjSectionList.size();i++ ) {
          DirectedSection actNextSect = adjSectionList[i];
          prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                     dist,dist,
                                     actNextSect.GetUpDownFlag(),
                                     actPQEntry->sectID,
                                     actPQEntry->upDownFlag),
                          visitedSect, 0);
        }
      } else {
        if (!actPQEntry->upDownFlag) sectPos--;
        else sectPos++;
        res->Put(sectPos, ShortestPathTreeEntry(actPQEntry->valFromStart +
                                                fabs(sectMeas2 - sectMeas1),
                                                !actPQEntry->upDownFlag));
        toReach->Remove(Entry<SectionValue>(SectionValue(sectID,
                                                    !actPQEntry->upDownFlag)));
      }
      adjSectionList.clear();
      if (actSection != 0) {
        actSection->DeleteIfAllowed();
        actSection = 0;
      }
      if (actPQEntry != 0) {
        delete actPQEntry;
        actPQEntry=0;
      }
    }
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    startSection->DeleteIfAllowed();
  }
}

/*
Almost analogous to shortest path tree but stops computation if all sections
within the maximum distance maxdist are inserted.

*/

void GPoint::Out_Circle(const Network* pNetwork, GLine* res,
                        const double maxdist) const
{
  res->Clear();
  RITreeP *ritree = new RITreeP(0);
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
      GetNetworkId() == pNetwork->GetId() && res != 0)
  {
    res->SetDefined(true);
    res->SetNetworkId(GetNetworkId());
    TupleId startSectionTID = pNetwork->GetTupleIdSectionOnRoute(this);
    PrioQueueA *prioQ = new PrioQueueA ( 0 );
    SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
    int pHelp = -1;
    visitedSect->Insert(SectIDTreeEntry(SectEntry(startSectionTID,
                                                  startSectionTID,
                                                  true, true, -1, 0.0),
                                        -1,-1),
                        pHelp);
    visitedSect->Insert(SectIDTreeEntry(SectEntry(startSectionTID,
                                                  startSectionTID,
                                                  false, false, -1, 0.0),
                                        -1,-1),
                        pHelp);
    Tuple* startSection = pNetwork->GetSection(startSectionTID);
    double sectMeas1 =
      ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    int actRouteId =
      ((CcInt*)startSection->GetAttribute(SECTION_RID))->GetIntval();
    double dist1 = fabs(GetPosition() - sectMeas1);
    double dist2 = fabs(GetPosition() - sectMeas2);
    double length = fabs(sectMeas2 - sectMeas1);
    bool s1 = true;
    bool s2 = true;
    if (length <= maxdist)
    {
      ritree->Insert(actRouteId, sectMeas1, sectMeas2);
    }
    else
    {
      if (dist1 <= maxdist )
        ritree->Insert(actRouteId, sectMeas1, GetPosition());
      else
      {
        ritree->Insert(actRouteId, GetPosition() - maxdist, GetPosition());
        s1 = false;
      }
      if (dist2 <= maxdist)
        ritree->Insert(actRouteId, GetPosition(), sectMeas2);
      else
      {
        ritree->Insert(actRouteId, GetPosition(), GetPosition() + maxdist );
        s2 = false;
      }
    }
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    if (s1)
    {
      pNetwork->GetAdjacentSections ( startSectionTID, false, adjSectionList );
      for ( size_t i = 0;  i < adjSectionList.size(); i++ )
      {
        DirectedSection actNextSect = adjSectionList[i];
        prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist1, dist1,
                                   actNextSect.GetUpDownFlag(),
                                   startSectionTID, false ),
                        visitedSect, 0);
      }
      adjSectionList.clear();
    }
    if (s2)
    {
      pNetwork->GetAdjacentSections ( startSectionTID, true,
                                      adjSectionList );
      for ( size_t i = 0;  i < adjSectionList.size(); i++ ) {
        DirectedSection actNextSect = adjSectionList[i];
        prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist2, dist2,
                                   actNextSect.GetUpDownFlag(),
                                   startSectionTID, true ),
                        visitedSect, 0);
      }
      adjSectionList.clear();
    }
    PQEntryA* actPQEntry = 0;
    double actDist = 0.0;
    bool reached = false;
    while (!prioQ->IsEmpty() && !reached)
    {
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      if (actPQEntry->valFromStart > maxdist)
      {
        reached = true;
      }
      else
      {
        Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
        sectMeas1 =
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
        sectMeas2 =
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
        actRouteId =
          ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
        length = fabs(sectMeas2 - sectMeas1);
        actDist = actPQEntry->valFromStart + length;
        if(actDist <= maxdist)
        {
          ritree->Insert(actRouteId, sectMeas1, sectMeas2);
          pNetwork->GetAdjacentSections ( actPQEntry->sectID,
                                          actPQEntry->upDownFlag,
                                          adjSectionList );
          if (adjSectionList.size() != 0)
          {
            for ( size_t i = 0; i <adjSectionList.size();i++ )
            {
              DirectedSection actNextSect = adjSectionList[i];
              prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                         actDist,actDist,
                                         actNextSect.GetUpDownFlag(),
                                         actPQEntry->sectID,
                                         actPQEntry->upDownFlag),
                              visitedSect, 0);
            }
            adjSectionList.clear();
          }
        }
        else
        {
          if (actPQEntry->upDownFlag)
          {
            ritree->Insert(actRouteId, sectMeas1,
                          sectMeas1 + fabs(maxdist - actPQEntry->valFromStart));
          }
          else
          {
            ritree->Insert(actRouteId,
                           sectMeas2 - fabs(maxdist - actPQEntry->valFromStart),
                           sectMeas2);
          }
        }
        if (actSection != 0)
        {
          actSection->DeleteIfAllowed();
          actSection = 0;
        }
      }
      if (actPQEntry != 0)
      {
        delete actPQEntry;
        actPQEntry=0;
      }
    }
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    startSection->DeleteIfAllowed();
    ritree->TreeToGLine(res,0);
    res->SetSorted(true);
  }
  else
  {
    res->SetDefined(false);
  }
  ritree->Destroy();
  delete ritree;
}


/*
Returns the reverse shortest path tree of the gpoint from all sections of the
network. The distances are stored in an DbArray<double>, where the index
of the Array-Field is two times the section number for up sections and
two times the section number plus one for down sections.

*/

void GPoint::ReverseShortestPathTree(const Network* pNetwork,
                             DbArray<ShortestPathTreeEntry> *res) const
{
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
    GetNetworkId() == pNetwork->GetId() && res != 0) {
    TupleId startSectionTID = pNetwork->GetTupleIdSectionOnRoute(this);
    PrioQueueA *prioQ = new PrioQueueA ( 0 );
    SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
    int pHelp = -1;
    int sectPos = -1;
    Tuple* startSection = pNetwork->GetSection(startSectionTID);
    double sectMeas1 =
      ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    int actRouteId =
      ((CcInt*)startSection->GetAttribute(SECTION_RID))->GetIntval();
    int sectID =
      ((CcInt*)startSection->GetAttribute(SECTION_SID))->GetIntval();
    res->Put(2*sectID, ShortestPathTreeEntry(0.0, true));
    res->Put(2*sectID + 1, ShortestPathTreeEntry(0.0, false));
    double dist = fabs(GetPosition() - sectMeas2);
    visitedSect->Insert(SectIDTreeEntry(
      SectEntry(startSectionTID,
                startSectionTID,
                false, false,
                -1,
                0.0),
                -1,-1),
                pHelp);
    visitedSect->Insert(SectIDTreeEntry(
      SectEntry(startSectionTID,
                startSectionTID,
                true, true,
                -1,
                0.0),
                -1,-1),
                pHelp);
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    pNetwork->GetReverseAdjacentSections (startSectionTID, false,
                                          adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ ){
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist, dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectionTID, false ),
                      visitedSect, 0);
    }
    adjSectionList.clear();
    dist = fabs(sectMeas1 - GetPosition());

    pNetwork->GetReverseAdjacentSections ( startSectionTID, true,
                                    adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ ) {
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist,dist,
                                 actNextSect.GetUpDownFlag(),
                                 startSectionTID, true ),
                      visitedSect, 0);
    }
    adjSectionList.clear();
    PQEntryA* actPQEntry = 0;
    while (!prioQ->IsEmpty()) {
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      actRouteId =
        ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      sectID =
        ((CcInt*)actSection->GetAttribute(SECTION_SID))->GetIntval();
      sectPos = 2*sectID;
      if (!actPQEntry->upDownFlag)
        sectPos++;
      res->Put(sectPos, ShortestPathTreeEntry(actPQEntry->valFromStart,
                                              actPQEntry->upDownFlag));

      dist = actPQEntry->valFromStart + fabs ( sectMeas2 - sectMeas1 );
      pNetwork->GetReverseAdjacentSections ( actPQEntry->sectID,
                                      actPQEntry->upDownFlag,
                                      adjSectionList );
      if (adjSectionList.size() != 0) {
        for ( size_t i = 0; i <adjSectionList.size();i++ ) {
          DirectedSection actNextSect = adjSectionList[i];
          prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                     dist,dist,
                                     actNextSect.GetUpDownFlag(),
                                     actPQEntry->sectID,
                                     actPQEntry->upDownFlag),
                          visitedSect, 0);
        }
      } else {
        if (!actPQEntry->upDownFlag) sectPos--;
        else sectPos++;
        res->Put(sectPos,
                 ShortestPathTreeEntry(actPQEntry->valFromStart +
                                          fabs(sectMeas2 - sectMeas1),
                                       !actPQEntry->upDownFlag));

      }
      adjSectionList.clear();
      if (actSection != 0) {
        actSection->DeleteIfAllowed();
        actSection = 0;
      }
      if (actPQEntry != 0) {
        delete actPQEntry;
        actPQEntry=0;
      }
    }
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    startSection->DeleteIfAllowed();
  }
}

/*
Almost analogous to reverse shortest path tree but stops computation if all
sections of ~toReach~ are inserted.

*/

void GPoint::ReverseShortestPathTree(const Network* pNetwork,
                           DbArray<ShortestPathTreeEntry> *res,
                           SortedTree<Entry<SectionValue> > *toReach) const{
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
      GetNetworkId() == pNetwork->GetId() && res != 0) {
    TupleId startSectionTID = pNetwork->GetTupleIdSectionOnRoute(this);
    PrioQueueA *prioQ = new PrioQueueA ( 0 );
    SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
    int pHelp = -1;
    int sectPos = -1;
    Tuple* startSection = pNetwork->GetSection(startSectionTID);
    double sectMeas1 =
      ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    int actRouteId =
      ((CcInt*)startSection->GetAttribute(SECTION_RID))->GetIntval();
    int sectID =
      ((CcInt*)startSection->GetAttribute(SECTION_SID))->GetIntval();
    res->Put(2*sectID, ShortestPathTreeEntry(0.0, true));
    res->Put(2*sectID + 1, ShortestPathTreeEntry(0.0, false));
    toReach->Remove(Entry<SectionValue>(SectionValue(sectID, true)));
    toReach->Remove(Entry<SectionValue>(SectionValue(sectID, false)));
    double dist = fabs(GetPosition() - sectMeas2);
    visitedSect->Insert(SectIDTreeEntry(
       SectEntry(startSectionTID,
                 startSectionTID,
                 false, false,
                 -1,
                 0.0),
                 -1,-1),
                 pHelp);
    visitedSect->Insert(SectIDTreeEntry(
       SectEntry(startSectionTID,
                 startSectionTID,
                 true, true,
                 -1,
                 0.0),
                 -1,-1),
                 pHelp);
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    pNetwork->GetReverseAdjacentSections (startSectionTID, false,
                                           adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ ){
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist, dist,
                                  actNextSect.GetUpDownFlag(),
                                  startSectionTID, false ),
                       visitedSect, 0);
    }
    adjSectionList.clear();
    dist = fabs(sectMeas1 - GetPosition());

    pNetwork->GetReverseAdjacentSections ( startSectionTID, true,
                                           adjSectionList );
    for ( size_t i = 0;  i < adjSectionList.size(); i++ ) {
      DirectedSection actNextSect = adjSectionList[i];
      prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist,dist,
                                  actNextSect.GetUpDownFlag(),
                                  startSectionTID, true ),
                      visitedSect, 0);
    }
    adjSectionList.clear();
    PQEntryA* actPQEntry = 0;
    while (!prioQ->IsEmpty() && !toReach->isEmpty()) {
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      actRouteId =
        ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      sectID =
        ((CcInt*)actSection->GetAttribute(SECTION_SID))->GetIntval();
      sectPos = 2*sectID;
      if (!actPQEntry->upDownFlag) sectPos++;
      res->Put(sectPos, ShortestPathTreeEntry(actPQEntry->valFromStart,
                                              actPQEntry->upDownFlag));
      toReach->Remove(Entry<SectionValue>(SectionValue(sectID,
                                                     actPQEntry->upDownFlag)));
      dist = actPQEntry->valFromStart + fabs ( sectMeas2 - sectMeas1 );
      pNetwork->GetReverseAdjacentSections ( actPQEntry->sectID,
                                              actPQEntry->upDownFlag,
                                              adjSectionList );
      if (adjSectionList.size() != 0) {
        for ( size_t i = 0; i <adjSectionList.size();i++ ) {
          DirectedSection actNextSect = adjSectionList[i];
          prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                      dist,dist,
                                      actNextSect.GetUpDownFlag(),
                                      actPQEntry->sectID,
                                      actPQEntry->upDownFlag),
                          visitedSect, 0);
        }
      } else {
        if (!actPQEntry->upDownFlag) sectPos--;
        else sectPos++;
        res->Put(sectPos,
                 ShortestPathTreeEntry(actPQEntry->valFromStart +
                                                 fabs(sectMeas2 - sectMeas1),
                                       !actPQEntry->upDownFlag));
        toReach->Remove(Entry<SectionValue>(SectionValue(sectID,
                                                    !actPQEntry->upDownFlag)));
       }
       adjSectionList.clear();
       if (actSection != 0) {
         actSection->DeleteIfAllowed();
         actSection = 0;
       }
       if (actPQEntry != 0) {
         delete actPQEntry;
         actPQEntry=0;
       }
    }
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    startSection->DeleteIfAllowed();
  }
}

/*
Almost analogous to reverse shortest path tree but stops computation if all
sections from which the gpoint can be reached within maxdist are inserted.

*/

void GPoint::In_Circle(const Network* pNetwork, GLine* res,
                       const double maxdist) const
{
  res->Clear();
  RITreeP *ritree = new RITreeP(0);
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
    GetNetworkId() == pNetwork->GetId() && res != 0)
  {
    res->SetDefined(true);
    res->SetNetworkId(GetNetworkId());
    TupleId startSectionTID = pNetwork->GetTupleIdSectionOnRoute(this);
    PrioQueueA *prioQ = new PrioQueueA ( 0 );
    SectIDTreeP *visitedSect =  new SectIDTreeP ( 0);
    int pHelp = -1;
    Tuple* startSection = pNetwork->GetSection(startSectionTID);
    double sectMeas1 =
      ((CcReal* )startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
    double sectMeas2 =
      ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
    int actRouteId =
      ((CcInt*)startSection->GetAttribute(SECTION_RID))->GetIntval();
    double dist1 = fabs(GetPosition() - sectMeas1);
    double dist2 = fabs(GetPosition() - sectMeas2);
    double length = fabs(sectMeas2 - sectMeas1);
    bool s1 = true;
    bool s2 = true;
    if (length <= maxdist)
    {
      ritree->Insert(actRouteId, sectMeas1, sectMeas2);
    }
    else
    {
      if (dist1 <= maxdist )
        ritree->Insert(actRouteId, sectMeas1, GetPosition());
      else
      {
        ritree->Insert(actRouteId, GetPosition() - maxdist, GetPosition());
        s1 = false;
      }
      if (dist2 <= maxdist)
        ritree->Insert(actRouteId, GetPosition(), sectMeas2);
      else
      {
        ritree->Insert(actRouteId, GetPosition(), GetPosition() + maxdist );
        s2 = false;
      }
    }
    visitedSect->Insert(SectIDTreeEntry(SectEntry(startSectionTID,
                                                  startSectionTID,
                                                  false, false, -1, 0.0),
                                        -1,-1),
                        pHelp);
    visitedSect->Insert(SectIDTreeEntry(SectEntry(startSectionTID,
                                                  startSectionTID,
                                                  true, true, -1, 0.0),
                                        -1,-1),
                        pHelp);
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    if (s1)
    {
      pNetwork->GetReverseAdjacentSections (startSectionTID, false,
                                            adjSectionList );
      for ( size_t i = 0;  i < adjSectionList.size(); i++ )
      {
        DirectedSection actNextSect = adjSectionList[i];
        prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist2, dist2,
                                   actNextSect.GetUpDownFlag(),
                                   startSectionTID, false ),
                        visitedSect, 0);
      }
      adjSectionList.clear();
    }
    if (s2)
    {
      pNetwork->GetReverseAdjacentSections ( startSectionTID, true,
                                             adjSectionList );
      for ( size_t i = 0;  i < adjSectionList.size(); i++ )
      {
        DirectedSection actNextSect = adjSectionList[i];
        prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(), dist1, dist1,
                                   actNextSect.GetUpDownFlag(),
                                   startSectionTID, true ),
                        visitedSect, 0);
      }
      adjSectionList.clear();
    }
    PQEntryA* actPQEntry = 0;
    double actDist = 0.0;
    bool reached = false;
    while (!prioQ->IsEmpty() && !reached)
    {
      actPQEntry = prioQ->GetAndDeleteMin(visitedSect);
      if (actPQEntry->valFromStart > maxdist)
      {
        reached = true;
      }
      else
      {
        Tuple *actSection = pNetwork->GetSection ( actPQEntry->sectID );
        sectMeas1 =
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
        sectMeas2 =
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
        actRouteId =
          ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
        length = fabs(sectMeas2 - sectMeas1);
        actDist = actPQEntry->valFromStart + length;
        if(actDist <= maxdist)
        {
          ritree->Insert(actRouteId, sectMeas1, sectMeas2);
          pNetwork->GetReverseAdjacentSections ( actPQEntry->sectID,
                                          actPQEntry->upDownFlag,
                                          adjSectionList );
          if (adjSectionList.size() != 0)
          {
            for ( size_t i = 0; i <adjSectionList.size();i++ )
            {
              DirectedSection actNextSect = adjSectionList[i];
              prioQ->Insert ( PQEntryA ( actNextSect.GetSectionTid(),
                                         actDist,actDist,
                                         actNextSect.GetUpDownFlag(),
                                         actPQEntry->sectID,
                                         actPQEntry->upDownFlag),
                              visitedSect, 0);
            }
            adjSectionList.clear();
          }
        }
        else
        {
          if (actPQEntry->upDownFlag)
          {
            ritree->Insert(actRouteId,
                           sectMeas2 - fabs(maxdist - actPQEntry->valFromStart),
                           sectMeas2);
          }
          else
          {
            ritree->Insert(actRouteId, sectMeas1,
                          sectMeas1 + fabs(maxdist - actPQEntry->valFromStart));
          }
        }
        if (actSection != 0)
        {
          actSection->DeleteIfAllowed();
          actSection = 0;
        }
      }
      if (actPQEntry != 0)
      {
        delete actPQEntry;
        actPQEntry=0;
      }
    }
    ritree->TreeToGLine(res,0);
    res->SetSorted(true);
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    startSection->DeleteIfAllowed();
  }
  else
  {
    res->SetDefined(false);
  }
  ritree->Destroy();
  delete ritree;
}

/*
Returns the route intervals whithin distance maxdist from ~GPoint~ ignoring
connectivity in the junctions.

*/

void GPoint::Circle(const Network* pNetwork, GLine *res,
                    const double maxdist) const
{
  res->Clear();
  if (IsDefined() && pNetwork != 0 && pNetwork->IsDefined() &&
    GetNetworkId() == pNetwork->GetId() && res != 0)
  {
    res->SetNetworkId(GetNetworkId());
    res->SetDefined(true);
    GLine *outcircle = new GLine(0);
    Out_Circle(pNetwork, outcircle, maxdist);
    GLine *incircle = new GLine(0);
    In_Circle(pNetwork, incircle, maxdist);
    incircle->Uniongl(outcircle, res);
    outcircle->DeleteIfAllowed();
    incircle->DeleteIfAllowed();
    res->SetSorted(true);
  }
  else res->SetDefined(false);
}

/*
Returns the x,y point represented by gpoint.

*/

void GPoint::ToPoint ( Point *&res )const
{
  //Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(), netList );
  if ( pNetwork != 0 )
  {
    pNetwork->GetPointOnRoute ( this, res );
  }
  else
  {
    res->SetDefined ( false );
  }
  NetworkManager::CloseNetwork ( pNetwork );
};

Point* GPoint::ToPoint() const
{
  Point *res = new Point ( true );
  //Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(), netList );
  if ( pNetwork != 0 ) pNetwork->GetPointOnRoute ( this, res );
  NetworkManager::CloseNetwork ( pNetwork );
  return res;
};

Point* GPoint::ToPoint ( const Network*& pNetwork ) const
{
  Point *res = new Point ( true );
  if ( pNetwork != 0 ) pNetwork->GetPointOnRoute ( this, res );
  return res;
};

/*
Returns the bounding GPoints of the given GLine.

*/

void GLine::GetBGP(GPoints* result)const
{
  result->Clear();
  if (!IsDefined() || NoOfComponents()==0)
  {
    result->SetDefined(false);
  }
  else
  {
    Network* pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
    GetBGP(pNetwork, result);
    NetworkManager::CloseNetwork(pNetwork);
  }
}

void GLine::GetBGP(const Network* pNetwork, GPoints* result)const
{
  result->Clear();
  if ( !IsDefined() || NoOfComponents() == 0 )
  {
    result->SetDefined(false);
  }
  else
  {
  RouteInterval ri;
  vector<SectTreeEntry> actSections;
  actSections.clear();
  for (int i = 0; i < Size(); i++)
  {
    Get(i, ri);
    pNetwork->GetSectionsOfRouteInterval(&ri, actSections);
    SectTreeEntry nEntry;
    for (size_t a = 0; a < actSections.size() ; a++)
    {
      nEntry = actSections[a];
      vector<DirectedSection> adjSect;
      adjSect.clear();
      if (!nEntry.startbool)
      {
        result->MergeAdd(GPoint(true, GetNetworkId(), nEntry.rid,
                                  nEntry.start),
                           pNetwork);
      }
      else
      {
        pNetwork->GetAdjacentSections(nEntry.secttid, false, adjSect);
        for (size_t b = 0; b < adjSect.size(); b++)
        {
          DirectedSection actDirSection = adjSect[b];
          if (actDirSection.GetSectionTid() != nEntry.secttid)
          {
            Tuple* actSection =
              pNetwork->GetSection(actDirSection.GetSectionTid());
            RouteInterval* pri =
              new RouteInterval(
              ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval(),
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval() +
              1.0*pNetwork->GetScalefactor(),
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval() -
              1.0*pNetwork->GetScalefactor());
            actSection->DeleteIfAllowed();
            actSection = 0;
            if (!Intersects(pri, pNetwork->GetScalefactor()*0.01))
            {
              result->MergeAdd(GPoint(true, GetNetworkId(), nEntry.rid,
                                      nEntry.start),
                              pNetwork);
              adjSect.clear();
              delete pri;
              break;
            }
            delete pri;
            pri = 0;
          }
        }
        adjSect.clear();
      }
      if (!nEntry.endbool)
      {
        result->MergeAdd(GPoint(true, GetNetworkId(), nEntry.rid,
                                  nEntry.end),
                           pNetwork);
      }
      else
      {
        pNetwork->GetAdjacentSections(nEntry.secttid, true, adjSect);
        for (size_t b = 0; b < adjSect.size(); b++)
        {
          DirectedSection actDirSection = adjSect[b];
          if (actDirSection.GetSectionTid() != nEntry.secttid)
          {
            Tuple* actSection =
              pNetwork->GetSection(actDirSection.GetSectionTid());
            RouteInterval* pri1 =
              new RouteInterval(
                ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval(),
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval() +
              1.0 *pNetwork->GetScalefactor(),
          ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval() -
              1.0*pNetwork->GetScalefactor());
            actSection->DeleteIfAllowed();
            actSection = 0;
            if (!Intersects(pri1, pNetwork->GetScalefactor()*0.01))
            {
              result->MergeAdd(GPoint(true, GetNetworkId(), nEntry.rid,
                                      nEntry.end),
                              pNetwork);
              adjSect.clear();
              delete pri1;
              break;
            }
            delete pri1;
            pri1 = 0;
          }
        }
        adjSect.clear();
      }
    }
    actSections.clear();
    }
    result->SetDefined(true);
  }
};

bool GLine::Contains(const RouteInterval* ri, const double tolerance)const
{
  if (IsSorted())
  {
    return searchRouteInterval(ri, this,tolerance, 0, NoOfComponents()-1);
  }
  else
  {
    RouteInterval rigl;
    for (int i = 0; i < NoOfComponents(); i++)
    {
      Get(i,rigl);
      if (rigl.Contains(ri, tolerance)) return true;
    }
  }
  return false;
};


/*
Secondo Type Constructor for class ~GPoint~

*/

struct gpointInfo:ConstructorInfo{
  gpointInfo():ConstructorInfo(){
    name = GPoint::BasicType();
    signature = "-> DATA";
    typeExample = GPoint::BasicType();
    listRep = "(<netId> <routeId> <pos> <side>)";
    valueExample = "(1 2 5.4 1)";
    remarks = "Single position in a network.";
  }
};

struct gpointFunctions: ConstructorFunctions<GPoint>{
  gpointFunctions(){
    in = GPoint::InGPoint;
    out = GPoint::OutGPoint;
    create = GPoint::CreateGPoint;
    deletion = GPoint::DeleteGPoint;
    open = OpenAttribute<GPoint>;
    save = SaveAttribute<GPoint>;
    close = GPoint::CloseGPoint;
    cast = GPoint::CastGPoint;
    clone = GPoint::CloneGPoint;
    sizeOf = GPoint::SizeOfGPoint;
    kindCheck = GPoint::CheckGPoint;
  }
};

gpointInfo gpi;
gpointFunctions gf;
TypeConstructor gpointTC(gpi,gf);

/*
5 Class GPoints implemented by Jianqiu Xu

*/

string edistjoinpointlist = "(rel(tuple((pid int)(p point))))";
enum edistjoinpointlistrelation {POINTSID = 0,POINTSOBJECT};

GPoints::GPoints():Attribute()
{}

GPoints::GPoints(const bool def):Attribute(def),m_xGPoints(0)
{}

GPoints::GPoints ( const int in_iSize ) :Attribute(true),m_xGPoints ( in_iSize )
{}

GPoints::GPoints (const GPoints* in_xOther ) :
    Attribute(in_xOther->IsDefined()),
    m_xGPoints ( in_xOther->Size() )
{
  GPoint pCurrentInterval;
  for ( int i = 0; i < in_xOther->m_xGPoints.Size(); i++ )
  {
    // Get next Interval
    in_xOther->m_xGPoints.Get ( i, pCurrentInterval );
    m_xGPoints.Append ( pCurrentInterval );
  }
  SetDefined(in_xOther->IsDefined());
  TrimToSize();
}

void GPoints::Clear()
{
  m_xGPoints.clean();
  SetDefined(true);
}

bool GPoints::IsEmpty() const
{
  return m_xGPoints.Size() == 0;
}
ostream& GPoints::Print ( ostream& os ) const
{
  if (IsDefined())
  {
    for ( int i = 0;i < m_xGPoints.Size();i++ )
    {
      GPoint pGP;
      m_xGPoints.Get ( i,pGP );
      os<<"GPoint "<< i <<": ";
      pGP.Print(os);
      os << endl;
    }
  }
  else os << "not defined." << endl;
  return os;
}

GPoints& GPoints::operator= ( const GPoints& gps )
{
  SetDefined(gps.IsDefined());
  m_xGPoints.copyFrom(gps.m_xGPoints);
  return *this;
}

int GPoints::NumOfFLOBs() const
{
  return 1;
}

Flob* GPoints::GetFLOB ( const int i )
{
  assert ( i >= 0 && i < NumOfFLOBs() );
  return &m_xGPoints;
}

size_t GPoints::Sizeof() const
{
  return sizeof ( *this );
}

int GPoints::SizeOf()
{
  return sizeof ( GPoints );
}
int GPoints::Size() const
{
  return m_xGPoints.Size();
}

GPoints& GPoints::operator+= ( const GPoint& gp )
{
  m_xGPoints.Append ( gp );
  TrimToSize();
  return *this;
}

void GPoints::MergeAdd(const GPoint gp, const Network* pNetwork)
{
  if (!Contains(gp,pNetwork)) m_xGPoints.Append(gp);
}

bool GPoints::Contains(const GPoint gp)const
{
  Network* pNetwork = NetworkManager::GetNetworkNew(gp.GetNetworkId(),netList);
  bool result = Contains(gp,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}

bool GPoints::Contains(const GPoint gp, const Network* pNetwork) const
{
  GPointList *gpl = new GPointList(&gp,pNetwork);
  GPoint gptest = gpl->NextGPoint();
  while (gptest.IsDefined())
  {
    GPoint gpold;
    for (int i = 0; i < m_xGPoints.Size(); i++)
    {
      m_xGPoints.Get(i,gpold);
      if (gpold == gptest)
      {
        gpl->Destroy();
        delete gpl;
        return true;
      }
    }
    gptest = gpl->NextGPoint();
  }
  gpl->Destroy();
  delete gpl;
  return false;
}

int GPoints::GetNetworkId() const
{
  if(!IsDefined() || Size() < 1) return -1;
  GPoint gp;
  Get(0,gp);
  return gp.GetNetworkId();
}

bool GPoints::Inside(const GPoint gp)const
{
  if (!IsDefined() || Size() < 1) return false;
  GPoint gp1;
  for (int i = 0; i < Size(); i++)
  {
    Get(0,gp1);
    if (gp1 == gp) return true;
  }
  return false;
}

void GPoints::GetSectionTupleIds(DbArray<GPointsSections>* gpSections,
                                 const Network *pNetwork)const
{
  GPoint gp;
  for (int i = 0 ; i < Size() ; i++)
  {
    Get(i,gp);
    Point *pPoint = gp.ToPoint(pNetwork);
    gpSections->Append(GPointsSections(gp,
                                       pNetwork->GetTupleIdSectionOnRoute(&gp),
                                       *pPoint));
    pPoint->DeleteIfAllowed();
  }
}


bool GPoints::Intersects(const GPoints* bgp)const
{
  if(!IsDefined() || !bgp->IsDefined() || Size() < 1 || bgp->Size() < 1)
    return false;
  GPoint gp;
  Get(0,gp);
  Network *pNetwork =
    NetworkManager::GetNetworkNew(gp.GetNetworkId(),netList);
  bool result = Intersects(bgp, pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}

bool GPoints::Intersects(const GPoints* bgp, const Network* pNetwork)const
{
  if(!IsDefined() || !bgp->IsDefined() || Size() < 1 || bgp->Size() < 1)
    return false;
  GPoint gp;
  for (int i = 0; i < Size(); i++)
  {
    Get(i,gp);
    if (bgp->Contains(gp,pNetwork))
    {
      return true;
    }
  }
  return false;
}

bool GPoints::ShortestPathAStar(const GPoints* bgp, GLine* res,
                           DbArray<TupleId>* touchedSects)const
{
  res->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if(!IsDefined() || !bgp->IsDefined() || Size() < 1 || bgp->Size() < 1)
  {
    res->SetDefined(false);
    return false;
  }
  Network *pNetwork =
    NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  bool result = ShortestPathAStar(bgp, res, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}


bool GPoints::ShortestPathAStar(const GPoints* bgp, GLine* res,
                                const Network* pNetwork,
                           DbArray<TupleId>* touchedSects)const
{
  res->Clear();
  res->SetDefined(false);
  if (touchedSects != 0) touchedSects->clean();
  if(!IsDefined() || !bgp->IsDefined() || Size() < 1 || bgp->Size() < 1 ||
     GetNetworkId() != bgp->GetNetworkId() ||
     GetNetworkId() != pNetwork->GetId())
  {
    return false;
  }
  if (Intersects(bgp,pNetwork))
  {
    res->SetDefined(true);
    res->SetNetworkId(pNetwork->GetId());
    return true;
  }
  DbArray<GPointsSections>* startSections = new DbArray<GPointsSections>(0);
  DbArray<GPointsSections>* endSections = new DbArray<GPointsSections>(0);
  GetSectionTupleIds(startSections,pNetwork);
  bgp->GetSectionTupleIds(endSections,pNetwork);
  if (startSections->Size() < 1 || endSections->Size() < 1)
  {
    startSections->Destroy();
    endSections->Destroy();
    delete startSections;
    delete endSections;
    return false;
  }
  Points* endPoints = new Points(endSections->Size());
  endPoints->StartBulkLoad();
  GPointsSections gps;
  for (int i = 0; i < endSections->Size(); i++)
  {
    endSections->Get(i,gps);
    *endPoints += gps.GetPoint();
  }
  endPoints->EndBulkLoad();
  /*
  Initialize PriorityQueue

  */
  GPointsSections source, target;
  PrioQueueA* prioQ = new PrioQueueA(0);
  SectIDTreeP *visitedSect = new SectIDTreeP (0);
  double dist = 0.0;
  int pHelp = -1;
  double sectMeas1 = 0.0;
  double sectMeas2 = 0.0;
  double startRI = 0.0;
  double endRI = 0.0;
  int actRouteId = -1;
  int lastSectPos = -1;
  int firstSectPos = -1;
  Tuple *actSection = 0;
  for (int i = 0; i < startSections->Size(); i++)
  {
    startSections->Get(i,source);
    visitedSect->Insert(SectIDTreeEntry(
                          SectEntry(source.GetTid(),
                                    (TupleId) numeric_limits<long>::max(),
                                    true, true,
                                    -1,
                                    0.0),
                          -1,-1),
                          pHelp);
    visitedSect->Insert(SectIDTreeEntry(
                          SectEntry(source.GetTid(),
                                    (TupleId) numeric_limits<long>::max(),
                                    false, false,
                                    -1,
                                    0.0),
                          -1,-1),
                          pHelp);
    if (touchedSects != 0) touchedSects->Append(source.GetTid());
    if (IsLastSection(source.GetTid(), endSections, lastSectPos))
    {
      endSections->Get(lastSectPos,target);
      if (source.GetGP().GetRouteId() == target.GetGP().GetRouteId())
      {
        RouteInterval* ri = new RouteInterval(source.GetGP().GetRouteId(),
                                              source.GetGP().GetPosition(),
                                              target.GetGP().GetPosition());
        double length = ri->Length();
        i++;
        while (i < startSections->Size())
        {
          startSections->Get(i,source);
          visitedSect->Insert(SectIDTreeEntry(
                          SectEntry(source.GetTid(),
                                    (TupleId) numeric_limits<long>::max(),
                                    true,true,
                                    -1,
                                    0.0),
                          -1,-1),
                          pHelp);
          visitedSect->Insert(SectIDTreeEntry(
                          SectEntry(source.GetTid(),
                                    (TupleId) numeric_limits<long>::max(),
                                    false,false,
                                    -1,
                                    0.0),
                          -1,-1),
                          pHelp);
          if (touchedSects != 0) touchedSects->Append(source.GetTid());
          if (IsLastSection(source.GetTid(), endSections, lastSectPos))
          {
            endSections->Get(lastSectPos,target);
            if (source.GetGP().GetRouteId() == target.GetGP().GetRouteId() &&
                fabs(source.GetGP().GetPosition() -
                        target.GetGP().GetPosition())
                    < length)
            {
              ri->SetStartPos(source.GetGP().GetPosition());
              ri->SetEndPos(target.GetGP().GetPosition());
              ri->SetRouteId(source.GetGP().GetRouteId());
              length = ri->Length();
            }
          }
          i++;
        }
        res->AddRouteInterval(*ri);
        delete ri;
        startSections->Destroy();
        endSections->Destroy();
        delete startSections;
        delete endSections;
        endPoints->Destroy();
        delete endPoints;
        prioQ->Destroy();
        delete prioQ;
        visitedSect->Destroy();
        delete visitedSect;
        res->SetDefined(true);
        return true;
      }
    }
  }
  for (int i = 0; i < startSections->Size(); i++)
  {
    startSections->Get(i,source);
    dist =  source.GetGP().GetPosition();
    InsertAdjacentSections(source.GetTid(), Up, true, dist, endPoints, pNetwork,
                           prioQ, visitedSect, touchedSects);
    dist =  source.GetGP().GetPosition();
    InsertAdjacentSections(source.GetTid(), Down, true, dist, endPoints,
                           pNetwork, prioQ, visitedSect, touchedSects);
  }
/*
Use priorityQueue to find shortestPath.

*/

  PQEntryA *actPQEntry = 0;
  bool found = false;
  while (!prioQ->IsEmpty() && !found )
  {
    actPQEntry = prioQ->GetAndDeleteMin ( visitedSect );
    actSection = pNetwork->GetSection ( actPQEntry->sectID );
    if (!IsLastSection(actPQEntry->sectID, endSections, lastSectPos))
    {
      dist = actPQEntry->valFromStart;
      if (actPQEntry->upDownFlag)
        InsertAdjacentSections(actPQEntry->sectID, Up, false, dist, endPoints,
                               pNetwork, prioQ, visitedSect, touchedSects);
      else
        InsertAdjacentSections(actPQEntry->sectID, Down, false, dist, endPoints,
                               pNetwork, prioQ, visitedSect, touchedSects);
      delete actPQEntry;
      actPQEntry = 0;
      actSection->DeleteIfAllowed();
      actSection = 0;
    }
    else
    {
  /*
  Shortest Path nearly found.

  It might exist another endSection in the priority queue
  with the same distance from the start section, where the distance of the
  gpoint is nearer to the end of the last section. Therefore we have to check
  the values of the priorty queue with the same weight. To find the correct
  end Section.

  Compute last route interval and resulting gline.

  */
      found = true;
      endSections->Get(lastSectPos, target);
      endRI = target.GetGP().GetPosition();
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      actRouteId =
        ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      if (bgp->Contains(GPoint(true,GetNetworkId(),actRouteId,sectMeas1),
                        pNetwork) &&
          bgp->Contains(GPoint(true,GetNetworkId(),actRouteId,sectMeas2),
                        pNetwork))
        startRI = endRI;
      else
      {
        if ( actPQEntry->upDownFlag)
          startRI = sectMeas1;
        else
          startRI = sectMeas2;
      }
      double minPathLength = actPQEntry->valFromStart + fabs(endRI-startRI);
      bool btestended = false;
      while (!btestended && !prioQ->IsEmpty())
      {
        PQEntryA* pqtest = prioQ->GetAndDeleteMin(visitedSect);
        if (pqtest->prioval < minPathLength)
        {
          int posLastTest = -1;
          Tuple *actTestSection = pNetwork->GetSection(pqtest->sectID);
          int actTestRouteId =
            ((CcInt*)actTestSection->GetAttribute(SECTION_RID))->GetIntval();
          if (!IsLastSection(pqtest->sectID, endSections, posLastTest))
          {
            dist = pqtest->valFromStart;
            if (pqtest->upDownFlag)
              InsertAdjacentSections(pqtest->sectID, Up, false, dist, endPoints,
                                     pNetwork, prioQ, visitedSect,
                                     touchedSects);
            else
              InsertAdjacentSections(pqtest->sectID, Down, false, dist,
                                     endPoints, pNetwork, prioQ, visitedSect,
                                     touchedSects);
            delete pqtest;
            pqtest = 0;
            actTestSection->DeleteIfAllowed();
            actTestSection = 0;
          }
          else
          {
            GPointsSections testgps;
            endSections->Get(posLastTest,testgps);
            double endTestRI = testgps.GetGP().GetPosition();
            double testSectMeas1 =
              ((CcReal*)actTestSection->GetAttribute(SECTION_MEAS1))
                ->GetRealval();
            double testSectMeas2 =
              ((CcReal*)actTestSection->GetAttribute(SECTION_MEAS2))
                ->GetRealval();
            double startTestRI = 0.0;
            if (bgp->Contains(GPoint(true,GetNetworkId(),actTestRouteId,
                                     testSectMeas1), pNetwork) &&
                bgp->Contains(GPoint(true,GetNetworkId(),actTestRouteId,
                                     testSectMeas2),pNetwork))
              startTestRI = endTestRI;
            else
            {
              if ( pqtest->upDownFlag)
                startTestRI = testSectMeas1;
              else
                startTestRI = testSectMeas2;
            }
            double actPathLength =
              pqtest->valFromStart + fabs(endTestRI - startTestRI);
            if (actPathLength < minPathLength)
            {
              endRI = endTestRI;
              startRI = startTestRI;
              lastSectPos = posLastTest;
              minPathLength = actPathLength;
              actRouteId = actTestRouteId;
              delete actPQEntry;
              actPQEntry = pqtest;
              actSection->DeleteIfAllowed();
              actSection = actTestSection;
            }
          }
          if (actSection != actTestSection && actTestSection != 0)
          {
            actTestSection->DeleteIfAllowed();
            actTestSection = 0;
          }
        }
        else
          btestended = true;
        if (pqtest != 0 && pqtest != actPQEntry)
        {
          delete pqtest;
          pqtest = 0;
        }
      }
    }
  }
  /*
  Get the sections used for shortest path and write them in right
  order (from start to end gpoint) in the resulting gline. Because Dijkstra
  gives the sections from end to start, we have to put the result sections
  on a stack to turn them in right order.

  */
  if (!found)
  {
    startSections->Destroy();
    endSections->Destroy();
    delete startSections;
    delete endSections;
    endPoints->Destroy();
    delete endPoints;
    prioQ->Destroy();
    delete prioQ;
    visitedSect->Destroy();
    delete visitedSect;
    res->SetDefined(false);
    return false;
  }
  RIStackP *riStack = new RIStackP(0);
  if (startRI != endRI) riStack->Push( actRouteId, startRI, endRI );
  TupleId lastSectId = actPQEntry->sectID;
  int pElem = visitedSect->Find ( actPQEntry->beforeSectID,
                                  actPQEntry->beforeUpDown );
  bool end = false;
  while ( !end && pElem > -1 && pElem < visitedSect->fFree)
  {
    SectEntry actSectEntry = visitedSect->GetTreeEntry(pElem).GetEntry();
    //GetSection
    TupleId actTupleId = actSectEntry.GetSectId();
    bool upDown = actSectEntry.GetUpDownFlag();
    if (actSection != 0) actSection->DeleteIfAllowed();
    actSection = pNetwork->GetSection(actTupleId);
    if (actSection != 0)
    {
      actRouteId =
        (( CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
      sectMeas1 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
      sectMeas2 =
        ((CcReal*)actSection->GetAttribute(SECTION_MEAS2))->GetRealval();
      actSection->DeleteIfAllowed();
      actSection = 0;
      if(!IsFirstSection(actTupleId, startSections, firstSectPos))
      {
        if ( actSectEntry.GetUpDownFlag())
          riStack->Push ( actRouteId, sectMeas1, sectMeas2);
        else
          riStack->Push ( actRouteId, sectMeas2, sectMeas1);
        lastSectId = actTupleId;
        pElem =
          visitedSect->Find(actSectEntry.GetBeforeSectId(),
                            actSectEntry.GetBeforeSectUpDown());
      }
      else
      {
        end = true;
        startSections->Get(firstSectPos,source);
        if (!(Contains(GPoint(pNetwork->GetId(), actRouteId, sectMeas1, 2),
                       pNetwork) &&
              Contains(GPoint(pNetwork->GetId(), actRouteId, sectMeas2, 2),
                       pNetwork)))
        {
          if (upDown)
            riStack->Push ( actRouteId, source.GetGP().GetPosition(),
                                sectMeas2);
          else
            riStack->Push ( actRouteId, source.GetGP().GetPosition(),
                                  sectMeas1);
        }
      }
    }
    else
    {
      actSection->DeleteIfAllowed();
      startSections->Destroy();
      endSections->Destroy();
      delete startSections;
      delete endSections;
      endPoints->Destroy();
      delete endPoints;
      prioQ->Destroy();
      delete prioQ;
      visitedSect->Destroy();
      delete visitedSect;
      res->SetDefined(false);
      return false;
    }
  }
  // Cleanup and return result
  riStack->StackToGLine ( res, GetNetworkId() );
  riStack->Destroy();
  delete riStack;
  if ( actSection )
  {
    actSection->DeleteIfAllowed();
    actSection = 0;
  }
  if ( actPQEntry != 0)
  {
    delete actPQEntry;
    actPQEntry = 0;
  }
  visitedSect->Destroy();
  delete visitedSect;
  prioQ->Destroy();
  delete prioQ;
  startSections->Destroy();
  endSections->Destroy();
  delete startSections;
  delete endSections;
  endPoints->Destroy();
  delete endPoints;
  res->SetSorted ( false );
  res->SetDefined ( found);
  res->TrimToSize();
  return true;
}

bool GPoints::ShortestPathAStar(const GPoint* gp, GLine* res,
                           DbArray<TupleId>* touchedSects)const
{
  res->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if(!IsDefined() || !gp->IsDefined() || Size() < 1 )
  {
    res->SetDefined(false);
    return false;
  }
  Network *pNetwork =
    NetworkManager::GetNetworkNew(gp->GetNetworkId(),netList);
  bool result = ShortestPathAStar(gp, res, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}

bool GPoints::ShortestPathAStar(const GPoint* gp, GLine* res,
                                const Network* pNetwork,
                           DbArray<TupleId>* touchedSects)const
{
  res->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if ( GetNetworkId() != gp->GetNetworkId() )
  {
    res->SetDefined(false);
    return false;
  }
  res->SetNetworkId(pNetwork->GetId());
  if (Contains(*gp))
  {
    res->SetDefined(true);
    return true;
  }
  GPoints *bGPgl2 = new GPoints(true);
  bGPgl2->MergeAdd(*gp,pNetwork);
  bool bres = ShortestPathAStar(bGPgl2,res,pNetwork, touchedSects);
  bGPgl2->DeleteIfAllowed();
  return bres;
}

bool GPoints::ShortestPathAStar(const GLine* gl, GLine* res,
                           DbArray<TupleId>* touchedSects)const
{
  res->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if(!IsDefined() || !gl->IsDefined() || Size() < 1 || gl->NoOfComponents() < 1)
  {
    res->SetDefined(false);
    return false;
  }
  Network *pNetwork =
    NetworkManager::GetNetworkNew(gl->GetNetworkId(),netList);
  bool result = ShortestPathAStar(gl, res, pNetwork, touchedSects);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}

bool GPoints::ShortestPathAStar(const GLine* gl, GLine* res,
                                const Network* pNetwork,
                    DbArray<TupleId>* touchedSects)const
{
  res->Clear();
  if (touchedSects != 0) touchedSects->clean();
  if ( GetNetworkId() != gl->GetNetworkId() )
  {
    cmsg.inFunError ( "Both glines must belong to the network." );
    return false;
  }
  if (gl->Includes(this)) return true;
  GPoints *bGPgl2 = new GPoints(true);
  gl->GetBGP(pNetwork,bGPgl2);
  bool bres = ShortestPathAStar(bGPgl2,res,pNetwork, touchedSects);
  bGPgl2->DeleteIfAllowed();
  return bres;
}

double GPoints::NetdistanceNew(const GPoints* bgp, const Network* pNetwork)const
{
  double res = -1.0;
  if(!IsDefined() || !bgp->IsDefined() || Size() < 1 || bgp->Size() < 1)
    return res;
  if (Intersects(bgp,pNetwork)) return 0.0;
  GLine* pSP = new GLine(true);
  if (ShortestPathAStar(bgp,pSP,pNetwork,0))
    res = pSP->GetLength();
  pSP->DeleteIfAllowed();
  return res;
}

double GPoints::NetdistanceNew(const GPoints* bgp)const
{
  double result = -1.0;
  if(!IsDefined()||Size() < 1 || !bgp->IsDefined() || bgp->Size() < 1)
    return result;
  Network *pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  result = NetdistanceNew(bgp,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return result;
}

double GPoints::NetdistanceNew(const GPoint* gp, const Network* pNetwork)const
{
  double res = -1.0;
  if(!IsDefined() || !gp->IsDefined() || Size() < 1)
    return res;
  GLine* pSP = new GLine(0);
  if (ShortestPathAStar(gp, pSP, pNetwork, 0))
    res = pSP->GetLength();
  pSP->DeleteIfAllowed();
  return res;
}

double GPoints::NetdistanceNew(const GPoint* gp)const
{
  double res = -1.0;
  if(!IsDefined() || Size() < 1 || !gp->IsDefined())
    return res;
  Network *pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(gp,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

double GPoints::NetdistanceNew(const GLine* gl, const Network* pNetwork)const
{
  double res = -1.0;
  if(!IsDefined() || !gl->IsDefined() || Size() < 1 || gl->NoOfComponents()<1)
    return res;
  if (gl->Includes(this,pNetwork)) return 0.0;
  GLine* pSP = new GLine(0);
  if (ShortestPathAStar(gl, pSP, pNetwork, 0))
    res = pSP->GetLength();
  pSP->DeleteIfAllowed();
  return res;
}

double GPoints::NetdistanceNew(const GLine* gl)const
{
  double res = -1.0;
  if(!IsDefined()||Size() < 1 || !gl->IsDefined() || gl->NoOfComponents() < 1)
    return res;
  Network *pNetwork = NetworkManager::GetNetworkNew(GetNetworkId(),netList);
  res = NetdistanceNew(gl,pNetwork);
  NetworkManager::CloseNetwork(pNetwork);
  return res;
}

GPoints& GPoints::operator-= ( const GPoint& gp )
{
  GPoints *nGPs = new GPoints ( 0 );
  GPoint actGP;
  for ( int i = 0; i < m_xGPoints.Size(); i++ )
  {
    m_xGPoints.Get ( i, actGP );
    if ( gp != actGP ) nGPs->m_xGPoints.Append ( actGP );
  }
  nGPs->TrimToSize();
  return *nGPs;
}

void GPoints::Get ( const int i, GPoint& pgp ) const
{
  assert ( i >= 0 && i < m_xGPoints.Size() );
  m_xGPoints.Get ( i,pgp );
}


ListExpr GPoints::Out ( ListExpr in_xTypeInfo,
                        Word in_xValue )
{

  GPoints *pGPS = ( GPoints* ) (in_xValue.addr);
  //if (!pGPS->IsDefined()) return nl->SymbolAtom("undef");
  if ( pGPS->IsEmpty())
  {
    return nl->TheEmptyList();
  }
  GPoint pgp;
  pGPS->Get ( 0,pgp );
  ListExpr result =
      nl->OneElemList(GPoint::OutGPoint(nl->TheEmptyList(),
                                        SetWord((void*)&pgp)));
  ListExpr last = result;
  if (pGPS->Size() > 1)
  {
    for ( int i = 1; i < pGPS->Size();i++ )
    {
      pGPS->Get ( i,pgp );
      last =
        nl->Append(last,GPoint::OutGPoint (nl->TheEmptyList(),
                                           SetWord((void*)&pgp)));
    }
  }
  return result;
}

Word GPoints::In ( const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->ListLength ( instance ) == 0 )
  {
    correct = false;
    cmsg.inFunError ( "Empty List" );
    return SetWord ( Address ( 0 ) );
  }
  GPoints* pGPS = new GPoints ( nl->ListLength ( instance ) );
  if (nl->IsEqual(instance,"undef"))
  {
    correct = true;
    pGPS->SetDefined(false);
    return SetWord(Address(pGPS));
  }

  ListExpr rest = instance;
  while ( !nl->IsEmpty ( rest ) )
  {
    ListExpr first = nl->First ( rest );
    rest = nl->Rest ( rest );
    if (!nl->ListLength ( first ) == 4)
    {
      correct = false;
      cmsg.inFunError ( "GPoint incorrect.Expected list of 4 Elements." );
      return SetWord ( Address ( 0 ) );
    }
    GPoint* pgp =
      ( GPoint*)GPoint::InGPoint(nl->TheEmptyList(),
                                 first,
                                 0,
                                 errorInfo,
                                 correct ).addr;
    if ( correct )
    {
      ( *pGPS ) += ( *pgp );
      pgp->DeleteIfAllowed();
    }
    else
    {
      pgp->DeleteIfAllowed();
    }
  }
  pGPS->SetDefined ( true );
  pGPS->TrimToSize();
  return SetWord ( pGPS );
}

bool GPoints::OpenGPoints ( SmiRecord& valueRecord,size_t& offset,
                            const ListExpr typeInfo, Word& value )
{
  GPoints* pGPS = ( GPoints* ) Attribute::Open ( valueRecord,offset,typeInfo );
  value = SetWord ( pGPS );
  pGPS->TrimToSize();
  return true;
}
bool GPoints::SaveGPoints ( SmiRecord& valueRecord,size_t& offset,
                            const ListExpr typeInfo, Word& value )
{
  GPoints* pGPS = ( GPoints* ) value.addr;
  Attribute::Save ( valueRecord,offset,typeInfo,pGPS );
  return true;
}

Word GPoints::Create ( const ListExpr typeInfo )
{
  return SetWord ( new GPoints ( true ) );
}

void GPoints::Delete ( const ListExpr typeInfo,
                       Word& w )
{
  GPoints *gp = ( GPoints * ) w.addr;
  // if (l->del.refs == 1) { l->m_xRouteIntervals.Destroy();}
  if ( gp->DeleteIfAllowed() == true )
    w.addr = 0;
}

void GPoints::Close ( const ListExpr typeInfo,
                      Word& w )
{
  if ( ( ( GPoints* ) w.addr )->DeleteIfAllowed() == true )
    w.addr = 0;
}

Word GPoints::CloneGPoints ( const ListExpr typeInfo,
                             const Word& w )
{
  return SetWord ( ( ( GPoints* ) w.addr )->Clone() );
}

GPoints* GPoints::Clone() const
{
  GPoints *xOther = new GPoints ( Size() );
  xOther->SetDefined ( IsDefined() );
  xOther->m_xGPoints.copyFrom(m_xGPoints);
  return xOther;
}

void* GPoints::Cast ( void* addr )
{
  return new ( addr ) GPoints;
}

bool GPoints::Check ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, GPoints::BasicType() ) );
}


int GPoints::Compare ( const Attribute* a) const
{
  GPoints* pGPS = (GPoints*) a;
  if (Size() < pGPS->Size()) return -1;
  if (Size() > pGPS->Size()) return 1;
  GPoint gp1, gp2;
  for (int i = 0; i < Size(); i++)
  {
    Get(i,gp1);
    Get(i,gp2);
    if (gp1.Compare(gp2) != 0) return gp1.Compare(gp2);
  }
  return 0;
}

bool GPoints::Adjacent ( const Attribute* a) const
{
  return false;
}

size_t GPoints::HashValue() const
{
  size_t xHash = 0;
  // Iterate over all GPoint objects
  for ( int i = 0; i < m_xGPoints.Size(); ++i )
  {
    // Get next Interval
    GPoint pCurrentInterval;
    m_xGPoints.Get ( i, pCurrentInterval );

    // Add something for each entry
    int iNetworkId = pCurrentInterval.GetNetworkId();
    int iRouteId = pCurrentInterval.GetRouteId();
    double iPosition = pCurrentInterval.GetPosition();
    int iSide = ( int ) pCurrentInterval.GetSide();
    xHash += iNetworkId + iRouteId + ( size_t ) iPosition + iSide;
  }
  return xHash;
}

void GPoints::CopyFrom ( const Attribute* right )
{
  *this = * ( ( const GPoints * ) right );
}

struct gpointsInfo:ConstructorInfo{
  gpointsInfo(){
    name = GPoints::BasicType();
    signature = "-> DATA";
    typeExample = GPoints::BasicType();
    listRep = "(<gpoint1> <gpoint2> ...)";
    valueExample = "((1 34 235.65 1)(1 98 234.1 0))";
    remarks = "Set of network positions.";
  }
};

struct gpointsFunctions:ConstructorFunctions<GPoints>{
  gpointsFunctions(){
    in = GPoints::In;
    out = GPoints::Out;
    create = GPoints::Create;
    deletion = GPoints::Delete;
    open = GPoints::OpenGPoints;
    save = GPoints::SaveGPoints;
    close = GPoints::Close;
    clone = GPoints::CloneGPoints;
    cast = GPoints::Cast;
    sizeOf = GPoints::SizeOf;
    kindCheck = GPoints::Check;
  }
};

gpointsInfo gpsi;
gpointsFunctions gpsf;
TypeConstructor gpointsTC(gpsi,gpsf);



/*
6 Secondo Operators

6.1 Operator ~netdistance~

Returns the network distance between two ~Gpoints~ or two ~GLines~. Using
Dijkstras Algorithm for computation of the shortest paths.

*/

ListExpr OpNetNetdistanceTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );
  //ListExpr param3 = nl->Third(args);

  if ( nl->IsAtom ( param1 ) && nl->AtomType ( param1 ) == SymbolType &&
       nl->IsAtom ( param2 ) && nl->AtomType ( param2 ) == SymbolType &&
      ( ( nl->SymbolValue ( param1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( param2 ) == GPoint::BasicType() ) ||
        ( nl->SymbolValue ( param1 ) == GLine::BasicType() &&
          nl->SymbolValue ( param2 ) == GLine::BasicType() ) ) )
  {
    return nl->SymbolAtom ( CcReal::BasicType() );
  }
  return nl->SymbolAtom ( Symbol::TYPEERROR() );
}

int OpNetNetdistance_gpgp ( Word* args, Word& result, int message,
                            Word& local, Supplier in_pSupplier )
{
  GPoint* pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint* pToGPoint = ( GPoint* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pFromGPoint->IsDefined() ) || ! ( pToGPoint->IsDefined() ) )
  {
    cmsg.inFunError ( "Both gpoint must be defined!" );
    return 0;
  };
  double dist =  pFromGPoint->Netdistance ( pToGPoint );
  if ( dist > -1.0 ) pResult->Set ( true,dist );
  else pResult->Set ( false, dist );
  return 1;
};

int OpNetNetdistance_glgl ( Word* args, Word& result, int message,
                            Word& local, Supplier in_pSupplier )
{
  GLine* pGLine1 = ( GLine* ) args[0].addr;
  GLine* pGLine2 = ( GLine* ) args[1].addr;
  CcReal* pResult = ( CcReal* ) qp->ResultStorage ( in_pSupplier ).addr;
  result = SetWord ( pResult );
  if ( ! ( pGLine1->IsDefined() ) || ! ( pGLine2->IsDefined() ) )
  {
    cmsg.inFunError ( "Both gpoint must be defined!" );
    return 0;
  };
  double dist = pGLine1->Netdistance ( pGLine2 );
  if ( dist > -1.0 ) pResult->Set ( true, dist );
  else pResult->Set ( false, dist );
  return 1;
};

ValueMapping OpNetNetdistancemap[] =
{
  OpNetNetdistance_gpgp,
  OpNetNetdistance_glgl
};

int OpNetNetdistanceselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  return -1; // This point should never be reached
};

struct netdistanceInfo : OperatorInfo {
  netdistanceInfo():OperatorInfo(){
    name = "netdistance";
    signature = "gpoint X gpoint -> real";
    appendSignature("gline X gline -> real");
    syntax = "netdistance(_,_)";
    meaning = "Returns network distance from 1st to 2nd.";
  }
};

ListExpr OpNetNetdistanceNewTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length() != 2)
  {
    return listutils::typeError("netdistancenew expects 2 arguments.");
  }
  if (!( param.first().isSymbol(GPoint::BasicType())
    || param.first().isSymbol(GLine::BasicType())
       || param.first().isSymbol(GPoints::BasicType())))
  {
      return
        listutils::typeError("1.argument should be gpoint, gline or gpoints.");
  }
  if (!(param.second().isSymbol(GPoint::BasicType())
    || param.second().isSymbol(GLine::BasicType()) ||
        param.second().isSymbol(GPoints::BasicType())))
  {
    return
      listutils::typeError("2.argument should be gpoint, gline or gpoints.");
  }
  return nl->SymbolAtom ( CcReal::BasicType() );
}

template<class Source, class Target>
int OpNetNetdistanceNew ( Word* args, Word& result, int message,
                          Word& local, Supplier in_pSupplier )
{
  Source* pFrom = ( Source* ) args[0].addr;
  Target* pTo = ( Target* ) args[1].addr;
  CcReal* pResult = ( CcReal* ) qp->ResultStorage ( in_pSupplier ).addr;
  result = SetWord ( pResult );
  if ( ! ( pFrom->IsDefined() ) || ! ( pTo->IsDefined() ) )
  {
    cmsg.inFunError ( "Both arguments must be defined!" );
    return 0;
  }
  double dist = pFrom->NetdistanceNew( pTo );
  if ( dist > -1.0 ) pResult->Set ( true, dist );
  else pResult->Set ( false, dist );
  return 1;
};


ValueMapping OpNetNetdistanceNewmap[] =
{
  OpNetNetdistanceNew<GPoint, GPoint>,
  OpNetNetdistanceNew<GLine, GLine>,
  OpNetNetdistanceNew<GPoints, GPoints>,
  OpNetNetdistanceNew<GPoint, GLine>,
  OpNetNetdistanceNew<GPoint, GPoints>,
  OpNetNetdistanceNew<GLine, GPoint>,
  OpNetNetdistanceNew<GLine, GPoints>,
  OpNetNetdistanceNew<GPoints, GPoint>,
  OpNetNetdistanceNew<GPoints, GLine>
};

int OpNetNetdistanceNewselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoints::BasicType() )
    return 2;
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 3;
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoints::BasicType() )
    return 4;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 5;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoints::BasicType() )
    return 6;
  if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 7;
  if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 8;
  return -1; // This point should never be reached
};

struct netdistanceNewInfo : OperatorInfo {
  netdistanceNewInfo():OperatorInfo(){
    name = "netdistancenew";
    signature = "gpoint X gpoint -> real";
    appendSignature("gline X gline -> real");
    appendSignature("gpoints X gpoints -> real");
    appendSignature("gpoint x gline -> real");
    appendSignature("gpoint x gpoints-> real");
    appendSignature("gline X gpoint -> real");
    appendSignature("gline X gpoints -> real");
    appendSignature("gpoints X gpoint -> real");
    appendSignature("gpoints X gline -> real");
    syntax = "netdistancenew(_,_)";
    meaning = "Network distance from 1st to 2nd using AStar.";
  }
};

/*
6.2 Operator ~gpoint2rect~

Returns a rectangle degenerated to a point with coordinates rid, rid, pos, pos.

*/

ListExpr OpGPoint2RectTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, GPoint::BasicType() ) )
  {
    sendMessage ( "Element must be of type gpoint." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return ( nl->SymbolAtom ( Rectangle<2>::BasicType() ) );
}

int OpGPoint2RectValueMapping ( Word* args,
                                Word& result,
                                int message,
                                Word& local,
                                Supplier in_xSupplier )
{
  result = qp->ResultStorage ( in_xSupplier );
  Rectangle<2>* box = static_cast<Rectangle<2>* > ( result.addr );
  GPoint* arg = static_cast<GPoint*> ( args[0].addr );
  if ( !arg->IsDefined() )
  {
    box->SetDefined ( false );
  }
  else
  {
    ( *box ) = arg->NetBoundingBox();
  }
  return 0;
} //end ValueMapping

struct gpoint2rectInfo:OperatorInfo{
  gpoint2rectInfo():OperatorInfo(){
    name = "gpoint2rect";
    signature = "gpoint -> rect";
    syntax ="gpoint2rect(_)";
    meaning = "Returns a rectangle representing the gpoint.";
  }
};


/*
6.3 Operator ~inside~

Returns true if ths ~GPoint~ is inside the ~GLine~ false elsewhere.

*/
ListExpr OpInsideTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gpoint = nl->First ( args );
  ListExpr gline = nl->Second ( args );
  if ( !nl->IsAtom ( gpoint ) || nl->AtomType ( gpoint ) != SymbolType ||
          nl->SymbolValue ( gpoint ) != GPoint::BasicType() ||
          !nl->IsAtom ( gline ) ||
          nl->AtomType ( gline ) != SymbolType || nl->SymbolValue ( gline ) !=
GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpInsideValueMap ( Word* args, Word& result, int message,
                       Word& local, Supplier in_pSupplier )
{
  GPoint* pGPoint = ( GPoint* ) args[0].addr;
  GLine* pGLine = ( GLine* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( pGPoint->GetNetworkId() != pGLine->GetNetworkId() ||
          !pGLine->IsDefined() || !pGPoint->IsDefined() )
  {
    pResult->Set ( false, false );
  }
  pResult->Set ( true, pGPoint->Inside ( pGLine ,0.0) );
  return 0;
}

struct insideInfo : OperatorInfo{
  insideInfo():OperatorInfo(){
    name = "inside";
    signature = "gpoint X gline -> bool";
    syntax = "_ inside _";
    meaning = "Returns true if the gpoint is on the gline.";
  }
};


/*
6.4 Operator ~length~

Returns the length of a ~GLine~.

*/
ListExpr OpLengthTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline = nl->First ( args );
  if ( !nl->IsAtom ( gline ) || nl->AtomType ( gline ) != SymbolType ||
          nl->SymbolValue ( gline ) != GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcReal::BasicType() );
}

int OpLengthValueMap ( Word* args, Word& result, int message,
                       Word& local, Supplier in_pSupplier )
{
  GLine* pGline = ( GLine* ) args[0].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pGline->IsDefined() ) )
  {
    cmsg.inFunError ( "gline is not defined!" );
    return 0;
  };
  pResult-> Set ( true, pGline->GetLength() );
  return 1;
}

struct lengthInfo: OperatorInfo{
  lengthInfo():OperatorInfo(){
    name = "length";
    signature = "gline -> real";
    syntax = "length(_)";
    meaning = "Returns the length of the gline.";
  }
};

/*
6.5 Operator ~line2gline~

Translates a spatial ~line~ value into a network ~GLine~ value if possible.

*/

ListExpr OpLine2GLineTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
  {
    sendMessage ( "Expects a list of length 2." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xNetworkIdDesc = nl->First ( in_xArgs );
  ListExpr xLineDesc = nl->Second ( in_xArgs );

  if ( ( !nl->IsAtom ( xNetworkIdDesc ) ) ||
          !nl->IsEqual ( xNetworkIdDesc, "network" ) )
  {
    sendMessage ( "First element must be of type network." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( ( !nl->IsAtom ( xLineDesc ) ) ||
          nl->AtomType ( xLineDesc ) != SymbolType ||
          nl->SymbolValue ( xLineDesc ) != Line::BasicType() )
  {
    sendMessage ( "Second element must be of type sline." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( GLine::BasicType() );
}

int OpLine2GLineValueMapping ( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier in_xSupplier )
{
  //Initialize return value
  GLine* pGLine = ( GLine* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pGLine );
  // Get and check input values.
  Network* pNetwork = ( Network* ) args[0].addr;
  GLine *res = new GLine ( 0 );
  if ( pNetwork == 0 || !pNetwork->IsDefined() )
  {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    res->SetDefined ( false );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
    return 0;
  }
  res->SetNetworkId ( pNetwork->GetId() );
  Line* pLine = ( Line* ) args[1].addr;
  if ( pLine == NULL || !pLine->IsDefined() )
  {
    string strMessage = "line does not exist.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    res->SetDefined ( false );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
    return 0;
  }
  if ( pLine->Size() <= 0 )
  {
    string strMessage = "line is empty";
    cerr <<strMessage << endl;
    sendMessage ( strMessage );
    res->SetDefined ( true );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
    return 0;
  }

  HalfSegment hs;
  pLine->Get ( 0,hs );
  RouteInterval *ri =
      pNetwork->Find ( hs.GetLeftPoint(), hs.GetRightPoint() );
  if ( ri!= 0 )
  {
    RITree *riTree = new RITree ( ri->GetRouteId(), ri->GetStartPos(),
                                  ri->GetEndPos() );
    delete ri;
    for ( int i = 1; i < pLine->Size();i++ )
    {
      pLine->Get ( i,hs );
      ri = pNetwork->Find ( hs.GetLeftPoint(), hs.GetRightPoint() );
      if ( ri!=0 )
      {
        riTree->Insert ( ri->GetRouteId(), ri->GetStartPos(), ri->GetEndPos() );
        delete ri;
      }
    }
    riTree->TreeToGLine ( res );
    riTree->RemoveTree();
    res->SetDefined ( true );
    res->SetSorted ( true );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
  }
  else
  {
    res->SetDefined ( false );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
  }
  return 0;
} //end ValueMapping

struct line2glineInfo : OperatorInfo{
  line2glineInfo():OperatorInfo(){
    name = "line2gline";
    signature = "network X line -> gline";
    syntax = "line2gline(_,_)";
    meaning = "Translates a line into an gline if possible.";
  }
};




/*
6.6 Operator ~=~

Returns true if two ~GPoint~s respectively ~GLine~s are identical false
elsewhere.

*/

ListExpr OpNetEqualTypeMap ( ListExpr args )
{
  NList param(args);

  if ( param.length() != 2 )
    return listutils::typeError("Two arguments expected.");

  NList arg1(param.first());
  NList arg2(param.second());

  if (!(arg1.isSymbol(GPoint::BasicType())
    || arg1.isSymbol(GLine::BasicType())))
    return listutils::typeError("expected gpoint or gline values");

  if (arg1 != arg2)
    return listutils::typeError("Arguments must be of same type.");
  else
    return nl->SymbolAtom ( CcBool::BasicType() );
}

template<class Arg>
int OpNetEqual ( Word* args, Word& result, int message,
                      Word& local, Supplier in_pSupplier )
{
  Arg* p1 = ( Arg* ) args[0].addr;
  Arg* p2 = ( Arg* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( ! ( p1->IsDefined() ) || !p2->IsDefined() )
  {
    cmsg.inFunError ( "Both arguments must be defined!" );
    pResult->Set ( false, false );
    return 1;
  };
  pResult-> Set ( true, *p1 == *p2 );
  return 0;
}

ValueMapping OpNetEqualmap[] =
{
  OpNetEqual<GPoint>,
  OpNetEqual<GLine>,
};

int OpNetEqualselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  return -1; // This point should never be reached
};

struct equalInfo:OperatorInfo{
  equalInfo():OperatorInfo(){
    name = "=";
    signature = "gpoint X gpoint -> bool";
    appendSignature("gline X gline -> bool");
    syntax = "_ = _";
    meaning = "Returns true if the two objects are equal";
  }
};



/*
6.7 Operator ~intersects~

Returns true if two ~Gline~ intersect false elsewhere.

*/

ListExpr OpNetIntersectsTypeMap ( ListExpr in_xArgs )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength ( in_xArgs ) == 2 )
  {
    arg1 = nl->First ( in_xArgs );
    arg2 = nl->Second ( in_xArgs );
    if ( nl->IsAtom ( arg1 ) && nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
            nl->IsAtom ( arg2 ) &&
            nl->AtomType ( arg2 ) == SymbolType &&
            nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    {
      return ( nl->SymbolAtom ( CcBool::BasicType() ) );
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetIntersectsValueMapping ( Word* args,
                                  Word& result,
                                  int message,
                                  Word& local,
                                  Supplier in_xSupplier )
{
  // Get (empty) return value
  CcBool* pResult = ( CcBool* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pResult );
  pResult->Set ( false, false );
  // Get input values
  GLine* pGLine1 = ( GLine* ) args[0].addr;
  if ( pGLine1 == NULL || !pGLine1->IsDefined() )
  {
    cerr << "First gline does not exist." << endl;
    pResult->Set ( false, false );
    return 0;
  }
  GLine* pGLine2 = ( GLine* ) args[1].addr;
  if ( pGLine2 == NULL || !pGLine2->IsDefined() )
  {
    cerr << "Second gline does not exist." << endl;
    pResult->Set ( false, false );
    return 0;
  }
//  const RouteInterval *pRi1, *pRi2;
  if ( pGLine1->GetNetworkId() != pGLine2->GetNetworkId() )
  {
    cerr << "glines belong to different networks." << endl;
    pResult->Set ( true, false );
    return 0;
  }
  pResult->Set ( true, pGLine1->Intersects ( pGLine2 ) );
  return 0;
}

struct intersectsInfo:OperatorInfo{
  intersectsInfo():OperatorInfo(){
    name = "intersects";
    signature = "gline X gline -> bool";
    syntax = "intersects(_,_)";
    meaning = "Returns true if the glines intersect.";
  }
};



/*
6.8 Operator ~junctions~

Returns the junctions relation of the network.

*/
ListExpr OpNetworkJunctionsTypeMap ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength ( args ) == 1 )
  {
    arg1 = nl->First ( args );
    if ( nl->IsAtom ( arg1 ) &&
            nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString ( Network::junctionsInternalTypeInfo, xType );
      return xType;
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetworkJunctionsValueMapping ( Word* args, Word& result, int message,
                                     Word& local, Supplier s )
{
  Network *network = ( Network* ) args[0].addr;
  result = SetWord ( network->GetJunctions() );

  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );

  return 0;
}

struct junctionsInfo:OperatorInfo{
  junctionsInfo():OperatorInfo(){
    name = "junctions";
    signature = "network -> rel";
    syntax = "junctions(_)";
    meaning = "Returns a relation with the junctions of the network";
  }
};


/*
6.9 Operator ~routes~

Returns the routes relation of the network.

*/
ListExpr OpNetworkRoutesTypeMap ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength ( args ) == 1 )
  {
    arg1 = nl->First ( args );
    if ( nl->IsAtom ( arg1 ) &&
            nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString ( Network::routesTypeInfo, xType );
      return xType;
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetworkRoutesValueMapping ( Word* args, Word& result, int message,
                                  Word& local, Supplier s )
{
  Network *network = ( Network* ) args[0].addr;
  Relation* pRoute = network->GetRoutes();
  result =  SetWord ( pRoute->Clone() );
  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );

  return 0;
}

struct routesInfo:OperatorInfo{
  routesInfo():OperatorInfo(){
    name = "routes";
    signature = "network -> rel";
    syntax = "routes(_)";
    meaning = "Returns a relation with the routes of the network.";
  }
};


/*
6.10 Operator ~sections~

Returns the sections relation of the network.

*/

ListExpr OpNetworkSectionsTypeMap ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength ( args ) == 1 )
  {
    arg1 = nl->First ( args );
    if ( nl->IsAtom ( arg1 ) &&
            nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString ( Network::sectionsInternalTypeInfo, xType );
      return xType;
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetworkSectionsValueMapping ( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  Network *network = ( Network* ) args[0].addr;
  result = SetWord ( network->GetSections() );

  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );

  return 0;
}

struct sectionsInfo:OperatorInfo{
  sectionsInfo():OperatorInfo(){
    name = "sections";
    signature = "network -> rel";
    syntax = "sections(_)";
    meaning = "Returns a relation of the network sections.";
  }
};



/*
6.11 Operator ~thenetwork~

Creates a network with the given id, from the given routes and junctions
relations.

*/
ListExpr OpNetworkTheNetworkTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 4 )
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );

  ListExpr xIdDesc = nl->First ( in_xArgs );
  ListExpr xScaleDesc = nl->Second(in_xArgs);
  ListExpr xRoutesRelDesc = nl->Third ( in_xArgs );
  ListExpr xJunctionsRelDesc = nl->Fourth ( in_xArgs );

  if ( !nl->IsEqual ( xIdDesc, CcInt::BasicType() ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( !nl->IsEqual ( xScaleDesc, CcReal::BasicType() ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( !IsRelDescription ( xRoutesRelDesc ) ||
          !IsRelDescription ( xJunctionsRelDesc ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xType;
  nl->ReadFromString ( Network::routesTypeInfo, xType );
  if ( !CompareSchemas ( xRoutesRelDesc, xType ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  nl->ReadFromString ( Network::junctionsTypeInfo, xType );
  if ( !CompareSchemas ( xJunctionsRelDesc, xType ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( "network" );
}

int OpNetworkTheNetworkValueMapping ( Word* args, Word& result,
                                      int message, Word& local, Supplier s )
{
  Network* pNetwork = ( Network* ) qp->ResultStorage ( s ).addr;
  CcInt* pId = ( CcInt* ) args[0].addr;
  int iId = pId->GetIntval();
  map<int,string>::iterator it = netList->find ( iId );
  if ( it != netList->end() )
  {
    cerr << "NetworkId used before" << iId << endl;
    while ( it != netList->end() )
    {
      it = netList->find ( ++iId );
    }
    cerr << "NetworkId changed to: " << iId << endl;
  }
  netList->insert ( pair<int,string> ( iId, "" ) );
  double scalef = ((CcReal*)args[1].addr)->GetRealval();
  Relation* pRoutes = ( Relation* ) args[2].addr;
  Relation* pJunctions = ( Relation* ) args[3].addr;
  pNetwork->Load ( iId,
                   scalef,
                   pRoutes,
                   pJunctions );
  result = SetWord ( pNetwork );

  return 0;
}

const string TheNetworkSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + CcInt::BasicType() + " X " + CcReal::BasicType() + "X " +
  Relation::BasicType() + " X " + Relation::BasicType() + " -> " +
  Network::BasicType() + "</text--->"
  "<text> thenetwork( <id> , <scaleinformation> , <roadsrel> , "+
  "<crossings> ) </text--->"
  "<text> Creates the network with identifier id, from the informations in "+
  "the roads relation and crossings relation. The scaleinformation is used"+
  "within map matching operations. It tells the system where how much it must"+
  "scale the given coordinates to get meter values for distances. Because we" +
  "the tolerance value of map matching computations must be given relativ to" +
  "the data format.</text--->"
  "<text> query getfirstelem(1, 1.0, roads, crossings)</text--->))";

  Operator theNetwork(
    "thenetwork",
    TheNetworkSpec,
    OpNetworkTheNetworkValueMapping,
    Operator::SimpleSelect,
    OpNetworkTheNetworkTypeMap
  );


/*
6.12 Operator ~no\_components~

Returns the number of ~RouteIntervals~ of the ~GLine~.

*/

ListExpr OpNoComponentsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline = nl->First ( args );
  if ( !nl->IsAtom ( gline ) || nl->AtomType ( gline ) != SymbolType ||
          nl->SymbolValue ( gline ) != GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcInt::BasicType() );
}

int OpNoComponentsValueMapping ( Word* args, Word& result, int message,
                                 Word& local, Supplier in_pSupplier )
{
  GLine* pGline = ( GLine* ) args[0].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcInt* pResult = ( CcInt* ) result.addr;
  if ( ! ( pGline->IsDefined() ) )
  {
    pResult->Set ( false, 0 );
    return 0;
  };
  pResult-> Set ( true, pGline->NoOfComponents() );
  return 1;
}

struct noComponentsInfo:OperatorInfo{
  noComponentsInfo():OperatorInfo(){
    name = "no_components";
    signature = "gline -> int";
    syntax = "no_components(_)";
    meaning = "Returns the number of Route Intervals of the gline.";
  }
};



/*
6.13 Operator ~point2gpoint~

Translates a spatial ~Point~ value into a network ~GPoint~ value if possible.

*/
ListExpr OpPoint2GPointTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
  {
    sendMessage ( "Expects a list of length 2." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xNetworkIdDesc = nl->First ( in_xArgs );
  ListExpr xMPointDesc = nl->Second ( in_xArgs );

  if ( ( !nl->IsAtom ( xNetworkIdDesc ) ) ||
          !nl->IsEqual ( xNetworkIdDesc, "network" ) )
  {
    sendMessage ( "First element must be of type network." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( ( !nl->IsAtom ( xMPointDesc ) ) ||
          nl->AtomType ( xMPointDesc ) != SymbolType ||
          nl->SymbolValue ( xMPointDesc ) != Point::BasicType() )
  {
    sendMessage ( "Second element must be of type point." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( GPoint::BasicType() );
}

int OpPoint2GPointValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  GPoint* pGPoint = ( GPoint* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pGPoint );
  Network* pNetwork = ( Network* ) args[0].addr;
  if ( pNetwork == 0 || !pNetwork->IsDefined() )
  {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    pGPoint->SetDefined ( false );
    return 0;
  }

  Point* pPoint = ( Point* ) args[1].addr;
  if ( pPoint == NULL || !pPoint->IsDefined() )
  {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    pGPoint->SetDefined ( false );
    return 0;
  }
  GPoint *res = pNetwork->GetNetworkPosOfPoint ( *pPoint );
  pGPoint->CopyFrom ( res );
  res->DeleteIfAllowed();
  /*GPoint *res = pNetwork->GetNetworkPosOfPoint(*pPoint);
  qp->ChangeResultStorage(in_xSupplier, res);*/
  return 0;
} //end ValueMapping

struct point2gpointInfo:OperatorInfo{
  point2gpointInfo():OperatorInfo(){
    name = "point2gpoint";
    signature = "network X point -> gpoint";
    syntax = "point2gpoint(_,_)";
    meaning = "Translates a point into a gpoint value.";
  }
};


/*
6.14 Operator ~gpoint2point~

Translates a ~gpoint~ into a spatial ~Point~ value.

*/

ListExpr OpGPoint2PointTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xMPointDesc = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xMPointDesc ) ) ||
          nl->AtomType ( xMPointDesc ) != SymbolType ||
          nl->SymbolValue ( xMPointDesc ) != GPoint::BasicType() )
  {
    sendMessage ( "Element must be of type gpoint." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( Point::BasicType() );
}

int OpGPoint2PointValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  Point* pPoint = ( Point* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pPoint );
  GPoint* pGPoint = ( GPoint* ) args[0].addr;
  if ( pGPoint == NULL || !pGPoint->IsDefined() )
  {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    pPoint->SetDefined ( false );
    return 0;
  }
  pGPoint->ToPoint ( pPoint );
  return 0;
} //end ValueMapping

struct gpoint2pointInfo:OperatorInfo{
  gpoint2pointInfo():OperatorInfo(){
    name = "gpoint2point";
    signature = "gpoint -> point";
    syntax = "gpoint2point(_)";
    meaning = "Translates a gpoint into an point value.";
  }
};



/*
6.15 Operator ~polygpoint~

Returns a  stream of all ~GPoint~ values which are at the same network position
as the given ~GPoint~. Including the given  ~GPoint~.

*/

ListExpr OpPolyGPointTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );
  ListExpr xnetwork = nl->Second ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, GPoint::BasicType() ) )
  {
    sendMessage ( "First Element must be of type gpoint." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  if ( ( !nl->IsAtom ( xnetwork ) ) ||
          !nl->IsEqual ( xnetwork, "network" ) )
  {
    sendMessage ( "Second Element must be of type network." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->TwoElemList ( nl->SymbolAtom ( Symbol::STREAM() ),
                           nl->SymbolAtom ( GPoint::BasicType() ) );
}

int OpPolyGPointValueMapping ( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier in_xSupplier )
{
  GPointList* localinfo;
  GPoint res;
  result = qp->ResultStorage ( in_xSupplier );
  switch ( message )
  {
    case OPEN:
      local = SetWord ( new GPointList ( ( GPoint* ) args[0].addr,
                                         ( Network* ) args[1].addr ) );
      return 0;
    case REQUEST:
      localinfo = ( GPointList* ) local.addr;
      res = localinfo->NextGPoint();
      if ( !res.IsDefined() )
      {
        return CANCEL;
      }
      else
      {
        result = SetWord ( new GPoint ( res ) );
        return YIELD;
      }
    case CLOSE:
      if ( local.addr != 0 )
      {
        localinfo = ( GPointList* ) local.addr;
        localinfo->Destroy();
        delete localinfo;
        localinfo = 0;
        local = SetWord ( Address ( 0 ) );
      }
      return 0;
  }
  return 0; // ignore unknown message
} //end ValueMapping

struct polygpointsInfo:OperatorInfo{
  polygpointsInfo():OperatorInfo(){
    name = "polygpoints";
    signature = "gpoint X network -> stream(gpoint)";
    syntax = "polygpoints(gpoint, network)";
    meaning = "Returns all gpoints with the network position of gpoint.";
  }
};




/*
6.16 Operator ~routeintervals~

Returns a stream of rectangles representing the ~RouteIntervals~ of the ~GLine~.

*/

ListExpr OpRouteIntervalsTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, GLine::BasicType() ) )
  {
    sendMessage ( "First Element must be of type gline." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->TwoElemList ( nl->SymbolAtom ( Symbol::STREAM() ),
                           nl->SymbolAtom ( Rectangle<2>::BasicType() ) );
}

int OpRouteIntervalsValueMapping ( Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier )
{
  RectangleList* localinfo;
  Rectangle<2> res;
  result = qp->ResultStorage ( in_xSupplier );
  switch ( message )
  {
    case OPEN:
      local = SetWord ( new RectangleList ( ( GLine* ) args[0].addr ) );
      return 0;
    case REQUEST:
      localinfo = ( RectangleList* ) local.addr;
      res = localinfo->NextRectangle();
      if ( !res.IsDefined() ) return CANCEL;
      else
      {
        result = SetWord ( new Rectangle<2> ( res ) );
        return YIELD;
      }
    case CLOSE:
      if ( local.addr != 0 )
      {
        delete ( RectangleList* ) local.addr;
        local = SetWord ( Address ( 0 ) );
      }
      return 0;
  }
  return 0; // ignore unknown message
} //end ValueMapping

struct routeIntervalsInfo:OperatorInfo{
  routeIntervalsInfo():OperatorInfo(){
    name = "routeintervals";
    signature = "gline -> stream(rect)";
    syntax = "routeintervals(_)";
    meaning = "Returns a netbox for each routeinterval of the gline.";
  }
};


/*
6.17 Operator ~shortest\_path~

Returns the shortest path in the ~Network~ between two ~GPoint~. Using Dijkstra
Algorithm to compute the shortest path.

*/

ListExpr OpShortestPathTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length() != 2)
  {
    return listutils::typeError("shortest_path expects 2 arguments.");
  }

  if ((param.first().isSymbol(GPoint::BasicType())
    && param.second().isSymbol(GPoint::BasicType())) ||
      (param.first().isSymbol(GLine::BasicType())
      && param.second().isSymbol(GLine::BasicType())))
    return nl->SymbolAtom(GLine::BasicType());
  else
    return listutils::typeError("Both arguments should be gpoint or gline");
}

int OpShortestPath_gpgp ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  result = qp->ResultStorage(in_xSupplier);
  GLine* pGLine = static_cast<GLine*>(result.addr);
  GPoint *pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint *pToGPoint = ( GPoint* ) args[1].addr;
  Network* pNetwork = NetworkManager::GetNetworkNew(pFromGPoint->GetNetworkId(),
                                                    netList);
  pGLine->SetSorted ( false );
  pGLine->SetDefined ( pFromGPoint->ShortestPath ( pToGPoint, pGLine, pNetwork,
                                                   0 ) );
  NetworkManager::CloseNetwork(pNetwork);
  return 0;
}

int OpShortestPath_glgl ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  result = qp->ResultStorage(in_xSupplier);
  GLine* pGLine = static_cast<GLine*>(result.addr);
  GLine *pFromGLine = ( GLine* ) args[0].addr;
  GLine *pToGLine = ( GLine* ) args[1].addr;
  Network* pNetwork = NetworkManager::GetNetworkNew(pFromGLine->GetNetworkId(),
                                                    netList);
  pGLine->SetSorted ( false );
  pGLine->SetDefined ( pFromGLine->ShortestPathBF( pToGLine, pGLine, pNetwork,
                                                   0 ) );
  NetworkManager::CloseNetwork(pNetwork);
  return 0;
}

int OpShortestPathSelect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  return -1; // This point should never be reached
};

ValueMapping OpShortestPathValMap[] =
{
  OpShortestPath_gpgp,
  OpShortestPath_glgl
};

struct shortestpathInfo:OperatorInfo{
  shortestpathInfo():OperatorInfo(){
    name = "shortest_path";
    signature = "gpoint X gpoint -> gline";
    appendSignature ("gline X gline -> gline");
    syntax = "shortest_path (_,_)";
    meaning = "Dijkstra for gpoint. Bruteforce for gline.";
  }
};


/*
6.17 Operator ~shortest\_pathastar~

Returns the shortest path in the ~Network~ between two ~GPoint~. Using
AStar-Algorithm to compute the shortest path.

*/

ListExpr OpShortestPathAStarTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length() != 2)
  {
    return listutils::typeError("netdistancenew expects 2 arguments.");
  }
  if (!( param.first().isSymbol(GPoint::BasicType())
    || param.first().isSymbol(GLine::BasicType())
       || param.first().isSymbol(GPoints::BasicType())))
  {
      return
        listutils::typeError("1.argument should be gpoint, gline or gpoints.");
  }
  if (!(param.second().isSymbol(GPoint::BasicType())
    || param.second().isSymbol(GLine::BasicType()) ||
        param.second().isSymbol(GPoints::BasicType())))
  {
    return
      listutils::typeError("2.argument should be gpoint, gline or gpoints.");
  }
  return nl->SymbolAtom ( GLine::BasicType() );
}

template<class Source, class Target>
int OpShortestPathAStar ( Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier in_xSupplier )
{
  result = qp->ResultStorage(in_xSupplier);
  GLine* pGLine = static_cast<GLine*>(result.addr);
  Source *pFrom = ( Source* ) args[0].addr;
  Target *pTo = ( Target* ) args[1].addr;
  pGLine->SetSorted ( false );
  if (!pFrom->IsDefined() || !pTo->IsDefined())
  {
    pGLine->SetDefined(false);
    return 0;
  }
  Network* pNetwork =
    NetworkManager::GetNetworkNew(pFrom->GetNetworkId(), netList);
  pGLine->SetDefined ( pFrom->ShortestPathAStar( pTo, pGLine,
                                                 pNetwork, 0 ) );
  NetworkManager::CloseNetwork(pNetwork);
  return 0;
}

int OpShortestPathAStarSelect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoints::BasicType() )
    return 2;
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 3;
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoints::BasicType() )
    return 4;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 5;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoints::BasicType() )
    return 6;
  if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 7;
  if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 8;
  return -1; // This point should never be reached
};

ValueMapping OpShortestPathAStarMap[] =
{
  OpShortestPathAStar<GPoint, GPoint>,
  OpShortestPathAStar<GLine, GLine>,
  OpShortestPathAStar<GPoints, GPoints>,
  OpShortestPathAStar<GPoint, GLine>,
  OpShortestPathAStar<GPoint, GPoints>,
  OpShortestPathAStar<GLine, GPoint>,
  OpShortestPathAStar<GLine, GPoints>,
  OpShortestPathAStar<GPoints, GPoint>,
  OpShortestPathAStar<GPoints, GLine>
};

struct shortestpathAstarInfo:OperatorInfo{
  shortestpathAstarInfo():OperatorInfo(){
    name = "shortest_pathastar";
    signature = "gpoint X gpoint -> gline";
    appendSignature("gline X gline -> gline");
    appendSignature("gpoints X gpoints -> gline");
    appendSignature("gpoint x gline -> gline");
    appendSignature("gpoint x gpoints -> gline");
    appendSignature("gline X gpoint -> gline");
    appendSignature("gline X gpoints -> gline");
    appendSignature("gpoints X gpoint -> gline");
    appendSignature("gpoints X gline -> gline");
    syntax = "shortest_pathastar (_,_)";
    meaning = "Returns shortest path between objects using Astar-Variant.";
  }
};



/*
6.18 Operator ~gline2line~

Returns the ~line~ value of the given ~GLine~.

*/

ListExpr OpGLine2LineTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, GLine::BasicType() ) )
  {
    sendMessage ( "Element must be of type gline." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( Line::BasicType() );
}

int OpGLine2LineValueMapping ( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier in_xSupplier )
{
  Line* pLine = ( Line* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pLine );
  GLine* pGLine = ( GLine* ) args[0].addr;
  if ( pGLine == NULL || !pGLine->IsDefined() )
  {
    sendMessage ( "GLine must be defined!" );
    pLine->SetDefined ( false );
    return 0;
  }
  pGLine->Gline2line ( pLine );
  return 0;
}

struct gline2lineInfo:OperatorInfo{
  gline2lineInfo():OperatorInfo(){
    name = "gline2line";
    signature = "gline -> line";
    syntax = "gline2line(_)";
    meaning = "Translates a gline into an line value.";
  }
};


/*
6.19 Operator ~isempty~

Returns if the ~GLine~. is empty.

*/
ListExpr OpNetIsEmptyTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline = nl->First ( args );
  if ( !nl->IsAtom ( gline ) || nl->AtomType ( gline ) != SymbolType ||
          nl->SymbolValue ( gline ) != GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpNetIsEmptyValueMap ( Word* args, Word& result, int message,
                           Word& local, Supplier in_pSupplier )
{
  GLine* pGline = ( GLine* ) args[0].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( ( ! ( pGline->IsDefined() ) ) || pGline->NoOfComponents() == 0 )
  {
    pResult->Set ( true, true );
    return 0;
  }
  else
  {
    pResult->Set ( true,false );
    return 0;
  }
}

struct isEmptyInfo:OperatorInfo{
  isEmptyInfo():OperatorInfo(){
    name = "isempty";
    signature = "gline -> bool";
    syntax = "isempty(_)";
    meaning = "Returns true if gline has no route intervals.";
  }
};



/*
6.20 Operator ~union~

Builds the union of the two given glines as sorted gline.

*/

ListExpr OpNetUnionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline1 = nl->First ( args );
  ListExpr gline2 = nl->Second ( args );

  if ( !nl->IsAtom ( gline1 ) || nl->AtomType ( gline1 ) != SymbolType ||
          nl->SymbolValue ( gline1 ) != GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( !nl->IsAtom ( gline2 ) || nl->AtomType ( gline2 ) != SymbolType ||
          nl->SymbolValue ( gline2 ) != GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( GLine::BasicType() );
}

int OpNetUnionValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GLine *pGL1 = ( GLine* ) args[0].addr;
  GLine *pGL2 = ( GLine* ) args[1].addr;
  GLine *pGLine = ( GLine* ) qp->ResultStorage ( in_pSupplier ).addr;
  result = SetWord ( pGLine );
  pGL1->Uniongl ( pGL2, pGLine );
  return 0;
}

struct unionInfo:OperatorInfo{
  unionInfo():OperatorInfo(){
    name = "union";
    signature = "gline X gline -> gline";
    syntax = "union (_,_)";
    meaning = "Returns one gline containing both gline.";
  }
};

/*
6.21 Operator ~distance~

Returns the Euclidean Distance between two ~Gpoints~ or two ~GLines~.

*/

ListExpr OpNetDistanceTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );

  if ( ( nl->IsAtom ( param1 ) && nl->AtomType ( param1 ) == SymbolType &&
          nl->IsAtom ( param2 ) && nl->AtomType ( param2 ) == SymbolType &&
          ( ( nl->SymbolValue ( param1 ) == GPoint::BasicType() &&
              nl->SymbolValue ( param2 ) == GPoint::BasicType() ) ||
            ( nl->SymbolValue ( param1 ) == GLine::BasicType() &&
              nl->SymbolValue ( param2 ) == GLine::BasicType() ) ) ) )
  {
    return nl->SymbolAtom ( CcReal::BasicType() );
  }
  return nl->SymbolAtom ( Symbol::TYPEERROR() );
}

int OpNetDistance_gpgp ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GPoint* pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint* pToGPoint = ( GPoint* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pFromGPoint->IsDefined() ) || ! ( pToGPoint->IsDefined() ) )
  {
    cmsg.inFunError ( "Both gpoint must be defined!" );
    return 0;
  };
  pResult-> Set ( true, pFromGPoint->Distance ( pToGPoint ) );
  return 0;
};

int OpNetDistance_glgl ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GLine* pGLine1 = ( GLine* ) args[0].addr;
  GLine* pGLine2 = ( GLine* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pGLine1->IsDefined() ) || ! ( pGLine2->IsDefined() ) ||
          pGLine1->NoOfComponents() == 0 || pGLine2->NoOfComponents() == 0 )
  {
    cmsg.inFunError ( "Both gline must be defined! And have at least 1 interval"
);
    return 0;
  };
  pResult-> Set ( true, pGLine1->Distance ( pGLine2 ) );
  return 1;
};

ValueMapping OpNetDistancemap[] =
{
  OpNetDistance_gpgp,
  OpNetDistance_glgl,
  0
};

int OpNetDistanceselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  return -1; // This point should never be reached
};

struct distanceInfo:OperatorInfo{
  distanceInfo():OperatorInfo(){
    name = "distance";
    signature = "gpoint X gpoint -> real";
    appendSignature("gline X gline -> real");
    syntax = "distance(_,_)";
    meaning = "Computes the Euclidean Distance between the objects.";
  }
};

/*
6.20 Operator ~getBGP~

Returns the bounding GPoints of a gline.

*/

ListExpr OpGetBGPTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline1 = nl->First ( args );

  if ( !nl->IsAtom ( gline1 ) || nl->AtomType ( gline1 ) != SymbolType ||
          nl->SymbolValue ( gline1 ) != GLine::BasicType() )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( GPoints::BasicType() );
}

int OpGetBGPValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  GPoints* pGPoints = static_cast<GPoints*>(result.addr);
  GLine *pGL1 = ( GLine* ) args[0].addr;
  if (!pGL1->IsDefined() || pGL1->NoOfComponents() <= 0)
  {
    pGPoints->SetDefined(false);
    return 0;
  }
  pGL1->GetBGP(pGPoints);
  return 0;
}

struct getBGPInfo:OperatorInfo{
  getBGPInfo():OperatorInfo(){
    name = "getBGP";
    signature = "gline -> gpoints";
    syntax = "getBGP (_)";
    meaning = "Returns the bounding GPoints of gline.";
  }
};

/*
6.22 Operator ~spsearchvisited~

Returns the stream of section tuples that have been visited searching the
shortest path.

*/
ListExpr OpSPSearchVisitedTypeMap ( ListExpr args )
{
  NList param(args);

  ListExpr xType;
  nl->ReadFromString ( Network::sectionsInternalTypeInfo, xType );
  ListExpr tupleType = nl->Second(xType);

  if (param.length() != 3)
  {
    return listutils::typeError("spsearchvisited expects 3 arguments.");
  }
  if (!( param.first().isSymbol(GPoint::BasicType())
    || param.first().isSymbol(GLine::BasicType())
       || param.first().isSymbol(GPoints::BasicType())))
  {
      return
        listutils::typeError("1.argument should be gpoint, gline or gpoints.");
  }
  if (!(param.second().isSymbol(GPoint::BasicType())
    || param.second().isSymbol(GLine::BasicType()) ||
        param.second().isSymbol(GPoints::BasicType())))
  {
    if (param.first().isSymbol(GPoint::BasicType())
      && param.second().isSymbol("ugpoint"))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             tupleType);
    else
      return listutils::typeError(
      "2.argument should be (u)gpoint, gline or gpoints.");
  }
  if (!param.third().isSymbol(CcBool::BasicType()))
  {
    return listutils::typeError("3.argument should be bool");
  }

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         tupleType);
}

struct OpSPSearchVisitedLocalInfo
{
  OpSPSearchVisitedLocalInfo()
  {
    pNetwork = 0;
    visitedSections = 0;
    pos = 0;
  }

  Network* pNetwork;
  DbArray<TupleId>* visitedSections;
  int pos;
};

int OpSPSearchVisitedValueMap_gpgp(Word* args, Word& result, int message,
                                   Word& local, Supplier in_pSupplier)
{
  OpSPSearchVisitedLocalInfo* li = 0;

  switch(message)
  {
    case OPEN:
    {
      li = new OpSPSearchVisitedLocalInfo();
      li->visitedSections = new DbArray<TupleId>(1);
      GPoint *pFromGPoint = ( GPoint* ) args[0].addr;
      GPoint *pToGPoint = ( GPoint* ) args[1].addr;
      bool dijkstra = ((CcBool*)args[2].addr)->GetBoolval();
      bool success = false;
      GLine* pGLine = new GLine(0);
      li->pNetwork =
        NetworkManager::GetNetworkNew(pFromGPoint->GetNetworkId(),netList);
      if (dijkstra)
      {
        success = pFromGPoint->ShortestPath ( pToGPoint, pGLine, li->pNetwork,
                                              li->visitedSections);
      }
      else
      {
        success = pFromGPoint->ShortestPathAStar ( pToGPoint, pGLine,
                                                   li->pNetwork,
                                                   li->visitedSections);
      }
      if(!success)
      {
        li->visitedSections->Destroy();
        delete li->visitedSections;
        li->visitedSections = 0;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        delete li;
        li = 0;
      }
      pGLine->DeleteIfAllowed();
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      if (local.addr != 0)
        li = (OpSPSearchVisitedLocalInfo*) local.addr;
      else
        return CANCEL;
      if (li->pos < 0 || li->pos >= li->visitedSections->Size())
        return CANCEL;
      else
      {
        TupleId actTID;
        li->visitedSections->Get(li->pos,actTID);
        Tuple* actTuple = li->pNetwork->GetSection(actTID);
        result.setAddr(actTuple);
        li->pos++;
        return YIELD;
      }
      break;
    }

    case CLOSE:
    {
      if (local.addr != 0)
      {
        li = (OpSPSearchVisitedLocalInfo*) local.addr;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        li->visitedSections->Destroy();
        delete li->visitedSections;
        li->visitedSections = 0;
        delete li;
        li = 0;
        local.addr = 0;
      }
      return 0;
      break;
    }
  }
  return 0; //should never been reached
}

int OpSPSearchVisitedValueMap_glgl(Word* args, Word& result, int message,
                                   Word& local, Supplier in_pSupplier)
{
  OpSPSearchVisitedLocalInfo* li = 0;

  switch(message)
  {
    case OPEN:
    {
      li = new OpSPSearchVisitedLocalInfo();
      li->visitedSections = new DbArray<TupleId>(0);
      GLine *pFromGLine = ( GLine* ) args[0].addr;
      GLine *pToGLine = ( GLine* ) args[1].addr;
      bool bruteforce = ((CcBool*)args[2].addr)->GetBoolval();
      bool success = false;
      GLine* pGLine = new GLine(0);
      li->pNetwork =
        NetworkManager::GetNetworkNew(pFromGLine->GetNetworkId(),netList);
      if (bruteforce)
      {

        success = pFromGLine->ShortestPathBF(pToGLine, pGLine, li->pNetwork,
                                             li->visitedSections);
      }
      else
      {
        success = pFromGLine->ShortestPathAStar ( pToGLine, pGLine,
                                             li->pNetwork,
                                             li->visitedSections);
      }
      if(!success)
      {
        li->visitedSections->Destroy();
        delete li->visitedSections;
        li->visitedSections = 0;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        delete li;
        li = 0;
      }
      pGLine->DeleteIfAllowed();
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      if (local.addr != 0)
        li = (OpSPSearchVisitedLocalInfo*) local.addr;
      else
        return CANCEL;
      if (li->pos < 0 || li->pos >= li->visitedSections->Size())
        return CANCEL;
      else
      {
        TupleId actTID;
        li->visitedSections->Get(li->pos,actTID);
        Tuple* actTuple = li->pNetwork->GetSection(actTID);
        result.setAddr(actTuple);
        li->pos++;
        return YIELD;
      }
      break;
    }

    case CLOSE:
    {
      if (local.addr != 0)
      {
        li = (OpSPSearchVisitedLocalInfo*) local.addr;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        li->visitedSections->Destroy();
        delete li->visitedSections;
        li->visitedSections = 0;
        delete li;
        li = 0;
        local.addr = 0;
      }
      return 0;
      break;
    }
  }
  return 0; //should never been reached
}


template<class Source, class Target>
int OpSPSearchVisitedValueMap(Word* args, Word& result, int message,
                              Word& local, Supplier in_pSupplier)
{
  OpSPSearchVisitedLocalInfo* li = 0;

  switch(message)
  {
    case OPEN:
    {
      li = new OpSPSearchVisitedLocalInfo();
      li->visitedSections = new DbArray<TupleId>(0);
      Source *pFrom = ( Source* ) args[0].addr;
      Target *pTo = ( Target* ) args[1].addr;
      bool dijkstra = ((CcBool*)args[2].addr)->GetBoolval();
      bool success = false;
      GLine* pGLine = new GLine(0);
      li->pNetwork =
        NetworkManager::GetNetworkNew(pFrom->GetNetworkId(),netList);
      if (dijkstra)
      {
        success = false;
      }
      else
      {
        success = pFrom->ShortestPathAStar( pTo, pGLine,
                                              li->pNetwork,
                                              li->visitedSections);
      }
      if(!success)
      {
        li->visitedSections->Destroy();
        delete li->visitedSections;
        li->visitedSections = 0;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        delete li;
        li = 0;
      }
      pGLine->DeleteIfAllowed();
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      if (local.addr != 0)
        li = (OpSPSearchVisitedLocalInfo*) local.addr;
      else
        return CANCEL;
      if (li->pos < 0 || li->pos >= li->visitedSections->Size())
        return CANCEL;
      else
      {
        TupleId actTID;
        li->visitedSections->Get(li->pos,actTID);
        Tuple* actTuple = li->pNetwork->GetSection(actTID);
        result.setAddr(actTuple);
        li->pos++;
        return YIELD;
      }
      break;
    }

    case CLOSE:
    {
      if (local.addr != 0)
      {
        li = (OpSPSearchVisitedLocalInfo*) local.addr;
        NetworkManager::CloseNetwork(li->pNetwork);
        li->pNetwork = 0;
        li->visitedSections->Destroy();
        delete li->visitedSections;
        li->visitedSections = 0;
        delete li;
        li = 0;
        local.addr = 0;
      }
      return 0;
      break;
    }
  }
  return 0; //should never been reached
}

ValueMapping OpSpvisitedmap[] =
{
  OpSPSearchVisitedValueMap_gpgp,
  OpSPSearchVisitedValueMap_glgl,
  OpSPSearchVisitedValueMap<GPoints, GPoints>,
  OpSPSearchVisitedValueMap<GPoint, GLine>,
  OpSPSearchVisitedValueMap<GPoint, GPoints>,
  OpSPSearchVisitedValueMap<GLine, GPoint>,
  OpSPSearchVisitedValueMap<GLine, GPoints>,
  OpSPSearchVisitedValueMap<GPoints, GPoint>,
  OpSPSearchVisitedValueMap<GPoints, GLine>
};

int OpSpvisitedselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 1;
  if (nl->SymbolValue (arg1) == GPoints::BasicType() &&
      nl->SymbolValue (arg2) == GPoints::BasicType())
    return 2;
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() &&
          nl->SymbolValue ( arg2 ) == GLine::BasicType() )
    return 3;
   if (nl->SymbolValue (arg1) == GPoint::BasicType() &&
      nl->SymbolValue (arg2) == GPoints::BasicType())
    return 4;
   if ( nl->SymbolValue ( arg1 ) == GLine::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 5;
   if (nl->SymbolValue (arg1) == GLine::BasicType() &&
      nl->SymbolValue (arg2) == GPoints::BasicType())
    return 6;
   if ( nl->SymbolValue ( arg1 ) == GPoints::BasicType() &&
          nl->SymbolValue ( arg2 ) == GPoint::BasicType() )
    return 7;
   if (nl->SymbolValue (arg1) == GPoints::BasicType() &&
      nl->SymbolValue (arg2) == GLine::BasicType())
    return 8;
  return -1; // This point should never be reached
};

struct spsearchvisitedInfo:OperatorInfo{
  spsearchvisitedInfo():OperatorInfo(){
    name = "spsearchvisited";
    signature = "gpoint x gpoint x bool->stream(tuple(X)))";
    appendSignature("gline x gline x bool->stream(tuple(X)))");
    appendSignature("gpoints x gpoints x bool->stream(tuple(X)))");
    appendSignature("gpoint x gline x bool->stream(tuple(X)))");
    appendSignature("gpoint x gpoints x bool->stream(tuple(X)))");
    appendSignature("gline x gpoint x bool->stream(tuple(X)))");
    appendSignature("gline x gpoints x bool->stream(tuple(X)))");
    appendSignature("gpoints x gpoint x bool->stream(tuple(X)))");
    appendSignature("gpoints x gline x bool->stream(tuple(X)))");
    syntax = "spsearchvisited(_,_,_)";
    meaning = "Stream of network sections visited by shortestpath";
  }
};


/*
6.23 Operator ~shortestpathtree~

Returns the complete ~shortestpathtree~ in the network from the source.
If third argument is true the complete shortest path tree is computed,
If third arguemtn is false the computation stops if all parts of the target
have been reached.

*/

ListExpr OpShortestpathtreeTypeMap ( ListExpr args )
{
  NList param(args);

  if(param.length() != 2 )
    return listutils::typeError("2 arguments expected.");

  NList source(param.first());
  if (!source.isSymbol(GPoint::BasicType()))
    return listutils::typeError("First argument should be gpoint.");

  NList network(param.second());
  if (!network.isSymbol("network"))
    return listutils::typeError("Second argument should be a network.");

  //return nl->SymbolAtom(GLine::BasicType());

  NList secID("SecID");
  NList intVal(CcInt::BasicType());
  NList intAttr(secID, intVal);
  NList dist("Dist");
  NList realVal(CcReal::BasicType());
  NList distAttr(dist,realVal);
  NList up("Up");
  NList upVal(CcBool::BasicType());
  NList upAttr(up,upVal);
  NList tupleType(intAttr,distAttr,upAttr);
  return NList().tupleStreamOf(tupleType).listExpr();
}

struct ShortestPathTreeInfo
{
  ShortestPathTreeInfo()
  {
    pos = 2;
    resArray = 0;
  }

  ShortestPathTreeInfo(Network *pNetwork)
  {
    resArray =
      new DbArray<ShortestPathTreeEntry> (2*pNetwork->GetNoSections() + 1);
    ShortestPathTreeEntry spEntryUp(numeric_limits< double >::max(),true);
    ShortestPathTreeEntry spEntryDown(numeric_limits< double >::max(),
                                      false);
    for (int i = 0; i < resArray->Size(); i++)
    {
      if (i%2 == 0)
        resArray->Put(i,spEntryUp);
      else
        resArray->Put(i,spEntryDown);
    }
    pos = 2;
  }

  void Destroy()
  {
    resArray->Destroy();
  }

  ~ShortestPathTreeInfo(){};

  DbArray<ShortestPathTreeEntry> *resArray;
  int pos;
  TupleType* resTupleTyp;
};

int OpShortestpathtreeValueMap_gp(Word* args, Word& result, int message,
                               Word& local, Supplier in_pSupplier)
{
  ShortestPathTreeInfo *localinfo = 0;

  switch(message)
  {

     case OPEN:
     {
      GPoint* pSource = (GPoint*) args[0].addr;
      Network* pNetwork = (Network*) args[1].addr;
      if (pSource != 0 && pSource->IsDefined() &&
          pNetwork != 0 && pNetwork->IsDefined())
      {
        localinfo = new ShortestPathTreeInfo(pNetwork);
        pSource->ShortestPathTree(pNetwork,localinfo->resArray);
        ListExpr resultType = GetTupleResultType( in_pSupplier );
        localinfo->resTupleTyp = new TupleType(nl->Second(resultType));
      }
      local = SetWord(localinfo);
      return 0;
      break;
     }

     case REQUEST:
     {
        localinfo = (ShortestPathTreeInfo*) local.addr;
        if (localinfo != 0 && localinfo->pos < localinfo->resArray->Size())
        {

          Tuple *res = new Tuple(localinfo->resTupleTyp);
          if ((localinfo->pos % 2) == 0)
            res->PutAttribute(0,new CcInt(true,localinfo->pos / 2));
          else
            res->PutAttribute(0,new CcInt(true,(localinfo->pos - 1)/2));
          ShortestPathTreeEntry spTEntry;
          localinfo->resArray->Get(localinfo->pos,spTEntry);
          res->PutAttribute(1,new CcReal(true, spTEntry.GetDist()));
          res->PutAttribute(2,new CcBool(true, spTEntry.GetUpDown()));
          localinfo->pos++;
          result.setAddr(res);
          return YIELD;
        }
        else
          return CANCEL;
        break;
     }

     case CLOSE:
     {
       localinfo = (ShortestPathTreeInfo*) local.addr;
       if (localinfo != 0)
        {
          localinfo->Destroy();
          delete localinfo->resArray;
          localinfo->resArray = 0;
          if (localinfo->resTupleTyp != 0)
          {
            localinfo->resTupleTyp->DeleteIfAllowed();
            localinfo->resTupleTyp = 0;
          }
          delete localinfo;
          localinfo = 0;
          local.setAddr(0);
        }
        return 0;
        break;
     }
     default:
       return 0;
  }
}

ValueMapping OpShortestpathtreeMap[] =
{
  OpShortestpathtreeValueMap_gp,
};

int OpShortestpathtreeSelect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  if ( nl->SymbolValue ( arg1 ) == GPoint::BasicType() )
    return 0;
  return -1; // This point should never be reached
};

struct shortestpathtreeInfo:OperatorInfo{
  shortestpathtreeInfo():OperatorInfo(){
    name = "shortestpathtree";
    signature = "gpoint X network -> stream(tupel((int)(real)))";
    syntax = "shortestpathtree(_,_)";
    meaning = "Returns the shortestpathtree from source.";
  }
};

/*
6.24 Operator ~getAdjacentSections~

Returns the adjacent Sections of the given section.

*/

ListExpr OpGetAdjacentSectionsTypeMap ( ListExpr args )
{
  NList param(args);

  if(param.length() != 3 )
    return listutils::typeError("3 arguments expected.");

  NList network(param.first());
  if (!network.isSymbol("network"))
    return listutils::typeError("First argument should be network.");

  NList source(param.second());
  if (!source.isSymbol(CcInt::BasicType()))
    return listutils::typeError("Second argument should be int.");

  NList direction(param.third());
  if (!direction.isSymbol(CcBool::BasicType()))
    return listutils::typeError("Third argument should be bool.");

  //return nl->SymbolAtom(GLine::BasicType());

  NList secID("SecID");
  NList intVal(CcInt::BasicType());
  NList intAttr(secID, intVal);
  NList up("Up");
  NList upVal(CcBool::BasicType());
  NList upAttr(up,upVal);
  NList tupleType(intAttr,upAttr);
  return NList().tupleStreamOf(tupleType).listExpr();
}

/*
Helper Struct for Adjacenc Section Lists.

*/

struct AdjacentSectionsInfo
{
  AdjacentSectionsInfo()
  {
    pos = 0;
    resArray = new DbArray<SectionValue>(0);
  }

  void Destroy()
  {
    resArray->Destroy();
  }

  ~AdjacentSectionsInfo(){};

  DbArray<SectionValue> *resArray;
  int pos;
  TupleType* resTupleTyp;
};

int OpGetAdjacentSectionsValueMap(Word* args, Word& result, int message,
                                  Word& local, Supplier in_pSupplier)
{
  AdjacentSectionsInfo *localinfo = 0;

  switch(message)
  {

    case OPEN:
    {
      Network* pNetwork = (Network*) args[0].addr;
      int sectId =
      ((CcInt*) args[1].addr)->GetIntval();
      bool upDownFlag =
      ((CcBool*) args[2].addr)->GetBoolval();
      if (pNetwork != 0 && pNetwork->IsDefined())
      {
        localinfo = new AdjacentSectionsInfo();

        pNetwork->GetAdjacentSections(sectId, upDownFlag,
                                      localinfo->resArray);

        ListExpr resultType = GetTupleResultType( in_pSupplier );
        localinfo->resTupleTyp = new TupleType(nl->Second(resultType));
      }
      local = SetWord(localinfo);
      return 0;
      break;
    }

    case REQUEST:
    {
      localinfo = (AdjacentSectionsInfo*) local.addr;
      if (localinfo != 0 && localinfo->pos < localinfo->resArray->Size())
      {
        SectionValue adjSectEntry;
        localinfo->resArray->Get(localinfo->pos,adjSectEntry);
        Tuple *res = new Tuple(localinfo->resTupleTyp);
        res->PutAttribute(0,new CcInt(true,adjSectEntry.GetSectionID()));
        res->PutAttribute(1,new CcBool(true,adjSectEntry.GetUpDownFlag()));
        localinfo->pos++;
        result.setAddr(res);
        return YIELD;
      }
      else
        return CANCEL;
      break;
    }

    case CLOSE:
    {
      localinfo = (AdjacentSectionsInfo*) local.addr;
      if (localinfo != 0)
      {
        localinfo->Destroy();
        delete localinfo->resArray;
        localinfo->resArray = 0;
        if (localinfo->resTupleTyp != 0)
        {
          localinfo->resTupleTyp->DeleteIfAllowed();
          localinfo->resTupleTyp = 0;
        }
        delete localinfo;
        localinfo = 0;
        local.setAddr(0);
      }
      return 0;
      break;
    }
    default:
      return 0;
  }
}

struct getAdjacentSectionsInfo:OperatorInfo{
  getAdjacentSectionsInfo():OperatorInfo(){
    name = "getAdjacentSections";
    signature = "network X int X bool -> stream(tupel((int)(bool)))";
    syntax = "getAdjacentSections(_,_,_)";
    meaning = "Returns the adjacent sections in given direction.";
  }
};

/*
6.25 Operator ~getReverseAdjacentSections~

Returns the reverse adjacent Sections of the given section.

*/

int OpGetReverseAdjacentSectionsValueMap(Word* args, Word& result, int message,
                                  Word& local, Supplier in_pSupplier)
{
  AdjacentSectionsInfo *localinfo = 0;

  switch(message)
  {

    case OPEN:
    {
      Network* pNetwork = (Network*) args[0].addr;
      int sectId =
      ((CcInt*) args[1].addr)->GetIntval();
      bool upDownFlag =
      ((CcBool*) args[2].addr)->GetBoolval();
      if (pNetwork != 0 && pNetwork->IsDefined())
      {
        localinfo = new AdjacentSectionsInfo();

        pNetwork->GetReverseAdjacentSections(sectId, upDownFlag,
                                      localinfo->resArray);

        ListExpr resultType = GetTupleResultType( in_pSupplier );
        localinfo->resTupleTyp = new TupleType(nl->Second(resultType));
      }
      local = SetWord(localinfo);
      return 0;
      break;
    }

    case REQUEST:
    {
      localinfo = (AdjacentSectionsInfo*) local.addr;
      if (localinfo != 0 && localinfo->pos < localinfo->resArray->Size())
      {
        SectionValue adjSectEntry;
        localinfo->resArray->Get(localinfo->pos,adjSectEntry);
        Tuple *res = new Tuple(localinfo->resTupleTyp);
        res->PutAttribute(0,new CcInt(true,adjSectEntry.GetSectionID()));
        res->PutAttribute(1,new CcBool(true,adjSectEntry.GetUpDownFlag()));
        localinfo->pos++;
        result.setAddr(res);
        return YIELD;
      }
      else
        return CANCEL;
      break;
    }

    case CLOSE:
    {
      localinfo = (AdjacentSectionsInfo*) local.addr;
      if (localinfo != 0)
      {
        localinfo->Destroy();
        delete localinfo->resArray;
        localinfo->resArray = 0;
        if (localinfo->resTupleTyp != 0)
        {
          localinfo->resTupleTyp->DeleteIfAllowed();
          localinfo->resTupleTyp = 0;
        }
        delete localinfo;
        localinfo = 0;
        local.setAddr(0);
      }
      return 0;
      break;
    }
    default:
      return 0;
  }
}

struct getReverseAdjacentSectionsInfo:OperatorInfo{
  getReverseAdjacentSectionsInfo():OperatorInfo(){
    name = "getReverseAdjacentSections";
    signature = "network X int X bool -> stream(tupel((int)(bool)))";
    syntax = "getReverseAdjacentSections(_,_,_)";
    meaning = "Returns the reverse adjacent sections.";
  }
};

/*
6.26 Operators ~circle~, ~incircle~, ~outcircle~

6.26.1 TypeMapping for all operators

*/
ListExpr circleTypeMap ( ListExpr args )
{
  NList param(args);

  if(param.length() != 2 )
    return listutils::typeError("2 arguments expected.");

  NList gp(param.first());
  if (!gp.isSymbol(GPoint::BasicType()))
    return listutils::typeError("First argument should be gpoint.");

  NList dist(param.second());
  if (!dist.isSymbol(CcReal::BasicType()))
    return listutils::typeError("Second argument should be real.");

  return nl->SymbolAtom(GLine::BasicType());
}

/*
6.26.2 ValueMappings and OperatorInfos for cirlce-Operators

6.26.2.1 ~circle~

*/
int circleValueMap(Word* args, Word& result, int message,
                   Word& local, Supplier in_pSupplier)
{
  GPoint* gp = (GPoint*) args[0].addr;
  double maxdist = ((CcReal*)args[1].addr)->GetRealval();
  if (!gp->IsDefined()) return 0;
  Network *pNetwork = NetworkManager::GetNetworkNew(gp->GetNetworkId(),netList);
  result = qp->ResultStorage(in_pSupplier);
  GLine* pGLine = static_cast<GLine*>(result.addr);
  gp->Circle(pNetwork, pGLine, maxdist);
  NetworkManager::CloseNetwork(pNetwork);
  return 1;
}

struct circleInfo:OperatorInfo{
  circleInfo():OperatorInfo(){
    name = "circlen";
    signature = "gpoint X real -> gline";
    syntax = "circlen(_,_)";
    meaning = "Network part within dist around gpoint.";
  }
};

/*
6.26.2.2 ~incircle~

*/
int inCircleValueMap(Word* args, Word& result, int message,
                   Word& local, Supplier in_pSupplier)
{
  GPoint* gp = (GPoint*) args[0].addr;
  double maxdist = ((CcReal*)args[1].addr)->GetRealval();
  if (!gp->IsDefined()) return 0;
  Network *pNetwork = NetworkManager::GetNetworkNew(gp->GetNetworkId(),netList);
  result = qp->ResultStorage(in_pSupplier);
  GLine* pGLine = static_cast<GLine*>(result.addr);
  gp->In_Circle(pNetwork, pGLine, maxdist);
  NetworkManager::CloseNetwork(pNetwork);
  return 1;
}

struct inCircleInfo:OperatorInfo{
  inCircleInfo():OperatorInfo(){
    name = "in_circlen";
    signature = "gpoint X real -> gline";
    syntax = "in_circlen(_,_)";
    meaning = "Network part gpoint can be reached within dist.";
  }
};

/*
6.26.2.3 ~outcircle~

*/

int outCircleValueMap(Word* args, Word& result, int message,
                     Word& local, Supplier in_pSupplier)
{
  GPoint* gp = (GPoint*) args[0].addr;
  double maxdist = ((CcReal*)args[1].addr)->GetRealval();
  if (!gp->IsDefined()) return 0;
  Network *pNetwork = NetworkManager::GetNetworkNew(gp->GetNetworkId(),netList);
  result = qp->ResultStorage(in_pSupplier);
  GLine* pGLine = static_cast<GLine*>(result.addr);
  gp->Out_Circle(pNetwork, pGLine, maxdist);
  NetworkManager::CloseNetwork(pNetwork);
  return 1;
}

struct outCircleInfo:OperatorInfo{
  outCircleInfo():OperatorInfo(){
    name = "out_circlen";
    signature = "gpoint X real -> gline";
    syntax = "out_circlen(_,_)";
    meaning = "Network part within dist of gpoint.";
  }
};

/*
5.28 Operator ~bbox~

Returns the spatial bounding box of the ~network~

*/

ListExpr bboxTypeMap(ListExpr in_xArgs)
{
  NList param(in_xArgs);

  if (param.length() != 1)
    return listutils::typeError("one argument expected");

  if (param.first().isSymbol(Network::BasicType()))
    return nl->SymbolAtom( Rectangle<2>::BasicType() );
  else
    return listutils::typeError("network expected");
}

int bboxValueMap( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
  result = qp->ResultStorage( s );
  Rectangle<2>* box = static_cast<Rectangle<2>* >(result.addr);
  Network* arg = static_cast<Network*>(args[0].addr);
  if(!arg->IsDefined()){
    box->SetDefined(false);
  } else {
    *box = arg->BoundingBox();
  }
  return 0;
}

struct OpBboxInfo:OperatorInfo{
  OpBboxInfo(){
    name = "netbbox";
    signature = "network -> rect2";
    syntax = "netbbox(_)";
    meaning = "Returns the spatial bounding box of the network.";
  }
};

/*
7 Creating the ~NetworkAlgebra~

*/

class NetworkAlgebra : public Algebra
{
  public:
    NetworkAlgebra() : Algebra()
    {
      AddTypeConstructor ( &networkTC );
      AddTypeConstructor ( &gpointTC);
      AddTypeConstructor ( &glineTC );
      AddTypeConstructor ( &gpointsTC );

      gpointTC.AssociateKind ( Kind::DATA() );
      glineTC.AssociateKind ( Kind::DATA() );
      networkTC.AssociateKind ( Kind::NETWORK() );
      gpointsTC.AssociateKind ( Kind::DATA() );

      AddOperator ( lengthInfo(), OpLengthValueMap, OpLengthTypeMap);
      AddOperator ( noComponentsInfo(), OpNoComponentsValueMapping,
                    OpNoComponentsTypeMap);
      AddOperator ( isEmptyInfo(), OpNetIsEmptyValueMap, OpNetIsEmptyTypeMap);
      AddOperator ( &theNetwork);
      AddOperator ( routesInfo(), OpNetworkRoutesValueMapping,
                    OpNetworkRoutesTypeMap);
      AddOperator ( junctionsInfo(), OpNetworkJunctionsValueMapping,
                    OpNetworkJunctionsTypeMap);
      AddOperator ( sectionsInfo(), OpNetworkSectionsValueMapping,
                    OpNetworkSectionsTypeMap);
      AddOperator ( insideInfo(), OpInsideValueMap, OpInsideTypeMap);
      AddOperator ( netdistanceInfo(), OpNetNetdistancemap,
                    OpNetNetdistanceselect, OpNetNetdistanceTypeMap);
      AddOperator ( netdistanceNewInfo(), OpNetNetdistanceNewmap,
                    OpNetNetdistanceNewselect, OpNetNetdistanceNewTypeMap);
      AddOperator ( point2gpointInfo(), OpPoint2GPointValueMapping,
                    OpPoint2GPointTypeMap);
      AddOperator ( gpoint2pointInfo(), OpGPoint2PointValueMapping,
                    OpGPoint2PointTypeMap);
      AddOperator ( equalInfo(), OpNetEqualmap, OpNetEqualselect,
                    OpNetEqualTypeMap);
      AddOperator ( line2glineInfo(), OpLine2GLineValueMapping,
                    OpLine2GLineTypeMap);
      AddOperator ( polygpointsInfo(), OpPolyGPointValueMapping,
                    OpPolyGPointTypeMap);
      AddOperator ( routeIntervalsInfo(), OpRouteIntervalsValueMapping,
                    OpRouteIntervalsTypeMap);
      AddOperator ( intersectsInfo(), OpNetIntersectsValueMapping,
                    OpNetIntersectsTypeMap);
      AddOperator ( gpoint2rectInfo(), OpGPoint2RectValueMapping,
                    OpGPoint2RectTypeMap);
      AddOperator ( gline2lineInfo(), OpGLine2LineValueMapping,
                    OpGLine2LineTypeMap);
      AddOperator ( unionInfo(), OpNetUnionValueMap, OpNetUnionTypeMap);
      AddOperator ( distanceInfo(), OpNetDistancemap, OpNetDistanceselect,
                    OpNetDistanceTypeMap);
      AddOperator ( shortestpathInfo(), OpShortestPathValMap,
                    OpShortestPathSelect, OpShortestPathTypeMap);
      AddOperator ( shortestpathAstarInfo(), OpShortestPathAStarMap,
                    OpShortestPathAStarSelect, OpShortestPathAStarTypeMap);
      AddOperator ( getBGPInfo(), OpGetBGPValueMap, OpGetBGPTypeMap);
      AddOperator ( spsearchvisitedInfo(), OpSpvisitedmap, OpSpvisitedselect,
                    OpSPSearchVisitedTypeMap);
      AddOperator ( shortestpathtreeInfo(), OpShortestpathtreeMap,
                    OpShortestpathtreeSelect, OpShortestpathtreeTypeMap);
      AddOperator ( getAdjacentSectionsInfo(), OpGetAdjacentSectionsValueMap,
                    OpGetAdjacentSectionsTypeMap);
      AddOperator ( getReverseAdjacentSectionsInfo(),
                    OpGetReverseAdjacentSectionsValueMap,
                    OpGetAdjacentSectionsTypeMap);
      AddOperator ( inCircleInfo(), inCircleValueMap, circleTypeMap);
      AddOperator ( outCircleInfo(), outCircleValueMap, circleTypeMap);
      AddOperator ( circleInfo(), circleValueMap, circleTypeMap);
      AddOperator ( OpBboxInfo(), bboxValueMap, bboxTypeMap);
    }

    ~NetworkAlgebra()
    {
      delete netList;
      netList = 0;
    }
};

/*
Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
  Algebra*
  InitializeNetworkAlgebra ( NestedList* nlRef,
                             QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  netList = new map<int,string>();
  return ( new NetworkAlgebra() );
}
