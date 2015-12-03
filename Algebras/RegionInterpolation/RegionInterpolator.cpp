/*

 see RegionInterpolator.h for documentation

\tableofcontents

*/

#include "RegionInterpolator.h"

using namespace temporalalgebra;

#define COUNTWEIGHT 4      //How many weights shall be given to the OptimalMatch
namespace RegionInterpol
{

/*

1.1 The Value--Mapping--Function for interpolate without given weights

*/

static int interpolateValueMap_1(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s)
{
   try{
    result = qp->ResultStorage(s);
    Region* reg1 = (Region*) args[0].addr;
    Region* reg2 = (Region*) args[1].addr;
    Periods *range = ((Periods*)args[2].addr);
    Interval<Instant> *inter= new Interval<Instant>();
    range->Get( 0, *inter );
    assert(inter->IsValid());
    RegionForInterpolation *reginter1=new RegionForInterpolation(reg1);
    RegionForInterpolation *reginter2=new RegionForInterpolation(reg2);
    vector<double> weigths = vector<double>(COUNTWEIGHT);
    weigths[0] = 0.7;            // AreaWeight
    weigths[1] = 0.7;            // OverlapWeight
    weigths[2] = 0.5;            // HausdorffWeight
    weigths[3] = 1.0;            // LinearWeight
    Match *sm=new OptimalMatch(reginter1,reginter2,weigths);
    mLineRep *lines=new mLineRep(sm);
    URegion *res= new URegion(lines->getTriangles(),*inter);
    result.addr=res->Clone();
    delete inter;
    return 0;
  } catch(const std::exception& e){
     cerr << e.what() << endl;
     return 0;
  }
}
/*

1.1 The Value--Mapping--Function for interpolate with given weights

*/
static int interpolateValueMap_2(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s)
{
    result = qp->ResultStorage(s);
    Region* reg1 = (Region*) args[0].addr;
    Region* reg2 = (Region*) args[1].addr;
    Periods *range = ((Periods*)args[2].addr);
    Interval<Instant> *inter= new Interval<Instant>();
    range->Get( 0, *inter );
    assert(inter->IsValid());
    RegionForInterpolation *reginter1=new RegionForInterpolation(reg1);
    RegionForInterpolation *reginter2=new RegionForInterpolation(reg2);
    vector<double> weights = vector<double>(COUNTWEIGHT);
    for (int i = 0; i < COUNTWEIGHT; i++)
    {
      weights[i] = ((CcReal*)args[1].addr)->GetValue();
    }
    Match *sm=new OptimalMatch(reginter1,reginter2,weights);
    mLineRep *lines=new mLineRep(sm);
    URegion *res= new URegion(lines->getTriangles(),*inter);
    result.addr=res->Clone();
    delete inter;
    return 0;
}

ValueMapping interpolateValueMap[] = {
        interpolateValueMap_1, interpolateValueMap_2 };

/*

1.1 The Select--Function for the interpolate operator

*/

static int interpolateSelect(ListExpr args){
   if(nl->ListLength(args)==3)
      return(0);
  return(1);
}

/*

1.1 The Type--Mapping--Function for the interpolate operator

*/

static ListExpr interpolateTypeMap(ListExpr args)
{
    if( !((nl->ListLength(args)==3+COUNTWEIGHT)||(nl->ListLength(args)==3)) )
    {
        ErrorReporter::ReportError("invalid number of arguments");
        return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    if(!nl->IsEqual(nl->First(args),Region::BasicType()))
    {
        ErrorReporter::ReportError("region as second argument required");
        return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    if(!nl->IsEqual(nl->Second(args),Region::BasicType()))
    {
        ErrorReporter::ReportError("region as second argument required");
        return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    if(!nl->IsEqual(nl->Third(args),Periods::BasicType()))
    {
        ErrorReporter::ReportError("Period as third argument required");
        return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    if( nl->ListLength(args)!=3+COUNTWEIGHT)
    {
      for (int i=4; i<=3+COUNTWEIGHT; i++)
      {
          if(!nl->IsEqual(nl->Nth(i,args),CcReal::BasicType()))
         {
            ErrorReporter::ReportError("all weights must be of type real");
         }
      }
    }
    return nl->SymbolAtom(URegion::BasicType());
}

static const string movespec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    " ( \" region, region, periods (x real){0 or 4} -> uregion\""
    "    <text>interpolate( _ , _)</text--->"
    "    <text>Creates a regionunit from the given regions"
    "  with the given period. Weights for the calculation can be given,"
    "AreaWeight, OverlapWeight, HausdorffWeight and LinearWeight. </text--->"
    "    <text>query  interpolate(thecenter,thecenter translate[100.0,100.0],"
    "theRange(theInstant(2004,5,21),theInstant(2004,6,8),"
    "TRUE,FALSE))</text---> ) )";

static Operator interpolate("interpolate",
                     movespec,
                     2,
                     interpolateValueMap,
                     interpolateSelect,
                     interpolateTypeMap);



RegionInterpolationAlgebra::RegionInterpolationAlgebra():Algebra()
{
       AddOperator(&interpolate);
}

RegionInterpolationAlgebra::~RegionInterpolationAlgebra()
{
}

  static RegionInterpolationAlgebra regionInterpolationAlgebra;

extern "C"
Algebra*
InitializeRegionInterpolationAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return new RegionInterpolationAlgebra;
}

}
