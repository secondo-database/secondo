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
     os << "fid = " << fileId << ", "
        << "recId = " << recordId << ", "
        << "offset = " << offset;
     return os;
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

