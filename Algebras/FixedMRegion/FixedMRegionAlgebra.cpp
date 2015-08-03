/*
This is my FixedMRegionAlgebra. It is supported by other classes.

*/
using namespace std;
#include "FixedMRegionAlgebra.h"
extern NestedList *nl;
extern QueryProcessor *qp;

/*
~Out~-function

*/
ListExpr OutPoint3( ListExpr typeInfo, Word value ){
  Point3* point = (Point3*)(value.addr);
  if( point->IsDefined())
    return nl->ThreeElemList(
      nl->RealAtom( point->GetX()),
      nl->RealAtom( point->GetY()),
      nl->RealAtom( point->GetAlpha()));
  else
    return nl->SymbolAtom( Symbol::UNDEFINED());
}

/*
~In~-function

*/
Word InPoint3( const ListExpr typeInfo, const ListExpr instance,
const int errorPos, ListExpr& errorInfo, bool& correct){
  correct = true;
  if( nl->ListLength( instance ) == 3 ) {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);

    correct = listutils::isNumeric(first) && 
              listutils::isNumeric(second) &&
              listutils::isNumeric(third);
    if(!correct){
      return SetWord( Address(0) );
    } else {
      return SetWord(new Point3(true, listutils::getNumValue(first),
                     listutils::getNumValue(second),
                     listutils::getNumValue(third)));
    }
  } else if( listutils::isSymbolUndefined( instance ) ){
    return SetWord(new Point3(false));
  }
  correct = false;
  return SetWord( Address(0) );
}

/*
~Create~-function
 
*/
Word
CreatePoint3( const ListExpr typeInfo ){
  return SetWord( new Point3( false ));
}

/*
~Delete~-function

*/
void DeletePoint3( const ListExpr typeInfo, Word& w){
  ((Point3 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
~Close~-function

*/
void ClosePoint3( const ListExpr typeInfo, Word& w){
  ((Point3 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
~Clone~-function

*/
Word ClonePoint3( const ListExpr typeInfo, const Word& w ){
  return SetWord( new Point3( *((Point3 *)w.addr) ) );
}

/*
~SizeOf~-function

*/
int SizeOfPoint3(){
  return sizeof(Point3);
}

/*
Function describing the signature of the type constructor

*/
ListExpr Point3Property(){
  return nl->TwoElemList(
              nl->FourElemList(
                  nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List")),
              nl->FourElemList(
                  nl->StringAtom("-> DATA"),
                  nl->StringAtom(Point3::BasicType()),
                  nl->StringAtom("(x y a)"),
                  nl->StringAtom("(10 5 0)")));
}

/*
Kind Checking Function
 
This function checks whether the type constructor is applied correctly. Since
type constructor ~point3~ does not have arguments, this is trivial.

*/
bool CheckPoint3( ListExpr type, ListExpr& errorInfo){
  return (listutils::isSymbol( type, Point3::BasicType() ));
}

/*
~Cast~-function

*/
void* CastPoint3(void* addr){
  return (new (addr) Point3());
}

/*
Creation of the type constructor instance


*/
TypeConstructor point3(
  Point3::BasicType(),  //name 
  Point3Property,//property function describing signature  
  OutPoint3,      InPoint3,     //Out and In functions 
  0,   0,      //SaveToList and RestoreFromList functions
  CreatePoint3,   DeletePoint3, //object creation and deletion
  OpenAttribute<Point3>,
  SaveAttribute<Point3>,  // object open and save 
  ClosePoint3,    ClonePoint3,  //object close, and clone
  CastPoint3,                  //cast function   
  SizeOfPoint3,                //sizeof function
  CheckPoint3);               //kind checking function

/*
function Describing the Signature of the Type Constructor

*/
ListExpr UMoveProperty(){
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("("+UMove::BasicType()+") "),
      nl->TextAtom(
        "(timeInterval(real_x0 real_y0 real_a0 real_x1 real_y1 real_a1))"),
      nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 0.5 2.5 2.1 0.1))"))));
}

/*
Kind Checking Function

*/
bool CheckUMove( ListExpr type, ListExpr& errorInfo){
  return (nl->IsEqual( type, UMove::BasicType() ));
}

/*
~Out~-function

*/
ListExpr OutUMove( ListExpr typeInfo, Word value){
  UMove* umove = (UMove*)(value.addr);
  if( !(((UMove*)value.addr)->IsDefined()) )
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  else{
    ListExpr timeintervalList = nl->FourElemList(
      OutDateTime( nl->TheEmptyList(),
      SetWord(&umove->timeInterval.start) ),
      OutDateTime( nl->TheEmptyList(), SetWord(&umove->timeInterval.end) ),
      nl->BoolAtom( umove->timeInterval.lc ),
      nl->BoolAtom( umove->timeInterval.rc));
    ListExpr pointsList = nl->SixElemList(
      nl->RealAtom( umove->p0.GetX() ),
      nl->RealAtom( umove->p0.GetY() ),
      nl->RealAtom( umove->p0.GetAlpha() ),
      nl->RealAtom( umove->p1.GetX() ),
      nl->RealAtom( umove->p1.GetY() ),
      nl->RealAtom( umove->p1.GetAlpha() ));
    return nl->TwoElemList( timeintervalList, pointsList );  
  }
}

/*
~In~-function

*/
Word InUMove( const ListExpr typeInfo, const ListExpr instance,
const int errorPos, ListExpr& errorInfo, bool& correct ){
  string errmsg;
  if ( nl->ListLength( instance ) == 2 ){
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType ){

      correct = true;
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
              nl->First( first ), errorPos, errorInfo, correct ).addr;

      if( !correct || start == NULL || !start->IsDefined()){
        errmsg = "InUMove(): first instant must be defined!.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
                      nl->Second( first ),
                      errorPos, errorInfo, correct ).addr;

      if( !correct  || end == NULL || !end->IsDefined() ){
        errmsg = "InUMove(): second instant must be defined!.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if (!correct){
          errmsg = "InUMove(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
      }

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 6 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == RealType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == RealType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == RealType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == RealType &&
          nl->IsAtom( nl->Fifth( second ) ) &&
          nl->AtomType( nl->Fifth( second ) ) == RealType &&
          nl->IsAtom( nl->Sixth( second ) ) &&
          nl->AtomType( nl->Sixth( second ) ) == RealType ){
        UMove *umove = new UMove( tinterval,
                                     nl->RealValue( nl->First( second ) ),
                                     nl->RealValue( nl->Second( second ) ),
                                     nl->RealValue( nl->Third( second ) ),
                                     nl->RealValue( nl->Fourth( second ) ),
                                     nl->RealValue( nl->Fifth( second ) ),
                                     nl->RealValue( nl->Sixth( second ) ) );

        correct = umove->IsValid();
        if( correct )
          return SetWord( umove );

        errmsg = "InUMove(): Error in start/end point.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete umove;
      }
    }
  }
  else if ( listutils::isSymbolUndefined(instance) ){
      UMove *umove = new UMove(true);
      umove->SetDefined(false);
      umove->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = umove->timeInterval.IsValid();
      if ( correct )
        return (SetWord( umove ));
    }
  errmsg = "InUMove(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
~Create~-function

*/
Word CreateUMove( const ListExpr typeInfo ){
  return (SetWord( new UMove(false) ));
}

/*
~Delete~-function

*/
void DeleteUMove( const ListExpr typeInfo, Word& w ){
  delete (UMove *)w.addr;
  w.addr = 0;
}

/*
~Close~-function

*/
void CloseUMove( const ListExpr typeInfo, Word& w ){
  delete (UMove *)w.addr;
  w.addr = 0;
}

/*
~Clone~-function

*/
Word CloneUMove( const ListExpr typeInfo, const Word& w ){
  UMove *umove = (UMove *)w.addr;
  return SetWord( new UMove( *umove ) );
}

/*
~Sizeof~-function

*/
int SizeOfUMove(){
  return sizeof(UMove);
}

/*
~Cast~-function

*/
void* CastUMove(void* addr){
  return new (addr) UMove;
}

/*
Creation of the type constructor ~upoint~

*/
TypeConstructor unitmove(
        UMove::BasicType(),      //name
        UMoveProperty,               //property function describing signature
        OutUMove,     InUMove, //Out and In functions
        0,             0,  //SaveToList and RestoreFromList functions
        CreateUMove,
        DeleteUMove, //object creation and deletion
        OpenAttribute<UMove>,
        SaveAttribute<UMove>,  // object open and save
        CloseUMove,   CloneUMove, //object close and clone
        CastUMove, //cast function
        SizeOfUMove, //sizeof function
        CheckUMove );                    //kind checking function

/*
function Describing the Signature of the Type Constructor

*/
ListExpr MMoveProperty(){
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mmove) "),
                             nl->StringAtom("( u1 ... un ) "),
        nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)"))));
}

/*
Kind Checking Function

*/
bool CheckMMove( ListExpr type, ListExpr& errorInfo ){
  return (nl->IsEqual( type, MMove::BasicType() ));
}

/*
Creation of the type constructor ~mpoint~

*/
TypeConstructor movingmove(
        MMove::BasicType(),   //name
        MMoveProperty,        //property function describing signature
        OutMapping<MMove, UMove, OutUMove>,
        InMapping<MMove, UMove, InUMove>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<MMove>,
        DeleteMapping<MMove>,     //object creation and deletion
        OpenAttribute<MMove>,
        SaveAttribute<MMove>,      // object open and save
        CloseMapping<MMove>,
        CloneMapping<MMove>, //object close and clone
        CastMapping<MMove>,    //cast function
        SizeOfMapping<MMove>, //sizeof function
        CheckMMove );  //kind checking function

/*
function Describing the Signature of the Type Constructor

*/
ListExpr FixedMRegionProperty(){
  return 
    nl->TwoElemList(
      nl->FourElemList(
        nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List")),
      nl->FourElemList (
        nl->StringAtom("-> MAPPING"),
        nl->StringAtom("fixedmregion"),
        nl->StringAtom("(fixedmregion real) = (x,y)"),
        nl->StringAtom ("(fmr 2.0)")
      )
    );
}

/*
This is the in function.

*/
Word InFixedMRegion(const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr & errorInfo, bool & correct){
  if(nl->ListLength(instance) != 4){
    string strErrorMessage = "FMRA: List length must be 4";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }
  
  Word reg_addr= InRegion(typeInfo,nl->First(instance),errorPos,
    errorInfo,correct);
  Region* cr = (Region*)reg_addr.addr; 
  
  Word mm_addr= InMapping<MMove, UMove, InUMove>(
    typeInfo,nl->Second(instance),errorPos,errorInfo,correct);
  MMove* mm = (MMove*)mm_addr.addr; 
  
  Word p_addr= InPoint(typeInfo,nl->Third(instance),errorPos,
    errorInfo,correct);
  Point* p = (Point*)p_addr.addr; 
  
  double t = listutils::getNumValue(nl->Fourth(instance));
    
  FixedMRegion* fmr = new FixedMRegion(*cr, *mm, *p, t);
  
  correct = true;
  return SetWord(fmr);
}

/*
This is the out function.

*/
ListExpr OutFixedMRegion(ListExpr typeInfo, Word value){
  FixedMRegion* fmr = (FixedMRegion*)(value.addr);
  if(!fmr->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  Region cr(0);
  fmr->getRegion(cr) ;
  ListExpr regionNL = OutRegion(nl->TheEmptyList(), SetWord(&cr));

  MMove mm(fmr->getMove());
  ListExpr mmoveNL = OutMapping<MMove, UMove, OutUMove>(
    nl->TheEmptyList(), SetWord(&mm));

  Point p=fmr->getRotCenter();
  ListExpr pointNL = OutPoint(nl->TheEmptyList(), SetWord(&p));

  double t=fmr->gett();

  return nl->FourElemList(regionNL, mmoveNL, pointNL, nl->RealAtom(t));
}

/*
This is the create function.

*/
Word CreateFixedMRegion(const ListExpr typeInfo){
  Word w;
  w.addr = (new FixedMRegion());
  return w;
}

/*
This is the delete function.

*/
void DeleteFixedMRegion(const ListExpr typeInfo, Word & w){
  FixedMRegion *k = (FixedMRegion *) w.addr;
  delete k;
  w.addr = 0;
}

/*
This is the save function.

*/
bool SaveFixedMRegion(SmiRecord & valueRecord, size_t & offset,
  const ListExpr typeInfo, Word & value){
  FixedMRegion* mr = static_cast<FixedMRegion*> (value.addr);
  Attribute::Save(valueRecord, offset, typeInfo, mr);
  return true;
}

/*
This is the open function.

*/
bool OpenFixedMRegion(SmiRecord & valueRecord, size_t & offset, 
  const ListExpr typeInfo, Word & value){
  value = SetWord(Attribute::Open(valueRecord, offset, typeInfo));
  return true;
}

/*
This is the close function.

*/
void CloseFixedMRegion(const ListExpr typeInfo, Word & w){
  delete (FixedMRegion*) w.addr;
  w.addr = 0;
}

/*
This is the clone function.

*/
Word CloneFixedMRegion(const ListExpr typeInfo, const Word & w){
  return SetWord(((FixedMRegion*) w.addr)->Clone());
}

/*
This is the cast function.

*/
void * CastFixedMRegion(void *addr){
  return new (addr) FixedMRegion;
}

/*
This is the sizeof function.

*/
int SizeOfFixedMRegion(){
  int s = sizeof(FixedMRegion);
  return s;
}

/*
This is the type checking functin.

*/
bool FixedMRegionTypeCheck(ListExpr type, ListExpr & errorInfo){
  return nl->IsEqual(type, MRegion::BasicType())
        || nl->IsEqual(type, "fixedmovingregion");
}

/*
This is the constructor.

*/
TypeConstructor FixedMRegionTC (FixedMRegion::BasicType (), 
                                     FixedMRegionProperty, 
                                     OutFixedMRegion, 
                                     InFixedMRegion, 0, 0,
                                     CreateFixedMRegion,
                                     DeleteFixedMRegion,
                                     OpenFixedMRegion,
                                     SaveFixedMRegion,
                                     CloseFixedMRegion,
                                     CloneFixedMRegion,
                                     CastFixedMRegion,
                                     SizeOfFixedMRegion,
                                     FixedMRegionTypeCheck);

/*
This is the type mapping function.

*/
ListExpr testoperatoraTM (ListExpr args){
  string err = "one int is expected";
  if (!nl->HasLength (args, 1)){
    return listutils::typeError (err + " (wrong number of arguments)");  
  }
  return listutils::basicSymbol < CcInt > ();
}

/*
This is the value mapping function.

*/
int testoperatoraVM (Word * args, Word & result, int message,
Word & local, Supplier s){
  runTestMethod ();
  runFixedMTestMethod ();
  result = qp->ResultStorage (s);
  CcInt *res __attribute__ ((unused)) = (CcInt *) result.addr;
  res->Set (true, 2);
  return 1;
}

/*
This is the operator specification function.

*/
OperatorSpec testoperatoraSpec ("int -> int",
            "testoperatora(_)",
            "Computes nothing of an int and returns an int.",
            "query testoperatora(0)");

/*
This is the operator function.

*/
Operator testoperatoraOp ("testoperatora",
                          testoperatoraSpec.getStr (),
                          testoperatoraVM,
                          Operator::SimpleSelect, testoperatoraTM);


/*
This is the type mapping function.

*/
ListExpr AtInstantTM(ListExpr args){
  string err = "region and time expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!FixedMRegion::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcReal::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<Region>();
}

/*
This is the value mapping function.

*/
int AtInstantVM(Word* args, Word& result, int message,
Word& local, Supplier s){
  FixedMRegion* fmr = (FixedMRegion*) args[0].addr;
  double* d = (double*) args[1].addr;
  double e = *d;
  Region r(0);
  fmr->atinstant(e, r);
  
  result = qp->ResultStorage(s);
  Region* res  __attribute__ ((unused))= (Region*) result.addr;
  res = &r;
  return 0;  
}

/*
This is the operator specification function.

*/
OperatorSpec AtInstantSpec(
"fixedmregion x double -> region",
"_ atinstant _",
"Computes the fmr at a given time.",
"query [mregion1] atinstant [instant1])"
);

/*
This is the operator function.

*/
Operator AtInstantOp(
"atinstant",
AtInstantSpec.getStr(),
AtInstantVM,
Operator::SimpleSelect,
AtInstantTM
);

/*
This is the type mapping function.

*/
ListExpr TraversedTM(ListExpr args){
  string err = "region and start and end time are expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!FixedMRegion::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!CcReal::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  if(!CcReal::checkType(nl->Third(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<Region>();
}

/*
This is the value mapping function.

*/
int TraversedVM(Word* args, Word& result, int message,
Word& local, Supplier s){
  FixedMRegion* fmr = (FixedMRegion*) args[0].addr;
  double* d = (double*) args[1].addr;
  double t_start = *d;
  double* e = (double*) args[2].addr;
  double t_end = *e;
  Region r(0);
  fmr->traversedNew(r, t_start, t_end);
  result = qp->ResultStorage(s);
  Region* res  __attribute__ ((unused))= (Region*) result.addr;
  res = &r;
  return 0;  
}

/*
This is the operator specification function.

*/
OperatorSpec TraversedSpec(
"fixedmregion x double -> region",
"fmr_traversed _ _ _",
"Computes the fmr at a given time.",
"query fmr_traversed ( [mregion1] , [instant1] , [instant2])"
);

/*
This is the operator function.

*/
Operator TraversedOp(
"fmr_traversed",
TraversedSpec.getStr(),
TraversedVM,
Operator::SimpleSelect,
TraversedTM
);

/*
This is the type mapping function.

*/
ListExpr fmr_InsideTM(ListExpr args){
  string err = "fixedmregion and mpoint are expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!FixedMRegion::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!MPoint::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<MBool>();
}

/*
This is the value mapping function.

*/
int fmr_InsideVM(Word* args, Word& result, int message,
Word& local, Supplier s){
  FixedMRegion* fmr = (FixedMRegion*) args[0].addr;
  MPoint* d = (MPoint*) args[1].addr;
  
  MBool r(0);
  r = fmr->inside(*d);
  
  result = qp->ResultStorage(s);
  MBool* res  __attribute__ ((unused))= (MBool*) result.addr;
  res = &r;
  return 0;  
}

/*
This is the operator specification function.

*/
OperatorSpec fmr_InsideSpec(
"fixedmregion x mpoint -> mbool",
"_ inside _",
"Computes the times t which the mpoint is inside.",
"query [mregion1] inside [mpoint]"
);

/*
This is the operator function.

*/
Operator fmr_InsideOp(
"inside",
fmr_InsideSpec.getStr(),
fmr_InsideVM,
Operator::SimpleSelect,
fmr_InsideTM
);

/*
This is the type mapping function.

*/
ListExpr IntersectionTM(ListExpr args){
  string err = "fixedmregion and mpoint are expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!FixedMRegion::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(!MPoint::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<MPoint>();
}

/*
This is the value mapping function.

*/
int IntersectionVM(Word* args, Word& result, int message,
Word& local, Supplier s){
  FixedMRegion* fmr = (FixedMRegion*) args[0].addr;
  MPoint* d = (MPoint*) args[1].addr;
  MPoint r(0);
  r = fmr->intersection(*d);
  result = qp->ResultStorage(s);
  MPoint* res  __attribute__ ((unused))= (MPoint*) result.addr;
  res = &r;
  return 0;
}

/*
This is the operator specification function.

*/
OperatorSpec IntersectionSpec(
"fixedmregion x mpoint -> mbool",
"_ fmr_intersection _",
"Computes the movement where the mpoint is inside.",
"query [mregion1] fmr_intersection [mpoint]"
);

/*
This is the operator function.

*/
Operator IntersectionOp(
"fmr_intersection",
IntersectionSpec.getStr(),
IntersectionVM,
Operator::SimpleSelect,
IntersectionTM
);

/*
This is the type mapping function.

*/
ListExpr InterpolateTM(ListExpr args){
  string err = "fixedmregion and mpoint are expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!Region::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<FixedMRegion>();
}

/*
This is the value mapping function.

*/
int InterpolateVM(Word* args, Word& result, int message,
Word& local, Supplier s){
  Word tuple;
  FMRInterpolator *fmri=(FMRInterpolator*)local.addr;
  switch(message){
    case OPEN:
      qp->Open(args[0].addr);
      fmri=new FMRInterpolator();
      fmri->start();
      local.setAddr(fmri);
      return 0;
    case REQUEST:
      qp->Request(args[0].addr, tuple);
      if(qp->Received(args[0].addr)){
        IRegion* ir = (IRegion*) tuple.addr;
        fmri->addObservation(*ir);
      }
      return CANCEL;
    case CLOSE:
      if(local.addr){
        fmri->end();
        FixedMRegion r = fmri->getResult();
        result = qp->ResultStorage(s);
        FixedMRegion* res = (FixedMRegion*) result.addr;
        *res = r;
        delete fmri;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return YIELD;
  }
  return 0;
}

/*
This is the operator specification function.

*/
OperatorSpec InterpolateSpec(
"region[] -> fixedmregion",
"interpolate _",
"Computes the fixedmregion.",
"query interpolate [region[]]"
);

/*
This is the operator function.

*/
Operator InterpolateOp(
"interpolate",
InterpolateSpec.getStr(),
InterpolateVM,
Operator::SimpleSelect,
InterpolateTM
);

/*
This is the algebra.

*/
class FixedMRegionAlgebra:public Algebra{
public:
/*  
This is the constructor.

*/
  FixedMRegionAlgebra ():Algebra (){
    AddTypeConstructor(&point3);
    AddTypeConstructor(&unitmove);
    AddTypeConstructor(&movingmove);
    AddTypeConstructor(&FixedMRegionTC);
    unitmove.AssociateKind( Kind::TEMPORAL() );
    unitmove.AssociateKind( Kind::DATA() );
    unitmove.AssociateKind( "SPATIAL3D" ); 
    movingmove.AssociateKind( Kind::TEMPORAL() );
    movingmove.AssociateKind( Kind::DATA() );
    point3.AssociateKind(Kind::DATA());
    FixedMRegionTC.AssociateKind(Kind::TEMPORAL());
    FixedMRegionTC.AssociateKind(Kind::DATA());

    AddOperator(&testoperatoraOp);
    AddOperator(&AtInstantOp);
    AddOperator(&TraversedOp);
    AddOperator(&fmr_InsideOp);
    AddOperator(&IntersectionOp);
    AddOperator(&InterpolateOp);
  }
/*  
This is the destructor.

*/
   ~FixedMRegionAlgebra (){};
};

/*  
This is the initialization.

*/
extern "C"
Algebra* InitializeFixedMRegionAlgebra(NestedList* nlRef,
  QueryProcessor *qpRef) {
  nl = nlRef;
  qp = qpRef;
  return new FixedMRegionAlgebra();
}

;
