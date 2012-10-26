/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Update Relation Algebra

June 2005 Matthias Zielke

January 2006, Victor Almeida separated this algebra from the Extended
Relational Algebra and fixed some memory leaks.

October 2007, M. Spiekermann fixed bugs in the implementations of the operators
~updatedirectsave~ and ~updatedirectsearch~.

June 2008, M. Spiekermann bug fix for operator ~updatesearchsave~. However, the
implementation is still limited since the code expects that the relation to be
searched on can be hold in an random memory based hash table.

[TOC]

1 Includes and defines

*/
#include <vector>

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "LogMsg.h"
#include "RTreeAlgebra.h"
#include "ListUtils.h"
#include "Symbols.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operators

2.19 Operators ~createinsertrel~, ~createdeleterel~ and
~createupdaterel~

Create an empty relation with the schema of the result-tupletyp of
the corresponding 'update'-operators. This operator is used to
create an auxiliary relation for updates on relations which shall
be passed to existing indices later on.

2.19.1 General Type mapping function of operators ~createinsertrel~,
~createdeleterel~ and ~createupdaterel~



Type mapping ~createinsertrel~, ~createdeleterel~, and
~createupdaterel~

----     (rel X)

        -> (rel(tuple((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~createupdaterel~

----     (rel X)

        -> (rel(tuple(X (a1_old x1) ... (an_old xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/
ListExpr createAuxiliaryRelTypeMap( const ListExpr& args,
                                    const string opName )
{
  ListExpr first,rest,listn,lastlistn,oldAttribute, outList;
  string oldName;
  string argstr;

  if(nl->ListLength(args)!=1){
    return listutils::typeError("one argument expected");
  }
  first = nl->First(args);

  if(!listutils::isRelDescription(first) &&
      !listutils::isOrelDescription(first)){
    return listutils::typeError("relation expected");
  }


  // build first part of the result-tupletype
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     rest = nl->Rest(rest);
  }
  if ( opName == "createupdaterel")
    // Append once again all attributes from the argument-relation
    // to the result-tupletype but the names of the attributes
    // extendend by '_old'
  {
    rest = nl->Second(nl->Second(first));
    while (!(nl->IsEmpty(rest)))
    {
      nl->WriteToString(oldName, nl->First(nl->First(rest)));
      oldName += "_old";
      oldAttribute =
        nl->TwoElemList(
          nl->SymbolAtom(oldName),
          nl->Second(nl->First(rest)));
      lastlistn = nl->Append(lastlistn,oldAttribute);
      rest = nl->Rest(rest);
    }
  }

  //Append last attribute for the tupleidentifier
  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));
  outList = nl->TwoElemList(
              nl->SymbolAtom(Relation::BasicType()),
              nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                listn));
  return outList;
}

ListExpr createInsertRelTypeMap( ListExpr args)
{
  return createAuxiliaryRelTypeMap( args, "createinsertrel");
}

ListExpr createDeleteRelTypeMap( ListExpr args)
{
  return createAuxiliaryRelTypeMap( args, "createdeleterel");
}

ListExpr createUpdateRelTypeMap( ListExpr args)
{
  return createAuxiliaryRelTypeMap( args, "createupdaterel");
}

/*
2.19.2 General Value mapping function of operators ~createinsertrel~, ~createdeleterel~ and ~createupdaterel~

*/
int createUpdateRelValueMap(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  return 0;
}

/*
2.19.3 Specification of operator ~createinsertrel~

*/
const string createinsertrelSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel(tuple(x))"
  " -> rel(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ createinsertrel</text--->"
  "<text>Creates an empty relation with the schema of the "
  "result-tupletype of the 'insert'-operators. </text--->"
  "<text>let staedteI = staedte createinsertrel "
  "</text--->"
  ") )";

/*
2.19.4 Definition of operator ~createinsertrel~

*/
Operator extrelcreateinsertrel (
  "createinsertrel",             // name
  createinsertrelSpec,           // specification
  createUpdateRelValueMap,       // value mapping
  Operator::SimpleSelect,        // trivial selection function
  createInsertRelTypeMap         // type mapping
);


/*
2.20.3 Specification of operator ~createdeleterel~

*/
const string createdeleterelSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel(tuple(x))"
  " -> rel(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ createdeleterel</text--->"
  "<text>Creates an empty relation with the schema of the "
  "result-tupletype of the 'delete'-operators. </text--->"
  "<text>let staedteD = staedte createdeleterel"
  "</text--->"
  ") )";

/*
2.20.4 Definition of operator ~createdeleterel~

*/
Operator extrelcreatedeleterel (
  "createdeleterel",             // name
  createdeleterelSpec,           // specification
  createUpdateRelValueMap,       // value mapping
  Operator::SimpleSelect,        // trivial selection function
  createDeleteRelTypeMap         // type mapping
);


/*
2.21.3 Specification of operator ~createupdaterel~

*/
const string createupdaterelSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel(tuple(x))"
  " -> rel(tuple(x@[(a1_old x1)...(an_old xn)(TID:tid)]))] "
  "</text--->"
  "<text>_ createupdaterel</text--->"
  "<text>Creates an empty relation with the schema of the "
  "result-tupletype of the 'update'-operators. </text--->"
  "<text>let staedteU = staedte createupdaterel "
  "</text--->"
  ") )";

/*
2.21.4 Definition of operator ~createupdaterel~

*/
Operator extrelcreateupdaterel (
  "createupdaterel",             // name
  createupdaterelSpec,           // specification
  createUpdateRelValueMap,       // value mapping
  Operator::SimpleSelect,        // trivial selection function
  createUpdateRelTypeMap         // type mapping
);


/*
2.22 Operator ~insert~

Inserts each tuple of the inputstream into the relation. Returns a stream of tuples which is  basically
the same as the inputstream but each tuple extended by an attribute of type 'tid' which is the tupleidentifier
of the inserted tuple in the extended relation.

2.22.0 General type mapping function of operators ~insert~, ~deletesearch~ and ~deletedirect~

General type mapping for operators ~insert~ ,~deletesearch~ and ~deletedirect~

----     (stream X) (rel X)

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

*/
ListExpr insertDeleteRelTypeMap( ListExpr& args, string opName )
{
  ListExpr first, second, rest,listn,lastlistn, outList;
  string argstr, argstr2;

  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }

  first = nl->First(args);
  second  = nl->Second(args);

  if(!listutils::isTupleStream(first)){
   return listutils::typeError("first argument must be a tuple stream");
  }

  if(!listutils::isRelDescription(second) &&
      !listutils::isOrelDescription(second)){
    return listutils::typeError("second argument must be of type rel or orel");
  }

  if(!nl->Equal(nl->Second(first),nl->Second(second))){
   return listutils::typeError("tuple types must be equal");
  }


  // build resutllist
  rest = nl->Second(nl->Second(second));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));
  outList = nl->TwoElemList(
              nl->SymbolAtom(Symbol::STREAM()),
              nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                listn));
  return outList;
}

/*
2.22.1 Type mapping function of operator ~insert~

*/

ListExpr insertRelTypeMap(ListExpr args)
{
  return insertDeleteRelTypeMap(args, "insert");
}

/*
2.22.2 Value mapping function of operator ~insert~

*/
int insertRelValueMap(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  GenericRelation* relation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->CopyAttribute( i, tup, i );
        relation->AppendTuple(tup);
        const TupleId& tid = tup->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr );
        result.setAddr(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        resultTupleType = (TupleType*) local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
2.22.3 Specification of operator ~insert~

*/
const string insertSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x))"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ insert</text--->"
  "<text>Inserts all tuples of the stream "
  "into the relation.</text--->"
  "<text>query neueStaedte feed staedte insert "
  "consume"
  "</text--->"
  ") )";

/*
2.22.4 Definition of operator ~insert~

*/
Operator extrelinsert (
  "insert",                // name
  insertSpec,              // specification
  insertRelValueMap,       // value mapping
  Operator::SimpleSelect,  // trivial selection function
  insertRelTypeMap         // type mapping
);

/*
2.23 Operator ~insertsave~

Inserts each tuple of the inputstream into the relation. Returns a stream of tuples which is  basically
the same as the inputstream but each tuple extended by an attribute of type 'tid' which is the tupleidentifier
of the inserted tuple in the extended relation. In addition all the tuples of the resultstream are inserted
into the auxiliary relation which is given as the third argument.


2.23.1 Type mapping function of operator ~insertsave~



Type mapping ~insertsave~ on a relation

----     (stream X) (rel X) (rel X@[(TID tid)])

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/

ListExpr insertSaveRelTypeMap( ListExpr args )
{
  ListExpr first, second, third,rest,listn,lastlistn, outList;
  string argstr, argstr2;

  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  first = nl->First(args);
  second  = nl->Second(args);
  third  = nl->Third(args);

  if(!listutils::isTupleStream(first)){
    return listutils::typeError("first argument must be a tuple stream");
  }

  if(!listutils::isRelDescription(second) &&
     !listutils::isOrelDescription(second)){
    return listutils::typeError("second argument must be a relation");
  }

  if(!listutils::isRelDescription(third)){
    return listutils::typeError("third argument must be a relation");
  }

  if(!nl->Equal(nl->Second(first),nl->Second(second))){
    return listutils::typeError("tuple types of the first and "
                                "second argument have to be equal");
  }

  // build resutllist
  rest = nl->Second(nl->Second(second));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn= nl->Append(lastlistn,
                        nl->TwoElemList(
                          nl->SymbolAtom("TID"),
                          nl->SymbolAtom(TupleIdentifier::BasicType())));

  if(!nl->Equal(listn, nl->Second(nl->Second(third)))){
    return listutils::typeError("tuple type of the third argument must equal "
              "to the tuple type of the first arguments with an additional "
              "tid as last attribute");

  }
  outList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),listn));
  return outList;
}

/*
2.23.2 Value mapping function of operator ~insertsave~

*/
int insertSaveRelValueMap(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  GenericRelation* relation;
  Relation* auxRelation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      relation = (Relation*)(args[1].addr);
      assert(relation != 0);
      auxRelation = (Relation*)(args[2].addr);
      assert(auxRelation != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        relation->AppendTuple(tup);
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr );
        auxRelation->AppendTuple(newTuple);
        result.setAddr(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      qp->SetModified(qp->GetSon(s, 2));
      if(local.addr)
      {
        resultTupleType = (TupleType*) local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
2.23.3 Specification of operator ~insertsave~

*/
const string insertSaveSpec  =
  "(( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x@[TID:tid]))"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ _insertsave</text--->"
  "<text>Like insert but appends all result-tuples to "
  "an auxiliary relation.</text--->"
  "<text>query neueStaedte feed staedte staedteI insertsave "
  "consume"
  "</text--->"
  ") )";

/*
2.23.4 Definition of operator ~insertsave~

*/
Operator extrelinsertsave (
  "insertsave",              // name
  insertSaveSpec,            // specification
  insertSaveRelValueMap,     // value mapping
  Operator::SimpleSelect,    // trivial selection function
  insertSaveRelTypeMap       // type mapping
);

/*
2.24 Operator ~deletesearch~

Deletes each tuple from the relation which attribut-values are exactly the same as those ones of
a tuple of the inputstream. Returns a stream of tuples which is  basically
the stream of deleted tuples but each tuple extended by an attribute of type 'tid' which is the tupleidentifier
of the deleted tuple in the extended relation.

2.24.1 TypeMapping for operator ~deletesearch~

*/

ListExpr deleteSearchRelTypeMap( ListExpr args )
{
  return insertDeleteRelTypeMap(args, "deletesearch");
}



/*
2.24.2 Value mapping function of operator ~deletesearch~

*/
int deleteSearchRelValueMap(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  Tuple* nextTup;
  Tuple* newTuple;
  GenericRelation* relation;
  GenericRelationIterator* iter;
  ListExpr resultType;
  vector<Tuple*>* nextBucket;
  struct LocalTransport
  {
    LocalTransport():
    deletedTuples( new vector<Tuple*>() ),
    hashTable( new vector<vector<Tuple*>*>(256) ),
    resultTupleType( 0 )
    {
      for (int i = 0; i < 256; i++)
        (*hashTable)[i] = new vector<Tuple*>();
    }

    ~LocalTransport()
    {
      for( vector<Tuple*>::iterator i = deletedTuples->begin();
           i != deletedTuples->end();
           i++ )
      {
        (*i)->DeleteIfAllowed();
      }
      delete deletedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      resultTupleType->DeleteIfAllowed();
    }

    vector<Tuple*>* deletedTuples;
    vector<vector<Tuple*>*>* hashTable;
    TupleType* resultTupleType;
  } *localTransport;
  size_t hashValue ;
  bool tupleFound;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      localTransport = new LocalTransport();
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      iter = relation->MakeScan();
      nextTup = iter->GetNextTuple();
      // fill hashtable
      while (!iter->EndOfScan())
      {
        hashValue = 0;
        for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
           hashValue +=
             ((Attribute*)
               (nextTup->GetAttribute(i)))->HashValue();
        nextBucket =
          localTransport->hashTable->operator[](hashValue % 256);
        nextBucket->push_back(nextTup);
        nextTup = iter->GetNextTuple();
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType =
        new TupleType( nl->Second( resultType ) );
      local.setAddr( localTransport );
      return 0;

    case REQUEST :
      localTransport =  (LocalTransport*) local.addr;
      // deletedTuples can contain duplicates that have not been
      // given to the resultstream yet
      if (!localTransport->deletedTuples->empty())
      {
        newTuple = localTransport->deletedTuples->back();
        newTuple->DeleteIfAllowed();
        localTransport->deletedTuples->pop_back();
        result.setAddr(newTuple);
        return YIELD;
      }

      // no duplicate has to be send to the resultstream
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      tupleFound = false;
      // tupleFound will stay false until a matching tuple to a
      // inputtuple was found
      while (! tupleFound)
      {
        qp->Request(args[0].addr,t);
        if (qp->Received(args[0].addr))
        {
          tup = (Tuple*)t.addr;
          hashValue = 0;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            hashValue +=
              ((Attribute*)
                (tup->GetAttribute(i)))->HashValue();
          SortOrderSpecification full;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
               full.push_back(pair<int,bool> (i+1,false));
          TupleCompareBy compare(full);
          // get correct bucket from hashtable
          nextBucket =
            localTransport->hashTable->operator[](hashValue % 256);
          // look for all matching tuples in the bucket
          for (size_t j = 0; j < nextBucket->size(); j++)
          {
            nextTup = nextBucket->operator[](j);
            if (nextTup != 0)
            {
              if(!compare(nextTup,tup) && !compare(tup,nextTup))
              {
                newTuple =
                  new Tuple(localTransport->resultTupleType);
                assert( newTuple->GetNoAttributes() ==
                        nextTup->GetNoAttributes() + 1 );
                for( int i = 0;
                     i < nextTup->GetNoAttributes();
                     i++ )
                  newTuple->PutAttribute(
                    i, (nextTup->GetAttribute(i))->Clone() );
                const TupleId& tid = nextTup->GetTupleId();
                Attribute* tidAttr =
                  new TupleIdentifier(true,tid);
                newTuple->PutAttribute(
                  nextTup->GetNoAttributes(), tidAttr );
                relation->DeleteTuple(nextTup);
                newTuple->IncReference();
                localTransport->deletedTuples->push_back(newTuple);
              }
            }
          }
          if (!localTransport->deletedTuples->empty())
          {
            newTuple = localTransport->deletedTuples->back();
            newTuple->DeleteIfAllowed();
            localTransport->deletedTuples->pop_back();
            result.setAddr(newTuple);
            tupleFound = true;
          }
          tup->DeleteIfAllowed();
        }
        else// if (qp->Received(args[0].addr))
          return CANCEL;
      }// while (! tupleFound);
      return YIELD;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        localTransport = (LocalTransport*) local.addr;
        delete localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.24.3 Specification of operator ~deletesearch~

*/
const string deleteSearchSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x))"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ deletesearch</text--->"
  "<text>Deletes all tuples with the same values as tuples"
  "of the stream from the relation. </text--->"
  "<text>query alteStaedte feed "
  "staedte deletesearch consume"
  "</text--->"
  ") )";

/*
2.24.4 Definition of operator ~deletesearch~

*/
Operator extreldeletesearch (
  "deletesearch",              // name
  deleteSearchSpec,            // specification
  deleteSearchRelValueMap,     // value mapping
  Operator::SimpleSelect,      // trivial selection function
  deleteSearchRelTypeMap       // type mapping
);

/*
2.25 Operator ~deletedirect~

Deletes each tuple from the inputstream directly from the relation. Precondition is that the tuples
of the inputstream are tuples from the relation that shall be updated. Returns a stream of tuples
which is  basically the stream of deleted tuples but each tuple extended by an attribute of type
'tid' which is the tupleidentifier of the deleted tuple in the updated relation.

2.25.1 TypeMapping for operator ~deletedirect~

*/

ListExpr deleteDirectRelTypeMap( ListExpr args )
{
  return insertDeleteRelTypeMap(args, "deletedirect");
}


/*
2.25.2 Value mapping function of operator ~deletedirect~

*/
int deleteDirectRelValueMap(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  GenericRelation* relation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->PutAttribute( i, tup->GetAttribute( i )->Clone() );
        const TupleId& tid = tup->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute(tup->GetNoAttributes(), tidAttr);
        relation->DeleteTuple(tup);
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        resultTupleType = (TupleType*) local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.25.3 Specification of operator ~deletedirect~

*/
const string deleteDirectSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x))"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ deletedirect</text--->"
  "<text>Deletes directly all tuples of the stream "
  "from the relation.</text--->"
  "<text>query staedte feed filter [.SName = 'Hagen']"
  "staedte deletedirect consume"
  "</text--->"
  ") )";

/*
2.25.4 Definition of operator ~deletedirect~

*/
Operator extreldeletedirect (
  "deletedirect",              // name
  deleteDirectSpec,            // specification
  deleteDirectRelValueMap,     // value mapping
  Operator::SimpleSelect,      // trivial selection function
  deleteDirectRelTypeMap       // type mapping
);


/*
2.26 Operator ~deletesearchsave~

Deletes each tuple from the relation which attribut-values are exactly the same as those ones of a
tuple of the inputstream. Returns a stream of tuples which is  basically
the stream of deleted tuples but each tuple extended by an attribute of type 'tid' which is the tupleidentifier
of the deleted tuple in the updated relation. In addition all the tuples of the resultstream are inserted
into the auxiliary relation which is given as the third argument.


2.26.0 General Type mapping function of operators ~deletesearchsave~ and ~deletedirectsave~



General Type mapping ~deletesearchsave~ and ~deletedirectsave~

----     (stream X) (rel X) (rel (tuple ((a1 x1) ... (an xn) (TID tid))))

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/

ListExpr deleteSaveRelTypeMap( ListExpr& args, string opName )
{
  ListExpr first, second, third, rest,listn,lastlistn, outList;
  string argstr, argstr2;

  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }
  // Check inputstream
  first = nl->First(args);
  second  = nl->Second(args);
  third  = nl->Third(args);

  if(!listutils::isTupleStream(first)){
   return listutils::typeError("first argument has to be a tuple stream");
  }

  if(!listutils::isRelDescription(second) &&
      !listutils::isOrelDescription(second)){
    return listutils::typeError("second argument has to be of type rel "
                                "or orel");
  }

  if(!listutils::isRelDescription(third)){
    return listutils::typeError("third argument must be a relation");
  }

  if(!nl->Equal(nl->Second(first),nl->Second(second))){
   return listutils::typeError("tuple types of the first two "
                               "arguments must be equal");
  }

  rest = nl->Second(nl->Second(second));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));

  if(!nl->Equal(listn,nl->Second(nl->Second(third)))){
    return listutils::typeError("attribute list of the third argument "
            "must be equal to the attribute list of the first element "
            "with an additional tid attribute");
  }

  outList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),listn));
  return outList;
}

/*

2.26.1 Type Mapping for operator ~deletesearchsave~

*/

ListExpr deleteSearchSaveRelTypeMap(ListExpr args)
{
  return deleteSaveRelTypeMap(args, "deletesearchsave");
}

/*
2.26.2 Value mapping function of operator ~deletesearchsave~

*/
int deleteSearchSaveRelValueMap(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  Tuple* nextTup;
  Tuple* newTuple;
  GenericRelation* relation;
  Relation* auxRelation;
  GenericRelationIterator* iter;
  ListExpr resultType;
  vector<Tuple*>* nextBucket;
  struct LocalTransport
  {
    LocalTransport():
    deletedTuples( new vector<Tuple*>() ),
    hashTable( new vector<vector<Tuple*>*>(256) ),
    resultTupleType( 0 )
    {
      for (int i = 0; i < 256; i++)
        (*hashTable)[i] = new vector<Tuple*>();
    }

    ~LocalTransport()
    {
      for( vector<Tuple*>::iterator i = deletedTuples->begin();
           i != deletedTuples->end();
           i++ )
      {
        (*i)->DeleteIfAllowed();
      }
      delete deletedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      resultTupleType->DeleteIfAllowed();
    }

    vector<Tuple*>* deletedTuples;
    vector<vector<Tuple*>*>* hashTable;
    TupleType* resultTupleType;
  } *localTransport;
  size_t hashValue ;
  bool tupleFound;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      localTransport = new LocalTransport();
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      iter = relation->MakeScan();
      nextTup = iter->GetNextTuple();
      while (!iter->EndOfScan())
      {
        hashValue = 0;
        for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
          hashValue +=
            ((Attribute*)
             (nextTup->GetAttribute(i)))->HashValue();
        nextBucket =
          localTransport->hashTable->operator[](hashValue % 256);
        nextBucket->push_back(nextTup);
        nextTup = iter->GetNextTuple();
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType =
        new TupleType( nl->Second( resultType ) );
      local.setAddr( localTransport );
      return 0;

    case REQUEST :
      localTransport = (LocalTransport*) local.addr;

      // Check if already deleted duplicates have to be given to
      // the outputstream
      if (!localTransport->deletedTuples->empty())
      {
        newTuple = localTransport->deletedTuples->back();
        localTransport->deletedTuples->pop_back();
        result.setAddr(newTuple);
        return YIELD;
      }
      // No more duplicate left over to send to the outputstream
      relation = (Relation*)(args[1].addr);
      assert(relation != 0);
      auxRelation = (Relation*)(args[2].addr);
      assert(auxRelation != 0);
      tupleFound = false;
      // tupleFound will stay false until a tuple with the same
      // values as one of the inputtuples is found
      while (! tupleFound)
      {
        qp->Request(args[0].addr,t);
        if (qp->Received(args[0].addr))
        {
          tup = (Tuple*)t.addr;
          hashValue = 0;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            hashValue +=
              ((Attribute*)
               (tup->GetAttribute(i)))->HashValue();
          SortOrderSpecification full;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            full.push_back(pair<int,bool> (i+1,false));
          TupleCompareBy compare(full);
          // Get the right hash-bucket
          nextBucket =
            localTransport->hashTable->operator[](hashValue % 256);
          // Check all tuples in the bucket
          for (size_t j = 0; j < nextBucket->size(); j++)
          {
            nextTup = nextBucket->operator[](j);
            if (nextTup != 0)
            {
              if(!compare(nextTup,tup) && !compare(tup,nextTup))
              {
                newTuple =
                  new Tuple(localTransport->resultTupleType);
                assert( newTuple->GetNoAttributes() ==
                        nextTup->GetNoAttributes() +1 );
                for( int i = 0;
                     i < nextTup->GetNoAttributes();
                     i++ )
                  newTuple->PutAttribute(
                    i, (nextTup->GetAttribute(i))->Clone() );
                const TupleId& tid = nextTup->GetTupleId();
                Attribute* tidAttr =
                  new TupleIdentifier(true,tid);
                newTuple->PutAttribute(
                  nextTup->GetNoAttributes(), tidAttr);
                relation->DeleteTuple(nextTup);
                auxRelation->AppendTuple(newTuple);
                newTuple->IncReference();
                localTransport->deletedTuples->push_back(newTuple);
              }
            }
          }
          // Set result if at least one tuple was deleted
          if (!localTransport->deletedTuples->empty())
          {
            newTuple = localTransport->deletedTuples->back();
            newTuple->DeleteIfAllowed();
            localTransport->deletedTuples->pop_back();
            result.setAddr(newTuple);
            tupleFound = true;
          }
          tup->DeleteIfAllowed();
        }
        else// if (qp->Received(args[0].addr))
          return CANCEL;
      }// while (! tupleFound);
      return YIELD;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      qp->SetModified(qp->GetSon(s, 2));
      if(local.addr)
      {
        localTransport = (LocalTransport*) local.addr;
        delete localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.26.3 Specification of operator ~deletesearchsave~

*/
const string deleteSearchSaveSpec  =
  "(( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x@[TID:tid]))"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ _deletesearchsave</text--->"
  "<text>Like deletesearch but appends all result-tuples"
  "to the auxiliary relation.</text--->"
  "<text>query alteStaedte feed staedte staedteD deletesearchsave "
  "consume"
  "</text--->"
  ") )";

/*
2.26.4 Definition of operator ~deletesearchsave~

*/
Operator extreldeletesearchsave (
  "deletesearchsave",              // name
  deleteSearchSaveSpec,            // specification
  deleteSearchSaveRelValueMap,     // value mapping
  Operator::SimpleSelect,          // trivial selection function
  deleteSearchSaveRelTypeMap       // type mapping
);


/*
2.27 Operator ~deletedirectsave~

Deletes directly each tuple of the inputstream from the relation. Precondition is that all tuples
of the inputstream are originally tuples of the relation that shall be updated.
Returns a stream of tuples which is  basically
the stream of deleted tuples but each tuple extended by an attribute of type 'tid' which is the tupleidentifier
of the deleted tuple in the updated relation.In addition all the tuples of the resultstream are inserted
into the auxiliary relation which is given as the third argument.


2.27.1 Type Mapping for operator ~deletedirectsave~

*/

ListExpr deleteDirectSaveRelTypeMap(ListExpr args)
{
  return deleteSaveRelTypeMap(args, "deletedirectsave");
}

/*
2.27.2 Value mapping function of operator ~deletedirectsave~

*/

int deleteDirectSaveRelValueMap(Word* args, Word& result,
                                int message, Word& local,
                                Supplier s)
{
  Word t;
  Tuple* tup;
  GenericRelation* relation;
  Relation* auxRelation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      auxRelation = (Relation*)(args[2].addr);
      assert(auxRelation != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        assert( newTuple->GetNoAttributes() ==
                tup->GetNoAttributes() +1 );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->PutAttribute( i, tup->GetAttribute( i )->Clone() );
        const TupleId& tid = tup->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr);
        relation->DeleteTuple(tup);
        auxRelation->AppendTuple(newTuple);
        result.setAddr(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      qp->SetModified(qp->GetSon(s, 2));
      if(local.addr)
      {
        resultTupleType = (TupleType*) local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}



/*
2.27.3 Specification of operator ~deletedirectsave~

*/
const string deleteDirectSaveSpec  =
  "(( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x@[TID:tid]))"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ _deletedirectsave</text--->"
  "<text>Like 'deletedirect' but appends all result-tuples "
  "to the auxiliary relation.</text--->"
  "<text>query staedte feed filter [.Bev > 200000] "
  "staedte staedteD deletedirectsave consume"
  "</text--->"
  ") )";

/*
2.27.4 Definition of operator ~deletedirectsave~

*/
Operator extreldeletedirectsave (
  "deletedirectsave",              // name
  deleteDirectSaveSpec,            // specification
  deleteDirectSaveRelValueMap,     // value mapping
  Operator::SimpleSelect,          // trivial selection function
  deleteDirectSaveRelTypeMap       // type mapping
);


/*
2.28 Operator ~inserttuple~

Inserts a new tuple with the attribute-values of the second argument into the relation.
Returns a stream of one tuple which is  basically
the same as the inserted tuple but extended by an attribute of type 'tid' which is the tupleidentifier
of the inserted tuple in the extended relation.


2.28.1 Type mapping function of operator ~inserttuple~



Type mapping ~inserttuple~ on a relation

----     (rel(tuple(x))) x [t1,... tn]
          -> stream(tuple(x@[TID:tid])))
----

*/
ListExpr insertTupleTypeMap(ListExpr args)
{
  ListExpr first,second, rest, rest2, listn, lastlistn, outList;
  string argstr="",valueString;

  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }

  first = nl->First(args);
  second = nl->Second(args);

  if(!listutils::isRelDescription(first) &&
    !listutils::isOrelDescription(first)){
    return listutils::typeError("rel or orel as first arg expected");
  }

  if(nl->AtomType(second)!=NoAtom){
    return listutils::typeError("list as second arg expected");
  }
  if(nl->ListLength(nl->Second(nl->Second(first)))!=nl->ListLength(second)){
    return listutils::typeError("different lengths in tuple and update");
  }

  rest = nl->Second(nl->Second(first));
  rest2 = second;
  while (!(nl->IsEmpty(rest)))
  {
    if(!nl->Equal(nl->Second(nl->First(rest)),nl->First(rest2))){
      return listutils::typeError("type mismatch in attribute "
                                  "list and update list");
    }
    rest = nl->Rest(rest);
    rest2 = nl->Rest(rest2);
  }

  // build resultlist
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));
  outList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),listn));
  return outList;
}

/*
2.28.2 Value mapping function of operator ~inserttuple~

*/
int insertTupleRelValueMap(Word* args, Word& result, int message,
                           Word& local, Supplier s)
{
  Word attrValue;
  Tuple* insertTuple;
  Tuple* resultTuple;
  GenericRelation* relation;
  TupleType *resultTupleType;
  ListExpr resultType;
  TupleType* insertTupleType;
  ListExpr insertType;
  ListExpr rest;
  ListExpr last;
  bool* firstcall;
  Supplier supplier, supplier1;
  Attribute* attr;

  switch (message)
  {
    case OPEN :
      firstcall = new bool;
      *firstcall = true;
      local.setAddr( firstcall );
      return 0;

    case REQUEST :
      firstcall = (bool*) local.addr;
      if (*firstcall)
      {
        *firstcall = false;
        resultType = GetTupleResultType( s );
        resultTupleType = new TupleType( nl->Second( resultType ) );
        relation = (GenericRelation*)(args[0].addr);
        assert(relation != 0);
        resultTuple = new Tuple( resultTupleType );

        rest = nl->Second(nl->Second(resultType));
        insertType = nl->OneElemList(nl->First(rest));
        last = insertType;
        rest = nl->Rest(rest);
        while(nl->ListLength(rest)>1) {
          last = nl->Append(last, nl->First(rest));
          rest = nl->Rest(rest);
        }
        insertType = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                     insertType);

        insertTupleType = new TupleType( insertType );
        insertTuple = new Tuple( insertTupleType );
        supplier = args[1].addr;
        for( int i = 0; i < resultTuple->GetNoAttributes()-1; i++ )
        {
          supplier1 = qp->GetSupplier(supplier, i);
          qp->Request(supplier1,attrValue);
          attr = (Attribute*) attrValue.addr;
          resultTuple->PutAttribute(i,attr->Clone());
          insertTuple->CopyAttribute(i,resultTuple,i);
        }
        relation->AppendTuple(insertTuple);
        const TupleId& tid = insertTuple->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        resultTuple->PutAttribute(
          resultTuple->GetNoAttributes() -1, tidAttr);
        result.setAddr(resultTuple);
        insertTuple->DeleteIfAllowed();
        resultTupleType->DeleteIfAllowed();
        insertTupleType->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->SetModified(qp->GetSon(s, 0));
      if(local.addr)
      {
        firstcall = (bool*) local.addr;
        delete firstcall;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
2.28.3 Specification of operator ~inserttuple~

*/
const string insertTupleSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel(tuple(x))) x [t1 ... tn]"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text> _ inserttuple [list]</text--->"
  "<text>Inserts a new tuple with the values from"
  " the second argument-list into the relation. </text--->"
  "<text>query staedte inserttuple['Kassel', 200000, 50000] "
  "consume"
  "</text--->"
  ") )";

/*
2.28.4 Definition of operator ~inserttuple~

*/
Operator extrelinserttuple (
  "inserttuple",              // name
  insertTupleSpec,            // specification
  insertTupleRelValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  insertTupleTypeMap          // type mapping
);

/*
2.29 Operator ~inserttuplesave~

Inserts a new tuple with the attribute-values of the second argument into the relation.
Returns a stream of one tuple which is  basically
the same as the inserted tuple but extended by an attribute of type int which is the tupleidentificator
of the inserted tuple in the extended relation. In addition the tuple of the resultstream is inserted
into the relation which is given by the second argument.


2.29.1 Type mapping function of operator ~inserttuplesave~



Type mapping ~inserttuplesave~ on a relation

----     (rel(tuple(x))) x (rel(tuple(x@[TID:tid]))) x [t1 ... tn]
          -> stream(tuple(x@[TID:tid])))
----

*/


ListExpr insertTupleSaveTypeMap(ListExpr args)
{
  ListExpr first,second, third, rest, rest2, listn,
           lastlistn, outList;
  string argstr="",valueString, argstr2;

  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expecetd");
  }
  first = nl->First(args);
  second = nl->Second(args);
  third= nl->Third(args);

  if(!listutils::isRelDescription(first) &&
    !listutils::isOrelDescription(first)){
    return listutils::typeError("first argument must be a relation");
  }

  if(nl->ListLength(nl->Second(nl->Second(first)))!=nl->ListLength(third)){
    return listutils::typeError("number of attributes differs from number "
           "of update value");
  }

  if(!listutils::isRelDescription(second)){
    return listutils::typeError("second argument must be a relation");
  }


  // updatrelation at the same positions are the same
  rest = nl->Second(nl->Second(first));
  rest2 = third;
  while (!(nl->IsEmpty(rest)))
  {
    if(!nl->Equal(nl->Second(nl->First(rest)),nl->First(rest2))){
      return listutils::typeError("type mismatch in attribute list"
                                  " and update list");
    }
     rest = nl->Rest(rest);
     rest2 = nl->Rest(rest2);
   }
  // build resultlist
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn = nl->Append(lastlistn,
                         nl->TwoElemList(
                           nl->SymbolAtom("TID"),
                           nl->SymbolAtom(TupleIdentifier::BasicType())));
  // Check if result-tupletype and type of the auxiliary relation
  // are the same

  if(!nl->Equal(listn, nl->Second(nl->Second(second)))){
    return listutils::typeError("second argument must be a relation having "
              " the same attributes as the first relation and an additional "
              " tid attribute ");

  }
  outList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),listn));
  return outList;
}

/*
2.29.2 Value mapping function of operator ~inserttuplesave~

*/
int insertTupleSaveRelValueMap(Word* args, Word& result,
                               int message, Word& local,
                               Supplier s)
{
  Word attrValue;
  Tuple* insertTuple;
  Tuple* resultTuple;
  GenericRelation* relation;
  Relation* auxRelation;
  TupleType *resultTupleType;
  ListExpr resultType;
  TupleType* insertTupleType;
  ListExpr insertType;
  ListExpr rest;
  ListExpr last;
  bool* firstcall;
  Supplier supplier, supplier1;
  Attribute* attr;

  switch (message)
  {
    case OPEN :
      firstcall = new bool;
      *firstcall = true;
      local.setAddr( firstcall );
      return 0;

    case REQUEST :
      firstcall = (bool*) local.addr;
      if (*firstcall)
      {
        *firstcall = false;
        resultType = GetTupleResultType( s );
        resultTupleType = new TupleType( nl->Second( resultType ) );
        relation = (GenericRelation*)(args[0].addr);
        assert(relation != 0);
        auxRelation = (Relation*)(args[1].addr);
        assert(auxRelation != 0);
        resultTuple = new Tuple( resultTupleType );

        rest = nl->Second(nl->Second(resultType));
        insertType = nl->OneElemList(nl->First(rest));
        last = insertType;
        rest = nl->Rest(rest);
        while(nl->ListLength(rest)>1) {
          last = nl->Append(last, nl->First(rest));
          rest = nl->Rest(rest);
        }
        insertType = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                     insertType);

        insertTupleType = new TupleType( insertType );
        insertTuple = new Tuple( insertTupleType );
        supplier = args[2].addr;
        for( int i = 0; i < resultTuple->GetNoAttributes()-1; i++ )
        {
          supplier1 = qp->GetSupplier(supplier, i);
          qp->Request(supplier1,attrValue);
          attr = (Attribute*) attrValue.addr;
          resultTuple->PutAttribute(i,attr->Clone());
          insertTuple->CopyAttribute(i,resultTuple,i);
        }
        relation->AppendTuple(insertTuple);
        const TupleId& tid = insertTuple->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        resultTuple->PutAttribute(
          resultTuple->GetNoAttributes() -1, tidAttr);
        auxRelation->AppendTuple(resultTuple);
        result.setAddr(resultTuple);
        insertTuple->DeleteIfAllowed();
        resultTupleType->DeleteIfAllowed();
        insertTupleType->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->SetModified(qp->GetSon(s, 0));
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        firstcall = (bool*) local.addr;
        delete firstcall;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
2.29.3 Specification of operator ~inserttuplesave~

*/
const string insertTupleSaveSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel(tuple(x)) x rel(tuple(x@[TID:tid])) x [t1 ... tn]"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text> _ _ inserttuplesave [list]</text--->"
  "<text>Like `inserttuple` but appends the result-tuple "
  "to the auxiliary relation </text--->"
  "<text>query staedte staedteI "
  "inserttuplesave['Kassel', 200000, 50000] consume"
  "</text--->"
  ") )";

/*
2.29.4 Definition of operator ~inserttuplesave~

*/
Operator extrelinserttuplesave (
  "inserttuplesave",              // name
  insertTupleSaveSpec,            // specification
  insertTupleSaveRelValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  insertTupleSaveTypeMap          // type mapping
);

/*
2.18 Operator ~updatedirect~

Updates each input tuple by  replacing the attribute-values of the attributes given by their names in the
function-argumentlist with the new values received from the corresponding functions. Precondition is that
all tuples of the inpustream are originally tuples of the updated relation.
The updated tuple is made persistent and as the resultstream tuples are returned that contain the new
values in first places, then all old values and finally the TupleIdentifier of the updated tuple.

2.18.0 General Type mapping function of operators ~updatedirect~ and ~updatesearch~

Type mapping for ~updatedirect~ and ~updatesearch~ is

----     (stream X) (rel X) ((ai1 (map x xi1)) ... (aij (map x xij))

        -> (stream (tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn)(TID tid))))

        where X = (tuple ((a1 x1) ... (an xn))) and ai1 - aij in (a1 .. an)
----

*/
ListExpr updateTypeMap( ListExpr& args, string opName )
{
  ListExpr first, second,third,errorInfo, rest, listn,
           lastlistn, first2, second2, firstr,attrType,
           numberList, lastNumberList,oldAttribute, outlist;
  int attrIndex, noAttrs ;
  bool firstcall = true;
  string argstr, argstr2, argstr3, oldName;
  AlgebraManager* algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  first = nl->First(args);
  second  = nl->Second(args);
  third = nl->Third(args);

  if(!listutils::isTupleStream(first)){
    return listutils::typeError("tuple stream expected");
  }

  if(!listutils::isRelDescription(second) &&
    !listutils::isOrelDescription(second)){
    return listutils::typeError("second argument must be a relation");
  }

  if(!nl->Equal(nl->Second(nl->Second(first)),
                nl->Second(nl->Second(second)))){
    return listutils::typeError("tuple types of first and second arg  differ");
  }

  if(nl->ListLength(third<1)){
    return listutils::typeError("third arg must be a list of maps");
  }

  rest = third;
  noAttrs = nl->ListLength(third);
  // Go through all functions
  while (!(nl->IsEmpty(rest)))
  {
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);

    // Is it a function?
    if(!listutils::isMap<1>(second2)){
      return listutils::typeError("not a map found");
    }
    if(!nl->Equal(nl->Second(first),nl->Second(second2))){
      return listutils::typeError("argument of map is wrong");
    }

    // Is the name of the attribute that shall be computed by the
    // function a valid name
    if(!listutils::isSymbol(first2)){
      return listutils::typeError("not a valid attribute name:" +
                                  nl->ToString(first2));
    }
    nl->WriteToString(argstr, first2);
    attrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                              argstr, attrType);
    if(attrIndex==0){
      return listutils::typeError("attribute " + argstr + " not known");
    }
    if(!nl->Equal(attrType, nl->Third(second2))){
      return listutils::typeError("result of the map and attribute"
                                  " type differ");
    }
    // Construct a list with all indices of the changed attributes
    // in the inputstream to be appended to the resultstream
    if (firstcall)
    {
      numberList = nl->OneElemList(nl->IntAtom(attrIndex));
      lastNumberList = numberList;
      firstcall = false;
    }
    else
    {
      lastNumberList =
        nl->Append(
          lastNumberList,
          nl->IntAtom(attrIndex));
    }
  }
  // build first part of the resultstream
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  // build second part of the resultstream
  rest = nl->Second(nl->Second(first));
  while (!(nl->IsEmpty(rest)))
  {
    nl->WriteToString(oldName, nl->First(nl->First(rest)));
    oldName += "_old";
    oldAttribute =
      nl->TwoElemList(
        nl->SymbolAtom(oldName),
        nl->Second(nl->First(rest)));
    lastlistn = nl->Append(lastlistn,oldAttribute);
    rest = nl->Rest(rest);
  }
  lastlistn =
    nl->Append(
      lastlistn,
      nl->TwoElemList(
        nl->SymbolAtom("TID"),
        nl->SymbolAtom(TupleIdentifier::BasicType())));
  outlist =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(
        nl->IntAtom(noAttrs),
        numberList),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          listn)));
  return outlist;
}

/*
2.30.1 Type mapping function of operator ~updatedirect~

*/

ListExpr updateDirectRelTypeMap(ListExpr args)
{
  return updateTypeMap(args, "updatedirect");
}
/*
2.30.2 Value mapping function of operator ~updatedirect~

*/
int UpdateDirect(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Word t, value, elem;
  Tuple* tup;
  Supplier supplier, supplier2, supplier3, son;
  int  noOfAttrs, index;
  ArgVectorPointer funargs;
  TupleType *resultTupleType;
  ListExpr resultType;
  GenericRelation* relation;
  Attribute* newAttribute;

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
      relation = (GenericRelation*) args[1].addr;
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        // Copy the attributes from the old tuple
        assert( newTuple->GetNoAttributes() ==
                2 * tup->GetNoAttributes() + 1);
        for (int i = 0; i < tup->GetNoAttributes(); i++)
          newTuple->PutAttribute(
            tup->GetNoAttributes()+i, tup->GetAttribute(i)->Clone());
        // Number of attributes to be replaced
        noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
        // Supplier for the functions
        supplier = args[2].addr;
        vector<int>* changedIndices = new vector<int>(noOfAttrs);
        vector<Attribute*>* newAttrs =
          new vector<Attribute*>(noOfAttrs);
        for (int i=1; i <= noOfAttrs; i++)
        {
          // Get next appended index
          son = qp->GetSupplier(args[4].addr, i-1);
          qp->Request(son, elem);
          index = ((CcInt*)elem.addr)->GetIntval() -1;
          (*changedIndices)[i-1] = index;
          // Get next function definition
          supplier2 = qp->GetSupplier(supplier, i-1);
          // Get the function
          supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          ((*funargs)[0]).setAddr(tup);
          qp->Request(supplier3,value);
          newAttribute = ((Attribute*)value.addr)->Clone();
          (*newAttrs)[i-1] = newAttribute;
        }
        relation->UpdateTuple(tup,*changedIndices,*newAttrs);
        for (int i = 0; i < tup->GetNoAttributes(); i++)
          newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute(
          newTuple->GetNoAttributes() - 1, tidAttr );
        delete changedIndices;
        delete newAttrs;
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        resultTupleType = (TupleType *)local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.18.3 Specification of operator ~updatedirect~

*/
const string UpdateDirectSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x)) x [(a1, (tuple(x)"
  " -> d1)) ... (an, (tuple(x) -> dn))] -> "
  "stream(tuple(x @ [x1_old t1] @...[xn_old tn] @ [TID tid])))"
  "</text--->"
  "<text>_ _ updatedirect [funlist]</text--->"
  "<text>Updates each input tuple by replacing "
  "attributes as specified in the parameter"
  " funlist.</text--->"
  "<text>query ten feed ten updatedirect [nr: "
  ".nr * 5] consume"
  "</text--->"
  ") )";

/*
2.18.4 Definition of operator ~updatedirect~

*/
Operator extrelupdatedirect (
  "updatedirect",              // name
  UpdateDirectSpec,            // specification
  UpdateDirect,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  updateDirectRelTypeMap          // type mapping
);


/*
2.18 Operator ~updatesearch~

For each input-tuple searches for tuples in the relation with the same values
as the input-tuple. Each found tuple is updated by  replacing the attribute-values of the attributes given by their names in the
function-argumentlist with the new values received from the corresponding functions.
The updated tuple is made persistent and as the resultstream tuples are returned that contain the new
values in first places, then all old values and finally the TupleIdentifier of the updated tuple.

2.18.1 Type mapping function of operator ~updatesearch~

*/

ListExpr updateSearchRelTypeMap(ListExpr args)
{
  return updateTypeMap(args, "updatesearch");
}
/*
2.18.2 Value mapping function of operator ~updatesearch~

*/
int UpdateSearch(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Word t, value, elem;
  Tuple* tup;
  Tuple* newTuple;
  Tuple* nextTup;
  Supplier supplier, supplier2, supplier3, son;
  int noOfAttrs, index;
  ArgVectorPointer funargs;
  ListExpr resultType;
  GenericRelation* relation;
  GenericRelationIterator* iter;
  Attribute* newAttribute;
  vector<Tuple*>* nextBucket;
  struct LocalTransport
  {
    LocalTransport():
    updatedTuples( new vector<Tuple*>() ),
    hashTable( new vector<vector<Tuple*>*>(256) ),
    resultTupleType( 0 )
    {
      for (int i = 0; i < 256; i++)
        (*hashTable)[i] = new vector<Tuple*>();
    }

    ~LocalTransport()
    {
      for( vector<Tuple*>::iterator i = updatedTuples->begin();
           i != updatedTuples->end();
           i++ )
      {
        (*i)->DeleteIfAllowed();
      }
      delete updatedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      resultTupleType->DeleteIfAllowed();
    }

    vector<Tuple*>* updatedTuples;
    vector<vector<Tuple*>*>* hashTable;
    TupleType* resultTupleType;
  } *localTransport;
  size_t hashValue ;
  bool tupleFound;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      localTransport = new LocalTransport();
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);
      iter = relation->MakeScan();
      nextTup = iter->GetNextTuple();
      // Fill hashtable
      while (!iter->EndOfScan())
      {
        hashValue = 0;
        for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
           hashValue +=
             ((Attribute*)
              (nextTup->GetAttribute(i)))->HashValue();
        nextBucket =
          localTransport->hashTable->operator[](hashValue % 256);
        nextBucket->push_back(nextTup);
        nextTup = iter->GetNextTuple();
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType =
        new TupleType( nl->Second( resultType ) );
      local.setAddr( localTransport );
      return 0;

    case REQUEST :

      localTransport =  (LocalTransport*) local.addr;
      // Check if an already updated duplicate of the last
      // inputtuple has to be given to the resultstream first
      if (!localTransport->updatedTuples->empty())
      {
        newTuple = localTransport->updatedTuples->back();
        newTuple->DeleteIfAllowed();
        localTransport->updatedTuples->pop_back();
        result.setAddr(newTuple);
        return YIELD;
      }
      // No more duplicates to send to the resultstream
      relation = (GenericRelation*) args[1].addr;
      tupleFound = false;
      // tupleFound will stay false until a tuple with the same
      // values as the inputtuple was found and updated
      while (! tupleFound)
      {
        qp->Request(args[0].addr,t);
        if (qp->Received(args[0].addr))
        {
          tup = (Tuple*)t.addr;
          hashValue = 0;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            hashValue +=
              ((Attribute*)
                (tup->GetAttribute(i)))->HashValue();
          SortOrderSpecification full;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            full.push_back(pair<int,bool> (i+1,false));
          TupleCompareBy compare(full);
          // Get the right bucket from the hashtable
          nextBucket =
            localTransport->hashTable->operator[](hashValue % 256);
          // Get all tuples from the bucket that have the same
          // attributevalues as the inputtuple
          for (size_t j = 0; j < nextBucket->size(); j++)
          {
            nextTup = nextBucket->operator[](j);
            if (nextTup != 0)
            {
              if(!compare(nextTup,tup) && !compare(tup,nextTup))
              {
                newTuple =
                  new Tuple(localTransport->resultTupleType);
                assert( newTuple->GetNoAttributes() ==
                        2 * nextTup->GetNoAttributes() + 1);

                for (int i = 0; i < nextTup->GetNoAttributes(); i++)
                  newTuple->PutAttribute(
                    nextTup->GetNoAttributes()+i,
                    nextTup->GetAttribute(i)->Clone());
                noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
                // Supplier for the functions
                supplier = args[2].addr;
                vector<int>* changedIndices =
                  new vector<int>(noOfAttrs);
                vector<Attribute*>* newAttrs =
                  new vector<Attribute*>(noOfAttrs);
                for (int i=1; i <= noOfAttrs; i++)
                {
                  // Supplier for the next index
                  son = qp->GetSupplier(args[4].addr, i-1);
                  qp->Request(son, elem);
                  index = ((CcInt*)elem.addr)->GetIntval() -1;
                  (*changedIndices)[i-1] = index;
                  // Suppliers for the next function
                  supplier2 = qp->GetSupplier(supplier, i-1);
                  supplier3 = qp->GetSupplier(supplier2, 1);
                  funargs = qp->Argument(supplier3);
                  ((*funargs)[0]).setAddr(nextTup);
                  qp->Request(supplier3,value);
                  newAttribute =
                    ((Attribute*)value.addr)->Clone();
                  (*newAttrs)[i-1] = newAttribute;
                }
                relation->UpdateTuple(
                  nextTup,*changedIndices,*newAttrs);
                for (int i = 0; i < nextTup->GetNoAttributes(); i++)
                  newTuple->CopyAttribute( i, nextTup, i );
                const TupleId& tid = nextTup->GetTupleId();
                Attribute* tidAttr =
                  new TupleIdentifier(true,tid);
                newTuple->PutAttribute(
                  newTuple->GetNoAttributes() - 1, tidAttr );
                delete changedIndices;
                delete newAttrs;
                newTuple->IncReference();
                localTransport->updatedTuples->push_back(newTuple);
              }
            }
          }
          // Check if at least one tuple was updated
          if (!localTransport->updatedTuples->empty())
          {
            newTuple = localTransport->updatedTuples->back();
            newTuple->DeleteIfAllowed();
            localTransport->updatedTuples->pop_back();
            result.setAddr(newTuple);
            tupleFound = true;
          }
          tup->DeleteIfAllowed();
        }
        else// if (qp->Received(args[0].addr))
          return CANCEL;
      }// while (! tupleFound);
      return YIELD;

    case CLOSE :

      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        localTransport = (LocalTransport*) local.addr;
        delete localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.18.3 Specification of operator ~updatesearch~

*/
const string UpdateSearchSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x)) x [(a1, (tuple(x)"
  " -> d1)) ... (an, (tuple(x) -> dn))] -> "
  "stream(tuple(x @ [x1_old t1]@...[xn_old tn] @ [TID tid])))"
  "</text--->"
  "<text>_ _ updatesearch [funlist]</text--->"
  "<text>Each tuple of the relation with the same values as "
  "one input tuple is updated.</text--->"
  "<text>query staedteUpdate feed staedte updatesearch [Bev: "
  ".Bev + 1000] consume"
  "</text--->"
  ") )";

/*
2.18.4 Definition of operator ~updatesearch~

*/
Operator extrelupdatesearch (
  "updatesearch",              // name
  UpdateSearchSpec,            // specification
  UpdateSearch,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  updateSearchRelTypeMap          // type mapping
);

/*
2.18 Operator ~updatedirectsave~

Updates each input tuple by  replacing the attribute values of the attributes
given by their names in the function-argumentlist with the new values received
from the functions. Precondition is that the tuples of the input stream
originally belong to the relation that shall be updated.  The updated tuple is
made persistent and as the resultstream tuples are returned that contain the
new values in first places, then all old values and finally the tuple identifier (TID)
of the updated tuple. Additionally, the resulting tuples are stored in an auxiliary
relation given as the third argument

2.18.0  General Type mapping functions of operators ~updatedirectsave~ and ~updatesearchsave~

Type mapping for ~updatedirectsave~ and ~updatesearchsave~ is

----     (stream X) (rel X) (rel(tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn)(TID tid))))
     ((ai1 (map x xi1)) ... (aij (map x xij)))

        -> (stream (tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn)(TID tid))))

        where X = (tuple ((a1 x1) ... (an xn))) and i1, ..., ij in {1, ... , n}
----

*/

ListExpr updateSaveTypeMap( ListExpr& args, string opName )
{
  ListExpr first, second,third,fourth,errorInfo, rest, listn,
           lastlistn, first2, second2, firstr,attrType,numberList,
           lastNumberList,oldAttribute, outlist;
  int attrIndex, noAttrs ;
  bool firstcall = true;
  AlgebraManager* algMgr;
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  algMgr = SecondoSystem::GetAlgebraManager();
  string argstr, argstr2,argstr3,oldName;

  if(nl->ListLength(args)!=4){
    return listutils::typeError("four arguments expected");
  }

  first = nl->First(args);
  second  = nl->Second(args);
  third = nl->Third(args);
  fourth = nl->Fourth(args);
  // Check inputstream
  if(!listutils::isTupleStream(first)){
    return listutils::typeError("first argument must be a tuple stream");
  }
  if(!listutils::isRelDescription(second) &&
    !listutils::isOrelDescription(second)){
    return listutils::typeError("second argument must be a relation");
  }
  if(!nl->Equal(nl->Second(first),nl->Second(second))){
    return listutils::typeError("tuple type of first and"
                                " second argument differ");
  }

  if(!listutils::isRelDescription(third)){
    return listutils::typeError("third argument must be a relation");
  }

  if(nl->ListLength(fourth)<1){
    return listutils::typeError("fourth argument must be a function list");
  }
  rest = fourth;
  noAttrs = nl->ListLength(fourth);


  // Check each update-function
  while (!(nl->IsEmpty(rest)))
  {
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);

    if(!listutils::isMap<1>(second2)){
      return listutils::typeError("found a non function in function list");
    }
    if(!nl->Equal(nl->Second(first), nl->Second(second2))){
       return listutils::typeError("tuple type and result type of a"
                                   " function differ");
    }
    if(!listutils::isSymbol(first2)){
      return listutils::typeError("invalid attribute name found");
    }
    // Is the function attributename a valid name?
    nl->WriteToString(argstr, first2);
    attrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                                         argstr, attrType);
    if(attrIndex==0){
      return listutils::typeError("attribute " + argstr + " unknown");
    }
    if(!nl->Equal(attrType, nl->Third(second2))){
      return  listutils::typeError("result type of function differs from"
                                   " corresponding attribute type");
    }
    // build a list with all indices of the updated attributes in
    // the update-relation to be appended to the resultstream
    if (firstcall)
    {
      numberList = nl->OneElemList(nl->IntAtom(attrIndex));
      lastNumberList = numberList;
      firstcall = false;
    }
    else
    {
      lastNumberList =
        nl->Append(
          lastNumberList,
          nl->IntAtom(attrIndex));
    }
  }
  // build first part of the resultstream
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  // build second part of the resultstream
  rest = nl->Second(nl->Second(first));
  while (!(nl->IsEmpty(rest)))
  {
    nl->WriteToString(oldName, nl->First(nl->First(rest)));
    oldName += "_old";
    oldAttribute =
      nl->TwoElemList(
        nl->SymbolAtom(oldName),
        nl->Second(nl->First(rest)));
    lastlistn = nl->Append(lastlistn,oldAttribute);
    rest = nl->Rest(rest);
  }
  lastlistn =
    nl->Append(
      lastlistn,
      nl->TwoElemList(
        nl->SymbolAtom("TID"),
        nl->SymbolAtom(TupleIdentifier::BasicType())));

  outlist = nl->ThreeElemList(
              nl->SymbolAtom(Symbol::APPEND()),
              nl->TwoElemList(
                nl->IntAtom(noAttrs),
                numberList),
              nl->TwoElemList(
                nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList(
                  nl->SymbolAtom(Tuple::BasicType()),
                  listn)));

  // cout << "typemap-result:" << nl->ToString(outlist) << endl;

  return outlist;
}

/*
2.30.1 Type mapping function of operator ~updatedirectsave~

*/

ListExpr updateDirectSaveRelTypeMap(ListExpr args)
{
  return updateSaveTypeMap(args, "updatedirectsave");
}

/*
2.30.2 Value mapping function of operator ~updatedirectsave~

*/
int UpdateDirectSave(Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
  Word t, value, elem;
  t.addr = 0; value.addr = 0; elem.addr = 0;
  Tuple* tup = 0;
  Supplier supplier, supplier2, supplier3, son;
  int  noOfAttrs=0, index=0;
  ArgVectorPointer funargs;
  TupleType *resultTupleType = 0;
  ListExpr resultType;
  GenericRelation* relation = 0;
  Relation* auxRelation  = 0;
  Attribute* newAttribute = 0;

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
      relation = (GenericRelation*) args[1].addr;
      auxRelation = (Relation*)(args[2].addr);
      assert(auxRelation != 0);
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
	//tup->IncReference();

        Tuple *newTuple = new Tuple( resultTupleType );
        assert( newTuple->GetNoAttributes() ==
                2 * tup->GetNoAttributes() + 1);

	// store old values int the result tuple
        for (int i = 0; i < tup->GetNoAttributes(); i++)
        {
          newTuple->CopyAttribute(i, tup , i);
          newTuple->CopyAttribute(i, tup , tup->GetNoAttributes()+i);
        }

        noOfAttrs = ((CcInt*)args[4].addr)->GetIntval();
        // Get the supplier for the updatefunctions
        supplier = args[3].addr;
        vector<int>* changedIndices = new vector<int>(noOfAttrs);
        vector<Attribute*>* newAttrs = new vector<Attribute*>(noOfAttrs);

        for (int i=1; i <= noOfAttrs; i++)
        { // supplier for the next index of an updated attribute
          son = qp->GetSupplier(args[5].addr, i-1);
          qp->Request(son, elem);
          index = ((CcInt*)elem.addr)->GetIntval() -1;
          (*changedIndices)[i-1] = index;

          // Suppliers for the next updatefunction
          supplier2 = qp->GetSupplier(supplier, i-1);
          supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          ((*funargs)[0]).setAddr(tup);
          qp->Request(supplier3,value);
          newAttribute = ((Attribute*)value.addr)->Clone();
          (*newAttrs)[i-1] = newAttribute;

          // store new value in result tuple
          newTuple->PutAttribute( index, newAttribute->Copy() );
        }

        // store tid in output tuple
        const TupleId& tid = tup->GetTupleId();
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute(
                        newTuple->GetNoAttributes() - 1, tidAttr);

        // copy new attribute values into auxTuple
        Tuple *auxTuple = new Tuple( auxRelation->GetTupleType() );

        for (int i=0; i< auxTuple->GetNoAttributes() - 1; i++){
            auxTuple->CopyAttribute( i, newTuple, i);
        }
        auxTuple->PutAttribute(auxTuple->GetNoAttributes()-1, tidAttr->Copy());

        auxRelation->AppendTuple(auxTuple);
        auxTuple->DeleteIfAllowed();

        relation->UpdateTuple(tup,*changedIndices,*newAttrs);
        
        tup->DeleteIfAllowed();

        delete changedIndices;
        delete newAttrs;
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      qp->SetModified(qp->GetSon(s, 2));
      if(local.addr)
      {
        resultTupleType = (TupleType *)local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.30.3 Specification of operator ~updatedirectsave~

*/
const string UpdateDirectSaveSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x @ x @ [TID tid]))"
  " x [(a1, (tuple(x)"
  " -> d1)) ... (an, (tuple(x) -> dn))] -> "
  "stream(tuple(x @[x1_old t1]@ ...[xn_old tn] @ [TID tid]))"
  "</text--->"
  "<text>_ _ _ updatedirectsave [funlist]</text--->"
  "<text>Like updatedirect but saves the resulttuples"
  " to the auxiliary relation.</text--->"
  "<text>query ten feed ten tenU updatedirectsave [nr : "
  ".nr * 5] consume"
  "</text--->"
  ") )";

/*
2.30.4 Definition of operator ~updatedirectsave~

*/
Operator extrelupdatedirectsave (
  "updatedirectsave",              // name
  UpdateDirectSaveSpec,            // specification
  UpdateDirectSave,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  updateDirectSaveRelTypeMap          // type mapping
);


/*
2.31 Operator ~updatesearchsave~

Updates each tuple of the relation that has the same values as one of the
input-tuples by replacing the attributevalues of the attributes given by their
names in the function-argumentlist with the new values received from the
functions.  The updated tuple is made persistent and as the resultstream tuples
are returned that contain the new values in first places, then all old values
and finally the TupleIdentifier of the updated tuple. Additionally the
resulttuples are stored in an auxiliary relation given as the third argument

2.31.1 Type mapping function of operator ~updatesearchsave~

*/

ListExpr updateSearchSaveRelTypeMap(ListExpr args)
{
  return updateSaveTypeMap(args, "updatesearchsave");
}

/*
2.31.2 Value mapping function of operator ~updatesearchsave~


*/
int UpdateSearchSave(Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
  Word t, value, elem;
  t.addr = 0; value.addr = 0; elem.addr = 0;
  Tuple* tup = 0;
  Tuple* newTuple = 0;
  Tuple* nextTup = 0;
  Supplier supplier, supplier2, supplier3, son;
  int noOfAttrs = 0, index = 0;
  ArgVectorPointer funargs;
  ListExpr resultType = nl->Empty();
  GenericRelation* relation = 0;
  Relation* auxRelation = 0;
  GenericRelationIterator* iter = 0;
  Attribute* newAttribute = 0;

  typedef vector<Tuple*> Bucket;
  Bucket* nextBucket = 0;

  struct LocalTransport
  {

    LocalTransport(size_t bucks = 997 ):
    buckets(bucks),
    updatedTuples( new Bucket() ),
    hashTable( new vector<Bucket*>(buckets) ),
    resultTupleType( 0 )
    {
      vector<Bucket*>::iterator i = hashTable->begin();
      for (; i != hashTable->end(); i++) {
        *i = new Bucket();
      }
    }

    ~LocalTransport()
    {
      for( Bucket::iterator i = updatedTuples->begin();
           i != updatedTuples->end();
           i++ )
      {
        (*i)->DeleteIfAllowed();
      }
      delete updatedTuples;

      vector<Bucket*>::iterator i = hashTable->begin();
      for (; i != hashTable->end(); i++)
      {
        for( Bucket::iterator j = (*i)->begin();
             j != (*i)->end();
             j++ )
        {
          (*j)->DeleteIfAllowed();
        }
        delete (*i);
      }
      delete hashTable;

      resultTupleType->DeleteIfAllowed();
    }

    inline Bucket* getBucket(size_t hashvalue)
    {
      return (*hashTable)[hashvalue % buckets];
    }

    inline void newUpdate(Tuple* t)
    {
      t->IncReference();
      updatedTuples->push_back(t);
    }

    inline Tuple* lastUpdate()
    {
      if ( updatedTuples->empty() ) {
        return 0;
      }

      Tuple* t = updatedTuples->back();
      t->DeleteIfAllowed();
      updatedTuples->pop_back();

      return t;
    }

    private: size_t buckets;

    public:
    Bucket* updatedTuples;
    vector<Bucket*>* hashTable;
    TupleType* resultTupleType;
  };

  LocalTransport* localTransport;
  size_t hashValue ;
  bool tupleFound;

  switch (message)
  {
    case OPEN :

      localTransport = new LocalTransport();
      qp->Open(args[0].addr);
      relation = (GenericRelation*)(args[1].addr);
      assert(relation != 0);

      // fill hashtable
      iter = relation->MakeScan();
      nextTup = iter->GetNextTuple();
      while (!iter->EndOfScan())
      {
        hashValue = 0;
        for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
          hashValue +=
            ((Attribute*)
              (nextTup->GetAttribute(i)))->HashValue();
        nextBucket =
          localTransport->getBucket(hashValue);
        nextBucket->push_back(nextTup);
        if(!iter->EndOfScan()){  
            nextTup = iter->GetNextTuple();
        }
      }
      delete iter;

      resultType = GetTupleResultType( s );
      localTransport->resultTupleType =
        new TupleType( nl->Second( resultType ) );
      local.setAddr( localTransport );
      return 0;

    case REQUEST : {

      localTransport =  (LocalTransport*) local.addr;

      // Check if an already updated duplicate of the last
      // input tuples has to be sent to the outputstream first
      Tuple* lt = localTransport->lastUpdate();
      if ( lt != 0 )
      {
        result.setAddr(lt);
        return YIELD;
      }

      // No more duplicates to be sent to the outputstream
      relation = (GenericRelation*) args[1].addr;
      auxRelation = (Relation*)(args[2].addr);
      assert(auxRelation != 0);

      // tupleFound will stay false until a tuple with the same
      // values as one of the input tuples was found
      tupleFound = false;
      while ( !tupleFound )
      {
        qp->Request(args[0].addr,t);
        if (qp->Received(args[0].addr))
        {
	  // process a tuple of the input stream
          tup = (Tuple*)t.addr;
          hashValue = 0;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            hashValue +=
              ((Attribute*)
                (tup->GetAttribute(i)))->HashValue();
          SortOrderSpecification full;
          for( int i = 0; i < tup->GetNoAttributes(); i++ )
            full.push_back(pair<int,bool> (i+1,false));
          TupleCompareBy compare(full);

          // look up the hash table for the correct bucket
          nextBucket =
            localTransport->getBucket(hashValue);

          // Look for all tuples in the bucket if they have the
          // same attribute values
          for (size_t j = 0; j < nextBucket->size(); j++)
          {
            nextTup = (*nextBucket)[j];
            if (nextTup != 0)
            {
              if(!compare(nextTup,tup) && !compare(tup,nextTup))
              {
		int nextTupAttrs = nextTup->GetNoAttributes();
                newTuple = new Tuple( localTransport->resultTupleType );
                assert( newTuple->GetNoAttributes() ==
                        2 * nextTupAttrs + 1);
                for (int i = 0; i < nextTupAttrs; i++) {
                   newTuple->CopyAttribute(i, nextTup , i);
                   newTuple->CopyAttribute(i, nextTup , nextTupAttrs+i);
		}

                noOfAttrs = ((CcInt*)args[4].addr)->GetIntval();
                // Supplier for the updatefunctions
                supplier = args[3].addr;
                vector<int>* changedIndices = new vector<int>(noOfAttrs);
                vector<Attribute*>* newAttrs
			              = new vector<Attribute*>(noOfAttrs);

                for (int i=1; i <= noOfAttrs; i++)
                {
                  // Supplier for the next attributeindex
                  son = qp->GetSupplier(args[5].addr, i-1);
                  qp->Request(son, elem);
                  index = ((CcInt*)elem.addr)->GetIntval() -1;
                  (*changedIndices)[i-1] = index;
                  // Suppliers for the next updatefunctions
                  supplier2 = qp->GetSupplier(supplier, i-1);
                  supplier3 = qp->GetSupplier(supplier2, 1);
                  funargs = qp->Argument(supplier3);
                  ((*funargs)[0]).setAddr(nextTup);
                  qp->Request(supplier3,value);
                  newAttribute =
                    ((Attribute*)value.addr)->Clone();
                  (*newAttrs)[i-1] = newAttribute;

		  // change attribute value for the output tuple
		  newTuple->PutAttribute(index, newAttribute);
                }
		nextTup->IncReference();
                relation->UpdateTuple( nextTup,*changedIndices,*newAttrs);

		// add tid to newTuple
		const TupleId& tid = nextTup->GetTupleId();
                Attribute* tidAttr = new TupleIdentifier(true,tid);
                newTuple->PutAttribute(
                  newTuple->GetNoAttributes() - 1, tidAttr);

		// copy the updated tuple into auxTuple and append it
		// to the auxiliary relation
                Tuple *auxTuple = new Tuple( auxRelation->GetTupleType() );
                for (int i = 0; i < auxTuple->GetNoAttributes()-1; i++)
                {
                  auxTuple->CopyAttribute(i, newTuple, i);
                }
                auxTuple->PutAttribute( auxTuple->GetNoAttributes()-1,
				        tidAttr->Copy() );

                auxRelation->AppendTuple(auxTuple);

                auxTuple->DeleteIfAllowed();
                delete changedIndices;
                delete newAttrs;
                localTransport->newUpdate(newTuple);
              }
            }
          }
          // Check if at least one updated tuple has to be send to
          // the outputstream
	  Tuple* t = localTransport->lastUpdate();
          if ( t != 0 )
          {
            result.setAddr(t);
            tupleFound = true;
          }
          tup->DeleteIfAllowed();
        }
        else {
	  // if (qp->Received(args[0].addr))
          return CANCEL;
	}
      }// while ( !tupleFound );
      return YIELD;

    } // case REQUEST

    case CLOSE :

      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      qp->SetModified(qp->GetSon(s, 2));
      if(local.addr)
      {
        localTransport = (LocalTransport*) local.addr;
        delete localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.31.3 Specification of operator ~updatesearchsave~

*/
const string UpdateSearchSaveSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x)) x rel(tuple(x)) x "
  "(rel(tuple(x @ x @ [TID tid]))) x [(a1, (tuple(x)"
  " -> d1)) ... (an, (tuple(x) -> dn))] -> "
  "stream(tuple(x @[x1_old t1]@...[xn_old t1] @ [TID tid]))"
  "</text--->"
  "<text>_ _ _updatesearchsave [funlist]</text--->"
  "<text>Like `updatesearch` but each result-tuple "
  " is appended to the auxiliary relation.</text--->"
  "<text>query staedteUp feed staedte staedteU "
  "updatesearchsave [Bev : .Bev +1000] consume"
  "</text--->"
  ") )";

/*
2.31.4 Definition of operator ~updatesearchsave~

*/
Operator extrelupdatesearchsave (
  "updatesearchsave",              // name
  UpdateSearchSaveSpec,            // specification
  UpdateSearchSave,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  updateSearchSaveRelTypeMap          // type mapping
);

/*
2.32 Operator ~addid~

2.32.1 Type mapping function of operator ~addid~

----    ((stream (tuple (x1 ... xn))))

        -> (stream (tuple (x1 ... xn (TID tid))))
----

*/

ListExpr appendIdentifierTypeMap (ListExpr args)
{
  ListExpr first, rest,listn,lastlistn, outList;
    string argstr;

  if(nl->ListLength(args)!=1){
   return listutils::typeError("one argument expected");
  }

  first = nl->First(args);

  if(!listutils::isTupleStream(first)){
    return  listutils::typeError("tuple stream expected");
  }

  // build resultlist
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn =
    nl->Append(
      lastlistn,
      nl->TwoElemList(
        nl->SymbolAtom("TID"),
        nl->SymbolAtom(TupleIdentifier::BasicType())));
  outList =
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        listn));
  return outList;
}


/*
2.32.2 Value mapping function of operator ~addid~

*/
int appendIdentifierValueMap(Word* args, Word& result, int message,
                             Word& local, Supplier s)
{
  Word t;
  Tuple* tup;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
//         cout << "TID: " << tid << endl;
        Attribute* tidAttr = new TupleIdentifier(true,tid);
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr);
        result.setAddr(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      if(local.addr)
      {
        resultTupleType = (TupleType*) local.addr;
        resultTupleType->DeleteIfAllowed();
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}


/*
2.32.3 Specification of operator ~addid~

*/
const string appendIdentifierSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" \" \" \" \") "
  "( <text>stream(tuple(x)) "
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ addid</text--->"
  "<text>Appends an attribute which is the"
  " tuple-id of the tuple to each tuple.</text--->"
  "<text>query staedte feed addid consume"
  "</text--->"
  "<text>Apply addid directly after a feed, because other </text--->"
  "<text>operators my corrupt the tid </text--->"
  "<text>(in-memory tuples all have tid=0).</text--->"
  ") )";

/*
2.32.4 Definition of operator ~addid~

*/
Operator extreladdid (
  "addid",              // name
  appendIdentifierSpec,            // specification
  appendIdentifierValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  appendIdentifierTypeMap         // type mapping
);

/*
2.33 Operator ~deletebyid~

2.33.1 Type mapping function of operator ~deletebyid~

----    ((rel (tuple (x1 ... xn)))) x (TID tid)

        -> (stream (tuple (x1 ... xn (TID tid))))
----

*/

ListExpr deleteByIdTypeMap(ListExpr args)
{
  ListExpr first,second, rest, listn, lastlistn, outList;
  string argstr="",valueString;

  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }

  first = nl->First(args);
  second = nl->Second(args);

  if(!listutils::isRelDescription(first)){
    return listutils::typeError("first argument must be a relation");
  }
  if(!listutils::isSymbol(second,TupleIdentifier::BasicType())){
    return listutils::typeError("second argument must be a tid");
  }
  // build resultlist
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  lastlistn =
    nl->Append(
      lastlistn,
      nl->TwoElemList(
        nl->SymbolAtom("TID"),
        nl->SymbolAtom(TupleIdentifier::BasicType())));
  outList =
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        listn));
  return outList;
}


/*
2.33.2 Value mapping function of operator ~deletebyid~

*/
int deleteByIdValueMap(Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
  Tuple* resultTuple;
  Tuple* deleteTuple;
  Relation* relation;
  TupleIdentifier* tid;
  TupleType *resultTupleType;
  ListExpr resultType;
  bool* firstcall;

  switch (message)
  {
    case OPEN :
      firstcall = new bool;
      *firstcall = true;
      local.setAddr( firstcall );
      return 0;

    case REQUEST :
      firstcall = (bool*) local.addr;
      if (*firstcall)
      {
        *firstcall = false;
        resultType = GetTupleResultType( s );
        resultTupleType = new TupleType( nl->Second( resultType ) );
        relation = (Relation*)(args[0].addr);
        assert(relation != 0);
        tid = (TupleIdentifier*)(args[1].addr);
        resultTuple = new Tuple( resultTupleType );
        deleteTuple = relation->GetTuple(tid->GetTid(), true);
        if (deleteTuple == 0)
        {
           resultTupleType->DeleteIfAllowed();
           resultTuple->DeleteIfAllowed();
           return CANCEL;
        }
        for (int i = 0; i < deleteTuple->GetNoAttributes(); i++)
          resultTuple->PutAttribute(
            i, deleteTuple->GetAttribute(i)->Clone());
        relation->DeleteTuple(deleteTuple);
        resultTuple->PutAttribute(
          resultTuple->GetNoAttributes() -1, tid->Clone());
        result.setAddr(resultTuple);
        resultTupleType->DeleteIfAllowed();
        deleteTuple->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->SetModified(qp->GetSon(s, 0));
      if(local.addr)
      {
        firstcall = (bool*) local.addr;
        delete firstcall;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}



/*
2.33.3 Specification of operator ~deletebyid~

*/
const string deleteByIdSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel(tuple(x))) x (tid) "
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text> _ deletebyid _</text--->"
  "<text>Deletes the tuple with the id from the"
  "second argument from the relaton.</text--->"
  "<text>query staedte deletebyid [[const tid value (5)]] count"
  "</text--->"
  ") )";

/*
2.33.4 Definition of operator ~deletebyid~

*/
Operator extreldeletebyid (
  "deletebyid",              // name
  deleteByIdSpec,            // specification
  deleteByIdValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  deleteByIdTypeMap         // type mapping
);

/*
2.34 Operator ~updatebyid~

2.34.1 Type mapping function of operator ~updatebyid~

----    ((rel (tuple (X)))) x (tid) x ((ai1 (map x xi1)) ... (aij (map x xij))))

        -> (stream (tuple (X @ (a1_old x1) ... (an_old xn)(TID tid)))))

        where X = ((a1 x1) ... (an xn)) and ai1 - aij in (a1 .. an)
----

*/

ListExpr updateByIdTypeMap(ListExpr args)
{
  ListExpr first, second,third,errorInfo, rest, listn, lastlistn,
           first2, second2, firstr,attrType, numberList,
           lastNumberList,oldAttribute, outlist;
  string argstr,valueString, argstr2, argstr3, oldName;
  int attrIndex, noAttrs ;
  bool firstcall = true;
  AlgebraManager* algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  first = nl->First(args);
  second = nl->Second(args);
  third = nl->Third(args);

  // Check relationa
  if(!listutils::isRelDescription(first)){
    return listutils::typeError("relation as first arg expected");
  }
  // Check second argument
  if(!listutils::isSymbol(second,TupleIdentifier::BasicType())){
    return listutils::typeError("second argument should be tid");
  }

  if(nl->ListLength(third<1)){
   return listutils::typeError("non-empty function list expected "
                               "as third arg");
  }
  rest = third;
  noAttrs = nl->ListLength(third);
  // Go through all functions
  while (!(nl->IsEmpty(rest)))
  {
    firstr = nl->First(rest);
    rest = nl->Rest(rest);
    first2 = nl->First(firstr);
    second2 = nl->Second(firstr);

    // Is it a function?
    nl->WriteToString(argstr, second2);
    if(!listutils::isMap<1>(second2)){
      return listutils::typeError("invalid function definition found");
    }
    if(!nl->Equal(nl->Second(first),nl->Second(second2))){
     return listutils::typeError("argument of the function differs "
                "from the corresponding attribute");
    }

    if(!listutils::isSymbol(first2)){
      return listutils::typeError("invalid attribute name " +
                                  nl->ToString(first2));
    }
    nl->WriteToString(argstr, first2);
    attrIndex = listutils::findAttribute(nl->Second(nl->Second(first)),
                              argstr, attrType);
    if(attrIndex==0){
      return listutils::typeError("attribute " + argstr + " not known");
    }
    if(!nl->Equal(attrType,nl->Third(second2))){
      return listutils::typeError("attribute type and result type"
                                  " of the function differ");
    }
    // Construct a list with all indices of the changed attributes
    // in the inputstream to be appended to the resultstream
    if (firstcall)
    {
      numberList = nl->OneElemList(nl->IntAtom(attrIndex));
      lastNumberList = numberList;
      firstcall = false;
    }
    else
    {
      lastNumberList =
        nl->Append(
          lastNumberList,
          nl->IntAtom(attrIndex));
    }
  }
  // build first part of the resultstream
  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  while (!(nl->IsEmpty(rest)))
  {
    lastlistn = nl->Append(lastlistn,nl->First(rest));
    rest = nl->Rest(rest);
  }
  // build second part of the resultstream
  rest = nl->Second(nl->Second(first));
  while (!(nl->IsEmpty(rest)))
  {
    nl->WriteToString(oldName, nl->First(nl->First(rest)));
    oldName += "_old";
    oldAttribute =
      nl->TwoElemList(
        nl->SymbolAtom(oldName),
        nl->Second(nl->First(rest)));
    lastlistn = nl->Append(lastlistn,oldAttribute);
    rest = nl->Rest(rest);
  }
  lastlistn =
    nl->Append(
      lastlistn,
      nl->TwoElemList(
        nl->SymbolAtom("TID"),
        nl->SymbolAtom(TupleIdentifier::BasicType())));
  outlist =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(
        nl->IntAtom(noAttrs),
        numberList),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          listn)));
  return outlist;
}


/*
2.34.2 Value mapping function of operator ~updatebyid~

*/
int updateByIdValueMap(Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
  Word elem, value;
  Tuple* resultTuple;
  Tuple* updateTuple;
  Supplier supplier, supplier2, supplier3, son;
  Relation* relation;
  TupleIdentifier* tid;
  int  noOfAttrs, index;
  ArgVectorPointer funargs;
  Attribute* newAttribute;
  TupleType *resultTupleType;
  ListExpr resultType;
  bool* firstcall;

  switch (message)
  {
    case OPEN :
      firstcall = new bool;
      *firstcall = true;
      local.setAddr( firstcall );
      return 0;

    case REQUEST :
    firstcall = (bool*) local.addr;
      if (*firstcall)
      {
        *firstcall = false;
        resultType = GetTupleResultType( s );
        resultTupleType = new TupleType( nl->Second( resultType ) );
        relation = (Relation*)(args[0].addr);
        assert(relation != 0);
        tid = (TupleIdentifier*)(args[1].addr);
        resultTuple = new Tuple( resultTupleType );
        updateTuple = relation->GetTuple(tid->GetTid(), true);
        if (updateTuple == 0)
        {
           resultTupleType->DeleteIfAllowed();
           resultTuple->DeleteIfAllowed();
           return CANCEL;
        }
        for (int i = 0; i < updateTuple->GetNoAttributes(); i++)
          resultTuple->PutAttribute(
            updateTuple->GetNoAttributes() +i,
            updateTuple->GetAttribute(i)->Clone());
        // Number of attributes to be replaced
        noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
        // Supplier for the functions
        supplier = args[2].addr;
        vector<int>* changedIndices = new vector<int>(noOfAttrs);
        vector<Attribute*>* newAttrs =
          new vector<Attribute*>(noOfAttrs);
        for (int i=1; i <= noOfAttrs; i++)
        {
          // Get next appended index
          son = qp->GetSupplier(args[4].addr, i-1);
          qp->Request(son, elem);
          index = ((CcInt*)elem.addr)->GetIntval() -1;
          (*changedIndices)[i-1] = index;
          // Get next function definition
          supplier2 = qp->GetSupplier(supplier, i-1);
          // Get the function
          supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          ((*funargs)[0]).setAddr(updateTuple);
          qp->Request(supplier3,value);
          newAttribute = ((Attribute*)value.addr)->Clone();
          (*newAttrs)[i-1] = newAttribute;
        }
        relation->UpdateTuple(updateTuple,*changedIndices,*newAttrs);
        for (int i = 0; i < updateTuple->GetNoAttributes(); i++)
          resultTuple->CopyAttribute( i, updateTuple, i );
        resultTuple->PutAttribute(
          resultTuple->GetNoAttributes() -1, tid->Clone());
        result.setAddr(resultTuple);
        delete changedIndices;
        delete newAttrs;
        resultTupleType->DeleteIfAllowed();
        updateTuple->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->SetModified(qp->GetSon(s, 0));
      if(local.addr)
      {
        firstcall = (bool*) local.addr;
        delete firstcall;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}



/*
2.34.3 Specification of operator ~updatebyid~

*/
const string updateByIdSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>stream(tuple(x))) x (rel(tuple(x))) x "
  "(tid) x [(a1, (tuple(x)"
  " -> d1)) ... (an, (tuple(x) -> dn))] -> "
  "stream(tuple(x @[x1_old t1] @...[xn_old tn] @[TID tid])))"
  "</text--->"
  "<text> _ _ updatebyid [ _; funlist ]</text--->"
  "<text>Updateds the tuple with the id from the"
  "third argument by applying the functions of the "
  " list to the tuple.</text--->"
  "<text>query staedte feed staedte "
  "updatebyid [[const tid value (5)]"
  " (Bev: .Bev + 1000, PLZ: .PLZ - 50] count"
  "</text--->"
  ") )";

/*
2.34.4 Definition of operator ~updatebyid~

*/
Operator extrelupdatebyid (
  "updatebyid",              // name
  updateByIdSpec,            // specification
  updateByIdValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  updateByIdTypeMap         // type mapping
);

/*
7.3 Operator ~insertrtree~

For each tuple of the inputstream inserts an entry into the rtree. The entry is built from the
spatial-attribute over which the tree is built and the tuple-identifier wich is extracted from the
input-tuples as the last attribute. The inputstream is returned again as the result of this operator.


7.3.0 General Type mapping function of operators ~insertrtree~, ~deletertree~ and ~updatertree~




Type mapping ~insertrtree~ and ~deletertree~ on a rtree

----     (stream (tuple ((a1 x1) ... (an xn) (tid int)))) (rtree X ti) ai

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~updatertree~ on a rtree

----     (stream (tuple ((a1 x1) ... (an xn)(a1_old x1)... (an_old xn) (TID tid)))) (rtree X ti) ai

        -> (stream (tuple ((a1 x1) ... (an xn)(a1_old x1) (an_old xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

*/

ListExpr allUpdatesRTreeTypeMap( ListExpr& args, string opName )
{


   ListExpr rest,next,listn,lastlistn,restRTreeAttrs,
           oldAttribute,outList;
  string argstr, argstr2, oldName;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  if(nl->ListLength(args)!=3){
    return listutils::typeError("three arguments expected");
  }

  /* Split argument in three parts */
  ListExpr streamDescription = nl->First(args);
  ListExpr rtreeDescription = nl->Second(args);
  ListExpr nameOfKeyAttribute = nl->Third(args);

  if(!listutils::isTupleStream(streamDescription)){
   return listutils::typeError("first argument is not a tuple stream");
  }
  // Test if last attribute is of type 'tid'
  rest = nl->Second(nl->Second(streamDescription));
  while (!(nl->IsEmpty(rest)))
  {
    next = nl->First(rest);
    rest = nl->Rest(rest);
  }
  if(!listutils::isSymbol(nl->Second(next),TupleIdentifier::BasicType())){
    return listutils::typeError("last attribute in the tuple must be a tid");
  }

  if(!listutils::isRTreeDescription(rtreeDescription)){
    return listutils::typeError("second argument is not an rtree");
  }

  // Test rtree

  ListExpr rtreeSymbol = nl->First(rtreeDescription);;
  ListExpr rtreeTupleDescription = nl->Second(rtreeDescription);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);
  ListExpr rtreeType = nl->Fourth(rtreeDescription);

  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  /* handle rtree type constructor */
  if(!listutils::isSymbol(rtreeSymbol,RTree2TID::BasicType()) &&
     !listutils::isSymbol(rtreeSymbol,RTree3TID::BasicType())){
    return listutils::typeError("rtree or rtree3 expected as third argument");
  }

  if(!listutils::isDATA(rtreeKeyType)){
    return listutils::typeError("the rtree does not index a relation");
  }

  if( nl->BoolValue(rtreeType)){
     return listutils::typeError("double indexing not supported");
  }


  if(!listutils::isSymbol(nameOfKeyAttribute)){
    return listutils::typeError("key attribute in rtree is not a valid name");
  }

  // Check whether tupledescription of the stream without the last
  // attribute is the same as the tupledescription of the rtree
  rest = nl->Second(nl->Second(streamDescription));
  if(nl->ListLength(rest)<2){
    return listutils::typeError("at least two attributes"
                                " required in tuple stream");
  }

  //Test if stream-tupledescription fits to btree-tupledescription
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  rest = nl->Rest(rest);
  // For updates the inputtuples need to carry the old
  // attributevalues after the new values but their names with
  // an additional _old at the end
  if (opName == "updatebtree")
  {
    // Compare first part of the streamdescription
    while (nl->ListLength(rest) > nl->ListLength(rtreeAttrList) + 1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }

    if(!nl->Equal(listn,rtreeAttrList)){
      return listutils::typeError("tuple descriprions differ");
    }
    // Compare second part of the streamdescription
    restRTreeAttrs = rtreeAttrList;
    while (nl->ListLength(rest) >  1)
    {
      nl->WriteToString(oldName,
                        nl->First(nl->First(restRTreeAttrs)));
      oldName += "_old";
      oldAttribute =
        nl->TwoElemList(
          nl->SymbolAtom(oldName),
          nl->Second(nl->First(restRTreeAttrs)));
      if(!nl->Equal(oldAttribute,nl->First(rest))){
        return listutils::typeError(
                   "Second part of the tupledescription of the stream "
                   "without the last attribute has to be the same as the "
                   "tupledescription of the rtree except for that"
                   " the attributenames carry an additional '_old.'");
      }
      rest = nl->Rest(rest);
      restRTreeAttrs = nl->Rest(restRTreeAttrs);
    }
  }
  // For insert and delete check whether tupledescription of the
  // stream without the last attribute is the same as the
  // tupledescription of the rtree
  else
   // operators insertrtree and deletertree
  {
    while (nl->ListLength(rest) > 1)
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      rest = nl->Rest(rest);
    }
    if(!nl->Equal(listn,rtreeAttrList)){
      return listutils::typeError(
                        " tupledescription of the stream without the"
                        "last attribute has to be the same as the "
                        "tupledescription of the rtree");
    }
  }

  // Test if attributename of the third argument exists as a name
  // in the attributlist of the streamtuples
  string attrname = nl->SymbolValue(nameOfKeyAttribute);
  ListExpr attrType;
  int j = listutils::findAttribute(listn,attrname,attrType);
  if(j==0){
    return listutils::typeError("attribute " + attrname + " not known");
  }
  //Test if type of the attriubte which shall be taken as a key is
  // the same as the keytype of the rtree
  if(!nl->Equal(attrType,rtreeKeyType)){
    return listutils::typeError("attribute type in rtree and relation differ");
  }

  // Check if indexed attribute has a spatial-type
  if( !listutils::isKind(attrType,Kind::SPATIAL2D()) &&
      !listutils::isKind(attrType,Kind::SPATIAL3D()) &&
      !listutils::isKind(attrType,Kind::SPATIAL4D()) &&
      !listutils::isSymbol(attrType,Rectangle<2>::BasicType()) &&
      !listutils::isSymbol(attrType,Rectangle<3>::BasicType()) &&
      !listutils::isSymbol(attrType,Rectangle<4>::BasicType()) ){
    return listutils::typeError("indexed type not supported"
                                " (not in SPATIAL2D, SPATOIAL3D, SPATIAL4D,"
                                " rect, rect3, rect4}");
  }

   // Extract dimension and spatianltype to append them to the
  // resultlist
  int dim = 0;
  int spatial = 0;
  if (nl->IsEqual(attrType, Rectangle<2>::BasicType()))
    dim = 2;
  if (nl->IsEqual(attrType, Rectangle<3>::BasicType()))
    dim = 3;
  if (nl->IsEqual(attrType, Rectangle<4>::BasicType()))
    dim = 4;
  if (algMgr->CheckKind(Kind::SPATIAL2D(), attrType, errorInfo))
  {
    dim = 2;
    spatial = 1;
  }
  if (algMgr->CheckKind(Kind::SPATIAL3D(), attrType, errorInfo))
  {
    dim = 3;
    spatial = 1;
  }
  if (algMgr->CheckKind(Kind::SPATIAL4D(), attrType, errorInfo))
  {
    dim = 4;
    spatial = 1;
  }
  // Append the index of the attribute over which the btree is built
  // to the resultlist.
  ListExpr append = nl->OneElemList(nl->IntAtom(j));
  ListExpr lastAppend = append;
  //Append the dimension of the spatial-attribute to the resutllist.
  lastAppend = nl->Append(lastAppend,nl->IntAtom(dim));
  //Append if the index-attribute is of 'rect'- or 'spatial'-type
  lastAppend = nl->Append(lastAppend,nl->IntAtom(spatial));
  //Append the index of the attribute over which the btree is built
  //to the resultlist.
  outList =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(append),
      streamDescription);
  return outList;
}

/*

7.3.1 TypeMapping of operator ~insertrtree~

*/

ListExpr insertRTreeTypeMap(ListExpr args)
{
  return allUpdatesRTreeTypeMap(args, "insertrtree");
}

/*

7.3.2 ValueMapping of operator ~insertrtree~

*/


template<unsigned dim>
void insertRTree_rect(Word& rtreeWord, Attribute* keyAttr,
                      TupleId& oldTid)
{
  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)rtreeWord.addr;
  BBox<dim> *box = (BBox<dim>*)keyAttr;
  R_TreeLeafEntry<dim, TupleId> e( *box, oldTid );
  rtree->Insert( e );
}

template<unsigned dim>
void insertRTree_spatial(Word& rtreeWord, Attribute* keyAttr,
                         TupleId& oldTid){
  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)rtreeWord.addr;
  BBox<dim> box =
    ((StandardSpatialAttribute<dim>*)keyAttr)->BoundingBox();
  R_TreeLeafEntry<dim, TupleId> e( box, oldTid );
  rtree->Insert( e );
}

int insertRTreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word t, attrPos, dimWord, spatialWord;
  Tuple* tup;
  CcInt* indexp;
  CcInt* dimp;
  CcInt* spatialp;
  int index, dim, spatial;
  Attribute* keyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;
  int* localTransport;
  Supplier suppIndex, suppDim, suppSpatial;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      suppIndex = qp->GetSupplier(args[3].addr,0);
      qp->Request(suppIndex,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      suppDim = qp->GetSupplier(args[3].addr,1);
      qp->Request(suppDim,dimWord);
      dimp = ((CcInt*)dimWord.addr);
      suppSpatial = qp->GetSupplier(args[3].addr,2);
      qp->Request(suppSpatial,spatialWord);
      spatialp = ((CcInt*)spatialWord.addr);
      localTransport = new int[3];
      localTransport[0] = indexp->GetIntval();
      localTransport[1] = dimp->GetIntval();
      localTransport[2] = spatialp->GetIntval();
      local.setAddr(localTransport );
      return 0;

    case REQUEST :
    localTransport = (int*) local.addr;
      index = localTransport[0];
      dim = localTransport[1];
      spatial = localTransport[2];
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        if (spatial)
        {
          switch(dim)
          {
          case 2:
            insertRTree_spatial<2>(args[1],keyAttr,oldTid);
            break;
          case 3:
            insertRTree_spatial<3>(args[1],keyAttr,oldTid);
            break;
          case 4:
            insertRTree_spatial<4>(args[1],keyAttr,oldTid);
            break;
          }
        }
        else
        {
          switch(dim)
          {
          case 2:
            insertRTree_rect<2>(args[1],keyAttr,oldTid);
            break;
          case 3:
            insertRTree_rect<3>(args[1],keyAttr,oldTid);
            break;
          case 4:
            insertRTree_rect<4>(args[1],keyAttr,oldTid);
            break;
          }
        }
        result.setAddr(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        localTransport = (int*) local.addr;
        delete[] localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
7.3.3 Specification of operator ~insertrtree~

*/
const string insertRTreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x@[TID:tid])) x rtree(tuple(x) ti) x xi"
  " -> stream(tuple(x@[TID:tid]))] "
  "</text--->"
  "<text>_ _ insertrtree [_]</text--->"
  "<text>Inserts references to the tuples with TupleId 'TID' "
  "into the rtree.</text--->"
  "<text>query neueStaedte feed staedte insert staedte_Ort "
  " insertrtree [Ort] count "
  "</text--->"
  ") )";

/*
7.3.4 Definition of operator ~insertrtree~

*/
Operator insertrtree (
  "insertrtree",              // name
  insertRTreeSpec,            // specification
  insertRTreeValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  insertRTreeTypeMap          // type mapping
);

/*
7.4 Operator ~deletertree~

For each tuple of the inputstream removes an entry from the rtree. The entry is built from the
spatial-attribute over which the tree is built and the tuple-identifier wich is extracted from the
input-tuples as the last attribute. The inputstream is returned again as the result of this operator.


7.4.1 Type mapping function of operator ~deletertree~


*/

ListExpr deleteRTreeTypeMap( ListExpr args )
{
  return allUpdatesRTreeTypeMap(args, "deletertree");
}

/*
7.4.1 Value mapping function of operator ~deletertree~


*/

template<unsigned dim>
void deleteRTree_rect(Word& rtreeWord, Attribute* keyAttr,
                      TupleId& oldTid)
{
  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)rtreeWord.addr;
  BBox<dim> *box = (BBox<dim>*)keyAttr;
  R_TreeLeafEntry<dim, TupleId> e( *box, oldTid );
  rtree->Remove( e );
}

template<unsigned dim>
void deleteRTree_spatial(Word& rtreeWord, Attribute* keyAttr,
                         TupleId& oldTid)
{
  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)rtreeWord.addr;
  BBox<dim> box =
    ((StandardSpatialAttribute<dim>*)keyAttr)->BoundingBox();
  R_TreeLeafEntry<dim, TupleId> e( box, oldTid );
  rtree->Remove( e );
}

int deleteRTreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word t, attrPos, dimWord, spatialWord;
  Tuple* tup;
  CcInt* indexp;
  CcInt* dimp;
  CcInt* spatialp;
  int index, dim, spatial;
  Attribute* keyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;
  int* localTransport;
  Supplier suppIndex, suppDim, suppSpatial;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      suppIndex = qp->GetSupplier(args[3].addr,0);
      qp->Request(suppIndex,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      suppDim = qp->GetSupplier(args[3].addr,1);
      qp->Request(suppDim,dimWord);
      dimp = ((CcInt*)dimWord.addr);
      suppSpatial = qp->GetSupplier(args[3].addr,2);
      qp->Request(suppSpatial,spatialWord);
      spatialp = ((CcInt*)spatialWord.addr);
      localTransport = new int[3];
      localTransport[0] = indexp->GetIntval();
      localTransport[1] = dimp->GetIntval();
      localTransport[2] = spatialp->GetIntval();
      local.setAddr(localTransport );
      return 0;

    case REQUEST :
    localTransport = (int*) local.addr;
      index = localTransport[0];
      dim = localTransport[1];
      spatial = localTransport[2];
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        if (spatial)
        {
          switch(dim)
          {
          case 2:
            deleteRTree_spatial<2>(args[1],keyAttr,oldTid);
            break;
          case 3:
            deleteRTree_spatial<3>(args[1],keyAttr,oldTid);
            break;
          case 4:
            deleteRTree_spatial<4>(args[1],keyAttr,oldTid);
            break;
          }
        }
        else
        {
          switch(dim)
          {
          case 2:
            deleteRTree_rect<2>(args[1],keyAttr,oldTid);
            break;
          case 3:
            deleteRTree_rect<3>(args[1],keyAttr,oldTid);
            break;
          case 4:
            deleteRTree_rect<4>(args[1],keyAttr,oldTid);
            break;
          }
        }
        result.setAddr(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        localTransport = (int*) local.addr;
        delete[] localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
7.4.3 Specification of operator ~deletertree~

*/
const string deleteRTreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(x@[TID: tid])) x rtree(tuple(x) ti) x xi)"
  " -> stream(tuple(x@[TID: tid]))] "
  "</text--->"
  "<text>_ _ deletertree [_]</text--->"
  "<text>Deletes references to the tuples with TupleId 'TID' "
  "from the rtree.</text--->"
  "<text>query staedte feed filter [.Name = 'Hagen'] "
  " staedte deletedirect staedte_Ort deletertree [Ort] count "
  "</text--->"
  ") )";

/*
7.4.4 Definition of operator ~deletertree~

*/
Operator deletertree (
  "deletertree",              // name
  deleteRTreeSpec,            // specification
  deleteRTreeValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  deleteRTreeTypeMap          // type mapping
);

/*
7.5 Operator ~updatertree~

For each tuple of the inputstream checks if the attribute over which the tree has been built has changed.
If it has changed the old entry for this tuple is removed and a new one is inserted.
The entry is built from the spatial-attribute over which the tree is built and the tuple-identifier
wich is extracted from the input-tuples as the last attribute. The inputstream is returned
again as the result of this operator.


7.5.1 Type mapping function of operator ~updatertree~


*/

ListExpr updateRTreeTypeMap( ListExpr args )
{
  return allUpdatesRTreeTypeMap (args, "updatebtree");
}

/*
7.5.1 Value mapping function of operator ~updatertree~


*/

int updateRTreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word t, attrPos, dimWord, spatialWord;
  Tuple* tup;
  CcInt* indexp;
  CcInt* dimp;
  CcInt* spatialp;
  int index, dim, spatial;
  Attribute* keyAttr;
  Attribute* oldKeyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;
  int* localTransport;
  Supplier suppIndex, suppDim, suppSpatial;


  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      suppIndex = qp->GetSupplier(args[3].addr,0);
      qp->Request(suppIndex,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      suppDim = qp->GetSupplier(args[3].addr,1);
      qp->Request(suppDim,dimWord);
      dimp = ((CcInt*)dimWord.addr);
      suppSpatial = qp->GetSupplier(args[3].addr,2);
      qp->Request(suppSpatial,spatialWord);
      spatialp = ((CcInt*)spatialWord.addr);
      localTransport = new int[3];
      localTransport[0] = indexp->GetIntval();
      localTransport[1] = dimp->GetIntval();
      localTransport[2] = spatialp->GetIntval();
      local.setAddr(localTransport );
      return 0;

    case REQUEST :
    localTransport = (int*) local.addr;
      index = localTransport[0];
      dim = localTransport[1];
      spatial = localTransport[2];
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        keyAttr = tup->GetAttribute(index - 1);
        int noOfAttrs = tup->GetNoAttributes();
        int oldIndex = (noOfAttrs - 1)/2 + index -1;
        oldKeyAttr = tup->GetAttribute(oldIndex);
        tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
        oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
        if (spatial)
        {
          switch(dim)
          {
            case 2:
              if ((((StandardSpatialAttribute<2>*) keyAttr)->
                     BoundingBox()) !=
                  (((StandardSpatialAttribute<2>*) oldKeyAttr)->
                     BoundingBox()))
              {
                deleteRTree_spatial<2>(args[1],oldKeyAttr,oldTid);
                insertRTree_spatial<2>(args[1],keyAttr,oldTid);
              }
              break;
            case 3:
              if ((((StandardSpatialAttribute<3>*) keyAttr)->
                     BoundingBox()) !=
                  (((StandardSpatialAttribute<3>*) oldKeyAttr)->
                     BoundingBox()))
              {
                deleteRTree_spatial<3>(args[1],oldKeyAttr,oldTid);
                insertRTree_spatial<3>(args[1],keyAttr,oldTid);
              }
              break;
            case 4:
              if ((((StandardSpatialAttribute<4>*) keyAttr)->
                     BoundingBox()) !=
                  (((StandardSpatialAttribute<4>*) oldKeyAttr)->
                     BoundingBox()))
              {
                deleteRTree_spatial<4>(args[1],oldKeyAttr,oldTid);
                insertRTree_spatial<4>(args[1],keyAttr,oldTid);
              }
              break;
          }
        }
        else
        {
          switch(dim)
          {
            case 2:
              if (((Rectangle<2>*) keyAttr) !=
                  ((Rectangle<2>*) oldKeyAttr))
              {
                deleteRTree_rect<2>(args[1],oldKeyAttr,oldTid);
                insertRTree_rect<2>(args[1],keyAttr,oldTid);
              }
              break;
            case 3:
              if (((Rectangle<3>*) keyAttr) !=
                  ((Rectangle<3>*) oldKeyAttr))
              {
                deleteRTree_rect<3>(args[1],oldKeyAttr,oldTid);
                insertRTree_rect<3>(args[1],keyAttr,oldTid);
              }
              break;
            case 4:
              if (((Rectangle<4>*) keyAttr) !=
                  ((Rectangle<4>*) oldKeyAttr))
              {
                deleteRTree_rect<4>(args[1],oldKeyAttr,oldTid);
                insertRTree_rect<4>(args[1],keyAttr,oldTid);
              }
              break;
          }
        }
        result.setAddr(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
      qp->Close(args[0].addr);
      qp->SetModified(qp->GetSon(s, 1));
      if(local.addr)
      {
        localTransport = (int*) local.addr;
        delete[] localTransport;
        local.setAddr(0);
      }
      return 0;
  }
  return 0;
}

/*
7.5.3 Specification of operator ~updatertree~

*/
const string updateRTreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>stream(tuple(x@[(a1_old x1) ... (an_old xn) (TID: tid)])) "
  "x rtree(tuple(x) ti) x xi"
  " -> stream(tuple(x@[(a1_old x1) ... (an_old xn) (TID: tid)]))] "
  "</text--->"
  "<text>_ _ updatertree [_]</text--->"
  "<text>Updates references to the tuples with TupleId 'TID'"
  "in the rtree.</text--->"
  "<text>query staedte feed filter [.Name = 'Hagen'] "
  " staedte updatedirect [Ort : newRegion] staedte_Ort "
  "updatertree [Ort] count "
  "</text--->"
  ") )";

/*
7.5.4 Definition of operator ~updatertree~

*/
Operator updatertree (
  "updatertree",              // name
  updateRTreeSpec,            // specification
  updateRTreeValueMap,                // value mapping
  Operator::SimpleSelect,          // trivial selection function
  updateRTreeTypeMap          // type mapping
);

/*

3 Class ~UpdateRelationAlgebra~

A new subclass ~UpdateRelationAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~ is defined.

*/

class UpdateRelationAlgebra : public Algebra
{
 public:
  UpdateRelationAlgebra() : Algebra()
  {
    AddOperator(&extrelcreateinsertrel);
    AddOperator(&extrelcreatedeleterel);
    AddOperator(&extrelcreateupdaterel);
    AddOperator(&extrelinsert);
    AddOperator(&extrelinsertsave);
    AddOperator(&extrelinserttuple);
    AddOperator(&extrelinserttuplesave);
    AddOperator(&extreldeletesearch);
    AddOperator(&extreldeletedirect);
    AddOperator(&extreldeletesearchsave);
    AddOperator(&extreldeletedirectsave);
    AddOperator(&extrelupdatesearch);
    AddOperator(&extrelupdatedirect);
    AddOperator(&extrelupdatesearchsave);
    AddOperator(&extrelupdatedirectsave);
    AddOperator(&extreladdid);
    AddOperator(&extreldeletebyid);
    AddOperator(&extrelupdatebyid);

    AddOperator( &insertrtree );
    AddOperator( &deletertree );
    AddOperator( &updatertree );
  }
  ~UpdateRelationAlgebra() {};
};

/*

4 Initialization

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
InitializeUpdateRelationAlgebra( NestedList* nlRef,
                                 QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new UpdateRelationAlgebra());
}

