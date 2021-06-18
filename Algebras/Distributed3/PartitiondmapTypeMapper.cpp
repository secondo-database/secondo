/*

*/
#include "Algebras/Distributed2/DArray.h"
#include "Algebras/Distributed2/fsrel.h"
#include <ListUtils.h>
#include <StandardTypes.h>
#include "Stream.h"
#include "PartitiondmapTypeMapper.h"

namespace distributed3 {

PartitiondmapTypeMapper::PartitiondmapTypeMapper(ListExpr& _args) {
  args = _args;
  array = nl->First(args);  // array
  name = nl->Second(args); // name of the result
  partitionfunction = nl->Third(args);
  numberOfSlots = nl->Fourth(args);
  dmap2 = nl->Fifth(args);
}
std::string PartitiondmapTypeMapper::err() {
  return "expected: d[f]array(rel(Tuple)) x string x (Tuple -> int) "
                     "x int x (stream(Tuple) -> X)";
}
bool PartitiondmapTypeMapper::rightNumberOfArgs() {
  if(!nl->HasLength(args,5)){ 
    msg = "wrong number of args";
    return false;
  }
  return true;
}
bool PartitiondmapTypeMapper::checkArgs() {
  if (   !checkArrayType()
      || !checkNameType()
      || !checkPartitionType()
      || !checkNumberOfSlotsType()
      || !checkDmap2Type()) {
    return false;
  } 
  return true;
}
bool PartitiondmapTypeMapper::checkInterdependencies() {
  ListExpr arrayType = nl->First(array);
  ListExpr partitionfunctionType = nl->First(partitionfunction);
  ListExpr dmap2Type = nl->First(dmap2);
  ListExpr relation = nl->Second(arrayType);
  
  if(!nl->Equal(nl->Second(relation), nl->Second(partitionfunctionType)) || 
     !nl->Equal(nl->Second(relation), 
                nl->Second(nl->Second(dmap2Type)))){
    msg = "tuple types of darray's relation and "
          "of functions [sub]subtypes differ";
    return false;
  }
  return true;
}
ListExpr PartitiondmapTypeMapper::result() {
  return nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->FourElemList(
                       nl->TextAtom(nl->ToString(appendPartition())),
                         nl->TextAtom(nl->ToString(appendDmap2())),  //  dmap
                         nl->BoolAtom(isRel()),               //  dmap
                         nl->BoolAtom(isStream())),           //  dmap
                   resultType());  
}
ListExpr PartitiondmapTypeMapper::appendPartition() {
  ListExpr relation = nl->Second(nl->First(array));
  return replaceTypeOperator(nl->Second(partitionfunction), 
                             nl->Second(relation));
}

} // end namespace
