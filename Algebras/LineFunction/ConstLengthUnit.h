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

[1] ConstLengthUnit.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef CONST_LENGTH_UNIT_H
#define CONST_LENGTH_UNIT_H

#include "StandardLengthUnit.h"


/*
3 C++ Classes (Defintion)

3.1 ConstLengthUnit

This class will be used in the ~const~ type constructor. It constructs constant
length units, i.e. it has a constant value and the length function always
return this value. The explicit purpose of the ~const~ type constructor is to
define length units for ~int~, ~string~, and ~bool~, i.e., for types where
their values change only in discrete steps.

This class inherits a defined flag!

*/
template <typename Alpha>
class ConstLengthUnit : public StandardLengthUnit<Alpha>
{
public:
/*
3.1.1 Constructors and Destructor

3.1.1.1 Standard constructor

*/

    ConstLengthUnit() {}
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Default constructor

*/
    ConstLengthUnit(bool is_defined):
        StandardLengthUnit<Alpha>(is_defined),
        constValue(is_defined)
    { }
/*
This constructor initializes the LInterval and the constValue with zero. If the
passed bool flag is true start, end and the constValue are defined, otherwise not.
The produced linterval is left and right closed.

3.1.1.3 Setting constructor

*/

    ConstLengthUnit( const LInterval& linterval, const Alpha& m ):
        StandardLengthUnit<Alpha>( linterval ),
        constValue(m)
    {
        this->del.isDefined = true;
    }
/*
This constructor intializes the length interval of the ConstLengthUnit with the
passes linterval and the constValue with the passed value a.

3.1.1.4 Copy constructor

*/

    ConstLengthUnit( const ConstLengthUnit<Alpha>& u ):
        StandardLengthUnit<Alpha>( u.lengthInterval ),
        constValue(u.constValue)
    {
        this->del.isDefined = u.del.isDefined;
    }
/*
This constructor intializes the ConstLengthUnit wiht the values of the passed
ConstLengthUnit u.

3.1.2 Member functions

3.1.2.1 LengthFunction

*/
    virtual void LengthFunction( const CcReal& l,
                                Alpha& result,
                                bool ignoreLimits = false ) const
    {
        if ( !this->IsDefined() ||
            !l.IsDefined() ||
            (!this->lengthInterval.Contains( l ) && !ignoreLimits))
        {
            result.SetDefined( false );
        }
        else
        {
            result.CopyFrom( &constValue );
            result.SetDefined( true );
        }
    }
/*
~LengthFunction~ returns an undefined result if the LengthUnit or the CcReal
is undefined, or the CcReal is not within the unit's lengthInterval.

3.1.2.2 EqualValue

*/

    virtual bool EqualValue( const LengthUnit<Alpha>& i) const
    {
        return EqualValue( *((const ConstLengthUnit*)&i));
    }

    virtual bool EqualValue( const ConstLengthUnit<Alpha>& i ) const
    {
        return this->IsDefined() && (constValue.Compare( &i.constValue ) == 0);
    }
/*
Returns ~true~ if the value of this length unit is defined and equal to the
value of the length unit ~i~ and ~false~ if they are different.

3.1.3 Operator Redefinitions

3.1.3.1 Operation ~copy~

*/

    virtual ConstLengthUnit<Alpha>& operator=( const ConstLengthUnit<Alpha>& i )
    {
        this->del.isDefined = i.del.isDefined;
        if( !i.IsDefined() )
        {
            return *this;
        }
        *((LengthUnit<Alpha>*)this) = *((LengthUnit<Alpha>*)&i);
        constValue.CopyFrom( &i.constValue );
        return *this;
    }
/*
Redefinition of the copy operator ~=~.

3.1.3.2 Operation $=$ (~equal~)

*/

    virtual bool operator==( const ConstLengthUnit<Alpha>& i ) const
    {
        if( !this->IsDefined() && !i.IsDefined() )
        {
            return true;
        }
        return
        (
            (this->IsDefined()) && (i.IsDefined())
            && *((LengthUnit<Alpha>*)this) == *((LengthUnit<Alpha>*)&i)
            && (constValue.Compare( &i.constValue ) == 0)
        );
    }
/*
Two ConstLengthUnits are equal, if both are either undefined, or both are
defined and represent the same length function
Returns ~true~ if this length unit is equal to the length unit ~i~ and ~false~
if they are different.

3.1.3.3 Operation $\neq$ (~not equal~)

*/

    virtual bool operator!=( const ConstLengthUnit<Alpha>& i ) const
    {
        return !( *this == i );
    }
/*
Returns ~true~ if this length unit is different to the length unit ~i~
and ~false~ if they are equal.

3.1.4 Functions to be part of relations

3.1.4.1 Compare

*/

    virtual int Compare( const Attribute* arg ) const
    {
        ConstLengthUnit<Alpha>*  clu = (ConstLengthUnit<Alpha>*)arg;

        if (this->IsDefined() && !clu->IsDefined())
            return 0;
        if (!this->IsDefined())
            return -1;
        if (!clu->IsDefined())
            return 1;

        int cmp = this->lengthInterval.CompareTo(clu->lengthInterval);
        if(cmp)
        {
            return cmp;
        }
        return constValue.Compare(&(clu->constValue));
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
        if(!this->IsDefined())
        {
            return 0;
        }
        return static_cast<size_t>
        (
            this->lengthInterval.start.HashValue()^
            this->lengthInterval.end.HashValue()
        );
    }

/*
3.1.4.4 CopyFrom

*/

    virtual void CopyFrom( const Attribute* right )
    {
        const ConstLengthUnit<Alpha>* clu =
            (const ConstLengthUnit<Alpha>*)right;
        this->SetDefined(clu->IsDefined());
        this->lengthInterval.CopyFrom( clu->lengthInterval );
        constValue.CopyFrom( &(clu->constValue) );
    }

/*
3.1.4.5 NumOfFLOBs

*/

    inline int NumOfFLOBs() const {return constValue.NumOfFLOBs();}

/*
3.1.4.6 GetFLOB

*/

    inline Flob *GetFLOB(const int i) {return constValue.GetFLOB(i);}

/*
3.1.4.7 Print

*/

    virtual std::ostream& Print( std::ostream &os ) const
    {
        if( this->IsDefined() )
        {
            os << ConstLengthUnit<Alpha>::BasicType() << ": ( ";
            LengthUnit<Alpha>::lengthInterval.Print(os);
            os << ", ";
            constValue.Print(os);
            os << " ) " << endl;
            return os;
        }
        else
            return os << ConstLengthUnit<Alpha>::BasicType()<<": (undef) ";
    }

/*
3.1.4.8 Basic Type

*/

    inline static const std::string BasicType()
    {
        return "lu"+Alpha::BasicType();
    }

/*
3.1.4.9 Check Type

*/

    static const bool checkType(const ListExpr type)
    {
        return listutils::isSymbol(type, BasicType());
    }

/*
3.1.4.10 Clone Function

*/

    virtual ConstLengthUnit<Alpha>* Clone() const
    {
        return new ConstLengthUnit<Alpha>(*this);
    }

/*
3.1.4.11 SizeOf Funtion

*/

    virtual size_t Sizeof() const
    {
        return sizeof( *this );
    }

/*
3.1.5 Attributes

*/

    Alpha constValue;
/*
The constant value of the length unit.

*/

};

/*
3.2 Class ~LUBool~

*/
typedef ConstLengthUnit<CcBool> LUBool;

/*
3.3 Class ~LUInt~

*/
typedef ConstLengthUnit<CcInt> LUInt;

/*
3.4 Class ~LUString~

*/
typedef ConstLengthUnit<CcString> LUString;

/*
3 Type Constructors

1.1 Type Constructor ~ConstLengthUnit~

1.1.1 ~Out~-function

*/



template <class Alpha, ListExpr (*OutFun)( ListExpr, Word )>
ListExpr OutConstLengthUnit( ListExpr typeInfo, Word value )
{
    //1.get the address of the object and have a class object
    ConstLengthUnit<Alpha>* constunit = (ConstLengthUnit<Alpha>*)(value.addr);

    //2.test for undefined value
    if ( !constunit->IsDefined() )
        return (nl->SymbolAtom(Symbol::UNDEFINED()));

    //3.get the length interval NL
    ListExpr intervalList = nl->FourElemList(
    OutCcReal( nl->TheEmptyList(), SetWord(&constunit->lengthInterval.start) ),
    OutCcReal( nl->TheEmptyList(), SetWord(&constunit->lengthInterval.end) ),
    nl->BoolAtom( constunit->lengthInterval.lc ),
    nl->BoolAtom( constunit->lengthInterval.rc));

    //4. return the final result
    return nl->TwoElemList
    (
        intervalList,
        OutFun( nl->TheEmptyList(), SetWord( &constunit->constValue ) ) );
    }

/*
1.1.2 ~In~-function

*/
template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr,
                                      const int, ListExpr&, bool&     )>
Word InConstLengthUnit( const ListExpr typeInfo,
                        const ListExpr instance,
                        const int errorPos,
                        ListExpr& errorInfo,
                        bool& correct             )
{
    std::string errmsg;

    if( nl->ListLength( instance ) == 2 )
    {
        //1. deal with the length interval
        ListExpr first = nl->First( instance );

        if( nl->ListLength( first ) == 4 &&
            nl->IsAtom( nl->Third( first ) ) &&
            nl->AtomType( nl->Third( first ) ) == BoolType &&
            nl->IsAtom( nl->Fourth( first ) ) &&
            nl->AtomType( nl->Fourth( first ) ) == BoolType )
        {
            CcReal *start =
                (CcReal *)InCcReal( nl->TheEmptyList(), nl->First( first ),
                                    errorPos, errorInfo, correct ).addr;
            if( !correct )
            {
                errmsg = "InConstLengthUnit(): Error in first instant.";
                errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
                delete start;
                return SetWord( Address(0) );
            }

            CcReal *end =
                (CcReal *)InCcReal( nl->TheEmptyList(), nl->Second( first ),
                                    errorPos, errorInfo, correct ).addr;
            if( !correct )
            {
                errmsg = "InConstLengthUnit(): Error in second instant.";
                errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
                delete start;
                delete end;
                return SetWord( Address(0) );
            }
            // get closedness parameters
            bool lc = nl->BoolValue( nl->Third( first ) );
            bool rc = nl->BoolValue( nl->Fourth( first ) );

            LInterval linterval( *start, *end, lc, rc );

            delete start;
            delete end;

            // check, wether linterval is well defined
            correct = linterval.IsValid();
            if ( !correct )
            {
                errmsg = "InConstLengthUnit(): Non valid length interval.";
                errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
                return SetWord( Address(0) );
            }

            //2. deal with the alpha value
            Alpha *value = (Alpha *)InFun(
                        nl->TheEmptyList(), nl->Second( instance ),
                        errorPos, errorInfo, correct ).addr;

            //3. create the class object
            if( correct  )
            {
                ConstLengthUnit<Alpha> *constunit =
                    new ConstLengthUnit<Alpha>( linterval, *value );

                if( constunit->IsValid() )
                {
                    delete value;
                    return SetWord( constunit );
                }
                delete constunit;
            }
            delete value;
        }
    }
    else if ( listutils::isSymbolUndefined( instance ) )
    {
        ConstLengthUnit<Alpha> *constunit =
            new ConstLengthUnit<Alpha>();
        constunit->SetDefined(false);
        constunit->lengthInterval= LInterval(true);
        correct = true;
        return (SetWord( constunit ));
    }
    errmsg = "InConstLengthUnit(): Error in representation.";
    errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    correct = false;
    return SetWord( Address(0) );
}

/*
1.1.3 ~Create~-function

*/
template <class Alpha>
Word CreateConstLengthUnit( const ListExpr typeInfo )
{
    return (SetWord( new ConstLengthUnit<Alpha>(false) ));
}

/*
1.1.4 ~Delete~-function

*/
template <class Alpha>
void DeleteConstLengthUnit( const ListExpr typeInfo, Word& w )
{
    delete (ConstLengthUnit<Alpha> *)w.addr;
    w.addr = 0;
}

/*
1.1.5 ~Close~-function

*/
template <class Alpha>
void CloseConstLengthUnit( const ListExpr typeInfo, Word& w )
{
    delete (ConstLengthUnit<Alpha> *)w.addr;
    w.addr = 0;
}

/*
1.1.6 ~Clone~-function

*/
template <class Alpha>
Word CloneConstLengthUnit( const ListExpr typeInfo, const Word& w )
{
    ConstLengthUnit<Alpha> *constunit = (ConstLengthUnit<Alpha> *)w.addr;
    return SetWord( new ConstLengthUnit<Alpha>( *constunit ) );
}

/*
1.1.7 ~Sizeof~-function

*/
template <class Alpha>
int SizeOfConstLengthUnit()
{
    return sizeof(ConstLengthUnit<Alpha>);
}

/*
1.1.8 ~Cast~-function

*/
template <class Alpha>
void* CastConstLengthUnit(void* addr)
{
    return new (addr) ConstLengthUnit<Alpha>;
}

#endif //CONST_LENGTH_UNIT_H
