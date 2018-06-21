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
//[bl] [\\]

[1] Implementation of Module

April 2015 Rene Steinbrueck
[TOC]

1 Overview

2 Inclusion of the Header File

*/

#include "LineFunctionAlgebra.h"
//#include <iostream>
#include "Algebras/Spatial/Geoid.h"

/*
3.1 Type Constructor ~lubool~

Type ~lubool~ represents an (linterval, boolvalue)-pair.

3.1.1 List Representation

The list representation of an ~lubool~ is

----    ( lengthInterval bool-value )
----

For example:

----    ( 1.0 5.2 TRUE FALSE) TRUE )
----

3.1.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LUBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LUNIT"),
                             nl->StringAtom("(lubool) "),
                             nl->StringAtom("(lengthInterval bool) "),
                             nl->StringAtom("((1.3 5.2 FALSE FALSE) TRUE)"))));
}

/*
3.1.3 Kind Checking Function

*/
bool
CheckLUBool( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, LUBool::BasicType() ));
}

/*
3.1.4 Creation of the type constructor ~lubool~

*/
TypeConstructor lunitbool(
        LUBool::BasicType(),   //name
        LUBoolProperty,    //property function describing signature
        OutConstLengthUnit<CcBool, OutCcBool>,
        InConstLengthUnit<CcBool, InCcBool>,    //Out and In functions
        0,     0,  //SaveToList and RestoreFromList functions
        CreateConstLengthUnit<CcBool>,
        DeleteConstLengthUnit<CcBool>,          //object creation and deletion
        OpenAttribute<LUBool>,
        SaveAttribute<LUBool>,  // object open and save
        CloseConstLengthUnit<CcBool>,
        CloneConstLengthUnit<CcBool>,           //object close and clone
        CastConstLengthUnit<CcBool>,            //cast function
        SizeOfConstLengthUnit<CcBool>,          //sizeof function
        CheckLUBool );                             //kind checking function

/*
3.2 Type Constructor ~luint~

Type ~luint~ represents an (linterval, intvalue)-pair.

3.2.1 List Representation

The list representation of an ~luint~ is

----    ( lengthInterval int-value )
----

For example:

----    ( ( 1.0 5.7 TRUE FALSE)   5 )
----

3.2.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LUIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LUNIT"),
                             nl->StringAtom("(luint) "),
                             nl->StringAtom("(lengthInterval int) "),
                             nl->StringAtom("((1.3 5.2 FALSE FALSE) 1)"))));
}

/*
3.2.3 Kind Checking Function

*/
bool
CheckLUInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, LUInt::BasicType() ));
}

/*
3.2.4 Creation of the type constructor ~luint~

*/
TypeConstructor lunitint(
        LUInt::BasicType(),     //name
        LUIntProperty, //property function describing signature
        OutConstLengthUnit<CcInt, OutCcInt>,
        InConstLengthUnit<CcInt, InCcInt>, //Out and In functions
        0,                      0,//SaveToList and RestoreFromList functions
        CreateConstLengthUnit<CcInt>,
        DeleteConstLengthUnit<CcInt>, //object creation and deletion
        OpenAttribute<LUInt>,
        SaveAttribute<LUInt>,  // object open and save
        CloseConstLengthUnit<CcInt>,
        CloneConstLengthUnit<CcInt>, //object close and clone
        CastConstLengthUnit<CcInt>,       //cast function
        SizeOfConstLengthUnit<CcInt>, //sizeof function
        CheckLUInt );                    //kind checking function
/*
3.3 Type Constructor ~lustring~

Type ~lustring~ represents an (linterval, intvalue)-pair.

3.3.1 List Representation

The list representation of an ~lustring~ is

----    ( lengthInterval string-value )
----

For example:

----    ( ( 1.0 5.7 TRUE FALSE) "Hello" )
----

3.3.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LUStringProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                    nl->StringAtom("Example Type List"),
                    nl->StringAtom("List Rep"),
                    nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LUNIT"),
                    nl->StringAtom("(lustring) "),
                    nl->StringAtom("(lengthInterval string) "),
                    nl->StringAtom("((1.3 5.2 FALSE FALSE) 'Hello')"))));
}

/*
3.3.3 Kind Checking Function

*/
bool
CheckLUString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, LUString::BasicType() ));
}

/*
3.3.4 Creation of the type constructor ~lustring~

*/
TypeConstructor lunitstring(
        LUString::BasicType(),     //name
        LUStringProperty, //property function describing signature
        OutConstLengthUnit<CcString, OutCcString>,
        InConstLengthUnit<CcString, InCcString>, //Out and In functions
        0,                      0,//SaveToList and RestoreFromList functions
        CreateConstLengthUnit<CcString>,
        DeleteConstLengthUnit<CcString>, //object creation and deletion
        OpenAttribute<LUString>,
        SaveAttribute<LUString>,  // object open and save
        CloseConstLengthUnit<CcString>,
        CloneConstLengthUnit<CcString>, //object close and clone
        CastConstLengthUnit<CcString>,       //cast function
        SizeOfConstLengthUnit<CcString>, //sizeof function
        CheckLUString );

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
  std::string errmsg;
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


/*
3.5 Type Constructor ~lbool~

Type ~lbool~ represents a length boolean.

3.5.1 List Representation

The list representation of a ~lbool~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~lubool~.

For example:

----    (
          ( (1.0 5.4 TRUE FALSE) TRUE )
          ( (5.5 6.8 FALSE FALSE) FALSE )
        )
----

3.5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LMAPPING"),
                             nl->StringAtom("(lbool) "),
                             nl->StringAtom("( u1 ... un)"),
           nl->StringAtom("(((1.3 5.2 TRUE TRUE) TRUE) ...)"))));
}

/*
3.5.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckLBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, LBool::BasicType() ));
}

/*
3.5.4 Creation of the type constructor ~lbool~

*/
TypeConstructor lengthbool(
        LBool::BasicType(), //name
        LBoolProperty,  //property function describing signature
        OutLMapping<LBool, LUBool, OutConstLengthUnit<CcBool, OutCcBool> >,
        InLMapping<LBool, LUBool, InConstLengthUnit<CcBool, InCcBool> >,
       //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateLMapping<LBool>,
        DeleteLMapping<LBool>,        //object creation and deletion
        OpenAttribute<LBool>,
        SaveAttribute<LBool>,          // object open and save
        CloseLMapping<LBool>,
        CloneLMapping<LBool>,     //object close and clone
        CastLMapping<LBool>,     //cast function
        SizeOfLMapping<LBool>,    //sizeof function
        CheckLBool );          //kind checking function

/*
3.6 Type Constructor ~lint~

Type ~lint~ represents a length integer.

3.6.1 List Representation

The list representation of a ~lint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~lint~.

For example:

----    (
          ( 1.0 5.4 TRUE FALSE) 1 )
          ( 5.5 8.2 FALSE FALSE) 4 )
        )
----

3.6.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LMAPPING"),
                             nl->StringAtom("(lint) "),
                             nl->StringAtom("( u1 ... un)"),
                             nl->StringAtom("(((1.3 5.2 TRUE TRUE) 1) ...)"))));
}

/*
3.6.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckLInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, LInt::BasicType() ));
}

/*
3.6.4 Creation of the type constructor ~lint~

*/
TypeConstructor lengthint(
        LInt::BasicType(),               //name
        LIntProperty,   //property function describing signature
        OutLMapping<LInt, LUInt, OutConstLengthUnit<CcInt, OutCcInt> >,
        InLMapping<LInt, LUInt, InConstLengthUnit<CcInt, InCcInt> >,
    //Out and In functions
        0,
        0,            //SaveToList and RestoreFromList functions
        CreateLMapping<LInt>,
        DeleteLMapping<LInt>,   //object creation and deletion
        OpenAttribute<LInt>,
        SaveAttribute<LInt>,           // object open and save
        CloseLMapping<LInt>,
        CloneLMapping<LInt>, //object close and clone
        CastLMapping<LInt>,  //cast function
        SizeOfLMapping<LInt>,  //sizeof function
        CheckLInt );      //kind checking function

/*
3.7 Type Constructor ~lstring~

Type ~lstring~ represents a length string.

3.7.1 List Representation

The list representation of a ~lstring~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~lstring~.

For example:

----    (
          ( 1.0 5.4 TRUE FALSE) 1 )
          ( 5.5 8.2 FALSE FALSE) 4 )
        )
----

3.7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LStringProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LMAPPING"),
                             nl->StringAtom("(lstring) "),
                             nl->StringAtom("( u1 ... un)"),
                             nl->StringAtom("(((1.3 5.2 TRUE TRUE) 1) ...)"))));
}

/*
3.7.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckLString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, LString::BasicType() ));
}

/*
3.7.4 Creation of the type constructor ~lstring~

*/
TypeConstructor lengthstring(
        LString::BasicType(),               //name
        LStringProperty,   //property function describing signature
        OutLMapping<LString, LUString,
            OutConstLengthUnit<CcString, OutCcString> >,
        InLMapping<LString, LUString, InConstLengthUnit<CcString, InCcString> >,
    //Out and In functions
        0,
        0,            //SaveToList and RestoreFromList functions
        CreateLMapping<LString>,
        DeleteLMapping<LString>,   //object creation and deletion
        OpenAttribute<LString>,
        SaveAttribute<LString>,           // object open and save
        CloseLMapping<LString>,
        CloneLMapping<LString>, //object close and clone
        CastLMapping<LString>,  //cast function
        SizeOfLMapping<LString>,  //sizeof function
        CheckLString );      //kind checking function

/*
3.8 Type Constructor ~lreal~

Type ~lreal~ represents a length real.

3.8.1 List Representation

The list representation of a ~lreal~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~lureal~.

For example:

----    (
          ( 6.37  9.9  TRUE FALSE) (1.0 2.3) )
          ( 11.4  13.9  FALSE FALSE) (2.0 2.8) )
        )
----

3.8.2 function Describing the Signature of the Type Constructor

*/
ListExpr
LRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> LMAPPING"),
                             nl->StringAtom("(lreal) "),
                             nl->StringAtom("( u1 ... un) "),
           nl->StringAtom("(((1.3 5.2 TRUE FALSE) (1.0 2.2)) ...)"))));
}

/*
3.8.3 Kind Checking Function

*/
bool
CheckLReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, LReal::BasicType() ));
}

/*
3.8.4 Creation of the type constructor ~lreal~

*/
TypeConstructor lengthreal(
        LReal::BasicType(),                    //name
        LRealProperty,    //property function describing signature
        OutLMapping<LReal, LUReal, OutLUReal>,
        InLMapping<LReal, LUReal, InLUReal>,   //Out and In functions
        0,
        0,      //SaveToList and RestoreFromList functions
        CreateLMapping<LReal>,
        DeleteLMapping<LReal>,    //object creation and deletion
        OpenAttribute<LReal>,
        SaveAttribute<LReal>,        // object open and save
        CloseLMapping<LReal>,
        CloneLMapping<LReal>,    //object close and clone
        CastLMapping<LReal>,    //cast function
        SizeOfLMapping<LReal>,    //sizeof function
        CheckLReal );                        //kind checking function


/*
4 Operators

*/

double DistanceWithHeight(const Point& pointSource, const Point& pointTarget,
 CcReal& weight, LReal& heightfunction, const Geoid* geoid )
{
  assert( pointSource.IsDefined() );
  assert( pointTarget.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  assert( weight.IsDefined() );

  const Geoid mygeoid(Geoid::WGS1984);

    bool ok = false;
    double bearInitial = 0, bearFinal = 0;
//    std::cout << "LineFunctionA.cpp mygeoid: \t Address: " << mygeoid ;
//      std::cout << "\t Content: " << mygeoid << "\n";
    double distance = pointSource.DistanceOrthodromePrecise(pointTarget,
        mygeoid,ok,bearInitial,bearFinal);
    /*hier muesste das Gewicht pro Steigung uebergeben werden und die 
    HeightDifference Methode angepasst werden, wenn es diese Tabellenfunktion
     geben soll*/
    double height= HeightDifference(heightfunction);
    //distance und height sind hier in koordinateneinheiten, nicht in metern
    double distanceWithHeigt= distance + (height * weight.GetValue());
    return distanceWithHeigt;
}


double HeightDifference(LReal heightfunction)
{
    double heightDiff=0;

    if(!heightfunction.IsDefined()){
        return heightDiff;
    }
    heightfunction.Print(cout);
//    cout << heightfunction.GetNoComponents() << "\n";
    for (int i = 0; i < heightfunction.GetNoComponents(); i++) {
        LUReal unit;
//        cout << "LFA.cpp Line " << __LINE__ << ", i: " << i << "\n";
        heightfunction.Get(i,unit);
        //Steigung des Intervals
        double steigung=unit.m;

        double intervStart=unit.getLengthInterval().start.GetRealval();
        double intervEnd=unit.getLengthInterval().end.GetRealval();

        //Laenge des Intervals
        double intervalLength= intervEnd-intervStart;

        if(steigung>0){
            heightDiff=heightDiff + steigung*intervalLength;
        }
        //cout<< "m:" << steigung <<"\t istart:"<<intervStart
//        cout<<"\t iende:"<<intervEnd<<"\t heightDiff:"<<heightDiff;
    }
    //cout<<"\n------------------\n";

return heightDiff;
}

/*
2 Operators

*/

ValueMapping heightatpositionFuns[] =
{
    heightatpositionFun<raster2::sint>,
    heightatpositionFun<raster2::sreal>,
    0
};


int heightatpositionSelectFun(ListExpr args)
{
    NList type(args);

    if (type.first().isSymbol(raster2::sint::BasicType()))
    {
        return (0);
    };
    if (type.first().isSymbol(raster2::sreal::BasicType()))
    {
        return (1);
    };
 return (-1);
}

ListExpr heightatpositionTypeMap(ListExpr args)
{
    if(!nl->HasLength(args,2))
    {
        return (listutils::typeError("2 arguments expected"));
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);

    std::string err = "stype x point expected";

        if(!raster2::util::isSType(arg1))
            return (listutils::typeError(err + " (first arg is not an stype)"));
        if(!Point::checkType(arg2))
            return (listutils::typeError(err
                + " (second arg is not an point)"));

    return (nl->SymbolAtom(CcReal::BasicType()));
}

/*
1.2 Operator ~lcompose~

*/

struct lcomposeInfo : OperatorInfo
{
    lcomposeInfo()
    {
    name      = "lcompose";
    signature = raster2::sbool::BasicType()
        + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lbool";
    appendSignature(raster2::sreal::BasicType() + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lreal");
    appendSignature(raster2::sint::BasicType() + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lint");
    appendSignature(raster2::sstring::BasicType() + " lcompose "
        + SimpleLine::BasicType()
        + " , "
        + CcBool::BasicType() + "-> lstring");

    syntax    = "_ lcompose [_,_]";
    meaning   = "This function merges sline, sT and boolean into lT. "
    "If the bool parameter is TRUE, the Distance between "
    "two points on the line is calculated by orthodrome "
    "distance, otherwise by the euclidean distance.";
    }
};

template <typename ST, typename UT, typename RT>
int constlcomposeFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
{
    // storage for the result
    result = qp->ResultStorage(s);

    // the sT object
    ST* raster = static_cast<ST*>(args[0].addr);

    // the simple Line
    SimpleLine* simpleLine = static_cast<SimpleLine*>(args[1].addr);

    // the computation method for distance
    //(true = Distance Orthodrome, false = Euclidean Distance)
    CcBool* distCalc = static_cast<CcBool*>(args[2].addr);

    // The result of the lcompose
    RT* pResult = static_cast<RT*>(result.addr);


    if (!simpleLine->IsDefined() || !raster->isDefined())
    {
            pResult->SetDefined(false);
            return 0;
    }

    pResult->Clear();
    pResult->StartBulkLoad();

    // get the number of components
    int num = simpleLine->Size();


    HalfSegment unit;
    raster2::grid2 grid = raster->getGrid();
    raster2::grid2::index_type cell1;
    raster2::grid2::index_type cell2;
    double start = 0.0;
    double end = 0.0;
    bool definedGeoID;
    bool distOrthodrome = distCalc->GetBoolval();

    if (simpleLine->StartsSmaller())
    {
        for (int i = 0; i < num; i++)
        {
            simpleLine->Get(i,unit);
            if (unit.IsLeftDomPoint())
            {
                // get the coordinates
                double xStart = unit.GetLeftPoint().GetX();
                double yStart = unit.GetLeftPoint().GetY();
                double xEnd = unit.GetRightPoint().GetX();
                double yEnd = unit.GetRightPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetLeftPoint().DistanceOrthodrome(
                    unit.GetRightPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetLeftPoint().Distance(unit.GetRightPoint());
                end = start + distance;


                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);

                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end),true, (i==(num-2))?true:false);

                    typename ST::cell_type v1 = raster->atlocation(
                        (xEnd-xStart)/2,(yEnd-yStart)/2);
                    if(!raster->isUndefined(v1))
                    {
                        typename ST::wrapper_type v(v1);
                        pResult->MergeAdd(UT(linterval,v));
                    }
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);

                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;
                    while(it.hasNext())
                    {
                        std::pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);
                        if(e>s)
                        {
                            LInterval linterval(CcReal(s),
                                CcReal(e),true,(i==(num-2))?true:false);
                            double delta  =(p.first + p.second) / 2.0;
                            double x = xStart + delta*dx;
                            double y = yStart + delta*dy;
                            typename ST::cell_type v1 = raster->atlocation(x,y);
                            if(!raster->isUndefined(v1))
                            {
                                typename ST::wrapper_type v(v1);
                                pResult->MergeAdd(UT(linterval,v));
                            }
                        }
                        else
                        assert(e==s);
                    }
                }

            }
        }
    }
    else
    {
        for (int i = num-1; i >=0; i--)
        {
            simpleLine->Get(i,unit);
            if (!unit.IsLeftDomPoint())
            {

                // get the coordinates
                double xStart = unit.GetRightPoint().GetX();
                double yStart = unit.GetRightPoint().GetY();
                double xEnd = unit.GetLeftPoint().GetX();
                double yEnd = unit.GetLeftPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetRightPoint().DistanceOrthodrome(
                    unit.GetLeftPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetRightPoint().Distance(unit.GetLeftPoint());
                end = start + distance;


                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);

                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end),true, (i==0)?true:false);

                    typename ST::cell_type v1 =
                        raster->atlocation((xEnd-xStart)/2,(yEnd-yStart)/2);
                    if(!raster->isUndefined(v1))
                    {
                        typename ST::wrapper_type v(v1);
                        pResult->MergeAdd(UT(linterval,v));
                    }
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);

                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;
                    while(it.hasNext())
                    {
                        std::pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);
                        if(e>s)
                        {
                            LInterval linterval(CcReal(s),
                                CcReal(e),true,(i==0)?true:false);
                            double delta  =(p.first + p.second) / 2.0;
                            double x = xStart + delta*dx;
                            double y = yStart + delta*dy;
                            typename ST::cell_type v1 = raster->atlocation(x,y);
                            if(!raster->isUndefined(v1))
                            {
                                typename ST::wrapper_type v(v1);
                                pResult->MergeAdd(UT(linterval,v));
                            }
                        }
                        else
                        assert(e==s);
                    }
                }

            }
        }
    }
    pResult->EndBulkLoad();
    return 0;
}



template <typename ST>
int reallcomposeFun
    (Word* args, Word& result, int message, Word& local, Supplier s)
{
    // storage for the result
    result = qp->ResultStorage(s);

    // the sT object
    ST* raster = static_cast<ST*>(args[0].addr);

    // the simple Line
    SimpleLine* simpleLine = static_cast<SimpleLine*>(args[1].addr);

    // the computation method for distance
    //(true = Distance Orthodrome, false = Euclidean Distance)
    CcBool* distCalc = static_cast<CcBool*>(args[2].addr);

    // The result of the lcompose
    LReal* pResult = static_cast<LReal*>(result.addr);


    if (!simpleLine->IsDefined() || !raster->isDefined())
    {
            pResult->SetDefined(false);
            return 0;
    }

    pResult->Clear();
    pResult->StartBulkLoad();

    // get the number of components
    int num = simpleLine->Size();

    HalfSegment unit;
    raster2::grid2 grid = raster->getGrid();
    raster2::grid2::index_type cell1;
    raster2::grid2::index_type cell2;
    double start = 0.0;
    double end = 0.0;
    bool definedGeoID;
    bool distOrthodrome = distCalc->GetBoolval();

    if (simpleLine->StartsSmaller())
    {
        for (int i = 0; i < num; i++)
        {
            simpleLine->Get(i,unit);
            if (unit.IsLeftDomPoint())
            {
                // get the coordinates
                double xStart = unit.GetLeftPoint().GetX();
                double yStart = unit.GetLeftPoint().GetY();
                double xEnd = unit.GetRightPoint().GetX();
                double yEnd = unit.GetRightPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetLeftPoint().DistanceOrthodrome(
                    unit.GetRightPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetLeftPoint().Distance(unit.GetRightPoint());
                end = start + distance;
                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);


                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end), true, (i==(num-2))?true:false);
                    double v1 = getHeightAtPosition<ST>(xStart,yStart,*raster);
                    double v2 = getHeightAtPosition<ST>(xEnd,yEnd,*raster);
                    if(!raster->isUndefined(v1) && !raster->isUndefined(v2))
                        pResult->MergeAdd(LUReal(linterval,v1,v2,false));
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);
                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;

                    while(it.hasNext())
                    {
                        std::pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);

                        if(e>s)
                        {
                            LInterval linterval(CcReal(s),
                                CcReal(e),true,(i==(num-2))?true:false);
                            double xs = xStart + p.first * dx;
                            double ys = yStart + p.first * dy;
                            double xe = xStart + p.second * dx;
                            double ye = yStart + p.second * dy;
                            double v1 = getHeightAtPosition<ST>(xs,ys,*raster);
                            double v2 = getHeightAtPosition<ST>(xe,ye,*raster);
                            if(!raster->isUndefined(v1) &&
                                !raster->isUndefined(v2))
                                pResult->MergeAdd(
                                    LUReal(linterval,v1,v2,false));
                        }
                        else
                        assert(e==s);
                    }
                }
            }
        }
    }
    else
    {
        for (int i = num-1; i >=0; i--)
        {
            simpleLine->Get(i,unit);
            if (!unit.IsLeftDomPoint())
            {
                // get the coordinates
                double xStart = unit.GetLeftPoint().GetX();
                double yStart = unit.GetLeftPoint().GetY();
                double xEnd = unit.GetRightPoint().GetX();
                double yEnd = unit.GetRightPoint().GetY();

                start = end;
                double distance = distOrthodrome
                ? unit.GetLeftPoint().DistanceOrthodrome(
                    unit.GetRightPoint(),Geoid(true),definedGeoID)/1000
                : unit.GetLeftPoint().Distance(unit.GetRightPoint());
                end = start + distance;
                cell1 = grid.getIndex(xStart,yStart);
                cell2 = grid.getIndex(xEnd, yEnd);


                if(cell1==cell2)
                { // only a constant unit in result
                    LInterval linterval(CcReal(start),
                        CcReal(end), true, (i==0)?true:false);
                    double v1 = getHeightAtPosition<ST>(xStart,yStart,*raster);
                    double v2 = getHeightAtPosition<ST>(xEnd,yEnd,*raster);
                    if(!raster->isUndefined(v1) && !raster->isUndefined(v2))
                        pResult->MergeAdd(LUReal(linterval,v1,v2,false));
                }

                else
                {
                    CellIterator it(grid,xStart,yStart,xEnd,yEnd);
                    double dx = xEnd - xStart;
                    double dy = yEnd - yStart;

                    while(it.hasNext())
                    {
                        std::pair<double,double> p = it.next();
                        double s = start + (distance*p.first);
                        double e = start + (distance*p.second);

                        if(e>s)
                        {
                            LInterval linterval(
                                CcReal(s),CcReal(e),true,(i==0)?true:false);
                            double xs = xStart + p.first * dx;
                            double ys = yStart + p.first * dy;
                            double xe = xStart + p.second * dx;
                            double ye = yStart + p.second * dy;
                            double v1 = getHeightAtPosition<ST>(xs,ys,*raster);
                            double v2 = getHeightAtPosition<ST>(xe,ye,*raster);
                            if(!raster->isUndefined(v1)
                                && !raster->isUndefined(v2))
                                pResult->MergeAdd(
                                    LUReal(linterval,v1,v2,false));
                        }
                        else
                        assert(e==s);
                    }
                }
            }
        }
    }

    pResult->EndBulkLoad();

return 0;
}


ValueMapping lcomposeFuns[] =
{
    constlcomposeFun<raster2::sbool, LUBool, LBool>,
    constlcomposeFun<raster2::sstring, LUString, LString>,
    reallcomposeFun<raster2::sint>,
    reallcomposeFun<raster2::sreal>,
    0
};


int lcomposeSelectFun(ListExpr args)
{
std::string errmsg = "lcomposeSelectFun started";

    NList type(args);

        if (type.first().isSymbol(raster2::sbool::BasicType()))
            return 0;
        if (type.first().isSymbol(raster2::sstring::BasicType()))
            return 1;
        if (type.first().isSymbol(raster2::sint::BasicType()))
            return 2;
        if(type.first().isSymbol(raster2::sreal::BasicType()))
            return 3;
        return -1; //this point should never be reached
}

ListExpr lcomposeTypeMap(ListExpr args)
{
std::string errmsg = "lcomposeTypeMap started";

    if(!nl->HasLength(args,3))
        return listutils::typeError("3 arguments expected");

    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);

    std::string err = "stype x sline x bool expected";
    if(!raster2::util::isSType(arg1))
        return listutils::typeError(err + " (first arg is not an stype)");
    if(!SimpleLine::checkType(arg2))
        return listutils::typeError(err + " (second arg is not an sline)");
    if(!CcBool::checkType(arg3))
        return listutils::typeError(err + " (third arg is not an bool)");

    std::string sname = nl->SymbolValue(arg1);
    if (sname==raster2::sbool::BasicType())
        return (nl->SymbolAtom(LBool::BasicType()));
    if (sname==raster2::sstring::BasicType())
        return (nl->SymbolAtom(LString::BasicType()));
    if (sname==raster2::sreal::BasicType())
        return (nl->SymbolAtom(LReal::BasicType()));
    if (sname==raster2::sint::BasicType())
        return (nl->SymbolAtom(LReal::BasicType()));
    return listutils::typeError();
}


/*
1.3 Operator ~lfdistance~

*/

struct lfdistanceInfo : OperatorInfo
    {
    lfdistanceInfo()
    {
        name      = "lfdistance";
        signature = "lfdistance ("
          + Point::BasicType()
          + " , "
          + Point::BasicType()
              + ", "
          + raster2::sint::BasicType()
          + " , "
          + CcInt::BasicType()
          + "-> Real";
        appendSignature("lfdistance ("
              + Point::BasicType()
              + " , "
              + Point::BasicType()
                + " , "
              + raster2::sreal::BasicType()
              + " , "
                + CcInt::BasicType()
                + "-> Real");

        syntax    =   "lfdistance ( _ , _ , _ , _ )";
        meaning   =   "This function computes the distance "
      "between two points including their height. "
      "The First two parameters are "
          "the two points, between the distance "
      "should be calculated "
          "and the third parameter is a raster, "
      "from which the height "
      "could be get. "
      "The fourth parameter is 1 or 2: "
      "1 for computing the pure "
      "driving distance or "
      "2 for computing the driving distance, "
      "depending on the gradient.";
      }
    };

template<typename Raster> int lfdistanceFun
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  Point* a = static_cast<Point*>(args[0].addr);
  Point* b = static_cast<Point*>(args[1].addr);
  Raster* raster = static_cast<Raster*>(args[2].addr);
  CcInt* suchart = static_cast<CcInt*>(args[3].addr);
  CcReal* res = static_cast<CcReal*>(result.addr);

    bool checkCoord;

  if(!a->IsDefined() || !b->IsDefined() || a->IsEmpty() || b->IsEmpty() ||
       !suchart->IsDefined() || !( suchart->GetIntval() == 1 ||
            suchart->GetIntval() == 2 ) )
  {
    res->SetDefined(false);
  }
  else
  {
        if (suchart->GetIntval() == 1)
        {
            double airDistance =
                a->DistanceOrthodrome(*b,Geoid(true),checkCoord);
            double heightA=getHeightAtPosition(a->GetX(),a->GetY(),*raster);
            double heightB=getHeightAtPosition(b->GetX(),b->GetY(),*raster);
            double dheight=heightB-heightA;
            double drivingDistance=sqrt(pow(dheight,2)+pow(airDistance,2));
            res->Set(true, drivingDistance);
        }
        if (suchart->GetIntval() == 2)
        {
            double airDistance =
                a->DistanceOrthodrome(*b,Geoid(true),checkCoord);
            double heightA=getHeightAtPosition(a->GetX(),a->GetY(),*raster);
            double heightB=getHeightAtPosition(b->GetX(),b->GetY(),*raster);
            double dheight=heightB-heightA;
            double drivingDistance=sqrt(pow(dheight,2)+pow(airDistance,2));
            double climb = dheight/airDistance;
            double dist=0.0;


            if ((climb < (-1.0))||(climb > 1.0))
                res->SetDefined(false);
            else if ((climb >= -1.00)&&(climb <= -0.10))
                dist = drivingDistance *  0.30;
      else if ((climb >  -0.10)&&(climb <  -0.01))
                dist = drivingDistance *  0.60;
      else if ((climb >= -0.01)&&(climb <=  0.01))
                dist = drivingDistance *  1.00;
      else if ((climb >   0.01)&&(climb <   0.02))
                dist = drivingDistance *  1.25;
      else if ((climb >=  0.02)&&(climb <   0.05))
                dist = drivingDistance *  1.75;
      else if ((climb >=  0.05)&&(climb <   0.10))
                dist = drivingDistance *  2.75;
      else if ((climb >=  0.10)&&(climb <=  1.00))
                dist = drivingDistance * 20.00;

            res->Set(true,dist);
        }
    }
  return (0);
}


ValueMapping lfdistanceFuns[] =
{
    lfdistanceFun<raster2::sint>,
    lfdistanceFun<raster2::sreal>,
    0
};


int lfdistanceSelectFun(ListExpr args)
{
  NList type(args);

  if (type.third().isSymbol(raster2::sint::BasicType()))
  {
    return (0);
  }
  if(type.third().isSymbol(raster2::sreal::BasicType()))
  {
    return (1);
  }
  return (-1);
}

ListExpr lfdistanceTypeMap(ListExpr args)
{
  if(!nl->HasLength(args,4)){
    return (listutils::typeError("4 arguments expected"));
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  std::string err = "Point x Point x stype x int expected";
  if(!Point::checkType(arg1))
  {
    return (listutils::typeError(err
            + " (first arg is not an point)"));
  }
  if(!Point::checkType(arg2))
  {
    return (listutils::typeError(err
            + " (second arg is not an point)"));
  }
  if(!raster2::util::isSType(arg3)){
    return (listutils::typeError(err
            + " (third arg is not an stype)"));
  }
  if(!CcInt::checkType(arg4))
  {
    return (listutils::typeError(err
            + " (fourth arg is not an int)"));
  }
  return (listutils::basicSymbol<CcReal>());
}

/*
1.4 Operator ~lfdistanceparam~

*/

struct lfdistanceparamInfo : OperatorInfo
    {
    lfdistanceparamInfo()
    {
        name      = "lfdistanceparam";
        signature = "lfdistanceparam ("
          + Point::BasicType()
          + " , "
          + Point::BasicType()
              + " , "
          + raster2::sint::BasicType()
              + " , "
              + CcString::BasicType()
              + " , "
              + CcString::BasicType()
              + " , "
              + CcString::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + "-> Real )";
        appendSignature("lfdistanceparam ("
              + Point::BasicType()
              + " , "
              + Point::BasicType()
              + " , "
              + raster2::sreal::BasicType()
              + " , "
              + CcString::BasicType()
              + " , "
              + CcString::BasicType()
              + " , "
              + CcString::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + " , "
              + CcReal::BasicType()
              + "-> Real )");

        syntax    = "lfdistanceparam ( _ , _ , _ , _ , _ , _ , "
            "_ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ , _ )";
        meaning   =   "This function computes the distance "
      "between two points including their height. "
                     "The First two parameters are the two points, "
                     "between the distance should be calculated "
      "and the third parameter is a raster, "
      "from which the height could be get. "
                     "Parameter fith, six and seven is used "
                     "to pass the sort of way, the dependence to "
      "a cycle route and the surface. "
                     "With the last parameters the dependence "
      "on the sort of way, "
                     "the gradient and the surface can be set. "
                     "Put in Zero for default value.";

      }
    };

template<typename Raster> int lfdistanceparamFun
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  Point* a = static_cast<Point*>(args[0].addr);
  Point* b = static_cast<Point*>(args[1].addr);
  Raster* raster = static_cast<Raster*>(args[2].addr);
  CcString* roadType = static_cast<CcString*>(args[3].addr);
  CcString* routePart = static_cast<CcString*>(args[4].addr);
  CcString* surface = static_cast<CcString*>(args[5].addr);
  CcReal* w1 = static_cast<CcReal*>(args[6].addr);
  CcReal* w2 = static_cast<CcReal*>(args[7].addr);
  CcReal* w3 = static_cast<CcReal*>(args[8].addr);
  CcReal* w4 = static_cast<CcReal*>(args[9].addr);
  CcReal* s1 = static_cast<CcReal*>(args[10].addr);
  CcReal* s2 = static_cast<CcReal*>(args[11].addr);
  CcReal* s3 = static_cast<CcReal*>(args[12].addr);
  CcReal* s4 = static_cast<CcReal*>(args[13].addr);
  CcReal* s5 = static_cast<CcReal*>(args[14].addr);
  CcReal* s6 = static_cast<CcReal*>(args[15].addr);
  CcReal* s7 = static_cast<CcReal*>(args[16].addr);
  CcReal* o1 = static_cast<CcReal*>(args[17].addr);
  CcReal* o2 = static_cast<CcReal*>(args[18].addr);
  CcReal* o3 = static_cast<CcReal*>(args[19].addr);
  CcReal* o4 = static_cast<CcReal*>(args[20].addr);
  CcReal* res = static_cast<CcReal*>(result.addr);

    bool checkCoord;

    double wayFactor;
    if ((routePart->toText()=="yes") && (w1->GetRealval()!=0.0))
        wayFactor = w1->GetRealval();
    else if ((routePart->toText()=="yes") && (w1->GetRealval()==0.0))
        wayFactor = 0.5;
    else if ((roadType->toText()=="cycleway") && (w2->GetRealval()!=0.0))
        wayFactor = w2->GetRealval();
    else if ((roadType->toText()=="cycleway") && (w2->GetRealval()==0.0))
        wayFactor = 0.5;
    else if ((roadType->toText()=="track" ||
        roadType->toText() == "path") && (w3->GetRealval()!=0.0))
        wayFactor = w3->GetRealval();
    else if ((roadType->toText()=="trunk" ||
            roadType->toText()=="primary" ||
            roadType->toText()=="secondary" ||
            roadType->toText()=="tertiary" ||
            roadType->toText()=="unclassified" ||
            roadType->toText()=="residential" ||
            roadType->toText()=="trunk")
            && (w4->GetRealval()!=0.0))
            wayFactor = w4->GetRealval();
    else if ((roadType->toText()=="trunk" ||
            roadType->toText()=="primary" ||
            roadType->toText()=="secondary" ||
            roadType->toText()=="tertiary" ||
            roadType->toText()=="unclassified" ||
            roadType->toText()=="residential" ||
            roadType->toText()=="trunk") && (w4->GetRealval()==0.0))
            wayFactor = 2.0;
    else wayFactor = 1.0;

    double surfaceFactor;
    if ((surface->toText()=="paved" || surface->toText()=="asphalt" ||
         surface->toText()=="concrete") && (o1->GetRealval() != 0.0))
         surfaceFactor = o1->GetRealval();
    else if ((surface->toText()=="cobblestone" ||
            surface->toText()=="cobblestone:flattened" ||
            surface->toText()=="sett" ||
            surface->toText()=="concrete:lanes" ||
            surface->toText()=="concrete:plates" ||
            surface->toText()=="paving_stones" ||
            surface->toText()=="metal" ||
            surface->toText()=="wood" ||
            surface->toText()=="paving_stones:30" ||
            surface->toText()=="paving_stones:20" )
            && (o2->GetRealval()!=0.0))
            surfaceFactor = o2->GetRealval();
    else if ((surface->toText()=="unpaved" ||
            surface->toText()=="compacted" ||
            surface->toText()=="dirt" ||
            surface->toText()=="earth" ||
            surface->toText()=="grass" ||
            surface->toText() == "grass_paver" ||
            surface->toText()=="ground" ||
            surface->toText()=="mud" ||
            surface->toText()=="ice" ||
            surface->toText()=="salt" ||
            surface->toText()=="sand" ||
            surface->toText()=="snow"||
            surface->toText()=="woodchips" ) && (o3->GetRealval()!=0.0))
            surfaceFactor = o3->GetRealval();
        else if ((surface->toText()=="gravel" ||
            surface->toText()=="pebblestone" )
            && (o4->GetRealval()!=0.0))
              surfaceFactor = o4->GetRealval();
    else if ((surface->toText()=="gravel" ||
            surface->toText()=="pebblestone" )
            && (o4->GetRealval()==0.0))
              surfaceFactor = 3.0;
    else surfaceFactor = 1.0;


  if(!a->IsDefined() || !b->IsDefined() || a->IsEmpty() || b->IsEmpty() )
  {
    res->SetDefined(false);
  }
  else
  {
        double airDistance = a->DistanceOrthodrome(*b,Geoid(true),checkCoord);
        double heightA=getHeightAtPosition(a->GetX(),a->GetY(),*raster);
        double heightB=getHeightAtPosition(b->GetX(),b->GetY(),*raster);
        double dheight=heightB-heightA;
        double drivingDistance=sqrt(pow(dheight,2)+pow(airDistance,2));
        double climb = dheight/airDistance;
        double dist=0;


            if ((climb < (-1.0))||(climb > 1.0)) res->SetDefined(false);
            else if ((climb >= -1.00)&&(climb <= -0.10))
                dist = drivingDistance *
                    (s1->GetRealval()!=0 ? s1->GetRealval() : 0.30);
      else if ((climb >  -0.10)&&(climb <  -0.01))
                dist = drivingDistance *
                    (s2->GetRealval()!=0 ? s2->GetRealval() : 0.60);
      else if ((climb >= -0.01)&&(climb <=  0.01))
                dist = drivingDistance *
                    (s3->GetRealval()!=0 ? s3->GetRealval() : 1.00);
      else if ((climb >   0.01)&&(climb <   0.02))
                dist = drivingDistance *
                    (s4->GetRealval()!=0 ? s4->GetRealval() : 1.25);
      else if ((climb >=  0.02)&&(climb <   0.05))
                dist = drivingDistance *
                    (s5->GetRealval()!=0 ? s5->GetRealval() : 1.75);
      else if ((climb >=  0.05)&&(climb <   0.10))
                dist = drivingDistance *
                    (s6->GetRealval()!=0 ? s6->GetRealval() : 2.75);
      else if ((climb >=  0.10)&&(climb <=  1.00))
                dist = drivingDistance *
                    (s7->GetRealval()!=0 ? s7->GetRealval() : 20.00);

            dist = dist * wayFactor * surfaceFactor;
            res->Set(true,dist);
        }
  return (0);
}


ValueMapping lfdistanceparamFuns[] =
{
    lfdistanceparamFun<raster2::sint>,
    lfdistanceparamFun<raster2::sreal>,
    0
};


int lfdistanceparamSelectFun(ListExpr args)
{
  NList type(args);

  if (type.third().isSymbol(raster2::sint::BasicType()))
  {
    return (0);
  }
  if(type.third().isSymbol(raster2::sreal::BasicType()))
  {
    return (1);
  }
  return (-1);
}

ListExpr lfdistanceparamTypeMap(ListExpr args)
{
  if(!nl->HasLength(args,21)){
    return (listutils::typeError("21 arguments expected"));
  }
    ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
    ListExpr arg5 = nl->Fifth(args);
    ListExpr arg6 = nl->Sixth(args);

  std::string err = "Point x Point x stype x 3 string x 15 real expected ";
  if(!Point::checkType(arg1))
  {
    return (listutils::typeError(err + " (arg 1 is not an point)"));
  }
  if(!Point::checkType(arg2))
  {
    return (listutils::typeError(err + " (arg 2 is not an point)"));
  }
  if(!raster2::util::isSType(arg3))
  {
    return (listutils::typeError(err + " (arg 3 is not an stype)"));
  }
  if(!CcString::checkType(arg4))
  {
            return (listutils::typeError(err
                + " (arg 4 arg is not an string)"));
  }
  if(!CcString::checkType(arg5))
  {
    return (listutils::typeError(err
                + " (arg 5 is not an string)"));
  }
  if(!CcString::checkType(arg6))
  {
            return (listutils::typeError(err + " (arg 6 is not an string)"));
  }

  NList type(args);

    if(!CcReal::checkType(type.elem(7).listExpr()))
        {
            return (listutils::typeError(err + "arg 7 is not an real "));
        }
    if(!CcReal::checkType(type.elem(8).listExpr()))
        {
            return (listutils::typeError(err + "arg 8 is not an real "));
        }
    if(!CcReal::checkType(type.elem(9).listExpr()))
        {
            return (listutils::typeError(err + "arg 9 is not an real "));
        }
    if(!CcReal::checkType(type.elem(10).listExpr()))
        {
            return (listutils::typeError(err + "arg 10 is not an real "));
        }
    if(!CcReal::checkType(type.elem(11).listExpr()))
        {
            return (listutils::typeError(err + "arg 11 is not an real "));
        }
    if(!CcReal::checkType(type.elem(12).listExpr()))
        {
            return (listutils::typeError(err + "arg 12 is not an real "));
        }
    if(!CcReal::checkType(type.elem(13).listExpr()))
        {
            return (listutils::typeError(err + "arg 13 is not an real "));
        }
    if(!CcReal::checkType(type.elem(14).listExpr()))
        {
            return (listutils::typeError(err + "arg 14 is not an real "));
        }
    if(!CcReal::checkType(type.elem(15).listExpr()))
        {
            return (listutils::typeError(err + "arg 15 is not an real "));
        }
    if(!CcReal::checkType(type.elem(16).listExpr()))
        {
            return (listutils::typeError(err + "arg 16 is not an real "));
        }
    if(!CcReal::checkType(type.elem(17).listExpr()))
        {
            return (listutils::typeError(err + "arg 17 is not an real "));
        }
    if(!CcReal::checkType(type.elem(18).listExpr()))
        {
            return (listutils::typeError(err + "arg 18 is not an real "));
        }
    if(!CcReal::checkType(type.elem(19).listExpr()))
        {
            return (listutils::typeError(err + "arg 19 is not an real "));
        }
    if(!CcReal::checkType(type.elem(20).listExpr()))
        {
            return (listutils::typeError(err + "arg 20 is not an real "));
        }
    if(!CcReal::checkType(type.elem(21).listExpr()))
        {
            return (listutils::typeError(err + "arg 21 is not an real "));
        }
    return (listutils::basicSymbol<CcReal>());
}

/*
1.5 Operator ~distanceWithGradient~

*/

struct distanceWithGradientInfo : OperatorInfo
    {
    distanceWithGradientInfo()
    {//Point,Point,CcReal, LReal>
        name      = "distanceWithGradient";
        signature = "distanceWithGradient ("
          + Point::BasicType()
          + " , "
          + Point::BasicType()
          + " , "
          + CcReal::BasicType()
          + " , "
          + LReal::BasicType()
          + "-> Real";
        appendSignature("distanceWithGradient ("
              + Point::BasicType()
              + " , "
              + Point::BasicType()
                + " , "
              + CcReal::BasicType()
              + " , "
                + LReal::BasicType()
                + "-> Real");

        syntax    =   "distanceWithGradient ( _ , _ , _ , _ )";
        meaning   =   "This function computes the distance "
      "between two points including their height. "
      "The First two parameters are "
      "the two points, between the distance "
      "should be calculated. "
      "The third parameter gives the weight for the gradient."
      "The fourth parameter is a lreal with linear functions "
      "from which the height could be get. "
      ;
      }
    };


int distanceWithGradientFun( Word* args, Word& result, int message,
                     Word& local, Supplier s ){

   result = qp->ResultStorage( s );
   CcReal* res = static_cast<CcReal*>(result.addr);
  // const Geoid* geoid =0;
     const Geoid geoid(Geoid::WGS1984);
   //Argumente auseinander nehmen und casten
     Point* pointSource = static_cast<Point*>(args[0].addr);
     Point* pointTarget = static_cast<Point*>(args[1].addr);
     CcReal* weight = static_cast<CcReal*>(args[2].addr);
     LReal* heightfunction = static_cast<LReal*>(args[3].addr);
     double distance=DistanceWithHeight(*pointSource,*pointTarget,*weight,
        *heightfunction,&geoid);
     res->Set(true,distance);
   return 0;
}

ListExpr distanceWithGradientTypeMap(ListExpr args)
{
  if(!nl->HasLength(args,4)){
    return (listutils::typeError("4 arguments expected"));
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  std::string err = "Point x Point x CcReal x LReal expected";
  if(!Point::checkType(arg1))
  {
    return (listutils::typeError(err
            + " (first arg is not a point)"));
  }
  if(!Point::checkType(arg2))
  {
    return (listutils::typeError(err
            + " (second arg is not a point)"));
  }
  if(!CcReal::checkType(arg3)){
    return (listutils::typeError(err
            + " (third arg is not a ccreal)"));
  }
  if(!LReal::checkType(arg4))
  {
    return (listutils::typeError(err
            + " (fourth arg is not a LReal)"));
  }
  //return listutils::typeError();
  return (listutils::basicSymbol<CcReal>());
}

/*
1.5 Operator ~lfResult~

*/

struct lfResultInfo : OperatorInfo
    {
    lfResultInfo()
    {
      name      = "lfResult";
      signature = "lfResult (Real,LReal)->Real";
      appendSignature("lfResult (Real,LReal)->Real");
      syntax    =   "lfResult ( _ , _ )";
      meaning   =   "A LReal is a datatype which consists of "
        "multiple linear functions for given intervals"
        "This function computes the result at a given position of a LReal. "
        "The first parameter is the value for x in a linear function "
        "like (y=m*x +n). "
        "The second parameter is the LReal which includes all linear functions"
        "with interval."
        "The result is the y of the linear function "
      ;
      }
    };


int lfResultFun( Word* args, Word& result, int message,
                     Word& local, Supplier s ){

   result = qp->ResultStorage( s );
   CcReal* res = static_cast<CcReal*>(result.addr);

   //Argumente auseinander nehmen und casten
     CcReal* xValue = static_cast<CcReal*>(args[0].addr);
     LReal* heightfunction = static_cast<LReal*>(args[1].addr);

     double x = xValue->GetRealval();
     double y;
     res->Set(false,0.0);
     for (int i = 0; i < heightfunction->GetNoComponents(); i++) {
         LUReal unit;
         heightfunction->Get(i,unit);

         double intervStart=unit.getLengthInterval().start.GetRealval();
         double intervEnd=unit.getLengthInterval().end.GetRealval();

         if ((intervStart <= x) && (x <= intervEnd)){
             //Steigung und Anfangshoehe der Funktion
             double m=unit.m;
             double n=unit.n;
             //Lineare Funktion ausrechnen
             y= (m * x) + n;
             res->Set(true,y);
             break;
         }
     }
   return 0;
}

ListExpr lfResultTypeMap(ListExpr args)
{
  if(!nl->HasLength(args,2)){
    return (listutils::typeError("2 arguments expected"));
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  std::string err = "CcReal x LReal expected";

  if(!CcReal::checkType(arg1)){
    return (listutils::typeError(err
            + " (first arg is not a ccreal)"));
  }
  if(!LReal::checkType(arg2))
  {
    return (listutils::typeError(err
            + " (second arg is not a LReal)"));
  }
  //return listutils::typeError();
  return (listutils::basicSymbol<CcReal>());
}


Operator distanceWithGradient
(
    distanceWithGradientInfo(),
    distanceWithGradientFun,
    distanceWithGradientTypeMap
);

Operator lfResult
(
    lfResultInfo(),
    lfResultFun,
    lfResultTypeMap
);

Operator lcompose
(
    lcomposeInfo(),
    lcomposeFuns,
    lcomposeSelectFun,
    lcomposeTypeMap
);

Operator heightatposition
(
    heightatpositionInfo(),
    heightatpositionFuns,
    heightatpositionSelectFun,
    heightatpositionTypeMap
);

Operator lfdistance
(
    lfdistanceInfo(),
    lfdistanceFuns,
    lfdistanceSelectFun,
    lfdistanceTypeMap
);

Operator lfdistanceparam
(
    lfdistanceparamInfo(),
    lfdistanceparamFuns,
    lfdistanceparamSelectFun,
    lfdistanceparamTypeMap
);

/*
5 Creating the Algebra

*/
class LineFunctionAlgebra : public Algebra
{
 public:
  LineFunctionAlgebra() : Algebra()
  {
    AddTypeConstructor( &lunitbool );
    AddTypeConstructor( &lunitint );
    AddTypeConstructor( &lunitstring );
    AddTypeConstructor( &lunitreal );

    AddTypeConstructor( &lengthbool );
    AddTypeConstructor( &lengthint );
    AddTypeConstructor( &lengthstring );
    AddTypeConstructor( &lengthreal );

    lunitbool.AssociateKind( Kind::DATA() );
    lunitint.AssociateKind( Kind::DATA() );
    lunitstring.AssociateKind( Kind::DATA() );
    lunitreal.AssociateKind( Kind::DATA() );

    lengthbool.AssociateKind( Kind::DATA() );
    lengthint.AssociateKind( Kind::DATA() );
    lengthstring.AssociateKind( Kind::DATA() );
    lengthreal.AssociateKind( Kind::DATA() );

    AddOperator( &lcompose );
    AddOperator( &heightatposition );
    AddOperator( &lfdistance );
    AddOperator( &lfdistanceparam);
    AddOperator( &distanceWithGradient);
    AddOperator( &lfResult);
  }
  ~LineFunctionAlgebra() {};
};


/*
6 Initialization

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
InitializeLineFunctionAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new LineFunctionAlgebra());
}
