/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]

//[TOC] [\tableofcontents]

[1] TemporalUnitAlgebra - Implementing Units-Operators

May 2006, initial version implemented by Thomas Fischer for diploma
thesis with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Feruniversit[ae]t Hagen.

----
                            Signatur
  Trajectory           upoint    -> line
  Makemvalue           stream (tuple ([x1:t1,xi:uType,..,xn:tn]))  ->  mType
  Size                 periods  -> real
  Deftime              unit(a)  -> periods
  Atinstant            unit(a) x instant  -> ix
  Atperiods            unit(a) x periods  -> stream(ua)
  Initial              unit(a)  -> ia
  final                unit(a)  -> ia
  Present              unit(a) x instant  -> bool
                       unit(a) x periods  -> bool
  point2d              periods  -> point
  queryrect2d          instant  -> rect
  speed                mpoint   -> mreal
                       upoint   -> ureal
  passes               upoint x point     -> bool
  at                   upoint x point     -> upoint
  circle               point x real x int -> region
  velocity             mpoint  -> mpoint
                       upoint  -> upoint
  derivable            mreal   -> mbool
                       ureal   -> ubool
  derivative           mreal   -> mreal
                       ureal   -> ureal

----

*/

/*

July 2006, Christian D[ue]ntgen: The so far implemented operators do not suffice
to model typical queries using the compact unit representation ~moving(T)~ with
relations of units. Instead, we also need variants of spatiotemporal operators
that process streams of units. Instead of implementing them directly, we will 
fake them using the set wrapper-operators ~suse~ on the native single-unit 
operators.

The ~suse~ operators passes single values to an operator one-by-one value and 
collects all returned values/streams of values in a flat stream of values.

This does not work for some binary predicates, like equal, but one could implement
an ordered pairwise comparison here.

It may be useful, to have some operators consuming a stream of units and
returning an aggregated vale, as e.g. initial, final, present, never, always.

So we are to implement:

----

State Operator/Signatures

ERR  suse: (stream X)            (map X Y)            -> (stream Y)
OK         (stream X)            (map X (stream Y))   -> (stream Y)
           (stream X) Y          (map X Y Z)          -> (stream Z)
           (stream X) Y          (map X Y stream(Z))  -> (stream Z)
           X          (stream Y) (map X y Z)          -> (stream Z)
           X          (stream Y) (map X y (stream Z)) -> (stream Z)
           (stream X) (stream Y) (map X Y Z)          -> (stream Z)
           (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
           for X,Y,Z of kind DATA




OK   ufeed: unit(a) --> stream(unit(a))                                   



     intersects: unit(a) x unit(a) --> stream(ubool)

     intersection: unit(a) x unit(a) --> stream(unit(a))

     atmax, atmin: unit(a) x unit(a) --> stream(unit(a))

     at:        unit(a)  x a --> stream(unit(a))

     mdirection: upoint --> ureal

     no_components: unit(a) --> uint

     area: uregion --> ureal

     distance: T in {real, int, point}
              uT x uT -> ureal,
              uT x  T -> ureal, 
               T x uT -> ureal

     distance: T in {real, int, region, point}
              uT x uT -> stream(ureal),
              uT x  T -> stream(ureal), 
               T x uT -> stream(ureal)

     and, or: ubool x ubool --> stream(ubool)
               bool x ubool --> stream(ubool)
              ubool x  bool --> stream(ubool)

     =, #: uT x uT --> stream(ubool)
            T x uT --> stream(ubool)
           uT x  T --> stream(ubool)




     initial, final: stream(uT) --> intime(T)
     present: stream(uT) x instant --> bool
     present: stream(uT) x periods --> bool
     never:   stream(uBbool) --> bool
     always:  stream(uBbool) --> bool

----

*/

/*

August 2006, Christian D[ue]ntgen: Added missing checks for ~undefined~ 
argument values to all value mapping functions. The functions will now
return ~undefined~ result values for these cases.

Changed structure of the file to become ordered by operators rather than by
typemapping, valuemapping etc. This makes the file easier to extend.

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
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


#include "DateTime.h"
using namespace datetime;



/*
2.1 Definition of some constants

*/
const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = numeric_limits<double>::min();


/*
3 Implementation of the unit class method operators

This section implements operators as member functions of the respective
unit class.

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

  if ( ! IsDefined() )
    result.SetDefined( false );

  else
    {
      result.StartBulkLoad();

      for( int i = 0; i < GetNoComponents(); i++ )
	{
	  Get( i, uPoint );
	  
	  x0 = uPoint->p0.GetX(); // initial pos
	  y0 = uPoint->p0.GetY();
	  
	  x1 = uPoint->p1.GetX(); // final pos
	  y1 = uPoint->p1.GetY();
	  
	  uReal.timeInterval = uPoint->timeInterval; // copy interval 
	  inf = uReal.timeInterval.start;            // for result 
	  sup = uReal.timeInterval.end;              // from argument
	  
	  t0 = inf.ToDouble(); // convert to milliseconds
	  t1 = sup.ToDouble();
	  
	  duration = (t1 - t0)/1000; // interval duration to seconds
	  
	  /*
	    The point unit can be represented as a function of
	    f(t) = (x0 + x1 * t, y0 + y1 * t).
	    The result of the derivation is the constant (x1,y1).
	    The speed is constant in each time interval.
	    Its value is represented by variable c. The variables a and b  
	    are set to zero.
	    
	  */
	  uReal.a = 0;
	  uReal.b = 0;
	  uReal.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
	  uReal.r = false;
	  
	  result.Add( uReal ); // append ureal to mreal
	}
      result.EndBulkLoad( false );
    }
}
void UPoint::USpeed( UReal& result ) const
{

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  if ( ! IsDefined() )
    result.SetDefined( false );
  else
    {
      
      x0 = p0.GetX();
      y0 = p0.GetY();
      
      x1 = p1.GetX();
      y1 = p1.GetY();
      
      result.timeInterval = timeInterval;
      
      if (result.IsDefined() )
	{
	  inf = result.timeInterval.start;
	  sup = result.timeInterval.end;
	  
	  t0 = inf.ToDouble();
	  t1 = sup.ToDouble();
	  
	  duration = (t1 - t0)/1000;   // value in seconds
	  
	  result.a = 0;                // speed is constant in the interval
	  result.b = 0;
	  result.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
	  result.r = false;
	}
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

  result.Clear();

  if ( ! IsDefined() )
    result.SetDefined( false );
  else
    {
      result.StartBulkLoad();

      for( int i = 0; i < GetNoComponents(); i++ )
	{
	  Get( i, uPoint );
	  
	  x0 = uPoint->p0.GetX(); // initial coordinates
	  y0 = uPoint->p0.GetY();
	  
	  x1 = uPoint->p1.GetX(); // final coordinates
	  y1 = uPoint->p1.GetY();
	  
	  inf = uPoint->timeInterval.start;
	  sup = uPoint->timeInterval.end;
	  
	  t0 = inf.ToDouble();
	  t1 = sup.ToDouble();
	  
	  duration = (t1 - t0)/1000;    // value in second
	  
	  //  create an interval:
	  Interval<Instant> iv(uPoint->timeInterval.start,
			       uPoint->timeInterval.end,
			       uPoint->timeInterval.lc,
			       uPoint->timeInterval.rc);
	  
	  /*
	    Definition of a new point unit p. The velocity is constant
	    at all times of the interval of the unit. This is exactly
	    the same as within operator ~speed~. The result is a vector
	    and can be represented as a upoint.
	    
	  */
	  
	  UPoint p(iv,(x1-x0)/duration,0,(y1- y0)/duration,0);
	  
	  result.Add( p );
	}
      result.EndBulkLoad( false );
    }
}

void UPoint::UVelocity( UPoint& result ) const
{

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  if ( ! IsDefined() )
    result.SetDefined( false );
  else
    {
      x0 = p0.GetX();
      y0 = p0.GetY();
  
      x1 = p1.GetX();
      y1 = p1.GetY();
  
      result.timeInterval = timeInterval;
      
      if (result.IsDefined() )
	{
	  inf = result.timeInterval.start;
	  sup = result.timeInterval.end;
	  
	  t0 = inf.ToDouble();
	  t1 = sup.ToDouble();

	  duration = (t1 - t0)/1000;   // value in second

	  Interval<Instant> iv(result.timeInterval.start,
			       result.timeInterval.end,
			       result.timeInterval.lc,
			       result.timeInterval.rc);

	  UPoint p(iv,(x1-x0)/duration,0,(y1-y0)/duration,0);
	  
	  result.CopyFrom( &p );
	  
	}
    }
}

/*
3.6 Operator ~Trajectory~

This function is introduced as an additional memberfunction of class
~UPoint~:

*/

void UPoint::UTrajectory( Line& line ) const
{

  line.Clear();

  if ( ! IsDefined() )
    line.SetDefined( false );
  else 
    {
      
      line.StartBulkLoad();
      
      HalfSegment hs;
      
      if( !AlmostEqual( p0, p1 ) )
	{
	  
	  hs.Set( true, p0, p1 );

	  line += hs;
	  hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
	  line += hs;
	}
      
      line.EndBulkLoad();
    }
}



/*
4 General Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function within the operator's ~ValueMapping~ array, being able to deal 
with the respective combination of input parameter types.

Non-overloaded opertors (having only one single signature) do not need a
selection function or a ValueMapping array.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

Only some more general selection functions are listed here. More of them can
be found in the operator implementation section below.

4.1 Selection function ~UnitSimpleSelect~

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

4.2 Selection function ~UnitCombinedUnitStreamSelect~

This extended version of ~UnitSimpleSelect~ can map (unit) as well as 
(stream unit):

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
5 Implementation of Algebra Operators

5.1 Operator ~speed~

5.1.1 Type mapping function for ~speed~

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
5.1.2 Value mapping for operator ~speed~

*/

int MPointSpeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  MPoint* input = (MPoint*)result.addr;
  
  if ( input->IsDefined() )
    // call member function:
    ((MPoint*)args[0].addr)->MSpeed( *((MReal*)result.addr) ); 
  else
    ((MPoint*)args[0].addr)->SetDefined(false);

  return 0;
}

int UnitPointSpeed(Word* args,Word& result,int message,Word& local,Supplier s)
{
  result = qp->ResultStorage( s );
  UPoint* input = (UPoint*)result.addr;
  
  if ( input->IsDefined() )
    // call member function:
    ((UPoint*)args[0].addr)->USpeed(  *((UReal*)result.addr) );
  else
     ((UPoint*)args[0].addr)->SetDefined(false);
  return 0;
}

/* 
5.1.3 Specification for operator ~speed~

*/

const string
TemporalSpecSpeed  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(mpoint/upoint) -> mreal/ureal</text--->"
"<text>speed( _ )</text--->"
"<text>return the speed of a spatial object.</text--->"
"<text>query speed(mp1)</text---> ) )";

/* 
5.1.4 Selection Function of operator ~speed~

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

ValueMapping temporalspeedmap[] = { MPointSpeed,
                                    UnitPointSpeed };

/* 
5.1.5 Definition of operator ~speed~

*/

Operator temporalspeed( "speed",
                      TemporalSpecSpeed,
                      2,
                      temporalspeedmap,
                      SpeedSelect,
                      TypeMapSpeed);

/*
5.2 Operator ~queryrect2d~

5.2.1 Type Mapping for ~queryresct2d~

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
5.2.2 Value Mapping for ~queryrect2d~

*/

int Queryrect2d(Word* args, Word& result, int message, Word& local, Supplier s)
{
  const unsigned dim = 2;
  double min[dim], max[dim];
  double timevalue;
  double mininst, maxinst;
  Instant* Inv = (Instant*)args[0].addr;
  DateTime tmpinst;

  tmpinst.ToMinimum(); 
  mininst = tmpinst.ToDouble();
  tmpinst.ToMaximum();
  maxinst = tmpinst.ToDouble();

  result = qp->ResultStorage( s );
  
  if (Inv->IsDefined()) // Inv is defined: create rect2
    {
      timevalue = Inv->ToDouble();
      
      min[0] = mininst;   // x1
      min[1] = timevalue; // x2
      max[0] = timevalue; // y1
      max[1] = maxinst;   // y2
     
      Rectangle<dim>* rect = new Rectangle<dim>(true, min, max);
      ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
      delete rect;
    }
  else // Inv is undefined: return mininst^4
    {
      min[0] = mininst;
      min[1] = mininst;
      max[0] = mininst;
      max[1] = mininst;
    
      Rectangle<dim>* rect = new Rectangle<dim>(false, min, max);
      ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
      delete rect;
    }
  
  return 0;
}

/*
5.2.3 Specification for operator ~queryrect2d~

*/

const string
TemporalSpecQueryrect2d  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(instant) -> rect</text--->"
"<text>queryrect2d( _ )</text--->"
"<text>return the rect of an instant object for a time interval.</text--->"
"<text>query queryrect2d(instant)</text---> ) )";

/* 
5.2.4 Selection Function of operator ~queryrect2d~

Not necessary.

*/

/*
5.2.5  Definition of operator ~queryrect2d~

*/

Operator temporalunitqueryrect2d( "queryrect2d",
                      TemporalSpecQueryrect2d,
                      Queryrect2d,
                      Operator::SimpleSelect,
                      InstantTypeMapQueryrect2d);

/*
5.3 Operator ~point2d~

5.3.1 Type Mapping for ~point2d~

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
5.3.2 Value Mapping for ~point2d~

*/
int Point2d( Word* args, Word& result, int message, Word& local, Supplier s )
{
  double  X, Y;
  Instant sup, inf;

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

  if ( !range->IsDefined() )
    ((Point*)result.addr)->SetDefined( false );

  else 
    {
      X = 0; 
      Y = 0;
      if( !range->IsEmpty()  )
	{
	  const Interval<Instant> *intv1, *intv2;
	  
	  range->Get( 0, intv1 );
	  range->Get( range->GetNoComponents()-1, intv2 );
	  
	  Interval<Instant> timeInterval(intv1->start,intv2->end,
					 intv1->lc,intv2->rc);
	  
	  sup = timeInterval.end;
	  inf = timeInterval.start;
	  Y = sup.ToDouble(); // Derives the maximum of all intervals.
	  X = inf.ToDouble(); // Derives the minimum of all intervals.
	}
      ((Point*)result.addr)->Set(X,Y ); // Returns the calculated point.
    }
  
  return 0;
}


/*
5.3.3 Specification for operator ~point2d~

*/
const string
TemporalSpecPoint2d  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(periods) -> point</text--->"
"<text>point2d( _ )</text--->"
"<text>return the point of a given interval.</text--->"
"<text>query point2d(periods)</text---> ) )";


/* 
5.3.4 Selection Function of operator ~point2d~

Not necessary.

*/

/*
5.3.5  Definition of operator ~point2d~

*/
Operator temporalunitpoint2d( "point2d",
                      TemporalSpecPoint2d,
                      Point2d,
                      Operator::SimpleSelect,
                      PeriodsTypeMapPoint2d);

/*
5.4 Operator ~size~

5.4.1 Type Mapping for ~size~

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
5.4.2 Value Mapping for ~size~

*/
int Size( Word* args, Word& result, int message, Word& local, Supplier s )
{
  double res, duration, intervalue_sup, intervalue_inf;
  Instant sup, inf;

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

  if ( !range->IsDefined() )
    ((CcReal*)result.addr)->SetDefined( false );
  else 
    {
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
	      // summarize all time intervals in seconds:
	      duration += (intervalue_sup - intervalue_inf)/1000;
	    }
	  res = duration;
	}
      ((CcReal*)result.addr)->Set(true, res);  // return the resulT
    }
  return 0;
}

/*
5.4.3 Specification for operator ~size~

*/
const string
TemporalSpecSize  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(periods) -> real</text--->"
"<text>size( _ )</text--->"
"<text>return the duration of a moving object.</text--->"
"<text>query size(periods)</text---> ) )";

/* 
5.4.4 Selection Function of operator ~size~

Not necessary.

*/


/*
5.4.5  Definition of operator ~size~

*/
Operator temporalunitsize( "size",
                      TemporalSpecSize,
                      Size,
                      Operator::SimpleSelect,
                      PeriodsTypeMapSize );

/*
5.5 Operator ~makemvalue~

5.5.1 Type Mapping for ~makemvalue~

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


  //check the list length.
  CHECK_COND(nl->ListLength(args) == 2,
	     "Operator makemvalue expects a list of length two.");
  
  first = nl->First(args);
  nl->WriteToString(argstr, first);


  // check the structure of the list.
  CHECK_COND(nl->ListLength(first) == 2  &&
	     (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	     (nl->ListLength(nl->Second(first)) == 2) &&
	     (IsTupleDescription(nl->Second(nl->Second(first)))),
  "Operator makemvalue expects as first argument a list with structure "
  "(stream (tuple ((a1 t1)...(an tn))))\n"
  "Operator makemvalue gets as first argument '" + argstr + "'." );


  // check the given parameter
  second  = nl->Second(args);
  nl->WriteToString(argstr, second);
  CHECK_COND(argstr != "typeerror",
  "Operator makemvalue expects a name of a attribute and not a type.");

  // inputname represented the name of the given attribute
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

  // compare with the attributes in the relation
  if (attrname == inputname)
     inputtype = argstr2;

  // save from the detected attribute the type
  while (!(nl->IsEmpty(rest)))
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      firstr = nl->First(rest);
      rest = nl->Rest(rest);
      first2 = nl->First(firstr);
      second2 = nl->Second(firstr);
      nl->WriteToString(attrname, first2);
      nl->WriteToString(argstr2, second2);

      // compare with the attributes in the relation
      if (attrname == inputname)
	inputtype = argstr2;

      // save from the detected attribute the type
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
  
  // Appending the number of the attribute in the relation is very important,
  // because in the other case we can't work with it in the value function.
}

/*
5.5.2 Value Mapping for ~makemvalue~

*/

template <class Mapping, class Unit>
int MappingMakemvalue(Word* args,Word& result,int message,
                      Word& local,Supplier s)
{
  Mapping* m;
  Unit* unit;

  Word currentTupleWord;

  assert(args[2].addr != 0); // assert existence of input
  assert(args[3].addr != 0); // assert existence of input

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  result = qp->ResultStorage(s);

/*
  ~Mapping~ is a template function for many datatypes.
  The mapping class can cast moving datatypes.
  The unit class can cast  unit datatypes.

*/
  
  m = (Mapping*) result.addr;
  m->Clear();
  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) ) // get all tuples
    {
      Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
      Attribute* currentAttr = (Attribute*)currentTuple->
	GetAttribute(attributeIndex);
      
      if(currentAttr->IsDefined())
	{
	  unit = (Unit*) currentAttr;
	  m->Add( *unit );
	  currentTuple->DeleteIfAllowed();
	}
      qp->Request(args[0].addr, currentTupleWord);
    }
  m->EndBulkLoad( false );
  
  qp->Close(args[0].addr);

  return 0;

}

/*
5.5.3 Specification for operator ~makemvalue~

*/
const string
TemporalSpecMakemvalue  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>((stream (tuple ((x1 t1)...(xn tn)))"
" (unit(xi))))-> moving(x)</text--->"
"<text> makemvalue[ _ ]</text--->"
"<text>get the moving value of the stream of units.</text--->"
"<text>makemvalue[ u1 ]</text---> ) )";

/* 
5.5.4 Selection Function of operator ~makemvalue~

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

ValueMapping temporalmakemvaluemap[] = { MappingMakemvalue<MBool, UBool>,
                                         MappingMakemvalue<MBool, UBool>,
                                         MappingMakemvalue<MReal, UReal>,
                                         MappingMakemvalue<MPoint, UPoint> };

/*
5.5.5  Definition of operator ~makemvalue~

*/
Operator temporalunitmakemvalue( "makemvalue",
                        TemporalSpecMakemvalue,
                        4,
                        temporalmakemvaluemap,
                        MakemvalueSelect,
                        MovingTypeMapMakemvalue );

/*
5.6 Operator ~trajectory~

5.6.1 Type Mapping for ~trajectory~

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
5.6.2 Value Mapping for ~trajectory~

*/
int UnitPointTrajectory(Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Line *line = ((Line*)result.addr);
  UPoint *upoint = ((UPoint*)args[0].addr);

  if ( !upoint->IsDefined() )
    line->SetDefined( false );
  else 
    upoint->UTrajectory( *line );   // call memberfunction
  
  return 0;
}

/*
5.6.3 Specification for operator ~trajectory~

*/
const string
TemporalSpecTrajectory  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>upoint -> line</text--->"
"<text> trajectory( _ )</text--->"
"<text>get the trajectory of the corresponding"
"unit point object.</text--->"
"<text>trajectory( up1 )</text---> ) )";

/* 
5.6.4 Selection Function of operator ~trajectory~

NOt necessary.

*/

/*
5.6.5  Definition of operator ~trajectory~

*/
Operator temporalunittrajectory( "trajectory",
                          TemporalSpecTrajectory,
                          UnitPointTrajectory,
                          Operator::SimpleSelect,
                          UnitTrajectoryTypeMap );

/*
5.7 Operator ~deftime~

5.7.1 Type Mapping for ~deftime~

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
5.7.2 Value Mapping for ~deftime~

*/
template <class Mapping>
int MappingUnitDefTime( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* r = ((Periods*)result.addr);
  Mapping* m = ((Mapping*)args[0].addr);

  if ( !m->IsDefined() )
    r->SetDefined( false );
  else
    {
      assert( r->IsOrdered() );
  
      r->Clear();
      r->StartBulkLoad();
      r->Add( m->timeInterval );
      r->EndBulkLoad( false );
    }
  return 0;
}

/*
5.7.3 Specification for operator ~deftime~

*/
const string
TemporalSpecDefTime  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>unit(x) -> periods</text--->"
"<text> deftime( _ )</text--->"
"<text>get the defined time of the corresponding"
" unit data objects.</text--->"
"<text>deftime( up1 )</text---> ) )";

/* 
5.7.4 Selection Function of operator ~deftime~

Uses ~UnitSimpleSelect~.

*/

ValueMapping temporalunitdeftimemap[] = { MappingUnitDefTime<UBool>,
                                          MappingUnitDefTime<UInt>,
                                          MappingUnitDefTime<UReal>,
                                          MappingUnitDefTime<UPoint> };


/*
5.7.5  Definition of operator ~deftime~

*/

Operator temporalunitdeftime( "deftime",
                          TemporalSpecDefTime,
                          4,
                          temporalunitdeftimemap,
                          UnitSimpleSelect,
                          UnitTypeMapPeriods );
/*
5.8 Operator ~atinstant~

5.8.1 Type Mapping for ~atinstant~

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
5.8.2 Value Mapping for ~atinstant~

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

  if ( !t->IsDefined() || !posUnit->IsDefined() ) 
    pResult->SetDefined( false );

  else if( posUnit->timeInterval.Contains(t1) )
    {
      pResult->SetDefined( true );
      posUnit->TemporalFunction( *t, pResult->value );
    }
  else    // instant not contained by deftime interval
    pResult->SetDefined( false );
  
  return 0;
}

/*
5.8.3 Specification for operator ~atinstant~

*/
const string
TemporalSpecAtInstant  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(unit(x) instant) -> intime(x)</text--->"
"<text>_ atinstant _ </text--->"
"<text>get the Intime value corresponding to the "
"instant.</text--->"
"<text>upoint1 atinstant instant1</text---> ) )";

/* 
5.8.4 Selection Function of operator ~atinstant~

Uses ~UnitSimpleSelect~.

*/



ValueMapping temporalunitatinstantmap[] ={MappingUnitAtInstant<UBool, CcBool>,
                                          MappingUnitAtInstant<UInt, CcInt>,
                                          MappingUnitAtInstant<UReal, CcReal>,
                                          MappingUnitAtInstant<UPoint, Point>,};


/*
5.8.5  Definition of operator ~atinstant~

*/
Operator temporalunitatinstant( "atinstant",
                            TemporalSpecAtInstant,
                            4,
                            temporalunitatinstantmap,
                            UnitSimpleSelect,
                            UnitInstantTypeMapIntime );

/*
5.9 Operator ~atperiods~

5.9.1 Type Mapping for ~atperiods~

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
5.9.2 Value Mapping for ~atperiods~

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
  // should not happen:
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
5.9.3 Specification for operator ~atperiods~

*/
const string
TemporalSpecAtPeriods  =
"( ( \"Algebra\" \"Signature\" \"Signature (1)\" \"Signature (2)\" \"Syntax\""
" \"Meaning\" \"Example (1)\" \"Example (2)\") "
"( <text>TemporalUnitAlgebra</text--->"
"<text>For T in {int, bool, real, point}:</text--->"
"<text>(unit(T) periods) -> stream uT\n</text--->"
"<text>((stream uT) periods) -> stream uT</text--->"
"<text>_ atperiods _ </text--->"
"<text>restrict the movement to the given"
" periods.</text--->"
"<text>upoint1 atperiods thehour(2003,11,11,8)</text--->"
"<text>ufeed(upoint1) atperiods thehour(2003,11,11,8)</text--->) )";

/* 
5.9.4 Selection Function of operator ~atperiods~

Uses ~UnitCombinedUnitStreamSelect~.

*/

ValueMapping temporalunitatperiodsmap[] = { MappingUnitAtPeriods<UBool>,
                                            MappingUnitAtPeriods<UInt>,
                                            MappingUnitAtPeriods<UReal>,
                                            MappingUnitAtPeriods<UPoint>,
                                            MappingUnitStreamAtPeriods<UBool>,
                                            MappingUnitStreamAtPeriods<UInt>,
                                            MappingUnitStreamAtPeriods<UReal>,
                                            MappingUnitStreamAtPeriods<UPoint>  
                                          };

/*
5.9.5  Definition of operator ~atperiods~

*/
Operator temporalunitatperiods( "atperiods",
                            TemporalSpecAtPeriods,
                            8,
                            temporalunitatperiodsmap,
                            UnitCombinedUnitStreamSelect,
                            UnitPeriodsTypeMap );

/*
5.10 Operators ~initial~ and ~final~

5.10.1 Type Mapping for ~initial~ and ~final~

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
5.10.2 Value Mapping for ~initial~ and ~final~

*/
template <class Unit, class Alpha>
int MappingUnitInitial( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit* unit = ((Unit*)args[0].addr);
  Intime<Alpha>* res = ((Intime<Alpha>*)result.addr);


  if( !unit->IsDefined() || !(unit->timeInterval.start.IsDefined()) )
     res->SetDefined( false );
  else
   {
    res->SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.start, res->value );
    res->instant.CopyFrom( &unit->timeInterval.start );
    }
  return 0;
}

template <class Unit, class Alpha>
int MappingUnitFinal( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit* unit = ((Unit*)args[0].addr);
  Intime<Alpha>* res = ((Intime<Alpha>*)result.addr);

  if( !unit->IsDefined() || !(unit->timeInterval.end.IsDefined()) )
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
5.10.3 Specification for operator ~initial~ and ~final~

*/
const string
TemporalSpecInitial  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>unit(x) -> intime(x)</text--->"
"<text> initial( _ )</text--->"
"<text>get the intime value corresponding"
" to the initial instant.</text--->"
"<text>initial( upoint1 )</text---> ) )";

const string
TemporalSpecFinal  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>unit(x) -> intime(x)</text--->"
"<text> final( _ )</text--->"
"<text>get the intime value corresponding"
" to the final instant.</text--->"
"<text>final( upoint1 )</text---> ) )";

/* 
5.10.4 Selection Function of operator ~initial~ and ~final~

Use ~UnitSimpleSelect~ / ~UnitCombinedUnitStreamSelect~.

*/

ValueMapping temporalunitinitialmap[] = { MappingUnitInitial<UBool, CcBool>,
                                          MappingUnitInitial<UInt, CcInt>,
                                          MappingUnitInitial<UReal, CcReal>,
                                          MappingUnitInitial<UPoint, Point> };

ValueMapping temporalunitfinalmap[] = {  MappingUnitFinal<UBool, CcBool>,
                                         MappingUnitFinal<UInt, CcInt>,
                                         MappingUnitFinal<UReal, CcReal>,
                                         MappingUnitFinal<UPoint, Point> };

/*
5.10.5  Definition of operator ~initial~ and ~final~

*/
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

/*
5.11 Operator ~present~

5.11.1 Type Mapping for ~present~

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
5.11.2 Value Mapping for ~present~

*/
template <class Mapping>
int MappingUnitPresent_i( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);
  Instant t1 = *inst;

  if ( !inst->IsDefined() || !m->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );

  else if( m->timeInterval.Contains(t1) )
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
5.11.3 Specification for operator ~present~

*/
const string
TemporalSpecPresent  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(moving/unit(x) instant) -> bool,"
" (moving/unit(x) periods) -> bool</text--->"
"<text>_ present _ </text--->"
"<text>whether the object is present at the"
" given instant or period.</text--->"
"<text>mpoint1 present instant1</text---> ) )";

/* 
5.11.4 Selection Function of operator ~present~

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

ValueMapping temporalunitpresentmap[] = { MappingUnitPresent_i<UBool>,
                                          MappingUnitPresent_i<UInt>,
                                          MappingUnitPresent_i<UReal>,
                                          MappingUnitPresent_i<UPoint>,
                                          MappingUnitPresent_p<UBool>,
                                          MappingUnitPresent_p<UInt>,
                                          MappingUnitPresent_p<UReal>,
                                          MappingUnitPresent_p<UPoint> };

/*
5.11.5  Definition of operator ~present~

*/
Operator temporalunitpresent( "present",
                          TemporalSpecPresent,
                          8,
                          temporalunitpresentmap,
                          UnitInstantPeriodsSelect,
                          UnitInstantPeriodsTypeMapBool);

/*
5.12 Operator ~passes~

5.12.1 Type Mapping for ~passes~

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
5.12.2 Value Mapping for ~passes~

*/
template <class Mapping, class Alpha>
int MappingUnitPasses( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);

  if( !val->IsDefined() || !m->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Passes( *val ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  
  return 0;
}

/*
5.12.3 Specification for operator ~passes~

*/
const string
TemporalSpecPasses =
"( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(unit(T) T) -> bool</text--->"
"<text>for T in {bool, int, real, point} </text--->"
"<text>_ passes _ </text--->"
"<text>whether the object unit passes the given"
" value.</text--->"
"<text>upoint1 passes point1</text---> ) )";

/* 
5.12.4 Selection Function of operator ~passes~

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

ValueMapping temporalunitpassesmap[] = { MappingUnitPasses<UBool, CcBool>,
                                         MappingUnitPasses<UInt, CcInt>,
                                         MappingUnitPasses<UReal, CcReal>,
                                         MappingUnitPasses<UPoint, Point> };

/*
5.12.5  Definition of operator ~passes~

*/
Operator temporalunitpasses( "passes",
                         TemporalSpecPasses,
                         4,
                         temporalunitpassesmap,
                         UnitBaseSelect,
                         UnitBaseTypeMapBool);

/*
5.13 Operator ~at~

5.13.1 Type Mapping for ~at~

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
5.13.2 Value Mapping for ~at~

*/
template <class Unit, class Alpha>
int MappingUnitAt( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit* unit = ((Unit*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);

  Unit* pResult = ((Unit*)result.addr);

  if ( !unit->IsDefined() || !val->IsDefined() )
    pResult->SetDefined(false);
  else if (unit->At( *val, *pResult ))
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
5.13.3 Specification for operator ~at~

*/
const string
TemporalSpecAt =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(unit(x) x) -> unit(x)</text--->"
"<text> _ at _ </text--->"
"<text>restrict the movement at the times"
" where the equality occurs.</text--->"
"<text>upoint1 at point1</text---> ) )";

/* 
5.13.4 Selection Function of operator ~at~

Uses ~unitBaseSelect~.

*/

ValueMapping temporalunitatmap[] = {  MappingUnitAt< UBool, CcBool>,
                                      MappingUnitAt< UInt, CcInt>,
                                      MappingUnitAt< UReal, CcReal>,
                                      MappingUnitAt< UPoint, Point> };

/*
5.13.5  Definition of operator ~at~

*/
Operator temporalunitat( "at",
                     TemporalSpecAt,
                     4,
                     temporalunitatmap,
                     UnitBaseSelect,
                     UnitBaseTypeMapUnit );

/*
5.14 Operator ~circle~

5.14.1 Type Mapping for ~circle~

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
5.14.2 Value Mapping for ~circle~

*/
int Circle( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Point* p = (Point*)args[0].addr; // Centre of the circle
  CcReal* r = (CcReal*)args[1].addr; // Radius of the circle.
  CcInt* narg = (CcInt*)args[2].addr; // number of edges
  Region *res = (Region*)result.addr;
  double x, y;
  int n;
  double radius;
  double valueX, valueY;
  double angle;
  int partnerno = 0;

  res->Clear();

  if (!p->IsDefined() || !r->IsDefined() || !narg->IsDefined() )
    {
      res->SetDefined( false );
    }
  else
    {
      x = p->GetX();
      y = p->GetY();
      
      n = narg->GetIntval();
      radius = r->GetRealval();
      
      res->StartBulkLoad();
      
      // Definition of an empty region:
      Region rg;
      HalfSegment hs;
      
      
      if (( p->IsDefined())&&(n>3)&&(n<100)&&(radius >0.0))
	{
	  
	  //  Determination of a n polygon.
	  //  Division of 360 degree in n parts with the help of
	  //  a standardised circle and the circumference. U = 2 * PI
	  
	  for( int i = 0; i < n; i++ )
	    {
	      angle = i * 2 * PI/n;
	      valueX = x + radius * cos(angle);
	      valueY = y + radius * sin(angle);
	      
	      //  The first point of the segment of a region.
	      //  The x-value can be defined with the cosine and
	      //  the y-value with the sine.
	      
	      Point edge1(true, valueX ,valueY);
	      
	      hs.attr.faceno = 0;
	      hs.attr.cycleno = 0;
	      hs.attr.edgeno = partnerno;
	      hs.attr.partnerno = partnerno++;
	      
	      if ((i+1) >= n)
		angle = 0 * 2 * PI/n;
	      else
		angle = (i+1) * 2 * PI/n;
	      
	      valueX = x + radius * cos(angle);
	      valueY = y + radius * sin(angle);
	      // The second point of the segment of a region.
	      Point edge2(true, valueX ,valueY);
	      
	      // Definition of the halfsegments.
	      hs.Set(true, edge1, edge2);
	      *res += hs;
	      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
	      *res += hs;	  
	    }      
	}
      res->EndBulkLoad(true);
      res->SetPartnerNo();
      res->ComputeRegion();
    }
  return 0;
}

/*
5.14.3 Specification for operator ~circle~

*/
const string
TemporalSpecCircle =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>point x real x int -> region</text--->"
"<text> circle ( _ ) </text--->"
"<text>defines a circle with a given radius"
" and n calculated points.</text--->"
"<text>circle (p,10.0,10)</text---> ) )";

/* 
5.14.4 Selection Function of operator ~circle~

Not necessary.

*/

/*
5.14.5  Definition of operator ~circle~

*/
Operator temporalcircle( "circle",
                      TemporalSpecCircle,
                      Circle,
                      Operator::SimpleSelect,
                      TypeMapCircle);

/*
5.15 Operator ~makepoint~

5.15.1 Type Mapping for ~makepoint~

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
5.15.2 Value Mapping for ~makepoint~

*/
int MakePoint( Word* args, Word& result, int message, Word& local, Supplier s )
{

  result = qp->ResultStorage( s );
  CcInt* value1 = (CcInt*)args[0].addr;
  CcInt* value2 = (CcInt*)args[1].addr;

  if ( !value1->IsDefined() || !value2->IsDefined() )
    ((Point*)result.addr)->SetDefined( false );
  else
    ((Point*)result.addr)->Set(value1->GetIntval(),value2->GetIntval() );

  return 0;
}

/*
5.15.3 Specification for operator ~makepoint~

*/
const string
TemporalSpecMakePoint =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>int x int -> point</text--->"
"<text> makepoint ( _ ) </text--->"
"<text>create a point with two"
" given values.</text--->"
"<text>makepoint (5,5)</text---> ) )";

/* 
5.15.4 Selection Function of operator ~makepoint~

Not necessary.

*/

/*
5.15.5  Definition of operator ~makepoint~

*/
Operator temporalmakepoint( "makepoint",
                      TemporalSpecMakePoint,
                      MakePoint,
                      Operator::SimpleSelect,
                      TypeMapMakepoint);

/*
5.16 Operator ~velocity~

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
5.16.2 Value Mapping for ~velocity~

*/
int MPointVelocity(Word* args, Word& result, int message,
                   Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  MPoint* input = (MPoint*)args[0].addr;
  
  if ( !input->IsDefined() )
    ((MPoint*)args[0].addr)->SetDefined( false );
  else 
    ((MPoint*)args[0].addr)->MVelocity(  *((MPoint*)result.addr) );

  return 0;
}

int UnitPointVelocity(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  UPoint* input = (UPoint*)args[0].addr;

  if ( !input->IsDefined() )
    ((UPoint*)args[0].addr)->SetDefined( false );
  else
    ((UPoint*)args[0].addr)->UVelocity(  *((UPoint*)result.addr) );

  return 0;
}

/*
5.16.3 Specification for operator ~velocity~

*/
const string
TemporalSpecVelocity=
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>mpoint/upoint -> mpoint/upoint</text--->"
"<text> velocity ( _ ) </text--->"
"<text>describes the vector of the speed"
" to the given object.</text--->"
"<text>velocity (mpoint)</text---> ) )";

/* 
5.16.4 Selection Function of operator ~velocity~

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

ValueMapping temporalvelocitymap[] = { MPointVelocity,
                                       UnitPointVelocity };

/*
5.16.5  Definition of operator ~velocity~

*/
Operator temporalvelocity( "velocity",
                      TemporalSpecVelocity,
                      2,
                      temporalvelocitymap,
                      VelocitySelect,
                      TypeMapVelocity);

/*
5.17 Operator ~derivable~

5.17.1 Type Mapping for ~derivable~

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
5.17.2 Value Mapping for ~derivable~

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

  if ( !value->IsDefined() )
    res->SetDefined(false);
  else
    {
      res->StartBulkLoad();
      for( int i = 0; i < value->GetNoComponents(); i++ )
	{	  
	  value->Get( i, uReal ); // Load a real unit.

	  // FALSE means in this case that a real unit describes a quadratic
	  // polynomial. A derivation is possible and the operator returns TRUE.
	  if (uReal->r == false)
	    b.Set(true,true);
	  else
	    b.Set(true,false);
	  
	  UBool boolvalue (uReal->timeInterval,b);
	  res->Add( boolvalue );
	}
      res->EndBulkLoad( false );
    }
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
       b.Set(true,true);
     else
       b.Set(true,false);
     
     UBool boolvalue (uReal->timeInterval,b);
     res->CopyFrom(&boolvalue);
   }
 else
   res->SetDefined( false );

 return 0;
}

/*
5.17.3 Specification for operator ~derivable~

*/
const string
TemporalSpecDerivable=
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>mreal/ureal -> mbool/ubool</text--->"
"<text> derivable ( _ ) </text--->"
"<text>get the hint"
" for a mreal/ureal which part is derivable.</text--->"
"<text>derivable (mreal)</text---> ) )";

/* 
5.17.4 Selection Function of operator ~derivable~

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

ValueMapping temporalderivablemap[] = { MPointDerivable,
                                        UnitPointDerivable };

/*
5.17.5  Definition of operator ~derivable~

*/

Operator temporalderivable( "derivable",
                      TemporalSpecDerivable,
                      2,
                      temporalderivablemap,
                      DerivableSelect,
                      TypeMapDerivable);

/*
5.18 Operator ~derivative~

5.18.1 Type Mapping for ~derivative~

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
5.18.2 Value Mapping for ~derivative~

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
  if ( value->IsDefined() )
    {
      res->StartBulkLoad();
      
      for( int i = 0; i < value->GetNoComponents(); i++ )
	{ // load a real unit
	  value->Get( i, Unit );
	  
	  // FALSE means in this case that a real unit describes a quadratic
	  // polynomial. A derivation is possible. 
	  // The polynom looks like at^2 + bt + c.
	  // The derivative of this polynom is 2at + b.
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
	      // derivation of the real unit is not possible
	      uReal.SetDefined(false);
	    }
	  
	  res->Add( uReal );
	}
      res->EndBulkLoad( false );
    }
  else // value is undefined
    res->SetDefined( false );
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
  else // Unit is undefines
    res->SetDefined(false);
  return 0;
}

/*
5.18.3 Specification for operator ~derivative~

*/
const string
TemporalSpecDerivative=
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>mreal/ureal -> mreal/ureal</text--->"
"<text> derivative ( _ ) </text--->"
"<text>determination the derivative"
" of a mreal/ureal.</text--->"
"<text>derivable (mreal)</text---> ) )";

/* 
5.18.4 Selection Function of operator ~deriavtive~

Uses ~DerivableSelect~.

*/

ValueMapping temporalderivativemap[] = { MPointDerivative,
                                       UnitPointDerivative };

/*
5.18.5  Definition of operator ~derivative~

*/
Operator temporalderivative( "derivative",
                      TemporalSpecDerivative,
                      2,
                      temporalderivativemap,
                      DerivableSelect,
                      TypeMapDerivative);

/*
5.19 Operator ~ufeed~

The operator is used to cast a single unit(T) to a stream(unit(T))
having a single element unit(T).

5.19.1 Type Mapping for ~ufeed~

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
5.19.2 Value Mapping for ~ufeed~

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
5.19.3 Specification for operator ~ufeed~

*/
const string
TemporalSpecUfeed=
"( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>For T in {bool, int, real point} </text--->"
"<text>uT -> stream uT</text--->"
"<text>ufeed ( _ ) </text--->"
"<text>create a single-unit stream from "
"a single unit.</text--->"
"<text>ufeed (ureal)</text---> ) )";

/* 
5.19.4 Selection Function of operator ~ufeed~

Uses ~UnitSimpleSelect~.

*/

ValueMapping temporalunitfeedmap[] = { MappingUnitFeed<UBool>,
				       MappingUnitFeed<UInt>,
				       MappingUnitFeed<UReal>,
				       MappingUnitFeed<UPoint>};

/*
5.19.5  Definition of operator ~ufeed~

*/
Operator temporalufeed( "ufeed",
                      TemporalSpecUfeed,
                      4,
                      temporalunitfeedmap,
                      UnitSimpleSelect,
                      TypeMapUfeed);


/*
5.20 Operator ~suse~

The ~suse~ class of operators implements a set of functors, that derive
stream-valued operators from operators taking scalar arguments and returning
scalar values or streams of values:

----

     suse: (stream X)            (map X Y)            -> (stream Y)
           (stream X)            (map X (stream Y))   -> (stream Y)
           (stream X) Y          (map X Y Z)          -> (stream Z)
           (stream X) Y          (map X Y stream(Z))  -> (stream Z)
           X          (stream Y) (map X y Z)          -> (stream Z)
           X          (stream Y) (map X y (stream Z)) -> (stream Z)
           (stream X) (stream Y) (map X Y Z)          -> (stream Z)
           (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
           for X,Y,Z of kind DATA

----

5.20.1 Type Mapping for ~suse~

*/

ListExpr 
TypeMapSuse( ListExpr args )
{
  string outstr1, outstr2;            // output strings
  ListExpr errorInfo;
  ListExpr sarg1, map;                // arguments to suse
  ListExpr marg1, mres;               // argument to mapping
  ListExpr sarg1Type, sresType;       // 'flat' arg type 

  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( (nl->ListLength( args ) != 2) )
    {
      ErrorReporter::ReportError("Operator suse expects a list of length two ");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // get suse arguments
  sarg1 = nl->First( args );
  map = nl->Second( args ); 
  
  // check sarg1 for being a stream
  if(     nl->IsAtom( sarg1 )
       || ( nl->ListLength( sarg1 ) != 2) 
       || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    {
      ErrorReporter::ReportError(
	"Operator suse expects its first Argument to "
	"be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );	      
    }
  sarg1Type = nl->Second(sarg1);
  
  // check sarg1 to be a (stream T) for T in kind DATA 
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type ) 
      && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    {
      nl->WriteToString(outstr1, sarg1Type);      
      ErrorReporter::ReportError("Operator suse expects its 1st argument "
				 "to be '(stream T)', T of kind DATA, but"
				 "receives '" + outstr1 + "' as T.");

      return nl->SymbolAtom( "typeerror" );      
    }
  
  if ( !nl->IsAtom( sarg1Type ) &&
       ( (nl->ListLength( sarg1Type ) != 2) ||
	 !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
	 !IsTupleDescription(nl->Second(sarg1Type))
	 ) 
       )
    {
      nl->WriteToString(outstr1, sarg1);
      return nl->SymbolAtom( "typeerror" );      
    }
  
  // check for map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator suse expects a map as "
				 "2nd argument, but gets '" + outstr1 +
				 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }    

  if ( nl->ListLength(map) != 3 )       
    {
      ErrorReporter::ReportError("Number of map arguments must be 1 "
				 "for operator suse.");
      return nl->SymbolAtom( "typeerror" );
    }

  // get map arguments
  marg1 = nl->Second(map);
  mres  = nl->Third(map);

  // check marg1

  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
      nl->WriteToString(outstr1, sarg1Type);
      nl->WriteToString(outstr2, marg1);
      ErrorReporter::ReportError("Operator suse: 1st argument's stream"
				 "type does not match the type of the "
				 "mapping's 1st argument. If e.g. the first "
				 "is 'stream X', then the latter must be 'X'."
				 "The types passed are '" + outstr1 + 
				 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  // get map result type 'sresType'
  if( !( nl->IsAtom( mres ) ) && ( nl->ListLength( mres ) == 2) ) 
    {
      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
	{
	  if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) )
	    {
	      ErrorReporter::ReportError(
		"Operator suse expects its 2nd Argument to "
		"return a '(stream T)', T of kind DATA'.");
	      return nl->SymbolAtom( "typeerror" );	      
	    }
	  sresType = mres; // map result type is already a stream
	  nl->WriteToString(outstr1, sresType);
	  cout << "\nTypeMapSuse Resulttype (1): " << outstr1 << "\n";
	  return sresType;
	}
    }
  else // map result type is not a stream, so encapsulate it
    {
      if ( !am->CheckKind("DATA", mres, errorInfo) )
	{
	  ErrorReporter::ReportError(
	    "Operator suse expects its 2nd Argument to "
	    "return a type of kind DATA.");
	  return nl->SymbolAtom( "typeerror" );	      
	}
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);  
      nl->WriteToString(outstr1, sresType);
      cout << "\nTypeMapSuse Resulttype (2): " << outstr1 << "\n";
      return sresType;
    }
  
  // otherwise (some unmatched error)
  return nl->SymbolAtom( "typeerror" );
}


ListExpr 
TypeMapSuse2( ListExpr args )
{
  string   outstr1, outstr2;               // output strings
  ListExpr errorInfo;
  ListExpr sarg1, sarg2, map;              // arguments to suse
  ListExpr marg1, marg2, mres;             // argument to mapping
  ListExpr sarg1Type, sarg2Type, sresType; // 'flat' arg type 
  ListExpr argConfDescriptor;
  bool 
    sarg1isstream = false, 
    sarg2isstream = false,
    resisstream   = false;
  int argConfCode = 0;


  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  // 0. Check number of arguments
  if ( (nl->ListLength( args ) != 3) )
    {
      ErrorReporter::ReportError("Operator suse2 expects a list of "
				 "length three ");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // 1. get suse arguments
  sarg1 = nl->First( args );
  sarg2 = nl->Second( args );
  map   = nl->Third( args ); 
  
  // 2. First argument
  // check sarg1 for being a stream
  if( nl->IsAtom( sarg1 )
      || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    { // non-stream datatype
      sarg1Type = sarg1;
      sarg1isstream = false;
    }
  else if ( !nl->IsAtom( sarg1 )
	    && ( nl->ListLength( sarg1 ) == 2) 
	    && (TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    { // (stream datatype)
      sarg1Type = nl->Second(sarg1);
      sarg1isstream = true;
    }
  else // wrong type for sarg1
    {
      ErrorReporter::ReportError(
	"Operator suse2 expects its first Argument to "
	"be of type 'T' or '(stream T).");
      return nl->SymbolAtom( "typeerror" );	      
    }
     
  // check sarg1 to be a (stream T) for T in kind DATA 
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type ) 
      && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    { // T is not of kind DATA
      nl->WriteToString(outstr1, sarg1Type);      
      ErrorReporter::ReportError("Operator suse2 expects its 1st argument "
				 "to be '(stream T)', T of kind DATA, but"
				 "receives '" + outstr1 + "' as T.");

      return nl->SymbolAtom( "typeerror" );      
    }
  else if ( !nl->IsAtom( sarg1Type ) &&
	    ( (nl->ListLength( sarg1Type ) != 2) ||
	      !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
	      !IsTupleDescription(nl->Second(sarg1Type))
	      ) 
	    )
    { // neither T is tuple(X)
      nl->WriteToString(outstr1, sarg1Type);
      ErrorReporter::ReportError("Operator suse2 expects its 1st argument "
				 "to be 'T' or '(stream T), T of kind DATA "
				 "or of type 'tuple(X))', but"
				 "receives '" + outstr1 + "' for T.");
      return nl->SymbolAtom( "typeerror" );      
    }
  
  // 3. Second Argument
  // check sarg2 for being a stream
  if( nl->IsAtom( sarg2 )
      || !(TypeOfRelAlgSymbol(nl->First(sarg2) == stream )) )
    { // non-stream datatype
      sarg2Type = sarg2;
      sarg2isstream = false;
    }
  else if ( !nl->IsAtom( sarg2 )
	    && ( nl->ListLength( sarg2 ) == 2) 
	    && (TypeOfRelAlgSymbol(nl->First(sarg2) == stream )) )
    { // (stream datatype)
      sarg2Type = nl->Second(sarg2);
      sarg2isstream = true;
    }
  else // wrong type for sarg2
    {
      ErrorReporter::ReportError(
	"Operator suse2 expects its second Argument to "
	"be of type 'T' or '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );	      
    }
     
  // check sarg2 to be a (stream T) for T in kind DATA 
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg2Type ) 
      && !am->CheckKind("DATA", nl->Second( sarg2Type ), errorInfo) )
    { // T is not of kind DATA
      nl->WriteToString(outstr2, sarg2Type);      
      ErrorReporter::ReportError("Operator suse2 expects its 2nd argument "
				 "to be '(stream T)', T of kind DATA, but"
				 "receives '" + outstr1 + "' as T.");

      return nl->SymbolAtom( "typeerror" );      
    }
  else if ( !nl->IsAtom( sarg2Type ) &&
	    ( (nl->ListLength( sarg2Type ) != 2) ||
	      !nl->IsEqual( nl->First(sarg2Type), "tuple") ||
	      !IsTupleDescription(nl->Second(sarg2Type))
	      ) 
	    )
    { // neither T is tuple(X)
      nl->WriteToString(outstr1, sarg2Type);
      ErrorReporter::ReportError("Operator suse2 expects its 2nd argument "
				 "to be 'T' or '(stream T), T of kind DATA "
				 "or of type 'tuple(X))', but"
				 "receives '" + outstr1 + "' for T.");
      return nl->SymbolAtom( "typeerror" );      
    }
  
  // 4. First and Second argument
  // check whether at least one stream argument is present
  if ( !sarg1isstream && !sarg2isstream )
    {
      ErrorReporter::ReportError(
	"Operator suse2 expects at least one of its both first "
	"argument to be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );	      
    }

  // 5. Third argument
  // check third for being a map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator suse2 expects a map as "
				 "3rd argument, but gets '" + outstr1 +
				 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }    

  if ( nl->ListLength(map) != 4 )       
    {
      ErrorReporter::ReportError("Number of map arguments must be 2 "
				 "for operator suse2.");
      return nl->SymbolAtom( "typeerror" );
    }

  // get map arguments
  marg1 = nl->Second(map);
  marg2 = nl->Third(map);
  mres  = nl->Fourth(map);

  // check marg1

  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
      nl->WriteToString(outstr1, sarg1Type);
      nl->WriteToString(outstr2, marg1);
      ErrorReporter::ReportError("Operator suse2: 1st argument's stream"
				 "type does not match the type of the "
				 "mapping's 1st argument. If e.g. the first "
				 "is 'stream X', then the latter must be 'X'."
				 "The types passed are '" + outstr1 + 
				 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  // check marg2
  if ( !( nl->Equal(marg2, sarg2Type) ) )
    {
      nl->WriteToString(outstr1, sarg2Type);
      nl->WriteToString(outstr2, marg2);
      ErrorReporter::ReportError("Operator suse2: 2nd argument's stream"
				 "type does not match the type of the "
				 "mapping's 2nd argument. If e.g. the second"
				 " is 'stream X', then the latter must be 'X'."
				 "The types passed are '" + outstr1 + 
				 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  // 6. Determine result type
  // get map result type 'sresType'
  if( !( nl->IsAtom( mres ) ) && ( nl->ListLength( mres ) == 2) ) 
    {
      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
	{
	  if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) &&
	       !( !nl->IsAtom(nl->Second(mres)) &&
		  nl->ListLength(nl->Second(mres)) == 2 &&
		  TypeOfRelAlgSymbol(nl->First(nl->Second(mres)) == tuple) &&
		  IsTupleDescription(nl->Second(nl->Second(mres)))
		) 
	     )
	    {
	      ErrorReporter::ReportError(
		"Operator suse2 expects its 3rd Argument to "
		"return a '(stream T)', T of kind DATA or T = 'tuple(X)'.");
	      return nl->SymbolAtom( "typeerror" );	      
	    }
	  resisstream = true;
	  sresType = mres; // map result type is already a stream
	}
    }
  else // map result type is not a stream, so encapsulate it
    {
      if ( !am->CheckKind("DATA", mres, errorInfo)  &&
	   !( !nl->IsAtom(nl->Second(mres)) &&
	      nl->ListLength(nl->Second(mres)) == 2 &&
	      TypeOfRelAlgSymbol(nl->First(nl->Second(mres)) == tuple) &&
	      IsTupleDescription(nl->Second(nl->Second(mres)))
	    )
	 ) 
	{
	  ErrorReporter::ReportError(
	    "Operator suse2 expects its 3rd Argument to "
	    "return a type of kind DATA or T = 'tuple(X)'.");
	  return nl->SymbolAtom( "typeerror" );	      
	}
      resisstream = false;
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);  
    }

  // 7. Append flags describing argument configuration for value mapping:
  //     0: no stream
  //     1: sarg1 is a stream
  //     2: sarg2 is a stream
  //     4: map result is a stream
  //
  //    e.g. 7=4+2+1: both arguments are streams and the 
  //                  map result is a stream

  if(sarg1isstream) argConfCode += 1;
  if(sarg2isstream) argConfCode += 2;
  if(resisstream)   argConfCode += 4;

  argConfDescriptor = nl->OneElemList(nl->IntAtom(argConfCode));
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
              argConfDescriptor, sresType);  
}

/*
5.20.2 Value Mapping for ~suse~

*/

struct SuseLocalInfo{
  bool Xfinished, Yfinished, funfinished; // whether we have finished
  Word X, Y, fun;                         // pointers to the argument nodes
  Word XVal, YVal, funVal;                // the last arg values
  int  argConfDescriptor;          // type of argument configuration
};

// (stream X) (map X Y) -> (stream Y)
int Suse_SN( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              instream = args[0], fun = args[1]; 
  Word              funResult, argValue;
  ArgVectorPointer  funArgs;

  switch (message)
    {
    case OPEN :
      
      // cout << "Suse_SN received OPEN" << endl;
      sli = new SuseLocalInfo;
      sli->Xfinished = true;
      qp->Open(instream.addr);
      sli->Xfinished = false;
      local = SetWord(sli);
      // cout << "Suse_SN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // For each REQUEST, we get one value from the stream,
      // pass it to the parameter function and evalute the latter.
      // The result is simply passed on.

      // cout << "Suse_SN received REQUEST" << endl;
      if( local.addr == 0 )
	{
	  // cout << "Suse_SN finished REQUEST: CANCEL (1)" << endl;  
	  return CANCEL;
	}
      sli = (SuseLocalInfo*)local.addr;
      
      if (sli->Xfinished)
	{
	  // cout << "Suse_SN finished REQUEST: CANCEL (2)" << endl;  
	  return CANCEL;
	}
      
      funResult.addr = 0;
      argValue.addr  = 0;
      qp->Request(instream.addr, argValue); // get one arg value from stream
      if(qp->Received(instream.addr))       
	{
	  funArgs = qp->Argument(fun.addr); // set argument for the
	  (*funArgs)[0] = argValue;         //   parameter function
	  qp->Request(fun.addr, funResult); // call parameter function
	  // copy result:
	  result = SetWord(((Attribute*) (funResult.addr))->Clone());
	  ((Attribute*) (argValue.addr))->DeleteIfAllowed(); // delete argument
	  //cout << "        result.addr    =" << result.addr << endl;
	  argValue.addr = 0;
	  // cout << "Suse_SN finished REQUEST: YIELD" << endl;
	  return YIELD;
	}
      else // (input stream consumed completely)
	{
	  qp->Close(instream.addr);
	  sli->Xfinished = true;
	  result.addr = 0;
	  // cout << "Suse_SN finished REQUEST: CANCEL (3)" << endl;  
	  return CANCEL;
	}
      
    case CLOSE :
      
      // cout << "Suse_SN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if ( !sli->Xfinished )
	    qp->Close( instream.addr );
	  delete sli;
	}
      // cout << "Suse_SN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "Suse_SN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}  
  

// (stream X) (map X (stream Y)) -> (stream Y)
int Suse_SS( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{
  SuseLocalInfo    *sli;
  Word             funResult;  
  ArgVectorPointer funargs;

  switch (message)
  {
  case OPEN :

    sli = new SuseLocalInfo;
    sli->X   = SetWord( args[0].addr );
    sli->fun = SetWord( args[1].addr );
    sli->Xfinished   = true;
    sli->funfinished = true;
    sli->XVal.addr   = 0;
    // open the ("outer") input stream and
    qp->Open( sli->X.addr );
    sli->Xfinished = false;
    // save the local information
    local = SetWord(sli);
    
    return 0;

  case REQUEST :
    
    // For each value from the 'outer' stream, an 'inner' stream
    // of values is generated by the parameter function.
    // For each REQUEST, we pass one value from the 'inner' stream
    // as the result value.
    // If the inner stream is consumed, we try to get a new value
    // from the 'outer' stream and re-open the inner stream 
    
    // cout << "\nSuse_SS: Received REQUEST";
    //1. recover local information
    if( local.addr == 0 )
      return CANCEL;
    sli = (SuseLocalInfo*)local.addr;
    
    // create the next result 
    while( !sli->Xfinished )
      {
	if( sli->funfinished )
	  {// end of map result stream reached -> get next X
	    qp->Request( sli->X.addr, sli->XVal);
	    if (!qp->Received( sli->X.addr ))
	      { // Stream X is exhaused
		qp->Close( sli->X.addr );
		sli->Xfinished = true;
		result.addr = 0;
		return CANCEL;
	      } // got an X-elem
	    funargs = qp->Argument( sli->fun.addr );
	    (*funargs)[0] = sli->XVal;
	    qp->Open( sli->fun.addr );
	    sli->funfinished = false;
	  } // Now, we have an open map result stream		
        qp->Request( sli->fun.addr, funResult );
	if(qp->Received( sli->fun.addr ))
	  { // cloning and passing the result
	    result = SetWord(((Attribute*) (funResult.addr))->Clone());
	    ((Attribute*) (funResult.addr))->DeleteIfAllowed(); 
	    // cout << "     result.addr=" << result.addr << endl;
	    return YIELD;      
	  }
	else 
	  { // end of map result stream reached
	    qp->Close( sli->fun.addr );
	    sli->funfinished = true;	    
	    ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();	    
	  }
      } // end while
  
  case CLOSE :

    if( local.addr != 0 )
      {
	sli = (SuseLocalInfo*)local.addr;     	  
	if( !sli->funfinished )
	  {
	    qp->Close( sli->fun.addr );
	    ((Attribute*)(sli->X.addr))->DeleteIfAllowed(); 	
	  }
	if ( !sli->Xfinished )
	  qp->Close( sli->X.addr );
	delete sli;
      }
    return 0;
  }  // end switch
  cout << "\nSuse_SS received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}  


// (stream X) Y          (map X Y Z) -> (stream Z)
// X          (stream Y) (map X y Z) -> (stream Z)
int Suse_SNN( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              xval, funresult;
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      // cout << "\nSuse_SNN received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;  
      sli->fun = SetWord(args[2].addr); 
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
	{ 
	  delete( sli );
	  local.addr = 0;
	  cout << "\nSuse_SNN was called with stream result mapping!" 
	       <<  endl;
	  return 0;
	}
      if(sli->argConfDescriptor & 1)
	{ // the first arg is the stream
	  sli->X = SetWord(args[0].addr); // X is the stream
	  sli->Y = SetWord(args[1].addr); // Y is the constant value
	} 
      else
	{ // the second arg is the stream
	  sli->X = SetWord(args[1].addr); // X is the stream
	  sli->Y = SetWord(args[0].addr); // Y is the constant value
	}
      
      qp->Open(sli->X.addr);              // open outer stream argument
      sli->Xfinished = false;
      qp->Request(sli->Y.addr, sli->Y);   // save value of constant argument

      local = SetWord(sli);
      // cout << "Suse_SNN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // For each REQUEST, we get one value from the stream,
      // pass it (and the remaining constant argument) to the parameter 
      // function and evalute the latter. The result is simply passed on.
      // sli->X is the stream, sli->Y the constant argument.

      //cout << "Suse_SNN received REQUEST" << endl;

      // 1. get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (1)" << endl;
	  return CANCEL;
	}
      sli = (SuseLocalInfo*) local.addr;
      if (sli->Xfinished)
	{ // stream already exhausted earlier
	  result.addr = 0;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (2)" << endl;
	  return CANCEL;
	}
      
      // 2. request value from outer stream
      qp->Request( sli->X.addr, xval );
      if(!qp->Received( sli->X.addr ))
	{ // stream exhausted now
	  qp->Close( sli->X.addr );
	  sli->Xfinished = true;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (3)" << endl;
	  return CANCEL;
	}
      
      // 3. call parameter function, delete args and return result
      funargs = qp->Argument( sli->fun.addr );
      if (sli->argConfDescriptor & 1)
	{
	  (*funargs)[0] = xval;
	  (*funargs)[1] = sli->Y;
	}
      else
	{
	  (*funargs)[0] = sli->Y;
	  (*funargs)[1] = xval;
	}
      qp->Request( sli->fun.addr, funresult );     
      result = SetWord(((Attribute*) (funresult.addr))->Clone());
      //cout << "     result.addr=" << result.addr << endl;
      ((Attribute*) (xval.addr))->DeleteIfAllowed(); 
      //cout << "Suse_SNN finished REQUEST: YIELD" << endl;
      return YIELD;

    case CLOSE :
      
      //cout << "Suse_SNN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if (!sli->Xfinished)
	    qp->Close( sli->X.addr ); // close input
	  delete sli;
	}
      //cout << "Suse_SNN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "\nSuse_SNN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}


// (stream X) Y          (map X Y (stream Z)) -> (stream Z)
// X          (stream Y) (map X y (stream Z)) -> (stream Z)
int Suse_SNS( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{

  SuseLocalInfo     *sli;
  Word              funresult;
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      // cout << "\nSuse_SNS received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;  
      sli->fun.addr = 0;
      sli->XVal.addr = 0;
      sli->YVal.addr = 0;
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
      if(! (sli->argConfDescriptor & 4))
	{ 
	  delete( sli );
	  local.addr = 0;
	  cout << "\nSuse_SNS was called with non-stream result mapping!" 
	       <<  endl;	  
	  return 0;
	}
      if(sli->argConfDescriptor & 1)
	{ // the first arg is the stream
	  sli->X = SetWord(args[0].addr); // X is the stream
	  sli->Y = SetWord(args[1].addr); // Y is the constant value
	} 
      else
	{ // the second arg is the stream
	  sli->X = SetWord(args[1].addr); // X is the stream
	  sli->Y = SetWord(args[0].addr); // Y is the constant value
	}
      qp->Request(sli->Y.addr, sli->YVal); // save value of constant argument
      qp->Open(sli->X.addr);               // open the ("outer") input stream
      sli->Xfinished = false;
      sli->fun = SetWord(args[2].addr);
      local = SetWord(sli);
      // cout << "Suse_SNN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // First, we check whether an inner stream is finished 
      // (sli->funfinished). If so, we try to get a value from 
      // the outer stream and try to re-open the inner stream.
      // sli->X is a pointer to the OUTER stream, 
      // sli->Y is a pointer to the constant argument.

      // cout << "Suse_SNN received REQUEST" << endl;

      // 1. get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (1)" << endl;
	  return CANCEL;
	}
      sli = (SuseLocalInfo*) local.addr;
      // 2. request values from inner stream
      while (!sli->Xfinished)
	{
	  while (sli->funfinished)
	    { // the inner stream is closed, try to (re-)open it
	      // try to get the next X-value from outer stream
	      qp->Request(sli->X.addr, sli->XVal);
	      if (!qp->Received(sli->X.addr))
		{ // stream X exhaused. CANCEL
		  sli->Xfinished = true;
		  // cout << "Suse_SNN finished REQUEST: CLOSE (3)" << endl;
		  return CANCEL;
		}
	      funargs = qp->Argument( sli->fun.addr );
	      if (sli->argConfDescriptor & 1)
		{
		  (*funargs)[0] = sli->XVal;
		  (*funargs)[1] = sli->YVal;	  
		}
	      else
		{
		  (*funargs)[0] = sli->YVal;
		  (*funargs)[1] = sli->XVal;	  
		}
	      qp->Open( sli->fun.addr );
	      sli->funfinished = false;
	    } // end while - Now, the inner stream is open again
	  qp->Request(sli->fun.addr, funresult);
	  if (qp->Received(sli->fun.addr))
	    { // inner stream returned a result
	      result = SetWord(((Attribute*) (funresult.addr))->Clone());
	      ((Attribute*) (funresult.addr))->DeleteIfAllowed(); 
	      // cout << "     result.addr=" << result.addr << endl;
	      // cout << "Suse_SNN finished REQUEST: YIELD" << endl;	  
	      return YIELD;
	    }
	  else{ // inner stream exhausted
	    qp->Close(sli->fun.addr);
	    sli->funfinished = true;
	    ((Attribute*)(sli->XVal.addr))->DeleteIfAllowed();
	    sli->XVal.addr = 0;
	  }
	} // end while
      result.addr = 0;
      // cout << "Suse_SNN finished REQUEST: CLOSE (4)" << endl;      
      return CANCEL;

    case CLOSE :
      
      // cout << "Suse_SNN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if (!sli->funfinished)
	    qp->Close( sli->fun.addr ); // close map result stream
	  if (!sli->Xfinished)
	    qp->Close( sli->X.addr );   // close outer stream
	  delete sli;
	}
      // cout << "Suse_SNN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "Suse_SNN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached

}

// (stream X) (stream Y) (map X Y Z) -> (stream Z)
int Suse_SSN( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              funresult;
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      //cout << "\nSuse_SSN received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished = true;
      sli->Yfinished = true;
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
	{ 
	  delete( sli );
	  local.addr = 0;
	  cout << "\nSuse_SSN was called with stream result mapping!" 
	       <<  endl;
	  return 0;
	}
      if(!(sli->argConfDescriptor & 3))
	{ 
	  delete( sli );
	  local.addr = 0;
	  cout << "\nSuse_SSN was called with non-stream arguments!" 
	       <<  endl;
	  return 0;
	}
      sli->X = SetWord(args[0].addr);   // X is the stream
      sli->Y = SetWord(args[1].addr);   // Y is the constant value
      sli->fun = SetWord(args[2].addr); // fun is the mapping function
      
      qp->Open(sli->X.addr);            // open outer stream argument
      sli->Xfinished = false;
      local = SetWord(sli);
      //cout << "Suse_SSN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we evaluate the
      // parameter function (sli->fun), which return a single result.
      // A clone of the result is passed as the result.
      // We also need to delete each element, when it is not required
      // anymore.

      //cout << "Suse_SSN received REQUEST" << endl;

      // get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  //cout << "Suse_SSN finished REQUEST: CLOSE (1)" << endl;
	  return CANCEL;
	}
      sli = (SuseLocalInfo*) local.addr;

      while(!sli->Xfinished)
	{
	  if (sli->Yfinished)
	    { // try to (re-) start outer instream
	      qp->Request(sli->X.addr, sli->XVal);
	      if (!qp->Received(sli->X.addr))
		{ // outer instream exhaused
		  qp->Close(sli->X.addr);
		  sli->Xfinished = true;
		  result.addr = 0;
		  //cout << "Suse_SSN finished REQUEST: CANCEL (2)" << endl;
		  return CANCEL;
		} 
	      // Got next X-elem. (Re-)Start inner instream:
	      qp->Open(sli->Y.addr);
	      sli->Yfinished = false;
	    } 
	  // Now, we have open inner and outer streams
	  qp->Request(sli->Y.addr, sli->YVal);
	  if (!qp->Received(sli->Y.addr))
	    { // inner stream is exhausted
	      qp->Close(sli->Y.addr);
	      // Delete current X-elem:
	      ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
	      sli->Yfinished = true;
	    } 
	  // got next Y-elem
	  if (!sli->Xfinished && !sli->Yfinished)
	    { // pass parameters and call mapping, clone result
	      funargs = qp->Argument( sli->fun.addr );
	      (*funargs)[0] = sli->XVal;
	      (*funargs)[1] = sli->YVal;
	      qp->Request( sli->fun.addr, funresult );
	      result = SetWord(((Attribute*) (funresult.addr))->Clone());
	      ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed(); 
	      //cout << "Suse_SSN finished REQUEST: YIELD" << endl;
	      return YIELD;
	    }
	} // end while
      //cout << "Suse_SSN finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      //cout << "Suse_SSN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if (!sli->Yfinished)
	    {
	      qp->Close( sli->Y.addr ); // close inner instream
	      // Delete current X-elem:
	      ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
	    }
	  if (!sli->Xfinished)
	    qp->Close( sli->X.addr ); // close outer instream
	  delete sli;
	}
      result.addr = 0;
      //cout << "Suse_SSN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  result.addr = 0;
  cout << "\nSuse_SSN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}



// (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
int Suse_SSS( Word* args, Word& result, int message,
		     Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              funresult;
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      //cout << "\nSuse_SSS received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished   = true;
      sli->Yfinished   = true;
      sli->funfinished = true;
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
      if(!(sli->argConfDescriptor & 4) )
	{ 
	  delete( sli );
	  local.addr = 0;
	  cout << "\nSuse_SSS was called with non-stream result mapping!" 
	       <<  endl;
	  return 0;
	}
      if(!(sli->argConfDescriptor & 3))
	{ 
	  delete( sli );
	  local.addr = 0;
	  cout << "\nSuse_SSS was called with non-stream arguments!" 
	       <<  endl;
	  return 0;
	}
      sli->X   = SetWord(args[0].addr); // X is the stream
      sli->Y   = SetWord(args[1].addr); // Y is the constant value
      sli->fun = SetWord(args[2].addr); // fun is the mapping function
      qp->Open(sli->X.addr);            // open X stream argument      
      sli->Xfinished = false;
      local = SetWord(sli);
      //cout << "Suse_SSS finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we open the
      // parameter function (sli->fun), which returns a stream result.
      // We consume this map result stream one-by-one.
      // When it is finally consumed, we try to restart it with the next 
      // X/Y value pair.
      // A clone of the result is passed as the result.
      // We also need to delete each X/Y element, when it is not required
      // any more.

      //cout << "Suse_SSS received REQUEST" << endl;

      // get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  //cout << "Suse_SSS finished REQUEST: CLOSE (1)" << endl;
	  return CANCEL;
	}
      sli = (SuseLocalInfo*) local.addr;

      while(!sli->Xfinished)
	{
	  if (sli->Yfinished)
	    { // get next X-value from outer instream
	      // and restart inner (Y-) instream
	      qp->Request(sli->X.addr, sli->XVal);
	      if (!qp->Received(sli->X.addr))
		{ // X-instream exhaused
		  qp->Close(sli->X.addr);
		  sli->Xfinished = true;
		  //cout << "Suse_SSS finished REQUEST: CANCEL (2)" << endl;
		  result.addr = 0;
		  return CANCEL;
		} 
	      // Got next X-elem. (Re-)Start inner Y-instream:
	      qp->Open(sli->Y.addr);
	      sli->Yfinished = false;
	    } // Now, we have open X- and Y- streams
	  if (sli->funfinished)
	    { // get next Y-value from inner instream
	      // and open new map result stream
	      qp->Request(sli->Y.addr, sli->YVal);
	      if (!qp->Received(sli->Y.addr))
		{
		  qp->Close(sli->Y.addr);
		  ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
		  sli->Yfinished = true;		  
		}
	      else
		{
		  funargs = qp->Argument( sli->fun.addr );
		  (*funargs)[0] = sli->XVal;
		  (*funargs)[1] = sli->YVal;		  
		  qp->Open( sli->fun.addr );
		  sli->funfinished = false;
		}
	    }
	  // Now, we have an open map result streams
	  if (!sli->Xfinished && !sli->Yfinished && !sli->funfinished)
	    { // pass parameters and call mapping, clone result
	      funargs = qp->Argument( sli->fun.addr );
	      (*funargs)[0] = sli->XVal;
	      (*funargs)[1] = sli->YVal;
	      qp->Request( sli->fun.addr, funresult );
	      if ( qp->Received(sli->fun.addr) )
		{ // got a value from map result stream
		  result = SetWord(((Attribute*) (funresult.addr))->Clone());
		  //cout << "Suse_SSS finished REQUEST: YIELD" << endl;
		  return YIELD;
		}
	      else
		{ // map result stream exhausted
		  qp->Close( sli->fun.addr) ;
		  ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
		  sli->funfinished = true;
		} // try to restart with new X/Y pairing
	    }
	} // end while
      result.addr = 0;
      //cout << "Suse_SSS finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      //cout << "Suse_SSS received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if (!sli->funfinished)
	    {
	      qp->Close( sli->fun.addr ); // close map result stream
	      // Delete current Y-elem:
	      ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
	    }
	  if (!sli->Yfinished)
	    {
	      qp->Close( sli->Y.addr ); // close inner instream
	      // Delete current X-elem:
	      ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
	    }
	  if (!sli->Xfinished)
	    qp->Close( sli->X.addr ); // close outer instream
	  delete sli;
	}
      //cout << "Suse_SSS finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "\nSuse_SSS received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}



/*
5.20.3 Specification for operator ~suse~

*/
const string
TemporalSpecSuse=
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>TemporalUnitAlgebra</text--->"
  "<text>For X in kind DATA or X = tuple(Z), Y in kind DATA:\n"
  "(stream X) (map X Y)          -> (stream Y) \n"
  "(stream X) (map X (stream Y)) -> (stream Y)</text--->"
  "<text>_ suse [ _ ]</text--->"
  "<text>The suse class of operators implements "
  "a set of functors, that derive stream-valued "
  "operators from operators taking scalar "
  "arguments and returning scalar values or "
  "streams of values.</text--->"
  "<text>atmax(runitvalue) suse[(fun(x:runit) initial(x))]</text---> ) )";

const string
TemporalSpecSuse2=
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>TemporalUnitAlgebra</text--->"
  "<text>For X in kind DATA or X = tuple(W), Y,Z in kind DATA:\n"
  "(stream X) Y          (map X Y Z)          -> (stream Z) \n"
  "(stream X) Y          (map X Y stream(Z))  -> (stream Z) \n"
  "X          (stream Y) (map X y Z)          -> (stream Z) \n"
  "X          (stream Y) (map X y (stream Z)) -> (stream Z) \n"
  "(stream X) (stream Y) (map X Y Z)          -> (stream Z) \n"
  "(stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)</text--->"
  "<text>_ _ suse2 [ _ ]</text--->"
  "<text>The suse2 class of operators implements "
  "a set of functors, that derive stream-valued "
  "operators from operators taking scalar "
  "arguments and returning scalar values or "
  "streams of values. suse2 performs a product "
  "between the two first of its arguments, passing each "
  "combination to the mapped function once.</text--->"
  "<text> Still missing.</text---> ) )";

/* 
5.20.4 Selection Function of operator ~suse~

*/

ValueMapping temporalunitsusemap[] = 
  { Suse_SN,
    Suse_SS 
    //    ,
    //    Suse_TsN,
    //    Suse_TsS
  };

int
temporalunitSuseSelect( ListExpr args )
{ 
  ListExpr sarg1     = nl->First( args );
  ListExpr mapresult = nl->Third(nl->Second(args));
  bool     isStream  = false;
  bool     isTuple   = false;

  // check type of sarg1
  if( TypeOfRelAlgSymbol(nl->First(sarg1)) == stream &&
      (!nl->IsAtom(nl->Second(sarg1))) &&
      (nl->ListLength(nl->Second(sarg1)) == 2) &&
      TypeOfRelAlgSymbol(nl->First(nl->Second(sarg1))) == tuple &&
      IsTupleDescription(nl->Second((nl->Second(sarg1)))) )
    isTuple = true;
  else 
    isTuple = false;

  // check type of map result (stream or non-stream type)
  if(   !( nl->IsAtom(mapresult) ) 
      && ( nl->ListLength( mapresult ) == 2)
      && TypeOfRelAlgSymbol(nl->First(sarg1) == stream ) )
    isStream = true;
  else 
    isStream = false;  

  // compute index without offset
  if      (!isTuple && !isStream) return 0;
  else if (!isTuple &&  isStream) return 1;
    //  else if ( isTuple && !isStream) return 2;
    //  else if ( isTuple &&  isStream) return 3;
  else
    {
      cout << "\ntemporalunitSuseSelect: Something's wrong!\n";
    }
  return -1;
}


ValueMapping temporalunitsuse2map[] =
  { Suse_SNN,
    Suse_SNS,
    Suse_SSN,
    Suse_SSS
    //     ,
    //    Suse_TsNN,
    //    Suse_TsNS,
    //    Suse_TsTsN,
    //    Suse_TsTsS
  };

int
temporalunitSuse2Select( ListExpr args )
{ 
  ListExpr 
    X = nl->First(args), 
    Y = nl->Second(args),
    M = nl->Third(args);
  bool 
    xIsStream = false, 
    yIsStream = false, 
    resIsStream = false;
  bool 
    xIsTuple = false,
    yIsTuple = false, 
    resIsTuple = false;
  int index = 0;
    
  // examine first arg
  // check type of sarg1
  if( nl->IsAtom(X) )
    { xIsTuple = false; xIsStream = false;}
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == tuple )
    { xIsTuple = true; xIsStream = false; }
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == stream )
    {
      xIsStream = true;
      if(!nl->IsAtom(nl->Second(X)) &&
	 (nl->ListLength(nl->Second(X)) == 2) &&
	 TypeOfRelAlgSymbol(nl->First(X)) == tuple )
	xIsTuple = true;
      else
	xIsTuple = false;
    }
 
  // examine second argument
  if( nl->IsAtom(Y) )
    { yIsTuple = false; yIsStream = false;}
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
    { yIsTuple = true; yIsStream = false; }
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == stream )
    {
      yIsStream = true;
      if(!nl->IsAtom(nl->Second(Y)) &&
	 (nl->ListLength(nl->Second(Y)) == 2) &&
	 TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
	yIsTuple = true;
      else
	yIsTuple = false;
    }

  // examine mapping result type
  if( nl->IsAtom(nl->Fourth(M)) )
    { resIsTuple = false; resIsStream = false;}
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
    { resIsTuple = true; resIsStream = false; }
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == stream )
    {
      resIsStream = true;
      if(!nl->IsAtom(nl->Second(nl->Fourth(M))) &&
	 (nl->ListLength(nl->Second(nl->Fourth(M))) == 2) &&
	 TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
	resIsTuple = true;
      else
	resIsTuple = false;
    }

  // calculate appropriate index value
  
  // tuple variants offest    : +4
  // both args streams        : +2
  // mapping result is stream : +1
  index = 0;
  if ( xIsTuple || yIsTuple )   index += 4;
  if ( xIsStream && yIsStream ) index += 2;
  if ( resIsStream )            index += 1;
    
  if (index > 3)
    cout << "\nWARNING: index =" << index 
	 << ">3 in temporalunitSuse2Select!" << endl;

  return index;
}

/*
5.20.5  Definition of operator ~suse~

*/


Operator temporalsuse( "suse",
                      TemporalSpecSuse,
                      2,
                      temporalunitsusemap,
                      temporalunitSuseSelect,
                      TypeMapSuse);


Operator temporalsuse2( "suse2",
                      TemporalSpecSuse2,
                      4,
                      temporalunitsuse2map,
                      temporalunitSuse2Select,
                      TypeMapSuse2);



/*
6 Creating the Algebra

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
   AddOperator( &temporalsuse );
   AddOperator( &temporalsuse2 );
  }
  ~TemporalUnitAlgebra() {};
};

TemporalUnitAlgebra temporalUnitAlgebra;

/*
7 Initialization

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


