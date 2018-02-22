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
//[->] [$\rightarrow$]
//[TOC] [\tableofcontents]
//[_] [\_]

FileIndexAlgebra

*/

#include <iostream>
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "Symbols.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "StandardTypes.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Algebras/MMRTree/MMRTreeAlgebra.h"
#include "MMRTree.h"
#include "RTreeNode.h"
#include "RTree.h"
#include "RTreeIterator.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include <fstream>
#include "Cache/LruCache.h"
#include "BPTree/BPTreeOperators.h"
#include <stdexcept>

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager *am;

using listutils::isTupleStream;
using std::exception;

using namespace std;

namespace fialgebra {
  
  // detects dimension of a tree
  // stored in a file
int checkDimension(const char* fileName){
  ifstream stream(fileName, ifstream::binary | ifstream::ate);
  if ( !stream.good() ) {
    stream.close();
    throw runtime_error( "File couldn't be opened: " + string(fileName) );
  }
  else {
    size_t pageSize = WinUnix::getPageSize(),
           length   = min<size_t>(pageSize, stream.tellg());

    stream.seekg(0, ios::beg);

    char* bytes = new char[length];

    stream.read( bytes, length );
    stream.close();
    
    RTreeHeader* header = new RTreeHeader(bytes, length, pageSize, 1);
    size_t dimension = header->GetDimension();
    delete header;
    
    return (int)dimension;
  } // else
}


/* supporting function: Checks the header
 * of an RTree. 
 * */


template<int dim>
bool checkHeader(string& fileName, string& errMsg){
  errMsg ="";
  
  /* open function also checks whether this is
   * a valid rtree which has been passed in the file*/
  
  RTree<dim>* rt1 = RTree<dim>::Open(fileName.c_str(), 1);
  size_t rootId = 0;


  rootId = rt1->GetHeader()->GetRoot();

  delete rt1;

  // Wenn die RootNode-ID 0 ist, ist der Baum leer
  if ( rootId == 0 ){
    errMsg= "RTree is empty" ;
    return false;
  }
  return true;
}

/*
There are no attributes in this algebra

Operators implementation

TypeMapping Functions

operator ~fwindowintersects~

*/


ListExpr fWindowIntersectTM(ListExpr args){
  // args: ((text 'ix_rtree.bin') (rect (rect (1 1 1 1))))

  //check number of arguments
  if (!nl->HasLength(args, 2))
    return listutils::typeError("wrong number of aruments");


  ListExpr fileArg = nl->First(args);
  //the list is coded as (<type> <query part>)
  if(!nl->HasLength(fileArg, 2)){
    return listutils::typeError("internal error");
  }
  //first arguments must be of a type string or text
  if (!CcString::checkType(nl->First(fileArg)) &&
      !FText::checkType(nl->First(fileArg))){
      return listutils::typeError("file name as argument expected");
     }

  ListExpr fn = nl->Second(fileArg);
  if(!((nl->AtomType(fn) == TextType)
    || (nl->AtomType(fn) == StringType))){
    return listutils::typeError("file name not constant");
  }

  // read file name
  string fileName;
  if(nl->AtomType(fn) == TextType){
    fileName = nl->Text2String(fn);
  }
  if(nl->AtomType(fn) == StringType){
    fileName = nl->StringValue(fn);
  }

  //second arguments must be of a type
  //Rectangle<dim> (dim={1, 2, 3, 4, 8})
  //or SPATIAL2D or SPATIAL3D
  ListExpr argRect = nl->Second(args);
  bool ok = false;
  string errMsg;

  if (listutils::isKind(nl->First(argRect), Kind::SPATIAL1D())){
     ok = checkHeader<1>(fileName, errMsg);
  }
  if (listutils::isKind(nl->First(argRect), Kind::SPATIAL2D())){
     ok = checkHeader<2>(fileName, errMsg);
  }
  if (listutils::isKind(nl->First(argRect), Kind::SPATIAL3D())){
     ok = checkHeader<3>(fileName, errMsg);
  }
  if (listutils::isKind(nl->First(argRect), Kind::SPATIAL4D())){
     ok = checkHeader<4>(fileName, errMsg);
  }
  if (listutils::isKind(nl->First(argRect), Kind::SPATIAL8D())){
     ok = checkHeader<8>(fileName, errMsg);
  }

  if(!ok){
    return listutils::typeError(errMsg);
    }

  return nl->TwoElemList(
    listutils::basicSymbol<Stream<TupleIdentifier> >(),
    listutils::basicSymbol<TupleIdentifier>() );
}


/*
 Operator createfrtree

*/

ListExpr createfrtreeTM(ListExpr args){
  string err = "stream(Tuple(X)) x string x a_i1 x a_i2 x int expected";

  if(!nl->HasLength(args,5)){
     return listutils::typeError(err);
  }

  if (!Stream<Tuple>::checkType(nl->First(args))){
    return listutils::typeError("Error in  first argument."
                                "Stream of tuples expected.");
  }

  if (!CcString::checkType(nl->Second(args)) &&
      !FText::checkType(nl->Second(args)))
    return listutils::typeError("Error in second argument. "
                                "string or text expected.");

  if(!listutils::isSymbol(nl->Third(args))){
     return listutils::typeError("Error in third argument. "
                                 "Attribute name expected.");
  }

  if(!listutils::isSymbol(nl->Fourth(args))){
     return listutils::typeError("Error in fourth argument. "
                                 "Attribute name expected");
  }

  if(!CcInt::checkType(nl->Fifth(args))){
    return listutils::typeError("Error in fifth argument. "
                                "int as argument expected.");
  }

  string name1 = nl->SymbolValue(nl->Third(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type1;
  int index1 = listutils::findAttribute(attrList, name1, type1);
  if(index1==0){
     return listutils::typeError("Attribute " + name1 + " unknown in tuple");
  }

  if(!listutils::isKind(type1, Kind::SPATIAL1D())
     && !listutils::isKind(type1, Kind::SPATIAL2D())
     && !listutils::isKind(type1, Kind::SPATIAL3D())
     && !listutils::isKind(type1, Kind::SPATIAL4D())
     && !listutils::isKind(type1, Kind::SPATIAL8D()))
  {
     return listutils::typeError("Attribute " + name1 + " not of kind" +
                                 "SPATIAL1D, SPATIAL2D, SPATIAL3D, SPATIAL4D " +
                                 "or SPATIAL8D");
  }

  string name2 = nl->SymbolValue(nl->Fourth(args));
  ListExpr type2;
  int index2 = listutils::findAttribute(attrList, name2, type2);
  if(index2==0){
     return listutils::typeError("Attribute " + name2 + " unknown in tuple");
  }
  if(!TupleIdentifier::checkType(type2) ){
      return listutils::typeError("Attribute " + name2 +
            " not of type " + TupleIdentifier::BasicType());
    }

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           nl->TwoElemList(nl->IntAtom(index1 - 1),
                                           nl->IntAtom(index2 - 1)),
                           nl->First(args));
}

//end of TM for Operator createfrtree//////////



//Operator: insertfrtree///////////////////////

ListExpr InsertfrtreeTM(ListExpr args){
  NList type(args);
  if (!type.hasLength(4))
  {
    return NList::typeError("Expecting four arguments.");
  }

  NList streamType = type.first().first();
  if (!isTupleStream(streamType.listExpr()))
  {
    return NList::typeError("Error in first argument! "
                            "Stream of tuples expected.");
  }

  NList pathType = type.second().first();
  bool isPathText;
  if (CcString::checkType(pathType.listExpr()))
  {
      isPathText = false;
  }
  else if (FText::checkType(pathType.listExpr()))
  {
    isPathText = true;
  }
  else
  {
    return NList::typeError("Error in second argument! "
                            "String or text expected!");
  }

  NList indexAttrNameType = type.third().first();
  if (!indexAttrNameType.isSymbol())
  {
    return NList::typeError("Error in third argument! "
                            "Attribute name expected. "
                            "Perhaps the attribute's name may be the name "
                            "of a Secondo object!");
  }
  string indexAttrName = indexAttrNameType.str();

  ListExpr attributeList = streamType.second().second().listExpr(),
           attrType = nl->Empty();

  int indexAttrIndex = FindAttribute(attributeList, indexAttrName, attrType);
  indexAttrIndex--;
  if (indexAttrIndex < 0)
  {
    return NList::typeError("Error in third argument!"
                            "Attribute name '" + indexAttrName +
                            "' not found!");
  }



  NList idAttrNameType = type.fourth().first();
  if (!idAttrNameType.isSymbol())
  {
    return NList::typeError("Error in fourth argument! "
                            "Attribute name expected. "
                            "Perhaps the attribute's name may be the name "
                            "of a Secondo object!");
  }
  string idAttrName = idAttrNameType.str();
  attrType = nl->Empty();

  int idAttrIndex = FindAttribute(attributeList, idAttrName, attrType);
  idAttrIndex--;
  if (idAttrIndex < 0)
  {
    return NList::typeError("Error in fourth argument!"
                            "Attribute name '" + indexAttrName +
                            "' not found!");
  }

  string path = type.second().second().str();
  char* pathCopy = new char[path.length() + 1];
  memcpy(pathCopy, path.c_str(), path.length() + 1);
  delete [] pathCopy;

  return NList(NList(Symbol::APPEND()),
               NList(NList(isPathText),
                     NList(indexAttrIndex),
                     NList(idAttrIndex)),
               streamType).listExpr();
}

//end of TM for Operator insertfrtree//////

//Operator deletefrtree/////////////////////////////////////////////////////
ListExpr deletefrtreeTM(ListExpr args){
  //identical to insertfrtree
  return InsertfrtreeTM(args);
}
//end of TM for Operator deletefrtree//////////

// TM for frebuildtree //////////////////


ListExpr rebuildfrtreeTM( ListExpr args )
{
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

  /*
  if (dat1.substr(dat1.size()-5, 4) != ".bin") {
    return NList::typeError(err + ": First argument hasn't extension .bin!");
  }
  */
  // Check if source file exists and can be opened
  std::ifstream file01;
  file01.open(dat1.c_str());
  if (!file01) {
    return NList::typeError(err + ": First file couldn't be opened!");
  } else {
    file01.close();
  }
  
  // Check if source file contains R-Tree
  // creation of a dummy tree with dim = 1
 /* RTree<1>* r1 = RTree<1>::OpenRebuild(dat1.c_str(), 50);
  if ((r1->GetHeader())->GetMarker() != '1') {
    return NList::typeError(err + ": First file doesn't contain an R-Tree!");
    }
  delete r1;*/
 
 
 /* Check dimension of the RTree
  * contained in the file that was passed.
  If the file does not contain an RTree 
  an error message will be returned*/
  const char * c = dat1.c_str();
  int dim = checkDimension(c);

  
  /* If the first file contains an RTree we will need
   * to check whether its dimension is actually 
   * covered*/
  if (dim < 1 || dim > 8) {
    return NList::typeError
    (err + ": First file doesn't contain a valid R-Tree!");
  }
  
    if (dim == 5 || dim == 6 || dim == 7) {
    return NList::typeError
    (err + ": First file doesn't contain a valid R-Tree!");
  }
  
  
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
//    dat2 = nl->SymbolValue(nl->Second(arg2));
    dat2 = nl->StringValue(nl->Second(arg2));
  }
  if ( nl -> AtomType(nl->Second(arg2)) == TextType ) {
    dat2 = nl->Text2String(nl->Second(arg2));
  }
  /*
  if (dat2.substr(dat2.size()-5, 4) != ".bin") {
    return NList::typeError(err + ": Second argument hasn't extension .bin!");
  }
  */
  ListExpr resType = listutils::basicSymbol<CcBool>();
  

  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
    nl->TwoElemList(nl->TextAtom(dat1), nl->TextAtom(dat2)), resType );  
  
}




//end of TM for Operator rebuildfrtree////////


// 2. ValueMapping

/*
Operator ~fwindowintersect~

*/
template <int dim>
int fWindowIntersectValMap ( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
   //define RTreeIterator*
   //it will used as localInfo-class
   RTreeIterator<dim>* rti = (RTreeIterator <dim>*)local.addr;
   
   switch (message) {
     case OPEN:{
       if (rti) delete rti;     
      
       //read file
       string fileName = ((Attribute*)args[0].addr)->toText();

       StandardSpatialAttribute<dim>* arg
       = (StandardSpatialAttribute<dim>*) args[1].addr;

       Rectangle<dim> rectArg = arg->BoundingBox();

       //create R-Tree and Iterator
       RTree<dim>* rtree = RTree<dim>::Open(fileName.c_str(), 0);
       rti = new RTreeIterator<dim>(rtree, rectArg);
       local.addr = rti;
       
       return 0;
     }// end of OPEN  

    case REQUEST:{
       size_t id = rti->Search();
    
       if (id != 0){
         //Falls Suche noch nicht zu Ende ist
         TupleIdentifier* tid = new TupleIdentifier( true, id );
         result.addr = tid;
         return YIELD;
       }   
       else{
         //Ende des Baums erreicht
         result.addr = 0;
         return CANCEL;
       }
     }//end of REQUEST

    case CLOSE:{
       if (rti){
         delete rti;
         local.addr = 0;
       }  
       return 0;
     }//end of CLOSE
   }//end of switch(message)
   //should never happen
   return -1;
}

//ValueMapping function: createfrtree

template <int dim>
int createfrtreeValMap ( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  Stream<Tuple> stream(args[0]);

  switch (message) {
    case OPEN:{
      // writing the file for the RTree
      string filename = ((Attribute*)args[1].addr)->toText();
      int min = ((CcInt*)args[4].addr)->GetValue();

      try{
        local.addr = RTree<dim>::Create(filename.c_str(), 512, min);
      }
      catch (exception& e){
        cerr << "Creation failed: " << e.what() << '\n';
        return CANCEL;
      }

      qp->Open(args[0].addr);
      break;
    }
    case REQUEST:{
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        if(qp->Received(args[0].addr)){
          int index1 = ((CcInt*)args[5].addr)->GetValue(),
              index2 = ((CcInt*) args[6].addr)->GetValue();

          Tuple* tuple = (Tuple*)tupleWord.addr;

          StandardSpatialAttribute<dim>* value =
            (StandardSpatialAttribute<dim>*)tuple->GetAttribute(index1);

          TupleIdentifier* id = (TupleIdentifier*)tuple->GetAttribute(index2);

          try{
            ((RTree<dim>*)local.addr)->Insert(value->BoundingBox(),
                                              (size_t)id->GetTid());
          }
          catch(exception& e){
            cerr << "Insertion failed: " << e.what() << '\n';
          }

          result = tupleWord;
          return YIELD;
        }
      }

      result.addr = NULL;
      return CANCEL;
    }
    case CLOSE:{
      qp->Close(args[0].addr);

      if (local.addr != NULL){

        delete((RTree<dim>*)local.addr);
        local.addr = NULL;
      }
      break;
    }
  }

  return 0;
}

// end of VM function for createfrtree //////////////////////////////////



// Value Mapping function for insertfrtree /////////////////////////////

template <int dim>
int insertfrtreeValMap(Word* args, Word& result, int message, Word& local,
                       Supplier s)
{
  Stream<Tuple> stream(args[0]);

  switch (message) {
    case OPEN:{
      string filename = ((Attribute*)args[1].addr)->toText();

      try{
        local.addr = RTree<dim>::Open(filename.c_str(), 50);
      }
      catch (exception& e){
        cerr << "Opening failed: " << e.what() << '\n';
        return CANCEL;
      }

      qp->Open(args[0].addr);
      break;
    }
    case REQUEST:{
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        if(qp->Received(args[0].addr)){
          int index1 = ((CcInt*)args[5].addr)->GetValue(),
              index2 = ((CcInt*) args[6].addr)->GetValue();

          Tuple* tuple = (Tuple*)tupleWord.addr;

          StandardSpatialAttribute<dim>* value =
            (StandardSpatialAttribute<dim>*)tuple->GetAttribute(index1);

          TupleIdentifier* id = (TupleIdentifier*)tuple->GetAttribute(index2);

          try{
            ((RTree<dim>*)local.addr)->Insert(value->BoundingBox(),
                                              (size_t)id->GetTid());
          }
          catch(exception& e){
            cerr << "Insertion failed: " << e.what() << '\n';
          }

          //cout << ((RTree<dim>*)local.addr)->ToString() << '\n';

          result = tupleWord;
          return YIELD;
        }
      }

      result.addr = NULL;
      return CANCEL;
    }
    case CLOSE:{
      qp->Close(args[0].addr);

      if (local.addr != NULL){
        delete((RTree<dim>*)local.addr);
        local.addr = NULL;
      }
      break;
    }
  }

  return 0;
}

// end of VM function for insertfrtree //////////////////////////////////

// Value Mapping function for deletefrtree /////////////////////////////
template <int dim>
int deletefrtreeValMap(Word* args, Word& result, int message, Word& local,
                       Supplier s)
{
  Stream<Tuple> stream(args[0]);

  switch (message) {
    case OPEN:{
      string filename = ((Attribute*)args[1].addr)->toText();

      try{
        local.addr = RTree<dim>::Open(filename.c_str(), 512);
      }
      catch (exception& e){
        cerr << "Opening failed: " << e.what() << '\n';
        return CANCEL;
      }

      qp->Open(args[0].addr);
      break;
    }
    case REQUEST:{
      if (local.addr != NULL){
        Word tupleWord(Address(NULL));
        qp->Request(args[0].addr, tupleWord);

        if(qp->Received(args[0].addr)){
          int index1 = ((CcInt*)args[5].addr)->GetValue(),
              index2 = ((CcInt*) args[6].addr)->GetValue();

          Tuple* tuple = (Tuple*)tupleWord.addr;

          StandardSpatialAttribute<dim>* value =
            (StandardSpatialAttribute<dim>*)tuple->GetAttribute(index1);

          TupleIdentifier* id = (TupleIdentifier*)tuple->GetAttribute(index2);

          try{
            ((RTree<dim>*)local.addr)->Delete(value->BoundingBox(),
                                              (size_t)id->GetTid());
          }
          catch(exception& e){
            cerr << "Deletion failed: " << e.what() << '\n';
          }

          //cout << ((RTree<dim>*)local.addr)->ToString() << '\n';

          result = tupleWord;
          return YIELD;
        }
      }

      result.addr = NULL;
      return CANCEL;
    }
    case CLOSE:{
      qp->Close(args[0].addr);

      if (local.addr != NULL){
        delete((RTree<dim>*)local.addr);
        local.addr = NULL;
      }
      break;
    }
  }

  return 0;
}
// end of VM function for deletefrtree //////////////////////////////////

// Value Mapping for rebuildfrtree

int rebuildfrtreeValMap (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  // Parameters 2 and 3 are delivered from the type mapping function
  // in text format, params 0 and 1 can thus be neglected
  FText* pre1 = (FText*) args[2].addr;  // get the argument and cast it
  FText* pre2 = (FText*) args[3].addr;  // get the argument and cast it
  // Cast file names to string (and later to char* for ctor)
  string dat1 = (pre1->GetValue()).c_str();
  string dat2 = (pre2->GetValue()).c_str();
  
  const char * c = dat1.c_str();
  
  int dim = checkDimension(c);
  
  if (dim == 1){
      RTree<1>* rtree = RTree<1>::Open(c, 512);
  rtree->RebuildR(dat2.c_str());
  delete rtree;    
  }
  
    if (dim == 2){
      RTree<2>* rtree = RTree<2>::Open(c, 512);
  rtree->RebuildR(dat2.c_str());
  delete rtree;    
  }
  
  if (dim == 3){
      RTree<3>* rtree = RTree<3>::Open(c, 512);
  rtree->RebuildR(dat2.c_str());
  delete rtree;    
  }
  
    if (dim == 4){
      RTree<4>* rtree = RTree<4>::Open(c, 512);
  rtree->RebuildR(dat2.c_str());
  delete rtree;    
  }
  
      if (dim == 8){
      RTree<8>* rtree = RTree<8>::Open(c, 512);
  rtree->RebuildR(dat2.c_str());
  delete rtree;    
  }
  
  
  
//  rtree->RebuildR(dat2.c_str());
//  delete rtree;
  // Delete source file
  //remove(dat1.c_str());
  result = qp->ResultStorage(s);       // use the result storage
  CcBool* b = static_cast<CcBool*> (result.addr); // cast the result
  b->Set(true, true);      // compute and set the result
  return 0;
}




// end of VM for rebuldfrtree


// 4. Selection Function

/////////// createfrtree /////////////////////////////////////////

int createfrtreeSelect (ListExpr args){
  //int index = nl->Sixth(args);
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  string name = nl->SymbolValue(nl->Third(args));
  ListExpr type;
  int index = listutils::findAttribute(attrList,name,type);
  assert(index > 0);

  //assert(index >0);
  if(listutils::isKind(type, Kind::SPATIAL1D())){
    return 0;
  }
  if(listutils::isKind(type, Kind::SPATIAL2D())){
    return 1;
  }
  if(listutils::isKind(type, Kind::SPATIAL3D())){
    return 2;
  }
  if(listutils::isKind(type, Kind::SPATIAL4D())){
    return 3;
  }
  if(listutils::isKind(type, Kind::SPATIAL8D())){
    return 4;
  }

  return -1;
}

/////////// end of selection function for createfrtree //////////


// 5. Arrays for ValueMapping


/////////// createfrtree /////////////////////////////////////////

ValueMapping createfrtreeVM[] = {
    createfrtreeValMap<1>,
    createfrtreeValMap<2>,
    createfrtreeValMap<3>,
    createfrtreeValMap<4>,
    createfrtreeValMap<8>
};

ValueMapping insertfrtreeVM[] = {
    insertfrtreeValMap<1>,
    insertfrtreeValMap<2>,
    insertfrtreeValMap<3>,
    insertfrtreeValMap<4>,
    insertfrtreeValMap<8>
};

ValueMapping deletefrtreeVM[] = {
    deletefrtreeValMap<1>,
    deletefrtreeValMap<2>,
    deletefrtreeValMap<3>,
    deletefrtreeValMap<4>,
    deletefrtreeValMap<8>
};


/////////// end of VM array for createfrtree ////////////////////


// 6. Operator Spec

OperatorSpec createfrtreeSpec(
  "stream(tuple(X))x{txt,str}xIdxIdxint->stream(tpl(X))",
  "createfrtree(_,_,_,_,_) ",
  "creates an Rtree file",
  "query createfrtree()"
  );

OperatorSpec insertfrtreeSpec(
  "stream(tuple(X))x{txt,str}xIdxId->stream(tpl(X))",
  "insertfrtree(_,_,_,_) ",
  "performs inserts into an Rtree file",
  "query insertfrtree()"
  );

OperatorSpec deletefrtreeSpec(
  "stream(tuple(X))x{txt,str}xIdxId->stream(tpl(X))",
  "deletefrtree(_,_,_,_) ",
  "performs delete operations on a Rtree file",
  "query deletefrtree()"
  );


OperatorSpec rebuildfrtreeSpec(
  "file{txt,str}xfile{txt,str}-> bool",
  "rebuildfrtree(_,_) ",
  "rebuilds an RTree and creates a new RTree file",
  "query rebuildfrtree(oldfile.bin, newfile.bin )"
  );


// 7. Operator Instance

Operator createfrtreeOp(
  "createfrtree",
  createfrtreeSpec.getStr(),
5,
  createfrtreeVM,
  createfrtreeSelect,
  createfrtreeTM
);

Operator insertfrtreeOp(
  "insertfrtree",
  insertfrtreeSpec.getStr(),
5,
  insertfrtreeVM,
  createfrtreeSelect,
  InsertfrtreeTM
);


Operator deletefrtreeOp(
  "deletefrtree",
  deletefrtreeSpec.getStr(),
5,
  deletefrtreeVM,
  createfrtreeSelect,
  deletefrtreeTM
);


Operator rebuildfrtreeOp(
  "rebuildfrtree",
  rebuildfrtreeSpec.getStr(),
  rebuildfrtreeValMap,
//  createfrtreeSelect,
  Operator::SimpleSelect,
  rebuildfrtreeTM
);


OperatorSpec fWindowIntersectSpec(
  "{txt, string}xR->stream(tid)",
  "_fwindowintersects(_)",
  "performs search in a Rtree index-file",
  "query fwindowintersects['strassen_GeoData_rtree', rectan] count"
);

ValueMapping fWindowIntersectVMA[] = {
    fWindowIntersectValMap<1>,
    fWindowIntersectValMap<2>,
    fWindowIntersectValMap<3>,
    fWindowIntersectValMap<4>,
    fWindowIntersectValMap<8>
};

int fWindowIntersectSelect (ListExpr args){

  if(listutils::isKind(nl->Second(args), Kind::SPATIAL1D())){
    return 0;
  }
  if (listutils::isKind(nl->Second(args), Kind::SPATIAL2D())){
    return 1;
  }
  if(listutils::isKind(nl->Second(args), Kind::SPATIAL3D())){
    return 2;
  }
  if(listutils::isKind(nl->Second(args), Kind::SPATIAL4D())){
    return 3;
  }
  if(listutils::isKind(nl->Second(args), Kind::SPATIAL8D())){
    return 4;
  }

   //should never be reached
   return -1;
}

Operator fWindowIntersectOp(
  "fwindowintersects",
  fWindowIntersectSpec.getStr(),
5,
  fWindowIntersectVMA,
  fWindowIntersectSelect,
  fWindowIntersectTM
);



// 
// RTree Bulkload
// 
// Value mapping select function
int bulkloadfrtreeSelect( ListExpr a ) {
  
  // args:
  // (
  //   (stream (tuple ((Rectangle rect) (TID tid))))
  //   text
  //   Rectangle
  //   TID
  //   int
  //   real
  // )
  
  NList args( a );
  NList argStream  = args.first();
  NList argKeyName = args.third();
  
  ListExpr attrList    = argStream.second().second().listExpr();
  string   keyAttrName = argKeyName.str();
  ListExpr keyAttrType = nl->Empty();
  FindAttribute( attrList, keyAttrName, keyAttrType );
  
  int res = -1;
  if ( listutils::isKind( keyAttrType, Kind::SPATIAL1D() ) )
    res = 0;
  else if ( listutils::isKind( keyAttrType, Kind::SPATIAL2D() ) )
    res = 1;
  else if ( listutils::isKind( keyAttrType, Kind::SPATIAL3D() ) )
    res = 2;
  else if ( listutils::isKind( keyAttrType, Kind::SPATIAL4D() ) )
    res = 3;
  else if ( listutils::isKind( keyAttrType, Kind::SPATIAL8D() ) )
    res = 4;
  
  return res;
}
// 
// Type mapping
ListExpr bulkloadfrtreeTM( ListExpr args ) {

  // args:
  // (
  //   ((stream (tuple ((Rectangle rect) (TID tid)))) (feed tmp1_tid))
  //   (text 'ix_rtree.bin')
  //   (Rectangle Rectangle)
  //   (TID TID)
  //   (int 2)
  //   (int 0)
  // )
  
  // log
  //cout << "bulkloadfrtreeTM( " << nl->ToString( args ) << " )" << endl;

  NList type( args );
  
  // Do some checks
  // 
  // Length check
  if ( !type.hasLength( 6 ) )
    return NList::typeError( "Expecting six arguments" );
  
  // 1: Stream
  NList argStream  = type.first();
  // 2: File
  NList argFile    = type.second();
  // 3: Key-Type
  NList argKeyName = type.third();
  // 4: TID
  NList argTidName = type.fourth();
  // 5: Int - Min. fill
  NList argMinFill = type.fifth();
  // 6: Int - Max. distance
  NList argMaxDist = type.sixth();
  
  // Length
  if ( !argStream.hasLength( 2 ) ||
       !argFile.hasLength( 2 ) ||
       !argKeyName.hasLength( 2 ) ||
       !argTidName.hasLength( 2 ) ||
       !argMinFill.hasLength( 2 ) ||
       !argMaxDist.hasLength( 2 ) )
    return NList::typeError( "internal error" );
  
  // Type checks
  if ( !Stream<Tuple>::checkType( argStream.first().listExpr() ) )
    return NList::typeError( "Stream of tuples expected" );
  //
  if ( !CcString::checkType( argFile.first().listExpr() ) &&
       !FText::checkType( argFile.first().listExpr() ) )
    return NList::typeError( "String or Text in first argument expected" );
  // 
  if ( !CcInt::checkType( argMinFill.first().listExpr() ) )
    return NList::typeError( "Integer in fourth argument expected" );
  // 
  if ( !CcReal::checkType( argMaxDist.first().listExpr() ) )
    return NList::typeError( "Real in fifth argument expected" );
  // 
  // Check attributes
  // 
  if ( !argKeyName.first().isSymbol() )
    return NList::typeError( "Attribute name in second argument expected" );
  if ( !argTidName.first().isSymbol() )
    return NList::typeError( "Attribute name in third argument expected" );
   
  ListExpr attrList = argStream.first().second().second().listExpr();
  // Key
  string   keyAttrName  = argKeyName.first().str();
  ListExpr keyAttrType  = nl->Empty();
  int      keyAttrIndex = FindAttribute( attrList, keyAttrName, keyAttrType );
  if ( keyAttrIndex <= 0 )
    NList::typeError( "Attribute name '" + keyAttrName + "' not found" );
  // TID
  string   tidAttrName  = argTidName.first().str();
  ListExpr tidAttrType  = nl->Empty();
  int      tidAttrIndex = FindAttribute( attrList, tidAttrName, tidAttrType );
  if ( tidAttrIndex <= 0 )
    NList::typeError( "Attribute name '" + tidAttrName + "' not found" );
  // 
  // Check attribute types
  if ( !TupleIdentifier::checkType( tidAttrType ) )
    return NList::typeError( "Attribute '" + tidAttrName + 
      "' is no TupleIdentifier" );
  if ( !listutils::isKind( keyAttrType, Kind::SPATIAL1D() ) &&
       !listutils::isKind( keyAttrType, Kind::SPATIAL2D() ) &&
       !listutils::isKind( keyAttrType, Kind::SPATIAL3D() ) &&
       !listutils::isKind( keyAttrType, Kind::SPATIAL4D() ) &&
       !listutils::isKind( keyAttrType, Kind::SPATIAL8D() ) )
     return NList::typeError( "Attribute '" + keyAttrName + 
       "' is not of kind SPATIAL1D, SPATIAL2D, SPATIAL3D, "
       "SPATIAL3D or SPATIAL8D" );
  
  // 
  // check file for existance
  string indexFile = argFile.second().str();
  ifstream f( indexFile.c_str() );
  if ( f.good() ) {
    f.close();
    return NList::typeError( "File '" + indexFile + "' already exists" );
  } else { f.close(); }
  
  // Result
  NList append;
  // (i i)
  append.append( nl->IntAtom( keyAttrIndex - 1 ) );
  append.append( nl->IntAtom( tidAttrIndex - 1 ) );
  // 
  ListExpr res = NList(
    // (APPEND)
    NList( Symbol::APPEND() ),
    // append
    append,
    // (stream(tupel(..)))
    argStream.first()
  ).listExpr();
  
  return res;
}
// 
// Value mapping function
template<int dim>
int bulkloadfrtreeVM( Word* args, Word& result,
    int message, Word& local, Supplier s ) {
  
  // args
  // 0 : stream(tuple( .. ))
  // 1 : text - Filename
  // 2 : Symbol - Key Attribute Name
  // 3 : Symbol - TID Attribute Name
  // 4 : int - Min. filling
  // 5 : real - Max. distance
  // 6 : int - Key Attribute Index
  // 7 : int - TID Attr Index
  
  RTree<dim>* tree = static_cast<RTree<dim>*>( local.addr );
  
  switch( message ) {
    case OPEN: {
      if ( tree != 0 ) delete tree;
      
      string  indexFile       = ((Attribute*)args[1].addr)->toText();
      int     minFilling      = StdTypes::GetInt( args[4].addr );
      CcReal* maxDistanceAttr = static_cast<CcReal*>( args[5].addr );
      
      double maxDistance = maxDistanceAttr->IsDefined() ?
        maxDistanceAttr->GetValue() : 0.0;
      
      if ( minFilling < 0 )
        minFilling = 0;
      
      // Create tree and start bulkload process
      try {
        tree = RTree<dim>::Create( indexFile.c_str(), 1024, minFilling );
        tree->BeginBulkload( maxDistance );
        local.addr = tree;
      }
      catch( exception& ex ) {
        cerr << "Creation failed: " << ex.what() << endl;
        return CANCEL;
      } // catch
      
      // Open stream
      qp->Open( args[0].addr );
      return 0;
    } // case OPEN
    case REQUEST: {
      if ( local.addr == 0 ) return CANCEL;
      
      // Request next element in stream
      Word tupleWord( Address( NULL ) );
      qp->Request( args[0].addr, tupleWord );
      
      if ( qp->Received( args[0].addr ) ) {
        Tuple* tuple = (Tuple*)tupleWord.addr;
        
        int keyAttrIndex = StdTypes::GetInt( args[6].addr );
        int tidAttrIndex = StdTypes::GetInt( args[7].addr );
         
        Attribute*       keyAttr = tuple->GetAttribute( keyAttrIndex );
        StandardSpatialAttribute<dim>* key =
          (StandardSpatialAttribute<dim>*)keyAttr;
        
        TupleIdentifier* tidAttr =
          (TupleIdentifier*)tuple->GetAttribute( tidAttrIndex );
        TupleId tid = tidAttr->GetTid();
        
        try {
          tree->Bulkload( key->BoundingBox(), (size_t)tid );
        }
        catch( exception& ex ) {
          cerr << "Bulkload error: " << ex.what() << endl;
          return CANCEL;
        } // catch
        
        result.setAddr( tupleWord.addr );
        return YIELD;
      }
      else {
        // End of stream reached
        result.addr = NULL;
        return CANCEL;
      } // else
    } // case REQUEST
    case CLOSE: {
      // close stream
      qp->Close( args[0].addr );
      
      if ( local.addr != 0 ) {
        // End bulkload, cleanup
        tree->EndBulkload();
        
        // log
        //cout << tree->ToString() << endl;
        
        delete tree;
        local.addr = NULL;
      } // if
      
      return 0;
    } // case CLOSE;
    default: {
      // should never happen
      return -1;
    } // default;
  } // switch
}
// 
// Value mapping array
ValueMapping bulkloadfrtreeVMA[] = {
  bulkloadfrtreeVM<1>,
  bulkloadfrtreeVM<2>,
  bulkloadfrtreeVM<3>,
  bulkloadfrtreeVM<4>,
  bulkloadfrtreeVM<8>
};
// 
// OperatorSpecs
OperatorSpec bulkloadfrtreeOperatorSpec(
  "stream( tuple( X ) ) x {text, string} x Ident x"
    "Ident x int x real -> stream( tuple( X ) )",
  "_ bulkloadfrtree [_, _, _, _, _]",
  "creates a file based R-Tree index with bulkload",
  "query buildings feed addid bulkloadfrtree"
    "['index.bin', Rect, TID, 10, 5.0] count"
);
// 
// Operators
Operator bulkloadfrtreeOp(
  "bulkloadfrtree",
  bulkloadfrtreeOperatorSpec.getStr(),
  5,
  bulkloadfrtreeVMA,     // value mapping array
  bulkloadfrtreeSelect,  // select function
  bulkloadfrtreeTM       // type mapping
);


/*
Definition of the algebra

*/
class FileIndexAlgebra : public Algebra{
  public:
  FileIndexAlgebra() : Algebra(){

    //AddOperator (&ftestOp);
    //AddOperator (&ftestOp);
    AddOperator (&createfrtreeOp);
    createfrtreeOp.SetUsesMemory();
    AddOperator (&fWindowIntersectOp);
    fWindowIntersectOp.SetUsesMemory();
    fWindowIntersectOp.SetUsesArgsInTypeMapping();
    AddOperator (&rebuildfrtreeOp);
    rebuildfrtreeOp.SetUsesMemory();
    rebuildfrtreeOp.SetUsesArgsInTypeMapping();
  //  rebuildfbtreeOp.SetUsesArgsInTypeMapping();
    AddOperator (&insertfrtreeOp);
    insertfrtreeOp.SetUsesMemory();
    insertfrtreeOp.SetUsesArgsInTypeMapping();
    //AddOperator (&FWindowintersectsOp);

    AddOperator (&deletefrtreeOp);
    deletefrtreeOp.SetUsesMemory();
    deletefrtreeOp.SetUsesArgsInTypeMapping();
    
    
    // 
    // R-Tree bulkload
    // 
    AddOperator( &bulkloadfrtreeOp );
    bulkloadfrtreeOp.SetUsesArgsInTypeMapping();
    bulkloadfrtreeOp.SetUsesMemory();
    
    
    //
    // BPTree operators
    //
    // Createfbtree
    AddOperator( &BPTreeOperators::GetCreatefbtreeOperator() );
    // Rebuildfbtree
    AddOperator( &BPTreeOperators::GetRebuildfbtreeOperator() );
    // Deletefbtree
    AddOperator( &BPTreeOperators::GetDeletefbtreeOperator() );
    // Insertfbtree
    AddOperator( &BPTreeOperators::GetInsertfbtreeOperator() );
    // Bulkloadfbtree
    AddOperator( &BPTreeOperators::GetBulkloadfbtreeOperator() );
    //
    // frange
    Operator& frangeOp = BPTreeOperators::GetFrangeOperator();
    AddOperator( &frangeOp );
    frangeOp.SetUsesArgsInTypeMapping();
    // fleftrange
    Operator& fleftrangeOp = BPTreeOperators::GetFleftrangeOperator();
    AddOperator( &fleftrangeOp );
    fleftrangeOp.SetUsesArgsInTypeMapping();
    // frightrange
    Operator& frightrangeOp = BPTreeOperators::GetFrightrangeOperator();
    AddOperator( &frightrangeOp );
    frightrangeOp.SetUsesArgsInTypeMapping();
    // fexactmatch
    Operator& fexactmatchOp = BPTreeOperators::GetFexactmatchOperator();
    AddOperator( &fexactmatchOp );
    fexactmatchOp.SetUsesArgsInTypeMapping();
  }

  ~FileIndexAlgebra() {};
};
} // end of namespace

/*
Initialization of algebra

*/
extern "C"
Algebra* InitializeFileIndexAlgebra( NestedList* nlRef, QueryProcessor* qpRef ){
  nl = nlRef;
  qp =qpRef;

  return new fialgebra::FileIndexAlgebra;
}

































