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

[1] LMapping.h

April 2015 - Rene Steinbrueck

1 Overview

[TOC]

2 Defines, includes, and constants

*/
#ifndef LMAPPING_H
#define LMAPPING_H

#include "../../Tools/Flob/DbArray.h"
#include "ConstLengthUnit.h"
#include "LUReal.h"
/*
3 C++ Classes (Defintion)
3.9 LMapping

This class will implement the functionalities of the ~mapping~ type constructor.
It contains a database array of length units. For that, ~Unit~ must implement
the class ~LengthUnit~ or ~ConstLengthUnit~, because functions of these classes
will be used.

*/

template <typename Unit, typename Alpha>
class LMapping : public Attribute
{
public:
/*
3.1.1 Constructors and Destructor

3.1.1.1 Standard constructor

*/
    LMapping() {}
/*
The simple constructor. This constructor should not be used.

3.1.1.2 Setting constructor

*/

    LMapping( const int n );
/*
The constructor. Initializes space for ~n~ elements.

3.1.1.3 Destructor

*/

    virtual ~LMapping();

    void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of length units. It marks the persistent array for destroying. The
destructor will perform the real destroying.

3.1.2 Functions for Bulk Load of length units

As said before, the moving unit set is implemented as an ordered persistent array of units.
The time complexity of an insertion operation in an ordered array is $O(n)$, where ~n~
is the size of the unit set. In some cases, bulk load of units for example, it is good
to relax the ordered condition to improve the performance. We have relaxed this ordered
condition only for bulk load of units. All other operations assume that the unit set is
ordered.

3.1.2.1 IsOrdered

*/

    bool IsOrdered() const;
/*
Returns true, if the unit set is ordered.
There is a flag ~ordered~ (see attributes) in order
to avoid a scan in the unit set to answer this question.

3.1.2.2 StartBulkLoad

*/

    void StartBulkLoad();
/*
Marks the start of a bulk load of units relaxing the condition that the units must be
ordered. We will assume that the only way to add units to an unit set is inside bulk
loads, i.e., into non-ordered mappings.

3.1.2.3 EndBulkLoad

*/

    virtual bool EndBulkLoad( const bool sort = true,
                              const bool checkvalid = false );
/*
Marks the end of a bulk load and sorts the unit set if the flag ~sort~ is set to true.
Checkvalid indicated, whether the validity (length non-overlapping units) should
be checked. In this case, if the Range is invalid, it is set to undefined!

3.1.3 Member functions

3.1.3.1 IsEmpty

*/
    bool IsEmpty() const;
/*
Returns true, if the mapping is undefined or empty of units.

3.1.3.2 Get

*/

    void Get( const int i, Unit& upi ) const;
/*
Returns the unit ~upi~ at the position ~i~ in the mapping.

*Precondition*: this[->]IsDefined() == true

*Precondition*: 0 <= this[->]NoComponents() <= i

3.1.3.3 Add

*/

    virtual void Add( const Unit& upi );
/*
Adds an unit ~upi~ to the mapping. We will assume that the only way of adding units
is in bulk loads, i.e., in a non-ordered array.

*Precondition:* this[->]IsDefined() == true

*Precondition:* ~IsOrdered() == false~

3.1.3.4 MergeAdd

*/

    virtual void MergeAdd( const Unit& upi );
/*
Adds an unit ~upi~ to the mapping. If the new unit and the last
unit in the Mapping are equalValue it merges the two units.
We will assume that the only way of adding units
is in bulk loads, i.e., in a non-ordered array.
Without defining the function ~equalValue~ for units
~MergeAdd~ works the same way as ~Add~.

*Precondition:* this[->]IsDefined() == true

*Precondition:* ~IsOrdered() == false~

3.1.3.5 Put

*/

    inline void Put(const int i, const Unit& u)
    {
        units.Put(i,u);
    }
/*
Replaces the unit at position ~i~ within the mapping's DbArray.

*Precondition:* this[->]IsDefined() == true

3.1.3.6 Clear

*/

    virtual void Clear();
/*
Remove all units in the mapping and set the defined flag.

3.1.3.7 IsValid

*/

    bool IsValid() const;
/*
This functions tests if a ~mapping~ is in a valid format. It is used for debugging
purposes only. The ~mapping~ is valid, if the following conditions are true:

Either the mapping is undefined, or the mapping is defined and conditions 1--3 hold:

  1 Each unit is valid

  2 Start of each unit $>=$ end of the unit before

  3 If start of an unit = end of the unit before, then one needs to
    make sure that the unit is not left-closed or the unit before
    is not right-closed

3.1.3.8 LengthFunction

*/

    void LengthFunction( const CcReal& l,
                         Alpha& result,
                         bool ignoreLimits = true) const;

/*
3.1.3.9 Position

*/

    int Position( const CcReal& l ) const;
/*
Returns the number ot the length unit which contains l. If there is no length unit
that contains l, it retuns -1.

*Precondition:* ~X.IsOrdered()~

*Semantics:*

*Complexity:* $O( \log n )$, where ~n~ is the number of units of this mapping ~X~

3.1.4 Operator Redefinitions

3.1.4.1 Operation $=$ (~equal~)

*/
    bool operator==( const LMapping<Unit, Alpha>& mp ) const;
/*
*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X = Y$

*Complexity:* $O(n+m)$, where ~n~ is the number of units of this mapping ~X~ and m the
number of units of the mapping ~Y~.

3.1.4.2 Operation $\neq$ (~not equal~)

*/
    bool operator!=( const LMapping<Unit, Alpha>& mp ) const;
/*
*Precondition:* ~X.IsOrdered() $\&\&$ Y.IsOrdered()~

*Semantics:* $X \neq Y$

*Complexity:* $O(n+m)$, where ~n~ is the number of units of this mapping ~X~ and m the
number of units of the mapping ~Y~.

3.1.4.3 Operation ~no\_components~

*/
    int GetNoComponents() const;
/*
*Precondition:* ~IsDefined() == true~

*Precondition:* ~X.IsOrdered()~

*Semantics:* $\| intvls(X) \|$

*Complexity:* $O(1)$

3.1.5 Functions to be part of relations

3.1.5.1 Compare

*/

    inline virtual int Compare( const Attribute* arg ) const;

/*
3.1.5.2 Adjacent

*/

    inline bool Adjacent( const Attribute* arg ) const;

/*
3.1.5.3 HashValue

*/

    inline size_t HashValue() const;

/*
3.1.5.4 CopyFrom

*/

    inline virtual void CopyFrom( const Attribute* right );

/*
3.1.5.5 NumOfFLOBs

*/

    inline int NumOfFLOBs() const;

/*
3.1.5.6 GetFLOB

*/

    inline Flob *GetFLOB(const int i);

/*
3.1.5.7 Print

*/

    inline virtual std::ostream& Print( std::ostream &os ) const;
/*
3.1.5.8 Basic Type

*/
    inline static const std::string BasicType()
    {
        return "l"+Alpha::BasicType();
    }

/*
3.1.5.9 Check Type

*/

    static const bool checkType(const ListExpr type)
    {
        return listutils::isSymbol(type, BasicType());
    }

/*
3.1.5.10 Clone

*/

    inline Attribute* Clone() const;

/*
3.1.5.11 SizeOf

*/

    inline virtual size_t Sizeof() const;

/*
3.1.6 Attributes

*/

private:

    bool canDestroy;
/*
A flag indicating if the destructor should destroy also the persistent
array of intervals.

*/

    bool ordered;
/*
A flag indicating whether the unit set is ordered or not.

*/

protected:
    DbArray< Unit > units;
/*
The database array of length units.

*/
};

/*
3.2 Class ~LBool~

*/
typedef LMapping< LUBool, CcBool > LBool;
/*
3.3 Class ~LInt~

*/
typedef LMapping< LUInt, CcInt > LInt;

/*
3.4 Class ~LString~

*/
typedef LMapping <LUString, CcString > LString;

/*
3.5 Class ~LReal~

*/
typedef LMapping < LUReal, CcReal > LReal;

/*
1 C++ Classes (Implementation)

1.1 LMapping

1.1.1 Constructors and destructor

1.1.1.1 Setting constructor

*/

template <typename Unit, typename Alpha>
LMapping<Unit, Alpha>::LMapping( const int n ):
    Attribute(true),
    canDestroy( false ),
    ordered( true ),
    units( n )
{
  del.refs=1;
  del.SetDelete();
  del.isDefined = true;
}

/*
1.1.1.2 Destructor

*/
template <typename Unit, typename Alpha>
LMapping<Unit, Alpha>::~LMapping()
{
    if( canDestroy )
        units.Destroy();
}


template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::Destroy()
{
    canDestroy = true;
}

/*
1.1.2 Functions for Bulk Load of length units

1.1.2.1 IsOrdered

*/

template <typename Unit, typename Alpha>
bool LMapping<Unit, Alpha>::IsOrdered() const
{
    return ordered;
}

/*
1.1.2.2 StartBulkLoad

*/

template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::StartBulkLoad()
{
    assert( IsDefined() );
    assert( ordered );
    ordered = false;
}
/*
1.1.2.3 LUnitCompare

*/
template <class Unit>
int LUnitCompare( const void *a, const void *b )
{
    Unit *unita = new ((void*)a) Unit,
         *unitb = new ((void*)b) Unit;

    if( *unita == *unitb )
        return 0;
    else if( unita->Before( *unitb ) )
        return -1;
    else
        return 1;
}
/*
1.1.2.4 EndBulkLoad

*/

template <typename Unit, typename Alpha>
bool LMapping<Unit, Alpha>::EndBulkLoad(
    const bool sort, const bool checkvalid )
{
    assert( !ordered );
    if( !IsDefined() )
        units.clean();
    else if( sort )
        units.Sort( LUnitCompare<Unit> );
    ordered = true;
    units.TrimToSize();
    if( checkvalid && !IsValid() )
    {
        units.clean();
        SetDefined( false );
        return false;
    }
    return true;
}
/*

1.1.3 Member functions

1.1.3.1 IsEmpty

*/

template <typename Unit, typename Alpha>
bool LMapping<Unit, Alpha>::IsEmpty() const
{
    return !IsDefined() || (units.Size() == 0);
}

/*
1.1.3.2 Get

*/

template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::Get( const int i, Unit &unit ) const
{
    assert( IsDefined() );
    assert(i>=0);
    assert(i<units.Size());
    bool ok = units.Get( i, unit );
    if(!ok)
    {
        std::cout << "Problem in getting data from " << units << std::endl;
        assert(ok);
    }
    if ( !unit.IsValid() )
    {
        std::cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
        << " Get(" << i << ", Unit): Unit is invalid:";
        unit.Print(std::cout); std::cout << std::endl;
        assert( unit.IsValid());
    }
}

/*
1.1.3.3 Add

*/

template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::Add( const Unit& unit )
{
    assert( IsDefined() );
    if ( !unit.IsDefined() || !unit.IsValid() )
    {
        std::cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
        << " Add(Unit): Unit is undefined or invalid:";
        unit.Print(std::cout); std::cout << std::endl;
        assert( false );
    }
    units.Append( unit );
}

/*
1.1.3.4 MergeAdd

*/

template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::MergeAdd( const Unit& unit )
{
    assert( IsDefined() );
    Unit lastunit;
    int size = units.Size();
    if ( !unit.IsDefined() || !unit.IsValid() )
    {
        std::cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
        << " MergeAdd(Unit): Unit is undefined or invalid:";
        unit.Print(std::cout); std::cout << std::endl;
        assert( false );
    }

    if (size > 0)
    {
        units.Get( size - 1, &lastunit );
        if (lastunit.EqualValue(unit) &&
           (lastunit.lengthInterval.end == unit.lengthInterval.start) &&
           (lastunit.lengthInterval.rc || unit.lengthInterval.lc))
           {
                lastunit.lengthInterval.end = unit.lengthInterval.end;
                lastunit.lengthInterval.rc = unit.lengthInterval.rc;
                if ( !lastunit.IsValid() )
                {
                    std::cout << __FILE__ << ","
                    << __LINE__ << ":" << __PRETTY_FUNCTION__
                    << "\nMapping::MergeAdd(): lastunit is invalid:";
                    lastunit.Print(std::cout); std::cout << std::endl;
                    assert( false );
                }
                units.Put(size - 1, lastunit);
            }
        else
            units.Append( unit );
    }
    else
        units.Append( unit );
}

/*
1.1.3.4 Clear

*/

template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::Clear()
{
    ordered = true;
    units.clean();
    this->del.isDefined = true;
}

/*
1.1.3.5 IsValid

*/

template <typename Unit, typename Alpha>
bool LMapping<Unit, Alpha>::IsValid() const
{
    if( !IsDefined() )
        return true;

    if( canDestroy )
        return false;

    if( !IsOrdered() )
        return false;

    if( IsEmpty() )
        return true;

    bool result = true;
    Unit lastunit, unit;

    Get( 0, lastunit );
    if ( !lastunit.IsValid() )
    {
        std::cerr << "Mapping<Unit, Alpha>::IsValid(): "
                "unit is invalid: i=0" << std::endl;
    return false;
    }
    if ( GetNoComponents() == 1 )
        return true;

    for( int i = 1; i < GetNoComponents(); i++ )
    {
        Get( i, unit );
        if( !unit.IsValid() )
        {
            result = false;
            std::cerr << "Mapping<Unit, Alpha>::IsValid(): "
                    "unit is invalid: i=" << i << std::endl;
            return false;
        }
        if( unit.lengthInterval.start < lastunit.lengthInterval.end)
        {
            std::cerr << "Units are not ordered by length" << std::endl;
            std::cerr << "lastUnit.lengthInterval =  ";
                lastunit.lengthInterval.Print(std::cerr);
            std::cerr << std::endl;
            std::cerr << "unit.lengthInterval =  "; 
            unit.lengthInterval.Print(std::cerr);
            std::cerr << std::endl;
            return false;
        }
        if( (!lastunit.lengthInterval.Disjoint(unit.lengthInterval)) )
        {
            result = false;
            std::cerr << "Mapping<Unit, Alpha>::IsValid(): "
                    "unit and lastunit not disjoint: i=" << i << std::endl;
            std::cerr << "\n\tlastunit = ";
                lastunit.lengthInterval.Print(std::cerr);
            std::cerr << "\n\tunit     = ";
                unit.lengthInterval.Print(std::cerr); std::cerr << std::endl;
            return false;
        }
        lastunit = unit;
    }
    return result;
}

/*
1.1.3.6 LengthFunction

*/

template <typename Unit, typename Alpha>
void LMapping<Unit, Alpha>::LengthFunction( const CcReal& l,
                                            Alpha& result,
                                            bool ignoreLimits) const
{
    if( !IsDefined() || !l.IsDefined() )
    {
        result.SetDefined( false );
        return;
    }
    assert( IsOrdered() );
    int pos = Position( l );
    if( pos == -1 )  // not contained in any unit
        result.SetDefined( false );
    else
    {
        Unit posUnit;
        units.Get( pos, &posUnit );
        result.SetDefined( true );
        posUnit.LengthFunction( l, result, ignoreLimits );
    }
}

/*
1.1.3.7 Position

*/

template <typename Unit, typename Alpha>
int LMapping<Unit, Alpha>::Position( const CcReal& l ) const
{
    assert( IsDefined() );
    assert( IsOrdered() );
    assert( l.IsDefined() );

    int first = 0, last = units.Size() - 1;
    CcReal l1 = l;

    while (first <= last)
    {
        int mid = ( first + last ) / 2;

        if( (mid < 0) || (mid >= units.Size()) )
            return -1;

        Unit midUnit;
        units.Get( mid, &midUnit );

        if( midUnit.lengthInterval.Contains(l1) )
            return mid;
        else  //not contained
        if( ( l1 > midUnit.lengthInterval.end ) ||
            ( l1 == midUnit.lengthInterval.end ) )
                first = mid + 1;
        else if( ( l1 < midUnit.lengthInterval.start ) ||
                 ( l1 == midUnit.lengthInterval.start ) )
                    last = mid - 1;
        else
            return -1; //should never be reached.
    }
    return -1;
}

/*
1.1.4 Operator functions

1.1.4.1 Operation $=$ (~equal~)

*/
template <typename Unit, typename Alpha>
bool LMapping<Unit, Alpha>::operator==( const LMapping<Unit, Alpha>& r ) const
{
    if( !IsDefined() && !r.IsDefined() )
        return true;
    if( !IsDefined() || !r.IsDefined() )
        return false;

    assert( IsOrdered() && r.IsOrdered() );

    if( GetNoComponents() != r.GetNoComponents() )
        return false;

    bool result = true;
    Unit thisunit, unit;

    for( int i = 0; i < GetNoComponents(); i++ )
    {
        Get( i, thisunit );
        r.Get( i, unit );

        if( thisunit != unit )
        {
        result = false;
        break;
        }
    }

    return result;
}

/*
1.1.4.2 Operation $neq$ (~not equal~)

*/

template <typename Unit, typename Alpha>
bool LMapping<Unit, Alpha>::operator!=( const LMapping<Unit, Alpha>& r ) const
{
  return !( *this == r );
}

/*
1.1.4.3 GetNoComponents

*/

template <class Unit, class Alpha>
int LMapping<Unit, Alpha>::GetNoComponents() const
{
    assert( IsDefined() );
    return units.Size();
}

/*
1.1.5 Functions to be part of relations

1.1.5.1 Compare

*/

template <typename Unit, typename Alpha>
inline int LMapping<Unit, Alpha>::Compare( const Attribute *arg ) const
{
    LMapping<Unit,Alpha>* map2 = (LMapping<Unit,Alpha>*) arg;
    if( !IsDefined() && !map2->IsDefined() )
        return 0;
    if( !IsDefined() &&  map2->IsDefined() )
        return -1;
    if(  IsDefined() && !map2->IsDefined() )
        return 1;
    size_t size1 = units.Size();
    size_t size2 = map2->units.Size();
    size_t index = 0;
    Unit u1;
    Unit u2;
    int cmp;
    while( (index < size1) && (index < size2))
    {
        units.Get(index,&u1);
        map2->units.Get(index,&u2);
        cmp = u1.Compare(&u2);
        if(cmp) // different units
            return cmp;
        index++;
    }
    // the common entries all equals
    if(size1<size2)
        cmp =  -1;
    else if(size1>size2)
        cmp = 1;
    else
        cmp= 0;
    return cmp;
}

/*
1.1.5.2 Adjacent

*/

template <typename Unit, typename Alpha>
inline bool LMapping<Unit, Alpha>::Adjacent( const Attribute *arg ) const
{
    return false;
}

/*
1.1.5.3 HashValue

*/

template <typename Unit, typename Alpha>
inline size_t LMapping<Unit, Alpha>::HashValue() const
{
    if(!IsDefined())
        return 0;
    Unit min, max;
    units.Get(0,&min);
    units.Get(GetNoComponents()-1,max);
    return static_cast<size_t>( min.HashValue()^
                                max.HashValue()   ) ;
    return 0;
}

/*
1.1.5.4 CopyFrom

*/

template <typename Unit, typename Alpha>
inline void LMapping<Unit, Alpha>::CopyFrom( const Attribute* right )
{
    const LMapping<Unit, Alpha> *r = (const LMapping<Unit, Alpha>*)right;
    Clear();
    SetDefined( r->IsDefined() );
    if( !r->IsDefined() )
        return;

    assert( r->IsOrdered() );
    StartBulkLoad();
    Unit unit;
    for( int i = 0; i < r->GetNoComponents(); i++ )
    {
        r->Get( i, unit );
        Add( unit );
    }
    EndBulkLoad( false );
    this->SetDefined(r->IsDefined());
}

/*
1.1.5.5 NumOfFLOBs

*/

template <typename Unit, typename Alpha>
inline int LMapping<Unit, Alpha>::NumOfFLOBs() const
{
    return 1;
}

/*
1.1.5.6 GetFLOB

*/
template <typename Unit, typename Alpha>
inline Flob *LMapping<Unit, Alpha>::GetFLOB(const int i)
{
    assert( i == 0 );
    return &units;
}

/*
1.1.5.7 Print

*/

template <typename Unit, typename Alpha>
inline std::ostream& LMapping<Unit, Alpha>::Print( std::ostream &os ) const
{
    if( !IsDefined() )
    {
        return os << "("<< LMapping<Unit,Alpha>::BasicType() <<": undefined)";
    }
    os  << "("<< LMapping<Unit,Alpha>::BasicType()
        <<": defined, contains " << GetNoComponents() << " units: ";
    for(int i=0; i<GetNoComponents(); i++)
    {
        Unit unit;
        Get( i , unit );
        os << "\n\t";
        unit.Print(os);
    }
    os << "\n)" << std::endl;
    return os;
}

/*
1.1.5.8 Clone

*/

template <typename Unit, typename Alpha>
inline Attribute* LMapping<Unit, Alpha>::Clone() const
{
    LMapping<Unit, Alpha> *result;

    if( !IsDefined() )
    {
        result = new LMapping<Unit, Alpha>( 0 );
        result->SetDefined( false );
        return result;
    }
    result = new LMapping<Unit, Alpha>( GetNoComponents() );
    result->SetDefined( true );

    assert( IsOrdered() );

    if(GetNoComponents()>0)
        result->units.resize(GetNoComponents());

    result->StartBulkLoad();
    Unit unit;
    for( int i = 0; i < GetNoComponents(); i++ )
    {
        Get( i, unit );
        result->Add( unit );
    }
    result->EndBulkLoad( false );
    return result;
}

/*
1.1.5.9 SizeOf

*/

template <typename Unit, typename Alpha>
inline size_t LMapping<Unit, Alpha>::Sizeof() const
{
    return sizeof( *this );
}


/*
1.2 Type Constructor ~LMapping~

1.2.1 ~Out~-function

*/
template <class LMapping, class Unit, ListExpr (*OutUnit)( ListExpr, Word )>
ListExpr OutLMapping( ListExpr typeInfo, Word value )
{
    LMapping* m = (LMapping*)(value.addr);
    if(! m->IsDefined())
        return nl->SymbolAtom(Symbol::UNDEFINED());
    else
    if( m->IsEmpty() )
        return (nl->TheEmptyList());
    else
    {
        assert( m->IsOrdered() );
        ListExpr l = nl->TheEmptyList();
        ListExpr lastElem = nl->TheEmptyList();
        ListExpr unitList;

    for( int i = 0; i < m->GetNoComponents(); i++ )
    {
        Unit unit;
        m->Get( i, unit );
        unitList = OutUnit( nl->TheEmptyList(), SetWord(&unit) );
        if( l == nl->TheEmptyList() )
        {
            l = nl->Cons( unitList, nl->TheEmptyList() );
            lastElem = l;
        }
        else
            lastElem = nl->Append( lastElem, unitList );
    }
        return l;
    }
}

/*
1.2.2 ~In~-function

*/
template <  class LMapping, class Unit,
            Word (*InUnit)( const ListExpr, const ListExpr,
            const int, ListExpr&, bool& )>
Word InLMapping( const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct )
{
    int numUnits = 0;
    if(nl->AtomType(instance)==NoAtom)
        numUnits = nl->ListLength(instance);
    LMapping* m = new LMapping( numUnits );
    correct = true;
    int unitcounter = 0;
    std::string errmsg;

    m->StartBulkLoad();

    ListExpr rest = instance;
    if (nl->AtomType( rest ) != NoAtom)
    {
        if(listutils::isSymbolUndefined(rest))
        {
            m->EndBulkLoad();
            m->SetDefined(false);
            return SetWord( Address( m ) );
        }
        else
        {
            correct = false;
            m->Destroy();
            delete m;
            return SetWord( Address( 0 ) );
        }
    }
    else while( !nl->IsEmpty( rest ) )
    {
        ListExpr first = nl->First( rest );
        rest = nl->Rest( rest );

        Unit *unit = (Unit*)InUnit( nl->TheEmptyList(), first,
                                    errorPos, errorInfo, correct ).addr;

        if ( !correct )
        {
        errmsg = "InLMapping(): Representation of Unit "
                + std::to_string(unitcounter) + " is wrong.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        m->Destroy();
        delete m;
        return SetWord( Address(0) );
        }
        if( !unit->IsDefined() || !unit->IsValid() )
        {
            errmsg = "InMapping(): Unit "
                + std::to_string(unitcounter) + " is undef.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            correct = false;
            delete unit;
            m->Destroy();
            delete m;
            return SetWord( Address(0) );
        }
        m->Add( *unit );
        unitcounter++;
        delete unit;
    }

    if(! m->EndBulkLoad( true,true ))
    { // overlapping intervals found
        m->Destroy();
        delete m;
        correct = false;
        return SetWord(Address(0));
    }

    return SetWord( m );
}

/*
1.2.3 ~Open~-function

*/
template <class LMapping>
bool OpenLMapping( SmiRecord& valueRecord,
    const ListExpr typeInfo, Word& value )
{
    LMapping *m = new LMapping( 0 );
    m->Open( valueRecord, typeInfo );
    value = SetWord( m );
    return true;
}

/*
1.2.4 ~Save~-function

*/
template <class LMapping>
bool SaveLMapping( SmiRecord& valueRecord,
                   const ListExpr typeInfo, Word& value )
{
    LMapping *m = (LMapping *)value.addr;
    m->Save( valueRecord, typeInfo );
    return true;
}

/*
1.2.5 ~Create~-function

*/
template <class LMapping>
Word CreateLMapping( const ListExpr typeInfo )
{
    return (SetWord( new LMapping( 0 ) ));
}

/*
1.2.6 ~Delete~-function

*/
template <class LMapping>
void DeleteLMapping( const ListExpr typeInfo, Word& w )
{
    ((LMapping *)w.addr)->Destroy();
    delete (LMapping *)w.addr;
    w.addr = 0;
}

/*
1.2.7 ~Close~-function

*/
template <class LMapping>
void CloseLMapping( const ListExpr typeInfo, Word& w )
{
    ((LMapping *)w.addr)->DeleteIfAllowed();
    w.addr = 0;
}

/*
1.2.8 ~Clone~-function

*/
template <class LMapping>
Word CloneLMapping( const ListExpr typeInfo, const Word& w )
{
    return SetWord( ((LMapping *)w.addr)->Clone() );
}

/*
1.2.9 ~Sizeof~-function

*/
template <class LMapping>
int SizeOfLMapping()
{
    return sizeof(LMapping);
}

/*
1.2.10 ~Cast~-function

*/
template <class LMapping>
void* CastLMapping(void* addr)
{
    return new (addr) LMapping;
}

#endif //LMAPPING_H
