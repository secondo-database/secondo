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

//paragraph [1] title: [{\Large \bf ]  [}]


[1] TupleIdentifier Algebra

March 2005 Matthias Zielke

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

The only purpose of this algebra is to provide a typeconstructor 'tid' so that the tupleidentifiers
of tuples from relations can be stored as attribute-values in different tuples. This feature is needed
for the implementation of operators to update relations.

1 Preliminaries

1.1 Includes

*/


#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "ListUtils.h"
#include <string>

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;


/*

2 Type Constructor ~tid~

*/

void TupleIdentifier::CopyFrom(const Attribute* attr)
{
  const TupleIdentifier* tupleI = (const TupleIdentifier*) attr;
  SetDefined(tupleI->IsDefined());
  tid = tupleI->GetTid();
}

bool TupleIdentifier::Adjacent( const Attribute* arg ) const
{
  TupleId argTid = ((const TupleIdentifier *)arg)->GetTid();

  return( tid == argTid -1 || tid == argTid + 1 );
}

TupleIdentifier::TupleIdentifier(bool DEFINED, TupleId TID):
Attribute(DEFINED), tid(TID)
{
}

TupleIdentifier::TupleIdentifier(const TupleIdentifier& source):
  Attribute(source.IsDefined()), tid(source.tid){
}

TupleIdentifier::~TupleIdentifier() {}

TupleId TupleIdentifier::GetTid() const {return tid;}

void TupleIdentifier::SetTid(TupleId TID)
{tid = TID; SetDefined(true);}

TupleIdentifier* TupleIdentifier::Clone() const
{ return new TupleIdentifier( *this ); }


ostream& TupleIdentifier::Print(ostream& out) const
{
  out << tid;
  return  out;
}


/*
2.2 List Representation

The list representation of a TupleIdentifier is

----    (tid)
----

2.3 ~In~ and ~Out~ Functions

*/

ListExpr
OutTupleIdentifier( ListExpr typeInfo, Word value )
{
  TupleIdentifier* tupleI = (TupleIdentifier*)(value.addr);
  return nl->IntAtom(tupleI->GetTid());
}

Word
InTupleIdentifier( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom(instance) && nl->AtomType(instance) == IntType)
  {
    correct = true;
    TupleIdentifier* newTid = new TupleIdentifier(true, nl->IntValue(instance));
    return SetWord(newTid);
  }
  correct = false;
  return SetWord(Address(0));
}

/*
2.4 Functions Describing the Signature of the Type Constructors

This one works for type constructor ~tid~.

*/
ListExpr
TupleIdentifierProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
              nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List"),
                  nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
              nl->StringAtom(TupleIdentifier::BasicType()),
                  nl->StringAtom("(<tid>)"),
                  nl->StringAtom("("+TupleIdentifier::Example()+")"),
                  nl->StringAtom("The tupleidentifier is a long."))));
}
Word
CreateTupleIdentifier( const ListExpr typeInfo )
{
  return (SetWord( new TupleIdentifier(true, 0) ));
}

void
DeleteTupleIdentifier( const ListExpr typeInfo, Word& w )
{
  delete (TupleIdentifier *)w.addr;
  w.addr = 0;
}

void
CloseTupleIdentifier( const ListExpr typeInfo, Word& w )
{
  delete (TupleIdentifier *)w.addr;
  w.addr = 0;
}

Word
CloneTupleIdentifier( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((TupleIdentifier *)w.addr)->Clone() );
}

int
SizeOfTupleIdentifier()
{
  return sizeof(TupleIdentifier);
}

/*
2.5 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~TupleIdentifier~ does not have arguments, this is trivial.

*/
bool
CheckTupleIdentifier( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, TupleIdentifier::BasicType() ));
}

/*
2.6 Creation of the Type Constructor Instance

*/
TypeConstructor tupleIdentifier
(
 TupleIdentifier::BasicType(),
   //name
 TupleIdentifierProperty,
   //property function describing signature
 OutTupleIdentifier, InTupleIdentifier,
   //Out and In functions
 0, 0,
   //SaveToList and RestoreFromList functions
 CreateTupleIdentifier, DeleteTupleIdentifier,
   //object creation and deletion
 0, 0, CloseTupleIdentifier, CloneTupleIdentifier,
   //object open,save,close,clone
 TupleIdentifier::Cast,
   //cast function
 SizeOfTupleIdentifier,
   //sizeof function
 CheckTupleIdentifier );
   //kind checking function

/*
3 Operators

3.1 Operator ~tupleid~

Returns the tuple identifier.

3.1.1 Type mapping function of operator ~tupleid~

Operator ~tupleid~ accepts a tuple and returns an integer.

----    (tuple x)           -> tid
----

*/
ListExpr
TupleIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
   return listutils::typeError("one argument expected");
  }

  ListExpr tuple = nl->First(args);
  if(!listutils::isTupleDescription(tuple)){
    return listutils::typeError("tuple expected");
  }
  return nl->SymbolAtom(TupleIdentifier::BasicType());
}

/*
3.1.2 Value mapping function of operator ~tupleid~

*/
int
TIDTupleId(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Tuple* t = (Tuple*)args[0].addr;
  result = qp->ResultStorage(s);
  ((TupleIdentifier *) result.addr)->SetTid( t->GetTupleId() );
  return 0;
}

/*
Additional functions for integration in ~jlist~

*/

ListExpr TupleIdentifier::Out(ListExpr typeInfo, Word value)
{
  TupleIdentifier* tupleI = (TupleIdentifier*)(value.addr);
  if (tupleI->IsDefined())
    return nl->IntAtom(tupleI->GetTid());
  else
    return nl->SymbolAtom("undef");
}

Word TupleIdentifier::In(const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if ( nl->IsAtom(instance))
  {
    if ( nl->AtomType(instance) == IntType)
    {
      correct = true;
      TupleIdentifier* newTid =
        new TupleIdentifier(true, nl->IntValue(instance));
      return SetWord(newTid);
    }
    else
    {
      if (nl->AtomType (instance) == SymbolType)
      {
        correct = true;
        TupleIdentifier* newTid =
                new TupleIdentifier(false, 0);
        return SetWord(newTid);
      }
      else
      {
        correct = false;
        return SetWord(Address(0));
      }
    }
  }
  else
  {
    correct = false;
    return SetWord(Address(0));
  }
}

bool TupleIdentifier::Save(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value )
{
  int t;
  TupleIdentifier* obj = (TupleIdentifier*) value.addr;
  if (obj->IsDefined())
  {
    t = obj->GetTid();
  }
  else
  {
    t = -1;
  }
  valueRecord.Write(&t, sizeof(int), offset);
  offset += sizeof(int);
  return true;
}

bool TupleIdentifier::Open(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value )
{
  int tid;
  valueRecord.Read(&tid, sizeof(int), offset);
  offset += sizeof(int);
  if (tid > -1)
    value = SetWord(new TupleIdentifier(true, tid));
  else
    value = SetWord(new TupleIdentifier(false, 0));
  return true;
}

TupleIdentifier& TupleIdentifier::operator=(const TupleIdentifier& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined()) tid = other.GetTid();
  return *this;
}

bool TupleIdentifier::operator==(const TupleIdentifier& other) const
{
  return (Compare(other) == 0);
}

/*
3.1.3 Specification of operator ~tupleid~

*/
const string TupleIdSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" \" \" \" \" ) "
  "( <text>(tuple x) -> int</text--->"
  "<text>tupleid( _ )</text--->"
  "<text>Returns the identification of the tuple.</text--->"
  "<text>query cities feed filter[ tupleid(.) < 100 ]"
  " consume</text--->"
  "<text>Apply tupleid(_) directly after a feed, because </text--->"
  "<text>other operators my corrupt the tid </text--->"
  "<text>(in-memory tuples all have tid=0).</text--->"
  ") )";

/*
3.1.4 Definition of operator ~tupleid~

*/
Operator tidtupleid (
         "tupleid",             // name
         TupleIdSpec,           // specification
         TIDTupleId,            // value mapping
         Operator::SimpleSelect,         // trivial selection function
         TupleIdTypeMap         // type mapping
);

/*
3.2 Operator ~addtupleid~

Appends the tuple identifier as an attribute in the stream of tuples.

3.2.1 Type mapping function of operator ~addtupleid~

Operator ~addtupleid~ accepts a stream of tuples and returns the same stream
with the tuple identifier attribute in the end.

----    (stream (tuple ((x1 t1) ... (xn tn))))   ->
        (stream (tuple ((x1 t1) ... (xn tn) (id tid))))
----

*/
ListExpr
AddTupleIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    return listutils::typeError("One argument expected");
  }

  ListExpr stream = nl->First(args);
  if(!listutils::isTupleStream(stream)){
   return listutils::typeError("stream(tuple(...)) expected");
  }

  set<string> names;
  ListExpr rest = nl->Second(nl->Second(stream));
  ListExpr head = nl->OneElemList(nl->First(rest));
  names.insert(nl->SymbolValue(nl->First(nl->First(rest))));
  ListExpr last = head;
  rest = nl->Rest(rest);
  while(!nl->IsEmpty(rest)){
    last = nl->Append(last, nl->First(rest));
    names.insert(nl->SymbolValue(nl->First(nl->First(rest))));
    rest = nl->Rest(rest);
  }

  if(names.find("TID")!=names.end()){
   return listutils::typeError("Attr name 'TID' already exists");
  }
  last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TID"),
                                nl->SymbolAtom(TupleIdentifier::BasicType())));

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                         head));
}

ListExpr TIDtid2intTypeMap (ListExpr args)
{
  string error = "TID expected";
  if( (nl->ListLength(args) == 1) && (nl->IsEqual(nl->First(args),
                                      TupleIdentifier::BasicType()))){
    return nl->SymbolAtom(CcInt::BasicType());
  }
  ErrorReporter::ReportError(error);
  return nl->TypeError();
}

ListExpr TIDint2tidTypeMap (ListExpr args)
{
  string error = "int expected";
  if( (nl->ListLength(args) == 1) && (nl->IsEqual(
                                    nl->First(args),CcInt::BasicType()))){
    return nl->SymbolAtom(TupleIdentifier::BasicType());
  }
  ErrorReporter::ReportError(error);
  return nl->TypeError();
}

/*
3.2.2 Value mapping function of operator ~addtupleid~

*/
int
TIDAddTupleId(Word* args, Word& result, int message, Word& local, Supplier s)
{
  TupleType *resultTupleType;
  ListExpr resultType;
  Word t;

  switch (message)
    {
    case OPEN :

      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;

    case REQUEST :

      resultTupleType = (TupleType *)local.addr;
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        Tuple *tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        assert( newTuple->GetNoAttributes() == tup->GetNoAttributes() + 1 );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->PutAttribute( i, tup->GetAttribute( i )->Clone() );
        newTuple->PutAttribute( newTuple->GetNoAttributes() - 1,
                          new TupleIdentifier(true,tup->GetTupleId()));

        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      if(local.addr)
      {
        ((TupleType *)local.addr)->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
3.2.3 Specification of operator ~addtupleid~

*/
const string AddTupleIdSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "( <text>(stream (tuple ((x1 t1) ... (xn tn)))) ->"
  "(stream (tuple ((x1 t1) ... (xn tn) (TID tid))))</text--->"
  "<text>_ addtupleid</text--->"
  "<text>Appends the tuple identifier in the tuple type</text--->"
  "<text>query cities feed addtupleid consume</text--->"
  "<text>Apply addtupleid directly after a feed, because other "
  "operators my corrupt the tid. All in-memory tuples all have tid=0."
  "</text--->"
  ") )";


/*
3.2.4 Definition of operator ~addtupleid~

*/
Operator tidaddtupleid (
         "addtupleid",             // name
         AddTupleIdSpec,           // specification
         TIDAddTupleId,            // value mapping
         Operator::SimpleSelect,         // trivial selection function
         AddTupleIdTypeMap         // type mapping
);


/*
3.3 Operator ~=~

Compares two TupleIdentifiers and returns TRUE, iff they are equal.

3.3.1 Type mapping function of operator ~=~

----    (tid tid) -> bool
----

*/

ListExpr
EqualTupleIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=2){
   return listutils::typeError("two arguments expected");
  }

  if(!listutils::isSymbol(nl->First(args),TupleIdentifier::BasicType()) ||
     !listutils::isSymbol(nl->Second(args),TupleIdentifier::BasicType())){
    return listutils::typeError("tid x tid expected");
  }

  return nl->SymbolAtom(CcBool::BasicType());

}


/*
3.3.2 Value mapping function of operators ~=, \#, $<$, $>$, $\leq$, $\geq$~

Comparison operators

----
  Operator   op
  <          0
  <=         1
  =          2
  >=         3
  >          4
  #          5
----

*/

template<int op>
int TIDCompareTupleId( Word* args, Word& result,
                       int message, Word& local, Supplier s )
{
  assert((op >= 0) && (op <=5));
  result = qp->ResultStorage( s );
  const TupleIdentifier* a = static_cast<const TupleIdentifier*>(args[0].addr);
  const TupleIdentifier* b = static_cast<const TupleIdentifier*>(args[1].addr);

  //int cmp = a->Compare(b);
  switch (op)
  {
    case 0: // <
      ((CcBool *)result.addr)->Set( true, a->Compare(b) < 0 );
      return (0);
    case 1: // <=
      ((CcBool *)result.addr)->Set( true, a->Compare(b) <= 0 );
      return (0);
    case 2: // =
      ((CcBool *)result.addr)->Set( true, a->Compare(b) == 0 );
      return (0);
    case 3: // >=
      ((CcBool *)result.addr)->Set( true, a->Compare(b) >= 0 );
      return (0);
    case 4: // >
      ((CcBool *)result.addr)->Set( true, a->Compare(b) > 0 );
      return (0);
    case 5: // #
      ((CcBool *)result.addr)->Set( true, a->Compare(b) != 0 );
      return (0);
  }
  // ERROR:
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

int TIDtid2intVM ( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt *res = static_cast<CcInt*>(result.addr);
  TupleIdentifier* a = static_cast<TupleIdentifier*>(args[0].addr);
  res->Set(a->IsDefined(),static_cast<int>(a->GetTid()));
  return 0;
}

int TIDint2tidVM ( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  TupleIdentifier *res = static_cast<TupleIdentifier*>(result.addr);
  CcInt* a = static_cast<CcInt*>(args[0].addr);
  res->Set(a->IsDefined(),static_cast<TupleId>(a->GetIntval()));
  return 0;
}


/*
3.3.3 Specification of operators ~=, \#, $<$, $>$, $\leq$, $\geq$~

*/

const string EqualTupleIdSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Result\" \"Comment\" ) "
  "( <text>(tid tid) -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Returns TRUE, iff both tuple identifiers are equal (i.e "
  "both refer to the same tuple).</text--->"
  "<text>query plz feed head[4] loopsel[plz_Ort exactmatchS[.Ort]] {A}\n"
  "plz feed head[4] loopsel[plz_Ort  exactmatchS[.Ort]] {B}\n"
  "symmjoin[.id_A = ..id_B] count</text--->"
  "<text>2336</text--->"
  "<text>Caution: Only compare TIDs referring to the same relation! "
  "All in-memory tuples have tid=0.</text--->"
  ") )";

const string NequalTupleIdSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>(tid tid) -> bool</text--->"
    "<text>_ # _</text--->"
    "<text>Returns TRUE, iff both tuple identifiers are different (i.e "
    "both refer to different tuples).</text--->"
    "<text>query plz feed head[4] loopsel[plz_Ort exactmatchS[.Ort]] {A}\n"
    "plz feed head[4] loopsel[plz_Ort  exactmatchS[.Ort]] {B}\n"
    "symmjoin[.id_A # ..id_B] count</text--->"
    "<text>338720</text--->"
    "<text>Caution: Only compare TIDs referring to the same relation! "
    "All in-memory tuples have tid=0.</text--->"
    ") )";

const string LessTupleIdSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>(tid tid) -> bool</text--->"
    "<text>_ < _</text--->"
    "<text>Returns TRUE, iff the first tuple identifier is less than "
    "the second one.</text--->"
    "<text>query plz feed head[4] loopsel[plz_Ort exactmatchS[.Ort]] {A}\n"
    "plz feed head[4] loopsel[plz_Ort  exactmatchS[.Ort]] {B}\n"
    "symmjoin[.id_A < ..id_B] count</text--->"
    "<text>169360</text--->"
    "<text>Caution: Only compare TIDs referring to the same relation! "
    "All in-memory tuples have tid=0.</text--->"
    ") )";

const string GreaterTupleIdSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>(tid tid) -> bool</text--->"
    "<text>_ > _</text--->"
    "<text>Returns TRUE, iff the first tuple identifier is greater than "
    "the second one.</text--->"
    "<text>query plz feed head[4] loopsel[plz_Ort exactmatchS[.Ort]] {A}\n"
    "plz feed head[4] loopsel[plz_Ort  exactmatchS[.Ort]] {B}\n"
    "symmjoin[.id_A > ..id_B] count</text--->"
    "<text>169360</text--->"
    "<text>Caution: Only compare TIDs referring to the same relation! "
    "All in-memory tuples have tid=0.</text--->"
    ") )";

const string LeqTupleIdSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>(tid tid) -> bool</text--->"
    "<text>_ <= _</text--->"
    "<text>Returns TRUE, iff the first tuple identifier is less or equal than "
    "the second one.</text--->"
    "<text>query plz feed head[4] loopsel[plz_Ort exactmatchS[.Ort]] {A}\n"
    "plz feed head[4] loopsel[plz_Ort  exactmatchS[.Ort]] {B}\n"
    "symmjoin[.id_A <= ..id_B] count</text--->"
    "<text>171696</text--->"
    "<text>Caution: Only compare TIDs referring to the same relation! "
    "All in-memory tuples have tid=0.</text--->"
    ") )";

const string GeqTupleIdSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>(tid tid) -> bool</text--->"
    "<text>_ < _</text--->"
    "<text>Returns TRUE, iff the first tuple identifier is greater or equal "
    "than the second one.</text--->"
    "<text>query plz feed head[4] loopsel[plz_Ort exactmatchS[.Ort]] {A}\n"
    "plz feed head[4] loopsel[plz_Ort  exactmatchS[.Ort]] {B}\n"
    "symmjoin[.id_A >= ..id_B] count</text--->"
    "<text>171696</text--->"
    "<text>Caution: Only compare TIDs referring to the same relation! "
    "All in-memory tuples have tid=0.</text--->"
    ") )";

const string Tid2IntSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>tid -> int</text--->"
    "<text>tid2int( _ )</text--->"
    "<text>Converts the TID to an int value.</text--->"
    "<text>query ten feed addid extend[tidint: tid2int(.TID)] "
    "extract[tidint]</text--->"
    "<text>1</text--->"
    "<text>Caution: Possible problems due to different value spaces.</text--->"
    ") )";

const string Int2TidSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Result\" \"Comment\" ) "
    "( <text>int -> tid</text--->"
    "<text>int2tid( _ )</text--->"
    "<text>Converts the int to a tid.</text--->"
    "<text>query int2tid(5) feed transformstream ten gettuples extract[no]"
    "</text--->"
    "<text>5</text--->"
    "<text>Caution: Possible problems due to different value spaces. "
    "NEVER use this operator!</text--->"
    ") )";

/*
3.3.4 Definition of operators ~=, \#, $<$, $>$, $\leq$, $\geq$~

*/

Operator tidless (
         "<",                      // name
         LessTupleIdSpec,          // specification
         TIDCompareTupleId<0>,     // value mapping
         Operator::SimpleSelect,   // trivial selection function
         EqualTupleIdTypeMap       // type mapping
                 );

Operator tidleq (
         "<=",                     // name
         LeqTupleIdSpec,           // specification
         TIDCompareTupleId<1>,     // value mapping
         Operator::SimpleSelect,   // trivial selection function
         EqualTupleIdTypeMap       // type mapping
                );

Operator tidequal (
         "=",                      // name
         EqualTupleIdSpec,         // specification
         TIDCompareTupleId<2>,     // value mapping
         Operator::SimpleSelect,   // trivial selection function
         EqualTupleIdTypeMap       // type mapping
);

Operator tidgeq (
         ">=",                      // name
         GeqTupleIdSpec,           // specification
         TIDCompareTupleId<3>,     // value mapping
         Operator::SimpleSelect,   // trivial selection function
         EqualTupleIdTypeMap       // type mapping
                  );

Operator tidgreater (
         ">",                      // name
         GreaterTupleIdSpec,       // specification
         TIDCompareTupleId<4>,     // value mapping
         Operator::SimpleSelect,   // trivial selection function
         EqualTupleIdTypeMap       // type mapping
                  );

Operator tidnequal (
         "#",                      // name
         NequalTupleIdSpec,        // specification
         TIDCompareTupleId<5>,     // value mapping
         Operator::SimpleSelect,   // trivial selection function
         EqualTupleIdTypeMap       // type mapping
                  );

Operator tidtid2int (
         "tid2int",                // name
         Tid2IntSpec,              // specification
         TIDtid2intVM,             // value mapping
         Operator::SimpleSelect,   // trivial selection function
         TIDtid2intTypeMap         // type mapping
                  );

Operator tidint2tid (
         "int2tid",                // name
         Int2TidSpec,              // specification
         TIDint2tidVM,             // value mapping
         Operator::SimpleSelect,   // trivial selection function
         TIDint2tidTypeMap         // type mapping
                  );

/*
5 Creating the Algebra

*/

class TupleIdentifierAlgebra : public Algebra
{
 public:
  TupleIdentifierAlgebra() : Algebra()
  {
    AddTypeConstructor( &tupleIdentifier );
    tupleIdentifier.AssociateKind( Kind::DATA() );

    AddOperator( &tidtupleid );
    AddOperator( &tidaddtupleid );
    AddOperator( &tidequal );
    AddOperator( &tidnequal );
    AddOperator( &tidless );
    AddOperator( &tidleq );
    AddOperator( &tidgreater );
    AddOperator( &tidgeq );
    AddOperator( &tidtid2int );
    AddOperator( &tidint2tid );
  }
  ~TupleIdentifierAlgebra() {};
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
InitializeTupleIdentifierAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new TupleIdentifierAlgebra());
}


