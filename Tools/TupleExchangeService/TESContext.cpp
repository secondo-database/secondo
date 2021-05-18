/*

*/
#include "TESContext.h"
#include <GenericTC.h>
#include "StandardTypes.h"
#include <boost/log/trivial.hpp>
#include "SecParser.h"
#include "typedefs.h"


namespace distributed3 {
 TESContext TESContext::context;

 int TESContext::getMessageServerPort() const {
  return messageServerPort;
 }

 void TESContext::setMessageServerPort(int messageServerPort) {
  TESContext::messageServerPort = messageServerPort;
 }
 
 std::string &TESContext::getStringTupleType(int eid) {
  return eid2StringTupleType[eid];
 }

 void TESContext::setTupleType(int eid, ListExpr tupleType) {
  eid2StringTupleType[eid] = nl->ToString(tupleType);
  //this->messageType.swap(stringTupleType);
  auto numericType = SecondoSystem::GetCatalog()->NumericType(tupleType);
  setNumericTupleType(eid, numericType);
  
 }
 
 void TESContext::setNumericTupleType(int eid, ListExpr numericMessageType) {
   TupleType* tt = eid2TupleType[eid];
    if(tt){
      tt->DeleteIfAllowed();
    }
    eid2TupleType[eid] = new TupleType(numericMessageType);
 }

 TESContext &TESContext::get() {
  return context;
 }

}




