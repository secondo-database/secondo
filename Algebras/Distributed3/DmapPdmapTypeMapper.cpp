/*

*/
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/fsrel.h"
#include <ListUtils.h>
#include <StandardTypes.h>
#include "Stream.h"
#include "DmapPdmapTypeMapper.h"

namespace distributed3 {

DmapPdmapTypeMapper::DmapPdmapTypeMapper(ListExpr& _args) {
  args = _args;
  array = nl->First(args);  // array
  name = nl->Second(args); // name of the result
  dmap1 = nl->Third(args); // function
  partitionfunction = nl->Fourth(args);  // redistribution function
  numberOfSlots = nl->Fifth(args);  // size of the result
  dmap2 = nl->Sixth(args); 
}
std::string DmapPdmapTypeMapper::err() {
  return "expected: d[f]array(rel(tuple(X))) x string x "
           "([fs]rel(tuple(X)) -> stream(tuple(Y))) x "
           "(tuple(Y)->int) x int x"
           "(stream(tuple(Y)) -> Z)";  
}
bool DmapPdmapTypeMapper::rightNumberOfArgs() {
  if(!nl->HasLength(args,6)){ 
    msg = "wrong number of args";
    return false;
  }
  return true;
}
bool DmapPdmapTypeMapper::checkArgs() {
  if (   !checkArrayType()
      || !checkNameType()
      || !checkDmap1Type()
      || !checkPartitionType()
      || !checkNumberOfSlotsType()
      || !checkDmap2Type()) {
    return false;
  } 
  return true;
}
bool DmapPdmapTypeMapper::checkDmap1Type() {
  ListExpr dmap1Type = nl->First(dmap1);
  if(!listutils::isMap<1>(dmap1Type) ){
    msg = "third arg is not a function";
    return false;
  }
  if(!Relation::checkType(getLastArgumentOf(dmap1Type))){
    msg = "argument of the dmap1 function is not a relation";
    return false;
  }  
  if(!Stream<Tuple>::checkType(getResultType(dmap1Type))){
    msg = "function result is not a tuple stream";
    return false;
  }
  // if dmap1Type is defined to have two arguments, we ensure that the 
  // second argument is unused
  // within the whole function definition
  if(listutils::isMap<2>(dmap1Type)){
    std::string arg2Name = nl->SymbolValue(
                                 nl->First(nl->Third(nl->Second(dmap1))));
    if(listutils::containsSymbol(nl->Fourth(nl->Second(dmap1)), arg2Name)){
      msg = "Usage of the second argument in function is not allowed";
      return false;
    }
  }  
  
  return true;
}
bool DmapPdmapTypeMapper::checkInterdependencies() {
  ListExpr arrayType = nl->First(array);
  ListExpr partitionfunctionType = nl->First(partitionfunction);
  ListExpr dmap2Type = nl->First(dmap2);
  ListExpr dmap1Type = nl->First(dmap1); // (map ...)
  
  if (!arraySubtypeEqualsFunctionArgument(arrayType, dmap1Type)) {
    msg = "argument of function does not fit the array type.";
    return false;
  }
  if(!nl->Equal(nl->Second(getResultType(dmap1Type)),
                getLastArgumentOf(partitionfunctionType))){
    msg = "type mismatch between result of the function "
          "and the argument of the distribution function";
    return false;
  }
    
  if(!nl->Equal(getResultType(dmap1Type), nl->Second(dmap2Type))){
    msg = "argument of third function does not "
          "fit the result type of the first function.";
     return false;
  }
  
  return true;
}
ListExpr DmapPdmapTypeMapper::result() {
    return  nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->FiveElemList(
                     nl->TextAtom(nl->ToString(appendDmap1())), // dmap1
                     nl->TextAtom(nl->ToString(appendPartition())), // partition
                     nl->TextAtom(nl->ToString(appendDmap2())),  //  dmap2
                     nl->BoolAtom(isRel()),               //  dmap2
                     nl->BoolAtom(isStream())),           //  dmap2
                   resultType());  
  
}
ListExpr DmapPdmapTypeMapper::appendDmap1() {
  ListExpr arrayType = nl->First(array);
  ListExpr relation = nl->Second(arrayType);
  ListExpr expFunArg =   distributed2::DArray::checkType(arrayType)
                        ?relation
                        : nl->TwoElemList(
                               listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second(relation));

  return replaceTypeOperator(nl->Second(dmap1), expFunArg);
}
ListExpr DmapPdmapTypeMapper::appendPartition() {
  return replaceTypeOperator(nl->Second(partitionfunction), 
                             nl->Second(getResultType(nl->First(dmap1))));
}


} // namespace
