/*

 STPatternAlgebra.cpp

 Created on: Jan 6, 2009
       Author: m.attia


*/

#include "STPatternAlgebra.h"
namespace STP{

inline int STVector::Count(int vec)
{
  int c=0;
  if(vec & aabb)    c++  ;
  if(vec & bbaa)    c++  ;
  if(vec & aa_bb)    c++  ;
  if(vec & bb_aa)    c++  ;
  if(vec & abab)    c++  ;
  if(vec & baba)    c++  ;
  if(vec & baab)    c++  ;
  if(vec & abba)    c++  ;
  if(vec & a_bab)    c++  ;
  if(vec & a_bba)    c++  ;
  if(vec & baa_b)    c++  ;
  if(vec & aba_b)    c++  ;
  if(vec & a_ba_b)  c++  ;
  if(vec & a_abb)    c++  ;
  if(vec & a_a_bb)  c++  ;
  if(vec & ba_ab)    c++  ;
  if(vec & bb_a_a)  c++  ;
  if(vec & bba_a)    c++  ;
  if(vec & b_baa)    c++  ;
  if(vec & b_b_aa)  c++  ;
  if(vec & ab_ba)    c++  ;
  if(vec & aa_b_b)  c++  ;
  if(vec & aab_b)    c++  ;
  if(vec & a_ab_b)  c++  ;
  if(vec & a_a_b_b)  c++  ;
  if(vec & b_ba_a)  c++  ;
  return c;
}
inline int STVector::Str2Simple(string s)
{
  if(s=="aabb") return  1 ;
  if(s=="bbaa") return  2 ;
  if(s=="aa.bb")return  4; if(s=="ab.ab")  return  4;
  if(s=="bb.aa")return  8; if(s=="ba.ba")  return  8;
  if(s=="abab") return  16;
  if(s=="baba") return  32;
  if(s=="baab") return  64;
  if(s=="abba") return  128;
  if(s=="a.bab")return  256; if(s=="b.aab")  return  256  ;
  if(s=="a.bba")return  512; if(s=="b.aba")  return  512  ;
  if(s=="baa.b")return  1024;if(s=="bab.a")  return  1024;
  if(s=="aba.b")return  2048;if(s=="abb.a")  return  2048;
  if(s=="a.ba.b")  return  4096  ; if(s=="a.bb.a")  return  4096;
  if(s=="b.aa.b")  return  4096  ;  if(s=="b.ab.a")  return  4096;
  if(s=="a.abb")  return  8192  ;
  if(s=="a.a.bb")  return  16384  ;  if(s=="a.b.ab")  return  16384;
  if(s=="b.a.ab")  return  16384  ;
  if(s=="ba.ab")  return  32768  ;
  if(s=="bb.a.a")  return  65536  ;  if(s=="ba.b.a")  return  65536;
  if(s=="ba.a.b")  return  65536  ;
  if(s=="bba.a")  return  131072  ;
  if(s=="b.baa")  return  262144  ;
  if(s=="b.b.aa")  return  524288  ;  if(s=="b.a.ba")  return  524288;
  if(s=="a.b.ba")  return  524288  ;
  if(s=="ab.ba")  return  1048576  ;    
  if(s=="aa.b.b")  return  2097152  ;  if(s=="ab.a.b")  return  2097152;
  if(s=="ab.b.a")  return  2097152  ;
  if(s=="aab.b")  return  4194304  ;
  if(s=="a.ab.b")  return  8388608  ;
  if(s=="a.a.b.b")return  16777216; if(s=="a.b.a.b")return  16777216;
  if(s=="a.b.b.a")return  16777216;  if(s=="b.b.a.a")return  16777216;
  if(s=="b.a.b.a")return  16777216;  if(s=="b.a.a.b")return  16777216;
  if(s=="b.ba.a")  return  33554432;
  return -1;
}
inline ListExpr STVector::Vector2List()
{
  ListExpr list;
  ListExpr last;
  int simple=1;
  int i=0;
  bool first=true;
  while(i<26 && first)
  {
    if(v & simple)  
    {
      list= nl->OneElemList(nl->SymbolAtom(StrSimpleConnectors[i]));
      last=list;
      first=false;
    }
    simple*=2;
    i++;
  }
  for(i=i; i<26; i++)
  {
    if(v & simple)
      last= nl->Append(last, nl->SymbolAtom(StrSimpleConnectors[i]));
      simple*=2;
  }
  return list;
}
bool STVector::Add(string simple)
{
  int vec= Str2Simple(simple);
  if(vec==-1) return false;
  v = v|vec;
  return true;
}

bool STVector::Add(int simple)
{
  if(simple <=  b_ba_a && Count(simple)==1) 
  {
    v = v|simple;
    return true;
  }
  return false;
}

Word STVector::In( const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));
  const string errMsg = "STVector::In: Expecting a simple temporal"
    " connector!";
  STVector* res=new STVector(0);
  NList list(instance), first;
  while(! list.isEmpty())
  {
    first= list.first();
    if(! res->Add(first.str()))
    {
      correct=false;
      cmsg.inFunError(errMsg);
      return result;
    }
    list.rest();
  }
  result.addr= res;
  return result;
}
ListExpr STVector::Out( ListExpr typeInfo, Word value )
{
  STVector* vec = static_cast<STVector*>( value.addr );
  return vec->Vector2List();
}
Word STVector::Create( const ListExpr typeInfo )
{
  return (SetWord( new STVector( 0)));
}
void STVector::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<STVector*>( w.addr );
  w.addr = 0;
}
bool STVector::Open( SmiRecord& valueRecord, size_t& offset, 
    const ListExpr typeInfo, Word& value ) 
{
  //cerr << "OPEN XRectangle" << endl;  
  size_t size = sizeof(int);   
  int vec;

  bool ok = true;
  ok = ok && valueRecord.Read( &vec, size, offset );
  offset += size;  
  value.addr = new STVector(vec); 
  return ok;
}
bool STVector::Save( SmiRecord& valueRecord, size_t& offset, 
    const ListExpr typeInfo, Word& value )
{  
  STVector* vec = static_cast<STVector*>( value.addr );  
  size_t size = sizeof(int);   

  bool ok = true;
  ok = ok && valueRecord.Write( &vec->v, size, offset );
  offset += size;  
  return ok;
}  
void STVector::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<STVector*>( w.addr );
  w.addr = 0;
}
Word STVector::Clone( const ListExpr typeInfo, const Word& w )
{
  STVector* vec = static_cast<STVector*>( w.addr );
  return SetWord( new STVector(*vec) );
}
int STVector::SizeOfObj()
{
  return sizeof(STVector);
}
ListExpr STVector::Property()
{
  return (nl->TwoElemList(
      nl->FiveElemList(nl->StringAtom("Signature"),
          nl->StringAtom("Example Type List"),
          nl->StringAtom("List Rep"),
          nl->StringAtom("Example List"),
          nl->StringAtom("Remarks")),
          nl->FiveElemList(nl->StringAtom("-> SIMPLE"),
              nl->StringAtom("stvector"),
              nl->StringAtom("(s1 s2 ...)"),
              nl->StringAtom("(1 128 32)"),
              nl->StringAtom(""))));
}
bool STVector::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "stvector" ));
}







TypeConstructor stvectorTC(
    "stvector",                       // name of the type in SECONDO
    STVector::Property,               // property function describing signature
    STVector::Out, STVector::In,      // Out and In functions
    0, 0,                             // SaveToList, RestoreFromList functions
    STVector::Create, STVector::Delete, // object creation and deletion
    STVector::Open, STVector::Save,   // object open, save
    STVector::Close, STVector::Clone, // close, and clone
    0,                                 // cast function
    STVector::SizeOfObj,              // sizeof function
    STVector::KindCheck );            // kind checking function


ListExpr CreateSTVectorTM(ListExpr args)
{
  bool debugme= false;
  string argstr;
  ListExpr rest= args, first;
  while (!nl->IsEmpty(rest)) 
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);
    nl->WriteToString(argstr, first);
    CHECK_COND (nl->IsAtom(first)&&  nl->SymbolValue(first)=="string",
        "Operator v: expects a list of strings but got '" +
        argstr + "'.\n" );
  }
  return nl->SymbolAtom("stvector");
}

ListExpr STPatternTM(ListExpr args)
{
  bool debugme= true;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  ListExpr tupleExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args);    //STConstraint list

  nl->WriteToString(argstr, tupleExpr);

  //  //checking for the first parameter tuple(x)
  CHECK_COND( (nl->ListLength(tupleExpr) == 2) &&
      (TypeOfRelAlgSymbol(nl->First(tupleExpr)) == tuple),
      "Operator stpattern: expects as first argument "
      "a list with structure (tuple ((a1 t1)...(an tn))).\n"
      "But got '" + argstr + "'.");

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  CHECK_COND( ! nl->IsAtom(NamedPredList) ,
      "Operator  stpattern expects as second argument a "
      "list of aliased lifted predicates.\n"
      "But got '" + argstr + "'.\n" );

  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    CHECK_COND
    ((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        nl->IsAtom(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Second(NamedPred))== "mbool"),
        "Operator stpattern: expects a list of aliased predicates. "
        "But got '" + argstr + "'.");
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    CHECK_COND
    ((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool"),
        "Operator stpattern: expects a list of temporal connectors. "
        "But got '" + argstr + "'.");
  }

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr STConstraintTM(ListExpr args)
{
  bool debugme= true;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  ListExpr alias1 = nl->First(args),   //tuple(x)
  alias2  = nl->Second(args),      //named predicate list
  temporalconnector = nl->Third(args);//STConstraint list

  nl->WriteToString(argstr, alias1);
  CHECK_COND(( nl->IsAtom(alias1)&&
      nl->SymbolValue(alias1)== "string"),
      "Operator stconstraint: expects a predicate label as first "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, alias2);
  CHECK_COND(( nl->IsAtom(alias2)&&
      nl->SymbolValue(alias2)== "string"),
      "Operator stconstraint: expects a predicate label as second "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, temporalconnector);
  CHECK_COND(( nl->IsAtom(temporalconnector)&&
      nl->SymbolValue(temporalconnector)== "stvector"),
      "Operator stconstraint: expects a temporal connector as third "
      "argument.\n But got '" + argstr + "'.");

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stconstraint accepted the input";
    cout.flush();
  }
  return result;
}

int CreateSTVectorVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int noconn= qp->GetNoSons(s);
  string simple;
  STVector* res= (STVector*) result.addr;
  for(int i=0;i<noconn; i++)
  {
    simple= ((CcString*)args[i].addr)->GetValue();
    if(! res->Add(simple))
      return 1;
  }
  return 0;
}

int STPatternVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  //  bool debugme=false;
  //  Word t, value;
  //  Tuple* tup;
  //  Supplier supplier,*supplier2,*supplier3;
  //  int nooffun;
  //  ArgVectorPointer*funargs;
  //  DateTime lastMatchTime(instanttype);
  //  bool matchPred;
  //  switch (message)
  //  {
  //  case OPEN :
  //
  //    qp->Open(args[0].addr);
  //    return 0;
  //
  //  case REQUEST :
  //    supplier = args[1].addr;
  //    nooffun = qp->GetNoSons(supplier);
  //    assert(nooffun>0);
  //    supplier2= new Supplier[nooffun];
  //    supplier3= new Supplier[nooffun];
  //    funargs=new ArgVectorPointer[nooffun];
  //
  //    for (int i=0; i < nooffun;i++)
  //    {
  //      supplier2[i] = qp->GetSupplier(supplier, i);
  //      supplier3[i] = qp->GetSupplier(supplier2[i], 1);
  //      funargs[i] = qp->Argument(supplier3[i]);
  //    }
  //
  //    qp->Request(args[0].addr,t);
  //    while (qp->Received(args[0].addr))
  //    {
  //      tup = (Tuple*)t.addr;
  //      lastMatchTime.ToMinimum();
  //      if(debugme)
  //        cout<<endl<< "Matching Tuple TID: "<<
  //          (int)tup->GetTupleId();
  //      for (int i=0; i < nooffun;i++)
  //      {
  //        ((*funargs[i])[0]).setAddr(tup);
  //        qp->Request(supplier3[i],value);
  //        matchPred= Match((MBool*)value.addr,
  //                lastMatchTime);
  //        if(debugme)
  //        {
  //          if(matchPred)
  //            cout<< " matched Pred "<< i <<
  //            " at time "<<
  //            lastMatchTime.ToDouble();
  //          else
  //            cout<< "didn't match Predicate "
  //              << i;
  //          cout.flush();
  //        }
  //        if(!matchPred) break;
  //      }
  //      if(!matchPred)
  //      {
  //        tup->DeleteIfAllowed();
  //        qp->Request(args[0].addr,t);
  //      }
  //      else
  //      {
  //        result.setAddr(tup);
  //        delete[] supplier2;
  //        delete[] supplier3;
  //        delete[] funargs;
  //        return YIELD;
  //      }
  //    }
  //    return CANCEL;
  //
  //  case CLOSE :
  //    qp->Close(args[0].addr);
  //    return 0;
  //  }
  return 0;
}

int STConstraintVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  return 0;
}

const string CreateSTVectorSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> (stringlist) -> stvector</text--->"
  "<text>v( _ )</text--->"
  "<text>Creates a vector temporal connector.</text--->"
  "<text>let meanwhile = v(\"abab\","
  "\"abba\",\"aba.b\")</text--->"
  ") )";

const string STPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> (stringlist) -> stvector</text--->"
  "<text>v( _ )</text--->"
  "<text>Creates a vector temporal connector.</text--->"
  "<text>let meanwhile = v(\"abab\","
  "\"abba\",\"aba.b\")</text--->"
  ") )";

const string STConstraintSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> (stringlist) -> stvector</text--->"
  "<text>v( _ )</text--->"
  "<text>Creates a vector temporal connector.</text--->"
  "<text>let meanwhile = v(\"abab\","
  "\"abba\",\"aba.b\")</text--->"
  ") )";

Operator createstvector (
    "v",    //name
    CreateSTVectorSpec,     //specification
    CreateSTVectorVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CreateSTVectorTM        //type mapping
);

Operator stpattern (
    "stpattern",    //name
    STPatternSpec,     //specification
    STPatternVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternTM        //type mapping
);

Operator stconstraint (
    "stconstraint",    //name
    STConstraintSpec,     //specification
    STConstraintVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STConstraintTM        //type mapping
);



class STPatternAlgebra : public Algebra
{
public:
  STPatternAlgebra() : Algebra()
  {

    /*

2.1 Registration of Types


     */
    AddTypeConstructor( &stvectorTC );

    /*
2.2 Registration of Operators

     */
    stpattern.SetRequestsArguments();
    AddOperator(&STP::createstvector);
    AddOperator(&STP::stpattern);
    AddOperator(&STP::stconstraint);
  }
  ~STPatternAlgebra() {};
};

};

/*
3 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>".

To link the algebra together with the system you must create an
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/



extern "C"
Algebra*
InitializeSTPatternAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new STP::STPatternAlgebra;
    }
