/*


1 Class FlobManager

The only class using the FlobManager class is Flob. For this reason,
all functions are declared as private and Flob is declared to be a friend
of that class.

*/

#include "SecondoSMI.h"
#include "Flob.h"
#include "assert.h"




/*
 
The FlobManager is realized to be a singleton. So, not the contructor
but this getInstance function is to call to get the only one
FlobManager instance.

*/    

FlobManager& FlobManager::getInstance(){
  if(!instance){
    instance = new FlobManager();
  }
  return *instance;
}	

SmiRecordFile* FlobManager::getFile(const SmiFileId& fileId) {

  SmiRecordFile* file(0);
  map<SmiFileId, SmiRecordFile*>::iterator it = openFiles.find(fileId);
  if(it == openFiles.end()){
     file = new SmiRecordFile(false);
     openFiles[fileId] = file;
     file->Open(fileId);
  } else{
     file = it->second;
  }
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

  FlobId id = flob.id;
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     floboffset = id.offset;
  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);
  
  bool ok = file->SelectRecord(recordId, record);
  if(!ok){
    return false;
  }
 
  SmiSize read = record.Read(dest,size, floboffset + offset);
  if(read!=size){
    return false;
  } 
  return true;
}


/*
~destroy~

Frees all resources occupied by the Flob. After destroying a flob. No data can be 
accessed.

*/
void FlobManager::destroy(Flob& victim) {

   FlobId id = victim.id;
   SmiSize size = victim.getSize();
   SmiRecordId recordId = id.recordId;
   SmiSize offset = id.offset;


   SmiRecordFile* file = getFile(id.fileId);
   SmiRecord record;
   bool ok = file->SelectRecord(recordId, record);  
   if(!ok){ // record not found in file
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

   SmiRecord record;
   SmiRecordFile* file = getFile(fileId);
   return false;
   if(!file->SelectRecord(recordId, record)){
     return false;
   }
   char buffer[src.size];
   getData(src,buffer,0,src.size);
   // save the data
   SmiSize wsize = record.Write(buffer, src.size, offset);
   record.Finish();
   if(wsize!=src.size){
     return false;
   }
   FlobId id;
   id.fileId = fileId;
   id.recordId = recordId;
   id.offset = offset;
   result.id = id;
   result.size = src.size;   
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
    SmiRecordId recId;
    SmiRecord rec;
    SmiRecordFile* file = getFile(fileId);
    if(!file->AppendRecord(recId,rec)){
      return false;
    }
    // write data
    char buffer[src.size];
    rec.Write(buffer, src.size,0);
    rec.Finish();
    FlobId fid(fileId, recId,0);
    result.id = fid;
    result.size = src.size;
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

  FlobId id = dest.id;
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     offset = id.offset;
  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);
  bool ok = file->SelectRecord(recordId, record);
  if(!ok){
    return false;
  }
  SmiSize wsize = record.Write(buffer, length, offset + targetoffset);
  if(wsize!=length){
    return false;
  }
  record.Finish();
  return true;
}
/*
~create~

Creates a new Flob with a given size which is assigned to a temporal file.


*/

 bool FlobManager::create(const SmiSize& size, Flob& result) {  // result flob
   SmiRecord rec;
   SmiRecordId recId;
   if(!(nativeFlobFile->AppendRecord(recId,rec))){
      return false;
   }
   rec.Finish();
   FlobId fid(nativeFlobs, recId, 0);
   result.id = fid;
   result.size = size;
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

   SmiRecordFile* file = getFile(fileId);
   SmiRecord record;
   if(!file->SelectRecord(recordId,record)){
     return false;
   } 
   FlobId fid(fileId, recordId, offset);
   result.id = fid;
   result.size = size;
   return true; 
}

  bool FlobManager::copyData(const Flob& src, Flob& dest){
    char buffer[src.size];
    if(!getData(src, buffer, 0, src.size)){
      return false;
    }
    if(! putData(dest,buffer,0,src.size)){
      return false;
    }
    dest.size = src.size;  
    return true;
  }


/*
~constructor~

Because Flobmanager is a singleton, the constructor should only be used
by the FlobManager class itself.

*/

  FlobManager::FlobManager():openFiles(){
    assert(instance==0); // the constructor should only called one time
    // construct the temporarly FlobFile
    nativeFlobFile  = new SmiRecordFile(false,0,true); 
    bool created = nativeFlobFile->Create();
    assert(created);
    nativeFlobs = nativeFlobFile->GetFileId();
  }



