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
//[TOC] [\tableofcontents]

[1] LUReal.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef LUREAL_H
#define LUREAL_H

#define PI 3.14159265

#include "StandardLengthUnit.h"

/*
3 C++ Classes (Defintion)

3.1 LUReal

This class will be used in the ~lureal~ type constructor, i.e., the type constructor
for the length unit of real numbers.
Inherits a denined flag.

*/

class LUReal : public StandardLengthUnit<CcReal>
{
public:
/*
3.1.1 Constructors and Destructor

3.1.1.1 Standard constructor

*/

    LUReal() {};
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Default constructor

*/

    LUReal(bool is_defined):
        StandardLengthUnit<CcReal>(is_defined),
        m(0.0), n(0.0)
    { };
/*
This constructor initializes the LInterval and the length function parameters with
zero. If the passed bool flag is true start and end are defined, otherwise not.
The produced linterval is left and right closed.

3.1.1.3 Setting constructor

*/

    LUReal( const LInterval& linterval,
            const double a,
            const double b,
            const bool functionparameters = true):
            StandardLengthUnit<CcReal>( linterval )
    {
        del.refs=1;
        del.SetDelete();
        del.isDefined=true;

        if (functionparameters)
        {
            m = a;
            n = b;
        }
        else
        {
            m = (b-a) / (linterval.end.GetRealval()
                - linterval.start.GetRealval());
            n = a - (m * linterval.start.GetRealval());
        }
    }
/*
This constructor intializes the length interval of the LUReal with the
passes linterval. If the boolean flag ~functionparameters~ is true m and n
are initialized with the passed values ~a~ and ~b~. Otherwise the values of ~m~ and
~n~ are calculated from the passed values ~a~ and ~b~ and the start and end value
of the passed linterval.

3.1.2 Member functions

3.1.2.1 LengthFunction

*/

    virtual void LengthFunction( const CcReal& l,
                                 CcReal& result,
                                 bool ignoreLimits = false ) const
    {
        if ( !this->IsDefined() ||
        !l.IsDefined() ||
        (!this->lengthInterval.Contains( l ) && !ignoreLimits) )
    {
        result.Set(false, 0.0);
    }
    else
    {
        double res = m * l.GetRealval() + n;
        result.Set( true, res );
    }
}

/*
3.1.2.2 EqualValue

*/
    virtual bool EqualValue( const LUReal& i1 ) const
    {
        if( !this->IsDefined() && !i1.IsDefined() )
            return true;
        if( !this->IsDefined() || !i1.IsDefined() )
            return false;

        double val1 = atan (this->m) * 180 / PI;
        double val2 = atan (i1.m) * 180 / PI;
        return (((val2-val1)<1 && (val2-val1)>-1) ? true : false) ;
  }

/*
Returns ~true~ if the value of this length unit is equal to the
value of the length unit ~i~ and ~false~ if they are different.
Two values are equal if the absolut difference between the gradient ~m~
is smaller than 1 degree.


3.1.3 Operator Redefinitions

3.1.3.1 Operation ~copy~

*/

    virtual LUReal& operator=( const LUReal& i )
    {
        del.isDefined = i.del.isDefined;
        if( !i.IsDefined() )
        {
            return *this;
        }
        *((LengthUnit<CcReal>*)this) = *((LengthUnit<CcReal>*)&i);
        m = i.m;
        n = i.n;
        return *this;
    }
/*
Redefinition of the copy operator ~=~.

3.1.3.2 Operation $=$ (~equal~)

*/

    virtual bool operator==( const LUReal& i ) const
    {
        if( !this->IsDefined() && !i.IsDefined() )
        {
            return true;
        }
        return
        (
            this->IsDefined() && i.IsDefined()
            && *((LengthUnit<CcReal>*)this) == *((LengthUnit<CcReal>*)&i)
            && AlmostEqual( m, i.m )
            && AlmostEqual( n, i.n )
        );
    }
/*
Returns ~true~ if this length unit is equal to the length unit ~i~ and ~false~
if they are different. Two undefined units are always equal.

3.1.3.3 Operation $\neq$ (~not equal~)

*/

    virtual bool operator!=( const LUReal& i ) const
    {
        return !( *this == i );
    }
/*
Returns ~true~ if this length unit is different to the length unit ~i~ and ~false~
if they are equal.

3.1.4 Functions to be part of relations

3.1.4.1 Compare

*/

    virtual int Compare( const Attribute* arg ) const
    {
        LUReal* lur2 = (LUReal*) arg;

        if (!IsDefined() && !lur2->IsDefined())
            return 0;
        if (!IsDefined())
            return -1;
        if (!lur2->IsDefined())
            return 1;

        // both ureals are defined...
        int cmp = lengthInterval.CompareTo(lur2->lengthInterval);
        if(cmp)
            return cmp;
        // because we can't compare the functions themself, we
        // use the lexicographical order of the parameters
        if(m<lur2->m) return -1;
        if(m>lur2->m) return 1;
        if(n<lur2->n) return -1;
        if(n>lur2->n) return 1;
        return 0;
    }

/*
3.1.4.2 Adjacent

*/

    virtual bool Adjacent( const Attribute* arg ) const
    {
        return false;
    }

/*
3.1.4.3 HashValue

*/

    virtual size_t HashValue() const
    {
        if(!IsDefined())
            return 0;
        return static_cast<size_t>
        (
            lengthInterval.start.HashValue()^
            lengthInterval.end.HashValue()
        );
    }

/*
3.1.4.4 CopyFrom

*/

    virtual void CopyFrom( const Attribute* right )
    {
        const LUReal* i = (const LUReal*)right;

        del.isDefined = i->del.isDefined;

        if(i->IsDefined())
        {
            lengthInterval.CopyFrom(i->lengthInterval);
            m = i->m;
            n = i->n;
        }
        else
        {
            lengthInterval = LInterval();
            m = 0;
            n = 0;
        }
    }

/*
3.1.4.5 Print

*/

    virtual ostream& Print( ostream &os ) const
    {
        if( IsDefined() )
        {
            os << "LUReal: " << "( ";
            lengthInterval.Print(os);
            os << ", ";
            // print specific stuff:
            os << " ( " << m << ", " << n << " )";
            os << " ) ";
            return os;
        }
        else
            return os << "LUReal: (undef) ";
    }

/*
3.1.4.6 Basic Type

*/

    static const string BasicType(){ return "lureal"; }

/*
3.1.4.7 Check Type

*/

    static const bool checkType(const ListExpr type)
    {
        return listutils::isSymbol(type, BasicType());
    }

/*
3.1.4.8 Clone Function

*/

    virtual LUReal* Clone() const
    {
        LUReal *res;
        if( !this->IsDefined() )
            res = new LUReal( false );
        else
            res = new LUReal( lengthInterval, m, n );
        res->del.isDefined = del.isDefined;
        return res;
    }

/*
3.1.4.9 SizeOf Function

*/

    virtual size_t Sizeof() const
    {
        return sizeof( *this );
    }


/*
3.1.6 Attributes

*/
    double m, n;

};

/*
3.4 Type Constructor ~lureal~

Type ~lureal~ represents an (linterval, (m,n))-pair, where
m and n are real numbers.

3.4.1 List Representation

The list representation of an ~lureal~ is

----    ( lengthInterval m n )
----

For example:

----    ( (1.0 5.7 TRUE FALSE) (3.2 4.5) )
----

3.4.2 Function Describing the Signature of the Type Constructor

*/
ListExpr
LURealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LUNIT"),
                             nl->StringAtom("("+LUReal::BasicType()+") "),
         nl->StringAtom("( lengthInterval (real1 real2)) "),
         nl->StringAtom("((1.3 5.2 TRUE FALSE) (1.0 2.2))"))));
}

/*
3.4.3 Kind Checking Function

*/
bool
CheckLUReal( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, LUReal::BasicType() ));
}

/*
3.4.4 ~Out~-function

*/
ListExpr OutLUReal( ListExpr typeInfo, Word value )
{
  LUReal* lureal = (LUReal*)(value.addr);

  if ( !lureal->IsDefined() )
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  else
    {

      ListExpr lengthIntervalList = nl->FourElemList(
             OutCcReal( nl->TheEmptyList(),
             SetWord(&lureal->lengthInterval.start) ),
             OutCcReal( nl->TheEmptyList(),
                          SetWord(&lureal->lengthInterval.end) ),
             nl->BoolAtom( lureal->lengthInterval.lc ),
             nl->BoolAtom( lureal->lengthInterval.rc));

      ListExpr realfunList = nl->TwoElemList(
             nl->RealAtom( lureal->m),
             nl->RealAtom( lureal->n));

      return nl->TwoElemList(lengthIntervalList, realfunList );
    }
}

/*
3.4.5 ~In~-function

*/
Word InLUReal( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  correct = true;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );
    if( nl->ListLength( first ) == 4 &&
      nl->IsAtom( nl->Third( first ) ) &&
      nl->AtomType( nl->Third( first ) ) == BoolType &&
      nl->IsAtom( nl->Fourth( first ) ) &&
      nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      CcReal *start = (CcReal *)InCcReal( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;
      if( !correct || !start->IsDefined() )
      {
        errmsg = "InLUReal(): Error in first instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      CcReal *end = (CcReal *)InCcReal( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;
      if( !correct || !end->IsDefined() )
      {
        errmsg = "InLUReal(): Error in second instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      LInterval linterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = linterval.IsValid();
      if ( !correct )
        {
          errmsg = "InLUReal(): Non valid length interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );

      if( nl->ListLength( second ) == 2 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == RealType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == RealType )
      {
        LUReal *lureal =
          new LUReal( linterval,
                     nl->RealValue( nl->First( second ) ),
                     nl->RealValue( nl->Second( second ) ));

        if( lureal->IsValid() )
        {
          correct = true;
          return SetWord( lureal );
        }
        delete lureal;
      }
    }
  }
  else if ( listutils::isSymbolUndefined(instance) )
    {
      LUReal *lureal = new LUReal();
      lureal->SetDefined(false);
      lureal->lengthInterval=LInterval(true);
      correct = lureal->lengthInterval.IsValid();
      if ( correct )
        return (SetWord( lureal ));
    }
  errmsg = "InLUReal(): Non valid representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
3.4.6 ~Create~-function

*/
Word CreateLUReal( const ListExpr typeInfo )
{
  return (SetWord( new LUReal(false) ));
}

/*
3.4.7 ~Delete~-function

*/
void DeleteLUReal( const ListExpr typeInfo, Word& w )
{
  delete (LUReal *)w.addr;
  w.addr = 0;
}

/*
3.4.8 ~Close~-function

*/
void CloseLUReal( const ListExpr typeInfo, Word& w )
{
  delete (LUReal *)w.addr;
  w.addr = 0;
}

/*
3.4.9 ~Clone~-function

*/
Word CloneLUReal( const ListExpr typeInfo, const Word& w )
{
  LUReal *lureal = (LUReal *)w.addr;
  return SetWord( new LUReal( *lureal ) );
}

/*
3.4.10 ~Sizeof~-function

*/
int SizeOfLUReal()
{
  return sizeof(LUReal);
}

/*
3.4.11 ~Cast~-function

*/
void* CastLUReal(void* addr)
{
  return new (addr) LUReal;
}

/*
3.4.12 Creation of the type constructor ~lureal~

*/
TypeConstructor lunitreal(
        LUReal::BasicType(),              //name
        LURealProperty,  //property function describing signature
        OutLUReal,     InLUReal, //Out and In functions
        0,            0,   //SaveToList and RestoreFromList functions
        CreateLUReal,
        DeleteLUReal, //object creation and deletion
        OpenAttribute<LUReal>,
        SaveAttribute<LUReal>,  // object open and save
        CloseLUReal,   CloneLUReal, //object close and clone
        CastLUReal, //cast function
        SizeOfLUReal, //sizeof function
        CheckLUReal );                     //kind checking function


#endif //LUREAL_H
