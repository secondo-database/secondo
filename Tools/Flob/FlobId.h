/*



1 Class FlobId

This class encapsulates fileId, recordId and an offset within the 
record.

*/



#ifndef FLOBID_H
#define FLOBID_H

#include <stdint.h>
#include <ostream>
#include <sstream>
#include "SecondoSMI.h"



 // forward declaration of class FlobManager
class FlobManager;



class FlobId{
 friend class FlobManager;
 friend class FlobCache;
 friend class NativeFlobCache;
 friend class NativeCacheEntry;
 public:

  

 
   FlobId(){} // does  nothing to support cast function



   ~FlobId(){}




   FlobId(const FlobId& src): fileId(src.fileId),
                              recordId(src.recordId), 
                              offset(src.offset),
                              mode(src.mode){}

   FlobId& operator=(const FlobId& src){
     fileId = src.fileId;
     recordId = src.recordId;
     offset = src.offset;
     mode = src.mode;
     return *this;
   }

   ostream& print(ostream& os) const {
     os << "id(file = "<< fileId << ", "
        << "rec = " << recordId << ", "
        << "offset = " << offset << ", "
        << "mode = " << (int) mode;
     return os;
   }

   string describe() const {
     stringstream ss;
     ss << fileId << " " << recordId << " " << offset << " " << (int)mode;
     return ss.str();
   }

   inline bool operator==(const FlobId& fid) const{
     return fileId == fid.fileId &&
            recordId == fid.recordId &&
            offset   == fid.offset &&
            mode == fid.mode;
   }
   
   inline bool operator!=(const FlobId& fid) const{
     return fileId != fid.fileId ||
            recordId != fid.recordId ||
            offset   != fid.offset ||
            mode != fid.mode;
   }
 
   inline bool operator>(const FlobId& fid) const{
      if( mode > fid.mode) return true;
      if( mode < fid.mode) return false;
      if( fileId > fid.fileId) return true;
      if( fileId < fid.fileId) return false;
      if( recordId > fid.recordId) return true;
      if(recordId < fid.recordId) return false;
      if(offset > fid.offset) return true;
      return false;
   } 
   inline bool operator<(const FlobId& fid) const{
      if( mode < fid.mode) return true;
      if( mode > fid.mode) return false;
      if( fileId < fid.fileId) return true;
      if( fileId > fid.fileId) return false;
      if( recordId < fid.recordId) return true;
      if(recordId > fid.recordId) return false;
      if(offset < fid.offset) return true;
      return false;
   } 

   inline size_t hashValue() const{
      size_t t = mode;
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

   inline char getMode(){ return mode;}

   inline SmiFileId getFileId(){ return fileId; }

   inline SmiRecordId getRecordId(){ return recordId; }

   // support for very  dirty, non explainable things, don't think about any 
   // meaning of this funtion!
   SmiSize   getOffset() { return  offset; }

 private:
   SmiFileId   fileId;  
   SmiRecordId recordId;
   SmiSize     offset;
   char        mode;  

/*
The mode has following meaning

0 : Flob data in a normal berkeley db file
1 : Flob data in a temporarly berkeley db file
2 : Flob data in a local file (outside berkeley db)
3 : Flob data in a remote file  

*/ 


  /*  Constructor */


   FlobId(const SmiFileId _fid, const SmiRecordId _rid, 
          const SmiSize _offset, const char _mode):
        fileId(_fid), recordId(_rid), offset(_offset), mode(_mode){}



 
};

ostream& operator<<(ostream& os, const FlobId& fid);
#endif

