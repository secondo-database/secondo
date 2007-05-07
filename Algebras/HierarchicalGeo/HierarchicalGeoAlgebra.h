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

\begin{displaymath}
\begin{array}{lll}
        & \to \textrm{BASE}     & {\underline{\smash{\mathit{int}}}}, {\underline{\smash{\mathit{real}}}},
                                  {\underline{\smash{\mathit{bool}}}}, {\underline{\smash{\mathit{string}}}} \\
        & \to \textrm{SPATIAL}  & {\underline{\smash{\mathit{point}}}}, {\underline{\smash{\mathit{points}}}},
                                  {\underline{\smash{\mathit{line}}}}, {\underline{\smash{\mathit{region}}}} \\
        & \to \textrm{TIME}     & {\underline{\smash{\mathit{instant}}}} \\
\textrm{BASE} \cup \textrm{TIME}        & \to \textrm{RANGE}    & {\underline{\smash{\mathit{range}}}} \\
\textrm{BASE} \cup \textrm{SPATIAL}     & \to \textrm{TEMPORAL} & {\underline{\smash{\mathit{intime}}}},
                                                                  {\underline{\smash{\mathit{moving}}}}
\end{array}
\end{displaymath}

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

This class represents an epsylon-value of type real. It will be used in type 
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

  Uncertain( const double& epsylon, const Alpha& alpha ):
    epsylon ( epsylon ),
    value (),
    defined ( true )
  {
    value.CopyFrom( &alpha );
  }

/*
The creation of the uncertain, setting all attributes.

*/
  Uncertain( const double& epsylon):
    epsylon ( epsylon ),
    value (),
    defined ( false )
  {}

/*
The creation of an uncertain value, setting only the epsylon value.


3.1.2 Member functions

*/

/* +++++ noch ausgeschaltet +++++++++++++++++++++++++++++++++++++
  void CopyFrom( const Uncertain<Alpha>& uncertain )
  {
    const Uncertain<Alpha> *u = (const Uncertain<Alpha>*)uncertain;
    
    defined = u->defined;
    if ( defined )
    {
      epsylon = u->epsylon;
      value.CopyFrom( &u->value );
    }
  }
+++++ noch ausgeschaltet +++++++++++++++++++++++++++++++++++++*/
  
  bool IsValid()
  {
    if (epsylon > 0 && defined)
      return true;
    return false;
  }
    
/*
Checks if the Uncertain is valid or not. This function should be used for debugging purposes
only. An uncertain is valid if the following conditions are true:

  1 ~alpha~ and ~epsylon~ are defined
  
  2 ~epsylon~ $>=$ 0
  
  3 ~defined~ $==$ TRUE
  
*/

  double GetEpsylon()
  {
    return epsylon;
  }

/*
Returns the epsylon value of the Uncertain value.

*/

  bool IsDefined()
  {
    return defined;
  }

/*
Checks if the Uncertain value is defined or not. If it is not defined, only the
epsylon value is set. The Alpha value is left to be set later.

*/

  // +++++ evtl. auf "protected" setzen! +++++++++++++++++++++++++++++
  void SetDefined( bool defined )
  {
    this->defined = defined;
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

  void Epsylon( CcReal& result);
/*
Returns the uncertain value's ~epsylon value~.

*/

/*
3.1.3 Attributes

*/
  // +++++ protected führt zu fehlern in der Out-function ++++++++++
  // +++++ daher ausgeschaltet: ++++++++++++++++++++++++
  //protected:

  double epsylon;
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

*/


/*
3.3 CInt

pair: double epsylon, CcInt intvalue
implements Uncertain

*/

class CInt : public Uncertain<CcInt>
{

/* 
3.3.1 Constructors

*/


/*
3.3.2 Member Functions

*/

//  void possibleMinimum ( int result ) const;
/*
Returns the minimal possible integer value by subtracting the epsylon value 
from the given integer value and rounding up to the next integer.

*/  
  
//  void possibleMaximum ( int result ) const;
/*
Returns the maximal possible integer value by adding the epsylon value to the 
given integer value and rounding down to the next integer.

*/
  
};

/*
3.4 CReal

pair: double epsylon, CcReal realvalue
implements Uncertain

*/



/*
3.5 CPoint

CPoint represents a Point value containing an epsylon value. It implements 
Uncertain.

*/
class CPoint : public Uncertain<Point>
{
  public:
/*
3.5.1 Constructors and Destructor
  
*/

  inline CPoint() {}
  
/*
The default constructor which should not be used.

*/
  
  CPoint(const double epsylon):
      Uncertain<Point>( epsylon ) 
      {}
      
/*
The undefined constructor. Only the epsylon value is set. The point value is 
left undefined for later definition.

*/

/* +++++ noch ausgeschaltet ++++++++++++++++++++
  CPoint( const double epsylon, const double x, const double y):
    Uncertain<Point>( epsylon, value(true, x, y) ) {}
+++++ noch ausgeschaltet ++++++++++++++++++++++

*/

/*
This constructor creates an uncertain point value from the given coordinates
and the epsylon value.

*/
  
  CPoint( const double epsylon, const StandardAttribute* point):
      Uncertain<Point>( epsylon ) 
      {
        value.CopyFrom(point);
      }
/*
The copy-constructor.

*/
  
  inline ~CPoint() {}
/*
The destructor.

3.5.2 Member functions

*/
  
//  inline const Rectangle<2> BoundingBox() const;
/*
Returns the bounding box of the uncertain point, i.e. a rectangle area, 
bounding the area where the point may be.

*/

/* +++++ noch ausgeschaltet ++++++++++++++++++++++++++++  
  virtual CPoint& operator=( const CPoint& cp )
  {
    *((Uncertain<Point>*)this) = *((Uncertain<Point>*)&cp);
    // +++++ hier ggf. noch Epsylon-Wert kopieren +++++++
  }
  
+++++ noch ausgeschaltet +++++++++++++++++++++++++++++

*/


/*
Redefinition of the copy operator ~=~.

*/
  
/* +++++ noch ausgeschaltet ++++++++++++++++++++++++++++
  virtual bool operator==( const CPoint& cp ) const
  {
    return *((Uncertain<Point>*)this) == *((Uncertain<Point>*)&cp) &&
    // +++++ hier noch die Punkt- und Epsylonwerte vergleichen +++++
  }
  
+++++ noch ausgeschaltet ++++++++++++++++++++++++++++++

*/
  
  
};


/*
3.6 CHalfSegment

*/


/*
3.7 CPoints

*/



/*
3.8 CLine

*/


/*
3.9 CRegion

*/


/*
3.10 SpatialUncertainUnit

a template class to connect the uncertain object to the objects it generalizes.
implements Uncertain
Attributes: 
- origin : Interval

*/



/*
3.11 CUBool

the implementation of an uncertain UBool

*/



/*
3.12 CUInt

the implementation of an uncertain UInt

*/


/*
3.13 CUReal

the implementation of an uncertain UReal

*/


/*
3.14 CUPoint

the implementation of an uncertain UPoint

*/



/*
3.15 CMBool

the implementation of an uncertain MBool

*/



/*
3.16 CMInt

the implementation of an uncertain MInt

*/



/*
3.17 CMReal

the implementation of an uncertain MReal 

*/



/*
3.18 CMPoint

the implementation of an uncertain MPoint

*/


/*
3.19 HierarchicalMapping

a template class to bind all (uncertain) representations of one object to one 
HierarchicalMapping object. 

Attributes: 
- uncertain : DBArray
- canDestroy : bool
- ordered : bool

*/


/*
3.20 HMPointLine

the HierarchicalMovingPoint Type, which binds all (uncertain) representations 
of one MPoint implements HierarchicalMapping

*/



/*
3.21 HLine

the HierarchicalLine Type, which binds all (uncertain) representations of one 
Line implements HierarchicalMapping

*/



/*
3.22 HRegion

the HierarchicalRegion Type, which binds all (uncertain) representations of 
one Region implements HierarchicalMapping

*/


/*
5.1 Type Constructor ~uncertain~

Type ~uncertain~ represents a pair ( epsylon ( <Alpha> )).

5.1.1 List Representation

The list representation of an ~uncertain~ is

----    ( epsylon ( <Alpha> ) )
----

For example a cpoint:

----    ( 20.5 ( 329.456 22.289 ) )
----

5.2 Function describing the signature of the Type Constructor

5.2.3 ~Out~-function

*/
template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutUncertain( ListExpr typeInfo, Word value )
{
  Uncertain<Alpha>* uncertain = (Uncertain<Alpha>*)(value.addr);
  
  if( uncertain->IsDefined() )
    return nl->TwoElemList(
      nl->RealAtom( &uncertain->GetEpsylon()),
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
5.2.2 ~In~-function

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
    ListExpr first = nl->First( instance );             // the epsylon value
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
5.2.3 ~Create~-function

*/
template <class Alpha>
Word CreateUncertain( const ListExpr typeInfo )
{
  return (SetWord( new Uncertain<Alpha>() ));
}


/*
5.2.4 ~Delete~-function

*/
template <class Alpha>
void DeleteUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.2.5 ~Close~-function

*/
template <class Alpha>
void CloseUncertain( const ListExpr typeInfo, Word& w )
{
  delete (Uncertain<Alpha> *)w.addr;
  w.addr = 0;
}

/*
5.2.6 ~Clone~-function

*/
template <class Alpha>
Word CloneUncertain( const ListExpr typeInfo, const Word& w )
{
  Uncertain<Alpha> *uncertain = (Uncertain<Alpha> *)w.addr;
  return SetWord( new Uncertain<Alpha>( *uncertain ) );
}

/*
5.2.7 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfUncertain()
{
  return sizeof(Uncertain<Alpha>);
}

/*
5.2.8 ~Cast~-function

*/
template <class Alpha>
void* CastUncertain(void* addr)
{
  return new (addr) Uncertain<Alpha>;
}


/*
5.4 Value mapping functions for class Uncertain


5.4.1 Value mapping functions of operator ~epsylon~

*/
template <class Alpha>
int UncertainEpsylon( Word* args, Word& result, int message, Word& local, 
                                  Supplier s )
{
  result = qp->ResultStorage( s );
  Uncertain<Alpha>* u = (Uncertain<Alpha>*)args[0].addr;

  if( u->IsDefined() )
    ((CcReal*)result.addr)->Set( u->epsylon );
  else
    ((CcReal*)result.addr)->SetDefined( false );

  return 0;
}




/*
6.5 Value mapping functions of operator ~val~

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




