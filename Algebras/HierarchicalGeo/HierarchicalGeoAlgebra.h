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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file of the Temporal Algebra

April 2007 Sascha Vaut

1 Overview

The type system of the HierarchicalGeo Algebra can be seen below.



2 Defines, includes, and constants

*/
#ifndef _HIERARCHICALGEO_ALGEBRA_H_
#define _HIERARCHICALGEO_ALGEBRA_H_
#endif

#include <iostream>
#include <sstream>
#include <string>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "SpatialAlgebra.h"
#include "DBArray.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace datetime;






/*
3 C++ Classes (Definition)

3.1 Uncertain

This class represents an epsilon-value of type real. It will be used in type 
constructors of all ~uncertain~ Datatypes.

*/
template <class Alpha>
class Uncertain
{
  
  public:
  
/*
3.1.1 Constructors

*/  
  Uncertain() {}
/*
The simple constructor. This constructor should not be used.

*/

  Uncertain( const double& epsilon, const Alpha& alpha ):
    epsilon ( epsilon ),
    value (),
    defined ( true )
  {
    value.CopyFrom( &alpha );
  }

/*
The creation of the uncertain, setting all attributes.

*/
  Uncertain( const double& epsilon):
    epsilon ( epsilon ),
    value (),
    defined ( false )
  {}

/*
The creation of an uncertain value, setting only the epsilon value.

3.1.2 Member functions

*/

  
/* +++++ noch ausgeschaltet +++++++++++++++++++++++++++++++++++++
  void CopyFrom( const Uncertain<Alpha>& uncertain )
  {
    const Uncertain<Alpha> *u = (const Uncertain<Alpha>*)uncertain;
    
    defined = u->defined;
    if ( defined )
    {
      epsilon = u->epsilon;
      value.CopyFrom( &u->value );
    }
  }
+++++ noch ausgeschaltet +++++++++++++++++++++++++++++++++++++*/
  
  bool IsValid()
  {
    if (epsilon >= 0 && defined)
      return true;
    return false;
  }
    
/*
Checks if the Uncertain is valid or not. This function should be used for debugging purposes
only. An uncertain is valid if the following conditions are true:

  1 ~alpha~ and ~epsilon~ are defined
  
  2 ~epsilon~ $>=$ 0
  
  3 ~defined~ $==$ TRUE
  
*/

  double GetEpsilon()
  {
    return epsilon;
  }

/*
Returns the epsilon value of the Uncertain value.

*/

  bool IsDefined()
  {
    return defined;
  }

/*
Checks if the Uncertain value is defined or not. If it is not defined, only the
epsilon value is set. The Alpha value is left to be set later.

*/

  // +++++ evtl. auf "protected" setzen! +++++++++++++++++++++++++++++
  void SetDefined( bool def )
  {
    this->defined = def;
  }
  
/*
Sets the argument ~defined~ to the given boolean value.

*/


//  Uncertain<Alpha>& operator=( const Uncertain<Alpha>& c);
/*
Redefinition of the copy operator ~=~.

*/

//  bool operator==( const Uncertain<Alpha>& c ) const;
/*
Returns ~true~ if the uncertain value is equal to ~c~ and ~false~ if they are
different.

*/

//  bool operator!=( const Uncertain<Alpha>& c ) const;
/*
Returns ~true~ if the uncertain value is different to ~c~ and ~false~ if they 
are equal.

*/

//  bool PossiblyIntersects( const Uncertain<Alpha>& c ) const;
/*
Returns ~true~ if the uncertain value may intersect ~c~ (if they may have the 
same values) and ~false~ if they are distinct.

*/

//  void Intersection( const Uncertain<Alpha>& c, 
//                      Uncertain<Alpha>& result ) const;
/*
Returns an uncertain value, representing the Intersection of this uncertain 
value and ~c~ into ~result~.

*/

  void Epsilon( CcReal& result);
/*
Returns the uncertain value's ~epsilon value~.

*/

/*
3.1.3 Attributes

*/
  // +++++ protected führt zu fehlern in der Out-function ++++++++++
  // +++++ daher ausgeschaltet: ++++++++++++++++++++++++
  //protected:

  double epsilon;
/*
The possible difference between the original value and the given value.

*/

  Alpha value;
/*
The value that is uncertain.

*/

  bool defined;
/*
The flag that indicates if the value is defined or not.

*/

};

/*
3.2 CBool

This datatype represents an ~uncertain~ boolean value. To define a ~boolean 
uncertainty~ a 3rd State (beyond the two known States TRUE and FALSE), called 
MAYBE, is introduced. 

*/

#define CBool int
#define maybe 2

/*
+++++++++++++++++++++++++++++++++++++++++++++++++ 
Ob der Integer-Wert 2 für maybe praktikabel ist, ist noch fraglich.
Alternative: -1
Ebenso ist fraglich, welche Operationen für diesen neuen Typ ggf. noch
erforderlich sind. 
+++++++++++++++++++++++++++++++++++++++++++++++++



3.3 CPoint

CPoint represents a Point value containing an epsilon value. It implements 
Uncertain.

*/
class CPoint : public Uncertain<Point>
{
  public:
/*
3.3.1 Constructors and Destructor
  
*/

  inline CPoint() {}
  
/*
The default constructor which should not be used.

*/
  
  CPoint(const double epsilon):
      Uncertain<Point>( epsilon ) 
      {}
      
/*
The undefined constructor. Only the epsilon value is set. The point value is 
left undefined for later definition.

*/

/* +++++ noch ausgeschaltet ++++++++++++++++++++
  CPoint( const double epsilon, const double x, const double y):
    Uncertain<Point>( epsilon, value(true, x, y) ) {}
+++++ noch ausgeschaltet ++++++++++++++++++++++

*/

/*
This constructor creates an uncertain point value from the given coordinates
and the epsilon value.

*/
  
  CPoint( const double epsilon, const StandardAttribute* point):
      Uncertain<Point>( epsilon ) 
      {
        value.CopyFrom(point);
        SetDefined( true );
      }
/*
The copy-constructor.

*/
  
  inline ~CPoint() {}
/*
The destructor.

3.3.2 Member functions

*/
  
  
  
  int ToCPoint( Word* args, Word& result, int message, Word& local,
                                        Supplier s );

/*
Transforms a given Point and a given positive real-value to a new cpoint-value.

*/
    
  void Set( CPoint cp )
  {
    this->epsilon = cp.epsilon;
    this->value = cp.value;
    this->defined = true;
  }
  
//  inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the uncertain point, i.e. a rectangle area, 
bounding the area where the point may be.

*/
  
};


/*
3.4 CUPoint

This class will be used in the ~cupoint~ type constructor, i.e., the type constructor
for the uncertain temporal unit of point values.

*/
class CUPoint : public Uncertain< UPoint >
{
  public:
/*
3.4.1 Constructors and Destructor

*/
  CUPoint() {}
/*
The simple constructor. This constructor should not be used.

*/
  
  CUPoint( const bool is_defined ):
      Uncertain<UPoint>(is_defined) 
  {
    SetDefined( is_defined );
    
  }
  
  CUPoint( const double epsilon ):
      Uncertain<UPoint>(epsilon) 
  {
    SetDefined( true );
    
  }
/*
The simple constructor, only defining the epsilon-value. 
  
*/
  
  CUPoint( const double epsilon, const StandardAttribute* upoint):
      Uncertain<UPoint>( epsilon ) 
  {
    value.CopyFrom(upoint);
    SetDefined( true );
  }
/*
The copy-constructor.
  
*/
  CUPoint( const double epsilon, const Interval<Instant>& interval,
      const Point& p0, const Point& p1 ):
    Uncertain<UPoint> (epsilon)
    {
      value.timeInterval = interval;
      value.p0 = p0;
      value.p1 = p1;
      SetDefined( true );
    }

  CUPoint( const double epsilon, const Interval<Instant>& interval,
      const double x0, const double y0,
      const double x1, const double y1 ):
    Uncertain<UPoint> (epsilon)
    {
      value.timeInterval = interval;
      value.p0.Set( (Coord&) x0, (Coord&) y0);
      value.p0.SetDefined( true );
      value.p1.Set( (Coord&) x1, (Coord&) y1);
      value.p1.SetDefined( true );
    }
  
  inline virtual ~CUPoint() {}
  
/*
The destructor.

3.4.2 Operator redefinitions

*/
  
  
  
/*
3.4.3 The Temporal Functions

*/
  
  
  
/*
3.4.4 Functions to be part of relations

*/
  
  inline virtual size_t Sizeof() const
  {
    return sizeof( *this );
  }
  
  
   inline virtual void CopyFrom( const StandardAttribute* right )
  {
    const CUPoint* i = (const CUPoint*)right;
    
    defined = i->defined;
    if( IsDefined () )
      epsilon = i->epsilon;
    else
      epsilon = 0.0;

    value.SetDefined( i->value.IsDefined() );
    if(i->value.IsDefined() )
      {
        const UPoint u = i->value;
        value = u;
      }
    else
      {
        value.timeInterval = Interval<Instant>();
        value.p0 = Point( false, 0.0, 0.0);
        value.p1 = Point( false, 0.0, 0.0);
      }
  }
  
  
  
  
};

/*
3.5 CMPoint

the implementation of an uncertain MPoint

*/


/*
3.6 HierarchicalMapping

a template class to bind all (uncertain) representations of one object to one 
HierarchicalMapping object. 

Attributes: 
- iDX: DBArray
- eLEM: DBArray
- canDestroy : bool
- ordered : bool

*/


/*
3.7 HMPoint

the HierarchicalMovingPoint Type, containing a set of CUPoints from which
every Generalization of the corresponding MPoint can be extracted. This
type also contains the UPoints of the origin MPoint as CUPoint-Objects with
an epsilon-value = 0.

*/

/*
3.8 HCMPoint

the type HierarchicalUncertainMovingPoint is a restricted variation of the
type HierarchicalMovingPoint. It just contains those UPoints of the origin
MPoint, that are necessary to build a particular minimal Generalization of
this MPoint.

*/

/*
4 Type Constructors


4.1 Type Constructor ~uncertain~

Type ~uncertain~ represents a pair ( epsilon ( <Alpha> )).

4.1.1 List Representation

The list representation of an ~uncertain~ is

----    ( epsilon ( <Alpha> ) )
----

For example a cpoint:

----    ( 20.5 ( 329.456 22.289 ) )
----

4.2 Function describing the signature of the Type Constructor

4.2.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr,
                                         const ListExpr,
                                         const int, ListExpr&, bool& )>
Word InUncertain( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct)
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );             // the epsilon value
    ListExpr second = nl->Second( instance );  // the Alpha-Object
    {
      if ( nl->IsAtom(first) && nl->AtomType(first) == RealType )
      {
        correct = true;
       
        Alpha *value = (Alpha *)InFun( nl->TheEmptyList(),
                                        second, errorPos, 
                                        errorInfo, correct ).addr;
        if ( correct == false )
        {
          delete value;
          return SetWord( Address(0) );  
        }
        else  // if correct
        {
          Uncertain<Alpha> *uncertain = new Uncertain<Alpha> (
                                        nl->RealValue(first), *value);
         
          delete value;
          return SetWord( uncertain );
        }
      }
      else
      {
        errmsg = "InUncertain(): First arg must be of type Real.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      }  
    }
  }
  correct = false;
  return SetWord(Address (0) );
}


/*
4.2.3 ~Out~-function

*/
template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutUncertain( ListExpr typeInfo, Word value )
{
  Uncertain<Alpha>* uncertain = (Uncertain<Alpha>*)(value.addr);
  
  if( uncertain->IsDefined() )
    return nl->TwoElemList(
      nl->RealAtom( &uncertain->GetEpsilon()),
      OutFun( nl->TheEmptyList(), SetWord( &uncertain->value ) ) );
      // Up to now the previous line sems to contain an error:
      // If the OutUncertain-function is mentioned in the typeconstructor
      // ~uncertainpoint~ (see HierarchicalGeoAlgebra.cpp line 316)
      // the compiler returns the following error message:
      //     'HierarchicalGeoAlgebra.h: In function ‘ListExpr OutUncertain
      //      (ListExpr, Word) [with Alpha = Point, ListExpr (* OutFun)
      //      (ListExpr, Word) = OutPoint]’:
      //      HierarchicalGeoAlgebra.cpp:328:   instantiated from here
      //      HierarchicalGeoAlgebra.h:613: error: invalid lvalue in unary ‘&'
      // I got no idea for the reason of this message. (Sascha Vaut)
  else
    return nl->SymbolAtom("undef");
}

/*
4.2.4 ~Create~-function

*/
template <class Alpha>
Word CreateUncertain( const ListExpr typeInfo )
{
  return (SetWord( new Uncertain<Alpha>() ));
}


/*
4.2.5 ~Delete~-function

*/
template <class Alpha>
void DeleteUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain<Alpha> *)w.addr;
  w.addr = 0;
}

/*
4.2.6 ~Close~-function

*/
template <class Alpha>
void CloseUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain<Alpha> *)w.addr;
  w.addr = 0;
}

/*
4.2.7 ~Clone~-function

*/
template <class Alpha>
Word CloneUncertain( const ListExpr typeInfo, const Word& w )
{
  Uncertain<Alpha> *uncertain = (Uncertain<Alpha> *)w.addr;
  return SetWord( new Uncertain<Alpha>( *uncertain ) );
}

/*
4.2.8 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfUncertain()
{
  return sizeof(Uncertain<Alpha>);
}

/*
4.2.9 ~Cast~-function

*/
template <class Alpha>
void* CastUncertain(void* addr)
{
  return new (addr) Uncertain<Alpha>;
}


/*
4.3 Value mapping functions for class Uncertain


4.3.1 Value mapping functions of operator ~epsilon~

*/
template <class Alpha>
int UncertainEpsilon( Word* args, Word& result, int message, Word& local, 
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  Uncertain<Alpha>* u = (Uncertain<Alpha>*)args[0].addr;

  if( u->IsDefined() )
    ((CcReal*)result.addr)->Set( u->epsilon );
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}




/*
4.4 Value mapping functions of operator ~val~

*/
template <class Alpha>
int UncertainVal( Word* args, Word& result, int message, 
                                               Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Uncertain<Alpha>* u = (Uncertain<Alpha>*)args[0].addr;

  if( u->IsDefined() )
    ((Alpha*)result.addr)->CopyFrom( 
                    &((Uncertain<Alpha>*)args[0].addr)->value );
  else
    ((Alpha*)result.addr)->SetDefined( false );

  return 0;
}



