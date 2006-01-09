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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Update Relation Algebra

June 2005 Matthias Zielke

January 2006, Victor Almeida separated this algebra from the Extended
Relational Algebra and fixed some memory leaks. 

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

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operators

2.19 Operators ~createinsertrel~, ~createdeleterel~ and ~createupdaterel~

Create an empty relation with the schema of the result-tupletyp of the corresponding 'update'-operators.
This operator is used to create an auxiliary relation for updates on relations which shall be passed to
existing indices later on.


2.19.1 General Type mapping function of operators ~createinsertrel~, ~createdeleterel~ and ~createupdaterel~



Type mapping ~createinsertrel~, ~createdeleterel~, and ~createupdaterel~ 

----     (rel X)

        -> (rel(tuple((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~createupdaterel~

----     (rel X)

        -> (rel(tuple((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----


*/

ListExpr createAuxiliaryRelTypeMap( const ListExpr& args, const string opName )
{
  ListExpr first,rest,listn,lastlistn,oldAttribute, outList;
  string oldName;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator " + opName +" expects a list of length one.");
  

  first = nl->First(args);

  CHECK_COND(!(nl->IsAtom(first)) &&
             (nl->ListLength(first) > 0),
   "Operator " + opName + ": First argument  may not be empty or an atom" );
   
   // Argument has to be a relation 
   nl->WriteToString(argstr, first);
   CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       		 (nl->ListLength(nl->Second(first)) == 2) &&
       		 (IsTupleDescription(nl->Second(nl->Second(first)))),
    		"Operator " + opName + " expects as first argument a list with structure "
    		"(rel(tuple ((a1 t1)...(an tn))))\n"
    		"Operator " + opName + " gets as first argument '" + argstr + "'." );

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
  if ( opName == "createupdaterel"){
  	//Append once again all attributes from the argument-relation to the result-tupletype but the 
  	//names of the attributes extendend by '_old'
  	rest = nl->Second(nl->Second(first));
  	 while (!(nl->IsEmpty(rest)))
  	{
     nl->WriteToString(oldName, nl->First(nl->First(rest)));
  	 oldName += "_old";
  	 oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),nl->Second(nl->First(rest)));
     lastlistn = nl->Append(lastlistn,oldAttribute);
     rest = nl->Rest(rest);
  	} 	
  }
  //Append last attribute for the tupleidentifier
  lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  outList = nl->TwoElemList(nl->SymbolAtom("rel"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  return outList;
}

ListExpr createInsertRelTypeMap( ListExpr args){
	return createAuxiliaryRelTypeMap( args, "createinsertrel");
}

ListExpr createDeleteRelTypeMap( ListExpr args){
	return createAuxiliaryRelTypeMap( args, "createdeleterel");
}

ListExpr createUpdateRelTypeMap( ListExpr args){
	return createAuxiliaryRelTypeMap( args, "createupdaterel");
}

/*
2.19.2 General Value mapping function of operators ~createinsertrel~, ~createdeleterel~ and ~createupdaterel~

*/
int createUpdateRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  	Relation* resultRelation;  
  	TupleType *resultTupleType;
    ListExpr resultType = SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    ListExpr tupleType = nl->Second(resultType); 
    resultTupleType = new TupleType( tupleType );
    resultRelation = new Relation(*resultTupleType);  
    result = SetWord(resultRelation);
    return 0;
}

/*
2.19.3 Specification of operator ~createinsertrel~

*/
const string createinsertrelSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "createinsertrel",              // name
         createinsertrelSpec,            // specification
         createUpdateRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         createInsertRelTypeMap         // type mapping
);


/*
2.20.3 Specification of operator ~createdeleterel~

*/
const string createdeleterelSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "createdeleterel",              // name
         createdeleterelSpec,            // specification
         createUpdateRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         createDeleteRelTypeMap         // type mapping
);


/*
2.21.3 Specification of operator ~createupdaterel~

*/
const string createupdaterelSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "createupdaterel",              // name
         createupdaterelSpec,            // specification
         createUpdateRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
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
  
   	CHECK_COND(nl->ListLength(args) == 2,
    	"Operator " + opName +" expects a list of length two.");
  

  	first = nl->First(args);
  	second  = nl->Second(args);

  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator " + opName +" expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName +" gets as first argument '" + argstr + "'." );

  	CHECK_COND(!(nl->IsAtom(second)) &&
             	(nl->ListLength(second) > 0),
    			"Operator " + opName +": Second argument list may not be empty or an atom" );
    
   	nl->WriteToString(argstr2, second);
   	CHECK_COND(nl->ListLength(second) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(second)) == rel) &&
             	(nl->ListLength(nl->Second(second)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       			(nl->ListLength(nl->Second(second)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(second)))),
    			"Operator " + opName +" expects as second argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName +" gets as second argument '" + argstr2 + "'." );

    nl->WriteToString(argstr, nl->Second(first));
    nl->WriteToString(argstr2, nl->Second(second));
    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second))),
      			"Operator " + opName +": Tuple type in the argumentstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the relation" );
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
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
            	nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}

/*
2.22.1 Type mapping function of operator ~insert~

*/

ListExpr insertRelTypeMap(ListExpr args){
	return insertDeleteRelTypeMap(args, "insert");	
}

/*
2.22.2 Value mapping function of operator ~insert~

*/
int insertRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRelation;
  Tuple* tup;
  Relation* relation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );  
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      qp->Request(args[1].addr, argRelation);
      relation = (Relation*)(argRelation.addr);    
      assert(relation != 0);
      qp->Request(args[0].addr,t);      
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( *resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->CopyAttribute( i, tup, i );
        relation->AppendTuple(tup);
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr );
        result = SetWord(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	resultTupleType = (TupleType*) local.addr;
    	delete resultTupleType;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}

/*
2.22.3 Specification of operator ~insert~

*/
const string insertSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         "insert",              // name
         insertSpec,            // specification
         insertRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
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
  
   	CHECK_COND(nl->ListLength(args) == 3,
    			"Operator insertsave expects a list of length three.");
  
  	first = nl->First(args);
  	second  = nl->Second(args);
  	third  = nl->Third(args);
  

  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator insertsave expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn))))\n"
    			"Operator insertsave gets as first argument '" + argstr + "'." );

  	CHECK_COND(!(nl->IsAtom(second)) &&
             	(nl->ListLength(second) > 0),
    			"Operator insertsave: Second argument list may not be empty or an atom" );
    
   	nl->WriteToString(argstr2, second);
   	CHECK_COND(nl->ListLength(second) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(second)) == rel) &&
             	(nl->ListLength(nl->Second(second)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       			(nl->ListLength(nl->Second(second)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(second)))),
    			"Operator insertsave expects as second argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator insertsave gets as second argument '" + argstr2 + "'." );
    
    nl->WriteToString(argstr2, third);
   	CHECK_COND(nl->ListLength(third) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(third)) == rel) &&
             	(nl->ListLength(nl->Second(third)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(third))) == tuple) &&
       			(nl->ListLength(nl->Second(third)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(third)))),
    			"Operator insertsave expects as third argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn)(TID tid)))\n"
    			"Operator insertsave gets as third argument '" + argstr2 + "'." );

  
    nl->WriteToString(argstr, nl->Second(first));
    nl->WriteToString(argstr2, nl->Second(second));
    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second))),
      			"Operator insertsave: Tuple type in the argumentstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the relation" );
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
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	nl->WriteToString(argstr, listn);
  	nl->WriteToString(argstr2, nl->Second(nl->Second(third)));
    CHECK_COND( (nl->Equal(listn,nl->Second(nl->Second(third)))),
      			"Operator insertsave: Tuple type of the resultstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the auxiliary-relation" );
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
           		 nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}

/*
2.23.2 Value mapping function of operator ~insertsave~

*/
int insertSaveRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRelation,argAuxRelation;
  Tuple* tup;
  Relation* relation;
  Relation* auxRelation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );  
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      qp->Request(args[1].addr, argRelation);
      relation = (Relation*)(argRelation.addr);    
      assert(relation != 0);
      qp->Request(args[2].addr, argAuxRelation);
      auxRelation = (Relation*)(argAuxRelation.addr);    
      assert(auxRelation != 0);
      qp->Request(args[0].addr,t);      
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        relation->AppendTuple(tup);	
        Tuple *newTuple = new Tuple( *resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr );
        auxRelation->AppendTuple(newTuple);
        result = SetWord(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	resultTupleType = (TupleType*) local.addr;
    	delete resultTupleType;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
    	qp->SetModified(args[2].addr);
      return 0;
  }
  return 0;
}

/*
2.23.3 Specification of operator ~insertsave~

*/
const string insertSaveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x@[TID:tid]))"
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
         insertSaveRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         insertSaveRelTypeMap         // type mapping
);

/*
2.24 Operator ~deletesearch~

Deletes each tuple from the relation which attribut-values are exactly the same as those ones of 
a tuple of the inputstream. Returns a stream of tuples which is  basically 
the stream of deleted tuples but each tuple extended by an attribute of type 'tid' which is the tupleidentifier
of the deleted tuple in the extended relation.

*/


/*

2.24.1 TypeMapping for operator ~deletesearch~

*/

ListExpr deleteSearchRelTypeMap( ListExpr args ){
	return insertDeleteRelTypeMap(args, "deletesearch");	
}



/*
2.24.2 Value mapping function of operator ~deletesearch~

*/
int deleteSearchRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRelation;
  Tuple* tup;
  Tuple* nextTup;
  Tuple* newTuple;
  Relation* relation;
  RelationIterator* iter;
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
        (*i)->DecReference();
        (*i)->DeleteIfAllowed();
      }
      delete deletedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DecReference();
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      delete resultTupleType;
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
      qp->Request(args[1].addr, argRelation);
      relation = (Relation*)(argRelation.addr); 
      assert(relation != 0);
      iter = new RelationIterator(*relation); 
      nextTup = iter->GetNextTuple(); 
      // fill hashtable   
      while (!iter->EndOfScan())
      {
      	hashValue = 0;
      	for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
           hashValue += ((StandardAttribute*)(nextTup->GetAttribute(i)))->HashValue();		
        nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        nextTup->IncReference();
      	nextBucket->push_back(nextTup);
      	nextTup = iter->GetNextTuple(); 
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( localTransport );
      return 0;

    case REQUEST :
      localTransport =  (LocalTransport*) local.addr; 
      // deletedTuples can contain duplicates that have not been given to the resultstream yet
      if (!localTransport->deletedTuples->empty())
      {
      	newTuple = localTransport->deletedTuples->back();
        newTuple->DecReference();
      	localTransport->deletedTuples->pop_back();
        result = SetWord(newTuple);
        return YIELD;
      }
      // no duplicate has to be send to the resultstream
      else
      {
      	qp->Request(args[1].addr, argRelation);
      	relation = (Relation*)(argRelation.addr);    
      	assert(relation != 0);
      	tupleFound = false;
      	// tupleFound will stay false until a matching tuple to a inputtuple was found
      	while (! tupleFound) 
        {
      		qp->Request(args[0].addr,t);      
      		if (qp->Received(args[0].addr))
      		{
        		tup = (Tuple*)t.addr;
        		hashValue = 0;
      			for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			hashValue += ((StandardAttribute*)(tup->GetAttribute(i)))->HashValue();		
        		SortOrderSpecification full;
        		for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			full.push_back(pair<int,bool> (i+1,false));
        		TupleCompareBy compare(full);
        		// get correct bucket from hashtable
        		nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        		// look for all matching tuples in the bucket
        		for (size_t j = 0; j < nextBucket->size(); j++)
            {
        			nextTup = nextBucket->operator[](j);
        			if (nextTup != 0)
              {
        				if(!compare(nextTup,tup) && !compare(tup,nextTup))
                {
        					newTuple = new Tuple( *localTransport->resultTupleType );
        					assert( newTuple->GetNoAttributes() == nextTup->GetNoAttributes() + 1 );
        					for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
          	        newTuple->PutAttribute( i, (nextTup->GetAttribute(i))->Clone() );
        					const TupleId& tid = nextTup->GetTupleId();
        					StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        					newTuple->PutAttribute( nextTup->GetNoAttributes(), tidAttr );
        					relation->DeleteTuple(nextTup);
                  newTuple->IncReference();
        					localTransport->deletedTuples->push_back(newTuple);
        				}
        			}
        		}
        		if (!localTransport->deletedTuples->empty())
            {
        			newTuple = localTransport->deletedTuples->back();
              newTuple->DecReference();
        			localTransport->deletedTuples->pop_back();
        			result = SetWord(newTuple);
        			tupleFound = true;
        		}
        		tup->DeleteIfAllowed();
      		}
      		else// if (qp->Received(args[0].addr))
        		return CANCEL;
      	}// while (! tupleFound);
        return YIELD;
    }
      
    case CLOSE :
  		localTransport = (LocalTransport*) local.addr;
    	delete localTransport;
    	qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}


/*
2.24.3 Specification of operator ~deletesearch~

*/
const string deleteSearchSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         deleteSearchRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         deleteSearchRelTypeMap         // type mapping
);

/*
2.25 Operator ~deletedirect~

Deletes each tuple from the inputstream directly from the relation. Precondition is that the tuples
of the inputstream are tuples from the relation that shall be updated. Returns a stream of tuples 
which is  basically the stream of deleted tuples but each tuple extended by an attribute of type 
'tid' which is the tupleidentifier of the deleted tuple in the updated relation.

2.25.1 TypeMapping for operator ~deletedirect~

*/

ListExpr deleteDirectRelTypeMap( ListExpr args ){
	return insertDeleteRelTypeMap(args, "deletedirect");	
}


/*
2.25.2 Value mapping function of operator ~deletedirect~

*/
int deleteDirectRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRelation;
  Tuple* tup;
  Relation* relation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );  
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      qp->Request(args[1].addr, argRelation);
      relation = (Relation*)(argRelation.addr);    
      assert(relation != 0);
      qp->Request(args[0].addr,t);      
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
      	Tuple *newTuple = new Tuple( *resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
        	newTuple->PutAttribute( i, (tup->GetAttribute(i))->Clone() );
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr );
        relation->DeleteTuple(tup);
        tup->DeleteIfAllowed();
        result = SetWord(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	resultTupleType = (TupleType*) local.addr;
    	delete resultTupleType;
    	qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}


/*
2.25.3 Specification of operator ~deletedirect~

*/
const string deleteDirectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
         deleteDirectRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         deleteDirectRelTypeMap         // type mapping
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
  
   CHECK_COND(nl->ListLength(args) == 3,
    			"Operator " + opName + " expects a list of length three.");
  
	// Check inputstream
  	first = nl->First(args);
  	second  = nl->Second(args);
  	third  = nl->Third(args);
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator " + opName + " expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as first argument '" + argstr + "'." );
	// Check updaterelation
  	CHECK_COND(!(nl->IsAtom(second)) &&
             	(nl->ListLength(second) > 0),
    			"Operator " + opName + ": Second argument list may not be empty or an atom" );
    
   	nl->WriteToString(argstr2, second);
   	CHECK_COND(nl->ListLength(second) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(second)) == rel) &&
             	(nl->ListLength(nl->Second(second)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       			(nl->ListLength(nl->Second(second)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(second)))),
    			"Operator " + opName + " expects as second argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as second argument '" + argstr2 + "'." );
    // Check auxiliary relation
    nl->WriteToString(argstr2, third);
   	CHECK_COND(nl->ListLength(third) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(third)) == rel) &&
             	(nl->ListLength(nl->Second(third)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(third))) == tuple) &&
       			(nl->ListLength(nl->Second(third)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(third)))),
    			"Operator " + opName + " expects as third argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as third argument '" + argstr2 + "'." );
    

  	// Compare tupletype of the inputstream with tupletype of the updaterelation
    nl->WriteToString(argstr, nl->Second(first));
    nl->WriteToString(argstr2, nl->Second(second));
    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second))),
      			"Operator " + opName + ": Tuple type in the argumentstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the relation" );
    // build resultlist
  	rest = nl->Second(nl->Second(second));
  	listn = nl->OneElemList(nl->First(rest));
  	lastlistn = listn;
  	rest = nl->Rest(rest);
  	while (!(nl->IsEmpty(rest)))
  	{
     	lastlistn = nl->Append(lastlistn,nl->First(rest));
     	rest = nl->Rest(rest);
  	}
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	// Compare tupletype of resulttuples with tupletype of auxiliary relation
  	nl->WriteToString(argstr, listn);
  	nl->WriteToString(argstr2, nl->Second(nl->Second(third)));
    CHECK_COND( (nl->Equal(listn,nl->Second(nl->Second(third)))),
      			"Operator " + opName + ": Tuple type of the resultstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the auxiliary-relation" );
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
            	nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}

/*

2.26.1 Type Mapping for operator ~deletesearchsave~

*/

ListExpr deleteSearchSaveRelTypeMap(ListExpr args){
	return deleteSaveRelTypeMap(args, "deletesearchsave");
}

/*
2.26.2 Value mapping function of operator ~deletesearchsave~

*/
int deleteSearchSaveRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRelation,argAuxRelation;
  Tuple* tup;
  Tuple* nextTup;
  Tuple* newTuple;
  Relation* relation;
  Relation* auxRelation;
  RelationIterator* iter;
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
        (*i)->DecReference();
        (*i)->DeleteIfAllowed();
      }
      delete deletedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DecReference();
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      delete resultTupleType;
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
      qp->Request(args[1].addr, argRelation);
      relation = (Relation*)(argRelation.addr); 
      assert(relation != 0);
      iter = new RelationIterator(*relation); 
      nextTup = iter->GetNextTuple();    
      while (!iter->EndOfScan())
      {
      	hashValue = 0;
      	for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
           hashValue += ((StandardAttribute*)(nextTup->GetAttribute(i)))->HashValue();		
        nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        nextTup->IncReference();
      	nextBucket->push_back(nextTup);
      	nextTup = iter->GetNextTuple(); 
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( localTransport );
      return 0;

    case REQUEST :
      // Check if already deleted duplicates have to be given to the outputstream 
      if (!localTransport->deletedTuples->empty())
      {
      	newTuple = localTransport->deletedTuples->back();
      	localTransport->deletedTuples->pop_back();
        result = SetWord(newTuple);
        return YIELD;
      }
      // No more duplicate left over to send to the outputstream
      else{
      	qp->Request(args[1].addr, argRelation);
      	relation = (Relation*)(argRelation.addr);    
      	assert(relation != 0);
      	qp->Request(args[2].addr, argAuxRelation);
      	auxRelation = (Relation*)(argAuxRelation.addr);    
      	assert(auxRelation != 0);
      	tupleFound = false;
      	// tupleFound will stay false until a tuple with the same values as one of the inputtuples 
      	// is found
      	while (! tupleFound){
      		qp->Request(args[0].addr,t);      
      		if (qp->Received(args[0].addr))
      		{
        		tup = (Tuple*)t.addr;
        		hashValue = 0;
      			for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			hashValue += ((StandardAttribute*)(tup->GetAttribute(i)))->HashValue();		
        		SortOrderSpecification full;
        		for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			full.push_back(pair<int,bool> (i+1,false));
        		TupleCompareBy compare(full);
        		// Get the right hash-bucket
        		nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        		// Check all tuples in the bucket
        		for (size_t j = 0; j < nextBucket->size(); j++)
            {
        			nextTup = nextBucket->operator[](j);
        			if (nextTup != 0)
              {
        				if(!compare(nextTup,tup) && !compare(tup,nextTup))
                {
        					newTuple = new Tuple( *localTransport->resultTupleType );
        					assert( newTuple->GetNoAttributes() == nextTup->GetNoAttributes() +1 );
        					for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
          						newTuple->PutAttribute( i, (nextTup->GetAttribute(i))->Clone() );
        					const TupleId& tid = nextTup->GetTupleId();
        					StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        					newTuple->PutAttribute( nextTup->GetNoAttributes(), tidAttr);
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
              newTuple->DecReference();
        			localTransport->deletedTuples->pop_back();
        			result = SetWord(newTuple);
        			tupleFound = true;
        		}
        		tup->DeleteIfAllowed();
      		}
      		else// if (qp->Received(args[0].addr))
        		return CANCEL;
      	}// while (! tupleFound);
        return YIELD;
    }
      
    case CLOSE :
  		localTransport = (LocalTransport*) local.addr;
    	delete localTransport;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
    	qp->SetModified(args[2].addr);
      return 0;
  }
  return 0;
}


/*
2.26.3 Specification of operator ~deletesearchsave~

*/
const string deleteSearchSaveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x@[TID:tid]))"
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
         deleteSearchSaveRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         deleteSearchSaveRelTypeMap         // type mapping
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

ListExpr deleteDirectSaveRelTypeMap(ListExpr args){
	return deleteSaveRelTypeMap(args, "deletedirectsave");
}

/*
2.27.2 Value mapping function of operator ~deletedirectsave~

*/

int deleteDirectSaveRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRelation,argAuxRelation;
  Tuple* tup;
  Relation* relation;
  Relation* auxRelation;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );  
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      qp->Request(args[1].addr, argRelation);
      relation = (Relation*)(argRelation.addr);    
      assert(relation != 0);
      qp->Request(args[2].addr, argAuxRelation);
      auxRelation = (Relation*)(argAuxRelation.addr);    
      assert(auxRelation != 0);
      qp->Request(args[0].addr,t);      
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( *resultTupleType );
        assert( newTuple->GetNoAttributes() == tup->GetNoAttributes() +1 );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->PutAttribute( i, (tup->GetAttribute(i))->Clone() );
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr);
	      relation->DeleteTuple(tup);
        auxRelation->AppendTuple(newTuple);
        result = SetWord(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	resultTupleType = (TupleType*) local.addr;
    	delete resultTupleType;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
    	qp->SetModified(args[2].addr);
      return 0;
  }
  return 0;
}



/*
2.27.3 Specification of operator ~deletedirectsave~

*/
const string deleteDirectSaveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x)) x rel(tuple(x)) x rel(tuple(x@[TID:tid]))"
                           " -> stream(tuple(x@[TID:tid]))] "
                           "</text--->"
                           "<text>_ _ _deletedirectsave</text--->"
                           "<text>Like 'deletedirect' but appends all result-tuples "
                           "to the auxiliary relation.</text--->"
                           "<text>query staedte feed filter [.Bev > 200000] staedte staedteD deletedirectsave "
                           "consume"
                           "</text--->"
                           ") )";

/*
2.27.4 Definition of operator ~deletedirectsave~ 

*/
Operator extreldeletedirectsave (
         "deletedirectsave",              // name
         deleteDirectSaveSpec,            // specification
         deleteDirectSaveRelValueMap,                // value mapping
         Operator::SimpleSelect,          // trivial selection function
         deleteDirectSaveRelTypeMap         // type mapping
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
  
  
  	CHECK_COND(nl->ListLength(args) == 2,
    			"Operator inserttuple expects a list of length two.");
  
  	first = nl->First(args);
  	second = nl->Second(args);
    // Check relation
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator inserttuple expects as first argument a list with structure "
    			"(rel (tuple ((a1 t1)...(an tn))))\n"
    			"Operator inserttuple gets as first argument '" + argstr + "'." );
    // Check second argument 
  	nl->WriteToString(argstr, second);    
  	CHECK_COND(nl->ListLength(second) > 0 &&
             	!nl->IsAtom(second),
    			"Operator inserttuple expects a list of attributetypes " 
    			"(t1...tn)\n"
    			"Operator inserttuple gets a list '" + argstr + "'.");
    // Check if there are as many values in the second argument as attributes in the 
    //tuples of the relation
   	CHECK_COND(nl->ListLength(second) == nl->ListLength(nl->Second(nl->Second(first))),
    	"Operator inserttuple expects the same nuber of attributetypes " 
    	"than attributes in the tupletype of the relation");
 	// Check if types of the new tuplevalues and types of the tuples of the relation
 	// at the same position are the same
  	rest = nl->Second(nl->Second(first));
 	rest2 = second;
 	 while (!(nl->IsEmpty(rest)))
  	{
     	CHECK_COND(nl->Equal(nl->Second(nl->First(rest)),nl->First(rest2)),
    				"Operator inserttuple: types of attributevalues at ech position of the new"
    				" tuple have to be the same as the types at the same position in the tuples"
    				"of the relation");
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
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
           						 nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}

/*
2.28.2 Value mapping function of operator ~inserttuple~

*/
int insertTupleRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word argRelation, attrValue;
  Tuple* insertTuple;
  Tuple* resultTuple;
  Relation* relation;
  TupleType *resultTupleType;
  ListExpr resultType;
  bool* firstcall;
  Supplier supplier, supplier1;
  Attribute* attr;

  switch (message)
  {
    case OPEN :
      firstcall = new bool;
      *firstcall = true;
      local = SetWord( firstcall );
      return 0;

    case REQUEST :
  	  firstcall = (bool*) local.addr;   
      if (*firstcall)
      {
      	*firstcall = false;
      	resultType = GetTupleResultType( s );
      	resultTupleType = new TupleType( nl->Second( resultType ) );
      	qp->Request(args[0].addr, argRelation);
      	relation = (Relation*)(argRelation.addr);    
      	assert(relation != 0);  
        resultTuple = new Tuple( *resultTupleType );
        insertTuple = new Tuple( relation->GetTupleType());
        supplier = args[1].addr;
        for( int i = 0; i < (resultTuple->GetNoAttributes() -1); i++ )
        {
          supplier1 = qp->GetSupplier(supplier, i);
          qp->Request(supplier1,attrValue);
          attr = (Attribute*) attrValue.addr;
          resultTuple->PutAttribute(i,attr->Clone());
          insertTuple->CopyAttribute(i,resultTuple,i);
        }
        relation->AppendTuple(insertTuple);
        const TupleId& tid = insertTuple->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        resultTuple->PutAttribute( resultTuple->GetNoAttributes() -1, tidAttr);
        result = SetWord(resultTuple);
        insertTuple->DeleteIfAllowed();
        delete resultTupleType;
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	firstcall = (bool*) local.addr;
    	delete firstcall;
    	qp->SetModified(args[0].addr);
      return 0;
  }
  return 0;
}

/*
2.28.3 Specification of operator ~inserttuple~

*/
const string insertTupleSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
  
  	ListExpr first,second, third, rest, rest2, listn, lastlistn, outList;
  	string argstr="",valueString, argstr2;
  
  
  	CHECK_COND(nl->ListLength(args) == 3,
    			"Operator inserttuplesave expects a list of length three.");
  
  	first = nl->First(args);
  	second = nl->Second(args);
  	third= nl->Third(args);
    // Check updaterelation
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator inserttuplesave expects as first argument a list with structure "
    			"(rel (tuple ((a1 t1)...(an tn))))\n"
    			"Operator inserttuplesave gets as first argument '" + argstr + "'." );
    // Check value-list
  	nl->WriteToString(argstr, third);    
  	CHECK_COND(nl->ListLength(third) > 0 &&
             	!nl->IsAtom(third),
    			"Operator inserttuplesave expects a list of attributetypes " 
    			"(t1...tn)\n"
    			"Operator inserttuplesave gets a list '" + argstr + "'.");
    // Check if there are as many new values as attributes in the tuples of the updaterelation
   	CHECK_COND(nl->ListLength(third) == nl->ListLength(nl->Second(nl->Second(first))),
    			"Operator inserttuplesave expects the same nuber of attributetypes " 
    			"than attributes in the tupletype of the relation");
    // Check auxiliary relation
    nl->WriteToString(argstr2, second);
   	CHECK_COND(nl->ListLength(second) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(second)) == rel) &&
             	(nl->ListLength(nl->Second(second)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       			(nl->ListLength(nl->Second(second)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(second)))),
    			"Operator inserttuplesave expects as second argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator inserttuplesave gets as second argument '" + argstr2 + "'." );

 	// Check if types of new values and types of the tuples of the updatrelation at the same
 	// positions are the same
  	rest = nl->Second(nl->Second(first));
  	rest2 = third;
 	 while (!(nl->IsEmpty(rest)))
  	{
     	CHECK_COND(nl->Equal(nl->Second(nl->First(rest)),nl->First(rest2)),
    				"Operator inserttuplesave: types of attributevalues at ech position of the new"
    				" tuple have to be the same as the types at the same position in the tuples"
    				"of the relation");
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
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	// Check if result-tupletype and type of the auxiliary relation are the same
   	nl->WriteToString(argstr, listn);
  	nl->WriteToString(argstr2, nl->Second(nl->Second(second)));
    CHECK_COND( (nl->Equal(listn,nl->Second(nl->Second(second)))),
      			"Operator inserttuplesave: Tuple type of the resultstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the auxiliary-relation" );
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
            					nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}

/*
2.29.2 Value mapping function of operator ~inserttuplesave~

*/
int insertTupleSaveRelValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word argRelation,argAuxRelation, attrValue;
  Tuple* insertTuple;
  Tuple* resultTuple;
  Relation* relation;
  Relation* auxRelation;
  TupleType *resultTupleType;
  ListExpr resultType;
  bool* firstcall;
  Supplier supplier, supplier1;
  Attribute* attr;

  switch (message)
  {
    case OPEN :
      firstcall = new bool;
      *firstcall = true;
      local = SetWord( firstcall );
      return 0;

    case REQUEST :
	  firstcall = (bool*) local.addr;   
      if (*firstcall)
      {
      	*firstcall = false;
      	resultType = GetTupleResultType( s );
      	resultTupleType = new TupleType( nl->Second( resultType ) );
      	qp->Request(args[0].addr, argRelation);
      	relation = (Relation*)(argRelation.addr);    
      	assert(relation != 0); 
      	qp->Request(args[1].addr, argAuxRelation);
      	auxRelation = (Relation*)(argAuxRelation.addr);    
      	assert(auxRelation != 0); 
        resultTuple = new Tuple( *resultTupleType );
        insertTuple = new Tuple( relation->GetTupleType());
        supplier = args[2].addr;
        for( int i = 0; i < (resultTuple->GetNoAttributes() -1); i++ )
        {
          supplier1 = qp->GetSupplier(supplier, i);
          qp->Request(supplier1,attrValue);
          attr = (Attribute*) attrValue.addr;
          resultTuple->PutAttribute(i,attr->Clone());
          insertTuple->CopyAttribute(i,resultTuple,i);
        }
        relation->AppendTuple(insertTuple);
        const TupleId& tid = insertTuple->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        resultTuple->PutAttribute( resultTuple->GetNoAttributes() -1, tidAttr);
        auxRelation->AppendTuple(resultTuple);
        result = SetWord(resultTuple);
        insertTuple->DeleteIfAllowed();
        delete resultTupleType;
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	firstcall = (bool*) local.addr;
    	delete firstcall;
    	qp->SetModified(args[0].addr);
    	qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}

/*
2.29.3 Specification of operator ~inserttuplesave~

*/
const string insertTupleSaveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>rel(tuple(x)) x rel(tuple(x@[TID:tid])) x [t1 ... tn]"
                           " -> stream(tuple(x@[TID:tid]))] "
                           "</text--->"
                           "<text> _ _ inserttuplesave [list]</text--->"
                           "<text>Like `inserttuple` but appends the result-tuple "
                           "to the auxiliary relation </text--->"
                           "<text>query staedte staedteI inserttuplesave['Kassel', 200000, 50000] "
                           "consume"
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
  	ListExpr first, second,third,errorInfo, rest, listn, lastlistn, first2, second2, firstr,attrType,
  				numberList, lastNumberList,oldAttribute, outlist;
  	int attrIndex, noAttrs ;
  	bool firstcall = true;
  	string argstr, argstr2, argstr3, oldName;
  	AlgebraManager* algMgr;
	algMgr = SecondoSystem::GetAlgebraManager();
	errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  	CHECK_COND(nl->ListLength(args) == 3,
    			"Operator " + opName + " expects a list of length three.");

  	first = nl->First(args);
  	second  = nl->Second(args);
  	third = nl->Third(args);
	// Check inputstream
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator " + opName + " expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as first argument '" + argstr + "'." );
    // Check update-relation
  	CHECK_COND(!(nl->IsAtom(second)) &&
             	(nl->ListLength(second) > 0),
    			"Operator " + opName + ": Second argument list may not be empty or an atom" );
    
   	nl->WriteToString(argstr2, second);
   	CHECK_COND(nl->ListLength(second) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(second)) == rel) &&
             	(nl->ListLength(nl->Second(second)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       			(nl->ListLength(nl->Second(second)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(second)))),
    			"Operator " + opName + " expects as second argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as second argument '" + argstr2 + "'." );

  	// Check if tuples of the inputstream and of the relation have the same schema
    nl->WriteToString(argstr, nl->Second(first));
    nl->WriteToString(argstr2, nl->Second(second));
    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second))),
      				"Operator " + opName + ": Tuple type in the argumentstream '" + argstr +
      				"' is different from the tuple type '" + argstr2 +
      				"' in the relation" );
	// Check function-argumentlist
  	CHECK_COND(!(nl->IsAtom(third)) &&
            		(nl->ListLength(third) > 0),
    				"Operator " + opName + ": Third argument list may not be empty or an atom" );

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
    	CHECK_COND( (nl->ListLength(second2) == 3) &&
                		(TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
          				(algMgr->CheckKind("DATA", nl->Third(second2), errorInfo)),
      					"Operator " + opName + " expects a mapping function with list structure"
      					" (<attrname> (map (tuple ( (a1 t1)...(an tn) )) ti) )\n. Operator"
      					" " + opName + " gets a list '" + argstr + "'.\n" );
		// Check if functions have argumenttuples of the same type as those ones from the inputstream
    	nl->WriteToString(argstr, nl->Second(first));
    	nl->WriteToString(argstr2, second2);
    	CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second2))),
      					"Operator " + opName + ": Tuple type in first argument '" + argstr +
      					"' is different from the argument tuple type '" + argstr2 +
      					"' in the mapping function" );
      	// Is the name of the attribute that shall be computed by the function a valid name
    	nl->WriteToString(argstr, first2);
    	CHECK_COND( (nl->IsAtom(first2)) &&
                		(nl->AtomType(first2) == SymbolType),
      					"Operator " + opName + ": Attribute name '" + argstr +
      					"' is not an atom or not of type SymbolType" );
      	// Is the name of the attribute of the function an attributename of the tuples of the inputstream
    	attrIndex = FindAttribute(nl->Second(nl->Second(first)), argstr, attrType);
    	CHECK_COND( attrIndex != 0 ,
      					"Operator " + opName + ": Attribute name '" + argstr + "' of the mapping-functions"
      					" is not an attributename of the tuples of the relation" );
      	// Is the type of the attribute of the function the same as the type of the attribute of the tuples of the inputstream
     	nl->WriteToString(argstr2, attrType);
     	nl->WriteToString(argstr3, nl->Third(second2));
     	CHECK_COND( nl->Equal(attrType, nl->Third(second2)),
      					"Operator " + opName + ": Attribute type '" + argstr3 + "' of the mapping-functions"
      					" is not the type " + argstr2 + " of attributename '" + argstr + "' of the tuples of the relation" );
      
     	// Construct a list with all indices of the changed attributes in the inputstream to be appended to the resultstream
     	if (firstcall){
     		numberList = nl->OneElemList(nl->IntAtom(attrIndex));
     		lastNumberList = numberList;
     		firstcall = false;
     	}
    	else{
     		lastNumberList = nl->Append(lastNumberList,nl->IntAtom(attrIndex));
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
  	// build secondo part of the resultstream
  	rest = nl->Second(nl->Second(first));
  	while (!(nl->IsEmpty(rest)))
  	{
  	 	nl->WriteToString(oldName, nl->First(nl->First(rest)));
  	 	oldName += "_old";
  	 	oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),nl->Second(nl->First(rest)));
     	lastlistn = nl->Append(lastlistn,oldAttribute);
     	rest = nl->Rest(rest);
  	}
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));  
  	outlist = nl->ThreeElemList(nl->SymbolAtom("APPEND"),nl->TwoElemList(nl->IntAtom(noAttrs),numberList),nl->TwoElemList(nl->SymbolAtom("stream"),
            					nl->TwoElemList(nl->SymbolAtom("tuple"),listn)));
  	return outlist;
}

/*
2.30.1 Type mapping function of operator ~updatedirect~

*/

ListExpr updateDirectRelTypeMap(ListExpr args){
	return updateTypeMap(args, "updatedirect");	
}
/*
2.30.2 Value mapping function of operator ~updatedirect~

*/
int UpdateDirect(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value ,relWord, noWord, elem;
  Tuple* tup;
  Supplier supplier, supplier2, supplier3, son;
  int  noOfAttrs, index;
  ArgVectorPointer funargs;
  TupleType *resultTupleType;
  ListExpr resultType;
  Relation* relation;
  Attribute* newAttribute;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :

      resultTupleType = (TupleType *)local.addr;
      qp->Request(args[1].addr,relWord);
      relation = (Relation*) relWord.addr;
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( *resultTupleType );
        // Copy the attributes from the old tuple
        assert( newTuple->GetNoAttributes() == 2 * tup->GetNoAttributes() + 1);
        for (int i = 0; i < tup->GetNoAttributes(); i++)
        	newTuple->PutAttribute( tup->GetNoAttributes()+i, tup->GetAttribute(i)->Clone() );	
        qp->Request(args[3].addr, noWord);
        // Number of attributes to be replaced
        noOfAttrs = ((CcInt*)noWord.addr)->GetIntval();
        // Supplier for the functions
        supplier = args[2].addr;
        vector<int>* changedIndices = new vector<int>(noOfAttrs);
        vector<Attribute*>* newAttrs = new vector<Attribute*>(noOfAttrs);
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
          (*funargs)[0] = SetWord(tup);
          qp->Request(supplier3,value);
          newAttribute = ((StandardAttribute*)value.addr)->Clone();
          (*newAttrs)[i-1] = newAttribute;
        }
        relation->UpdateTuple(tup,*changedIndices,*newAttrs); 
        for (int i = 0; i < tup->GetNoAttributes(); i++)
        	newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( newTuple->GetNoAttributes() - 1, tidAttr );
        delete changedIndices;
        delete newAttrs;
        tup->DeleteIfAllowed();
        result = SetWord(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      resultTupleType = (TupleType *)local.addr;
      delete resultTupleType;
      qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}


/*
2.18.3 Specification of operator ~updatedirect~

*/
const string UpdateDirectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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

ListExpr updateSearchRelTypeMap(ListExpr args){
	return updateTypeMap(args, "updatesearch");	
}
/*
2.18.2 Value mapping function of operator ~updatesearch~

*/
int UpdateSearch(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value ,relWord, noWord, elem;
  Tuple* tup;
  Tuple* newTuple;
  Tuple* nextTup;
  Supplier supplier, supplier2, supplier3, son;
  int noOfAttrs, index;
  ArgVectorPointer funargs;
  ListExpr resultType;
  Relation* relation;
  RelationIterator* iter;
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
        (*i)->DecReference();
        (*i)->DeleteIfAllowed();
      }
      delete updatedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DecReference();
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      delete resultTupleType;
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
      qp->Request(args[1].addr, relWord);
      relation = (Relation*)(relWord.addr); 
      assert(relation != 0);
      iter = new RelationIterator(*relation); 
      nextTup = iter->GetNextTuple();
      // Fill hashtable    
      while (!iter->EndOfScan())
      {
      	hashValue = 0;
      	for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
           hashValue += ((StandardAttribute*)(nextTup->GetAttribute(i)))->HashValue();		
        nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        nextTup->IncReference();
      	nextBucket->push_back(nextTup);
      	nextTup = iter->GetNextTuple(); 
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( localTransport );
      return 0;

    case REQUEST :

      localTransport =  (LocalTransport*) local.addr; 
      // Check if an already updated duplicate of the last inputtuple has to be given to the
      //resultstream first
      if (!localTransport->updatedTuples->empty())
      {
      	newTuple = localTransport->updatedTuples->back();
        newTuple->DecReference();
      	localTransport->updatedTuples->pop_back();
        result = SetWord(newTuple);
        return YIELD;
      }
      else
      // No more duplicates to send to the resultstream
      {     	 
      	qp->Request(args[1].addr,relWord);
      	relation = (Relation*) relWord.addr;
      	tupleFound = false;
      	// tupleFound will stay false until a tuple with the same values as the inputtuple was found and updated
      	while (! tupleFound)
        {
      		qp->Request(args[0].addr,t);
      		if (qp->Received(args[0].addr))
      		{
        		tup = (Tuple*)t.addr;
        		hashValue = 0;
      			for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			hashValue += ((StandardAttribute*)(tup->GetAttribute(i)))->HashValue();		
        		SortOrderSpecification full;
        		for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			full.push_back(pair<int,bool> (i+1,false));
        		TupleCompareBy compare(full);
        		// Get the right bucket from the hashtable
        		nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        		// Get all tuples from the bucket that have the same attributevalues as the inputtuple
        		for (size_t j = 0; j < nextBucket->size(); j++)
            {
        			nextTup = nextBucket->operator[](j);
        			if (nextTup != 0)
              {
        				if(!compare(nextTup,tup) && !compare(tup,nextTup))
                {
        					newTuple = new Tuple( *localTransport->resultTupleType );
        					assert( newTuple->GetNoAttributes() == 2 * nextTup->GetNoAttributes() + 1);
        					for (int i = 0; i < nextTup->GetNoAttributes(); i++)
        						newTuple->PutAttribute( nextTup->GetNoAttributes()+i, nextTup->GetAttribute(i)->Clone());	
        					qp->Request(args[3].addr, noWord);
        					noOfAttrs = ((CcInt*)noWord.addr)->GetIntval();
        					// Supplier for the functions
        					supplier = args[2].addr;
        					vector<int>* changedIndices = new vector<int>(noOfAttrs);
        					vector<Attribute*>* newAttrs = new vector<Attribute*>(noOfAttrs);
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
          					(*funargs)[0] = SetWord(nextTup);
          					qp->Request(supplier3,value);
          					newAttribute = ((StandardAttribute*)value.addr)->Clone();
          					(*newAttrs)[i-1] = newAttribute;       
        					}	
        					relation->UpdateTuple(nextTup,*changedIndices,*newAttrs); 
        					for (int i = 0; i < nextTup->GetNoAttributes(); i++)
        						newTuple->CopyAttribute( i, nextTup, i );
        					const TupleId& tid = nextTup->GetTupleId();
        					StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        					newTuple->PutAttribute( newTuple->GetNoAttributes() - 1, tidAttr );
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
              newTuple->DecReference();
        			localTransport->updatedTuples->pop_back();
        			result = SetWord(newTuple);
        			tupleFound = true;
        		}
        		tup->DeleteIfAllowed();
      		}
      		else// if (qp->Received(args[0].addr))
        		return CANCEL;
      	}// while (! tupleFound);
        return YIELD;
      }

    case CLOSE :

      localTransport = (LocalTransport*) local.addr;
      delete localTransport;
      qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      return 0;
  }
  return 0;
}


/*
2.18.3 Specification of operator ~updatesearch~

*/
const string UpdateSearchSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x)) x rel(tuple(x)) x [(a1, (tuple(x)"
                           " -> d1)) ... (an, (tuple(x) -> dn))] -> "
                           "stream(tuple(x @ [x1_old t1]@...[xn_old tn] @ [TID tid])))"
                           "</text--->"
                           "<text>_ _ updatesearch [funlist]</text--->"
                           "<text>Each tuple of the relation with the same values as one input tuple"
                           " is updated.</text--->"
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

Updates each input tuple by  replacing the attributevalues of the attributes given by their names in the 
function-argumentlist with the new values received from the functions. Precondition is that the tuples
of the inputstream originally belong to the relation that shall be updated.
The updated tuple is made persistent and as
the resultstream tuples are returned that contain the new values in first places, then all old values and finally
the TupleIdentifier of the updated tuple. Additionally the resulttuples are stored in an auxiliary relation
given as the third argument

2.18.0  General Type mapping functions of operators ~updatedirectsave~ and ~updatesearchsave~

Type mapping for ~updatedirectsave~ and ~updatesearchsave~is

----     (stream X) (rel X) (rel(tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn)(TID tid))))
		 ((ai1 (map x xi1)) ... (aij (map x xij)))

        -> (stream (tuple ((a1 x1) ... (an xn) (a1_old x1) ... (an_old xn)(TID tid))))

        where X = (tuple ((a1 x1) ... (an xn))) and ai1 - aij in (a1 .. an)
----

*/

ListExpr updateSaveTypeMap( ListExpr& args, string opName )
{
  	ListExpr first, second,third,fourth,errorInfo, rest, listn,lastlistn, first2, second2, 
  				firstr,attrType,numberList, lastNumberList,oldAttribute, outlist;
  	int attrIndex, noAttrs ;
  	bool firstcall = true;
  	AlgebraManager* algMgr;
  	errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  	algMgr = SecondoSystem::GetAlgebraManager();
  	string argstr, argstr2,argstr3,oldName;

  	CHECK_COND(nl->ListLength(args) == 4,
    			"Operator " + opName + " expects a list of length four.");

  	first = nl->First(args);
  	second  = nl->Second(args);
  	third = nl->Third(args);
  	fourth = nl->Fourth(args);
	// Check inputstream
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator " + opName + " expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as first argument '" + argstr + "'." );
    // Check updaterelation
  	CHECK_COND(!(nl->IsAtom(second)) &&
             	(nl->ListLength(second) > 0),
    			"Operator " + opName + ": Second argument list may not be empty or an atom" );
    
   	nl->WriteToString(argstr2, second);
   	CHECK_COND(nl->ListLength(second) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(second)) == rel) &&
             	(nl->ListLength(nl->Second(second)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) &&
       			(nl->ListLength(nl->Second(second)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(second)))),
    			"Operator " + opName + " expects as second argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as second argument '" + argstr2 + "'." );

  	// Check if tupletype of the tuples of the inputstream is the same as the one of
  	// the tuples of the update-relation
    nl->WriteToString(argstr, nl->Second(first));
    nl->WriteToString(argstr2, nl->Second(second));
    CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second))),
      			"Operator " + opName + ": Tuple type in the argumentstream '" + argstr +
      			"' is different from the tuple type '" + argstr2 +
      			"' in the relation" );
    // Check auxiliary relation  
    nl->WriteToString(argstr2, third);
   	CHECK_COND(nl->ListLength(third) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(third)) == rel) &&
             	(nl->ListLength(nl->Second(third)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(third))) == tuple) &&
       			(nl->ListLength(nl->Second(third)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(third)))),
    			"Operator " + opName + " expects as third argument a list with structure "
    			"(rel(tuple ((a1 t1)...(an tn))))\n"
    			"Operator " + opName + " gets as third argument '" + argstr2 + "'." );
	// Check functionlist for updating
  	CHECK_COND(!(nl->IsAtom(fourth)) &&
             	(nl->ListLength(fourth) > 0),
    			"Operator " + opName + ": Fourth argument list may not be empty or an atom" );

  	rest = fourth;
  	noAttrs = nl->ListLength(fourth);
  	// Check each update-function
  	while (!(nl->IsEmpty(rest)))
  	{
    	firstr = nl->First(rest);
    	rest = nl->Rest(rest);
    	first2 = nl->First(firstr);
    	second2 = nl->Second(firstr);
      
		// Is it a correct function
    	nl->WriteToString(argstr, second2);
    	CHECK_COND( (nl->ListLength(second2) == 3) &&
                	(TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
          			(algMgr->CheckKind("DATA", nl->Third(second2), errorInfo)),
      				"Operator " + opName + " expects a mapping function with list structure"
      				" (<attrname> (map (tuple ( (a1 t1)...(an tn) )) ti) )\n. Operator"
      				" " + opName + " gets a list '" + argstr + "'.\n" );
		// Is the type of the argumentuples of the function the same tupletype as in the updaterelation?
    	nl->WriteToString(argstr, nl->Second(first));
    	nl->WriteToString(argstr2, second2);
    	CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second2))),
      				"Operator " + opName + ": Tuple type in first argument '" + argstr +
      				"' is different from the argument tuple type '" + argstr2 +
      				"' in the mapping function" );
      	// Is the function attributename a valid name?
    	nl->WriteToString(argstr, first2);
    	CHECK_COND( (nl->IsAtom(first2)) &&
                	(nl->AtomType(first2) == SymbolType),
      				"Operator " + opName + ": Attribute name '" + argstr +
      				"' is not an atom or not of type SymbolType" );
      	// Is the attributname of the function an attributename in the tuples of the update-relation?
    	attrIndex = FindAttribute(nl->Second(nl->Second(first)), argstr, attrType);
    	CHECK_COND( attrIndex != 0 ,
      				"Operator " + opName + ": Attribute name '" + argstr + "' of the mapping-functions"
      				" is not an attributename of the tuples of the relation" );
      	// Are the types of those attributes the same?
    	nl->WriteToString(argstr2, attrType);
     	nl->WriteToString(argstr3, nl->Third(second2));
     	CHECK_COND( nl->Equal(attrType, nl->Third(second2)),
      				"Operator " + opName + ": Attribute type '" + argstr3 + "' of the mapping-functions"
      				" is not the type " + argstr2 + " of attributename '" + argstr + "' of the tuples of the relation" );
      	// build a list with all indices of the updated attributes in the update-relation to be appended
      	// to the resultstream
     	if (firstcall){
     		numberList = nl->OneElemList(nl->IntAtom(attrIndex));
     		lastNumberList = numberList;
     		firstcall = false;
     	}
     	else{
     		lastNumberList = nl->Append(lastNumberList,nl->IntAtom(attrIndex));
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
  	// build secondo part of the resultstream
  	rest = nl->Second(nl->Second(first));
  	while (!(nl->IsEmpty(rest)))
  	{
  	 	nl->WriteToString(oldName, nl->First(nl->First(rest)));
  	 	oldName += "_old";
  	 	oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),nl->Second(nl->First(rest)));
     	lastlistn = nl->Append(lastlistn,oldAttribute);
     	rest = nl->Rest(rest);
  	}
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  
  	outlist = nl->ThreeElemList(
                nl->SymbolAtom("APPEND"),
                nl->TwoElemList(
                  nl->IntAtom(noAttrs),
                  numberList),
                nl->TwoElemList(
                  nl->SymbolAtom("stream"),
            			nl->TwoElemList(
                    nl->SymbolAtom("tuple"),
                    listn)));
  	return outlist;
}


/*
2.30.1 Type mapping function of operator ~updatedirectsave~

*/

ListExpr updateDirectSaveRelTypeMap(ListExpr args){
	return updateSaveTypeMap(args, "updatedirectsave");
	
}

/*
2.30.2 Value mapping function of operator ~updatedirectsave~

*/
int UpdateDirectSave(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value ,relWord,argAuxRelation, noWord, elem;
  Tuple* tup;
  Supplier supplier, supplier2, supplier3, son;
  int  noOfAttrs, index;
  ArgVectorPointer funargs;
  TupleType *resultTupleType;
  ListExpr resultType;
  Relation* relation;
  Relation* auxRelation;
  Attribute* newAttribute;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :

      resultTupleType = (TupleType *)local.addr;
      qp->Request(args[1].addr,relWord);
      relation = (Relation*) relWord.addr;
      qp->Request(args[2].addr, argAuxRelation);
      auxRelation = (Relation*)(argAuxRelation.addr);    
      assert(auxRelation != 0); 
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( *resultTupleType );
        assert( newTuple->GetNoAttributes() == 2 * tup->GetNoAttributes() + 1);
        for (int i = 0; i < tup->GetNoAttributes(); i++)
        	newTuple->PutAttribute( tup->GetNoAttributes()+i, tup->GetAttribute(i)->Clone() );	
        qp->Request(args[4].addr, noWord);
        noOfAttrs = ((CcInt*)noWord.addr)->GetIntval();
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
          (*funargs)[0] = SetWord(tup);
          qp->Request(supplier3,value);
          newAttribute = ((StandardAttribute*)value.addr)->Clone();
          (*newAttrs)[i-1] = newAttribute;
        }
        relation->UpdateTuple(tup,*changedIndices,*newAttrs);
        for (int i = 0; i < tup->GetNoAttributes(); i++)
      		newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( newTuple->GetNoAttributes() - 1, tidAttr);
        auxRelation->AppendTuple(newTuple);
        tup->DeleteIfAllowed();
        delete changedIndices;
        delete newAttrs;
        result = SetWord(newTuple);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :

      resultTupleType = (TupleType *)local.addr;
      delete resultTupleType;
      qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      qp->SetModified(args[2].addr);
      return 0;
  }
  return 0;
}


/*
2.30.3 Specification of operator ~updatedirectsave~

*/
const string UpdateDirectSaveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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

Updates each tuple of the relation that has the same values as one of the input-tuples by  
replacing the attributevalues of the attributes given by their names in the 
function-argumentlist with the new values received from the functions. 
The updated tuple is made persistent and as
the resultstream tuples are returned that contain the new values in first places, then all old values and finally
the TupleIdentifier of the updated tuple. Additionally the resulttuples are stored in an auxiliary relation
given as the third argument

2.31.1 Type mapping function of operator ~updatesearchsave~

*/

ListExpr updateSearchSaveRelTypeMap(ListExpr args){
return updateSaveTypeMap(args, "updatesearchsave");
	
}

/*
2.31.2 Value mapping function of operator ~updatesearchsave~

*/
int UpdateSearchSave(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value ,relWord,argAuxRelation, noWord, elem;
  Tuple* tup;
  Tuple* newTuple;
  Tuple* nextTup;
  Supplier supplier, supplier2, supplier3, son;
  int noOfAttrs, index;
  ArgVectorPointer funargs;
  ListExpr resultType;
  Relation* relation;
  Relation* auxRelation;
  RelationIterator* iter;
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
        (*i)->DecReference();
        (*i)->DeleteIfAllowed();
      }
      delete updatedTuples;

      for (int i = 0; i < 256; i++)
      {
        for( vector<Tuple*>::iterator j = (*hashTable)[i]->begin();
             j != (*hashTable)[i]->end();
             j++ )
        {
          (*j)->DecReference();
          (*j)->DeleteIfAllowed();
        }
        delete (*hashTable)[i];
      }
      delete hashTable;

      delete resultTupleType;
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
      qp->Request(args[1].addr, relWord);
      relation = (Relation*)(relWord.addr); 
      assert(relation != 0);
      iter = new RelationIterator(*relation); 
      nextTup = iter->GetNextTuple();
      // fill hashtable    
      while (!iter->EndOfScan())
      {
        hashValue = 0;
        for( int i = 0; i < nextTup->GetNoAttributes(); i++ )
           hashValue += ((StandardAttribute*)(nextTup->GetAttribute(i)))->HashValue();
        nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        nextTup->IncReference();
        nextBucket->push_back(nextTup);
        nextTup = iter->GetNextTuple();
      }
      delete iter;
      resultType = GetTupleResultType( s );
      localTransport->resultTupleType = new TupleType( nl->Second( resultType ) );
      local = SetWord( localTransport );
      return 0;

    case REQUEST :

      localTransport =  (LocalTransport*) local.addr;
      // Check if an already updated duplicate of the last inputtuples has to be send to
      // the outputstream first
      if (!localTransport->updatedTuples->empty())
      {
        newTuple = localTransport->updatedTuples->back();
        newTuple->IncReference();
        localTransport->updatedTuples->pop_back();
        result = SetWord(newTuple);
        return YIELD;
      }
      else
      // No more duplicates to be send to the outputstream
      {
      	qp->Request(args[1].addr,relWord);
      	relation = (Relation*) relWord.addr;
      	qp->Request(args[2].addr, argAuxRelation);
      	auxRelation = (Relation*)(argAuxRelation.addr);    
      	assert(auxRelation != 0);
      	tupleFound = false;
      	// tupleFound will stay false until a tuple with the same values as one of the 
      	//inputtuples was found
      	while (! tupleFound)
        { 
      		qp->Request(args[0].addr,t);
      		if (qp->Received(args[0].addr))
      		{
        		tup = (Tuple*)t.addr;
        		hashValue = 0;
      			for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			hashValue += ((StandardAttribute*)(tup->GetAttribute(i)))->HashValue();		
        		SortOrderSpecification full;
        		for( int i = 0; i < tup->GetNoAttributes(); i++ )
           			full.push_back(pair<int,bool> (i+1,false));
        		TupleCompareBy compare(full);
        		// Get the right bucket
        		nextBucket = localTransport->hashTable->operator[](hashValue % 256);
        		// Look for all tuples in the bucket if they have the same attributevalues
        		for (size_t j = 0; j < nextBucket->size(); j++)
            {
        			nextTup = nextBucket->operator[](j);
        			if (nextTup != 0)
              {
        				if(!compare(nextTup,tup) && !compare(tup,nextTup))
                {
        					newTuple = new Tuple( *localTransport->resultTupleType );
        					assert( newTuple->GetNoAttributes() == 2 * nextTup->GetNoAttributes() + 1);
        					for (int i = 0; i < nextTup->GetNoAttributes(); i++)
        						newTuple->PutAttribute( nextTup->GetNoAttributes() +i, nextTup->GetAttribute(i)->Clone());	
        					qp->Request(args[4].addr, noWord);
        					noOfAttrs = ((CcInt*)noWord.addr)->GetIntval();
        					// Supplier for the updatefunctions
        					supplier = args[3].addr;
        					vector<int>* changedIndices = new vector<int>(noOfAttrs);
        					vector<Attribute*>* newAttrs = new vector<Attribute*>(noOfAttrs);
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
          					(*funargs)[0] = SetWord(nextTup);
          					qp->Request(supplier3,value);
          					newAttribute = ((StandardAttribute*)value.addr)->Clone();
          					(*newAttrs)[i-1] = newAttribute;       
        					}	
        					relation->UpdateTuple(nextTup,*changedIndices,*newAttrs); 
        					for (int i = 0; i < nextTup->GetNoAttributes(); i++)
        						newTuple->CopyAttribute( i, nextTup, i );
        					const TupleId& tid = nextTup->GetTupleId();
        					StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        					newTuple->PutAttribute( newTuple->GetNoAttributes() - 1, tidAttr);
        					auxRelation->AppendTuple(newTuple);	
        					delete changedIndices;
        					delete newAttrs;
                  newTuple->IncReference();
        					localTransport->updatedTuples->push_back(newTuple);
        				}
        			}
        		}  
        		// Check if at least one updated tuple has to be send to the outputstream    	
        		if (!localTransport->updatedTuples->empty())
            {
        			newTuple = localTransport->updatedTuples->back();
              newTuple->DecReference();
        			localTransport->updatedTuples->pop_back();
        			result = SetWord(newTuple);
        			tupleFound = true;
        		}
        		tup->DeleteIfAllowed();
      		}
      		else// if (qp->Received(args[0].addr))
        		return CANCEL;
      	}// while (! tupleFound);
        return YIELD;
      }

    case CLOSE :

      localTransport = (LocalTransport*) local.addr;
      delete localTransport;
      qp->Close(args[0].addr);
      qp->SetModified(args[1].addr);
      qp->SetModified(args[2].addr);
      return 0;
  }
  return 0;
}


/*
2.31.3 Specification of operator ~updatesearchsave~

*/
const string UpdateSearchSaveSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x)) x rel(tuple(x)) x (rel(tuple(x @ x @ [TID tid]))) x [(a1, (tuple(x)"
                           " -> d1)) ... (an, (tuple(x) -> dn))] -> "
                           "stream(tuple(x @[x1_old t1]@...[xn_old t1] @ [TID tid]))"
                           "</text--->"
                           "<text>_ _ _updatesearchsave [funlist]</text--->"
                           "<text>Like `updatesearch` but each result-tuple "
                           " is appended to the auxiliary relation.</text--->"
                           "<text>query staedteUp feed staedte staedteU updatesearchsave [Bev : "
                           ".Bev +1000] consume"
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
2.32 Operator ~appendidentifier~

2.32.1 Type mapping function of operator ~appendidentifier~

----    ((stream (tuple (x1 ... xn))))

        -> (stream (tuple (x1 ... xn (TID tid))))
----

*/

ListExpr appendIdentifierTypeMap (ListExpr args){
	ListExpr first, rest,listn,lastlistn, outList;
  	string argstr;
  
   	CHECK_COND(nl->ListLength(args) == 1,
    	"Operator 'appendidentifier' expects a list of length one.");
  

  	first = nl->First(args);
  	
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator 'appendidentifier' expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn))))\n"
    			"Operator 'appendidentifier' gets as first argument '" + argstr + "'." );

  	
    // build resutllist
  	rest = nl->Second(nl->Second(first));
  	listn = nl->OneElemList(nl->First(rest));
  	lastlistn = listn;
  	rest = nl->Rest(rest);
  	while (!(nl->IsEmpty(rest)))
  	{
     	lastlistn = nl->Append(lastlistn,nl->First(rest));
     	rest = nl->Rest(rest);
  	}
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
            	nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}


/*
2.32.2 Value mapping function of operator ~appendidentifier~

*/
int appendIdentifierValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
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
      local = SetWord( resultTupleType );
      return 0;

    case REQUEST :
      resultTupleType = (TupleType*) local.addr;
      qp->Request(args[0].addr,t);      
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( *resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ )
          newTuple->CopyAttribute( i, tup, i );
        const TupleId& tid = tup->GetTupleId();
        StandardAttribute* tidAttr = new TupleIdentifier(true,tid);      
        newTuple->PutAttribute( tup->GetNoAttributes(), tidAttr);
        result = SetWord(newTuple);
        tup->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	resultTupleType = (TupleType*) local.addr;
    	delete resultTupleType;
    	qp->Close(args[0].addr);
      return 0;
  }
  return 0;
} 



/*
2.32.3 Specification of operator ~appendidentifier~

*/
const string appendIdentifierSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x)) "
                           " -> stream(tuple(x@[TID:tid]))] "
                           "</text--->"
                           "<text>_ appendidentifier</text--->"
                           "<text>Appends an attribute which is the"
                           " tuple-id of the tuple to each tuple.</text--->"
                           "<text>query staedte feed appendidentifier consume"
                           "</text--->"
                           ") )";

/*
2.32.4 Definition of operator ~appendidentifier~

*/
Operator extrelappendidentifier (
         "appendidentifier",              // name
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
  
  
  	CHECK_COND(nl->ListLength(args) == 2,
    			"Operator deletebyid expects a list of length two.");
  
  	first = nl->First(args);
  	second = nl->Second(args);
    // Check relation
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator deletebyid expects as first argument a list with structure "
    			"(rel (tuple ((a1 t1)...(an tn))))\n"
    			"Operator deletebyid gets as first argument '" + argstr + "'." );
    // Check secondo argument 
  	nl->WriteToString(argstr, second);    
  	CHECK_COND(nl->IsAtom(second) &&
             	nl->SymbolValue(second) == "tid",
    			"Operator deletebyid expects as second argument a tupleidentifier "
    			"Operator deletebyid gets as second argument'" + argstr + "'.");
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
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));
  	outList = nl->TwoElemList(nl->SymbolAtom("stream"),
           						 nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
  	return outList;
}


/*
2.33.2 Value mapping function of operator ~deletebyid~

*/
int deleteByIdValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word argRelation, argTid;
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
      local = SetWord( firstcall );
      return 0;

    case REQUEST :
  	  firstcall = (bool*) local.addr;   
      if (*firstcall)
      {
      	*firstcall = false;
      	resultType = GetTupleResultType( s );
      	resultTupleType = new TupleType( nl->Second( resultType ) );
      	qp->Request(args[0].addr, argRelation);
      	relation = (Relation*)(argRelation.addr);    
      	assert(relation != 0);  
      	qp->Request(args[1].addr, argTid);
      	tid = (TupleIdentifier*)(argTid.addr);      
        resultTuple = new Tuple( *resultTupleType );
        deleteTuple = relation->GetTuple(tid->GetTid());
        if (deleteTuple == 0)
        {
        	 delete resultTupleType;
        	 delete resultTuple;
        	 return CANCEL;
        }
        for (int i = 0; i < deleteTuple->GetNoAttributes(); i++)
        	resultTuple->PutAttribute(i, deleteTuple->GetAttribute(i)->Clone());	
        relation->DeleteTuple(deleteTuple);
        resultTuple->PutAttribute( resultTuple->GetNoAttributes() -1, tid->Clone());
        result = SetWord(resultTuple);
        delete resultTupleType;
        deleteTuple->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	firstcall = (bool*) local.addr;
    	delete firstcall;
    	qp->SetModified(args[0].addr);
      return 0;
  }
  return 0;
}



/*
2.33.3 Specification of operator ~deletebyid~

*/
const string deleteByIdSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
  	ListExpr first, second,third,errorInfo, rest, listn, lastlistn, first2, second2, firstr,attrType,
  			 numberList, lastNumberList,oldAttribute, outlist;
  	string argstr,valueString, argstr2, argstr3, oldName;
  	int attrIndex, noAttrs ;
  	bool firstcall = true;
  	AlgebraManager* algMgr;
	algMgr = SecondoSystem::GetAlgebraManager();
	errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  
  	CHECK_COND(nl->ListLength(args) == 3,
    			"Operator updatebyid expects a list of length three.");
  
  	first = nl->First(args);
  	second = nl->Second(args);
  	third = nl->Third(args);
    // Check relation
  	nl->WriteToString(argstr, first);
  	CHECK_COND(nl->ListLength(first) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(first)) == rel) &&
             	(nl->ListLength(nl->Second(first)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       			(nl->ListLength(nl->Second(first)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(first)))),
    			"Operator updatebyid expects as first argument a list with structure "
    			"(rel (tuple ((a1 t1)...(an tn))))\n"
    			"Operator updatebyid gets as first argument '" + argstr + "'." );
    // Check second argument 
  	nl->WriteToString(argstr, second);    
  	CHECK_COND(nl->IsAtom(second) &&
             	nl->SymbolValue(second) == "tid",
    			"Operator updatebyid expects as second argument a tupleidentifier "
    			"Operator updatebyid gets as second argument'" + argstr + "'.");
  	// Check function-argumentlist
  	CHECK_COND(!(nl->IsAtom(third)) &&
            		(nl->ListLength(third) > 0),
    				"Operator updatebyid: Third argument list may not be empty or an atom" );

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
    	CHECK_COND( (nl->ListLength(second2) == 3) &&
                		(TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
          				(algMgr->CheckKind("DATA", nl->Third(second2), errorInfo)),
      					"Operator updatebyid expects a mapping function with list structure"
      					" (<attrname> (map (tuple ( (a1 t1)...(an tn) )) ti) )\n. Operator"
      					" updatebyid gets a list '" + argstr + "'.\n" );
		// Check if functions have argumenttuples of the same type as those ones from the relation
    	nl->WriteToString(argstr, nl->Second(first));
    	nl->WriteToString(argstr2, second2);
    	CHECK_COND( (nl->Equal(nl->Second(first),nl->Second(second2))),
      					"Operator updatebyid: Tuple type in first argument '" + argstr +
      					"' is different from the argument tuple type '" + argstr2 +
      					"' in the mapping function" );
      	// Is the name of the attribute that shall be computed by the function a valid name
    	nl->WriteToString(argstr, first2);
    	CHECK_COND( (nl->IsAtom(first2)) &&
                		(nl->AtomType(first2) == SymbolType),
      					"Operator updatebyid: Attribute name '" + argstr +
      					"' is not an atom or not of type SymbolType" );
      	// Is the name of the attribute of the function an attributename of the tuples of the relation
    	attrIndex = FindAttribute(nl->Second(nl->Second(first)), argstr, attrType);
    	CHECK_COND( attrIndex != 0 ,
      					"Operator updatebyid: Attribute name '" + argstr + "' of the mapping-functions"
      					" is not an attributename of the tuples of the relation" );
      	// Is the type of the attribute of the function the same as the type of the attribute of the tuples of the relation
     	nl->WriteToString(argstr2, attrType);
     	nl->WriteToString(argstr3, nl->Third(second2));
     	CHECK_COND( nl->Equal(attrType, nl->Third(second2)),
      					"Operator updatebyid: Attribute type '" + argstr3 + "' of the mapping-functions"
      					" is not the type " + argstr2 + " of attributename '" + argstr + "' of the tuples of the relation" );
      
     	// Construct a list with all indices of the changed attributes in the inputstream to be appended to the resultstream
     	if (firstcall){
     		numberList = nl->OneElemList(nl->IntAtom(attrIndex));
     		lastNumberList = numberList;
     		firstcall = false;
     	}
    	else{
     		lastNumberList = nl->Append(lastNumberList,nl->IntAtom(attrIndex));
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
  	 	oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),nl->Second(nl->First(rest)));
     	lastlistn = nl->Append(lastlistn,oldAttribute);
     	rest = nl->Rest(rest);
  	}
  	lastlistn= nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("TID"), nl->SymbolAtom("tid")));  
  	outlist = nl->ThreeElemList(nl->SymbolAtom("APPEND"),nl->TwoElemList(nl->IntAtom(noAttrs),numberList),nl->TwoElemList(nl->SymbolAtom("stream"),
            					nl->TwoElemList(nl->SymbolAtom("tuple"),listn)));
  	return outlist;
}


/*
2.34.2 Value mapping function of operator ~updatebyid~

*/
int updateByIdValueMap(Word* args, Word& result, int message, Word& local, Supplier s){
  Word argRelation, argTid, noWord, elem,value;
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
      local = SetWord( firstcall );
      return 0;

    case REQUEST :
	  firstcall = (bool*) local.addr;   
      if (*firstcall)
      {
      	*firstcall = false;
      	resultType = GetTupleResultType( s );
      	resultTupleType = new TupleType( nl->Second( resultType ) );
      	qp->Request(args[0].addr, argRelation);
      	relation = (Relation*)(argRelation.addr);    
      	assert(relation != 0);  
      	qp->Request(args[1].addr, argTid);
      	tid = (TupleIdentifier*)(argTid.addr);      
        resultTuple = new Tuple( *resultTupleType );
        updateTuple = relation->GetTuple(tid->GetTid());
        if (updateTuple == 0)
        {
        	 delete resultTupleType;
        	 delete resultTuple;
        	 return CANCEL;
        }
        for (int i = 0; i < updateTuple->GetNoAttributes(); i++)
        	resultTuple->PutAttribute( updateTuple->GetNoAttributes() +i, updateTuple->GetAttribute(i)->Clone());	
        qp->Request(args[3].addr, noWord);
        // Number of attributes to be replaced
        noOfAttrs = ((CcInt*)noWord.addr)->GetIntval();
        // Supplier for the functions
        supplier = args[2].addr;
        vector<int>* changedIndices = new vector<int>(noOfAttrs);
        vector<Attribute*>* newAttrs = new vector<Attribute*>(noOfAttrs);
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
          (*funargs)[0] = SetWord(updateTuple);
          qp->Request(supplier3,value);
          newAttribute = ((StandardAttribute*)value.addr)->Clone();
          (*newAttrs)[i-1] = newAttribute;
        }
        relation->UpdateTuple(updateTuple,*changedIndices,*newAttrs); 
        for (int i = 0; i < updateTuple->GetNoAttributes(); i++)
        	resultTuple->CopyAttribute( i, updateTuple, i );
        resultTuple->PutAttribute( resultTuple->GetNoAttributes() -1, tid->Clone());
        result = SetWord(resultTuple);
        delete changedIndices;
        delete newAttrs;
        delete resultTupleType;
        updateTuple->DeleteIfAllowed();
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
    	firstcall = (bool*) local.addr;
    	delete firstcall;
    	qp->SetModified(args[0].addr);
        return 0;
  }
  return 0;
}



/*
2.34.3 Specification of operator ~updatebyid~

*/
const string updateByIdSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x))) x (rel(tuple(x))) x (tid) x [(a1, (tuple(x)"
                           " -> d1)) ... (an, (tuple(x) -> dn))] -> "
                           "stream(tuple(x @[x1_old t1] @...[xn_old tn] @[TID tid])))"
                           "</text--->"
                           "<text> _ _ updatebyid [ _; funlist ]</text--->"
                           "<text>Updateds the tuple with the id from the"
                           "third argument by applying the functions of the "
                           " list to the tuple.</text--->"
                           "<text>query staedte feed staedte updatebyid [[const tid value (5)]"
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
    AddOperator(&extrelappendidentifier);
    AddOperator(&extreldeletebyid);
    AddOperator(&extrelupdatebyid);
  }
  ~UpdateRelationAlgebra() {};
};

UpdateRelationAlgebra updateRelationalgebra;

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
InitializeUpdateRelationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&updateRelationalgebra);
}

