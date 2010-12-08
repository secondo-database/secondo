

package tools;


import java.io.File;
import java.util.Map;
import java.util.HashMap;

public class Cache<T extends Cacheable, L extends ObjectLoader<T> > {

/** Creates a new cache holding maxMem bytes.**/
  public Cache(int maxMem, L loader){
    this.maxMem = Math.max(1024,maxMem);
    map = new HashMap<File,T>();
    currentMem = 0;
    this.loader = loader;
    this.lru = new LRU<File>();
  }

/** Returns the element for the given File.
  **/
  public T getElem(File f){
     if(f==null){
        return null;
     }
     if(map.containsKey(f)){
       lru.use(f);   // bring to from of lru
       return map.get(f);
     }
     T value = loader.loadFromFile(f);

     if(value.getUsedMem() > maxMem){
         // do not cache values exceeding the available memory for this cache
         return value;
     }
     insert(f,value);
     return value;
  }

 /** Inserts a new pair into the cache **/
  private void insert(File f,T value){
      map.put(f,value); 
      lru.use(f);   // add a new use of f 
      currentMem += value.getUsedMem();
      while(currentMem>maxMem){
         deleteLRUElem();
      }
  }


  /** removes the last recently used element from the cache **/
  private void deleteLRUElem(){
      File key = lru.deleteLastElem();
      if(key==null){ // internal error
        lru.clear();
        map.clear();
        currentMem = 0;
      }
      T value = map.remove(key);
      if(value!=null){
          currentMem -= value.getUsedMem();
      } 
  }





 

 private int maxMem;
 private int currentMem;
 private Map<File,T> map; 
 private L loader;
 private LRU<File> lru;

}
