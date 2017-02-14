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

*/

#include "BPTreeOperators.h"

#include <cstring>
#include <fstream>
#include <iostream>

#include "Algebra.h"
#include "AlgebraManager.h"
#include "Attribute.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "Stream.h"
#include "TupleIdentifier.h"
#include "TypeConstructor.h"

#include "BPTreeNode.h"
#include "BPTreeSearchEnumerator.h"
#include "TreeHeaderMarker.h"

//#define DEBUG

using listutils::isTupleStream;
using std::memcpy;
using namespace std;

extern AlgebraManager *am;
extern NestedList* nl;
extern QueryProcessor* qp;

namespace fialgebra {

// Standard Cache-Groesse in Seiten
size_t BPTreeOperators::defaultCacheSize = 1000;

Operator& BPTreeOperators::GetCreatefbtreeOperator()
{
  return createfbtreeOp;
}
Operator& BPTreeOperators::GetInsertfbtreeOperator()
{
  return insertfbtreeOp;
}
Operator& BPTreeOperators::GetDeletefbtreeOperator()
{
  return deletefbtreeOp;
}
Operator& BPTreeOperators::GetRebuildfbtreeOperator()
{
  return rebuildfbtreeOp;
}
Operator& BPTreeOperators::GetBulkloadfbtreeOperator()
{
  return bulkloadfbtreeOp;
}

//
// Info siehe *.h
//
// Operators
//
// frange
Operator& BPTreeOperators::GetFrangeOperator() {
    return frangeOp;
}
// fleftrange
Operator& BPTreeOperators::GetFleftrangeOperator() {
    return fleftrangeOp;
}
// frightrange
Operator& BPTreeOperators::GetFrightrangeOperator() {
    return frightrangeOp;
}
// fexactmatch
Operator& BPTreeOperators::GetFexactmatchOperator() {
    return fexactmatchOp;
}
//
// OperatorSpec
//
// frange
OperatorSpec BPTreeOperators::frangeOperatorSpec = OperatorSpec(
    "{text , string} × T × T -> stream( tid )",
    "_ frange[_, _]",
    "Searches an B+-Tree file for values in an closed interval",
    "query 'index.bin' frange['A', 'B'] strassen gettuples consume"
);
// fleftrange
OperatorSpec BPTreeOperators::fleftrangeOperatorSpec = OperatorSpec(
    "{text , string} × T -> stream(tid)",
    "_ fleftrange[_]",
    "Searches an B+-Tree file for values smaller/equal to the parameter",
    "query 'index.bin' fleftrange['A'] strassen gettuples consume"
);
// frightrange
OperatorSpec BPTreeOperators::frightrangeOperatorSpec = OperatorSpec(
    "{text , string} × T -> stream(tid)",
    "_ frightrange[_]",
    "Searche an B+-Tree file for values greater/equal to the parameter",
    "query 'index.bin' frightrange['A'] strassen gettuples consume"
);
// fexactmatch
OperatorSpec BPTreeOperators::fexactmatchOperatorSpec = OperatorSpec(
    "{text , string} × T -> stream(tid)",
    "_ fexactmatch[_]",
    "Searches an B+-Tree for values equal to the parameter",
    "query 'index.bin' fexactmatch['A'] strassen gettuples consume"
);
//
// Type Mapping
//
// frange
ListExpr BPTreeOperators::FrangeTM( ListExpr args ) {
    // args: ((string "fileindex1.bin") (int 100) (int 200))

    int searchType = 1;
    return GenericFSearchTM( searchType, args );
}
// fleftrange
ListExpr BPTreeOperators::FleftrangeTM( ListExpr args ) {
    // args : ((string "fileindex1.bin") (int 20))

    int searchType = 2;
    return GenericFSearchTM( searchType, args );
}
// frightrange
ListExpr BPTreeOperators::FrightrangeTM( ListExpr args ) {
    // args : ((string "fileindex1.bin") (int 20))

    int searchType = 3;
    return GenericFSearchTM( searchType, args );
}
// fexactmatch
ListExpr BPTreeOperators::FexactmatchTM( ListExpr args ) {
    // args : ((string "fileindex1.bin") (int 20))

    int searchType = 4;
    return GenericFSearchTM( searchType, args );
}
// Generic f*[range|exactmatch] Type Mapping
ListExpr BPTreeOperators::GenericFSearchTM( int searchType, ListExpr args ) {
    // searchType
    //  1 : frange
    //  2 : fleftrange
    //  3 : frightrange
    //  4 : fexactmatch
    int frange      = 1;
    int fleftrange  = 2;
    int frightrange = 3;
    int fexactmatch = 4;

    // args
    //  frange      : ((string "fileindex1.bin") (int 100) (int 200))
    //  fleftrange  : ((string "fileindex1.bin") (int 100))
    //  frightrange : ((string "fileindex1.bin") (int 100))
    //  fexactmatch : ((string "fileindex1.bin") (int 100))

    bool hasMinValue = true;
    bool hasMaxValue = true;
    // fleftrange hat keinen Min-Wert
    if ( searchType == fleftrange ) hasMinValue = false;
    // frightrange hat keinen Max-Wert
    if ( searchType == frightrange ) hasMaxValue = false;

    NList type( args );
    //
    // Laenge
    // frange-Operator braucht 3 Argumente
    if ( searchType == frange && !type.hasLength( 3 ) )
        return NList::typeError( "Expecting three arguments." );
    // fleftrange-, frightrange-, fexactmatch-Operator brauchen nur 2 Argumente
    if ( searchType != frange && !type.hasLength( 2 ) )
        return NList::typeError( "Expecting two arguments." );
    //
    // Type checks
    //
    NList file = type.first();  // (string "fileindex1.bin")
    NList min;
    NList max;
    //
    // Abhaengig vom Operator sind min und max anders belegt
    if ( searchType == frange ) {
        // frange: min und max gegeben
        min = type.second();
        max = type.third();
    } else if ( searchType == fleftrange ) {
        // fleftrange: nur max gegeben, min ist dummy (int 0)
        min = NList(
          nl->TwoElemList(
            nl->SymbolAtom( "int" ),
            nl->IntAtom( 0 ) ) );
        max = type.second();
    } else if ( searchType == frightrange ) {
        // frightrange: min ist gegeben, max ist dummy (int 0)
        min = type.second();
        max = NList(
          nl->TwoElemList(
            nl->SymbolAtom( "int" ),
            nl->IntAtom( 0 ) ) );
    } else if ( searchType == fexactmatch ) {
        // fexactmatch: nur ein Wert ist gegeben, min und max sind identisch
        min = type.second();
        max = type.second();
    } else {
        return NList::typeError( "internal error" );
    } // else

    // Alle Paramter muessen eine Laenge von 2 haben (type value)
    if ( !file.hasLength( 2 ) ||
         !min.hasLength( 2 ) ||
         !max.hasLength( 2 ) )
        return NList::typeError( "internal error" );

    // file : string|text
    if ( !CcString::checkType( file.first().listExpr() ) &&
         !FText::checkType( file.first().listExpr() ) )
        return NList::typeError(
          "String or Text in first argument expected." );

    // Types von min und max
    ListExpr minType = min.first().listExpr();
    ListExpr maxType = max.first().listExpr();

    // Indexfile name
    string fileName = file.second().str();

    // Baum pruefen: Marker, Algebra- und Type-ID
    bool   correctTree = true;
    size_t algId  = 0;
    size_t typeId = 0;
    size_t rootId = 0;
    //
    BPTree* tree = BPTree::Open( fileName.c_str(), 1 );
    // TreeHeaderMarker.BPlusTree
    if ( tree->GetHeader().GetMarker() != BPlusTree )
        correctTree = false;
    // Header-Werte koennen nur gelesen werden, wenn
    // der Baum den korrekten Typ hat
    if ( correctTree ) {
        algId  = tree->GetHeader().GetAlgebraID();
        typeId = tree->GetHeader().GetTypeID();
        rootId = tree->GetHeader().GetRoot();
    } // if
    // cleanup
    delete tree;
    //
    // Baum im file ist kein B+Tree
    if ( !correctTree )
        return NList::typeError( "File contains no B+Tree" );
    // Algebra- und Type-Id pruefen
    ListExpr errorInfo;
    if ( hasMinValue && !am->TypeCheck( algId, typeId, minType, errorInfo ) )
        return NList::typeError( "Wrong type of min value" );
    if ( hasMaxValue && !am->TypeCheck( algId, typeId, maxType, errorInfo ) )
        return NList::typeError( "Wrong type of max value" );
    //
    // Wenn die RootNode-ID 0 ist, ist der Baum leer
    if ( rootId == 0 ) return NList::typeError( "BPTree in is empty" );


    // Result
    NList append;
    // Bei fleftrange oder frightrange fuegen wir einen
    // Dummy ein, damit die Indexe von args[] im VM passen.
    if ( searchType == fleftrange || searchType == frightrange )
      append.append( nl->IntAtom( 0 ) ); // dummy
    // Bei fexactmatch fuegen wir den (einzigen) Wert
    // nochmal ein.
    if ( searchType == fexactmatch )
      append.append( min.second() ); // min = max

    // (b b i i)
    append.append( nl->BoolAtom( hasMinValue ) );
    append.append( nl->BoolAtom( hasMaxValue ) );
    append.append( nl->IntAtom( (int)algId ) );
    append.append( nl->IntAtom( (int)typeId ) );
    //
    ListExpr res = NList(
      // (APPEND)
      NList( Symbol::APPEND() ),
      // append
      append,
      // (stream tid)
      nl->TwoElemList(
        nl->SymbolAtom( Stream<TupleIdentifier>::BasicType() ),
        nl->SymbolAtom( TupleIdentifier::BasicType() ) )
    ).listExpr();

    return res;
}
//
// Value Mapping
//
int BPTreeOperators::FrangeVM( Word* args, Word& result,
    int message, Word& local, Supplier s ) {

    // Jetzt wird's interessant: Abhaengig vom Operator
    // enthaellt args[] unterschiedliche Werte.
    //
    // args[0] : string (filename)
    //
    // args[1]
    //   hasMinValue : minValue
    //  !hasMinValue : maxValue
    //
    // args[2]
    //  hasMinValue && hasMaxValue : maxValue
    //
    // args[3] : hasMinValue
    // args[4] : hasMaxValue
    // args[5] : Algebra-Id
    // args[6] : Type-Id

    // Durch das "spezielle" APPEND im Type Mapping bleiben die
    // Indexe fuer hasMinValue etc. gleich. Darum koennen wir fuer
    // alle Search-Operatoren das selbe Value Mapping verwenden.

    BPTreeSearchEnumerator* enumerator =
        static_cast<BPTreeSearchEnumerator*>( local.addr );

    switch( message ) {
        case OPEN: {
            if ( enumerator != 0 ) delete enumerator;

            string indexFile   = ((Attribute*)args[0].addr)->toText();
            bool   hasMinValue = StdTypes::GetBool( args[3].addr );
            bool   hasMaxValue = StdTypes::GetBool( args[4].addr );
            int    algId       = StdTypes::GetInt( args[5].addr );
            int    typeId      = StdTypes::GetInt( args[6].addr );

            // Castet Pointer in richtige Attribute
            ObjectCast cast = am->Cast( algId, typeId );
            // min, max
            Attribute* minValue = NULL;
            Attribute* maxValue = NULL;
            //
            // Wenn es einen Min-Value gibt, steht der immer am Index 1.
            if ( hasMinValue )
                minValue = (Attribute*)cast( args[1].addr );
            // Wenn es keinen Min-Wert, aber einen Max-Wert gibt,
            // steht der Max-Wert am Index 1.
            if ( !hasMinValue && hasMaxValue )
                maxValue = (Attribute*)cast( args[1].addr );
            // Wenn es Min- und Max-Werte gibt, steht der Max-Wert
            // am Index 2.
            if ( hasMinValue && hasMaxValue )
                maxValue = (Attribute*)cast( args[2].addr );

            // Wenn max < min tauschen wir einfach die Werte
            if ( hasMinValue &&
                 hasMaxValue &&
                 maxValue->Compare( minValue ) < 0 ) {
                Attribute* tmp = maxValue;
                maxValue = minValue;
                minValue = tmp;
            } // if

            // Baum erzeugen und Suche beginnen
            // Beim Suchen wird kein Cache genutzt
            BPTree* tree = BPTree::Open( indexFile.c_str(), 0 );
            enumerator = tree->SearchKeys( minValue, maxValue );

            local.addr = enumerator;

            #ifdef DEBUG

            cout << "Open B+Tree     : " <<
              indexFile << endl;
            cout << "Tree height     : " <<
              tree->GetHeight() << endl;
            cout << "Tree page size  : " <<
              tree->GetHeader().GetPageSize() << endl;
            cout << "Tree value size : " <<
              tree->GetHeader().GetValueSize() << endl;
            
            cout << endl;
            cout << "Tree:" << endl;
            cout << tree->ToString() << endl;

            #endif

            return 0;
        } // case OPEN
        case REQUEST: {
            if ( enumerator->MoveNext() ) {
                // Enumerator enthaellt weitere Elemente
                size_t id = enumerator->GetCurrentId();

                TupleIdentifier* tid = new TupleIdentifier( true, id );
                result.addr = tid;
                return YIELD;
            } else {
                // Ende des Enumerators erreicht
                result.addr = 0;
                return CANCEL;
            } // else
        } // case REQUEST
        case CLOSE: {
            if ( enumerator != 0 ) {
                BPTree* tree = enumerator->GetTree();
                // Baum und Enumerator loeschen
                delete enumerator;
                enumerator = NULL;

                delete tree;
                tree = NULL;

                local.addr = 0;
            } // if

            return 0;
        } // case CLOSE
        default: {
            // should never happen
            return -1;
        } // default
    } // switch
}
//
// Operators
//
// frange
Operator BPTreeOperators::frangeOp = Operator(
  "frange",
  frangeOperatorSpec.getStr(),
  FrangeVM,
  Operator::SimpleSelect,
  FrangeTM
);
// fleftrange
Operator BPTreeOperators::fleftrangeOp = Operator(
  "fleftrange",
  fleftrangeOperatorSpec.getStr(),
  FrangeVM,
  Operator::SimpleSelect,
  FleftrangeTM
);
// frightrange
Operator BPTreeOperators::frightrangeOp = Operator(
  "frightrange",
  frightrangeOperatorSpec.getStr(),
  FrangeVM,
  Operator::SimpleSelect,
  FrightrangeTM
);
// fexactmatch
Operator BPTreeOperators::fexactmatchOp = Operator(
  "fexactmatch",
  fexactmatchOperatorSpec.getStr(),
  FrangeVM,
  Operator::SimpleSelect,
  FexactmatchTM
);

Operator BPTreeOperators::createfbtreeOp = CreateCreatefbtreeOp();
Operator BPTreeOperators::insertfbtreeOp = CreateInsertfbtreeOp();
Operator BPTreeOperators::deletefbtreeOp = CreateDeletefbtreeOp();
Operator BPTreeOperators::bulkloadfbtreeOp = CreateBulkloadfbtreeOp();
Operator BPTreeOperators::rebuildfbtreeOp = CreateRebuildfbtreeOp();

Operator BPTreeOperators::CreateCreatefbtreeOp(){
  OperatorSpec spec(
    "stream(tuple(X)) x {string, text} x Ident x Ident -> stream(tuple(X))",
    "_createfbtree[_,_,_]",
    "creates a file based B+-Tree index",
    "query straßen feed addid createfbtree"
    "['straßen_name.bin', Name, TID] count");

  Operator op("createfbtree",
              spec.getStr(),
              CreatefbtreeVM,
              Operator::SimpleSelect,
              CreatefbtreeTM);

  op.SetUsesArgsInTypeMapping();

  return op;
}

Operator BPTreeOperators::CreateInsertfbtreeOp(){
  OperatorSpec spec(
    "stream(tuple(X)) x {string, text} x Ident x Ident -> stream(tuple(X))",
    "_insertfbtree[_,_,_]",
    "inserts values into a file based B+-Tree index",
    "query straßen feed addid insertfbtree"
    "['straßen_name.bin', Name, TID] count");

  Operator op("insertfbtree",
              spec.getStr(),
              InsertfbtreeVM,
              Operator::SimpleSelect,
              InsertfbtreeTM);

  op.SetUsesArgsInTypeMapping();

  return op;
}

Operator BPTreeOperators::CreateDeletefbtreeOp(){
  OperatorSpec spec(
    "stream(tuple(X)) x {string, text} x Ident x Ident -> stream(tuple(X))",
    "_deletefbtree[_,_,_]",
    "removes values from a file based B+-Tree index",
    "query straßen feed addid deletefbtree"
    "['straßen_name.bin', Name, TID] count");

  Operator op("deletefbtree",
              spec.getStr(),
              DeletefbtreeVM,
              Operator::SimpleSelect,
              InsertfbtreeTM);

  op.SetUsesArgsInTypeMapping();

  return op;
}

Operator BPTreeOperators::CreateRebuildfbtreeOp(){
  OperatorSpec spec(
    "{string, text} x {string, text} -> bool ",
    "rebuildfbtree(_,_)",
    "rebuilds a B+-Tree",
    "query rebuildfbtree('straßen_name.bin', 'straßen_name_neu.bin')");

  Operator op("rebuildfbtree",
              spec.getStr(),
              RebuildfbtreeVM,
              Operator::SimpleSelect,
              RebuildfbtreeTM);

  op.SetUsesArgsInTypeMapping();

  return op;
}

Operator BPTreeOperators::CreateBulkloadfbtreeOp(){
  OperatorSpec spec(
    "stream(tuple(X)) x {string, text} x Ident x Ident -> stream(tuple(X))",
    "_bulkloadfbtree[_,_,_]",
    "creates a file based B+-Tree index with bulkload",
    "query straßen feed addid bulkloadfbtree"
    "['straßen_name.bin', Name, TID] count");

  Operator op("bulkloadfbtree",
              spec.getStr(),
              BulkloadfbtreeVM,
              Operator::SimpleSelect,
              CreatefbtreeTM);

  op.SetUsesArgsInTypeMapping();

  return op;
}

ListExpr BPTreeOperators::CreatefbtreeTM(ListExpr args){
  NList type(args);
  if (!type.hasLength(4)
      && !(type.hasLength(5)
           && CcInt::checkType(type.fifth().first().listExpr())))
  {
    return NList::typeError("Expecting four arguments.");
  }

  if (!isTupleStream(type.first().first().listExpr()))
  {
    return NList::typeError("Error in first argument! "
                            "Stream of tuples expected.");
  }

  NList second = type.second();
  if (!CcString::checkType(second.first().listExpr())
      && !FText::checkType(second.first().listExpr()))
  {
    return NList::typeError("Error in second argument! "
                            "String or text expected!");
  }

  ifstream file(second.second().str());
  if (file.good()){
    return NList::typeError("Error in second argument! "
                            "File allready exists!");
  }
  file.close();

  NList third = type.third();
  if (!third.first().isSymbol())
  {
    return NList::typeError("Error in third argument! "
                            "Attribute name expected. "
                            "Perhaps the attribute's name may be the name "
                            "of a Secondo object!");
  }
  string indexAttrName = third.first().str();

  ListExpr attributeList = type.first().first().second().second().listExpr(),
    attrtype = nl->Empty();

  int indexAttrIndex = FindAttribute(attributeList, indexAttrName, attrtype);
  indexAttrIndex--;
  if (indexAttrIndex < 0)
  {
    return NList::typeError("Error in third argument!"
                            "Attribute name '" + indexAttrName +
                            "' not found!");
  }

  int algebraId,
      typeId;
  string typeName;

  ListExpr indexAttrTypeInfo = nl->Second(nl->Nth(indexAttrIndex + 1,
                                                  attributeList));

  if (!SecondoSystem::GetCatalog()->LookUpTypeExpr(indexAttrTypeInfo, typeName,
                                                   algebraId, typeId)){
    return NList::typeError("Error in first argument!"
                            "Failed to determine algebraId and typeId of "
                            "indexed attribute!");
  }

  ObjectCreation creationFunction = am->CreateObj(algebraId, typeId);
  Attribute* attributeInstance =
    (Attribute*)creationFunction(indexAttrTypeInfo).addr;

  if (attributeInstance->NumOfFLOBs() > 0){
    attributeInstance->DeleteIfAllowed();
    return NList::typeError("Error in first argument! "
                            "Indexed attribute contains FLOBs!");
  }

  attributeInstance->DeleteIfAllowed();
  attributeInstance = NULL;

  NList fourth = type.fourth();
  if (!fourth.first().isSymbol())
  {
    return NList::typeError("Error in fourth argument! "
                            "Attribute name expected. "
                            "Perhaps the attribute's name may be the name "
                            "of a Secondo object!");
  }
  string idAttrName = fourth.first().str();
  attrtype = nl->Empty();

  int idAttrIndex = FindAttribute(attributeList, idAttrName, attrtype);
  idAttrIndex--;
  if (idAttrIndex < 0)
  {
    return NList::typeError("Error in fourth argument!"
                            "Attribute name '" + indexAttrName +
                            "' not found!");
  }

  if (!TupleIdentifier::checkType(attrtype)){
    return NList::typeError("Error in fourth argument!"
                            "Attribute '" + indexAttrName +
                            "' isn't of type TupleIdentifier!");
  }

  //On my machine this was close to the optimum
  int cacheSize = 200;

  if (type.hasLength(5) && CcInt::checkType(type.fifth().first().listExpr())){
    cacheSize = type.fifth().second().intval();

    return NList(NList(Symbol::APPEND()),
               nl->FourElemList(nl->IntAtom(indexAttrIndex),
                                nl->IntAtom(idAttrIndex),
                                nl->IntAtom(algebraId),
                                nl->IntAtom(typeId)),
               type.first().first().listExpr()).listExpr();
  }

  return NList(NList(Symbol::APPEND()),
               nl->FiveElemList(nl->IntAtom(cacheSize),
                                nl->IntAtom(indexAttrIndex),
                                nl->IntAtom(idAttrIndex),
                                nl->IntAtom(algebraId),
                                nl->IntAtom(typeId)),
               type.first().first().listExpr()).listExpr();
}

ListExpr BPTreeOperators::InsertfbtreeTM(ListExpr args){
  NList type(args);
  if (!type.hasLength(4))
  {
    return NList::typeError("Expecting four arguments.");
  }

  if (!isTupleStream(type.first().first().listExpr()))
  {
    return NList::typeError("Error in first argument! "
                            "Stream of tuples expected.");
  }

  NList second = type.second();
  if (!CcString::checkType(second.first().listExpr())
      && !FText::checkType(second.first().listExpr()))
  {
    return NList::typeError("Error in second argument! "
                            "String or text expected!");
  }

  NList third = type.third();
  if (!third.first().isSymbol())
  {
    return NList::typeError("Error in third argument! "
                            "Attribute name expected. "
                            "Perhaps the attribute's name may be the name "
                            "of a Secondo object!");
  }
  string indexAttrName = third.first().str();

  ListExpr attributeList = type.first().first().second().second().listExpr(),
    attrtype = nl->Empty();

  int indexAttrIndex = FindAttribute(attributeList, indexAttrName, attrtype);
  indexAttrIndex--;
  if (indexAttrIndex < 0)
  {
    return NList::typeError("Error in third argument!"
                            "Attribute name '" + indexAttrName +
                            "' not found!");
  }

  int algebraId,
      typeId;
  string typeName;

  ListExpr indexAttrTypeInfo = nl->Second(nl->Nth(indexAttrIndex + 1,
                                                  attributeList));

  if (!SecondoSystem::GetCatalog()->LookUpTypeExpr(indexAttrTypeInfo, typeName,
                                                   algebraId, typeId)){
    return NList::typeError("Error in first argument!"
                            "Failed to determine algebraId and typeId of "
                            "indexed attribute!");
  }


  NList fourth = type.fourth();
  if (!fourth.first().isSymbol())
  {
    return NList::typeError("Error in fourth argument! "
                            "Attribute name expected. "
                            "Perhaps the attribute's name may be the name "
                            "of a Secondo object!");
  }
  string idAttrName = fourth.first().str();
  attrtype = nl->Empty();

  int idAttrIndex = FindAttribute(attributeList, idAttrName, attrtype);
  idAttrIndex--;
  if (idAttrIndex < 0)
  {
    return NList::typeError("Error in fourth argument!"
                            "Attribute name '" + indexAttrName +
                            "' not found!");
  }

  if (!TupleIdentifier::checkType(attrtype)){
    return NList::typeError("Error in fourth argument!"
                            "Attribute '" + indexAttrName +
                            "' isn't of type TupleIdentifier!");
  }

  BPTree* tree;
  try{
    tree = BPTree::Open(type.second().second().str().c_str(), 0);
  }
  catch (exception& e){
    return NList::typeError("Opening IndexFile failed! " + string(e.what()));
  }

  if (tree->GetAlgebraId() != algebraId
      || tree->GetTypeId() != typeId){
    delete(tree);
    return NList::typeError("The indexed types in stream and file "
                            "don't match!");
  }
  delete(tree);

  return NList(NList(Symbol::APPEND()),
               nl->TwoElemList(nl->IntAtom(indexAttrIndex),
                               nl->IntAtom(idAttrIndex)),
               type.first().first().listExpr()).listExpr();
}

ListExpr BPTreeOperators::RebuildfbtreeTM( ListExpr args ){
  string dat1;
  string dat2;
  string err = "string/text x string/text";
  if (!nl->HasLength( args, 2 )) {
    return listutils::typeError(err + ": wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  // Check first argument (name of source file)
  //if ( !( CcString::checkType(args.first().first())
  if ( !( CcString::checkType(nl->First(arg1))
          || FText::checkType(nl->First(arg1)))) {
    return NList::typeError(err + ": First argument is not string or text!");
  }
  if ( !( nl -> AtomType(nl->Second(arg1)) == StringType
          || nl -> AtomType(nl->Second(arg1)) == TextType) ) {
    return NList::typeError(err + ": First argument is not string or text!");
  }
  if ( nl -> AtomType(nl->Second(arg1)) == StringType ) {
    dat1 = nl->StringValue(nl->Second(arg1));
  }
  if ( nl -> AtomType(nl->Second(arg1)) == TextType ) {
    dat1 = nl->Text2String(nl->Second(arg1));
  }
  // Check if source file exists and can be opened
  std::ifstream file01;
  file01.open(dat1.c_str());
  if (!file01) {
    return NList::typeError(err + ": First file couldn't be opened!");
  } else {
    file01.close();
  }
  // Check if source file contains B+-Tree
  BPTree* bpt1 = BPTree::Open(dat1.c_str(), 50);
  if ((bpt1->GetHeader()).GetMarker() != TreeHeaderMarker::BPlusTree) {
    return NList::typeError(err + ": First file doesn't contain a B+-Tree!");
  }
  delete bpt1;
  // Check second argument (name of target file)
  if ( !( CcString::checkType(nl->First(arg2))
          || FText::checkType(nl->First(arg2)))) {
    return NList::typeError(err + ": Second argument is not string or text!");
  }
  if ( !( nl -> AtomType(nl->Second(arg2)) == StringType
          || nl -> AtomType(nl->Second(arg2)) == TextType) ) {
    return NList::typeError(err + ": Second argument is not string or text!");
  }
  if ( nl -> AtomType(nl->Second(arg2)) == StringType ) {
    dat2 = nl->StringValue(nl->Second(arg2));
  }
  if ( nl -> AtomType(nl->Second(arg2)) == TextType ) {
    dat2 = nl->Text2String(nl->Second(arg2));
  }
  ListExpr resType = listutils::basicSymbol<CcBool>();
  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
    nl->TwoElemList(nl->TextAtom(dat1), nl->TextAtom(dat2)), resType );
}

int BPTreeOperators::CreatefbtreeVM(Word* args, Word& result, int message,
                                    Word& local, Supplier s){
  result.addr = NULL;

  switch(message)
  {
    case OPEN: {
      string path = ((Attribute*)args[1].addr)->toText();

      int algebraId = ((CcInt*)args[7].addr)->GetValue(),
          typeId = ((CcInt*)args[8].addr)->GetValue(),
          cacheSize = ((CcInt*)args[4].addr)->GetValue();

      BPTree* tree = NULL;
      try{
       tree = BPTree::Create(path.c_str(), algebraId, typeId, cacheSize);
      }
      catch (exception& e){
        cout << e.what() << '\n';
        local.addr = NULL;
        return CANCEL;
      }

      local.addr = new OperatorContext(tree, 0);

      qp->Open(args[0].addr);

      return 0;
    }
    case REQUEST: {
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        while(qp->Received(args[0].addr))
        {
          Tuple* tuple = (Tuple*)tupleWord.addr;
          int attributeIndex = ((CcInt*)args[5].addr)->GetValue(),
            idIndex = ((CcInt*)args[6].addr)->GetValue();

          Attribute* attribute = tuple->GetAttribute(attributeIndex);
          TupleId id =
            ((TupleIdentifier*)tuple->GetAttribute(idIndex))->GetTid();

          OperatorContext* context = (OperatorContext*)local.addr;

          if (attribute->NumOfFLOBs() > 0){
            context->skipped++;
          }
          else{
            context->tree->InsertValue(*attribute, id);
            //cout << context->tree->ToString() << '\n';
          }

          result.addr = tupleWord.addr;
          return YIELD;
        }
      }

      return CANCEL;
    }
    case CLOSE:
    {
      qp->Close(args[0].addr);

      if (local.addr != NULL)
      {
        OperatorContext* context = (OperatorContext*)local.addr;

        //cout << context->tree->ToString() << '\n';

        if (context->skipped > 0){
          cout << "createfbtree: " << context->skipped;
          cout << " elements skipped!\n";
        }

        delete(context);
        context = NULL;

        local.addr = NULL;
      }

      return 0;
    }
  }

  return 0;
}

int BPTreeOperators::DeletefbtreeVM (Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result.addr = NULL;

  switch(message)
  {
    case OPEN: {
      string path = ((Attribute*)args[1].addr)->toText();
      BPTree* tree = NULL;

      try{
       tree = BPTree::Open(path.c_str(), 50);
      }
      catch (exception& e){
        cout << e.what() << '\n';
        local.addr = NULL;
        return CANCEL;
      }

      local.addr = new OperatorContext(tree, 0);

      qp->Open(args[0].addr);

      return 0;
    }
    case REQUEST: {
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        while(qp->Received(args[0].addr))
        {
          Tuple* tuple = (Tuple*)tupleWord.addr;
          int attributeIndex = ((CcInt*)args[4].addr)->GetValue(),
            idIndex = ((CcInt*)args[5].addr)->GetValue();

          Attribute* attribute = tuple->GetAttribute(attributeIndex);
          TupleId id =
            ((TupleIdentifier*)tuple->GetAttribute(idIndex))->GetTid();

          OperatorContext* context = (OperatorContext*)local.addr;

          if (attribute->NumOfFLOBs() > 0
              || !context->tree->DeleteValue(*attribute, id))
          {
            context->skipped++;
          }

          result.addr = tupleWord.addr;
          return YIELD;
        }
      }

      return CANCEL;
    }
    case CLOSE:
    {
      qp->Close(args[0].addr);

      if (local.addr != NULL)
      {
        OperatorContext* context = (OperatorContext*)local.addr;

        //cout << context->tree->ToString();

        if (context->skipped > 0){
          cout << "deletefbtree: " << context->skipped;
          cout << " elements skipped!\n";
        }

        delete(context);
        context = NULL;

        local.addr = NULL;
      }

      return 0;
    }
  }

  return 0;
}

int BPTreeOperators::InsertfbtreeVM (Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result.addr = NULL;

  switch(message)
  {
    case OPEN: {
      string path = ((Attribute*)args[1].addr)->toText();
      BPTree* tree = NULL;
      try{
        tree = BPTree::Open(path.c_str(), 50);
      }
      catch (exception& e){
        cout << e.what() << '\n';
        local.addr = NULL;
        return CANCEL;
      }

      local.addr = new OperatorContext(tree, 0);

      qp->Open(args[0].addr);

      return 0;
    }
    case REQUEST: {
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        while(qp->Received(args[0].addr))
        {
          Tuple* tuple = (Tuple*)tupleWord.addr;
          int attributeIndex = ((CcInt*)args[4].addr)->GetValue(),
            idIndex = ((CcInt*)args[5].addr)->GetValue();

          Attribute* attribute = tuple->GetAttribute(attributeIndex);
          TupleId id =
            ((TupleIdentifier*)tuple->GetAttribute(idIndex))->GetTid();

          OperatorContext* context = (OperatorContext*)local.addr;

          if (attribute->NumOfFLOBs() > 0){
            context->skipped++;
          }
          else{
            context->tree->InsertValue(*attribute, id);
          }

          result.addr = tupleWord.addr;
          return YIELD;
        }
      }

      return CANCEL;
    }
    case CLOSE:
    {
      qp->Close(args[0].addr);

      if (local.addr != NULL)
      {
        OperatorContext* context = (OperatorContext*)local.addr;

        //cout << context->tree->ToString();

        if (context->skipped > 0){
          cout << "insertfbtree: " << context->skipped;
          cout << " elements skipped!\n";
        }

        delete(context);
        context = NULL;

        local.addr = NULL;
      }

      return 0;
    }
  }

  return 0;
}

int BPTreeOperators::BulkloadfbtreeVM (Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result.addr = NULL;

  switch(message)
  {
    case OPEN: {
      string path = ((Attribute*)args[1].addr)->toText();

      int algebraId = ((CcInt*)args[7].addr)->GetValue(),
          typeId = ((CcInt*)args[8].addr)->GetValue(),
          cacheSize = ((CcInt*)args[4].addr)->GetValue();

      BPTree* tree;
      try{
       tree = BPTree::Create(path.c_str(), algebraId, typeId, cacheSize);
      }
      catch (exception& e){
        cout << e.what() << '\n';
        local.addr = NULL;
        return CANCEL;
      }
      tree->StartBulkload();

      local.addr = new BulkloadContext(tree, NULL, 0);

      qp->Open(args[0].addr);

      return 0;
    }
    case REQUEST: {
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        while(qp->Received(args[0].addr))
        {
          Tuple* tuple = (Tuple*)tupleWord.addr;
          int attributeIndex = ((CcInt*)args[5].addr)->GetValue(),
            idIndex = ((CcInt*)args[6].addr)->GetValue();

          Attribute* attribute = tuple->GetAttribute(attributeIndex);
          TupleId id =
            ((TupleIdentifier*)tuple->GetAttribute(idIndex))->GetTid();

          BulkloadContext* context = (BulkloadContext*)local.addr;

          if (attribute->NumOfFLOBs() > 0
              || (context->prevAttr != NULL
                  && attribute->Compare(context->prevAttr) < 0)){
            context->skipped++;
          }
          else{
            context->tree->InsertBulkload(*attribute, id);

            if (context->prevAttr != NULL){
              context->prevAttr->DeleteIfAllowed();
            }

            context->prevAttr = attribute->Copy();
          }

          result.setAddr(tupleWord.addr);
          return YIELD;
        }
      }

      return CANCEL;
    }
    case CLOSE:
    {
      qp->Close(args[0].addr);

      if (local.addr != NULL)
      {
        BulkloadContext* context = (BulkloadContext*)local.addr;
        context->tree->EndBulkload();

        //CheckTree
        /*BPTreeSearchEnumerator* e = context->tree->SearchKeys(NULL, NULL);
        size_t index = 0;

        qp->Open(args[0].addr);

        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        while (qp->Received(args[0].addr)){
          Tuple* tuple = (Tuple*)tupleWord.addr;
          int attributeIndex = ((CcInt*)args[5].addr)->GetValue(),
            idIndex = ((CcInt*)args[6].addr)->GetValue();

          Attribute* attribute = tuple->GetAttribute(attributeIndex);
          TupleId id =
            ((TupleIdentifier*)tuple->GetAttribute(idIndex))->GetTid();

          if (!e->MoveNext()){
            cout << "Missing Entry at " << index << '\n';
            break;
          }
          else{
            if (attribute->Compare(&(e->GetCurrentValue())) != 0){
              cout << "Invalid Attribute at " << index << '\n';
            }

            if (id != e->GetCurrentId()){
              cout << "Invalid TupleId at " << index << '\n';
            }
          }

          index++;
          qp->Request(args[0].addr, tupleWord);
        }

        delete(e);

        qp->Close(args[0].addr);*/

        if (context->skipped > 0){
          cout << "bulkloadfbtree: " << context->skipped;
          cout << " elements skipped!\n";
        }

        delete(context);
        context = NULL;

        local.addr = NULL;
      }

      return 0;
    }
  }

  return 0;
}

int BPTreeOperators::RebuildfbtreeVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  // Parameters 2 and 3 are delivered from the type mapping function
  // in text format, params 0 and 1 can thus be neglected
  FText* pre1 = (FText*) args[2].addr;  // get the argument and cast it
  FText* pre2 = (FText*) args[3].addr;  // get the argument and cast it
  // Cast file names to string (and later to char* for ctor)
  string dat1 = (pre1->GetValue());
  string dat2 = (pre2->GetValue());
  std::cout << "dat1: " << dat1.c_str() << endl;
  std::cout << "dat2: " << dat2.c_str() << endl;
  // Create source tree from first file
  BPTree* bpt1 = BPTree::Open(dat1.c_str(), 50);
  bpt1->Rebuild(dat2.c_str());
  delete bpt1;
  // Delete source file
  //remove(dat1.c_str());
  result = qp->ResultStorage(s);       // use the result storage
  CcBool* b = static_cast<CcBool*> (result.addr); // cast the result
  b->Set(true, true);      // compute and set the result
  return 0;
}

BPTreeOperators::OperatorContext::OperatorContext(BPTree* tree, size_t skipped){
  this->tree = tree;
  this->skipped = skipped;
}

BPTreeOperators::OperatorContext::~OperatorContext(){
  if (tree != NULL){
    delete(tree);
    tree = NULL;
  }
}

BPTreeOperators::BulkloadContext::BulkloadContext(BPTree* tree,
                                                  Attribute* prevAttr,
                                                  size_t skipped)
: OperatorContext(tree, skipped){
  if (prevAttr != NULL){
    this->prevAttr = prevAttr->Copy();
  }
  else{
    this->prevAttr = NULL;
  }
}

BPTreeOperators::BulkloadContext::~BulkloadContext(){
  if (this->prevAttr != NULL){
    this->prevAttr->DeleteIfAllowed();
    this->prevAttr = NULL;
  }
}
}






















