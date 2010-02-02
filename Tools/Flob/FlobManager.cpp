/*


1 Class FlobManager

The only class using the FlobManager class is Flob. For this reason,
all functions are declared as private and Flob is declared to be a friend
of that class.

*/

#include "SecondoSMI.h"
#include <stdlib.h>
#include "Flob.h"
#include "LRU.h"
#include "assert.h"
#include <iostream>
#include "NativeFlobCache.h"
#include "PersistentFlobCache.h"

#undef __TRACE_ENTER__
#undef __TRACE_LEAVE__

#define __TRACE_ENTER__
#define __TRACE_LEAVE__

  /* 
#define __TRACE_ENTER__ std::cerr << "Enter : " << \
        __PRETTY_FUNCTION__ << std::endl;
        
#define __TRACE_LEAVE__ std::cerr << "Leave : " << \
        __PRETTY_FUNCTION__ << "@" << __LINE__ << std::endl;

  */
FlobManager* FlobManager::instance = 0;

static NativeFlobCache* nativeFlobCache = 0;
static PersistentFlobCache* persistentFlobCache = 0;

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
       if(nativeFlobCache){
         delete nativeFlobCache;
         nativeFlobCache = 0;
       }
       if(!nativeFlobFile->Close()){
         std::cerr << "Problem in closing nativeFlobFile" << std::endl;
       }
       if(!nativeFlobFile->Remove()){
         std::cerr << "Problem in deleting nativeFlobFile" << std::endl;
       }
       delete nativeFlobFile;
       nativeFlobFile = 0;
       // close all files stored in Map   
       map<SmiFileId, SmiRecordFile*>::iterator iter;
       for( iter = openFiles.begin(); iter != openFiles.end(); iter++ ) {
         if(iter->first!=nativeFlobs) {      
           iter->second->Close(); 
           delete iter->second;
         }  
       }
       nativeFlobs = 0;
       openFiles.clear();
       // kill persistent Flob cache
       if(persistentFlobCache){
         delete persistentFlobCache;
         persistentFlobCache = 0;
       }
     }


SmiRecordFile* FlobManager::getFile(const SmiFileId& fileId) {
 __TRACE_ENTER__

   if(fileId==nativeFlobs){
     return nativeFlobFile;
   }

  // debug start+
  // if(changed){
  //    cout << "files currently open in FlobManager" << endl;
  //    map<SmiFileId, SmiRecordFile*>::iterator it2 = openFiles.begin();
  //    while(it2 != openFiles.end()){
  //      cout << it2->first << " name = " << it2->second->GetName() << endl;
  //      it2++;
  //   }
  //   changed = false;
  //}

  // debug end
     
  SmiRecordFile* file(0);
  map<SmiFileId, SmiRecordFile*>::iterator it = openFiles.find(fileId);
  if(it == openFiles.end()){
     file = new SmiRecordFile(false);
     openFiles[fileId] = file;
     file->Open(fileId);
     changed = true;
  } else{
     file = it->second;
  }
__TRACE_LEAVE__
  return file;
}

/*
~dropfile~

If Flob data is stored into a file, the flobmanager will
create a new File from the file id and keep the open file.
If the file should be deleted or manipulated by other code,
the flobmanger has to giv up the control over that file.
This can be realized by calling the ~dropFile~ function.

*/
  bool FlobManager::dropFile(const SmiFileId& id){
     __TRACE_ENTER__
     if(id==nativeFlobs){ // never give up the control of native flobs
        return false; 
     }
     map<SmiFileId, SmiRecordFile*>::iterator it = openFiles.find(id);
     if(it!= openFiles.end()){
         SmiRecordFile* file;
         file  = it->second;
         file->Close();
         delete file;
         openFiles.erase(it);
         changed = true;
         __TRACE_LEAVE__
         return true;
      } else{
        return false; // file  not handled by fm
        __TRACE_LEAVE__
      }
 }

 
 bool FlobManager::dropFiles(){
    map<SmiFileId, SmiRecordFile*>::iterator it = openFiles.begin();
    int count = 0;
    while(it!=openFiles.end()){
       if(it->first != nativeFlobs){
          it->second->Close();
          delete it->second;
          count++;
       }
       it++;
    }
    openFiles.clear();
    openFiles[nativeFlobs] = nativeFlobFile;
    changed = count>0;
    return count > 0;
 }


 void FlobManager::clearCaches(){
   if(nativeFlobCache){
     nativeFlobCache->clear();
   }
   if(persistentFlobCache){
     persistentFlobCache->clear();
   }
 }


     
/*
~resize~

Changes the size of flob.

*/
bool FlobManager::resize(Flob& flob, const SmiSize& newSize){

    

    FlobId id = flob.id;
    SmiFileId   fileId =  id.fileId;
    SmiRecordId recordId = id.recordId;
    SmiSize     offset = id.offset;
    SmiSize oldSize = flob.size;

 
    if(oldSize==newSize){
      return true;
    }

    if(persistentFlobCache && fileId != nativeFlobs){
      persistentFlobCache->killLastSlot(flob);
    }

    SmiRecord record;
    SmiRecordFile* file = getFile(fileId);
    bool ok = file->SelectRecord(recordId, record, SmiFile::Update);
    if(!ok){
      cerr << __PRETTY_FUNCTION__ << "@" << __LINE__ 
           << "Select Record failed:" << flob << endl;
      assert(false);
      __TRACE_LEAVE__
      return false;
    }
  //  cout << "Try to resize record to size : " << (offset + newSize) << endl;
    if( record.Resize(offset+newSize)){ 
       record.Finish();
       if(fileId == nativeFlobs && nativeFlobCache){
          nativeFlobCache->resize(flob, newSize);
       }
       flob.size = newSize;
       __TRACE_LEAVE__
  //     cout << "Resize successful" << endl;
       return true;
    } 
    cout << "Resize failed" << endl;
    
    return false;
   
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
         const SmiSize&  size,
         const bool ignoreCache) {  // requested data size
  __TRACE_ENTER__

  FlobId id = flob.id;
  SmiFileId   fileId =  id.fileId;

  if(!ignoreCache){
    if(fileId!=nativeFlobs){
       return persistentFlobCache->getData(flob,dest,offset,size);
    } else {
       if(nativeFlobCache->getData(flob, dest, offset, size)){
         return true;
       }
    }
  }


  SmiRecordId recordId = id.recordId;
  SmiSize     floboffset = id.offset;
  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);

  bool ok = file->SelectRecord(recordId, record);
  if(!ok){
      cerr << __PRETTY_FUNCTION__ << "@" << __LINE__ 
           << "Select Record failed:" << flob << endl;
      assert(false);
    __TRACE_LEAVE__
    return false;
  }

  SmiSize recOffset = floboffset + offset;
  if(record.Size() < recOffset){
     cerr << " try to read at an offset outside the flob size" << endl;
     return false;
  }
  SmiSize mySize = min(size, record.Size()-recOffset); // restrict read to the
                                                       // end of the record
  SmiSize read = record.Read(dest,mySize, recOffset);
  if(read!=mySize){

    cout << "Error in reding data from flob" << endl;
    cout << "read = " << read << endl;
    cout << "mySize = " << mySize << endl;
    cout << "record.Size = " << record.Size() << endl;
    cout << "floboffset = " << floboffset << endl;
    cout << "offset = " << offset << endl;
    assert(false);
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
bool FlobManager::destroy(Flob& victim) {

 __TRACE_ENTER__

   FlobId id = victim.id;
   SmiSize size = victim.getSize();
   SmiRecordId recordId = id.recordId;
   SmiSize offset = id.offset;

   // possible kill flob from persistent flob cache 
   // or wait until cache is removed automatically = current state


   SmiRecordFile* file = getFile(id.fileId);
   SmiRecord record;
   bool ok = file->SelectRecord(recordId, record, SmiFile::Update);  
   if(!ok){ // record not found in file
      cerr << __PRETTY_FUNCTION__ << "@" << __LINE__ 
           << "Select Record failed:" << victim << endl;
      assert(false);
     __TRACE_LEAVE__
     return false; 
   }
   
   if(id.fileId == nativeFlobs){
      // each native flob is exlusive owner of an record
      nativeFlobCache->erase(victim);
      file->DeleteRecord(recordId);
      return true;
   }

   // check whether the flob occupies the whole record
   SmiSize recordSize = record.Size();
   
   if(offset!=0){ // flob doesn't start at the begin of the record
      if( offset + size != recordSize){
         std::cout << "cannot destroy flob, because after the flob data are"
                      " available" << std::endl;
         return false;
        
      } else { // truncate record
         record.Truncate(offset);
         record.Finish();
      }
   } else {
     if(recordSize != size && (id.fileId != nativeFlobs) ){
       std::cout << "cannot destroy flob, because after the flob data are"
                    " available" << std::endl;
       return false;
     } else {
        // record stores only the flob
        file->DeleteRecord(recordId); 
     }
   }
   return true;
  __TRACE_LEAVE__
}



bool FlobManager::destroyIfNonPersistent(Flob& victim) {
   if(victim.id.fileId == nativeFlobs){
      return destroy(victim);
   }
   return true;
}


/*
~saveTo~

Save the content of a flob into a file at a specific position (given as recordId and 
offset). This will result in another Flob which is returned as the result of this 
function. A record with the corresponding id must already exist in the file.

Initial implementation, should be changed to support Flobs larger than the 
available main memory.

*/
bool FlobManager::saveTo(const Flob& src,   // Flob to save
       const SmiFileId fileId,  // target file id
       const SmiRecordId recordId, // target record id
       const SmiSize offset,
       Flob& result)  {   // offset within the record  

 __TRACE_ENTER__
   if(src.size==0){
     __TRACE_LEAVE__
     return false;
   }

   SmiRecordFile* file = getFile(fileId);
   return saveTo(src, file, recordId, offset, result);

}


/*
~saveTo~

Save the content of a flob into a file at a specific position (given as recordId and 
offset). This will result in another Flob which is returned as the result of this 
function. A record with the corresponding id must already exist in the file.

Initial implementation, should be changed to support Flobs larger than the 
available main memory.

*/
bool FlobManager::saveTo(const Flob& src,   // Flob to save
       SmiRecordFile* file,  // target file
       const SmiRecordId& recordId, // target record id
       const SmiSize& offset,
       Flob& result)  {   // offset within the record  

    __TRACE_ENTER__
   //if(src.size==0){
   //  __TRACE_LEAVE__
   //  return false;
   //}

   SmiRecord record;
   if(!file->SelectRecord(recordId, record, SmiFile::Update)){
     __TRACE_LEAVE__
     return false;
   }
   char* buffer = new char[src.size];
   getData(src,buffer,0,src.size);
   // save the data
   SmiSize wsize = record.Write(buffer, src.size, offset);
   record.Finish();
   if(wsize!=src.size){
     delete[] buffer;
     assert(false);
     __TRACE_LEAVE__
     return false;
   }
   FlobId id;
   id.fileId = file->GetFileId();
   id.recordId = recordId;
   id.offset = offset;
   result.id = id;
   result.size = src.size;   
   __TRACE_LEAVE__
   delete[] buffer;
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
   
    SmiRecordFile* file = getFile(fileId);
    return saveTo(src, file, result);
}

 bool FlobManager::saveTo(
             const Flob& src,             // flob to save
             SmiRecordFile* file,      // target file
             Flob& result){         // result



 __TRACE_ENTER__
    SmiRecordId recId;
    SmiRecord rec;
    if(!file->AppendRecord(recId,rec)){
      __TRACE_LEAVE__
      return false;
    }
    if(src.size==0){ // empty Flob
       rec.Finish();
       __TRACE_LEAVE__
       return true;
    }

    // write data
    char* buffer = new char[src.size+1];
    buffer[src.size] = '\0';
    getData(src, buffer, 0, src.size);
    rec.Write(buffer, src.size,0);
    delete [] buffer;
    rec.Finish();
    FlobId fid(file->GetFileId(), recId,0);
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
                          const SmiSize& length,
                          const bool ignoreCache) { // data size

 __TRACE_ENTER__
  FlobId id = dest.id;
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     offset = id.offset;


  if(!ignoreCache){
    if(fileId!=nativeFlobs){
      return persistentFlobCache->putData(dest, buffer, targetoffset, length);
    } else {
      if(nativeFlobCache->putData(dest, buffer, targetoffset, length)){
        return true;
      }
    }
  }

  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);
  bool ok = file->SelectRecord(recordId, record, SmiFile::Update);
  if(!ok){
      cerr << __PRETTY_FUNCTION__ << "@" << __LINE__ 
           << "Select Record failed:" << dest << endl;
      assert(false);
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

   FlobId fid(nativeFlobs, recId, 0);
   result.id = fid;
   result.size = size;


   nativeFlobCache->create(result);

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
    char* buffer = new char[src.size];
    if(!getData(src, buffer, 0, src.size)){
      __TRACE_LEAVE__
      delete[] buffer;
      return false;
    }
    if(dest.size!=src.size){
      resize(dest,src.size);
    }
    if(! putData(dest,buffer,0,src.size)){
      __TRACE_LEAVE__
      delete[] buffer;
      return false;
    }
    delete[] buffer;
    dest.size = src.size;  
    __TRACE_LEAVE__
    return true;
  }


/*
~createFrom~

return a Flob with persistent storage allocated and defined elsewhere

*/      

      Flob FlobManager::createFrom( const SmiFileId& fid,
                                    const SmiRecordId& rid,
                                    const SmiSize& offset, 
                                    const SmiSize& size) {

        Flob flob;
        FlobId flob_id(fid, rid, offset);
        flob.id = flob_id;
        flob.size = size;          
        return  flob;
      };


      void FlobManager::killNativeFlobs(){
        cout << "killNativeFlobs called" << endl;
        if(nativeFlobCache){
           nativeFlobCache->clear();
        }
        bool ok = nativeFlobFile->ReCreate();  
        assert(ok);
      }


/*
~constructor~

Because Flobmanager is a singleton, the constructor should only be used
by the FlobManager class itself.

*/

  FlobManager::FlobManager():openFiles(), changed(true){
     __TRACE_ENTER__
    assert(instance==0); // the constructor should only called one time
    // construct the temporarly FlobFile
    // not fixed size, dummy, temporarly
    nativeFlobFile  = new SmiRecordFile(false,0,true); 

    bool created = nativeFlobFile->Create("NativeFlobFile","Default");
    // bool created = nativeFlobFile->Create();

    assert(created);

    nativeFlobs = nativeFlobFile->GetFileId();
    openFiles[nativeFlobs] = nativeFlobFile;


    size_t maxCache = 64 * 1024 * 1024;
    nativeFlobCache = new NativeFlobCache( maxCache);

    size_t maxPCache = 64 * 1024 * 1024;
    persistentFlobCache = new PersistentFlobCache(maxPCache, 4096);  

    __TRACE_LEAVE__
  }



ostream& operator<<(ostream& os, const FlobId& fid) {
  return fid.print(os); 
}

ostream& operator<<(ostream& os, const Flob& f) {
  return f.print(os);
}


ostream& operator<<(ostream& o, const CacheEntry entry){
   return entry.print(o);
}

