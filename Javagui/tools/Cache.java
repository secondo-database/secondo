

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
    hits = 0;
    failures = 0;
    recentlyReplaced=0;;
  }

/** Returns the element for the given File.
  **/
  public T getElem(File f){
     recentlyReplaced=0;;
     if(f==null){
        return null;
     }
     if(map.containsKey(f)){
       hits++;
       lru.use(f);   // bring to from of lru
       return map.get(f);
     }
     failures++;
     T value = loader.loadFromFile(f);
     if(value==null){
       return null;
     }
     if(value.getUsedMem() > maxMem){
         // do not cache values exceeding the available memory for this cache
         return value;
     }
     insert(f,value);
     return value;
  }

  /** returns the number of chached files */
  public int size(){
     return map.size();
  }

  /** returns the currently used memory **/
  public int getUsedMem(){
     return currentMem;
  }

  /** returns the maximum value of memory for using by this cahce**/
  public int getMaxMem(){
    return maxMem;
  }

 
  /** removes an element from cache **/
  public boolean remove(File f){
     if(!map.containsKey(f)){
        return false;
     }  
     map.remove(f);
     lru.remove(f);
     return true;
  } 


  public String toString(){
    return "Cache[ " + map.size() + " | " + currentMem +" / " + maxMem +"; hits = " + hits +", fails = " + failures +"recentlyReplaced = " +  recentlyReplaced+"]";
  }

  public void clear(){
    map.clear();
    lru.clear();
    currentMem = 0;
    hits = 0;
    failures =0;
  }

  /** returns the number of objects found in cache **/
  public int getHits(){
     return hits;
  }

  /** returns the number of objects not found in cache **/
  public int getFailures() {
     return failures;
  }



 /** Inserts a new pair into the cache **/
  private void insert(File f,T value){
      map.put(f,value); 
      lru.use(f);   // add a new use of f 
      currentMem += value.getUsedMem();
      recentlyReplaced=0;;
      while(currentMem>maxMem){
         deleteLRUElem();
         recentlyReplaced++;
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


  /** Returns the number of elements replaced by the last get Operation **/
  public int getRecentlyReplaced(){
     return recentlyReplaced;
  } 


 

 private int maxMem;
 private int currentMem;
 private Map<File,T> map; 
 private L loader;
 private LRU<File> lru;


 // statistical information
 private int hits;
 private int failures;
 private int recentlyReplaced;

}
