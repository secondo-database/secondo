/*
This is my algebra. It is supportes by other classes in this 
folder.

*/
using namespace std;
#include "FixedMRegionAlgebra.h"
extern NestedList *nl;
extern QueryProcessor *qp;

class FixedMRegionPoint:public Attribute
{
public:
FixedMRegionPoint (double _x = 0, double _y = 0):
  Attribute (true), x (_x), y (_y){}
  ~FixedMRegionPoint (){}
  static const string BasicType (){
    return "pstpoint";
  }
  static const bool checkType (const ListExpr list)
  {
    return listutils::isSymbol (list, BasicType ());
  };

  bool operator== (const FixedMRegionPoint & _b)
  {
    if ((x == _b.x) && (y == _b.y))
      {
        return true;
      }
    return false;
  }
  double getX ()
  {
    return x;
  }
  double getY ()
  {
    return y;
  }
  void set (double _x, double _y)
  {
    x = _x;
    y = _y;
  }
  int NumOfFLOBs () const
  {
    return 0;
  }
  Flob *GetFLOB (const int i)
  {
    assert (false);
    return 0;
  }
  int Compare (const Attribute * arg) const
  {
    FixedMRegionPoint *c = (FixedMRegionPoint *) arg;
    if (x < c->getX ())
        return -1;
    if (x > c->getX ())
        return 1;
    if (y < c->getY ())
        return -1;
    if (y > c->getY ())
        return 1;
      return 0;
  }
  bool Adjacent (const Attribute * arg) const
  {
    return false;
  }
  FixedMRegionPoint *Clone () const
  {
    FixedMRegionPoint *res = new FixedMRegionPoint (x, y);
      return res;
  }
  size_t Sizeof () const
  {
    return sizeof (*this);
  }
  void CopyFrom (const Attribute * arg)
  {
    FixedMRegionPoint *p = (FixedMRegionPoint *) arg;
    x = p->getX ();
    y = p->getY ();
  }
  size_t HashValue () const
  {
    return (size_t) (x + y);
  }
private:double x;
  double y;
};


ListExpr
FixedMRegionPointProperty ()
{
  return nl->TwoElemList (nl->FourElemList (nl->StringAtom ("Signature"),
                                            nl->StringAtom
                                            ("Example Type List"),
                                            nl->StringAtom ("List Rep"),
                                            nl->StringAtom
                                            ("Example List")),
                          nl->FourElemList (nl->StringAtom ("-> DATA"),
                                            nl->StringAtom
                                            (FixedMRegionPoint::BasicType
                                             ()),
                                            nl->StringAtom
                                            ("(real real) = (x,y)"),
                                            nl->StringAtom ("(13.5 -76.0)")));
}


Word
InFixedMRegionPoint (const ListExpr typeInfo,
                     const ListExpr instance,
                     const int errorPos, ListExpr & errorInfo, bool & correct)
{
  Word res ((void *) 0);
  correct = false;
  if (!nl->HasLength (instance, 2))
    {
      return res;
    }
  if (!listutils::isNumeric (nl->First (instance))
      || !listutils::isNumeric (nl->Second (instance)))
    {
      return res;
    }
  double x = listutils::getNumValue (nl->First (instance));
  double y = listutils::getNumValue (nl->Second (instance));

  correct = true;
  res.addr = new FixedMRegionPoint (x, y);
  return res;
}

ListExpr
OutFixedMRegionPoint (ListExpr typeInfo, Word value)
{
  FixedMRegionPoint *k = (FixedMRegionPoint *) value.addr;
  return nl->TwoElemList (nl->RealAtom (k->getX ()), nl->RealAtom (k->getY ()));
}

Word
CreateFixedMRegionPoint (const ListExpr typeInfo)
{
  Word w;
  w.addr = (new FixedMRegionPoint (0, 0));
  return w;
}

void
DeleteFixedMRegionPoint (const ListExpr typeInfo, Word & w)
{
  FixedMRegionPoint *k = (FixedMRegionPoint *) w.addr;
  delete k;
  w.addr = 0;
}

bool
SaveFixedMRegionPoint (SmiRecord & valueRecord, size_t & offset,
                       const ListExpr typeInfo, Word & value)
{
  FixedMRegionPoint *k = static_cast < FixedMRegionPoint * >(value.addr);
  size_t size = sizeof (double);
  double v = k->getX ();
  bool ok = valueRecord.Write (&v, size, offset);
  offset += size;
  v = k->getY ();
  ok = ok && valueRecord.Write (&v, size, offset);
  offset += size;
  return ok;
}

bool
OpenFixedMRegionPoint (SmiRecord & valueRecord,
                       size_t & offset, const ListExpr typeInfo, Word & value)
{
  size_t size = sizeof (double);
  double x, y;
  bool ok = valueRecord.Read (&x, size, offset);
  offset += size;
  ok = ok && valueRecord.Read (&y, size, offset);
  offset += size;
  if (ok)
    {
      value.addr = new FixedMRegionPoint (x, y);
    }
  else
    {
      value.addr = 0;
    }
  return ok;
}

void
CloseFixedMRegionPoint (const ListExpr typeInfo, Word & w)
{
  FixedMRegionPoint *k = (FixedMRegionPoint *) w.addr;
  delete k;
  w.addr = 0;
}

Word
CloneFixedMRegionPoint (const ListExpr typeInfo, const Word & w)
{
  FixedMRegionPoint *k = (FixedMRegionPoint *) w.addr;
  Word res;
  res.addr = new FixedMRegionPoint (k->getX (), k->getY ());
  return res;
}

void *
CastFixedMRegionPoint (void *addr)
{
  return (new (addr) FixedMRegionPoint);
}

int
SizeOfFixedMRegionPoint ()
{
  return 2 * sizeof (double);
}

bool
FixedMRegionPointTypeCheck (ListExpr type, ListExpr & errorInfo)
{
  return nl->IsEqual (type, FixedMRegionPoint::BasicType ());
}



TypeConstructor FixedMRegionPointTC (FixedMRegionPoint::BasicType (), 
                                     FixedMRegionPointProperty, 
                                     OutFixedMRegionPoint, 
                                     InFixedMRegionPoint, 0, 0,
                                     CreateFixedMRegionPoint,
                                     DeleteFixedMRegionPoint,
                                     OpenFixedMRegionPoint,
                                     SaveFixedMRegionPoint,
                                     CloseFixedMRegionPoint,
                                     CloneFixedMRegionPoint,
                                     CastFixedMRegionPoint,
                                     SizeOfFixedMRegionPoint,
                                     FixedMRegionPointTypeCheck);

/*
4.2 List Representation
The list representation of a point3 is
(x y alpha)

4.3 ~Out~-function

*/
ListExpr
OutPoint3( ListExpr typeInfo, Word value )
{
  Point3* point = (Point3*)(value.addr);
  if( point->IsDefined() )
    return nl->ThreeElemList(
               nl->RealAtom( point->GetX() ),
               nl->RealAtom( point->GetY() ),
               nl->RealAtom( point->GetAlpha() ));
  else
    return nl->SymbolAtom( Symbol::UNDEFINED() );
}

/*
4.4 ~In~-function

*/
Word
InPoint3( const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct )
{
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
4.5 ~Create~-function
 
*/
Word
CreatePoint3( const ListExpr typeInfo )
{
  return SetWord( new Point3( false ) );
}

/*
4.6 ~Delete~-function

*/
void
DeletePoint3( const ListExpr typeInfo,
             Word& w )
{
  ((Point3 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.7 ~Close~-function

*/
void
ClosePoint3( const ListExpr typeInfo,
            Word& w )
{
  ((Point3 *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.8 ~Clone~-function

*/
Word
ClonePoint3( const ListExpr typeInfo,
            const Word& w )
{
  return SetWord( new Point3( *((Point3 *)w.addr) ) );
}

/*
4.8 ~SizeOf~-function

*/
int
SizeOfPoint3()
{
  return sizeof(Point3);
}

/*
4.9 Function describing the signature of the type constructor

*/
ListExpr
Point3Property()
{
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
4.10 Kind Checking Function
 
This function checks whether the type constructor is applied correctly. Since
type constructor ~point3~ does not have arguments, this is trivial.

*/
bool
CheckPoint3( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, Point3::BasicType() ));
}

/*
4.11 ~Cast~-function

*/
void* CastPoint3(void* addr)
{
  return (new (addr) Point3());
}

/*
4.12 Creation of the type constructor instance


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
4.9 Type Constructor ~upoint~

Type ~upoint~ represents an (tinterval, (x0, y0, x1, y1))-pair.

4.9.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 yo x1 y1) )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

4.9.2 function Describing the Signature of the Type Constructor

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
4.9.3 Kind Checking Function

*/
bool
CheckUMove( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UMove::BasicType() ));
}

/*
4.9.4 ~Out~-function

*/
ListExpr OutUMove( ListExpr typeInfo, Word value )
{
  UMove* umove = (UMove*)(value.addr);

  if( !(((UMove*)value.addr)->IsDefined()) )
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  else
    {
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
4.9.5 ~In~-function

The Nested list form is like this:( ( 6.37  9.9  TRUE FALSE) (1.0 2.3 4.1 2.1) )

*/
Word InUMove( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {

      correct = true;
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
              nl->First( first ), errorPos, errorInfo, correct ).addr;

      if( !correct || start == NULL || !start->IsDefined())
      {
//        "InUMove(): Error in first instant (Must be defined!).";
        errmsg = "InUMove(): first instant must be defined!.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
                      nl->Second( first ),
                      errorPos, errorInfo, correct ).addr;

      if( !correct  || end == NULL || !end->IsDefined() )
      {
//        errmsg = "InUMove(): Error in second instant (Must be defined!).";
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
      if (!correct)
        {
          errmsg = "InUMove(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 4 &&
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
          nl->AtomType( nl->Sixth( second ) ) == RealType )
      {
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
  else if ( listutils::isSymbolUndefined(instance) )
    {
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
4.9.6 ~Create~-function

*/
Word CreateUMove( const ListExpr typeInfo )
{
  return (SetWord( new UMove(false) ));
}

/*
4.9.7 ~Delete~-function

*/
void DeleteUMove( const ListExpr typeInfo, Word& w )
{
  delete (UMove *)w.addr;
  w.addr = 0;
}

/*
4.9.8 ~Close~-function

*/
void CloseUMove( const ListExpr typeInfo, Word& w )
{
  delete (UMove *)w.addr;
  w.addr = 0;
}

/*
4.9.9 ~Clone~-function

*/
Word CloneUMove( const ListExpr typeInfo, const Word& w )
{
  UMove *umove = (UMove *)w.addr;
  return SetWord( new UMove( *umove ) );
}

/*
4.9.10 ~Sizeof~-function

*/
int SizeOfUMove()
{
  return sizeof(UMove);
}

/*
4.9.11 ~Cast~-function

*/
void* CastUMove(void* addr)
{
  return new (addr) UMove;
}

/*
4.9.12 Creation of the type constructor ~upoint~

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
4.12 Type Constructor ~mpoint~

Type ~mpoint~ represents a moving point.

4.12.1 List Representation

The list representation of a ~mpoint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~upoint~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 2.1) )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) (4.1 2.1 8.9 4.3) )
        )
----

4.12.2 function Describing the Signature of the Type Constructor

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
4.12.3 Kind Checking Function

*/
bool CheckMMove( ListExpr type, ListExpr& errorInfo ){
  return (nl->IsEqual( type, MMove::BasicType() ));
}

/*
4.12.4 Creation of the type constructor ~mpoint~

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
This is my connection to my class FixedMRegion.

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
        nl->StringAtom("(fixedmregion)"),
        nl->StringAtom("(fixedmregion real) = (x,y)"),
        nl->StringAtom ("(fmr 2.0)")
      )
    );
}

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

ListExpr OutFixedMRegion(ListExpr typeInfo, Word value){
  FixedMRegion* fmr = (FixedMRegion*)(value.addr);
  if(!fmr->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  Region cr(0);
  fmr->getRegion(cr) ;//);
  ListExpr regionNL = OutRegion(nl->TheEmptyList(), SetWord(&cr));

  MMove mm(fmr->getMove());
  ListExpr mmoveNL = OutMapping<MMove, UMove, OutUMove>(
    nl->TheEmptyList(), SetWord(&mm));

  Point p=fmr->getRotCenter();
  ListExpr pointNL = OutPoint(nl->TheEmptyList(), SetWord(&p));

  double t=fmr->gett();

  return nl->FourElemList(regionNL, mmoveNL, pointNL, nl->RealAtom(t));
}

Word CreateFixedMRegion(const ListExpr typeInfo){
  Word w;
  w.addr = (new FixedMRegion());
  return w;
}

void DeleteFixedMRegion(const ListExpr typeInfo, Word & w){
  FixedMRegion *k = (FixedMRegion *) w.addr;
  delete k;
  w.addr = 0;
}

bool SaveFixedMRegion(SmiRecord & valueRecord, size_t & offset,
  const ListExpr typeInfo, Word & value){
  FixedMRegion* mr = static_cast<FixedMRegion*> (value.addr);
  Attribute::Save(valueRecord, offset, typeInfo, mr);
  return true;
}

bool OpenFixedMRegion(SmiRecord & valueRecord, size_t & offset, 
  const ListExpr typeInfo, Word & value){
  value = SetWord(Attribute::Open(valueRecord, offset, typeInfo));
  return true;
}

void CloseFixedMRegion(const ListExpr typeInfo, Word & w){
  delete (FixedMRegion*) w.addr;
  w.addr = 0;
}

Word CloneFixedMRegion(const ListExpr typeInfo, const Word & w){
  return SetWord(((FixedMRegion*) w.addr)->Clone());
}

void * CastFixedMRegion(void *addr){
  return new (addr) FixedMRegion;
}

int SizeOfFixedMRegion(){
  int s = sizeof(FixedMRegion);
  return s;
}

bool FixedMRegionTypeCheck(ListExpr type, ListExpr & errorInfo){
  return nl->IsEqual(type, MRegion::BasicType())
        || nl->IsEqual(type, "fixedmovingregion");
}

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
This is the end of class FixedMRegion.

*/

ListExpr
testoperatoraTM (ListExpr args)
{
  string err = "one int is expected";
  if (!nl->HasLength (args, 1))
    {
      return listutils::typeError (err + " (wrong number of arguments)");
    }
  return listutils::basicSymbol < CcInt > ();
}

int
testoperatoraVM (Word * args, Word & result, int message,
                 Word & local, Supplier s)
{

  runTestMethod ();
  runFixedMTestMethod ();

  result = qp->ResultStorage (s);
  CcInt *res __attribute__ ((unused)) = (CcInt *) result.addr;

  res->Set (true, 2);
  return 1;
}

OperatorSpec testoperatoraSpec ("int -> int",
            "testoperatora(_)",
            "Computes nothing of an int and returns an int.",
            "query testoperatora(0)");

Operator testoperatoraOp ("testoperatora",
                          testoperatoraSpec.getStr (),
                          testoperatoraVM,
                          Operator::SimpleSelect, testoperatoraTM);

/*
Start of operator

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
OperatorSpec AtInstantSpec(
"fixedmregion x double -> region",
"_ atinstant _",
"Computes the fmr at a given time.",
"query [mregion1] atinstant [instant1])"
);

Operator AtInstantOp(
"atinstant",
AtInstantSpec.getStr(),
AtInstantVM,
Operator::SimpleSelect,
AtInstantTM
);
/*
Start of operator

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
OperatorSpec TraversedSpec(
"fixedmregion x double -> region",
"fmr_traversed _ _ _",
"Computes the fmr at a given time.",
"query fmr_traversed ( [mregion1] , [instant1] , [instant2])"
);

Operator TraversedOp(
"fmr_traversed",
TraversedSpec.getStr(),
TraversedVM,
Operator::SimpleSelect,
TraversedTM
);
/*
Start of operator

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
OperatorSpec fmr_InsideSpec(
"fixedmregion x mpoint -> mbool",
"_ inside _",
"Computes the times t which the mpoint is inside.",
"query [mregion1] inside [mpoint]"
);

Operator fmr_InsideOp(
"inside",
fmr_InsideSpec.getStr(),
fmr_InsideVM,
Operator::SimpleSelect,
fmr_InsideTM
);
/*
Start of operator

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
OperatorSpec IntersectionSpec(
"fixedmregion x mpoint -> mbool",
"_ fmr_intersection _",
"Computes the movement where the mpoint is inside.",
"query [mregion1] fmr_intersection [mpoint]"
);

Operator IntersectionOp(
"fmr_intersection",
IntersectionSpec.getStr(),
IntersectionVM,
Operator::SimpleSelect,
IntersectionTM
);

/*
Start of operator

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

int InterpolateVM(Word* args, Word& result, int message,
Word& local, Supplier s){
  //Region* fmr = (Region*) args[0].addr;//FIXME
  vector<IRegion> fmr(0);
  FMRInterpolator fi;
  
  result = qp->ResultStorage(s);
  FixedMRegion* res  __attribute__ ((unused))= (FixedMRegion*) result.addr;
  FixedMRegion r(fi.interpolate(fmr));
  res->CopyFrom(&r);
  return 0;
  }
//FIXME
OperatorSpec InterpolateSpec(
"region[] -> fixedmregion",
"interpolate _",
"Computes the fixedmregion.",
"query interpolate [region[]]"
);

Operator InterpolateOp(
"interpolate",
InterpolateSpec.getStr(),
InterpolateVM,
Operator::SimpleSelect,
InterpolateTM
);

class FixedMRegionAlgebra:public Algebra
{
public:
  FixedMRegionAlgebra ():Algebra ()
  {
    AddTypeConstructor (&FixedMRegionPointTC);
    AddTypeConstructor(&point3);
    AddTypeConstructor(&unitmove);
    AddTypeConstructor(&movingmove);
    AddTypeConstructor(&FixedMRegionTC);
    unitmove.AssociateKind( Kind::TEMPORAL() );
    unitmove.AssociateKind( Kind::DATA() );
    unitmove.AssociateKind( "SPATIAL3D" ); 
    movingmove.AssociateKind( Kind::TEMPORAL() );
    movingmove.AssociateKind( Kind::DATA() );
    FixedMRegionPointTC.AssociateKind (Kind::DATA ());
    point3.AssociateKind(Kind::DATA());
//    point3.AssociateKind(Kind::SPATIAL());
    FixedMRegionTC.AssociateKind(Kind::TEMPORAL());
    FixedMRegionTC.AssociateKind(Kind::DATA());

    AddOperator(&testoperatoraOp);
    AddOperator(&AtInstantOp);
    AddOperator(&TraversedOp);
    AddOperator(&fmr_InsideOp);
    AddOperator(&IntersectionOp);
    AddOperator(&InterpolateOp);

  }
   ~FixedMRegionAlgebra (){};
};


extern "C"
Algebra* InitializeFixedMRegionAlgebra(NestedList* nlRef,
  QueryProcessor *qpRef) {
  nl = nlRef;
  qp = qpRef;
  return new FixedMRegionAlgebra();
}

;
