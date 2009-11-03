/*


1 Class FlobManager

The only class using the FlobManager class is Flob. For this reason,
all functions are declared as private and Flob is declared to be a friend
of that class.

*/

#include "SecondoSMI.h"
#include "Flob.h"
#include "assert.h"
#include <iostream>

#undef __TRACE_ENTER__
#undef __TRACE_LEAVE__

//#define __TRACE_ENTER__
//#define __TRACE_LEAVE__
#define __TRACE_ENTER__ std::cerr << "Enter : " << \
        __PRETTY_FUNCTION__ << std::endl;
#define __TRACE_LEAVE__ std::cerr << "Leave : " << \
        __PRETTY_FUNCTION__ << "@" << __LINE__ << std::endl;




FlobManager* FlobManager::instance = 0;

/*
 
The FlobManager is realized to be a singleton. So, not the contructor
but this getInstance function is to call to get the only one
FlobManager instance.

*/   



FlobManager& FlobManager::getInstance(){
 __TRACE_ENTER__
  if(!instance){
    instance = new FlobManager();
  }
__TRACE_LEAVE__
  return *instance;
}

bool FlobManager::destroyInstance(){
  if(!instance){
    return false;
  } else {
    delete instance;
    instance = 0;
    return true;
  }
}

/*
~Destructor~

The destructor should never be called from outside. Instead the destroyInstance
function should be called. By calling the destructor, all open files are closed
and the nativFlobFile is deleted.


*/
     FlobManager::~FlobManager(){
       if(!nativeFlobFile->Close()){
         std::cerr << "Problem in closing nativeFlobFile" << std::endl;
       }
       if(!nativeFlobFile->Remove()){
         std::cerr << "Problem in deleting nativeFlobFile" << std::endl;
       }
       delete nativeFlobFile;
       nativeFlobFile = 0;
       nativeFlobs = 0;
       // close all files stored in Map   
       map<SmiFileId, SmiRecordFile*>::iterator iter;
       for( iter = openFiles.begin(); iter != openFiles.end(); iter++ ) {
         iter->second->Close(); 
         delete iter->second;
       }
       openFiles.clear();

     }


SmiRecordFile* FlobManager::getFile(const SmiFileId& fileId) {
 __TRACE_ENTER__

  SmiRecordFile* file(0);
  map<SmiFileId, SmiRecordFile*>::iterator it = openFiles.find(fileId);
  if(it == openFiles.end()){
     file = new SmiRecordFile(false);
     openFiles[fileId] = file;
     file->Open(fileId);
  } else{
     file = it->second;
  }
__TRACE_LEAVE__
  return file;
}


/*
~getData~

The getData function retrieves Data from a Flob. 
The __dest__ buffer must be provided by the caller. The requested content is copyied
to that buffer. 

*/

bool FlobManager::getData(
         const Flob& flob,            // Flob containing the data
         char* dest,                  // destination buffer
         const SmiSize&  offset,     // offset within the Flob 
         const SmiSize&  size) {  // requested data size
 __TRACE_ENTER__

  FlobId id = flob.id;
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     floboffset = id.offset;
  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);
  
  bool ok = file->SelectRecord(recordId, record);
  if(!ok){
    __TRACE_LEAVE__
    return false;
  }
 
  SmiSize read = record.Read(dest,size, floboffset + offset);
  if(read!=size){
    __TRACE_LEAVE__
    return false;
  } 
  __TRACE_LEAVE__
  return true;
}


/*
~destroy~

Frees all resources occupied by the Flob. After destroying a flob. No data can be 
accessed.

*/
void FlobManager::destroy(Flob& victim) {

 __TRACE_ENTER__
   FlobId id = victim.id;
   SmiSize size = victim.getSize();
   SmiRecordId recordId = id.recordId;
   SmiSize offset = id.offset;


   SmiRecordFile* file = getFile(id.fileId);
   SmiRecord record;
   bool ok = file->SelectRecord(recordId, record);  
   if(!ok){ // record not found in file
     __TRACE_LEAVE__
     return; 
   }
   // check whether the flob occupies the whole record
   SmiSize recordSize = record.Size();
   
   if(offset!=0){ // flob doesn't start at the begin of the record
      if( offset + size != recordSize){
         std::cout << "cannot destroy flob, because after the flob data are"
                      " available" << std::endl;
        
      } else { // truncate record
         record.Truncate(offset);
         record.Finish();
      }
   } else {
     if(recordSize != size){
       std::cout << "cannot destroy flob, because after the flob data are"
                    " available" << std::endl;
     } else {
        // record stores only the flob
        file->DeleteRecord(recordId); 
     }
   }
  __TRACE_LEAVE__
}


/*
~saveTo~

Save the content of a flob into a file at a specific position (given as recordId and 
offset). This will result in another Flob which is returned as the result of this 
function. A record with the corresponding id must already exist in the file.

Initial implementation, should be changed to support Flobs larger than the 
available main memory.

*/
bool FlobManager::saveTo(const Flob& src,   // Flob tp save
       const SmiFileId fileId,  // target file id
       const SmiRecordId recordId, // target record id
       const SmiSize offset,
       Flob& result)  {   // offset within the record  

 __TRACE_ENTER__
   SmiRecord record;
   SmiRecordFile* file = getFile(fileId);
   if(!file->SelectRecord(recordId, record)){
     __TRACE_LEAVE__
     return false;
   }
   char buffer[src.size];
   getData(src,buffer,0,src.size);
   // save the data
   SmiSize wsize = record.Write(buffer, src.size, offset);
   record.Finish();
   if(wsize!=src.size){
     __TRACE_LEAVE__
     return false;
   }
   FlobId id;
   id.fileId = fileId;
   id.recordId = recordId;
   id.offset = offset;
   result.id = id;
   result.size = src.size;   
   __TRACE_LEAVE__
   return true;
}


/*
~saveTo~

Saves a Flob into a specific file. The flob is saved into a new created record
in the file at offset 0. By saving the old Flob, a new Flob is created which
is the result of this function.

Must be changed to support real large Flobs

*/

 bool FlobManager::saveTo(
             const Flob& src,             // flob to save
             const SmiFileId fileId,      // target file
             Flob& result){         // result
 __TRACE_ENTER__
    SmiRecordId recId;
    SmiRecord rec;
    SmiRecordFile* file = getFile(fileId);
    if(!file->AppendRecord(recId,rec)){
      __TRACE_LEAVE__
      return false;
    }
    // write data
    char buffer[src.size];
    rec.Write(buffer, src.size,0);
    rec.Finish();
    FlobId fid(fileId, recId,0);
    result.id = fid;
    result.size = src.size;
    __TRACE_LEAVE__
    return true;
 }

/*
~putData~

Puts data into a flob. 

*/

bool FlobManager::putData(const Flob& dest,         // destination flob
                          const char* buffer, // source buffer
                          const SmiSize& targetoffset, // offset within the Flob
                          const SmiSize& length) { // data size

 __TRACE_ENTER__
  FlobId id = dest.id;
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     offset = id.offset;
  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);
  bool ok = file->SelectRecord(recordId, record);
  if(!ok){
    __TRACE_LEAVE__
    return false;
  }
  SmiSize wsize = record.Write(buffer, length, offset + targetoffset);
  if(wsize!=length){
    __TRACE_LEAVE__
    return false;
  }
  record.Finish();
  __TRACE_LEAVE__
  return true;
}
/*
~create~

Creates a new Flob with a given size which is assigned to a temporal file.


*/

 bool FlobManager::create(const SmiSize& size, Flob& result) {  // result flob
 __TRACE_ENTER__
   SmiRecord rec;
   SmiRecordId recId;
   if(!(nativeFlobFile->AppendRecord(recId,rec))){
      __TRACE_LEAVE__
      return false;
   }
   rec.Finish();
   FlobId fid(nativeFlobs, recId, 0);
   result.id = fid;
   result.size = size;
   __TRACE_LEAVE__
   return true;
 }

/*
~create~

Creates a new flob with given file and position. File and record 
must exists.

*/
 bool FlobManager::create(const SmiFileId& fileId,        // target file
             const SmiRecordId& recordId,    // target record id
             const SmiSize& offset,      // offset within the record
             const SmiSize& size,
             Flob& result){       // initial size of the Flob

 __TRACE_ENTER__
   SmiRecordFile* file = getFile(fileId);
   SmiRecord record;
   if(!file->SelectRecord(recordId,record)){
     __TRACE_LEAVE__
     return false;
   } 
   FlobId fid(fileId, recordId, offset);
   result.id = fid;
   result.size = size;
   __TRACE_LEAVE__
   return true; 
}

  bool FlobManager::copyData(const Flob& src, Flob& dest){
 __TRACE_ENTER__
    char buffer[src.size];
    if(!getData(src, buffer, 0, src.size)){
      __TRACE_LEAVE__
      return false;
    }
    if(! putData(dest,buffer,0,src.size)){
      __TRACE_LEAVE__
      return false;
    }
    dest.size = src.size;  
    __TRACE_LEAVE__
    return true;
  }


/*
~constructor~

Because Flobmanager is a singleton, the constructor should only be used
by the FlobManager class itself.

*/

  FlobManager::FlobManager():openFiles(){
 __TRACE_ENTER__
    assert(instance==0); // the constructor should only called one time
    // construct the temporarly FlobFile
    nativeFlobFile  = new SmiRecordFile(false,0,true); 
    bool created = nativeFlobFile->Create();
    assert(created);
    nativeFlobs = nativeFlobFile->GetFileId();
    __TRACE_LEAVE__
  }



