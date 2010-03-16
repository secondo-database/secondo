/*



1 Class FlobId

This class encapsulates fileId, recordId and an offset within the 
record.

*/



#ifndef FLOBID_H
#define FLOBID_H

#include <stdint.h>
#include <ostream>
#include "SecondoSMI.h"



 // forward declaration of class FlobManager
class FlobManager;



class FlobId{
 friend class FlobManager;
 friend class FlobCache;
 friend class NativeFlobCache;
 friend class NativeCacheEntry;
 public: 
   FlobId(){} // makes nothing to support cast function

   ~FlobId(){}

   FlobId(const FlobId& src): fileId(src.fileId),
                              recordId(src.recordId), 
                              offset(src.offset),
                              isTemp(src.isTemp){}

   FlobId& operator=(const FlobId& src){
     fileId = src.fileId;
     recordId = src.recordId;
     offset = src.offset;
     isTemp = src.isTemp;
     return *this;
   }

   ostream& print(ostream& os) const {
     os << "id(file = "<< fileId << ", "
        << "rec = " << recordId << ", "
        << "offset = " << offset << ", "
        << "isTemp = " << (isTemp?"true":"false") << ")";
     return os;
   }    

   inline bool operator==(const FlobId& fid) const{
     return fileId == fid.fileId &&
            recordId == fid.recordId &&
            offset   == fid.offset &&
            isTemp == fid.isTemp;
   }
   
   inline bool operator!=(const FlobId& fid) const{
     return fileId != fid.fileId ||
            recordId != fid.recordId ||
            offset   != fid.offset ||
            isTemp != fid.isTemp;
   }

   
 
   inline bool operator>(const FlobId& fid) const{
      if(isTemp && !fid.isTemp) return false;
      if(!isTemp && fid.isTemp) return true;
      if( fileId > fid.fileId) return true;
      if( fileId < fid.fileId) return false;
      if( recordId > fid.recordId) return true;
      if(recordId < fid.recordId) return false;
      if(offset > fid.offset) return true;
      return false;
   } 
   inline bool operator<(const FlobId& fid) const{
      if(isTemp && !fid.isTemp) return true;
      if(!isTemp && fid.isTemp) return false;
      if( fileId < fid.fileId) return true;
      if( fileId > fid.fileId) return false;
      if( recordId < fid.recordId) return true;
      if(recordId > fid.recordId) return false;
      if(offset < fid.offset) return true;
      return false;
   } 

   inline size_t hashValue() const{
      size_t t = isTemp?1:0;
      return (size_t)(fileId + recordId + offset + t); 
   }

   void destroy(){
     fileId = 0;
     recordId = 0;
     offset = 0;
   }

   bool isDestroyed() const{
     return fileId == 0 &&
            recordId == 0 &&
            offset == 0;
   }

 private:
   SmiFileId   fileId;  
   SmiRecordId recordId;
   SmiSize     offset;
   bool        isTemp;


   FlobId(const SmiFileId fid, 
          const SmiRecordId rid,
          const SmiSize os,
          const bool tmp) : fileId(fid), recordId(rid), 
                            offset(os),isTemp(tmp){}



 
};

ostream& operator<<(ostream& os, const FlobId& fid);
#endif

