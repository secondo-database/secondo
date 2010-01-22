
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
       size -= it->first.size;
       free(it->second.second);
       it++;
     }
     cache.clear();
     lru.clear();
     if(size!=0){
        cerr << "There is an error in computing the currently "
             << "used size of the flobcahe!!!" << endl;
        cerr << " size = " << size << " but should be 0" << endl;
        size = 0;
     }
  }


/*
~erase~

This function will remove a flob from the cache. 
Without storing any changes.

*/
    bool erase(const Flob& victim){
       __TRACE_ENTER__
      assert(victim.id.fileId == this->fileId);
      map<Flob, pair<bool, char*> >::iterator it;
      it = cache.find(victim);
      if(it==cache.end()){ // not stored
         __TRACE_LEAVE__
         return false;
      }
      free(it->second.second);   // give up the memory
      size -= it->first.size;    // more cache available now
      lru.deleteMember(victim);  // remove from lru structure
      cache.erase(victim);
      __TRACE_LEAVE__
      return true;
    }

/*
~eraseFromCache~

This function removes a flob from the cache storing any changes if required.

*/
  bool eraseFromCache(const Flob& victim){
    return saveChanges(victim) & erase(victim);
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
     if(!useFlob(dest)){ // flob not cached
       return false;
     }
     map<Flob, pair<bool, char*> >::iterator it;
     it = cache.find(dest);
     assert(it!=cache.end());
     pair<bool, char*> p = it->second;
     char* buf = p.second;
     memcpy(buf+targetoffset , buffer, length);
     p.first = true; // mark as changed
     cache[dest] = p; // fix changes
     __TRACE_LEAVE__
     return true;
   }

/*
This fucntion is called if a flob is created or resized.
If the flob does not fit into the cache, the result will be
false.

*/
bool create(const Flob& flob){
   __TRACE_ENTER__
   bool res = useFlob(flob);
   __TRACE_LEAVE__
   return res;
}

/*
~resize~

resizes the  memory cached for the given flob

*/
bool resize(const Flob& flob){
    __TRACE_ENTER__
    assert(flob.id.fileId == this->fileId);
    map<Flob, pair<bool, char*> >::iterator it;
    it = cache.find(flob);
    if(it==cache.end()){ // flob not cached
       __TRACE_LEAVE__
       return useFlob(flob); // try to cache the flob
    } 
    SmiSize oldSize = it->first.size;
    if(flob.size>=maxSize){ // new Size too big, 
                            // delete it from cache
       eraseFromCache(flob); 
       __TRACE_LEAVE__
       return false;
    }
    if(flob.size == 0){ // never cache 0 size flobs
       eraseFromCache(flob);
       __TRACE_LEAVE__
       return false;
    }

    // really resize
    char* oldCache1 = it->second.second;
    char* oldCache = oldCache1;
    bool changed = it->second.first;
    cache.erase(it);
    oldCache = (char*)realloc(oldCache, flob.size); // realloc
    cache[flob] = pair<bool, char*>(changed, oldCache);
    size = (size + flob.size) - oldSize; // recompute size
    lru.use(flob);
    reduce(); // reduce possible overflowed cache
    __TRACE_LEAVE__
    return true;
}
     

  private:
    SmiFileId fileId;
    size_t maxSize;
    LRU<Flob> lru;
    map<Flob, pair<bool, char*> > cache;
    size_t size;    


    bool checkSize(){ // debugging function, checks whether the stored size
                      // is the sum of all flob sizes.

       map<Flob, pair<bool, char*> >::iterator it;
       it = cache.begin();
       unsigned int s = 0;
       while(it != cache.end()){
         s += it->first.size;
         it++;
       }

       if(s!=size){
          cout << "CHeckSize failed!!!"  << endl
               << "computed size = " << s << endl
               << "stored size = " << size << endl
               << "difference = " << ((int)s - (int)size) << endl;
               
       }

       return s == size;
    }


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
          if(flob.size==0){
            return false;
          }
          pair<bool, char*> p(true, (char*)malloc(flob.size));
          // get original data from flob manager
          FlobManager::getInstance().getData(flob,p.second, 0, flob.size, true);
          // mark as last used
          lru.use(flob);
          cache[flob] = p;
          size += flob.size;
          if(size>maxSize){
            reduce();
          }
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

      bool changed = false;
      Flob victim;
      while(size>maxSize && cache.size()>0){
         changed = true;
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
           int delSize = it->first.size; // lru does not store correct size
           char* buf = it->second.second;
           if(it->second.first){
             saveChanges(victim);
           }
           free(buf);
           cache.erase(victim);
           size -= delSize;
        }
      }
     assert(lru.size() == cache.size());
      __TRACE_LEAVE__
      return true;
    }


};

