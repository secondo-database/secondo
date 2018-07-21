/*
TemporalAlgebraOperators.cpp
Created on: 01.07.2018
Author: simon

*/

#include "TemporalAlgebraOperators.h"
#include "TemporalAlgebraFunctions.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "ListUtils.h"

#include "MPoint2.h"
#include "Algebras/Temporal/TemporalAlgebra.h"

#include "DateTime.h"


namespace temporal2algebra {

using temporalalgebra::Periods;
using datetime::DateTime;
/*
16.1.7 Type mapping function ~MovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr
MovingPeriodsTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Periods::BasicType() ) )
    {
      if( nl->IsEqual( arg1, MPoint2::BasicType() ) )
        return nl->SymbolAtom( MPoint2::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.2.3 Selection function ~MovingSimpleSelect~

Is used for the ~deftime~, ~initial~, ~final~, ~inst~, ~val~, ~atinstant~,
~atperiods~, ~getposition~  operations.

*/
int
MovingSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == MPoint2::BasicType() )
    return 0;

  return -1; // This point should never be reached
}


int MPoint2AtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MPoint2* mp = ((MPoint2*)args[0].addr);
  MPoint2* pResult = (MPoint2*)result.addr;
  Periods* per = (Periods*)args[1].addr;

  AtPeriods(*mp, *per,*pResult);
  return 0;
}

ValueMapping temporalatperiodsmap[] = { MPoint2AtPeriods };

const string TemporalSpecAtPeriods =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT periods) -> mT</text--->"
  "<text>_ atperiods _ </text--->"
  "<text>Restrict the moving object to the given periods.</text--->"
  "<text>mpoint1 atperiods periods1</text--->"
  ") )";

Operator* getAtperiodsOpPtr() {
    return new Operator( "atperiods",
                            TemporalSpecAtPeriods,
                            1,
                            temporalatperiodsmap,
                            MovingSimpleSelect,
                            MovingPeriodsTypeMapMoving
                            );
}


/*
16.1.18 Type mapping function "Temporal2BBoxTypeMap"

For operator ~bbox~

*/

ListExpr Temporal2BBoxTypeMap( ListExpr args )
{
  int noargs = nl->ListLength( args );
  string errmsg = "Expected (M [x geoid]) OR (T), where M in {upoint, mpoint, "
                  "ipoint}, T in {instant,periods}.";
  if ( (noargs<1) || (noargs>2) ){
    return listutils::typeError(errmsg);
  }
  if( (noargs==2) && !listutils::isSymbol(nl->Second(args),Geoid::BasicType())){
    return listutils::typeError(errmsg);
  }
  ListExpr arg1 = nl->First( args );

  if( listutils::isSymbol( arg1, MPoint2::BasicType() ) )
      return (nl->SymbolAtom( Rectangle<3>::BasicType() ));

  return listutils::typeError(errmsg);
}


/*
Selection function for the bbox operator

*/

int Temporal2BBoxSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == MPoint2::BasicType() )
    return 0;

  return -1; // This point should never be reached
}

/*
Since MPoint2 is not a subclass of ~SpatialAttribute~, it has no ~BoundingBox()~
function. One could make it inherit from ~SpatialAttribute~, but than one had to
restore all databases and adopt the Add, MergeAdd, +=, -=, etc.

*/
int MPoint2BBox(Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  Rectangle<3>* res = (Rectangle<3>*) result.addr;
  const MPoint2* arg = static_cast<const MPoint2*>(args[0].addr);
  const Geoid*  geoid =
                (qp->GetNoSons(s)==2)?static_cast<const Geoid*>(args[1].addr):0;

  if( !arg->IsDefined() || (arg->GetNoComponents() < 1) )
  { // empty/undefined MPoint2 --> undef
    res->SetDefined(false);
  }
  else { // return MBR
    *res = BoundingBox(*arg, geoid);
  }
  return 0;
}


ValueMapping temporal2bboxmap[] = {
        MPoint2BBox };

const string Temporal2SpecBBox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>upoint [x geoid] -> rect3,\n"
  "mpoint [x geoid] -> rect3,\n"
  "ipoint [x geoid] -> rect3,\n"
  "instant -> rect3,\n"
  "periods -> rect3</text--->"
  "<text>bbox ( Obj [, Geoid])</text--->"
  "<text>Returns the 3d bounding box of the spatio-temporal2 object Obj, \n"
  "resp. the universe restricted to the definition time of the instant/\n"
  "period value. If Geoid is passed, the geographic MBR is computed.</text--->"
  "<text>query bbox( upoint1 )</text--->"
  ") )";

Operator* getBBoxOpPtr() {
    return new Operator(
            "bbox",
            Temporal2SpecBBox,
            1,
            temporal2bboxmap,
            Temporal2BBoxSelect,
            Temporal2BBoxTypeMap
    );
}


/*
16.1.8 Type mapping function ~MovingTypeMapSpatial~

It is for the operator ~trajectory~.

*/
ListExpr
MovingTypeMapSpatial( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, MPoint2::BasicType() ) )
      return nl->SymbolAtom( Line::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
16.3.23 Value mapping functions of operator ~trajectory~

*/
int MPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Line *line = ((Line*)result.addr);
  MPoint2* mpoint = ((MPoint2*)args[0].addr);
  Trajectory(*mpoint, *line );

  return 0;
}

const string TemporalSpecTrajectory =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint -> line</text--->"
  "<text> trajectory( _ )</text--->"
  "<text>Get the trajectory of the corresponding moving point object.</text--->"
  "<text>trajectory( mp1 )</text--->"
  ") )";

Operator* getTrajectoryOpPtr() {
    return new Operator(
            "trajectory",
            TemporalSpecTrajectory,
            MPointTrajectory,
            Operator::SimpleSelect,
            MovingTypeMapSpatial
    );
}


/*
16.1.20 TranslateAppendS

This operator consumes a stream of tuples having an attribute
x of type mpoint. The attribute name must be given in the second
argument. As third argument, a duration is given which determines
a break between the two movements.

*/

ListExpr TranslateAppendSTM(ListExpr args){


  int len = nl->ListLength(args);
  if(len != 3 ){
      ErrorReporter::ReportError("three arguments expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  // check the third argument to be of type duration
  if(!nl->IsEqual(nl->Third(args),Duration::BasicType())){
    ErrorReporter::ReportError("the third argument has to be a duration");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  // extract the attribute name
  ListExpr attrlist = nl->Second(args);

  if(nl->AtomType(attrlist)!=SymbolType){
      ErrorReporter::ReportError("the second argument has to be a symbol");
      return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  string a1 = nl->SymbolValue(attrlist);

  int a1index = -1;

  // search for attrname in stream definition
  ListExpr stype = nl->First(args);
  if(nl->AtomType(stype)!=NoAtom){
     ErrorReporter::ReportError("stream(tuple(...))"
                                " expected as the first argument");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  if((nl->ListLength(stype)!=2) ||
     (!nl->IsEqual(nl->First(stype),Symbol::STREAM() ))){
     ErrorReporter::ReportError("stream(tuple(...))"
                                " expected as the first argument");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr ttype = nl->Second(stype);

  if((nl->ListLength(ttype)!=2) ||
     (!nl->IsEqual(nl->First(ttype),Tuple::BasicType() ))){
     ErrorReporter::ReportError("stream(tuple(...))"
                                " expected as the first argument");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr attributes = nl->Second(ttype);
  if(nl->AtomType(attributes)!=NoAtom){
      ErrorReporter::ReportError("invalid tuple type");
      return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  int pos = 0;
  while(!nl->IsEmpty(attributes)){
     ListExpr attr = nl->First(attributes);
     if( (nl->AtomType(attr)!=NoAtom) ||
         (nl->ListLength(attr)!=2)){
         ErrorReporter::ReportError("invalid tuple type");
         return nl->SymbolAtom(Symbol::TYPEERROR());
     }
     ListExpr anl = nl->First(attr);
     ListExpr atl = nl->Second(attr);
     if( (nl->AtomType(anl)!=SymbolType) ||
         (nl->AtomType(atl)!=SymbolType)){
         ErrorReporter::ReportError("invalid tuple type");
         return nl->SymbolAtom(Symbol::TYPEERROR());
     }
     string aname = nl->SymbolValue(anl);
     if(aname==a1){
        if(a1index>=0){
           ErrorReporter::ReportError("attr name occurs twice");
           return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        if(!nl->IsEqual(atl,MPoint2::BasicType())){
            ErrorReporter::ReportError("the attribute"
                                       " has to be of type mpoint2");
            return nl->SymbolAtom(Symbol::TYPEERROR());
        }
        a1index = pos;
     }
     pos++;
     attributes = nl->Rest(attributes);
  }

  if(a1index<0){
     ErrorReporter::ReportError("first attr name does"
                                " not occur in the typle");
     return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  // all is correct
  ListExpr ind = nl->OneElemList(nl->IntAtom(a1index));

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           ind,
                           nl->SymbolAtom(MPoint2::BasicType()));
}

/*
16.3.45 Value mapping function for ~translateAppendS~

*/

int TranslateAppendSVM(Word* args, Word& result,
                      int message, Word& local,
                      Supplier s){
   result = qp->ResultStorage(s);
   MPoint2* res = (MPoint2*) result.addr;
   int index = ((CcInt*)args[3].addr)->GetIntval();

   DateTime* duration = (DateTime*) args[2].addr;

   res->Clear();
   res->SetDefined(true);
   Word current;
   MPoint2* mpoint=NULL;
   qp->Open(args[0].addr);
   qp->Request(args[0].addr, current);
   while (qp->Received(args[0].addr)) {
      Tuple* tuple = (Tuple*)current.addr;
      mpoint =  (MPoint2*)(tuple->GetAttribute(index));
      TranslateAppend(*mpoint,*duration, *res);
      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, current);
   }
   qp->Close(args[0].addr);
   return 0;
}

const string TranslateAppendSSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(tuple((a1 t1)...(an tn))) x ai x duration"
    " -> mpoint, where ti = mpoint</text--->"
    "<text> _ translateappendS[ _ _ ]</text--->"
    "<text>Builds a single moving point from all mpoints in the stream \n"
    " translating the mpoints in such a way that a connected movement\n"
    " is created. 'Jumps' within stream elements are not removed.</text--->"
    "<text>query Trains feed translateappendS[Trip [const"
    " duration value(0 10000)]]</text--->"
    ") )";

Operator* getTranslateappendSOpPtr(){
    return new Operator(
            "translateappendS",
            TranslateAppendSSpec,
            TranslateAppendSVM,
            Operator::SimpleSelect,
            TranslateAppendSTM
    );
}

} /* namespace temporal2algebra */
