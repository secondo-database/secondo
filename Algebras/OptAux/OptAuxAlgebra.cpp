/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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

//paragraph [1] title: [{\Large \bf ]   [}]
//characters    [2]    verbatim:   [\verb@]    [@]
//[ue] [\"{u}]
//[toc] [\tableofcontents]

""[2]

[1] OptAux Algebra


*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"


/*
The file "Algebra.h" is included, since the new algebra must be a subclass of
class Algebra. All of the data available in Secondo has a nested list
representation. Therefore, conversion functions have to be written for this
algebra, too, and "NestedList.h" is needed for this purpose. The result of an
operation is passed directly to the query processor. An instance of
"QueryProcessor" serves for this. Secondo provides some standard data types, e.g.
"CcInt", "CcReal", "CcString", "CcBool", which is needed as the result type of the
implemented operations. To use them "StandardTypes.h" needs to be included.

*/

extern NestedList* nl;
extern QueryProcessor *qp;

/*
The variables above define some global references to unique system-wide
instances of the query processor and the nested list storage.

1.2 Auxiliaries

Within this algebra module implementation, we have to handle values of
four different types defined in namespace ~symbols~: ~INT~ and ~REAL~, ~BOOL~ and
~STRING~.  They are constant values of the C++-string class.


*/

#include "TypeMapUtils.h"
#include "Symbols.h"

using namespace mappings;

#include <string>
using namespace std;

/*
The implementation of the algebra is embedded into
a namespace ~prt~ in order to avoid name conflicts with other modules.

*/

namespace optaux {

/*
5 Creating Operators

5.1 The Local Info Datatype

The Local Info Datatype is a template class with parameters "INDEX\_TYPE" and
"RESULT\_TYPE". The parameter "INDEX\_TYPE" defines the data type to be used
for the index of the temporary result array. By defining this one determine the
maximum number of predicates evaluable by the operator predcounts, too. The
maximum number is determined by the length of the data type (in bit)
decremented by 1.

The parameter "RESULT\_TYPE" defines the data type of counters which counts the
tuples meeting the set of predicates identified by the index of the counter.
This data type determines the maximum number of tuples processable by the
operator predcounts. The selected data type for "RESULT\_TYPE" have to be int,
long, long or similiar. For safety reasons the maximum count is limited to
2\^(sizeof("RESULT\_TYPE")-1).

*/

template <class INDEX_TYPE, class RESULT_TYPE>
class GeneralPredcountsLocalData {

  private:
    RESULT_TYPE counter;

    inline static unsigned short highestBit(unsigned int value)
    {
	unsigned short rtn;

	for (rtn=0; (value>>rtn)>0; rtn++) /* nothing more to do */;

	return rtn;
    }

  public:

   GeneralPredcountsLocalData() :
     counter(0),
     resultTupleType(0),
     predicateCombinations(0),
     resultCounters(0)
   {}

   ~GeneralPredcountsLocalData()
   {
     resultTupleType->DeleteIfAllowed(); // produced tuples may have still
                                    // references to it;
     delete [] resultCounters;
   }

   TupleType*   resultTupleType;
   INDEX_TYPE   predicateCombinations;
   RESULT_TYPE* resultCounters;

   inline static size_t maxPredicateCount()
   {
      // the maximum count are equal to the count of bits of INDEX_TYPE
      // for savety reasons (usage of int instead of unsigned int anywhere)
      // decrement by 1 the memory to be allocated for the counter array
      // is the product of 2\^maxPredicateCount * 2\^sizeof(RESULT_TYPE)
      // this value must not greater than 2\^32 (2\^64 at 64 bit libs)
      // that's why reduce maxPredicateCount by ld(2\^sizeof(RESULT_TYPE))
      return (32/*Bit*/ - highestBit(sizeof(RESULT_TYPE)*8-1) - 1);
   }

   bool allocate(size_t predCount)
   {
     // calculate the limitations of operator predcounts and check them
     if ( predCount > maxPredicateCount() )
     {
        cerr << "ERROR: Anzahl der Praedikate (=" << predCount <<
               ") ist zu hoch fuer predcounts - max. " <<
               maxPredicateCount() << endl;
        return false;
     }
     predicateCombinations = 1 << predCount; // =2\^predCount
     try {
       resultCounters = new RESULT_TYPE[predicateCombinations];
     } catch (bad_alloc &e) {
       cerr << "ERROR: konnte Speicher " <<
                (sizeof(RESULT_TYPE) * predicateCombinations) <<
                " nicht allokieren (predicateCount=" << predCount <<
                " combinations=" << predicateCombinations << ")" << endl;
        return false;
     }

     for(INDEX_TYPE i=0; i<predicateCombinations; i++) {
        resultCounters[i] = 0;
     }

     return true;
   }


   inline bool checkOverFlow()
   {

     const RESULT_TYPE ctrMax =
        ((((RESULT_TYPE) 1) << ( sizeof(RESULT_TYPE)*8 - 1 ) ) - 1) ;

     counter++;

     if ( counter > ctrMax ) {
            cerr << "ERROR: es koennen nur max. " << ctrMax <<
               " Zeilen des Eingabestromes sicher verarbeitet werden, " <<
               "dannach besteht die Gefahr eines Ueberlaufes" << endl;
       return false;
     }

     return true;
   }

};

typedef GeneralPredcountsLocalData<unsigned int, unsigned int>
   PredcountsLocalData;


/*

The structure PredcountsLocalData is used to structure local data supplied by
parameter local of value mapping function. The element resultTupleType contains
the type of tuple to be returned by value mapping function. The element
predicateCombinations is used to store the count of predicate combinations
checked by the operator predcounts at first. During the REQUEST phase the
element is used to store the last responsed (pattern, count) tuple. The element
resultCounters is a dynamically allocated array. It's temporary used to count
up and store the occurrence of predicate combinations met by tuples of the
input stream.

5.2 Type Mapping Functions

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. Again we use interface ~NList.h~ for
manipulating list expressions.

5.2.1 The ~predcounts~ operator

The predcounts operator have to be called with two or more parameters.
The type of the first parameter have to be a stream of tuples.
(see Relational Algebras for details of tuples)
The type of the other parameters have to be the default datatype bool of Secondo.

The parameter two and later have to be named. This name is only necessary
for the parser not for the predicate itself.

An example for calling the operator is shown in file "OptAlg.example".

*/

/*
The helper function WriteIntToString writes an Integer (unsigned int) into a string.

*/
void WriteIntToString(string *p_str, unsigned int value) {
   char buffer[13];
   sprintf(buffer, "%i", value);
   (*p_str).erase(0);
   (*p_str) += buffer;
}

/*
The value mapping function now checks count and types of the parameters.

*/

ListExpr predcounts_tm(ListExpr args)
{
   ListExpr resultType;
   ListExpr predicateAlias, mapping,
      streamDeclaration, predicateDeclaration,
      rest, predicateDeclarations,
      streamTupleDeclaration, mappingInputType;
   string argstr, argstr2;
   int maxPredicateCount;

   // the upper limit of predicates is defined
   //by the count of bit of INDEX\_TYPE
   maxPredicateCount = PredcountsLocalData::maxPredicateCount();

   // we expect an input stream and
   // at least one additional parameter
   if(nl->ListLength(args) != 2){
      return listutils::typeError("Operator predcounts expects a "
                                                "list of length two.");
   }

   streamDeclaration = nl->First(args);
   predicateDeclarations = nl->Second(args);

   // check type of input stream
   nl->WriteToString(argstr, streamDeclaration);
   if( !listutils::isTupleStream(streamDeclaration) ) {
        return listutils::typeError(
            "Operator predcounts expects as "
            "first argument a list with structure "
            "(stream (tuple ((a1 t1)...(an tn))))\n"
            "Operator predcounts gets as first argument '" +
            argstr + "'." );
  }

   streamTupleDeclaration = nl->Second(streamDeclaration);

   // check predicate list - it should be a not empty list
   if((nl->IsAtom(predicateDeclarations)) ||
     (nl->ListLength(predicateDeclarations) <= 0)){
       return listutils::typeError(
          "Operator predcounts: Second argument list may"
          " not be empty or an atom" );
   }

   // if there are more then maxPredicateCount predicate declarations
   // return a type error - its necessary too increase
   // the count of bits of INDEX_TYPE
   WriteIntToString(&argstr, nl->ListLength(predicateDeclarations));
   WriteIntToString(&argstr2, maxPredicateCount);
   if( nl->ListLength(predicateDeclarations)>maxPredicateCount){
     return listutils::typeError(
      "Operator predcounts is just able to handle up to " + argstr2
      + " predicates - given predicates: " + argstr);
   }

   // check predicate list - check the list more detailed
   rest = predicateDeclarations;
   while ( !(nl->IsEmpty(rest)) ) {

      predicateDeclaration = nl->First(rest);
      rest = nl->Rest(rest);

      // check the predicate alias
      if(!nl->HasLength(predicateDeclaration,2)) {
        return listutils::typeError("PredicateDeclaration"
                                    " must have two elems");
      }     
 
      predicateAlias = nl->First(predicateDeclaration);
      nl->WriteToString(argstr, predicateAlias);
      if( !(nl->IsAtom(predicateAlias)) ||
        (nl->AtomType(predicateAlias) != SymbolType)){
          return listutils::typeError(
            "Operator predcounts: predicate alias '" + argstr +
            "' is not an atom or not of type SymbolType" );
        }
      // check type of mapping function
      mapping = nl->Second(predicateDeclaration);
      mappingInputType = nl->Second(mapping);
      nl->WriteToString(argstr, mapping);
      if( (nl->ListLength(mapping) != 3) ||
         (TypeOfRelAlgSymbol(nl->First(mapping)) != ccmap) ||
         (TypeOfRelAlgSymbol(nl->Third(mapping)) != ccbool) ){
        return listutils::typeError(
         "Operator predcounts expects a mapping "
         "function with list structure"
         " (<attrname> (map (tuple ( (a1 t1)...(an tn) )) bool) )\n. Operator"
         " predcounts gets a list '" + argstr + "'.\n" );
      }
      // check tuple type supplied by stream and requested by predicate
      nl->WriteToString(argstr, streamTupleDeclaration);
      nl->WriteToString(argstr2, mappingInputType);
      if( !(nl->Equal(streamTupleDeclaration, mappingInputType)) ){
        return listutils::typeError(
         "Operator predcounts: Tuple type in first argument '" + argstr +
         "' is different from the argument tuple type '" + argstr2 +
         "' in the mapping function" );
      }
   }

/*

The type of the result of the operator precounts is always the same.
Consequently this type could be contructed statically at the end of the type
mapping function.

The type is a stream of tuples. Each tuple is composed of two integer
attributes named pattern and count.

*/
   // Zieltyp zusammensetzen
   // ??? die hardcodierten Literale muessen raus
   resultType = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
         nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("Atom"),
                            nl->SymbolAtom(CcInt::BasicType())),
            nl->TwoElemList(nl->SymbolAtom("Count"),
                            nl->SymbolAtom(CcInt::BasicType())))));
   return resultType;
}

/*
5.3 Value Mapping Functions

5.3.1 The ~predcounts~ operator

The value mapping function handles three phases:
   OPEN   to prepare the operator for using
   REQUEST   to return the result stream step by step (one tuple at one call)
   CLOSE   to clean up the function environment

At the OPEN phase the function allocates temporary memory, reads the input
stream completly, counts up and stores the counters for predicate combinations,
deletes the tuples of input stream and construct the type of the result tuples.
At the REQUEST phase it returns the result tuples (pattern, count) step by
step. At the CLOSE phase it deallocates temporary memory.

*/

int predcounts_vm(Word* args, Word& result, int message,
              Word& local, Supplier s)
{
   // get local data (only useful in phases REQUEST or CLOSE
   PredcountsLocalData*
     tempData = static_cast<PredcountsLocalData*>( local.addr );

  switch (message)
  {
      case OPEN : {

      // local==0 means: there was an error occured in OPEN phase
      local = SetWord(Address(0));

      // allocate temporary memory to store local data
      try {
        tempData = new PredcountsLocalData;
      } catch (bad_alloc&) {
        cerr << "ERROR: konnte Speicher " <<
          sizeof(PredcountsLocalData) <<
          " fuer PredcountsLocalData" <<
          " nicht allokieren" << endl;
        return 1;
      }

      // get the predicate list
      Supplier predicateList = args[1].addr;
      int predicateCount = qp->GetNoSons(predicateList);

      // try to allocate memory for counters
      bool ok = tempData->allocate(predicateCount);
      if (!ok) {
         return 1; // if return codes will be evaluated
          // replace 1 by the code of critical errors
      }

      // construct type of result tuples
      try {
        tempData->resultTupleType =
          new TupleType( nl->Second( GetTupleResultType(s) ));
      } catch (bad_alloc&) {
        cerr << "ERROR: konnte Speicher " <<
          sizeof(TupleType) <<
          " fuer TupleType" <<
          " nicht allokieren" << endl;
        return 1;// if return codes will be evaluated
          // replace 1 by the code of critical errors
      }

      // allocate memory to store structures for handling
      // mapping functions itself and their parameters
      Supplier *funStructure = 0;
      try {
        funStructure = new Supplier[predicateCount];
      } catch (bad_alloc&) {
         cerr << "ERROR: konnte Speicher " <<
            (predicateCount*sizeof(Supplier)) <<
            " Bytes nicht allokieren (predicateCount=" << predicateCount <<
            " sizeof(Supplier)=" << sizeof(Supplier) << ")" << endl;
         return 1; // if return codes will be evaluated
          // replace 1 by the code of critical errors
      }
      ArgVectorPointer *funArguments = 0;
      try {
        funArguments = new ArgVectorPointer[predicateCount];
      } catch (bad_alloc&) {
         cerr << "ERROR: konnte Speicher " <<
            (predicateCount*sizeof(ArgVectorPointer)) <<
            " Bytes nicht allokieren (predicateCount=" << predicateCount <<
            " sizeof(ArgVectorPointer)=" << sizeof(ArgVectorPointer) <<
            ")" << endl;
         return 1; // if return codes will be evaluated
          // replace 1 by the code of critical errors
      }

      // initialize array which stores structures for
      // handling mapping functions itself and their parameters
      for (int predicateNumber=0; predicateNumber<predicateCount;
         predicateNumber++) {
         Supplier predicate = qp->GetSupplier(predicateList, predicateNumber);
         funStructure[predicateNumber] = qp->GetSupplier(predicate, 1);
         funArguments[predicateNumber] =
         qp->Argument(funStructure[predicateNumber]);
      }

      // open input stream
      qp->Open(args[0].addr);
      // read all tuples of the input stream
      Word elem;
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) )
      {
         // check that only maxSaveEvaluableRows are read
         if ( !tempData->checkOverFlow() )
            return 1; // if return codes will be evaluated
              // replace 1 by the code of critical errors
              // ALTERNATIVELY: It's possible to use just a warning
              // warning here - SKIPping increasing the counter and
              // continue with the next tuple of input stream.
              // In that case the result of predcounts is
              // no longer exactly, but this way could be
              // more useful than aborting the query.

         // some local variables
         int resultIndex = 0;
         Tuple* currentTuple = static_cast<Tuple*>( elem.addr );

         // for each predicate: calculate the result
         // of the predicate for the current tuple
         // and set the corresponding bit in resultIndex
         for (int predNumber=0; predNumber<predicateCount; predNumber++) {

            // set the tuple to be used by evaluation and evaluate predicate
            (*(funArguments[predNumber]))[0] = elem;
       Word funResult;
            qp->Request( funStructure[predNumber], funResult);
            if (((Attribute*)funResult.addr)->IsDefined()) {
               if (((CcBool*)funResult.addr)->GetBoolval()) {
                  // if the predicate is true for the tuple,
                  // set the corresponding bit
                  resultIndex = (1 << predNumber) | resultIndex;
               }
            } else {
               //??? Was ist das fuer ein Fall?
            }
         }

         // increase the counter which reflects the set of met predicates
         (tempData->resultCounters[resultIndex])++;

         // delete the current tuple
         currentTuple->DeleteIfAllowed();

         // get the next tuple
         qp->Request(args[0].addr, elem);
      }
      // close input stream
      qp->Close(args[0].addr);

      // deallocate memory
      if (funStructure) delete [] funStructure;
      if (funArguments) delete [] funArguments;

      // set local only if this point will be reached
      local = SetWord( tempData );
      return 0;
   }

   case REQUEST : {

      // for savety reasons
      if (tempData==0) {
         cerr << "ERROR: no local data are found in REQUEST phase "
           << "- abort" << endl;
         return CANCEL; // if return codes will be evaluated
           // replace 1 by the code of critical errors
      }

      // if no more result tuples are left - finish REQUEST phase
      if (tempData->predicateCombinations==0) {
         return CANCEL;
      }
      tempData->predicateCombinations--;

      // create a new tuple
      Tuple* newTuple = 0;
      try {
        newTuple = new Tuple( tempData->resultTupleType );
      } catch (bad_alloc&) {
         cerr << "ERROR: konnte Speicher " <<
            sizeof(Tuple) << " Bytes nicht allokieren" <<
            endl;
         return 1; // if return codes will be evaluated
          // replace 1 by the code of critical errors
      }
      //newTuple->IncReference(); ??? wird nicht noetig sein - oder doch ?

      // put values into new tuple
      CcInt* attr = 0;
      try {
        attr = new CcInt(true, tempData->predicateCombinations);
      } catch (bad_alloc&) {
         cerr << "ERROR: konnte Speicher " << sizeof(CcInt)
           << " Bytes nicht allokieren" << endl;
         return 1; // if return codes will be evaluated
          // replace 1 by the code of critical errors
      }
      // ??? korrekt? deallocierung erfolgt durch anderen code?
      newTuple->PutAttribute( 0, attr );

      attr = 0;
      try {
        attr = new CcInt(true,
          (tempData->resultCounters)[tempData->predicateCombinations]);
      } catch (bad_alloc&) {
         cerr << "ERROR: konnte Speicher " << sizeof(CcInt)
           << " Bytes nicht allokieren" << endl;
         return 1; // if return codes will be evaluated
          // replace 1 by the code of critical errors
      }
      // korrekt?
      newTuple->PutAttribute( 1,  attr);

      // return the tuple
      result = SetWord(newTuple);
      return YIELD;
   }

   case CLOSE: {

      if (tempData) {
        delete tempData;
      }
      local = SetWord(Address(0));
      return 0;
   }

   default: {

     // this point should never be reached, if so return an error
     cerr << "ERROR: this point should never be reached" << endl;
     return 1; // if return codes will be evaluated
       // replace 1 by the code of critical errors
   }
   } // end of switch
}

/*

4.4 Operator Descriptions

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/
struct predcountsInfo : OperatorInfo {

  predcountsInfo()
  {
    name      = "predcounts";
    signature = "stream(tuple(y)) x ( tuple(y) -> bool, ..., tuple(y) "
       "-> bool ) -> stream(tuple((Atom int)(Counter int)))";
    syntax    = " predcounts [ funlist ]";
    meaning   = "returns for each possible evaluation subset (called atoms) "
       "of the function list the count of tuples "
       "which exactly met these. An evaluation subset is encoded as "
       "integer in binary representation, e.g. a pattern value of "
       "\"101\" = 5 in the result stream indicates the number of "
       "tuples for which the first and third predicate is evaluated "
       " to TRUE and the second is evaluated to FALSE.";

  }

}; // Don't forget the semicolon here. Otherwise the compiler
   // returns strange error messages



/*

5 Implementation of the Algebra Class

*/

class OptAuxAlgebra : public Algebra
{
 public:
  OptAuxAlgebra() : Algebra()
  {

/*
5.3 Registration of Operators

*/

    AddOperator( predcountsInfo(), predcounts_vm, predcounts_tm );

  }
  ~OptAuxAlgebra() {};
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
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>".

To link the algebra together with the system you must create an
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/

} // end of namespace ~optaux~

extern "C"
Algebra*
InitializeOptAuxAlgebra( NestedList* nlRef,
                         QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  // The C++ scope-operator :: must be used to qualify the full name
  return new optaux::OptAuxAlgebra;
}

