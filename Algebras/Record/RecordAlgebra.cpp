
/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/


#include <string>
#include "Algebra.h"
#include "Attribute.h"
#include "NestedList.h"
#include "NList.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "RelationAlgebra.h"

#include "Record.h"
#include "ListIterator.h"
#include "Stream.h"

//#define RECORD_DEBUG
#undef RECORD_DEBUG

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace mappings;
using namespace std;

namespace record {

/*

1 Typkonstruktor for record

*/

TypeConstructor recordTC(
  Record::BasicType(),                        // name of the type in SECONDO
  Record::Property,                // property function describing signature
  Record::Out, Record::In,         // Out and In functions
  0, 0,                            // SaveToList, RestoreFromList functions
  Record::Create, Record::Delete,  // object creation and deletion
  OpenAttribute<Record>, SaveAttribute<Record>,  // object open, save
  Record::Close, Record::Clone,    // close, and clone
  Record::Cast,                    // cast function
  Record::SizeOfObj,               // sizeof function
  Record::KindCheck );             // kind checking function

/*
2 Operator ~createRecord~
Creates a record object with the listed elements.
Elements have to be kind DATA.

2.1 Type mapping of operator ~createRecord~

*/

ListExpr
createRecordTM(ListExpr args) {

  ListExpr elem, resultList, errorInfo, rest;
  string elemname, type;
  vector<string> elemnamelist;
  int unique;

#ifdef RECORD_DEBUG
  cerr<<endl<<"RecordAlgebra::createRecordTM start.";
    cerr<<" ("<<nl->ToString(args)<<")"<<endl;
#endif

  if ( nl->ListLength(args) == 0 )
  {
    ErrorReporter::ReportError("Operator needs at least one element.");
    return nl->TypeError();
  }

  rest = nl->First(args);
  unique = 0;
  while ( !nl->IsEmpty(rest) )
  {
    if ( nl->IsAtom(rest) ) {
      ErrorReporter::ReportError("Each Element must consist of id and type");
      return nl->TypeError();
    }
    elem = nl->First(rest);
    rest = nl->Rest(rest);

    if (nl->ListLength(elem) == 2)
    {
      if ((nl->IsAtom(nl->First(elem))) &&
         (nl->AtomType(nl->First(elem)) == SymbolType))
      {
        elemname = nl->SymbolValue(nl->First(elem));

        // check name convention for element names
        char * copy = new char[elemname.size() + 1];
        strcpy(copy, elemname.c_str());
        if ( isupper(*copy) == 0 )
        {
          ErrorReporter::ReportError("element name has to start with a capital "
                                     "letter: " + elemname);
          delete[] copy;
          return nl->TypeError();
        }
        delete[] copy;

        // check if name is a typename.
        if (SecondoSystem::GetCatalog()->IsTypeName(elemname))
        {
          ErrorReporter::ReportError(elemname + " is not allowed as element "
                                      "name. Maybe there exists an object"
                                      "with this name in the database. ");
          return nl->TypeError();
        }

        unique = std::count(elemnamelist.begin(),
                            elemnamelist.end(),
                            elemname);
        if (unique > 0)
        {
          ErrorReporter::ReportError("Doubly defined element name "
                                     + elemname);
          return nl->TypeError();
        }
        elemnamelist.push_back(elemname);

      } else { //nl->First(elem) not an atom and Type is not SymbolType
        ErrorReporter::ReportError("Invalid element name "
                                   + nl->ToString(nl->First(elem)));
        return nl->TypeError();
      }
    } else { //ListLength(elem) != 2
      ErrorReporter::ReportError("Invalid element definition "
                                 + nl->ToString(elem));
      return nl->TypeError();
    }

    //pruefen ob es sich um einen gueltigen Typen handelt
    if (nl->IsAtom(nl->Second(elem)))
    {
      type = nl->SymbolValue(nl->Second(elem));
    }
    else
    {
      type = nl->SymbolValue(nl->First(nl->Second(elem)));
    }
    if (!SecondoSystem::GetCatalog()->IsTypeName(type))
    {
      ErrorReporter::ReportError(type + " is not a type.");
      return nl->TypeError();
    }
    //check type to be kind DATA
    errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
    if( !am->CheckKind(Kind::DATA(), nl->Second(elem), errorInfo) ) {
      ErrorReporter::ReportError("Elements have to be kind DATA");
      return nl->TypeError();
    }
  }

  resultList = nl->Cons(nl->SymbolAtom(Record::BasicType()), nl->First(args));
#ifdef RECORD_DEBUG
  cerr<<"RecordAlgebra::createRecordTM end.";
    cerr<<"returns: "<<nl->ToString(resultList)<<endl<<endl;
#endif

  return resultList;
}

/*
2.2 Value mapping of operator ~createRecord~

*/

int
createRecordVM(Word* args, Word& result,int message, Word& local, Supplier s)
{
  Attribute* elem;
  Record* record;
  Supplier son;
  ListExpr name, type;
  int sons;
  Word value;

#ifdef RECORD_DEBUG
  cerr<<endl<<"RecordAlgebra::createRecordVM start."<<endl;
#endif

  result = qp->ResultStorage(s);
  record = static_cast<Record*>(result.addr);

  sons = qp->GetNoSons(args[0].addr);
  for (int i = 0; i< sons; i++)
  {
    son = qp->GetSupplier(args[0].addr, i);
    value = qp->Request(qp->GetSon(son,1));
    elem = (Attribute*)value.addr;
    name = qp->GetType(qp->GetSon(son,0));
    type = qp->GetType(qp->GetSon(son,1));
    if (!nl->IsAtom(type)) {
      type = nl->First(type);
    }

#ifdef RECORD_DEBUG
cerr<<"RecordAlgebra::createRecordVM Type = "<<nl->ToString(type)<<endl;
#endif

    record->AppendElement(elem, nl->ToString(type), nl->ToString(name));
  }

  result.addr = record;

#ifdef RECORD_DEBUG
  cerr<<"RecordAlgebra::createRecordVM end."<<endl<<endl;
#endif

  return 0;
}

/*
3 Operator ~attr~
Returns an element from a record object.

3.1 Type Mapping of Operator ~attr~

*/

ListExpr
attrTM(ListExpr args)
{
  ListExpr first, second, elemList, elemType;
  string elemName;
  int elemIndex;

#ifdef RECORD_DEBUG
  cerr<<endl<<"RecordAlgebra::attrTM start. ("<<nl->ToString(args)<<")"<<endl;
#endif

  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("Two arguments expected.");
    return nl->TypeError();
  }

  first = nl->First(args); //z.B. (record (name string) (age int))

  if(nl->ListLength(first)<=1) {
    ErrorReporter::ReportError("First argument must be a record.");
    return nl->TypeError();
  }

  if(!nl->IsEqual(nl->First(first), Record::BasicType())) {
    ErrorReporter::ReportError("First argument must be a record.");
    return nl->TypeError();
  }

  second = nl->Second(args);
  if(nl->AtomType(second)!=SymbolType) {
    ErrorReporter::ReportError("Second argument must be an attribute name.");
    return nl->TypeError();
  }

  elemList = nl->Rest(first);
  elemName = nl->SymbolValue(second);
  elemIndex = listutils::findAttribute(elemList, elemName, elemType);
  if (elemIndex == 0) {
    ErrorReporter::ReportError("Element " + elemName +
                               " not found in element list.");
    return nl->TypeError();
  }

  ListExpr ret = nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND())
                               , nl->OneElemList(nl->IntAtom(elemIndex))
                               , elemType);

#ifdef RECORD_DEBUG
  cerr<<"RecordAlgebra::attrTM end. returns "<<nl->ToString(ret)<<endl;
#endif
  return ret;

}

/*
3.2 Value Mapping of Operator ~attr~

*/

int
attrVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Record* recordptr;
  Attribute* element;
  int index;

  recordptr = (Record*)args[0].addr;
  index = ((CcInt*)args[2].addr)->GetIntval();
  assert(1<=index);
  assert(index <= recordptr->GetNoElements());

  element = recordptr->GetElement(index - 1);
  result = qp->ResultStorage(s);
  (static_cast<Attribute*>(result.addr))->CopyFrom(element);
  element->DeleteIfAllowed();
  return 0;
}

/*
3.3 Operator Descriptions

*/

struct createRecordInfo : OperatorInfo {

  createRecordInfo()
  {
    name      = "createRecord";
    signature = "( (id1 T1) (id2 T2) ... (idn Tn) ) -> "
                "record( (id1 T1) (id2 T2) ... (idn Tn) )";
    syntax    = "createRecord([id1: T1, id2: T2, ..., idn: Tn])";
    meaning   = "Creates a record with the elements of the given list.";
  }

};

struct attrInfo : OperatorInfo {

  attrInfo()
  {
    name      = "attr";
    signature = "record( (id1 T1) (id2 T2) ... (idn Tn) ) x idi -> Ti";
    syntax    = "attr(_, _)";
    meaning   = "Returns the value of the element identified with idi";
  }
};


/*

4 Operator ~transformT2Rstream~
Creates a stream of records from a stream of tuples,
each record contains attributes of tuple

4.1 Type mapping function of operator ~transformT2Rstream~

*/

ListExpr transformT2RstreamTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr arg = nl->First(args);

  if(!Stream<Tuple>::checkType(arg)){
    ErrorReporter::ReportError("stream(tuple) expected");
    return nl->TypeError();
  }
  ListExpr attrList = nl->Second(nl->Second(arg));
  return nl->TwoElemList(nl->SymbolAtom(Stream<Record>::BasicType()),
                         nl->Cons( nl->SymbolAtom(Record::BasicType()),
                                          attrList));

}


/*

4.2 Value mapping function of operator ~transformT2Rstream~

*/

int
transformT2RstreamValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  Record* rec;
  bool elementAppended;

  switch (message)
  {

    case OPEN :
    {
      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST :
    {
      qp->Request(args[0].addr, t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        rec = new Record(0);
        ListExpr rest = nl->Rest(nl->Second(qp->GetType( s )));
        int i = 0;
        while( !nl->IsEmpty( rest ) ){
          ListExpr cur = nl->First( rest );
          rest = nl->Rest( rest );
          Attribute * attr = (Attribute*)tup->GetAttribute(i++);
          elementAppended = rec->AppendElement(
                                     attr, nl->IsAtom(nl->Second(cur)) ?
                                     nl->ToString(nl->Second(cur)) :
                                     nl->ToString(nl->First(nl->Second(cur))),
                             nl->ToString(nl->First(cur)));
          assert(elementAppended);
        }
        tup->DeleteIfAllowed();
        result.setAddr(rec);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

/*

4.3 Specification of operator ~transformT2Rstream~

*/
const string transformT2RstreamSpec  = "( ( \"Signature\" "
                         "\"Syntax\" \"Meaning\" \"Example\" ) "
                         "( <text>stream(tuple(y)) -> stream(record(y))"
                         "</text--->"
                         "<text>_ transformT2Rstream</text--->"
                         "<text>Creates a stream of records from a stream "
                         "of tuples.</text--->"
                         "<text>query ten feed transformT2Rstream</text--->"
                              ") )";

/*

4.4 Definition of operator ~transformT2Rstream~

*/
Operator rectransformT2Rstream (
         "transformT2Rstream",             // name
         transformT2RstreamSpec,           // specification
         transformT2RstreamValueMapping,   // value mapping
         Operator::SimpleSelect,           // trivial selection function
         transformT2RstreamTypeMap         // type mapping
);


/*

5 Operator ~transformR2Tstream~
Creates a stream of tuples from a stream of records,
each tuple contains all attributes of records

5.1 Type mapping function of operator ~transformR2Tstream~

*/

ListExpr transformR2TstreamTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  if(!Stream<Record>::checkType(nl->First(args))){
    ErrorReporter::ReportError("stream(record(X)) expected");
    return nl->TypeError();
  }

  ListExpr attrList = nl->Rest(nl->Second(nl->First(args)));
  return nl->TwoElemList(nl->SymbolAtom( Stream<Tuple>::BasicType()),
                  nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                         attrList));


}

/*

5.2 Value mapping function of operator ~transformR2Tstream~

*/

int
transformR2TstreamValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  TupleType * resultTupleType;
  Record * rec;
  Word r;

  switch (message)
  {
    case OPEN :
    {
      qp->Open(args[0].addr);
      ListExpr resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;
    }
    case REQUEST :
    {
      resultTupleType = (TupleType *)local.addr;
      qp->Request(args[0].addr, r);
      if (qp->Received(args[0].addr))
      {
        Tuple *newTuple = new Tuple( resultTupleType );
        rec = static_cast<Record*>(r.addr);
        for(int i = 0; i < rec->GetNoElements();i++){
          newTuple->PutAttribute( i,
                                  ((Attribute*)rec->GetElement(i)));
        }
        rec->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else return CANCEL;
    }
    case CLOSE :
    {
      if(local.addr)
      {
         ((TupleType *)local.addr)->DeleteIfAllowed();
         local.setAddr(0);
      }
      qp->Close(args[0].addr);
      return 0;
    }
  }
  return 0;
}

/*

5.3 Specification of operator ~transformR2Tstream~

*/
const string transformR2TstreamSpec  = "( ( \"Signature\" \"Syntax\" "
                         "\"Meaning\" \"Example\" ) "
                         "( <text>stream(record(y)) -> stream(tuple(y))"
                         "</text--->"
                         "<text>_ transformR2Tstream</text--->"
                         "<text>Creates a stream of tuples from a stream of "
                         "records. </text--->"
                         "<text>query ten feed transformT2Rstream "
                         "transformR2Tstream</text--->) )";

/*

5.4 Definition of operator ~transformR2Tstream~

*/
Operator rectransformR2Tstream (
         "transformR2Tstream",             // name
         transformR2TstreamSpec,           // specification
         transformR2TstreamValueMapping,   // value mapping
         Operator::SimpleSelect,           // trivial selection function
         transformR2TstreamTypeMap         // type mapping
);


/*

6 Implementation of the Algebra Class

*/

class RecordAlgebra : public Algebra
{
  public:
  RecordAlgebra() : Algebra()
  {
/*
Registration of Types

*/
    AddTypeConstructor( &recordTC );
    recordTC.AssociateKind( Kind::DATA() );

/*
Registration of Operators

*/
    AddOperator( createRecordInfo(), createRecordVM, createRecordTM );
    AddOperator( attrInfo(), attrVM, attrTM );
    AddOperator(&rectransformT2Rstream);
    AddOperator(&rectransformR2Tstream);

  }

/*
  Destructor

*/
    ~RecordAlgebra() {};
};


} // end of namespace ~record~


extern "C"
Algebra*
InitializeRecordAlgebra(NestedList* nlRef,
                        QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new record::RecordAlgebra;
}


