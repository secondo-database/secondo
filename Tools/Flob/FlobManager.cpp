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


/*
~FlobCache~

A class managing caching for (native) Flobs.
This class has insecure operations, this means write operations
will lost after a crash of the system. For that reason. this class
can only handle the native Flobs.

*/
class FlobCache{
  public:

/*
~Constructor~

Creates a new FlobCache.
     
*/
    FlobCache(const SmiFileId _fileId, const size_t _maxSize):
      fileId(_fileId), maxSize(_maxSize), lru(), cache(), size(0){}



/*
~getData~

Returns the data stored in the cache. If the flob is not cached and 
the size is small enough to be fit into the cache, the Flob will be cached.
The function will return false if the flob is too large to fit into the cache.

*/
    bool getData(const Flob& flob,
                 char* dest,
                 const SmiSize offset,
                 const SmiSize size){
       __TRACE_ENTER__
       assert(flob.id.fileId == this->fileId);
      if(!useFlob(flob)){
         __TRACE_LEAVE__
         return false;
       }
       assert(offset+size <= flob.size);
       memcpy(dest, cache[flob].second + offset, size);
       __TRACE_LEAVE__
       return true;
    }

/*
~Destructor~

*/
   ~FlobCache(){
       __TRACE_ENTER__
       clear();
       __TRACE_LEAVE__
   }

/*
~Clear function~

*/
  void clear(){
     __TRACE_ENTER__
     map<Flob, pair<bool, char*> >::iterator it = cache.begin();
     while(it != cache.end()){
       free(it->second.second);
       it++;
     }
     cache.clear();
     lru.clear();
  }


/*
~erase~

This function will remove a flob from the cache.

*/
    bool erase(const Flob& victim){
       __TRACE_ENTER__
      assert(victim.id.fileId == this->fileId);
      map<Flob, pair<bool, char*> >::iterator it;
      it = cache.find(victim);
      if(it==cache.end()){
         __TRACE_LEAVE__
         return false;
      }
      free(it->second.second);   // give up the memory
      size -= victim.size;       // more cache available now
      lru.deleteMember(victim);  // remove from lru structure
      cache.erase(victim);
      __TRACE_LEAVE__
      return true;
    }

/*
~putData~

Write data into a cached Flob. If the flob cannot cached, the 
result will be false.

*/
   bool putData( const Flob& dest,
                 const char* buffer,
                 const SmiSize& targetoffset,
                 const SmiSize& length) {
     __TRACE_ENTER__
     assert(dest.id.fileId == this->fileId);
     assert(targetoffset + length <= dest.size);
     if(!useFlob(dest)){
       return false;
     }
     pair<bool, char*> p = cache[dest];
     char* buf = p.second;
     memcpy(buf+targetoffset , buffer, length);
     p.first = true; // mark as changed
     cache[dest] = p; // fix changes
     __TRACE_LEAVE__
     return true;
   }

/*
This fucntion is called if a flob is created or resized.
If the flob does not fit into the cahce, the result will be
fasle.

*/
bool create(const Flob& flob){
   __TRACE_ENTER__
   __TRACE_LEAVE__
   return useFlob(flob);
}

/*
~resize~

*/
bool resize(const Flob& flob){
    __TRACE_ENTER__
//    cout << "resize " << flob  << endl;
    assert(flob.id.fileId == this->fileId);
    map<Flob, pair<bool, char*> >::iterator it;
    it = cache.find(flob);
    if(it==cache.end()){ // flob not cached
       __TRACE_LEAVE__
       return useFlob(flob);
    } 
    SmiSize oldSize = it->first.size;
    if(flob.size>=maxSize){ // new Size too big
       cout << "Flob resize exceed cache size" << endl;
       if(it->second.first){
         saveChanges(it->first);
       }
       free(it->second.second);
       cache.erase(flob);
       lru.deleteMember(flob);
       size -= oldSize;
       __TRACE_LEAVE__
       return false;
    }
    char* oldCache1 = it->second.second;
    char* oldCache = oldCache1;
    bool changed = it->second.first;
    cache.erase(it);
    oldCache = (char*)realloc(oldCache, flob.size);
    cache[flob] = pair<bool, char*>(changed, oldCache);
    size = (size + flob.size) - oldSize;
    lru.use(flob);
    reduce();
    __TRACE_LEAVE__
    return true;
}
     

  private:
    SmiFileId fileId;
    size_t maxSize;
    LRU<Flob> lru;
    map<Flob, pair<bool, char*> > cache;
    size_t size;    
/*
~useFlob~

If not already cached, the given Flob will be inserted into the cache
except the flob exceeds the maximum available memory for the cache.
Possibly, other Flobs will be displaced. 


*/
    bool useFlob(const Flob& flob){
       __TRACE_ENTER__
       assert(flob.id.fileId == this->fileId);
       map<Flob, pair<bool, char*> >::iterator it;
       it = cache.find(flob);
       if(it==cache.end()){ // flob not cached.
          if(flob.size>=maxSize){
             __TRACE_LEAVE__
           // cout << "single flob exceeds maximum cache size" << endl;
            return false;
          }
          pair<bool, char*> p(true, (char*)malloc(flob.size));
          // get original data from flob manager
          FlobManager::getInstance().getData(flob,p.second, 0, flob.size, true);
          // mark as last used
          lru.use(flob);
          cache[flob] = p;
          __TRACE_LEAVE__
          return true;
       }
       if(it->first.size != flob.size){
         assert(false);
       }
       lru.use(flob);
       __TRACE_LEAVE__
       return true;
    }


/*
~saveChanges~

save the content of the Flobcache to disk.

*/
    bool saveChanges(const Flob& flob){
       __TRACE_ENTER__
       map<Flob, pair<bool, char*> >::iterator it;
       it = cache.find(flob);
       if(it==cache.end()){
         __TRACE_LEAVE__
         return false;
       }
       __TRACE_LEAVE__
       return FlobManager::getInstance().putData(it->first,
                                          it->second.second,
                                          0,
                                          it->first.size,
                                          true);
      
    }

/*
Removes flobs from cache until the size is smaller than or equal to
the cache's maximum size.

*/
    bool reduce(){
      __TRACE_ENTER__

     assert(lru.size() == cache.size());

      Flob victim;
      while(size>maxSize && cache.size()>0){
         assert(lru.size() == cache.size());
         if(!lru.deleteLast(victim)){
           cerr << "Internal error in FlobCache, "
                   "try to delete if cache is empty" 
                << endl;
           assert(false);
         }
         map<Flob, pair<bool, char*> >::iterator it;
         it = cache.find(victim);
         if(it==cache.end()){
           cerr << "internal error in the fLobCache2" << __LINE__ << endl;
           assert(false); // inconsistency between cache and lru
         } else {
           char* buf = it->second.second;
           if(it->second.first){
             saveChanges(victim);
           }
           free(buf);
           cache.erase(victim);
           size -= victim.size;
        }
      }
     assert(lru.size() == cache.size());
      __TRACE_LEAVE__
      return true;
    }


};



static FlobCache* flobCache = 0;

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
       if(flobCache){
         delete flobCache;
         flobCache = 0;
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

     }


SmiRecordFile* FlobManager::getFile(const SmiFileId& fileId) {
 __TRACE_ENTER__

   if(fileId==nativeFlobs){
     return nativeFlobFile;
   }

  // debug start+
   if(changed){
      cout << "files currently open in FlobManager" << endl;
      map<SmiFileId, SmiRecordFile*>::iterator it2 = openFiles.begin();
      while(it2 != openFiles.end()){
        cout << it2->first << " name = " << it2->second->GetName() << endl;
        it2++;
     }
     changed = false;
  }

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

    SmiRecord record;
    SmiRecordFile* file = getFile(fileId);
    bool ok = file->SelectRecord(recordId, record, SmiFile::Update);
    if(!ok){
      cout << "Select Record failed" << endl;
      __TRACE_LEAVE__
      return false;
    }
  //  cout << "Try to resize record to size : " << (offset + newSize) << endl;
    if( record.Resize(offset+newSize)){ 
       record.Finish();
       flob.size = newSize;
       if(fileId == nativeFlobs && flobCache){
          flobCache->resize(flob);
       }
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

  if(fileId == nativeFlobs && !ignoreCache){
     if(flobCache->getData(flob, dest, offset, size)){
        return true;
     }
  }


  SmiRecordId recordId = id.recordId;
  SmiSize     floboffset = id.offset;
  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);

  bool ok = file->SelectRecord(recordId, record);
  if(!ok){
    __TRACE_LEAVE__
    return false;
  }

  SmiSize recOffset = floboffset + offset;
  SmiSize mySize = min(size, record.Size()-recOffset); // restrict read to the
                                                       // end of the record
  SmiSize read = record.Read(dest,mySize, recOffset);
  if(read!=mySize){
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

   if(id.fileId == nativeFlobs){
      flobCache->erase(victim);
   }


   SmiRecordFile* file = getFile(id.fileId);
   SmiRecord record;
   bool ok = file->SelectRecord(recordId, record, SmiFile::Update);  
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
     if(recordSize != size && (id.fileId != nativeFlobs) ){
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
   if(src.size==0){
     __TRACE_LEAVE__
     return false;
   }

   SmiRecord record;
   if(!file->SelectRecord(recordId, record, SmiFile::Update)){
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
   id.fileId = file->GetFileId();
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
    char buffer[src.size+1];
    buffer[src.size] = '\0';
    getData(src, buffer, 0, src.size);
    rec.Write(buffer, src.size,0);
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

  if(fileId == nativeFlobs){
     if(!ignoreCache){
        if(flobCache->putData(dest, buffer, targetoffset, length)){
           return true;
        } 
     } 
  }


  SmiRecord record;
  SmiRecordFile* file = getFile(fileId);
  bool ok = file->SelectRecord(recordId, record, SmiFile::Update);
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

   FlobId fid(nativeFlobs, recId, 0);
   result.id = fid;
   result.size = size;


   flobCache->create(result);

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
    if(dest.size!=src.size){
      resize(dest,src.size);
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

    bool created = nativeFlobFile->Create();

    assert(created);

    nativeFlobs = nativeFlobFile->GetFileId();
    openFiles[nativeFlobs] = nativeFlobFile;

    cout << "nativeFlobs : " << nativeFlobs << endl;

    size_t maxCache = 64 * 1024 * 1024;
    flobCache = new FlobCache(nativeFlobs, maxCache);
   // flobCache = new FlobCache(nativeFlobs, 0);
 

    __TRACE_LEAVE__
  }



ostream& operator<<(ostream& os, const FlobId& fid) {
  return fid.print(os); 
}

ostream& operator<<(ostream& os, const Flob& f) {
  return f.print(os);
}

