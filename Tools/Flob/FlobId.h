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
 friend class CacheEntry;
 public: 
   FlobId(){} // makes nothing to support cast function
   ~FlobId(){}
   FlobId(const FlobId& src): fileId(src.fileId),
                              recordId(src.recordId), 
                              offset(src.offset){}
   FlobId& operator=(const FlobId& src){
     fileId = src.fileId;
     recordId = src.recordId;
     offset = src.offset;
     return *this;
   }

   ostream& print(ostream& os) const {
     os << "id(file = "<< fileId << ", "
        << "rec = " << recordId << ", "
        << "offset = " << offset << ")";
     return os;
   }    

   inline bool operator==(const FlobId& fid) const{
     return fileId == fid.fileId &&
            recordId == fid.recordId &&
            offset   == fid.offset;
   } 
   inline bool operator>(const FlobId& fid) const{
      if( fileId > fid.fileId) return true;
      if( fileId < fid.fileId) return false;
      if( recordId > fid.recordId) return true;
      if(recordId < fid.recordId) return false;
      if(offset > fid.offset) return true;
      return false;
   } 
   inline bool operator<(const FlobId& fid) const{
      if( fileId < fid.fileId) return true;
      if( fileId > fid.fileId) return false;
      if( recordId < fid.recordId) return true;
      if(recordId > fid.recordId) return false;
      if(offset < fid.offset) return true;
      return false;
   } 

 private:
   SmiFileId fileId;  
   SmiRecordId recordId;
   SmiSize offset;


   FlobId(const SmiFileId fid, 
          const SmiRecordId rid,
          const SmiSize os) : fileId(fid), recordId(rid), offset(os){}



 
};

ostream& operator<<(ostream& os, const FlobId& fid);
#endif

