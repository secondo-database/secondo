/*

*/

#ifndef SECONDO_TESCONTEXT_H
#define SECONDO_TESCONTEXT_H

#include <GenericTC.h>
#include <ostream>
#include "typedefs.h"
#include "Helpers/WorkerConfig.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace distributed3 {

class TESContext {
private:
  
  static TESContext context;

  std::map<int, std::string> eid2StringTupleType;
  std::map<int, TupleType*> eid2TupleType;
  
  std::map<int,int> slotCountMap;
  
  int messageServerPort = 0;
  

public:
 
  static TESContext &get();

  void reset() { // TODO könnte auch in den Destruktor !
    for (auto it = eid2TupleType.begin(); it != eid2TupleType.end(); ++it) {
      std::cout << "\ndelete " << (*it).first;
      if((*it).second){
       ((*it).second)->DeleteIfAllowed();
       ((*it).second) = nullptr;
       
     }
      //delete (*it).second; // TODO tupletype->DeleteIfAllowed() ?
    }
  
  /*
   messageType = "";
   if(tupleType){
     tupleType->DeleteIfAllowed();
     tupleType = 0;
   }
  

   // TODO nach TESManager.reset() verschieben workers.clear();
   messageServerPort = 0;

   */
  }

  ~TESContext(){
    std::cout << "\n~TESContext() aufgerufen";
    reset();
    /* map-destructor: "This destroys all container elements, 
       and deallocates all the storage capacity 
       allocated by the map container using its allocator." 
       http://www.cplusplus.com/reference/map/map/~map/
    */
  }

  std::string& getStringTupleType(int eid);
  
  void setTupleType(int eid, ListExpr messageTypeAsList);
 
  void setNumericTupleType(int eid, ListExpr messageType);
  
  TupleType* &getTupleType(int eid) {
    return eid2TupleType[eid];
  }
  
  void setNumberOfSlots(const int eid, const int slots) {
    slotCountMap[eid] = slots;
  }
  
  int getNumberOfSlots(const int eid) {
    assert (slotCountMap[eid] > 0);
    return slotCountMap[eid];
  }
 
  int getMessageServerPort() const;

  void setMessageServerPort(int);

};

} // namespace distributed3


#endif //SECONDO_TESCONTEXT_H
