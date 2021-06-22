/*

*/
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/fsrel.h"
#include <ListUtils.h>
#include <StandardTypes.h>
#include "Stream.h"
#include "DPDTypeMapper.h"

namespace distributed3 {

             /****** static helper functions *********/

bool DPDTypeMapper::arraySubtypeEqualsFunctionArgument(ListExpr arrayType, 
                                        ListExpr functionType) {
  ListExpr expFunArg =   distributed2::DArray::checkType(arrayType)
                        ?nl->Second(arrayType)
                        : nl->TwoElemList(
                               listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second(nl->Second(arrayType)));

  if(!nl->Equal(expFunArg, nl->Second(functionType))){
     return false;
  }
  return true;
}
/* returns the result type of the functions type */

ListExpr DPDTypeMapper::getResultType(ListExpr function) {
  ListExpr res = function;
  while(!nl->HasLength(res,1)){
     res = nl->Rest(res);
  }
  return nl->First(res);
}
/*
  returns the last argument of a function as a ListExpr
  TODO not tested for the value part of a function. 
    
*/
ListExpr DPDTypeMapper::getLastArgumentOf(ListExpr function) {
  ListExpr d1UsedArg = function;
  while(!nl->HasLength(d1UsedArg,2)){
    d1UsedArg = nl->Rest(d1UsedArg);
  }
  return nl->First(d1UsedArg);
}
/*
  replaces the occurrance of a type operator in the value part of a function
  (functionValue) by replacement. 
  TODO next part required?
  returns a function with one argument. If functionValue contains two arguments
  the second is used.
  
*/
ListExpr DPDTypeMapper::replaceTypeOperator(ListExpr functionValue, 
                                            ListExpr replacement) {

  ListExpr ddarg = nl->HasLength(functionValue,3)
                    ?nl->Second(functionValue)
                    :nl->Third(functionValue);
                    
  ListExpr result = nl->HasLength(functionValue,3)
                   ?nl->Third(functionValue)
                   :nl->Fourth(functionValue);
                   
  return nl->ThreeElemList(
                       nl->First(functionValue),
                       nl->TwoElemList(
                             nl->First(ddarg), // elem_X
                             replacement),
                       result
                     );
}


           /***** public member functions ******/


void DPDTypeMapper::printValues() {
  std::cout << "\nargs:";
  nl->WriteListExpr(args);
}

ListExpr DPDTypeMapper::typeMapping(){
  if (   !rightNumberOfArgs()
      || !usesArgsInTypeMapping()
      || !checkArgs()
      || !checkInterdependencies()) {
    return typeError();
  }
  return result();
}

      /****** private member functions ********/

std::string DPDTypeMapper::getErrorMessage() {
  return msg;
}

ListExpr DPDTypeMapper::typeError() {
  return listutils::typeError(err() + getErrorMessage());
}

bool DPDTypeMapper::isRel() {
  return Relation::checkType(getResultType(nl->First(dmap2)));
}

bool DPDTypeMapper::isStream() {  
  return Stream<Tuple>::checkType(getResultType(nl->First(dmap2)));
}

bool DPDTypeMapper::rightNumberOfArgs() {
  if(!nl->HasLength(args,numberOfArgs())){ 
    msg = "wrong number of args in specification file";
    return false;
  }
  return true;
}

bool DPDTypeMapper::usesArgsInTypeMapping() {
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
     if(!nl->HasLength(nl->First(tmp),2)){
       msg = "please set SetUsesArgsInTypeMapping() "
             "for this operator in Constructor of Algebra";
       return false;
     }
     tmp = nl->Rest(tmp);
  }
  return true;
}



bool DPDTypeMapper::checkArrayType() {
  ListExpr arrayType = nl->First(array);
  if(   !distributed2::DArray::checkType(arrayType) 
     && !distributed2::DFArray::checkType(arrayType)){
     msg = "first arg is not a d[f]array";
     return false;
  }
  ListExpr relation = nl->Second(arrayType);
  if(!Relation::checkType(relation)){
    msg = "array subtype is not a relation";
    return false;
  }
  return true;
}

bool DPDTypeMapper::checkNameType() {
  if(!CcString::checkType(nl->First(name))){
    msg = "second arg is not a string";
    return false;
  }
  return true;
}

bool DPDTypeMapper::checkPartitionType() {
  ListExpr partitionfunctionType = nl->First(partitionfunction);
  if(!listutils::isMap<1>(partitionfunctionType) ){
    msg = "fourth arg is not a function";
    return false;
  }
  if(!CcInt::checkType(getResultType(partitionfunctionType))){
    msg = "result for distribution function is not an int";
    return false;
  }
  if(!Tuple::checkType(getLastArgumentOf(partitionfunctionType))){
    msg = "argument of the distribution function is not a tuple";
    return false;
  }  
  return true;
}

bool DPDTypeMapper::checkNumberOfSlotsType() {
  if(!CcInt::checkType(nl->First(numberOfSlots))){
    msg = " (fifth arg is not an int)";
    return false;
  }
  return true;
}
bool DPDTypeMapper::checkDmap2Type() {
  ListExpr dmap2Type = nl->First(dmap2);
  if(!listutils::isMap<1>(dmap2Type) ){
    msg = "last arg is not a function";
    cout << msg;
    return false;
  }
  if (!Stream<Tuple>::checkType(getLastArgumentOf(dmap2Type))) {
    msg = "argument of dmap2 function is not a stream";
    return false;
  }
  if(listutils::isStream(getResultType(dmap2Type)) && !isStream()){
    msg = "function produces a stream of non-tuples.";
    return false;
  }
  return true;
}

ListExpr DPDTypeMapper::result() {
    return  nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   append(), 
                   resultType());  
}

ListExpr DPDTypeMapper::appendDmap2() {
  return replaceTypeOperator(nl->Second(dmap2),
                             nl->Second(nl->First(dmap2)));
}

ListExpr DPDTypeMapper::resultType() {

  ListExpr dmap2funRes = getResultType(nl->First(dmap2));
  // allowed result types are streams of tuples and
  // non-stream objects
 
  // compute the subtype of the resulting array
  if(isStream()){
    dmap2funRes = nl->TwoElemList(
               listutils::basicSymbol<Relation>(),
               nl->Second(dmap2funRes));
  }
  // determine the result array type
  // is the origin function result is a tuple stream,
  // the result will be a dfarray, otherwise a darray
  return nl->TwoElemList(
               isStream()?listutils::basicSymbol<distributed2::DFArray>()
                       :listutils::basicSymbol<distributed2::DArray>(),
               dmap2funRes);
}

} // end namespace
