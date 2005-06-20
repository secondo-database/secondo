package sj.lang;

import com.sleepycat.je.*;
import com.sleepycat.bind.tuple.LongBinding;
import java.io.File;


/**
This Class provides an array of inifinity size

*/

public class DBArray{

/**
Create a new DBArray with given cachesize. The cachsize determines 
not the size of the cahc in bytes bit in entries. Data greater than 
maxDataSize are not cached.

*/
public DBArray(int cacheSize, int maxDataSize){
    if(cacheSize<0)  cacheSize=1;
    // if this is the first dbarray => initialize a environment
    if(environment==null){
        initEnvironment();
    }
    initDatabase();
    this.cacheSize = cacheSize;
    this.maxDataSize = maxDataSize+1;
    this.cache = new Entry[cacheSize];
    // init the cache
    for(int i=0;i<cacheSize;i++)
      cache[i]=null;
}

/**
Puts an new entry in the given array slot 

*/
public boolean put(long key, byte[] data){
  int cacheIndex = (int)(key%cacheSize);
  // check whether this key is cashed
  Entry CE = cache[cacheIndex];
  if(data.length<maxDataSize){
     if(CE==null){ // slot is free
        CE = new Entry(key,data);
        CE.update=true;
        cache[cacheIndex] = CE;
        return true;
     }

     if(CE.key==key){ // update in cache
        CE.data=data;
        CE.update=true;
        return true;
     }else{
        boolean res;
        if(CE.update)
           res = writeToDatabase(CE); // swap to disk
        else // cache and dba are equal
           res = true;
        // update values in cache
        CE.key = key;
        CE.data=data;
        CE.update=true;
        return res;
     }
  }else{ // caching not allowed
      if((CE!=null) && (CE.key==key)){// remove from cache
         cache[cacheIndex] = null;
      }
      return writeToDatabase(key,data);
  }
}

/**
Gets the data from an array slot.

*/
public byte[] get(long key){
    int cacheIndex = (int)(key%cacheSize);
    Entry CE = cache[cacheIndex];
    if(CE!=null && CE.key==key){ // found in cache
       return CE.data;
    }else{
       Entry DBE = readFromDatabase(key);
       if(DBE==null) // error occured
          return null;
       if(DBE.data.length<maxDataSize){ // put in cache
          if(CE==null){
             // no entry in cache
             cache[cacheIndex] = DBE;
             DBE.update = false; // equal values in cache and db
             return DBE.data; 
          } else{ // cache occupied
             if(CE.update)
                if(!writeToDatabase(CE)) // error occured
                   return null;
             cache[cacheIndex] = DBE;
             return DBE.data; 
          }
       } else{ // data to big for caching
         return DBE.data;
       }  
    }
}


/**
Deletes an entry from the array

*/
public boolean delete(long key){
   int cacheIndex = (int)(key%cacheSize);
   Entry CE = cache[cacheIndex];
   if(CE!=null && CE.key==key)
      cache[cacheIndex]=null;
   return deleteFromDatabase(key); 
}


/**
Checks whether the entry is inside the cache

*/
public boolean isCached(long key){
   Entry CE = cache[(int)(key%cacheSize)];
   return (CE!=null && CE.key==key);
}



/*
private methods 


This method creates the berkeley db environment

*/
private boolean initEnvironment(){
   try{
     String Dir = System.getProperties().getProperty("user.dir");
     int Number = 0;
     Dir += "/TMP_DBArray_";
     File DBDir = new File(Dir+Number);
     Number++;
     while(DBDir.exists()){
        DBDir = new File(Dir+Number);
        Number++;
     }
     System.out.println("Creating new Directory "+Dir +Number+" for DBArrays");
     DBDir.mkdir();  // create a directory if not exists
     EnvironmentConfig envConf = new EnvironmentConfig();
     envConf.setTransactional(true); 
     envConf.setAllowCreate(true);
     environment=new Environment(DBDir,envConf);
     return true; 
    } catch(Exception e){
       if(DEBUG_MODE){
            e.printStackTrace();
       }
       return false; 
    }
}

/**
This method create a Berkeley-DB database for using as 
background memory.

*/
private boolean initDatabase(){
  try{ 
     DatabaseConfig dbConf = new DatabaseConfig();
     dbConf.setTransactional(false);
     dbConf.setAllowCreate(true);
     dbConf.setSortedDuplicates(false);
     DBName = "DB_Array_"+DBNumber;
     database = environment.openDatabase(null,DBName,dbConf);
     DBNumber++; 
     return true; 
  } catch (DatabaseException e){
     if(DEBUG_MODE)
        e.printStackTrace();
     return false;
  }
}

/**
If this dbarray is not longer needed, the appropriate 
database is removed.

*/
public void finalize() throws Throwable{
  try{
      environment.removeDatabase(null,DBName);
  } catch(Exception e){}
  super.finalize();
}

/**
This method write an entry to the background memory.
If there is already an entry with the same key, this
entry will be overwritten.

*/
private boolean writeToDatabase(Entry e){
   return writeToDatabase(e.key,e.data);
}

/**
This method write a key data pair into the database.
If the key exists, the data will be overwritten.

*/
private boolean writeToDatabase(long key, byte[] data){
   try{
       DatabaseEntry k= new DatabaseEntry();
       DatabaseEntry d = new DatabaseEntry(data);
       LongBinding.longToEntry(key,k);
       if(database.put(null,k,d)!=OperationStatus.SUCCESS)
          return false;
       else
          return true;
   }catch(DatabaseException e){
      if(DEBUG_MODE)
          e.printStackTrace();
      return false;
   }
}


/**
This function read an entry with the key given as argument
from the database.
  
*/
private Entry readFromDatabase(long key){
   try{
     DatabaseEntry k = new DatabaseEntry();
     DatabaseEntry d = new DatabaseEntry();
     LongBinding.longToEntry(key,k);
     if(database.get(null,k,d,LockMode.DIRTY_READ)!=OperationStatus.SUCCESS)
          return null;
     return new Entry(key,d.getData());
   } catch(DatabaseException e){
        if(DEBUG_MODE)
          e.printStackTrace();
        return null;
   }
}

/**
This function deletes an entry from the database.

*/
private boolean deleteFromDatabase(long key){
  try{
    DatabaseEntry k = new DatabaseEntry();
    LongBinding.longToEntry(key,k);
    database.delete(null,k);
    return true;
  } catch(DatabaseException e){
      if(DEBUG_MODE)
         e.printStackTrace();
      return false;
  }
}


/* 
Private members

*/

private int cacheSize;
private int maxDataSize;
private String DBName;

private boolean DEBUG_MODE=true;
private Entry[] cache; 

/** The Database environment */
private static Environment environment=null;
private static int DBNumber = 0;
private Database database=null;


private class Entry{
   public Entry(long key,byte[] data){
      this.key = key;
      this.data=data;
      update = true;
   }
   long key;
   byte[] data;
   boolean update; // flag specifiing whether on cache are newer data 
                   // than in the database
} 

}
