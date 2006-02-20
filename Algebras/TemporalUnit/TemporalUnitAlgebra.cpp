/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Units-Operators

February 2006  Thomas Fischer

-----
                            Signatur
  ~Trajectory~           upoint -> line
  ~Makemvalue~           ((stream (tuple ((x1 t1)...(xn tn))) xi(ua)))  ->  ma
  ~Size~                 periods  ->  real
  ~Deftime~              unit(a) -> periods
  ~Atinstant~            unit(a) x instant -> ix
  ~Atperiods~            unit(a) x periods -> ua
  ~Initial~              unit(a) -> ia
  ~final~                unit(a)  -> ia
  ~Present~              unit(a) x instant  -> bool
                         unit(a) x periods  -> bool
  ~point2d~              periods -> point
  ~queryrect2d~          instant -> rect
  ~speed~                mpoint -> mreal
                         upoint -> ureal
  ~passes~               upoint x point -> bool
  ~at~                   upoint x point -> upoint

-----





[TOC]

1 Overview

This file contains the implementation of the type constructors ~instant~,
~range~, ~intime~, ~const~, and ~mapping~. The memory data structures
used for these type constructors are implemented in the TemporalAlgebra.h
file.

2 Defines, includes, and constants

*/
#include <cmath>
#include "NestedList.h"

#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
using namespace datetime;

/*
2.1 Definition of some constants

*/
const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = numeric_limits<double>::min();


/*
3 Implementation of the Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

3.1 Operator ~speed~

*/
void MPoint::MSpeed( MReal& result ) const
{
  const UPoint *uPoint;

  UReal uReal;
  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  result.Clear();
  result.StartBulkLoad();

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, uPoint );

     x0 = uPoint->p0.GetX();
     y0 = uPoint->p0.GetY();

     x1 = uPoint->p1.GetX();
     y1 = uPoint->p1.GetY();

     uReal.timeInterval = uPoint->timeInterval;
     inf = uReal.timeInterval.start,
     sup = uReal.timeInterval.end;

     t0 = inf.GetAllMilliSeconds();
     t1 = sup.GetAllMilliSeconds();

     duration = (t1 - t0)/1000;    // value in second

     uReal.a = 0;                 // speed is constant in the interval
     uReal.b = 0;
     uReal.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
     uReal.r = false;


    result.Add( uReal );
  }
  result.EndBulkLoad( false );
}

void UPoint::USpeed( UReal& result ) const
{

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

     x0 = p0.GetX();
     y0 = p0.GetY();

     x1 = p1.GetX();
     y1 = p1.GetY();

     result.timeInterval = timeInterval;

  if (result.IsDefined() )
   {
     inf = result.timeInterval.start,
     sup = result.timeInterval.end;

     t0 = inf.GetAllMilliSeconds();
     t1 = sup.GetAllMilliSeconds();

     duration = (t1 - t0)/1000;   // value in second

     result.a = 0;                // speed is constant in the interval
     result.b = 0;
     result.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
     result.r = false;
   }
}

/*
3.6 Operator ~Trajectory~

*/
void UPoint::UTrajectory( UPoint& unit,CLine& line ) const
{
  line.Clear();
  line.StartBulkLoad();


  CHalfSegment chs( false );

    if( !AlmostEqual( unit.p0, unit.p1 ) )
    {

      chs.Set( true, unit.p0, unit.p1 );

      line += chs;
      chs.SetLDP( false );
      line += chs;
    }

  line.EndBulkLoad();
}


/*
9 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

9.1 Type mapping function ~InstantTypeMapSpeed~

Is used for the ~speed~ operator.

Type mapping for ~speed~ is

----  (mpoint)  ->  (mreal)

----

*/
ListExpr
InstantTypeMapSpeed( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mpoint" )  )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "upoint" )  )
      return nl->SymbolAtom( "ureal" );
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
9.2 Type mapping function ~InstantTypeMapQueryrect2d~

Is used for the ~queryrect2d~ operator.

Type mapping for ~queryrect2d~ is

----  (instant)  ->  (rect)

----

*/
ListExpr
InstantTypeMapQueryrect2d( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "instant" )  )
      return nl->SymbolAtom( "rect" );
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
9.3 Type mapping function ~PeriodsTypeMapPoint2d~

Is used for the ~point2d~ operator.

Type mapping for ~point2d~ is

----  (periods)  ->  (point)

----


*/
ListExpr
PeriodsTypeMapPoint2d( ListExpr args )
{

  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "periods" )  )
      return nl->SymbolAtom( "point" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
9.4 Type mapping function ~PeriodsTypeMapSize~

Is used for the ~size~ operator.

Type mapping for ~size~ is

----  (periods)  ->  (real)

----


*/
ListExpr
PeriodsTypeMapSize( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "periods" )  )
      return nl->SymbolAtom( "real" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
9.5 Type Mapping Function ~MovingTypeMapMakemvalue~

It is used for the operator ~makemvalue~

Type mapping for ~makemvalue~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi(ubool)))  ->  (mbool)
              APPEND (i ti)
      ((stream (tuple ((x1 t1)...(xn tn))) xi(uint)))  ->   (mint)
              APPEND (i ti)
      ((stream (tuple ((x1 t1)...(xn tn))) xi(ureal)))  ->  (mreal)
              APPEND (i ti)
      ((stream (tuple ((x1 t1)...(xn tn))) xi(upoint)))  -> (mpoint)
              APPEND (i ti)

----

*/
ListExpr MovingTypeMapMakemvalue( ListExpr args )

{
  ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr, listfull, attrtype;

  int j;
  string argstr, argstr2, attrname, inputtype, inputname, fulllist;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator makemvalue expects a list of length two.");

  first = nl->First(args);
  nl->WriteToString(argstr, first);

  CHECK_COND(nl->ListLength(first) == 2  &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (IsTupleDescription(nl->Second(nl->Second(first)))),
  "Operator makemvalue expects as first argument a list with structure "
  "(stream (tuple ((a1 t1)...(an tn))))\n"
  "Operator makemvalue gets as first argument '" + argstr + "'." );

  second  = nl->Second(args);
  nl->WriteToString(argstr, second);
  CHECK_COND(argstr != "typeerror",
  "Operator makemvalue expects a name of a attribute and not a type.");

  nl->WriteToString(inputname, second);    // name of the given attribute

  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  firstr = nl->First(rest);
  rest = nl->Rest(rest);
  first2 = nl->First(firstr);
  second2 = nl->Second(firstr);
  nl->WriteToString(attrname, first2);
  nl->WriteToString(argstr2, second2);
  // comparison with the attributes in the relation
  if (attrname == inputname)
     inputtype = argstr2;
  //  now I save from the detected attribute the type

while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     firstr = nl->First(rest);
     rest = nl->Rest(rest);
     first2 = nl->First(firstr);
     second2 = nl->Second(firstr);
     nl->WriteToString(attrname, first2);
     nl->WriteToString(argstr2, second2);
     // comparison with the attributes in the relation
     if (attrname == inputname)
         inputtype = argstr2;
    //  now I save from the detected attribute the type
  }
  rest = second;
  listfull = listn;
  nl->WriteToString(fulllist, listfull);


CHECK_COND(!(inputtype == "") ,
    "Operator makemvalue: Attribute name '"+ inputname+
    "' is not known.\n"+
    "Known Attribute(s): " + fulllist);


    CHECK_COND( (inputtype == "ubool"
          || inputtype == "uint"
          || inputtype == "ureal"
          || inputtype == "upoint" ),
          "Attribute type is not of type ubool, uint, ureal or upoint.");

  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);

  assert( j !=0 );

  if( inputtype == "upoint" )
      attrtype = nl->SymbolAtom( "mpoint" ) ;
  if( inputtype == "ubool" )
      attrtype = nl->SymbolAtom( "mbool" );
  if( inputtype == "ureal" )
      attrtype = nl->SymbolAtom( "mreal" );
  if( inputtype == "uint" )
      attrtype = nl->SymbolAtom( "mint" );

  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
         nl->TwoElemList(nl->IntAtom(j),
         nl->StringAtom(nl->SymbolValue(attrtype))), attrtype);

 /*

 Append and the number of the attribute in the relation is very important,
 because in the other case we can't work with it in the value function.

 */

 }
/*
9.6 Type mapping function ~UnitTrajectoryTypeMap~

It is for the operator ~trajectory~.

*/
ListExpr
UnitTrajectoryTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "upoint" ) )
      return nl->SymbolAtom( "line" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
9.7 Type mapping function ~UnitTypeMapRange~

It is for the operator ~deftime~.

*/
ListExpr
UnitTypeMapPeriods( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

   if(  nl->IsEqual( arg1, "ubool" ) ||
        nl->IsEqual( arg1, "uint" ) ||
        nl->IsEqual( arg1, "ureal" ) ||
        nl->IsEqual( arg1, "upoint" ) )
   return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
9.8 Type mapping function ~UnitInstantTypeMapIntime~

It is for the operator ~atinstant~.

*/
ListExpr
UnitInstantTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) )
    {
       if( nl->IsEqual( arg1, "ubool" ) )
        return nl->SymbolAtom( "ibool" );

      if( nl->IsEqual( arg1, "uint" ) )
        return nl->SymbolAtom( "iint" );

      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->SymbolAtom( "ireal" );

      if( nl->IsEqual( arg1, "upoint" ) )
        return nl->SymbolAtom( "ipoint" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
9.9 Type mapping function ~UnitPeriodsTypeMap~

It is for the operator ~atperiods~.

*/
ListExpr
UnitPeriodsTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

   if( nl->IsEqual( arg2, "periods" ) )
    {

      if( nl->IsEqual( arg1, "ubool" ) )
        return nl->SymbolAtom( "ubool" );

      if( nl->IsEqual( arg1, "uint" ) )
        return nl->SymbolAtom( "uint" );

      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->SymbolAtom( "ureal" );

      if( nl->IsEqual( arg1, "upoint" ) )
        return nl->SymbolAtom( "upoint" );

    }
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
10.0 Type mapping function ~UnitTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
UnitTypeMapIntime( ListExpr args )
{
 if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

       if( nl->IsEqual( arg1, "ubool" ) )
      return nl->SymbolAtom( "ibool" );

    if( nl->IsEqual( arg1, "uint" ) )
      return nl->SymbolAtom( "iint" );

    if( nl->IsEqual( arg1, "ureal" ) )
      return nl->SymbolAtom( "ireal" );

    if( nl->IsEqual( arg1, "upoint" ) )
      return nl->SymbolAtom( "ipoint" );

  }
  return nl->SymbolAtom( "typeerror" );
}
/*
10.1 Type mapping function ~UnitInstantPeriodsTypeMapBool~

It is for the operator ~present~.

*/
ListExpr
UnitInstantPeriodsTypeMapBool( ListExpr args )
{
   if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) ||
        nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "ubool" )||
          nl->IsEqual( arg1, "uint" ) ||
          nl->IsEqual( arg1, "ureal" ) ||
          nl->IsEqual( arg1, "upoint" ) )

        return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
10.2 Type mapping function ~UnitBaseTypeMapBool~

It is for the operator ~passes~.

*/
ListExpr
UnitBaseTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( ((nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "point" ))) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
10.3 Type mapping function ~UnitBaseTypeMapUnit~

It is for the operator ~at~.

*/
ListExpr
UnitBaseTypeMapUnit( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "upoint" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.


16.1.1 Selection function ~SpeedSelect~

*/

int
SpeedSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "mpoint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 1;

  return -1; // This point should never be reached
}
/*
16.1.2 Selection function ~MakemvalueSelect~

*/

int
MakemvalueSelect( ListExpr args )
{

 ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr;


  string argstr, argstr2, attrname, inputtype, inputname;


  first = nl->First(args);
  second  = nl->Second(args);
  nl->WriteToString(argstr, first);


  nl->WriteToString(inputname, second);


  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  firstr = nl->First(rest);
  rest = nl->Rest(rest);
  first2 = nl->First(firstr);
  second2 = nl->Second(firstr);
  nl->WriteToString(attrname, first2);
  nl->WriteToString(argstr2, second2);
  if (attrname == inputname)
     inputtype = argstr2;

while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     firstr = nl->First(rest);
     rest = nl->Rest(rest);
     first2 = nl->First(firstr);
     second2 = nl->Second(firstr);
     nl->WriteToString(attrname, first2);
     nl->WriteToString(argstr2, second2);
     if (attrname == inputname)
         inputtype = argstr2;
  }

  if( inputtype == "ubool" )
    return 0;

  if( inputtype == "uint" )
    return 1;

  if( inputtype == "ureal" )
    return 2;

  if( inputtype == "upoint" )
    return 3;


  return -1; // This point should never be reached
}
/*
16.1.3 Selection function ~UnitSimpleSelect~

Is used for the ~deftime~,~atinstant~,~atperiods~, ~initial~, ~final~  operations.

*/
int
UnitSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );


  if( nl->SymbolValue( arg1 ) == "ubool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "uint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ureal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 3;

  return -1; // This point should never be reached
}
/*
16.1.4 Selection function ~UnitInstantPeriodsSelect~

Is used for the ~present~ operations.

*/
int
UnitInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );


  if( nl->SymbolValue( arg1 ) == "ubool" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "uint" &&
     nl->SymbolValue( arg2 ) == "instant" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ureal" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "upoint" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "ubool" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "uint" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "ureal" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "upoint" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 7;

  return -1; // This point should never be reached
}
/*
16.1.5 Selection function ~UnitBaseSelect~

Is used for the ~passes~ operations.

*/
int
UnitBaseSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "ubool" &&
      nl->SymbolValue( arg2 ) == "bool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "uint" &&
      nl->SymbolValue( arg2 ) == "int" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ureal" &&
      nl->SymbolValue( arg2 ) == "real" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "upoint" &&
      nl->SymbolValue( arg2 ) == "point" )
    return 3;

  return -1; // This point should never be reached
}


/*
16.2 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

16.2.1 Value mapping functions of operator ~speed~

*/
int MPointSpeed(Word* args, Word& result, int message, Word& local, Supplier s)
{

  result = qp->ResultStorage( s );
 ((MPoint*)args[0].addr)->MSpeed(  *((MReal*)result.addr) );

  return 0;
}
int UnitPointSpeed(Word* args,Word& result,int message,Word& local,Supplier s)
{

  result = qp->ResultStorage( s );
 ((UPoint*)args[0].addr)->USpeed(  *((UReal*)result.addr) );

  return 0;
}
/*
16.2.2 Value mapping functions of operator ~queryrect2d~

*/
int Queryrect2d(Word* args, Word& result, int message, Word& local, Supplier s)
{
  const unsigned dim = 2;
  double min[dim], max[dim];
  double x1,y1,x2,y2;
  double timevalue;

  result = qp->ResultStorage( s );
  Instant* Inv = (Instant*)args[0].addr;

  x1 = 0;
  x2 = 0;
  y1 = 0;
  y2 = 0;

  if (Inv->IsDefined())
   {
    timevalue = Inv->GetAllMilliSeconds();

    x1 = 0;
    x2 = timevalue;
    y1 = timevalue;
    y2 = MAXDOUBLE;

    min[0] = x1;
    min[1] = x2;               // definition of the rectangle
    max[0] = y1;
    max[1] = y2;

    Rectangle<dim>* rect = new Rectangle<dim>(true, min, max);
    ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
    delete rect;
   }
  else
   {

    min[0] = x1;
    min[1] = x2;                // definition of the rectangle
    max[0] = y1;
    max[1] = y2;

    Rectangle<dim>* rect = new Rectangle<dim>(false, min, max);
    ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
    delete rect;

    }

  return 0;
}
/*
16.2.3 Value mapping functions of operator ~point2d~

*/
int Point2d( Word* args, Word& result, int message, Word& local, Supplier s )
{
  double  X, Y;
  Instant sup, inf;

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

if( !range->IsEmpty()  )
  {
    const Interval<Instant> *intv1, *intv2;
    X = 0;
    Y = 0;

    range->Get( 0, intv1 );
    range->Get( range->GetNoComponents()-1, intv2 );

    Interval<Instant> timeInterval(intv1->start,intv2->end,intv1->lc,intv2->rc);

    sup = timeInterval.end;
    inf = timeInterval.start;

    Y = sup.GetAllMilliSeconds(); // derives the maximum of all intervals
    X = inf.GetAllMilliSeconds();// derives the minimum of all intervals

  }

  ((Point*)result.addr)->Set(X,Y );

  return 0;
}
/*
16.2.4 Value mapping functions of operator ~size~

*/
int Size( Word* args, Word& result, int message, Word& local, Supplier s )
{
  double res, duration, intervalue_sup, intervalue_inf;
  Instant sup, inf;

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

if( !range->IsEmpty()  )
  {
    const Interval<Instant> *intv1, *intv2;
    duration = 0;

  for( int i = 0; i < range->GetNoComponents(); i++ )
  {
    range->Get( i, intv1 );
    range->Get( i, intv2 );

    Interval<Instant> timeInterval(intv1->start,intv2->end,intv1->lc,intv2->rc);

    sup = timeInterval.end;
    inf = timeInterval.start;


    intervalue_sup = sup.GetAllMilliSeconds();
    intervalue_inf = inf.GetAllMilliSeconds();

    duration += (intervalue_sup - intervalue_inf)/1000;    // value in second


   }

    res = duration;
  }

  ((CcReal*)result.addr)->Set(true, res);

  return 0;
}
/*
16.2.5 Value mapping functions of operator ~makemvalue~

*/

template <class Mapping, class Unit>
int MappingMakemvalue(Word* args,Word& result,int message,
                      Word& local,Supplier s)
{
  Mapping* m;
  Unit* unit;

  Word currentTupleWord;

  assert(args[2].addr != 0);
  assert(args[3].addr != 0);

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;


  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  result = qp->ResultStorage(s);

  m = (Mapping*) result.addr;
  m->Clear();
  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) )
  {

    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    Attribute* currentAttr = (Attribute*)currentTuple->
                              GetAttribute(attributeIndex);
    if(currentAttr->IsDefined())
    {

     unit = (Unit*) currentAttr;

     m->Add( *unit );

     currentTuple->DeleteIfAllowed();            //consume the stream objects
     }
    qp->Request(args[0].addr, currentTupleWord);
  }
   m->EndBulkLoad( false );


  qp->Close(args[0].addr);

  return 0;

}
/*
16.2.6 Value mapping functions of operator ~trajectory~

*/
int UnitPointTrajectory(Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  CLine *line = ((CLine*)result.addr);
  UPoint *upoint = ((UPoint*)args[0].addr);
  upoint->UTrajectory( *upoint, *line );


  return 0;
}
/*
16.2.7 Value mapping functions of operator ~deftime~

*/
template <class Mapping>
int MappingUnitDefTime( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* r = ((Periods*)result.addr);
  Mapping* m = ((Mapping*)args[0].addr);

  assert( r->IsOrdered() );

  r->Clear();
  r->StartBulkLoad();
  r->Add( m->timeInterval );
  r->EndBulkLoad( false );

  return 0;
}
/*
16.2.8 Value mapping functions of operator ~atinstant~

*/
template <class Mapping, class Alpha>
int MappingUnitAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;
  Mapping* posUnit = ((Mapping*)args[0].addr);
  Instant* t = ((Instant*)args[1].addr);
  Instant t1 = *t;

  assert( t->IsDefined() );

  int pos;

  if( posUnit->timeInterval.Contains(t1) )
      pos = 1;
  else                               //not contained
      pos = -1;


  if( pos == -1 )                    // not contained in the unit
    pResult->SetDefined( false );
  else
  {
    pResult->SetDefined( true );
    posUnit->TemporalFunction( *t, pResult->value );
  }

  return 0;
}
/*
16.2.9 Value mapping functions of operator ~atperiods~

*/
template <class Mapping>
int MappingUnitAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* periods = ((Periods*)args[1].addr);
  Mapping* m = ((Mapping*)args[0].addr);
  Mapping* res = ((Mapping*)result.addr);


  if( periods->IsEmpty() )
    return 0;

  Mapping rt;

  const Interval<Instant> *interval;

  int j = 0;
  periods->Get( j, interval );

  res->SetDefined(false);
  res->CopyFrom(&rt);


  while( 1 )
  {
    if( m->timeInterval.Before( *interval ) )
    {
         break;
    }
    else if( interval->Before( m->timeInterval ) )
    {
      if( ++j == periods->GetNoComponents() )
         break;
      periods->Get( j, interval );
    }
    else
    {
      Mapping r;
      m->AtInterval( *interval, r );
      res->SetDefined(true);
      res->CopyFrom(&r);

      if( interval->end == m->timeInterval.end )
      {
        if( interval->rc == m->timeInterval.rc )
        {
          if( ++j == periods->GetNoComponents() )
            break;
          periods->Get( j, interval );
        }
        else if( interval->rc == true )
        {
             break;
        }
        else
        {
          assert( m->timeInterval.rc == true );
          if( ++j == periods->GetNoComponents() )
            break;
          periods->Get( j, interval );
        }
      }
      else if( interval->end > m->timeInterval.end )
      {
          break;
      }
      else
      {
         assert( interval->end < m->timeInterval.end );
        if( ++j == periods->GetNoComponents() )
          break;
        periods->Get( j, interval );
      }
    }
  }
  return 0;
}
/*
16.2.10 Value mapping functions of operator ~initial~

*/
template <class Unit, class Alpha>
int MappingUnitInitial( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Unit* unit = ((Unit*)args[0].addr);
  Intime<Alpha>* res = ((Intime<Alpha>*)result.addr);


  if( !(unit->timeInterval.start.IsDefined()) )
     res->SetDefined( false );
  else
   {
    res->SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.start, res->value );
    res->instant.CopyFrom( &unit->timeInterval.start );
    }
  return 0;
}
/*
16.2.11 Value mapping functions of operator ~final~

*/
template <class Unit, class Alpha>
int MappingUnitFinal( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Unit* unit = ((Unit*)args[0].addr);
  Intime<Alpha>* res = ((Intime<Alpha>*)result.addr);

  if( !(unit->timeInterval.end.IsDefined()) )
     res->SetDefined( false );
  else
   {
    res->SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.end, res->value );
    res->instant.CopyFrom( &unit->timeInterval.end );
   }
  return 0;
}
/*
16.2.12 Value mapping functions of operator ~present~

*/
template <class Mapping>
int MappingUnitPresent_i( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);
  Instant t1 = *inst;

  assert( inst->IsDefined() );

  bool pos;

  if( m->timeInterval.Contains(t1) )
      pos = true;
  else                               //not contained
      pos = false;

  if( !inst->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( pos )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}
template <class Mapping>
int MappingUnitPresent_p( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  assert( periods->IsOrdered() );

  Periods defTime( 0 );

  periods->Clear();
  periods->StartBulkLoad();
  periods->Add( m->timeInterval );
  periods->EndBulkLoad( false );
  periods->Merge( defTime );

  if( periods->IsEmpty() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( periods->Inside( defTime ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
  }

/*
16.2.13 Value mapping functions of operator ~passes~

*/
 template <class Mapping, class Alpha>
int MappingUnitPasses( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);

  if( !val->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
   else if( m->Passes( *val ) )
      ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
16.2.14 Value mapping functions of operator ~at~

*/
template <class Unit, class Alpha>
int MappingUnitAt( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit* unit = ((Unit*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);

  Unit* pResult = ((Unit*)result.addr);


  if (unit->At( *val, *pResult ))
   {
     pResult->SetDefined(true);
     pResult->timeInterval.start.SetDefined(true);
     pResult->timeInterval.end.SetDefined(true);
    }
  else
   {
     pResult->SetDefined(false);
     pResult->timeInterval.start.SetDefined(false);
     pResult->timeInterval.end.SetDefined(true);
   }


  return 0;
}

/*
17 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

17.1  ValueMapping arrays

*/

ValueMapping temporalspeedmap[] = { MPointSpeed,
                                    UnitPointSpeed };

ValueMapping temporalmakemvaluemap[] = { MappingMakemvalue<MBool, UBool>,
                                         MappingMakemvalue<MBool, UBool>,
                                         MappingMakemvalue<MReal, UReal>,
                                         MappingMakemvalue<MPoint, UPoint> };

ValueMapping temporalunitdeftimemap[] = { MappingUnitDefTime<UBool>,
                                          MappingUnitDefTime<UInt>,
                                          MappingUnitDefTime<UReal>,
                                          MappingUnitDefTime<UPoint> };

ValueMapping temporalunitatinstantmap[] ={MappingUnitAtInstant<UBool, CcBool>,
                                          MappingUnitAtInstant<UInt, CcInt>,
                                          MappingUnitAtInstant<UReal, CcReal>,
                                          MappingUnitAtInstant<UPoint, Point>,};

ValueMapping temporalunitatperiodsmap[] = { MappingUnitAtPeriods<UBool>,
                                            MappingUnitAtPeriods<UInt>,
                                            MappingUnitAtPeriods<UReal>,
                                            MappingUnitAtPeriods<UPoint> };

ValueMapping temporalunitinitialmap[] = { MappingUnitInitial<UBool, CcBool>,
                                          MappingUnitInitial<UInt, CcInt>,
                                          MappingUnitInitial<UReal, CcReal>,
                                          MappingUnitInitial<UPoint, Point> };

ValueMapping temporalunitfinalmap[] = {  MappingUnitFinal<UBool, CcBool>,
                                         MappingUnitFinal<UInt, CcInt>,
                                         MappingUnitFinal<UReal, CcReal>,
                                         MappingUnitFinal<UPoint, Point> };

ValueMapping temporalunitpresentmap[] = { MappingUnitPresent_i<UBool>,
                                          MappingUnitPresent_i<UInt>,
                                          MappingUnitPresent_i<UReal>,
                                          MappingUnitPresent_i<UPoint>,
                                          MappingUnitPresent_p<UBool>,
                                          MappingUnitPresent_p<UInt>,
                                          MappingUnitPresent_p<UReal>,
                                          MappingUnitPresent_p<UPoint> };

ValueMapping temporalunitpassesmap[] = { MappingUnitPasses<UBool, CcBool>,
                                         MappingUnitPasses<UInt, CcInt>,
                                         MappingUnitPasses<UReal, CcReal>,
                                         MappingUnitPasses<UPoint, Point> };

ValueMapping temporalunitatmap[] = {  MappingUnitAt< UBool, CcBool>,
                                      MappingUnitAt< UInt, CcInt>,
                                      MappingUnitAt< UReal, CcReal>,
                                      MappingUnitAt< UPoint, Point> };
/*
17.2 Specification strings

17.2.1 Specification string of operator ~speed~

*/
const string
TemporalSpecSpeed  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                     "\"Example\" ) "
                     "( <text>(mpoint/upoint) -> mreal/ureal</text->"
                     "<text>speed( _ )</text--->"
                     "<text>return the speed of a spatial object.</text--->"
                     "<text>query speed(mp1)</text--->"
                     ") )";
/*
17.2.2 Specification string of operator ~queryrect2d~

*/
const string
TemporalSpecQueryrect2d  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
"\"Example\" ) "
"( <text>(instant) -> rect</text--->"
"<text>queryrect2d( _ )</text--->"
"<text>return the rect of an instant object for a time interval.</text--->"
"<text>query queryrect2d(instant)</text--->"
") )";
/*
17.2.3 Specification string of operator ~point2d~

*/
const string
TemporalSpecPoint2d  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                       "\"Example\" ) "
                       "( <text>(periods) -> point</text--->"
                       "<text>point2d( _ )</text--->"
                       "<text>return the point of a given interval.</text--->"
                       "<text>query point2d(periods)</text--->"
                       ") )";
/*
17.2.4 Specification string of operator ~size~

*/
const string
TemporalSpecSize  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                    "\"Example\" ) "
                    "( <text>(periods) -> real</text--->"
                    "<text>size( _ )</text--->"
                    "<text>return the duration of a moving object.</text--->"
                    "<text>query size(periods)</text--->"
                    ") )";
/*
17.2.5 Specification string of operator ~makemvalue~

*/
const string
TemporalSpecMakemvalue  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>((stream (tuple ((x1 t1)...(xn tn)))"
                          " (unit(xi))))-> moving(x)</text--->"
                          "<text> makemvalue[ _ ]</text--->"
                          "<text>get the moving value of the stream "
                          "of units.</text--->"
                          "<text>makemvalue[ u1 ]</text--->"
                          ") )";
/*
17.2.6 Specification string of operator ~trajectory~

*/
const string
TemporalSpecTrajectory  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" ) "
                          "( <text>upoint -> line</text--->"
                          "<text> trajectory( _ )</text--->"
                          "<text>get the trajectory of the corresponding"
                          "unit point object.</text--->"
                          "<text>trajectory( up1 )</text--->"
                          ") )";
/*
17.2.7 Specification string of operator ~deftime~

*/
const string
TemporalSpecDefTime  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                       "\"Example\" ) "
                       "( <text>unit(x) -> periods</text--->"
                       "<text> deftime( _ )</text--->"
                       "<text>get the defined time of the corresponding"
                       " unit data objects.</text--->"
                       "<text>deftime( up1 )</text--->"
                       ") )";
/*
17.2.8 Specification string of operator ~atinstant~

*/
const string
TemporalSpecAtInstant  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(unit(x) instant) -> intime(x)</text--->"
                         "<text>_ atinstant _ </text--->"
                         "<text>get the Intime value corresponding to the "
                         "instant.</text--->"
                         "<text>upoint1 atinstant instant1</text--->"
                         ") )";

/*
17.2.9 Specification string of operator ~atperiods~

*/
const string
TemporalSpecAtPeriods  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(unit(x) periods) -> unit(x)</text--->"
                         "<text>_ atperiods _ </text--->"
                         "<text>restrict the movement to the given"
                         " periods.</text--->"
                         "<text>upoint1 atperiods periods1</text--->"
                         ") )";

/*
17.2.10 Specification string of operator ~Initial~

*/
const string
TemporalSpecInitial  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                       "\"Example\" ) "
                       "( <text>unit(x) -> intime(x)</text--->"
                       "<text> initial( _ )</text--->"
                       "<text>get the intime value corresponding"
                       " to the initial instant.</text--->"
                       "<text>initial( upoint1 )</text--->"
                       ") )";

/*
17.2.11 Specification string of operator ~final~

*/
const string
TemporalSpecFinal  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                     "\"Example\" ) "
                     "( <text>unit(x) -> intime(x)</text--->"
                     "<text> final( _ )</text--->"
                     "<text>get the intime value corresponding"
                     " to the final instant.</text--->"
                     "<text>final( upoint1 )</text--->"
                     ") )";
/*
17.2.12 Specification string of operator ~present~

*/
const string
TemporalSpecPresent  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                       "\"Example\" ) "
                       "( <text>(moving/unit(x) instant) -> bool,"
                       " (moving/unit(x) periods) -> bool</text--->"
                       "<text>_ present _ </text--->"
                       "<text>whether the object is present at the"
                       " given instant or period.</text--->"
                       "<text>mpoint1 present instant1</text--->"
                       ") )";

/*
17.2.13 Specification string of operator ~passes~

*/
const string
TemporalSpecPasses = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                     "\"Example\" ) "
                     "( <text>(unit(x) x) -> bool</text--->"
                     "<text>_ passes _ </text--->"
                     "<text>whether the object passes the given"
                     " value.</text--->"
                     "<text>upoint1 passes point1</text--->"
                     ") )";

/*
17.2.14 Specification string of operator ~at~

*/
const string
TemporalSpecAt = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                 "\"Example\" ) "
                 "( <text>(unit(x) x) -> unit(x)</text--->"
                 "<text> _ at _ </text--->"
                 "<text>restrict the movement at the times"
                 " where the equality occurs.</text--->"
                 "<text>upoint1 at point1</text--->"
                 ") )";
/*
17.3 Operators

*/

Operator temporalspeed( "speed",
                      TemporalSpecSpeed,
                      2,
                      temporalspeedmap,
                      SpeedSelect,
                      InstantTypeMapSpeed);

Operator temporalunitqueryrect2d( "queryrect2d",
                      TemporalSpecQueryrect2d,
                      Queryrect2d,
                      Operator::SimpleSelect,
                      InstantTypeMapQueryrect2d);

Operator temporalunitpoint2d( "point2d",
                      TemporalSpecPoint2d,
                      Point2d,
                      Operator::SimpleSelect,
                      PeriodsTypeMapPoint2d);

Operator temporalunitsize( "size",
                      TemporalSpecSize,
                      Size,
                      Operator::SimpleSelect,
                      PeriodsTypeMapSize );

Operator temporalunitmakemvalue( "makemvalue",
                        TemporalSpecMakemvalue,
                        4,
                        temporalmakemvaluemap,
                        MakemvalueSelect,
                        MovingTypeMapMakemvalue );

Operator temporalunittrajectory( "trajectory",
                          TemporalSpecTrajectory,
                          UnitPointTrajectory,
                          Operator::SimpleSelect,
                          UnitTrajectoryTypeMap );

Operator temporalunitdeftime( "deftime",
                          TemporalSpecDefTime,
                          4,
                          temporalunitdeftimemap,
                          UnitSimpleSelect,
                          UnitTypeMapPeriods );

Operator temporalunitatinstant( "atinstant",
                            TemporalSpecAtInstant,
                            4,
                            temporalunitatinstantmap,
                            UnitSimpleSelect,
                            UnitInstantTypeMapIntime );

Operator temporalunitatperiods( "atperiods",
                            TemporalSpecAtPeriods,
                            4,
                            temporalunitatperiodsmap,
                            UnitSimpleSelect,
                            UnitPeriodsTypeMap );

Operator temporalunitinitial( "initial",
                          TemporalSpecInitial,
                          4,
                          temporalunitinitialmap,
                          UnitSimpleSelect,
                          UnitTypeMapIntime );

Operator temporalunitfinal( "final",
                        TemporalSpecFinal,
                        4,
                        temporalunitfinalmap,
                        UnitSimpleSelect,
                        UnitTypeMapIntime );

Operator temporalunitpresent( "present",
                          TemporalSpecPresent,
                          8,
                          temporalunitpresentmap,
                          UnitInstantPeriodsSelect,
                          UnitInstantPeriodsTypeMapBool);

Operator temporalunitpasses( "passes",
                         TemporalSpecPasses,
                         4,
                         temporalunitpassesmap,
                         UnitBaseSelect,
                         UnitBaseTypeMapBool);

Operator temporalunitat( "at",
                     TemporalSpecAt,
                     4,
                     temporalunitatmap,
                     UnitBaseSelect,
                     UnitBaseTypeMapUnit );
/*
18 Creating the Algebra

*/

class TemporalUnitAlgebra : public Algebra
{
 public:
  TemporalUnitAlgebra() : Algebra()
  {
   AddOperator( &temporalspeed );
   AddOperator( &temporalunitqueryrect2d );
   AddOperator( &temporalunitpoint2d );
   AddOperator( &temporalunitsize );
   AddOperator( &temporalunitmakemvalue );
   AddOperator( &temporalunittrajectory );
   AddOperator( &temporalunitdeftime );
   AddOperator( &temporalunitatinstant );
   AddOperator( &temporalunitatperiods );
   AddOperator( &temporalunitinitial );
   AddOperator( &temporalunitfinal );
   AddOperator( &temporalunitpresent );
   AddOperator( &temporalunitpasses );
   AddOperator( &temporalunitat );

  }
  ~TemporalUnitAlgebra() {};
};

TemporalUnitAlgebra temporalUnitAlgebra;

/*
19 Initialization

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
InitializeTemporalUnitAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&temporalUnitAlgebra);
}


