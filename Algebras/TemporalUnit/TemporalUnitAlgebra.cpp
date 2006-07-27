/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]

//[TOC] [\tableofcontents]

[1] Implementation of the Units-Operators

May 2006, initial version implemented by Thomas Fischer for diploma
thesis with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Feruniversit[ae]t Hagen.

-----
                            Signatur
  ~Trajectory~           upoint    -> line
  ~Makemvalue~           stream (tuple ([x1:t1,xi:uType,..,xn:tn]))  ->  mType
  ~Size~                 periods  -> real
  ~Deftime~              unit(a)  -> periods
  ~Atinstant~            unit(a) x instant  -> ix
  ~Atperiods~            unit(a) x periods  -> stream(ua)
  ~Initial~              unit(a)  -> ia
  ~final~                unit(a)  -> ia
  ~Present~              unit(a) x instant  -> bool
                         unit(a) x periods  -> bool
  ~point2d~              periods  -> point
  ~queryrect2d~          instant  -> rect
  ~speed~                mpoint   -> mreal
                         upoint   -> ureal
  ~passes~               upoint x point     -> bool
  ~at~                   upoint x point     -> upoint
  ~circle~               point x real x int -> region
  ~velocity~             mpoint  -> mpoint
                         upoint  -> upoint
  ~derivable~            mreal   -> mbool
                         ureal   -> ubool
  ~derivative~           mreal   -> mreal
                         ureal   -> ureal

-----

*/

/*

July 2006, Christian D[ue]ntgen: The so far implemented operators do not suffice
to model typical queries using the compact unit representation ~moving(T)~ with
relations of units. Instead, we also need variants of spatiotemporal operators
that process streams of units.

So we are to implement:

----

OK   ufeed: unit(a) --> stream(unit(a))                                   

     intersects: unit(a) x unit(a) --> stream(ubool)
     intersects: stream(unit(a)) x stream(unit(a)) --> stream(ubool)

     intersection: unit(a) x unit(a) --> stream(unit(a))
     intersection: stream(unit(a)) x stream(unit(a)) --> stream(unit(a))

     atmax, atmin: unit(a) x unit(a) --> stream(unit(a))
     atmax, atmin: stream(unit(a)) x stream(unit(a)) --> stream(unit(a))

     mdirection: upoint --> ureal
     mdirection: stream(upoint) --> stream(ureal)

     no_components: unit(a) --> uint
     no_components: stream(unit(a)) --> stream(uint)

     area: uregion --> ureal
     area: stream(uregion) --> stream(ureal)


     distance: T in {real, int, point}
              uT x uT -> ureal,
              uT x  T -> ureal, 
               T x uT -> ureal

     distance: T in {real, int, region, point}
              uT x uT -> stream(ureal),
              uT x  T -> stream(ureal), 
               T x uT -> stream(ureal)

     distance: T in {real, int, region, point}
              stream(uT) x stream(uT) -> stream(ureal),
              stream(uT) x T          -> stream(ureal), 
              T          x stream(uT) -> stream(ureal)

     and, or: ubool x ubool --> stream(ubool)
               bool x ubool --> stream(ubool)
              ubool x  bool --> stream(ubool)

     and, or: stream(ubool) x stream(ubool) --> stream(ubool)
              stream(ubool) x         bool  --> stream(ubool)
                      bool  x stream(ubool) --> stream(ubool)
     
     =, #: uT x uT --> stream(ubool)
            T x uT --> stream(ubool)
           uT x  T --> stream(ubool)

     =, #: stream (uT) x stream(uT) --> stream(ubool)
                    T  x stream(uT) --> stream(ubool)
           stream (uT) x         T  --> stream(ubool)

     trajectory: stream(upoint) --> line
     trajectory: stream(upoints) --> points

     deftime: stream(uT) --> periods
     rangevalues: stream(uT) --> range(T)

     traversed: stream(uline) --> region
     traverse: stream(uregion) --> region

     atinstant: stream(uT) x instant --> íntime(T)
ERR  atperiods: stream(uT) x periods --> stream(uT)

     initial, final: stream(uT) --> intime(T)

     present: stream(uT) x instant --> bool
     present: stream(uT) x periods --> bool

     at: stream(uT) x T --> stream(uT)
         stream(ut) x range(T) --> stream(uT)
         stream(uT) x point --> stream(upoint)
         stream(uT) x T' --> stream(uT)

----

*/

/*

[TOC]

1 Overview

This file contains the implementation of the unit operators and
helping operators for indexing instant values in R-trees.

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
#include "MovingRegionAlgebra.h"

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

/*
The result storage are cleared before use.

*/

  result.Clear();

/*
Start the collection of real units.

*/
  result.StartBulkLoad();

  for( int i = 0; i < GetNoComponents(); i++ )
  {
/*
first I load the unit i of the moving point

*/

    Get( i, uPoint );
/*
The initial coordinate of the point unit.

*/
     x0 = uPoint->p0.GetX();
     y0 = uPoint->p0.GetY();

/*
The final coordinate of the point unit.

*/
     x1 = uPoint->p1.GetX();
     y1 = uPoint->p1.GetY();

/*
The real unit gets the time interval from the point unit.

*/
     uReal.timeInterval = uPoint->timeInterval;
     inf = uReal.timeInterval.start,
     sup = uReal.timeInterval.end;
/*
convert to milliseconds

*/
     t0 = inf.ToDouble();
     t1 = sup.ToDouble();

/*
The duration within a time interval converted in seconds. 

*/

     duration = (t1 - t0)/1000;

/*
The point unit can be represented as a function of
f(t)= (x0 + x1 * t, y0 + y1 * t).
The result of the derivation is the constant (x1,y1).
Concerning of this the speed is constant in this time interval.
The value are represented in the variable c. The variables a and b
are set to zero.

*/
     uReal.a = 0;
     uReal.b = 0;
     uReal.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
     uReal.r = false;

/*
The real unit are added to the moving real.

*/
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

     t0 = inf.ToDouble();
     t1 = sup.ToDouble();

     duration = (t1 - t0)/1000;   // value in second

     result.a = 0;                // speed is constant in the interval
     result.b = 0;
     result.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
     result.r = false;
   }
}
/*
3.2 Operator ~Velocity~

*/
void MPoint::MVelocity( MPoint& result ) const
{
  const UPoint *uPoint;

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

/*
The result storage are cleared before use.

*/

  result.Clear();
  result.StartBulkLoad();

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, uPoint );

/*
The initial coordinate of the point unit.

*/

     x0 = uPoint->p0.GetX();
     y0 = uPoint->p0.GetY();

/*
The final coordinate of the point unit.

*/

     x1 = uPoint->p1.GetX();
     y1 = uPoint->p1.GetY();

     inf = uPoint->timeInterval.start,
     sup = uPoint->timeInterval.end;

     t0 = inf.ToDouble();
     t1 = sup.ToDouble();

     duration = (t1 - t0)/1000;    // value in second

/*
The time interval iv of the point unit.

*/

     Interval<Instant> iv(uPoint->timeInterval.start,
                          uPoint->timeInterval.end,
                          uPoint->timeInterval.lc,
                          uPoint->timeInterval.rc);

/*
Definition of a new point unit p. The velocity is constant
at all times of the interval of the unit. This is exactly
the same as at the operator speed. The result is a vector
and can be represented as a upoint.

*/

     UPoint p(iv,(x1-x0)/duration,0,(y1- y0)/duration,0);

    result.Add( p );
  }
  result.EndBulkLoad( false );
}

void UPoint::UVelocity( UPoint& result ) const
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

     t0 = inf.ToDouble();
     t1 = sup.ToDouble();

     duration = (t1 - t0)/1000;   // value in second

     Interval<Instant> iv(result.timeInterval.start,
                          result.timeInterval.end,
                          result.timeInterval.lc,
                          result.timeInterval.rc);

     UPoint p(iv,(x1-x0)/duration,0,(y1- y0)/duration,0);

/*
The new point unit p copied to the result storage.

*/

    result.CopyFrom( &p );

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

9.1 Type mapping function ~TypeMapSpeed~

Is used for the ~speed~ operator.

Type mapping for ~speed~ is

----  mpoint  [->]  mreal

----

*/
ListExpr
TypeMapSpeed( ListExpr args )
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

----  instant  [->]  rect

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

----  periods  [->]  point

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

----  periods  [->]  real

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

----  stream (tuple ([x1:t1,x1:ubool,..,[xn:tn)))   ->  mbool
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:uint,..,[xn:tn)))    ->  mint
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:ureal,..,[xn:tn)))   ->  mreal
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:upoint,..,[xn:tn)))  ->  mpoint
              APPEND (i ti)

----

*/
ListExpr MovingTypeMapMakemvalue( ListExpr args )

{
  ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr, listfull, attrtype;

  int j;
  string argstr, argstr2, attrname, inputtype, inputname, fulllist;
/*
The task of the Type Mapping are checking of the nested list.
First check of the list length.

*/
  CHECK_COND(nl->ListLength(args) == 2,
    "Operator makemvalue expects a list of length two.");

  first = nl->First(args);
  nl->WriteToString(argstr, first);

/*
The second one checks the structure of the list.

*/

  CHECK_COND(nl->ListLength(first) == 2  &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
        (nl->ListLength(nl->Second(first)) == 2) &&
        (IsTupleDescription(nl->Second(nl->Second(first)))),
  "Operator makemvalue expects as first argument a list with structure "
  "(stream (tuple ((a1 t1)...(an tn))))\n"
  "Operator makemvalue gets as first argument '" + argstr + "'." );

/*
The third one checks the given parameter in the operator.

*/ 
  second  = nl->Second(args);
  nl->WriteToString(argstr, second);
  CHECK_COND(argstr != "typeerror",
  "Operator makemvalue expects a name of a attribute and not a type.");
/*
inputname represented the name of the given attribute

*/
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
/*
comparison with the attributes in the relation

*/
  if (attrname == inputname)
     inputtype = argstr2;
/*
now I save from the detected attribute the type

*/
while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     firstr = nl->First(rest);
     rest = nl->Rest(rest);
     first2 = nl->First(firstr);
     second2 = nl->Second(firstr);
     nl->WriteToString(attrname, first2);
     nl->WriteToString(argstr2, second2);
/*
comparison with the attributes in the relation

*/
     if (attrname == inputname)
         inputtype = argstr2;
/*
now I save from the detected attribute the type

*/
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
  ListExpr arg1, arg2;
  string argstr;

  if ( nl->ListLength( args ) != 2 )
    {
      ErrorReporter::ReportError("Operator atperiods expects "
				 "a list of length 2.");
      return nl->SymbolAtom( "typeerror" );
    }

  arg1 = nl->First( args );
  arg2 = nl->Second( args );
  
  nl->WriteToString(argstr, arg2);
  if ( !( nl->IsEqual( arg2, "periods" ) ) )
    {
      ErrorReporter::ReportError("Operator atperiods expects a second argument"
			     " of type 'periods' but gets '" + argstr + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if( nl->IsAtom( arg1 ) )
    {
      if( nl->IsEqual( arg1, "ubool" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("ubool"));
      if( nl->IsEqual( arg1, "uint" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("uint"));
      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("ureal"));
      if( nl->IsEqual( arg1, "upoint" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("upoint"));

      nl->WriteToString(argstr, arg1);
      ErrorReporter::ReportError("Operator atperiods expect a first argument "
                                 "of type T in {ubool, uint, ureal, upoint} "
				 "but gets a '" + argstr + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if( !( nl->IsAtom( arg1 ) ) && ( nl->ListLength( arg1 ) == 2) ) 
    {
      cout << "t3a ";
      nl->WriteToString(argstr, arg1);
      if ( !( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
	{
	  ErrorReporter::ReportError("Operator atperiods expects as first "
				     "argument a list with structure 'T' or "
				     "'stream(T)', T in {ubool, uint, ureal, "
				     "upoint} but gets a list with structure '" 
				     + argstr + "'.");
	  return nl->SymbolAtom( "typeerror" );
	}
      
      if( nl->IsEqual( nl->Second(arg1), "ubool" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("ubool"));
      if( nl->IsEqual( nl->Second(arg1), "uint" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("uint"));
      if( nl->IsEqual( nl->Second(arg1), "ureal" ) )
         return nl->TwoElemList(nl->SymbolAtom("stream"),
				nl->SymbolAtom("ureal"));
      if( nl->IsEqual( nl->Second(arg1), "upoint" ) )
	 return nl->TwoElemList(nl->SymbolAtom("stream"),
				 nl->SymbolAtom("upoint"));

      nl->WriteToString(argstr, nl->Second(arg1));
      cout << "t3c " << argstr;
      ErrorReporter::ReportError("Operator atperiods expects a type "
                              "(stream T); T in {ubool, uint, ureal, upoint} "
			      "but gets '(stream " + argstr + ")'.");
      return nl->SymbolAtom( "typeerror" );
    };

  nl->WriteToString( argstr, args );
  ErrorReporter::ReportError("Operator atperiods encountered an "
			     "unmatched typerror for arguments '"
			     + argstr + "'.");
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
10.4 Type mapping function ~TypeMapCircle~

It is for the operator ~circle~.

*/
ListExpr
TypeMapCircle( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );


    if( nl->IsEqual( arg1, "point" ) && nl->IsEqual( arg2, "real" )
       && nl->IsEqual( arg3, "int" ) )
      return nl->SymbolAtom( "region" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
10.6 Type mapping function ~TypeMapmakepoint~

It is for the operator ~makepoint~.

*/
ListExpr
TypeMapMakepoint( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "int" )
       && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "point" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
10.7 Type mapping function ~TypeMapvelocity~

It is for the operator ~velocity~.

Type mapping for ~velocity~ is

----  mpoint  ->  mpoint

----


*/
ListExpr
TypeMapVelocity( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "mpoint" );
   if( nl->IsEqual( arg1, "upoint" ) )
      return nl->SymbolAtom( "upoint" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
10.8 Type mapping function ~TypeMapderivable~

It is for the operator ~derivable~.

Type mapping for ~derivable~ is

----  mreal  ->  mbool

----


*/
ListExpr
TypeMapDerivable( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mreal" ) )
      return nl->SymbolAtom( "mbool" );
   if( nl->IsEqual( arg1, "ureal" ) )
      return nl->SymbolAtom( "ubool" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
10.9 Type mapping function ~TypeMapderivative~

It is for the operator ~derivative~.

Type mapping for ~derivative~ is

----  mreal  ->  mreal

----


*/
ListExpr 
TypeMapDerivative( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mreal" ) )
      return nl->SymbolAtom( "mreal" );
   if( nl->IsEqual( arg1, "ureal" ) )
      return nl->SymbolAtom( "ureal" );
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
10.10 Type mapping function for operator ~ufeed~

The operator is used to cast a single unit(T) to a stream(unit(T)).

*/

ListExpr 
TypeMapUfeed( ListExpr args )
{
  ListExpr arg1;

  if ( ( nl->ListLength(args) == 1 ) && ( nl->IsAtom(nl->First(args) ) ) )
    {
      arg1 = nl->First(args);
      
      if( nl->IsEqual( arg1, "ubool" ) )
	return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("ubool"));
      
      if( nl->IsEqual( arg1, "uint" ) )
	return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("uint"));
      
      if( nl->IsEqual( arg1, "ureal" ) )
	return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("ureal"));
      
      if( nl->IsEqual( arg1, "upoint" ) )
	return nl->TwoElemList(nl->SymbolAtom("stream"),
			       nl->SymbolAtom("upoint"));
    }
  ErrorReporter::ReportError("Operator ufeed  expects a list of length one, "
			     "containing a value of one type of {uint, ubool, "
			     "ureal, upoint}.");
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

Is used for the ~deftime~,~atinstant~,~atperiods~, ~initial~, ~final~  
operations.

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
An extended version can map (unit) as well as (stream unit):

*/

int
UnitCombinedUnitStreamSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (nl->IsAtom( arg1 ) )
    {
      if( nl->SymbolValue( arg1 ) == "ubool" )
        return 0;
      if( nl->SymbolValue( arg1 ) == "uint" )
        return 1;
      if( nl->SymbolValue( arg1 ) == "ureal" )
        return 2;
      if( nl->SymbolValue( arg1 ) == "upoint" )
        return 3;
    }

  if(   !( nl->IsAtom( arg1 ) )
      && ( nl->ListLength(arg1) == 2 )
      && ( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
    { if( nl->IsEqual( nl->Second(arg1), "ubool" ) )
	return 4;
      if( nl->IsEqual( nl->Second(arg1), "uint" ) )
	return 5;
      if( nl->IsEqual( nl->Second(arg1), "ureal" ) )
	return 6;
      if( nl->IsEqual( nl->Second(arg1), "upoint" ) )
	return 7;
    }
  
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
16.1.6 Selection function ~VelocitySelect~

Is used for the ~velocity~ operations.

*/
int
VelocitySelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "mpoint")
    return 0;

  if( nl->SymbolValue( arg1 ) == "upoint")
    return 1;

  return -1; // This point should never be reached
}
/*
16.1.7 Selection function ~DerivableSelect~

Is used for the ~derivable~ and ~derivative~ operations.

*/
int
DerivableSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "mreal")
    return 0;

  if( nl->SymbolValue( arg1 ) == "ureal")
    return 1;

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
/*
The call of the member function MSpeed.

*/
 ((MPoint*)args[0].addr)->MSpeed(  *((MReal*)result.addr) );

  return 0;
}
int UnitPointSpeed(Word* args,Word& result,int message,Word& local,Supplier s)
{

  result = qp->ResultStorage( s );
/*
The call of the member function USpeed.

*/

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
    timevalue = Inv->ToDouble();

    x1 = 0;
    x2 = timevalue;
    y1 = timevalue;
    y2 = MAXDOUBLE;

/*
Definition of a rectangle with the dimension 2.

*/
    min[0] = x1;
    min[1] = x2;
    max[0] = y1;
    max[1] = y2;

    Rectangle<dim>* rect = new Rectangle<dim>(true, min, max);
    ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
    delete rect;
   }
  else
   {
/*
Definition of a rectangle with the dimension 2.
The instant are not defined. The rectangle filled
with zero values.

*/

    min[0] = x1;
    min[1] = x2;
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
/*
Checking if the periods value is valid and defined.

*/
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
/*
Derives the maximum of all intervals.

*/
    Y = sup.ToDouble();
/*
Derives the minimum of all intervals.

*/
    X = inf.ToDouble();

  }
/*
Returns the calculated point.

*/ 
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

/*
Checking if the periods value is valid and defined.

*/

  if( !range->IsEmpty()  )
    {
      const Interval<Instant> *intv1, *intv2;
      duration = 0;
      
      for( int i = 0; i < range->GetNoComponents(); i++ )
	{
	  range->Get( i, intv1 );
	  range->Get( i, intv2 );
	  
	  Interval<Instant> timeInterval(intv1->start,intv2->end,
					 intv1->lc, intv2->rc);
	  
	  sup = timeInterval.end;
    inf = timeInterval.start;

    intervalue_sup = sup.ToDouble();
    intervalue_inf = inf.ToDouble();
/*
  Summarizing of all time intervals.
      
*/
    duration += (intervalue_sup - intervalue_inf)/1000;    // value in second
    
	}
      
    res = duration;
    }
/*
  Returns the result in the storage.
    
*/
  
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

/*
Assertion if not two input values are available.

*/

  assert(args[2].addr != 0);
  assert(args[3].addr != 0);

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

/*
In the first input channel is the index of the attribute. The stream
processing opens in the first step the queryprocessor. In the next steps
tuple for tuple will be requested. 

*/

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  result = qp->ResultStorage(s);

/*
This is a template function for many datatypes.
The mapping class can cast moving datatypes.
The unit class can cast  unit datatypes.

*/

  m = (Mapping*) result.addr;

/*
For the first use the storage must be cleared.

*/

  m->Clear();
  m->StartBulkLoad();

/*
The loop runs until the last tuple is received.

*/

  while ( qp->Received(args[0].addr) )
  {

    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    Attribute* currentAttr = (Attribute*)currentTuple->
                              GetAttribute(attributeIndex);

    if(currentAttr->IsDefined())
    {

     unit = (Unit*) currentAttr;

     m->Add( *unit );
/*
Consume the stream object, if the deletion is possible.

*/
     currentTuple->DeleteIfAllowed();
     }
    qp->Request(args[0].addr, currentTupleWord);
  }
   m->EndBulkLoad( false );

/*
The queryprocessor are closed. The stream processing is finished.

*/

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

/*
The result is of the datatype line.

*/
  CLine *line = ((CLine*)result.addr);
/*
The input parameter if from type upoint.

*/
  UPoint *upoint = ((UPoint*)args[0].addr);
/*
Call of the Memberfunction UTrajectory.

*/
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
  else    //not contained
      pos = -1;


  if( pos == -1 )  // not contained in the unit
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

struct AtPeriodsLocalInfo
{
  Word uWord;     // the address of the unit point/int/real value
  Word pWord;    //  the adress of the period value
  int  j;       //   save the number of the interval
};

/*
Variant 1: first argument is a scalar value

*/

template <class Mapping>
int MappingUnitAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{

  AtPeriodsLocalInfo *localinfo;
  const Interval<Instant> *interval;

  const Mapping* unit;
  Mapping r;
  Periods* periods;


  switch( message )
  {
    case OPEN:

      localinfo = new AtPeriodsLocalInfo;
      qp->Request(args[0].addr, localinfo->uWord);
      qp->Request(args[1].addr, localinfo->pWord);
      localinfo->j = 0;
      local = SetWord(localinfo);
      return 0;

    case REQUEST:

      if( local.addr == 0 )
        return CANCEL;
      localinfo = (AtPeriodsLocalInfo *)local.addr;
      unit = (Mapping*)localinfo->uWord.addr;
      periods = (Periods*)localinfo->pWord.addr;

      if( localinfo->j == periods->GetNoComponents() )
         return CANCEL;
      periods->Get( localinfo->j, interval );

    if( interval->Before( unit->timeInterval ) )
    {
    while (1)
     {
      if( ++localinfo->j == periods->GetNoComponents() )
         break;
       periods->Get(localinfo->j, interval);
       if (!( interval->Before( unit->timeInterval )))
          break;
      }
     }

     if( localinfo->j >= periods->GetNoComponents() ) {
         result.addr = 0;
         return CANCEL;
       }

   if( unit->timeInterval.Before( *interval ) )
    {
         result.addr = 0;
         return CANCEL;
    }
    else
    {

      unit->AtInterval( *interval, r );
      Mapping* aux = new Mapping( r );
      result = SetWord( aux );

      localinfo->j++;

      return YIELD;

    }

    return CANCEL; // should not happen


    case CLOSE:

      if( local.addr != 0 )
        delete (AtPeriodsLocalInfo *)local.addr;
      return 0;
  }
  /* should not happen */
  return -1;

}

/*
Variant 2: first argument is a stream
Implemented on 07/17/2006 by Christian D[ue]ntgen

*/

struct AtPeriodsLocalInfoUS
{
  Word uWord;  // address of the input stream
  Word pWord;  // address of the input periods value
  int j;       // interval counter for within periods
};


template <class Mapping>
int MappingUnitStreamAtPeriods( Word* args, Word& result, int message,
                                Word& local, Supplier s )
{
  AtPeriodsLocalInfoUS *localinfo;
  Mapping *unit, *aux;
  Mapping resultUnit;
  Periods *periods;
  const Interval<Instant> *interval;

  switch( message )
  { 
  case OPEN:
    localinfo = new AtPeriodsLocalInfoUS;
    qp->Request(args[1].addr, localinfo->pWord);   // get address of periods
    localinfo->j = 0;                              // init interval counter
    qp->Open( args[0].addr );                      // open stream of units
    qp->Request( args[0].addr, localinfo->uWord ); // request first unit
    if ( !( qp->Received( args[0].addr) ) )
      {result.addr = 0; return CANCEL; }
    local = SetWord(localinfo);                    // pass up link to localinfo
    return 0;

  case REQUEST:
    if ( local.addr == 0 )
      return CANCEL;
    localinfo = (AtPeriodsLocalInfoUS *) local.addr; // restore local data
    if ( localinfo->uWord.addr == 0 )
      { result.addr = 0; return CANCEL; }
    unit = (Mapping *) localinfo->uWord.addr;
    if ( localinfo->pWord.addr == 0 )
      { result.addr = 0; return CANCEL; }
    periods = (Periods *) localinfo->pWord.addr;
    
    // search for a pair of overlapping unit/interval:
    
    while (1)
      {
	if ( localinfo->j == periods->GetNoComponents() ) // redo first interval
	  { localinfo->j = 0;
	    unit->DeleteIfAllowed();                // delete original unit?
	    qp->Request(args[0].addr, localinfo->uWord);  // get new unit
	    if( qp->Received( args[0].addr ) )
	      unit = (Mapping *) localinfo->uWord.addr;
	    else 
	      { result.addr = 0; return CANCEL; }   // end of unit stream
	  }
	periods->Get(localinfo->j, interval);       // get an interval
	if (    !( interval->Before( unit->timeInterval ) )
		&& !( unit->timeInterval.Before( *interval) ) )
	  break;                           // found candidate, break while
	localinfo->j++;                             // next interval, loop
      }
    
    // We have an interval possibly overlapping the unit's interval now
    // Return unit restricted to overlapping part of both intervals
    
    unit->AtInterval( *interval, resultUnit); // intersect unit and interval
    aux = new Mapping( resultUnit );
    result = SetWord( aux );
    localinfo->j++;                           // increase interval counter
    return YIELD;
    
  case CLOSE:
    if ( local.addr != 0 )
      {
	localinfo = (AtPeriodsLocalInfoUS *) local.addr;
	if ( localinfo->uWord.addr != 0 )
	  {
	    unit = (Mapping *) localinfo->uWord.addr;
	    unit->DeleteIfAllowed();   // delete remaining original unit
	  }
	delete (AtPeriodsLocalInfoUS *)localinfo;
      }
    return 0;
    
  } // end switch
  
  return -1; // should never be reached
  
} // end MappingUnitStreamAtPeriods


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
16.2.15 Value mapping functions of operator ~circle~

*/
int Circle( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
/*
Centre of the circle.

*/
  Point* p = (Point*)args[0].addr;
/*
Radius of the circle.

*/
  CcReal* r = (CcReal*)args[1].addr;
/*
The amount of edges for the polygon.
Increasing the amount come a polygon closer to a circle.

*/
  CcInt* narg = (CcInt*)args[2].addr;

  CRegion *res = (CRegion*)result.addr;


  double x, y;
  int n;
  double radius;
  double valueX, valueY;
  double angle;
  int partnerno = 0;


  x = p->GetX();
  y = p->GetY();

  n = narg->GetIntval();
  radius = r->GetRealval();

  res->Clear();
  res->StartBulkLoad();

/*
Definition of a empty region.

*/
  CRegion rg;
  CHalfSegment chs(false);


  if (( p->IsDefined())&&(n>3)&&(n<100)&&(radius >0.0))
    {

/*
Determination of a n polygon.
Division of 360 degree in n parts with the help of
a standardised circle and the circumference. U = 2 * PI

*/
     for( int i = 0; i < n; i++ )
        {
        angle = i * 2 * PI/n;
        valueX = x + radius * cos(angle);
        valueY = y + radius * sin(angle);
/*
The first point of the segment of a region.
The x-value can be defined with the cosine and
the y-value with the sine.

*/
        Point edge1(true, valueX ,valueY);

        chs.attr.faceno = 0;
        chs.attr.cycleno = 0;
        chs.attr.edgeno = partnerno;
        chs.attr.partnerno = partnerno++;

        if ((i+1) >= n)
          angle = 0 * 2 * PI/n;
       else
          angle = (i+1) * 2 * PI/n;

        valueX = x + radius * cos(angle);
        valueY = y + radius * sin(angle);
/*
The second point of the segment of a region.

*/
        Point edge2(true, valueX ,valueY);

/*
Definition of the halfsegments.

*/
         chs.Set(true, edge1, edge2);
         *res += chs;
          chs.SetLDP( false );
         *res += chs;

        }

    }
   res->EndBulkLoad(true);
   res->SetPartnerNo();
   res->ComputeRegion();


  return 0;
}

/*
16.2.17 Value mapping functions of operator ~makepoint~

*/
int MakePoint( Word* args, Word& result, int message, Word& local, Supplier s )
{

  result = qp->ResultStorage( s );
  CcInt* value1 = (CcInt*)args[0].addr;
  CcInt* value2 = (CcInt*)args[1].addr;

  ((Point*)result.addr)->Set(value1->GetIntval(),value2->GetIntval() );

  return 0;
}
/*
16.2.18 Value mapping functions of operator ~velocity~

*/
int MPointVelocity(Word* args, Word& result, int message,
                   Word& local, Supplier s)
{

  result = qp->ResultStorage( s );
 ((MPoint*)args[0].addr)->MVelocity(  *((MPoint*)result.addr) );

  return 0;
}
int UnitPointVelocity(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{

  result = qp->ResultStorage( s );
 ((UPoint*)args[0].addr)->UVelocity(  *((UPoint*)result.addr) );

  return 0;
}

/*
16.2.19 Value mapping functions of operator ~derivable~

*/
int MPointDerivable( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{

  result = qp->ResultStorage( s );
  MReal* value = (MReal*)args[0].addr;
  MBool* res = ((MBool*)result.addr);

  const UReal *uReal;
  UBool boolvalue;
  CcBool b;

  res->Clear();
  res->StartBulkLoad();

  for( int i = 0; i < value->GetNoComponents(); i++ )
  {
/*
Load of a real unit.

*/
    value->Get( i, uReal );
/*
FALSE means in this case that a real unit describes a quadratic
polynomial. A derivation is possible and the operator returns TRUE.

*/
    if (uReal->r == false)
     {
       b.Set(true,true);
     }
    else
     {
       b.Set(true,false);
      }

    UBool boolvalue (uReal->timeInterval,b);
    res->Add( boolvalue );
  }
  res->EndBulkLoad( false );

  return 0;
}
int UnitPointDerivable( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  UReal* uReal = (UReal*)args[0].addr;
  UBool* res = ((UBool*)result.addr);

  CcBool b;

 if (uReal->IsDefined())
  {

   res->timeInterval = uReal->timeInterval;

   if (uReal->r == false)
     {
        b.Set(true,true);
     }
    else
     {
        b.Set(true,false);
      }

     UBool boolvalue (uReal->timeInterval,b);
     res->CopyFrom(&boolvalue);
  }

  return 0;
}
/*
16.2.20 Value mapping functions of operator ~derivative~

*/
int MPointDerivative( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{

  result = qp->ResultStorage( s );
  MReal* value = (MReal*)args[0].addr;
  MReal* res = ((MReal*)result.addr);

  const UReal *Unit;
  UReal uReal;

  res->Clear();
  res->StartBulkLoad();

  for( int i = 0; i < value->GetNoComponents(); i++ )
  {
/*
Load of a real unit.

*/
 
   value->Get( i, Unit );

/*
FALSE means in this case that a real unit describes a quadratic
polynomial. A derivation is possible. 
The polynom looks like at$^2$ + bt + c.
The derivative of this polynom is 2at + b.

*/   
 if (Unit->r == false)
   {
     uReal.timeInterval = Unit->timeInterval;
     uReal.a = 0;
     uReal.b = 2 * Unit->a;
     uReal.c = Unit->b;
     uReal.r = Unit->r;
   }
 else
   {
/*
A derivation of a real unit is not possible.

*/
     uReal.SetDefined(false);
   }
 
 res->Add( uReal );
  }
  res->EndBulkLoad( false );
  
  return 0;
}
int UnitPointDerivative( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  UReal* Unit = (UReal*)args[0].addr;
  UReal* res = ((UReal*)result.addr);


 if (Unit->IsDefined())
  {


    if (Unit->r == false)
     {
      res->timeInterval = Unit->timeInterval;
      res->a = 0;
      res->b = 2 * Unit->a;
      res->c = Unit->b;
      res->r = Unit->r;
     }
    else
     {
       res->SetDefined(false);
      }

   }

  return 0;
}

/*
16.2. Value mapping function for operator ~ufeed~

The operator is used to cast a single unit(T) to a stream(unit(T))
having a single element unit(T).

*/

struct UnitFeedLocalInfo
{
  Word uWord;
  bool finished;
};

template <class Mapping>
int MappingUnitFeed( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{
  UnitFeedLocalInfo *localinfo;
  Mapping *unit;

  switch( message )
    {
    case OPEN:
      localinfo = new UnitFeedLocalInfo;
      localinfo->finished = false;
      qp->Request(args[0].addr, localinfo->uWord);
      local = SetWord(localinfo);
      return 0;

    case REQUEST:
      if ( local.addr == 0 ) 
	return CANCEL;
      localinfo = ( UnitFeedLocalInfo *)local.addr;
      if ( localinfo->finished )
	return CANCEL;
      unit = (Mapping *)localinfo->uWord.addr;
      result = SetWord( unit );
      localinfo->finished = true;
      return YIELD;
	
    case CLOSE:
      if ( local.addr == 0 ) 
	{ 
	  localinfo = ( UnitFeedLocalInfo*) local.addr;
	  delete localinfo;
	}
      return 0;     
    }
  return -1; // should not be reached
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
                                            MappingUnitAtPeriods<UPoint>,
                                            MappingUnitStreamAtPeriods<UBool>,
                                            MappingUnitStreamAtPeriods<UInt>,
                                            MappingUnitStreamAtPeriods<UReal>,
                                            MappingUnitStreamAtPeriods<UPoint>  
                                          };

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

ValueMapping temporalvelocitymap[] = { MPointVelocity,
                                       UnitPointVelocity };

ValueMapping temporalderivablemap[] = { MPointDerivable,
                                        UnitPointDerivable };

ValueMapping temporalderivativemap[] = { MPointDerivative,
                                       UnitPointDerivative };

ValueMapping temporalunitfeedmap[] = { MappingUnitFeed<UBool>,
				       MappingUnitFeed<UInt>,
				       MappingUnitFeed<UReal>,
				       MappingUnitFeed<UPoint>};

/*
17.2 Specification strings

17.2.1 Specification string of operator ~speed~

*/
const string
TemporalSpecSpeed  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(mpoint/upoint) -> mreal/ureal</text--->"
"<text>speed( _ )</text--->"
"<text>return the speed of a spatial object.</text--->"
"<text>query speed(mp1)</text---> ) )";


/*
17.2.2 Specification string of operator ~queryrect2d~

*/
const string
TemporalSpecQueryrect2d  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(instant) -> rect</text--->"
"<text>queryrect2d( _ )</text--->"
"<text>return the rect of an instant object for a time interval.</text--->"
"<text>query queryrect2d(instant)</text---> ) )";

/*
17.2.3 Specification string of operator ~point2d~

*/
const string
TemporalSpecPoint2d  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(periods) -> point</text--->"
"<text>point2d( _ )</text--->"
"<text>return the point of a given interval.</text--->"
"<text>query point2d(periods)</text---> ) )";

/*
17.2.4 Specification string of operator ~size~

*/
const string
TemporalSpecSize  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(periods) -> real</text--->"
"<text>size( _ )</text--->"
"<text>return the duration of a moving object.</text--->"
"<text>query size(periods)</text---> ) )";

/*
17.2.5 Specification string of operator ~makemvalue~

*/
const string
TemporalSpecMakemvalue  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>((stream (tuple ((x1 t1)...(xn tn)))"
" (unit(xi))))-> moving(x)</text--->"
"<text> makemvalue[ _ ]</text--->"
"<text>get the moving value of the stream of units.</text--->"
"<text>makemvalue[ u1 ]</text---> ) )";

/*
17.2.6 Specification string of operator ~trajectory~

*/
const string
TemporalSpecTrajectory  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>upoint -> line</text--->"
"<text> trajectory( _ )</text--->"
"<text>get the trajectory of the corresponding"
"unit point object.</text--->"
"<text>trajectory( up1 )</text---> ) )";

/*
17.2.7 Specification string of operator ~deftime~

*/
const string
TemporalSpecDefTime  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>unit(x) -> periods</text--->"
"<text> deftime( _ )</text--->"
"<text>get the defined time of the corresponding"
" unit data objects.</text--->"
"<text>deftime( up1 )</text---> ) )";

/*
17.2.8 Specification string of operator ~atinstant~

*/
const string
TemporalSpecAtInstant  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(unit(x) instant) -> intime(x)</text--->"
"<text>_ atinstant _ </text--->"
"<text>get the Intime value corresponding to the "
"instant.</text--->"
"<text>upoint1 atinstant instant1</text---> ) )";

/*
17.2.9 Specification string of operator ~atperiods~

*/
const string
TemporalSpecAtPeriods  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>T in {int, bool, real, point},\n"
"(unit(T) periods) -> stream uT\n"
"((stream uT) periods) -> stream uT</text--->"
"<text>_ atperiods _ </text--->"
"<text>restrict the movement to the given"
" periods.</text--->"
"<text>upoint1 atperiods periods1</text---> ) )";


/*
17.2.10 Specification string of operator ~Initial~

*/
const string
TemporalSpecInitial  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>unit(x) -> intime(x)</text--->"
"<text> initial( _ )</text--->"
"<text>get the intime value corresponding"
" to the initial instant.</text--->"
"<text>initial( upoint1 )</text---> ) )";


/*
17.2.11 Specification string of operator ~final~

*/
const string
TemporalSpecFinal  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>unit(x) -> intime(x)</text--->"
"<text> final( _ )</text--->"
"<text>get the intime value corresponding"
" to the final instant.</text--->"
"<text>final( upoint1 )</text---> ) )";

/*
17.2.12 Specification string of operator ~present~

*/
const string
TemporalSpecPresent  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(moving/unit(x) instant) -> bool,"
" (moving/unit(x) periods) -> bool</text--->"
"<text>_ present _ </text--->"
"<text>whether the object is present at the"
" given instant or period.</text--->"
"<text>mpoint1 present instant1</text---> ) )";

/*
17.2.13 Specification string of operator ~passes~

*/
const string
TemporalSpecPasses =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(unit(x) x) -> bool</text--->"
"<text>_ passes _ </text--->"
"<text>whether the object passes the given"
" value.</text--->"
"<text>upoint1 passes point1</text---> ) )";


/*
17.2.14 Specification string of operator ~at~

*/
const string
TemporalSpecAt =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(unit(x) x) -> unit(x)</text--->"
"<text> _ at _ </text--->"
"<text>restrict the movement at the times"
" where the equality occurs.</text--->"
"<text>upoint1 at point1</text---> ) )";

/*
17.2.15 Specification string of operator ~circle~

*/
const string
TemporalSpecCircle =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>point x real x int -> region</text--->"
"<text> circle ( _ ) </text--->"
"<text>defines a circle with a given radius"
" and n calculated points.</text--->"
"<text>circle (p,10.0,10)</text---> ) )";

/*
17.2.16 Specification string of operator ~makepoint~

*/
const string
TemporalSpecMakePoint =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x int -> point</text--->"
"<text> makepoint ( _ ) </text--->"
"<text>create a point with two"
" given values.</text--->"
"<text>makepoint (5,5)</text---> ) )";

/*
17.2.17 Specification string of operator ~velocity~

*/
const string
TemporalSpecVelocity=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mpoint/upoint -> mpoint/upoint</text--->"
"<text> velocity ( _ ) </text--->"
"<text>describes the vector of the speed"
" to the given object.</text--->"
"<text>velocity (mpoint)</text---> ) )";

/*
17.2.18 Specification string of operator ~derivable~

*/
const string
TemporalSpecDerivable=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mreal/ureal -> mbool/ubool</text--->"
"<text> derivable ( _ ) </text--->"
"<text>get the hint"
" for a mreal/ureal which part is derivable.</text--->"
"<text>derivable (mreal)</text---> ) )";

/*
17.2.19 Specification string of operator ~derivative~

*/
const string
TemporalSpecDerivative=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mreal/ureal -> mreal/ureal</text--->"
"<text> derivative ( _ ) </text--->"
"<text>determination the derivative"
" of a mreal/ureal.</text--->"
"<text>derivable (mreal)</text---> ) )";

/*
17.2.20 Specification string of operator ~ufeed~

*/
const string
TemporalSpecUfeed=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, real point} \n"
"uT -> stream uT</text--->"
"<text>ufeed ( _ ) </text--->"
"<text>create a single-unit stream from "
"a single unit.</text--->"
"<text>ufeed (ureal)</text---> ) )";


/*
17.3 Operators

*/

Operator temporalspeed( "speed",
                      TemporalSpecSpeed,
                      2,
                      temporalspeedmap,
                      SpeedSelect,
                      TypeMapSpeed);

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
                            8,
                            temporalunitatperiodsmap,
                            UnitCombinedUnitStreamSelect,
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

Operator temporalcircle( "circle",
                      TemporalSpecCircle,
                      Circle,
                      Operator::SimpleSelect,
                      TypeMapCircle);

Operator temporalmakepoint( "makepoint",
                      TemporalSpecMakePoint,
                      MakePoint,
                      Operator::SimpleSelect,
                      TypeMapMakepoint);

Operator temporalvelocity( "velocity",
                      TemporalSpecVelocity,
                      2,
                      temporalvelocitymap,
                      VelocitySelect,
                      TypeMapVelocity);

Operator temporalderivable( "derivable",
                      TemporalSpecDerivable,
                      2,
                      temporalderivablemap,
                      DerivableSelect,
                      TypeMapDerivable);

Operator temporalderivative( "derivative",
                      TemporalSpecDerivative,
                      2,
                      temporalderivativemap,
                      DerivableSelect,
                      TypeMapDerivative);

Operator temporalufeed( "ufeed",
                      TemporalSpecUfeed,
                      4,
                      temporalunitfeedmap,
                      UnitSimpleSelect,
                      TypeMapUfeed);


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
   AddOperator( &temporalcircle );
   AddOperator( &temporalmakepoint );
   AddOperator( &temporalvelocity );
   AddOperator( &temporalderivable );
   AddOperator( &temporalderivative );
   AddOperator( &temporalufeed );

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
