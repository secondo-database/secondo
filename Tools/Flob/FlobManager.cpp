/*


1 Class FlobManager

The only class using the FlobManager class is Flob. For this reason,
all functions are declared as private and Flob is declared to be a friend
of that class.

*/

#include "SecondoSMI.h"
#include <stdlib.h>
#include <utility>
#include "Flob.h"
#include "assert.h"
#include <iostream>
#include "NativeFlobCache.h"
#include "PersistentFlobCache.h"
#include "Stack.h"
#include <fstream>
#include "ExternalFileCache.h"

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
static Stack<Flob>* DestroyedFlobs=0;

static ExternalFileCache* externalFileCache = 0;

 // some value for cache sizes
static size_t NATIVE_CACHE_MAXSIZE  = 64 * 1024 * 1024;
static size_t NATIVE_CACHE_SLOTSIZE = 16 * 1024 * 1024;
static size_t NATIVE_CACHE_AVGSIZE  = 512;


static size_t PERSISTENT_CACHE_MAXSIZE  = 64 * 1024 * 1024;
static size_t PERSISTENT_CACHE_SLOTSIZE = 16 * 1024 * 1024;
static size_t PERSISTENT_CACHE_AVGSIZE  = 512;

static size_t FILEID_CACHE_MAXSIZE  = 64 * 1024 * 1024;

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
       map< pair<SmiFileId, bool>, SmiRecordFile*>::iterator iter;
       for( iter = openFiles.begin(); iter != openFiles.end(); iter++ ) {
         if(iter->first.first!=nativeFlobs && iter->first.second) {      
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

       DestroyedFlobs->makeEmpty();
       delete DestroyedFlobs;
       DestroyedFlobs=0;

       if (externalFileCache){
         delete externalFileCache;
         externalFileCache = 0;
       }
     }


SmiRecordFile* FlobManager::getFile(const SmiFileId& fileId, const char mode) {
 __TRACE_ENTER__

   assert( (mode==0) || (mode == 1) || (mode == 2));
   bool isTemp = (mode!=0);
   if(fileId==nativeFlobs && isTemp){
     return nativeFlobFile;
   }

   SmiRecordFile* file(0);
   pair<SmiFileId, bool> finder(fileId,isTemp);
   map< pair<SmiFileId,bool>, SmiRecordFile*>::iterator it = 
         openFiles.find(finder);
   if(it == openFiles.end()){ // file not found
     file = new SmiRecordFile(false, 0, isTemp);
     openFiles[finder] = file;
     bool openOk = file->Open(fileId);
     assert(openOk);
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
the flobmanger has to give up the control over that file.
This can be realized by calling the ~dropFile~ function.

*/
  bool FlobManager::dropFile(const SmiFileId& id, const char mode){
     if( (mode==2) || (mode==3)){
        return false; // never drop non berkeley db files
     }

     bool isTemp = mode==1; 
     __TRACE_ENTER__
     if(isTemp && id==nativeFlobs){ //never give up the control of native flobs
        return false; 
     }
     pair<SmiFileId,bool> finder(id,isTemp);
     map< pair<SmiFileId,bool>, SmiRecordFile*>::iterator it = 
            openFiles.find(finder);
     if(it!= openFiles.end()){
         SmiRecordFile* file;
         file  = it->second;
         file->Close();
         delete file;
         openFiles.erase(it);
         __TRACE_LEAVE__
         return true;
      } else{
        return false; // file  not handled by fm
        __TRACE_LEAVE__
      }
 }

 
 bool FlobManager::dropFiles(){
    map< pair<SmiFileId, bool>, SmiRecordFile*>::iterator it = 
           openFiles.begin();
    int count = 0;
    while(it!=openFiles.end()){
       if(it->first.first != nativeFlobs || !it->first.second){
          it->second->Close();
          delete it->second;
          count++;
       }
       it++;
    }
    openFiles.clear();
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


 bool FlobManager::makeControllable(Flob& flob){

    if(flob.dataPointer){

      assert(flob.id.isDestroyed());
      // found an evil flob, create a nice one from it

      char* dp = flob.dataPointer;
      flob.dataPointer = 0;
     
      if(!create(flob.size,flob)){
         assert(false);
         return false;
      }

      if(! putData(flob, dp, 0,flob.size,false)){
       assert(false);
       return false;
      }  
      free(dp);
    }
    return true;
}

void FlobManager::createFromBlock(Flob& result, const char* buffer, 
                                 const SmiSize& size, const bool autodestroy){
   if(autodestroy){
     result.id.destroy();    
   }
   assert(result.id.isDestroyed());
   assert(result.dataPointer == 0);
   result.dataPointer = (char*) malloc(size);
   result.size = size;
   memcpy(result.dataPointer, buffer, size);
}


     
/*
~resize~

Changes the size of flob.

*/
bool FlobManager::resize(Flob& flob, const SmiSize& newSize, 
                         const bool ignoreCache/*=false*/){

    if(flob.dataPointer){
      makeControllable(flob);
    }

    if(newSize==flob.size){
       return true;
    }

    assert(!flob.id.isDestroyed());
    FlobId id = flob.id;
    assert( (id.mode==0) || (id. mode==1));  // don't allow resizing of 
                                             // non berkeley db flobs
    SmiFileId   fileId =  id.fileId;
    SmiRecordId recordId = id.recordId;
    SmiSize     offset = id.offset;
    bool        isTemp = id.mode == 1;

    if(!isTemp || (fileId != nativeFlobs)){
      // the allocated memory for the slot may be too small now
      cerr << "Warning resize a persistent Flob" << endl;
      persistentFlobCache->killLastSlot(flob);
    }

    if(isTemp &&  (fileId == nativeFlobs) && !ignoreCache){
      return nativeFlobCache->resize(flob, newSize);
    }

    // resize the record containing the flob
    SmiRecord record;
    SmiRecordFile* file = getFile(fileId,id.mode);


    bool ok = file->SelectRecord(recordId, record, SmiFile::Update);
    if(!ok){
      cerr << __PRETTY_FUNCTION__ << "@" << __LINE__ 
           << "Select Record failed:" << flob << endl;
      assert(false);
      __TRACE_LEAVE__
      return false;
    }
    if(record.Size() != newSize){
      if( record.Resize(offset+newSize)){ 
         record.Finish();
         flob.size = newSize;
         __TRACE_LEAVE__
         return true;
      } 
    } else {
       return true;
    }
    cerr << "Resize failed" << endl;
    
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
         const bool ignoreCache /* = false*/ ) {  // requested data size
  __TRACE_ENTER__

  assert(offset+size <= flob.size);
  if(size==0){
    return true;
  }
  if(flob.dataPointer){
    memcpy(dest, flob.dataPointer + offset, size);
    return true;
  }

  assert(!flob.id.isDestroyed());

  FlobId id = flob.id;
  assert((id.mode==0) || (id.mode==1) || (id.mode == 2));

  if (id.mode < 2)
  {
    SmiFileId   fileId =  id.fileId;
    bool isTemp = id.mode==1;

    if(!ignoreCache){
      if(fileId!=nativeFlobs || !isTemp){
         return persistentFlobCache->getData(flob,dest,offset,size);
      } else {
        return nativeFlobCache->getData(flob, dest, offset, size);
      }
    }

    // retrieve data from disk
    SmiRecordId recordId = id.recordId;
    SmiSize     floboffset = id.offset;
    SmiRecord record;
    SmiRecordFile* file = getFile(fileId,id.mode);

    SmiSize recOffset = floboffset + offset;

    SmiSize actRead;
    bool ok = file->Read(recordId, dest, size, recOffset, actRead);

    if(!ok){
      cerr << " error in getting data from flob " << flob << endl;
      cerr << " actSize = " << actRead << endl;
      cerr << "try to read = " << size << endl;
      string err;
      SmiEnvironment::GetLastErrorCode( err );
      cerr << " err "<< err << endl;

    }
    assert(ok);

    if(actRead!= size&& file == nativeFlobFile){
      return true;
    }
    //assert(actRead == size);
  }
  else if (id.mode == 2)
  {
    // retrieve data from external disk file
    SmiRecordId recordId = id.recordId;
    SmiSize     flobOffset = id.offset;
    SmiSize recOffset = flobOffset + offset;
    SmiSize actRead;

    ifstream* tupleFile = externalFileCache->getFile(recordId);

/*
Each time read part data within the Flob, decided by the recOffset and the size

*/
    tupleFile->seekg(recOffset, ios::beg);
    tupleFile->read(dest, size);

    SmiSize curr = tupleFile->tellg();
    actRead =  curr - recOffset;
    if (actRead == size)
      return true;
    else{
      cerr << " error in getting data from flob " << flob << endl;
      cerr << " mode = 2" << endl;
      cerr << " flobOffset = " << flobOffset << endl;
      cerr << " actSize = " << actRead << endl;
      cerr << " try to read = " << size << endl;
    }
  }
  else if (id.mode == 3)
  {
    // todo: something about the PSLocation
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

    if(victim.dataPointer){
      assert(victim.id.isDestroyed()); // should not have a valid id
      free(victim.dataPointer);
      victim.dataPointer = 0;
      return true; 
    }

    FlobId id = victim.id;
    if( (id.mode==2) || (id.mode == 3)){
      cerr << "FlobManager::destroy(Flob& victim)" << endl;
        // do not destroy flob data within non berkeley db files.
        victim.id.destroy();
        return true;
    }


    assert(!victim.id.isDestroyed());
   bool isTemp = id.mode == 1;
   if(victim.id.fileId == nativeFlobs && isTemp){
      nativeFlobCache->erase(victim); // delete from cache
      DestroyedFlobs->push(victim);
     // if(DestroyedFlobs->getSize() > 64000){
     //   cerr << "Stack of destroyed flobs reaches a critical size " << endl;
     // }
      victim.id.destroy();
      return true;
   }


   SmiSize size = victim.getSize();
   SmiRecordId recordId = id.recordId;
   SmiSize offset = id.offset;

   // possible kill flob from persistent flob cache 
   // or wait until cache is removed automatically = current state


   SmiRecordFile* file = getFile(id.fileId,id.mode);
   SmiRecord record;
   bool ok = file->SelectRecord(recordId, record, SmiFile::Update);  
   if(!ok){ // record not found in file
      cerr << __PRETTY_FUNCTION__ << "@" << __LINE__ 
           << "Select Record failed:" << victim << endl;
      assert(false);
     __TRACE_LEAVE__
     victim.id.destroy();
     return false; 
   }
   
   if(id.fileId == nativeFlobs && isTemp){
      // each native flob is exlusive owner of an record
      nativeFlobCache->erase(victim);
      file->DeleteRecord(recordId);
      victim.id.destroy();
      return true;
   }

   // check whether the flob occupies the whole record
   SmiSize recordSize = record.Size();
   
   if(offset!=0){ // flob doesn't start at the begin of the record
      if( offset + size != recordSize){
         std::cout << "cannot destroy flob, because after the flob data are"
                      " available" << std::endl;
         victim.id.destroy();
         return false;
        
      } else { // truncate record
         record.Truncate(offset);
         record.Finish();
      }
   } else {
     if((recordSize != size) && (id.fileId != nativeFlobs) ){
       std::cout << "cannot destroy flob, because after the flob data are"
                    " available" << std::endl;
       victim.id.destroy();
       return false;
     } else {
        // record stores only the flob
        file->DeleteRecord(recordId); 
     }
   }
   victim.id.destroy();
   return true;
  __TRACE_LEAVE__
}



bool FlobManager::destroyIfNonPersistent(Flob& victim) {
   if(victim.id.fileId == nativeFlobs && victim.id.mode == 1){
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
       const char mode,
       Flob& result)  {   // offset within the record  

 __TRACE_ENTER__
   assert(fileId != nativeFlobs);
   assert((mode==0) || (mode==1)); // no non-berkeley files 

   SmiRecordFile* file = getFile(fileId, mode);
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
   assert(file->GetFileId() != nativeFlobs || !file->IsTemp());

   char* buffer = new char[src.size];
   getData(src,buffer,0,src.size);

   SmiSize written;
   bool ok = file->Write(recordId, buffer, src.size, offset, written);
   assert(ok);
   delete[] buffer;
  

   char mode = file->IsTemp()?1:0; 
   FlobId id(file->GetFileId(), recordId, offset, mode);
   result.id = id;
   result.size = src.size;  
   if(result.dataPointer){
     free(result.dataPointer);
     result.dataPointer = 0;
   } 
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
             const char mode,           // environment
             Flob& result){         // result
    assert(fileId != nativeFlobs);
    assert( (mode==0) || (mode==1)); 
    SmiRecordFile* file = getFile(fileId,mode);
    return saveTo(src, file, result);
}

 bool FlobManager::saveTo(
             const Flob& src,             // flob to save
             SmiRecordFile* file,      // target file
             Flob& result){         // result



 __TRACE_ENTER__
    assert(file->GetFileId() != nativeFlobs || !file->IsTemp());
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
    char mode = file->IsTemp()?1:0;

    FlobId fid(file->GetFileId(), recId,0,mode);
    result.id = fid;
    result.size = src.size;
    if(result.dataPointer){
      free(result.dataPointer);
      result.dataPointer = 0;
    }
    __TRACE_LEAVE__
    return true;
 }

/*
~putData~

Puts data into a flob. 

*/

bool FlobManager::putData(Flob& dest,         // destination flob
                          const char* buffer, // source buffer
                          const SmiSize& targetoffset, // offset within the Flob
                          const SmiSize& length,
                          const bool ignoreCache) { // data size

  __TRACE_ENTER__
  if(dest.dataPointer){
    makeControllable(dest);
  }  

  assert(!dest.id.isDestroyed());
  FlobId id = dest.id;
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     offset = id.offset;

  assert(targetoffset + length <= dest.size);

  // do not allow putting data into non berkeley db flobs
  assert((id.mode==0) || (id.mode==1));

  bool isTemp = id.mode == 1;

  if(!ignoreCache){
    if(fileId!=nativeFlobs || !isTemp){
      cerr << "Warning maipulate a perisistent Flob" << endl;
      return persistentFlobCache->putData(dest, buffer, targetoffset, length);
    } else {
      return nativeFlobCache->putData(dest, buffer, targetoffset, length);
    }
  }

  // put data to disk
  SmiRecordFile* file = getFile(fileId,id.mode);
  SmiSize written;
  bool ok = file->Write(recordId, buffer, length, 
                        offset+targetoffset, written); 
  assert(ok);
  return true;
}

void FlobManager::changeMode(Flob* flob, const char mode)
{
  flob->id.mode = mode;
}


bool FlobManager::putData(const FlobId& id,         // destination flob
                          const char* buffer, // source buffer
                          const SmiSize& targetoffset, // offset within the Flob
                          const SmiSize& length
                         ) { // data size

  __TRACE_ENTER__
  assert(!id.isDestroyed());
  SmiFileId   fileId =  id.fileId;
  SmiRecordId recordId = id.recordId;
  SmiSize     offset = id.offset;

  // avoid putting data into non-berkeley db flobs
  assert((id.mode==0) || (id.mode==1) || (id.mode==2));
   
  SmiRecordFile* file = getFile(fileId,id.mode);
  SmiSize written;
  bool ok = file->Write(recordId, buffer, length , 
                        offset + targetoffset , written);
  assert(ok);
  return true;
}

bool FlobManager::setExFile(Flob& flob, const string& flobFile,
    const SmiSize size, const SmiSize flobOffset)
{
  __TRACE_ENTER__

  flob.id.destroy();
  assert(flob.id.isDestroyed());

  if (!create(size, flob)){
    assert(false);
    return false;
  }

  SmiRecordId recordId = flob.id.recordId;
  externalFileCache->cacheRecord(recordId, flobFile);

  flob.id.mode = 2;
  flob.id.offset = flobOffset;

  __TRACE_LEAVE__
  return true;
}


bool FlobManager::SwitchToMode1(Flob& flob, const string& flobFile,
    const SmiSize size, const SmiSize flobOffset)
{
  __TRACE_ENTER__

  if (flob.id.mode < 2)
    return true;
  assert(!flobFile.empty());
  assert(flob.id.mode == 2 || flob.id.mode == 3);

  Flob newFlob(size);

  //Read data from the disk file
  ifstream* tupleFile = 0;
  if (flob.id.mode == 3){
/*
In Mode 3, the record id is used to identify the remote DS,
it is impossible to cache the file input stream based on its value.
Therefore, we use the record id from the newly created flob structure.

*/
    SmiRecordId recordId = newFlob.id.recordId;
    externalFileCache->cacheRecord(recordId, flobFile, true);
    tupleFile = externalFileCache->getFile(recordId);
  }
  else {
    // mode 2
    SmiRecordId recordId = flob.id.recordId;
    tupleFile = externalFileCache->getFile(recordId);
  }

  tupleFile->seekg(flobOffset, ios::beg);
  char flobBlock[size];
  tupleFile->read(flobBlock, size);

  if ( (SmiSize)tupleFile->gcount() != size){
    cerr << "Error!! read " << tupleFile->gcount() 
         << " from " << flobFile 
         << " at " << flobOffset 
         << ", need " << size << endl;
    assert(false);
  }

  //Cache data into nativeFlobCache
  newFlob.write(flobBlock, size);
  flob = newFlob;

  __TRACE_LEAVE__
  return true;
}


/*
~create~

Creates a new Flob with a given size which is assigned to a temporal file.

Warning: this function does not change the dataPointer.


*/

 bool FlobManager::create(const SmiSize size, Flob& result) {  // result flob
 __TRACE_ENTER__
   SmiRecord rec;
   SmiRecordId recId;
  
  if(size > 536870912){ // 512 MB
    cerr << "Warning try to cretae a very big flob , size = " << size <<endl;
  }

   if(!DestroyedFlobs->isEmpty()){
     result = DestroyedFlobs->pop();
     result.size = size;
     return true;
   }
 

   // create a record from the flob to get an id 
   if(!(nativeFlobFile->AppendRecord(recId,rec))){
      __TRACE_LEAVE__
      return false;
   }

   FlobId fid(nativeFlobs, recId, 0,1);
   result.id = fid;
   result.size = size;

   if(!nativeFlobCache->create(result)){
      if(size>0){
         resize(result,size,true);
      }
   }

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
             const char mode,
             const SmiSize& size,
             Flob& result){       // initial size of the Flob

 __TRACE_ENTER__
   assert((mode==0) || (mode==1));

   bool isTemp = mode==1;
   assert(fileId != nativeFlobs || !isTemp);
   SmiRecordFile* file = getFile(fileId, mode);
   SmiRecord record;
   if(!file->SelectRecord(recordId,record)){
     __TRACE_LEAVE__
     return false;
   } 
   FlobId fid(fileId, recordId, offset, mode);
   result.id = fid;
   result.size = size;
   if(result.dataPointer){
     free(result.dataPointer);
     result.dataPointer = 0;
   }
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
                                    const char mode, 
                                    const SmiSize& size) {
        //assert(fid!=nativeFlobs);
        Flob flob;
        FlobId flob_id(fid, rid, offset,mode);
        flob.id = flob_id;
        flob.size = size;          
        flob.dataPointer = 0;
        return  flob;
      };


      void FlobManager::killNativeFlobs(){
        if(nativeFlobCache){
           nativeFlobCache->clear();
        }
        bool ok = nativeFlobFile->ReCreate();  
        DestroyedFlobs->makeEmpty();
        assert(ok);
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
    // not fixed size, dummy, temporarly
    nativeFlobFile  = new SmiRecordFile(false,0,true); 

    bool created = nativeFlobFile->Create("NativeFlobFile","Default");
    // bool created = nativeFlobFile->Create();

    assert(created);

    nativeFlobs = nativeFlobFile->GetFileId();


    nativeFlobCache = new NativeFlobCache(NATIVE_CACHE_MAXSIZE, 
                                          NATIVE_CACHE_SLOTSIZE,
                                          NATIVE_CACHE_AVGSIZE);

    persistentFlobCache = new PersistentFlobCache(PERSISTENT_CACHE_MAXSIZE, 
                                          PERSISTENT_CACHE_SLOTSIZE,
                                          PERSISTENT_CACHE_AVGSIZE);

    DestroyedFlobs = new Stack<Flob>();

    externalFileCache = new ExternalFileCache(FILEID_CACHE_MAXSIZE);

    __TRACE_LEAVE__
  }


void FlobManager::SetNativeCache(const size_t maxSize, 
                              const size_t slotSize,
                              const size_t avgSize){
   assert(FlobManager::instance == 0);
   NATIVE_CACHE_MAXSIZE = maxSize;
   NATIVE_CACHE_SLOTSIZE = slotSize;
   NATIVE_CACHE_AVGSIZE = avgSize;
 }

void FlobManager::SetPersistentCache(const size_t maxSize, 
                              const size_t slotSize,
                              const size_t avgSize){
   assert(FlobManager::instance == 0);
   PERSISTENT_CACHE_MAXSIZE = maxSize;
   PERSISTENT_CACHE_SLOTSIZE = slotSize;
   PERSISTENT_CACHE_AVGSIZE = avgSize;
 }
 


ostream& operator<<(ostream& os, const FlobId& fid) {
  return fid.print(os); 
}

ostream& operator<<(ostream& os, const Flob& f) {
  return f.print(os);
}


ostream& operator<<(ostream& o, const NativeCacheEntry entry){
   return entry.print(o);
}

