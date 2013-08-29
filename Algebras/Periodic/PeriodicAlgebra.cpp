/*
3 Connecting with Secondo

*/

#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <vector>

using namespace std;

#include "PeriodicTypes.h"
#include "PeriodicSupport.h"
#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "SpatialAlgebra.h"
#include "TopRel.h"
#include "GenericTC.h"
#include "GenOps.h"
#include "Symbols.h"
#include "TemporalAlgebra.h"
#include "PMSimple.h"


extern NestedList* nl;
extern QueryProcessor *qp;


namespace periodic {

/*
3.1 Secondo Type Constructors

*/

GenTC<PBBox> pbbox;

GenTC<RelInterval>  relinterval;

GenTC<PInterval> pinterval;

GenTC<PMPoint> pmpoint;

GenTC<PMPoints> pmpoints;

GenTC<PMBool> pmbool;

GenTC<PMInt9M> pmint9m;

GenTC<PMReal> pmreal;



/*

3.2 Definition of Operators


3.2.1 ~=~

~Type Mapping~ and ~Selection Function~

*/

static complexTM getEqualsCTM(){
   complexTM tm;
   tm.add(tm2<PBBox,PBBox,CcBool>());
   tm.add(tm2<PInterval,PInterval,CcBool>());
   return tm;
}

static ListExpr EqualsTypeMap(ListExpr args){
   return getEqualsCTM()(args);
}

static int EqualsSelect(ListExpr args){
   return getEqualsCTM().select(args);
}

/*
~Functor~ and ~Value Mapping Array~

*/

template<class A1,class A2,class R>
class EqualF{
public:
  void operator()(A1* a1, A2* a2, R* res){
     res->Set(true,a1->CompareTo(a2)==0);
  }
};

ValueMapping EqualsValueMap[] = {
  GenVM2<PBBox, PBBox, CcBool, EqualF<PBBox,PBBox,CcBool> >,
  GenVM2<PInterval, PInterval, CcBool, EqualF<PInterval,PInterval, CcBool> >
};

/*
~Operator Instance~

*/

Operator pequals(
       "=",
       getEqualsCTM().getSpecification(
           "_ = _ ",
           "Check for equality of the arguments",
           "query a = b"),
       getEqualsCTM().getVMCount(),
       EqualsValueMap,
       EqualsSelect,
       EqualsTypeMap);

/*
3.2.2  ~At~

*/

static complexTM getAtCTM(){
  complexTM tm;
  tm.add(tm2<PMPoint,DateTime,Point>());
  tm.add(tm2<PMBool,DateTime,CcBool>());
  tm.add(tm2<PMInt9M,DateTime,Int9M>());
  tm.add(tm2<PMPoints,DateTime,Points>());
  tm.add(tm2<PMReal,DateTime,CcReal>());
  return tm;
}

static ListExpr AtTypeMap(ListExpr args){
   return getAtCTM()(args);
}

static int AtSelect(ListExpr args){
   return getAtCTM().select(args);
}

template<class ArgT, class ResT>
class AtF_Sec{
  public:
  void operator()(ArgT* a1, DateTime* i, ResT* res){
    a1->At(i,*res);
  }
};

template<class ArgT, class ResT, class CT>
class AtF_C{
  public:
  void operator()(ArgT* a1, DateTime* i, ResT* res){
    CT r;
    a1->At(i,r);
    res->Set(true, r);
  }
};

ValueMapping AtValueMap[] = {
  GenVM2<PMPoint, DateTime, Point, AtF_Sec<PMPoint, Point> >,
  GenVM2<PMBool, DateTime, CcBool, AtF_C<PMBool, CcBool, bool> >,
  GenVM2<PMInt9M, DateTime, Int9M, AtF_Sec<PMInt9M, Int9M> >,
  GenVM2<PMPoints, DateTime, Points, AtF_Sec<PMPoints, Points> >,
  GenVM2<PMReal, DateTime, CcReal, AtF_C<PMReal, CcReal, double> >
};

Operator pat(
       "atinstant",
       getAtCTM().getSpecification(
         "_ atinstant _",
         "returns the value of the first argument for a given instant",
         "query pm atinstant i"),
       getAtCTM().getVMCount(),
       AtValueMap,
       AtSelect,
       AtTypeMap);


/*
3.2.3 ~Union~

*/

const string UnionSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pbbox x pbbox -> pbbox\""
   " \" _ union _ \" "
   " \"computes the union of the arguments\" "
   " \" query B1 union B2\" ))";

template<class A1, class A2,class R>
class UnionF{
public:
  void operator()(A1* a1, A2* a2, R* res){
     a1->Union(a2,*res);
  }
};

Operator punion(
        "union",
        UnionSpec,
        GenVM2<PBBox, PBBox, PBBox, UnionF<PBBox,PBBox,PBBox> >,
        Operator::SimpleSelect,
        TypeMap2<PBBox, PBBox, PBBox> );

/*
3.2.4 ~Trajectory~

*/

const string TrajectorySpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmpoint -> line\""
   " \" trajectory( _ ) \" "
   " \"computes the trajectory of the argument\" "
   " \" query trajectory(P5)\" ))";

template<class A,class R>
class TrajectoryF{
  public:
  void operator()(A* arg, R* res){
    arg->Trajectory(*res);
  }
};

Operator ptrajectory(
        "trajectory",
        TrajectorySpec,
        GenVM1<PMPoint,Line, TrajectoryF<PMPoint,Line> >,
        Operator::SimpleSelect,
        TypeMap1<PMPoint,Line>);


/*
3.2.5 ~length~

*/

const string LengthSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pinterval -> duration\""
   " \" length( _ ) \" "
   " \"computes the duration of a pinterval\" "
   " \" query length(I)\" ))";

template<class A, class R>
class LengthF{
  public:
  void operator()(A* arg, R* res){
    arg->Length(*res);
  }
};

Operator plength(
        "length",
        LengthSpec,
        GenVM1<PInterval, DateTime, LengthF<PInterval, DateTime> >,
        Operator::SimpleSelect,
        TypeMap1<PInterval, Duration>);

/*
3.2.6 ~length2~

*/

const string LengthSpec2 =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmpoint -> real\""
   " \" length( _ ) \" "
   " \"computes the length of the route of a pmpoint\" "
   " \" query length(p)\" ))";

Operator plength2(
        "length",
        LengthSpec2,
        GenVM1<PMPoint, CcReal, LengthF<PMPoint,CcReal> >,
        Operator::SimpleSelect,
        TypeMap1<PMPoint,CcReal>);


/*
3.2.7 ~start~

*/

const string StartSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pinterval -> instant\""
   " \" start( _ ) \" "
   " \"computes the start time of a pinterval\" "
   " \" query start(I)\" ))";

template<class A, class R>
class StartF{
  public:
  void operator()(A* a, R* res){
      a->GetStart(*res);
  }
};

Operator pstart(
        "start",
        StartSpec,
        GenVM1<PInterval, DateTime, StartF<PInterval,DateTime> >,
        Operator::SimpleSelect,
        TypeMap1<PInterval, DateTime>);


/*
3.2.8 ~end~

*/

const string EndSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pinterval -> instant\""
   " \" end( _ ) \" "
   " \"computes the start time of a pinterval\" "
   " \" query end(I)\" ))";

template<class A, class R>
class EndF{
  public:
  void operator()(A* a, R* res){
      a->GetEnd(*res);
  }
};

Operator pend(
        "end",
        EndSpec,
        GenVM1<PInterval,DateTime, EndF<PInterval,DateTime> >,
        Operator::SimpleSelect,
        TypeMap1<PInterval, Instant>);


/*
3.2.9 ~expand~

*/

const string ExpandSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmpoint -> mpoint\""
   " \" expand(_) \" "
   " \"creates a moving point from a periodic one\""
   " \"\""
   " \" query expand(I)\" ))";

template<class A, class R>
class ExpandF{
  public:
  void operator()(A* a, R* res){
      a->Expand(*res);
  }
};

Operator pexpand(
        "expand",
        ExpandSpec,
        GenVM1<PMPoint, MPoint, ExpandF<PMPoint, MPoint> >,
        Operator::SimpleSelect,
        TypeMap1<PMPoint,MPoint>);

/*
3.2.10 ~createpmpoint~

*/

const string CreatePMPointSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"mpoint  -> pmpoint\""
   " \" createpmpoint(_) \" "
   " <text>creates a periodic moving point "
          "from a linearly moving one</text--->"
   " \"\""
   " \" query createpmpoint(p1)\" ))";

template<class A, class R>
class CreateF{
  public:
  void operator()(A* a, R* res){
      res->ReadFrom(*a, false);
  }
};

Operator createpmpoint(
        "createpmpoint",
        CreatePMPointSpec,
        GenVM1<MPoint, PMPoint, CreateF<MPoint, PMPoint> >,
        Operator::SimpleSelect,
        TypeMap1<MPoint,PMPoint>);

/*
3.2.11 ~speed~

*/

const string SpeedSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmpoint -> pmreal \" "
   " \" speed( _ ) \" "
   " \" Computes the Speed of the argument.  \"  "
   " \" \" "
   " <text> query speed(p1)  </text---> ))";

template<class A, class R>
class SpeedF{
   public:
   void operator()(A* a, R* res){
     a->SpeedAndDirection(true,*res);
   }
};

Operator pspeed(
        "speed",
        SpeedSpec,
        GenVM1<PMPoint,PMReal, SpeedF<PMPoint, PMReal> >,
        Operator::SimpleSelect,
        TypeMap1<PMPoint,PMReal>);

/*
3.2.12 ~direction~

*/

const string DirectionSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmpoint -> pmreal \" "
   " \" direction( _ ) \" "
   " \" Computes the Speed of the argument.  \"  "
   " \" \" "
   " <text> query direction(p1)  </text---> ))";

template<class A, class R>
class DirectionF{
   public:
   void operator()(A* a, R* res){
     a->SpeedAndDirection(false,*res);
   }
};

Operator pdirection(
        "direction",
        DirectionSpec,
        GenVM1<PMPoint,PMReal, DirectionF<PMPoint, PMReal> >,
        Operator::SimpleSelect,
        TypeMap1<PMPoint,PMReal>);


/*
3.2.13 ~contains~

*/
static complexTM getContainsCTM(){
   complexTM tm;
   tm.add(tm2<PBBox,PBBox,CcBool>());
   tm.add(tm2<PInterval,PInterval,CcBool>());
   tm.add(tm2<PInterval,DateTime,CcBool>());
   return tm;
}

static ListExpr ContainsTypeMap(ListExpr args){
   return getContainsCTM()(args);
}

static int ContainsSelect(ListExpr args){
   return getContainsCTM().select(args);
}

template<class T1, class T2>
class ContainsF{
  public:
  void operator()(T1* a1, T2* a2, CcBool* res){
     res->Set(true,a1->Contains(a2));
  }
};

ValueMapping ContainsValueMap[] = {
  GenVM2<PBBox, PBBox, CcBool, ContainsF<PBBox, PBBox> >,
  GenVM2<PInterval, PInterval, CcBool, ContainsF<PInterval, PInterval> >,
  GenVM2<PInterval, DateTime, CcBool, ContainsF<PInterval, DateTime> >
};

Operator pcontains(
        "contains",
        getContainsCTM().getSpecification(
         "_ contains _ ",
         "check for containedness",
         "query a contains b"),
        getContainsCTM().getVMCount(),
        ContainsValueMap,
        ContainsSelect,
        ContainsTypeMap);


/*
3.2.14 ~intersects~

*/
static complexTM getIntersectsCTM(){
   complexTM tm;
   tm.add(tm2<PBBox,PBBox,CcBool>());
   tm.add(tm2<PInterval,PInterval,CcBool>());
   return tm;
}

static ListExpr IntersectsTypeMap(ListExpr args){
   return getIntersectsCTM()(args);
}

static int IntersectsSelect(ListExpr args){
   return getIntersectsCTM().select(args);
}

template<class A1, class A2>
class IntersectsF{
public:
  void operator()(A1* a1, A2* a2, CcBool* res){
    if(!a1->IsDefined() || !a2->IsDefined()){
      res->SetDefined(false);
    } else {
      res->Set(true,a1->Intersects(a2));
    }
  }
};

ValueMapping IntersectsValueMap[] = {
  GenVM2<PBBox, PBBox,CcBool, IntersectsF<PBBox, PBBox> >,
  GenVM2<PInterval, PInterval, CcBool, IntersectsF<PInterval, PInterval> >
};

Operator pintersects(
        "intersects",
        getIntersectsCTM().getSpecification(
          " _ intersects _ ",
          " Check for a common element",
          " query a intersects b "
        ),
        getIntersectsCTM().getVMCount(),
        IntersectsValueMap,
        IntersectsSelect,
        IntersectsTypeMap);

/*
3.2.15 ~initial~

*/
static complexTM getInitialCTM(){
   complexTM tm;
   tm.add(tm1<PMBool,CcBool>());
   tm.add(tm1<PMPoint,Point>());
   tm.add(tm1<PMInt9M,Int9M>());
   tm.add(tm1<PMPoints,Points>());
   tm.add(tm1<PMReal,CcReal>());
   return tm;
}

static ListExpr InitialTypeMap(ListExpr args){
   return getInitialCTM()(args);
}

static int InitialSelect(ListExpr args){
   return getInitialCTM().select(args);
}

template<class A, class R>
class InitialF_Sec{
  public:
  void operator()(A* a,R* res){
     a->Initial(*res);
  }
};

template<class A, class R, class CT>
class InitialF_C{
  public:
  void operator()(A* a,R* res){
     CT r=0;
     a->Initial(r);
     res->Set(true,r);
  }
};

ValueMapping InitialValueMap[] = {
  GenVM1<PMBool, CcBool, InitialF_C<PMBool, CcBool, bool> >,
  GenVM1<PMPoint, Point, InitialF_Sec<PMPoint, Point> >,
  GenVM1<PMInt9M, Int9M, InitialF_Sec<PMInt9M, Int9M> >,
  GenVM1<PMPoints, Points, InitialF_Sec<PMPoints, Points> >,
  GenVM1<PMReal, CcReal, InitialF_C<PMReal, CcReal, double> >
};

Operator pinitial(
       "initial",
       getInitialCTM().getSpecification(
          "initial( _ )",
          "the first defined vavlue of the argument",
          "qiery initial(pm1)"),
       getInitialCTM().getVMCount(),
       InitialValueMap,
       InitialSelect,
       InitialTypeMap);


/*
3.2.16 ~final~

*/
static complexTM getFinalCTM(){
   complexTM tm;
   tm.add(tm1<PMBool,CcBool>());
   tm.add(tm1<PMPoint,Point>());
   tm.add(tm1<PMInt9M,Int9M>());
   tm.add(tm1<PMPoints,Points>());
   tm.add(tm1<PMReal,CcReal>());
   return tm;
}

static ListExpr FinalTypeMap(ListExpr args){
   return getFinalCTM()(args);
}

static int FinalSelect(ListExpr args){
   return getFinalCTM().select(args);
}

template<class A, class R>
class FinalF_Sec{
  public:
  void operator()(A* a,R* res){
     a->Final(*res);
  }
};

template<class A, class R, class CT>
class FinalF_C{
  public:
  void operator()(A* a,R* res){
     CT r=0;
     a->Final(r);
     res->Set(true,r);
  }
};

ValueMapping FinalValueMap[] = {
  GenVM1<PMBool, CcBool, FinalF_C<PMBool, CcBool, bool> >,
  GenVM1<PMPoint, Point, FinalF_Sec<PMPoint, Point> >,
  GenVM1<PMInt9M, Int9M, FinalF_Sec<PMInt9M, Int9M> >,
  GenVM1<PMPoints, Points, FinalF_Sec<PMPoints, Points> >,
  GenVM1<PMReal, CcReal, FinalF_C<PMReal, CcReal, double> >
};

Operator pfinal(
       "final",
       getFinalCTM().getSpecification(
         "final ( _ )",
         "returns the last defined value of the argument",
         "query last(pm1)"),              // specification
       getFinalCTM().getVMCount(),
       FinalValueMap,
       FinalSelect,
       FinalTypeMap);

/*
3.2.17 ~breakpoints~

*/

ListExpr BreakpointsTypeMap(ListExpr args){
    __TRACE__
 int length = ::nl->ListLength(args);
   if(length!=1 && length!=3){
       ErrorReporter::ReportError(
         "Wrong number of arguments, one or three arguments expected\n");
       return ::nl->TypeError();
   }
   if(::nl->AtomType(::nl->First(args))!=SymbolType){
      ErrorReporter::ReportError("breakpoints can only handle simple types ");
      return ::nl->TypeError();
   }
   string arg = ::nl->SymbolValue(::nl->First(args));
   if(length==3){
     // check the second argument to be a duration type
     if(!::nl->IsEqual(::nl->Second(args),Duration::BasicType())){
        ErrorReporter::ReportError(
             "The second argument must be of type duration\n");
        return ::nl->TypeError();
     }
     // check the third argument for bool type
     if(!::nl->IsEqual(::nl->Third(args),CcBool::BasicType())){
        ErrorReporter::ReportError(
             "The third argument must be of type bool\n");
        return ::nl->TypeError();
     }
   }

   if(arg=="pmpoint")
      return ::nl->SymbolAtom(Points::BasicType());
   if(arg=="pmpoints")
      return ::nl->SymbolAtom(Points::BasicType());

   ErrorReporter::ReportError(
          "Invalid type for breakpoints operator : "+arg+"\n");
   return ::nl->TypeError();
}


template<class A>
int BreakpointsFun(Word* args, Word& result,
                   int message, Word& local, Supplier s){
    __TRACE__
    result = ::qp->ResultStorage(s);
    Points* res = static_cast<Points*>(result.addr);
    int sons = ::qp->GetNoSons(s);
    A* P = static_cast<A*>(args[0].addr);
    if(sons==1){
       P->Breakpoints(*res);
    } else{
      DateTime* DT = static_cast<DateTime*>(args[1].addr);
      CcBool* Inclusive = static_cast<CcBool*>(args[2].addr);
      if(!Inclusive->IsDefined() || !DT->IsDefined()){
         res->SetDefined(false);
      } else{
         P->Breakpoints(DT,Inclusive->GetBoolval(),*res);
      }
    }
    return 0;
}

ValueMapping BreakpointsValueMap[] = {
  BreakpointsFun<PMPoint>,
  BreakpointsFun<PMPoints>
};

static int BreakpointsSelect(ListExpr args){
   __TRACE__
   int len = ::nl->ListLength(args);
   if(len!=1 && len!=3)
      return -1;
   if(::nl->IsEqual(::nl->First(args),"pmpoint"))
        return 0;
   if(::nl->IsEqual(::nl->First(args),"pmpoints"))
        return 1;
   return -1;
}

const string BreakpointsSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( \"pmpoint | pmpoints  -> points\""
   " \" breakpoints( _ ) \" "
   " \"computes the breakpoints of the argument\" "
   " \" query breakpoints(P5)\" ))";

Operator pbreakpoints(
       "breakpoints",               // name
       BreakpointsSpec,              // specification
       2,                         // number of functions
       BreakpointsValueMap,
       BreakpointsSelect,
       BreakpointsTypeMap);


/*
3.2.18 ~toprel~

*/


static complexTM getTopRelCTM(){
   complexTM tm;
   tm.add(tm2<Point,PMPoint,PMInt9M>());
   tm.add(tm2<PMPoint,Point,PMInt9M>());
   tm.add(tm2<Points,PMPoint,PMInt9M>());
   tm.add(tm2<PMPoint,Points,PMInt9M>());
   return tm;
}

static ListExpr ToprelTypeMap(ListExpr args){
   return getTopRelCTM()(args);
}

static int ToprelSelect(ListExpr args){
   return getTopRelCTM().select(args);
}

template<class A1, class A2>
class ToprelF{
  public:
  void operator()(A1* a1, A2* a2, PMInt9M* r){
     a1->Toprel(*a2,*r);
  }
};

template<class A1, class A2>
class ToprelF_Symm{
  public:
  void operator()(A2* a1, A1* a2, PMInt9M* r){
     a2->Toprel(*a1,*r);
     r->Transpose();
  }
};

ValueMapping ToprelValueMap[] = {
  GenVM2<Point, PMPoint, PMInt9M, ToprelF_Symm<PMPoint, Point> >,
  GenVM2<PMPoint, Point, PMInt9M, ToprelF<PMPoint, Point> >,
  GenVM2<Points, PMPoint, PMInt9M, ToprelF_Symm<PMPoint, Points> >,
  GenVM2<PMPoint, Points, PMInt9M, ToprelF<PMPoint, Points> >
};

Operator ptoprel(
       "toprel",
       getTopRelCTM().getSpecification(
        "toprel(_,_)",
        "computes the topologicial relationship between the arguments",
        "query toprel( a, b)"
       ),           // specification
       getTopRelCTM().getVMCount(),                    // number of functions
       ToprelValueMap,
       ToprelSelect,
       ToprelTypeMap);

/*
3.2.18 ~intersection~

*/
static complexTM getIntersectionCTM(){
   complexTM tm;
   tm.add(tm2<PBBox,PBBox,PBBox>());
   return tm;
}

static ListExpr IntersectionTypeMap(ListExpr args){
   return getIntersectionCTM()(args);
}

static int IntersectionSelect(ListExpr args){
   return getIntersectionCTM().select(args);
}

template<class A1, class A2, class ResT>
class IntersectionF{
  public:
  void operator()(A1* a1, A2* a2, ResT* res){
     a1->Intersection(a2,*res);
  }
};

ValueMapping IntersectionValueMap[] = {
  GenVM2<PBBox, PBBox, PBBox, IntersectionF<PBBox, PBBox, PBBox> >
};

Operator pintersection(
       "intersection",
       getIntersectionCTM().getSpecification(
        " _ intersection _ ",
        "computes the common part of the arguments",
        " query b1 intersection b2"),
       getIntersectionCTM().getVMCount(),
       IntersectionValueMap,
       IntersectionSelect,
       IntersectionTypeMap);

/*
3.2.19 ~distance~

*/
static complexTM getDistanceCTM(){
   complexTM tm;
   tm.add(tm2<PMPoint,Point,PMReal>());
   tm.add(tm2<Point,PMPoint,PMReal>());
   return tm;
}

static ListExpr DistanceTypeMap(ListExpr args){
   return getDistanceCTM()(args);
}

static int DistanceSelect(ListExpr args){
   return getDistanceCTM().select(args);
}

template<class A1, class A2>
class DistanceF{
  public:
  void operator()(A1* a1, A2* a2, PMReal* res){
    a1->DistanceTo(a2->GetX(), a2->GetY(),*res);
  }
  void operator()(A2* a1, A1* a2, PMReal* res){
    a2->DistanceTo(a1->GetX(), a1->GetY(),*res);
  }
};

ValueMapping DistanceValueMap[] = {
 GenVM2<PMPoint,Point, PMReal, DistanceF<PMPoint, Point> >,
 GenVM2<Point, PMPoint, PMReal, DistanceF<PMPoint, Point> >
};

Operator pdistance(
       "distance",             // name
       getDistanceCTM().getSpecification(
         "distance(_, _",
         "computes the distance of the arguments",
         "query distance(pm1, p1)"),           // specification
       getDistanceCTM().getVMCount(),                    // number of functions
       DistanceValueMap,
       DistanceSelect,
       DistanceTypeMap);


/*
3.2.20 ~numberOfNodes~

*/
static complexTM getNumberOfNodesCTM(){
   complexTM tm;
   tm.add(tm1<PMPoint,CcInt>());
   return tm;
}

static ListExpr NumberOfNodesTypeMap(ListExpr args){
   return getNumberOfNodesCTM()(args);
}

static int NumberOfNodesSelect(ListExpr args){
   return getNumberOfNodesCTM().select(args);
}

template<class A, class R>
class NumberOfNodesF{
  public:
  void operator()(A* a, R* res){
      res->Set(true, static_cast<int>(a->NumberOfNodes()));
  }
};


ValueMapping NumberOfNodesValueMap[] ={
  GenVM1<PMPoint,CcInt, NumberOfNodesF<PMPoint, CcInt> >
};

Operator pnumberOfNodes(
       "numberOfNodes",             // name
       getNumberOfNodesCTM().getSpecification(
         "numberOfNodes( _ )",
         "number of nodes within the repetition tree",
         "query numberOfNodes(pm1)"),
       getNumberOfNodesCTM().getVMCount(),
       NumberOfNodesValueMap,
       NumberOfNodesSelect,
       NumberOfNodesTypeMap);


/*
3.2.21 ~NumberOfCNodes~

*/
static complexTM getNumberOfCNodesCTM(){
   complexTM tm;
   tm.add(tm1<PMPoint,CcInt>());
   return tm;
}

static ListExpr NumberOfCNodesTypeMap(ListExpr args){
   return getNumberOfCNodesCTM()(args);
}

static int NumberOfCNodesSelect(ListExpr args){
   return getNumberOfCNodesCTM().select(args);
}

template<class A, class R>
class NumberOfCompositeNodesF{
  public:
  void operator()(A* a, R* res){
      res->Set(true, static_cast<int>(a->NumberOfCompositeNodes()));
  }
};

ValueMapping NumberOfCompositeNodesValueMap[] ={
  GenVM1<PMPoint,CcInt, NumberOfCompositeNodesF<PMPoint, CcInt> >
};

Operator pnumberOfCNodes(
       "numberOfCNodes",             // name
       getNumberOfCNodesCTM().getSpecification(
         "numberOfCNodes( _ )",
         "number of composite nodes within the repetition tree",
         "query numberOfCNodes(pm1)"),
       getNumberOfCNodesCTM().getVMCount(),
       NumberOfCompositeNodesValueMap,
       NumberOfCNodesSelect,
       NumberOfCNodesTypeMap);


/*
3.2.22 ~numberOfPNodes~

*/
static complexTM getNumberOfPNodesCTM(){
   complexTM tm;
   tm.add(tm1<PMPoint,CcInt>());
   return tm;
}

static ListExpr NumberOfPNodesTypeMap(ListExpr args){
   return getNumberOfPNodesCTM()(args);
}

static int NumberOfPNodesSelect(ListExpr args){
   return getNumberOfPNodesCTM().select(args);
}

template<class A, class R>
class NumberOfPeriodicNodesF{
  public:
  void operator()(A* a, R* res){
      res->Set(true, static_cast<int>(a->NumberOfPeriodicNodes()));
  }
};

ValueMapping NumberOfPeriodicNodesValueMap[] ={
  GenVM1<PMPoint,CcInt, NumberOfPeriodicNodesF<PMPoint, CcInt> >
};

Operator pnumberOfPNodes(
       "numberOfPNodes",
       getNumberOfPNodesCTM().getSpecification(
         "numberOfPNodes( _ )",
         "number of periodic nodes within the repetition tree",
         "query numberOfPNodes(pm1)"),
       getNumberOfPNodesCTM().getVMCount(),
       NumberOfPeriodicNodesValueMap,
       NumberOfPNodesSelect,
       NumberOfPNodesTypeMap);

/*
3.2.23 ~numberOfUnits~

*/
static complexTM getNumberOfUnitsCTM(){
   complexTM tm;
   tm.add(tm1<PMPoint,CcInt>());
   return tm;
}

static ListExpr NumberOfUnitsTypeMap(ListExpr args){
   return getNumberOfUnitsCTM()(args);
}

static int NumberOfUnitsSelect(ListExpr args){
   return getNumberOfUnitsCTM().select(args);
}

template<class A, class R>
class NumberOfUnitsF{
  public:
  void operator()(A* a, R* res){
      res->Set(true, static_cast<int>(a->NumberOfUnits()));
  }
};

ValueMapping NumberOfUnitsValueMap[] ={
  GenVM1<PMPoint,CcInt, NumberOfUnitsF<PMPoint, CcInt> >
};

Operator pnumberOfUnits(
       "numberOfUnits",
        getNumberOfUnitsCTM().getSpecification(
         "numberOfUnits( _ )",
         "number of units within the repetition tree",
         "query numberUnits(pm1)"),
        getNumberOfUnitsCTM().getVMCount(),
       NumberOfUnitsValueMap,
       NumberOfUnitsSelect,
       NumberOfUnitsTypeMap);

/*
3.2.24 ~numberOfFlatUnits~

*/
static complexTM getNumberOfFlatUnitsCTM(){
   complexTM tm;
   tm.add(tm1<PMPoint,CcInt>());
   return tm;
}

static ListExpr NumberOfFlatUnitsTypeMap(ListExpr args){
   return getNumberOfFlatUnitsCTM()(args);
}

static int NumberOfFlatUnitsSelect(ListExpr args){
   return getNumberOfFlatUnitsCTM().select(args);
}

template<class A, class R>
class NumberOfFlatUnitsF{
  public:
  void operator()(A* a, R* res){
      res->Set(true, static_cast<int>(a->NumberOfFlatUnits()));
  }
};

ValueMapping NumberOfFlatUnitsValueMap[] ={
  GenVM1<PMPoint,CcInt, NumberOfFlatUnitsF<PMPoint, CcInt> >
};

Operator pnumberOfFlatUnits(
       "numberOfFlatUnits",
        getNumberOfFlatUnitsCTM().getSpecification(
         "numberOfFlatUnits( _ )",
         "number of units when the argument would be expanded",
         "query numberFlastUnits(pm1)"),
        getNumberOfFlatUnitsCTM().getVMCount(),
        NumberOfFlatUnitsValueMap,
        NumberOfFlatUnitsSelect,
        NumberOfFlatUnitsTypeMap);


/*
3.2.25 ~translate~

*/
static complexTM getTranslateCTM(){
   complexTM tm;
   tm.add(tm2<PMPoint,Duration,PMPoint>());
   tm.add(tm2<PMBool,Duration,PMBool>());
   tm.add(tm2<PMInt9M,Duration,PMInt9M>());
   tm.add(tm2<PMPoints,Duration,PMPoints>());
   return tm;
}

static ListExpr TranslateTypeMap(ListExpr args){
   return getTranslateCTM()(args);
}

static int TranslateSelect(ListExpr args){
   return getTranslateCTM().select(args);
}

template<class A1,class A2, class R>
class TranslateF{
  public:
  void operator()(A1* a1,A2* a2, R* res){
      a1->Translate(a2,*res);
    }
  };

ValueMapping TranslateValueMap[] ={
  GenVM2<PMPoint,DateTime,PMPoint, TranslateF<PMPoint,DateTime,PMPoint> >,
  GenVM2<PMBool,DateTime,PMBool, TranslateF<PMBool,DateTime,PMBool> >,
  GenVM2<PMInt9M,DateTime,PMInt9M, TranslateF<PMInt9M,DateTime,PMInt9M> >,
  GenVM2<PMPoints,DateTime,PMPoints, TranslateF<PMPoints,DateTime,PMPoints> >
};

Operator ptranslate(
       "ptranslate",             // name
       getTranslateCTM().getSpecification(
         " _ ptranslate [ _ ]",
         " translates the first argument in time",
         " query pm1 ptranslate [ d1 ] "),
       getTranslateCTM().getVMCount(),
       TranslateValueMap,
       TranslateSelect,
       TranslateTypeMap);



/*
3.2.27 ~minvalue~

*/
const string MinSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmreal -> real \" "
   " \" minvalue( _ ) \" "
   " \" Computes the minimum of the argument.  \"  "
   " \" \" "
   " <text> query minvalue(pmr1)  </text---> ))";

template<class A, class R>
class MinF{
   public:
   void operator()(A* a, R* res){
     a->min(*res);
   }
};

Operator min(
        "minvalue",
        MinSpec,
        GenVM1<PMReal, CcReal, MinF<PMReal, CcReal> >,
        Operator::SimpleSelect,
        TypeMap1<PMReal, CcReal>);

/*
3.2.29  ~maxvalue~

*/
const string MaxSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" \"Example\" )"
   " ( \"pmreal -> real \" "
   " \" maxvalue( _ ) \" "
   " \" Computes the maximum of the argument.  \"  "
   " \" \" "
   " <text> query maxvalue(pmr1)  </text---> ))";

template<class A, class R>
class MaxF{
   public:
   void operator()(A* a, R* res){
     a->max(*res);
   }
};

Operator max(
        "maxvalue",
        MaxSpec,
        GenVM1<PMReal, CcReal, MaxF<PMReal, CcReal> >,
        Operator::SimpleSelect,
        TypeMap1<PMReal, CcReal>);

} // namespace periodic

/*
3.3 Creating the Algebra

3.3.1 Definition of the PeriodicAlgebra

*/
class PeriodicMoveAlgebra : public Algebra
{
 public:
  PeriodicMoveAlgebra() : Algebra()
  {
    // type constructors
    AddTypeConstructor(&periodic::pbbox);
    periodic::pbbox.AssociateKind(Kind::DATA());
    AddTypeConstructor( &periodic::relinterval );
    periodic::relinterval.AssociateKind(Kind::DATA());
    AddTypeConstructor(&periodic::pmpoint);
    periodic::pmpoint.AssociateKind(Kind::DATA());
    AddTypeConstructor(&periodic::pmbool);
    periodic::pmbool.AssociateKind(Kind::DATA());
    AddTypeConstructor(&periodic::pmint9m);
    periodic::pmint9m.AssociateKind(Kind::DATA());
    AddTypeConstructor(&periodic::pinterval);
    periodic::pinterval.AssociateKind(Kind::DATA());
    AddTypeConstructor(&periodic::pmpoints);
    periodic::pmpoints.AssociateKind(Kind::DATA());
    AddTypeConstructor(&periodic::pmreal);
    periodic::pmreal.AssociateKind(Kind::DATA());
   // operators
    AddOperator(&periodic::pequals);
    AddOperator(&periodic::pcontains);
    AddOperator(&periodic::pintersects);
    AddOperator(&periodic::pintersection);
    AddOperator(&periodic::punion);
    AddOperator(&periodic::pat);
    AddOperator(&periodic::ptrajectory);
    AddOperator(&periodic::pbreakpoints);
    AddOperator(&periodic::plength);
    AddOperator(&periodic::plength2);
    AddOperator(&periodic::pstart);
    AddOperator(&periodic::pend);
    AddOperator(&periodic::pexpand);
    AddOperator(&periodic::pinitial);
    AddOperator(&periodic::pfinal);
    AddOperator(&periodic::createpmpoint);
    AddOperator(&periodic::ptoprel);
    AddOperator(&periodic::pdistance);
    AddOperator(&periodic::pnumberOfNodes);
    AddOperator(&periodic::pnumberOfCNodes);
    AddOperator(&periodic::pnumberOfPNodes);
    AddOperator(&periodic::pnumberOfUnits);
    AddOperator(&periodic::pnumberOfFlatUnits);
    AddOperator(&periodic::ptranslate);
    AddOperator(&periodic::pspeed);
    AddOperator(&periodic::pdirection);
    AddOperator(&periodic::min);
    AddOperator(&periodic::max);
  }
  ~PeriodicMoveAlgebra() {};
};




/*
3.3.2 Initialization


*/


extern "C"
Algebra*
InitializePeriodicAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  __TRACE__
  nl = nlRef;
  qp = qpRef;
  return (new PeriodicMoveAlgebra());
}
