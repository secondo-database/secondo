package sj.lang;

import com.sleepycat.je.*;
import com.sleepycat.bind.tuple.LongBinding;
import java.io.File;
import java.util.List;
import java.util.Vector;


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
  writeAccesses++;
  int cacheIndex = (int)(key%cacheSize);
  // check whether this key is cashed
  Entry CE = cache[cacheIndex];
  if(data.length<maxDataSize){
     if(CE==null){ // slot is free
        CE = new Entry(key,data);
        CE.update=true;
        cache[cacheIndex] = CE;
        cached++;
        return true;
     }

     if(CE.key==key){ // update in cache
        if(CE.data.length!=data.length)
            CE.update=true;
        else{
            CE.update=false;
            for(int i=0;i<data.length&&!CE.update;i++)
               CE.update = CE.data[i] == data[i];
        }
        CE.data=data;
        return true;
     }else{ // slot occupied by a different entry
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
         cached--;
      }
      return writeToDatabase(key,data);
  }
}

/**
Gets the data from an array slot.

*/
public byte[] get(long key){
    readAccesses++;
    int cacheIndex = (int)(key%cacheSize);
    Entry CE = cache[cacheIndex];
    if(CE!=null && CE.key==key){ // found in cache
       return CE.data;
    }else{
       Entry DBE = readFromDatabase(key);
       if(DBE==null) // error occured
          return null;
       if(DBE.data.length<maxDataSize){ // put into cache
          if(CE==null){
             // no entry in cache
             DBE.update = false; // equal values in cache and db
             cache[cacheIndex] = DBE;
             return DBE.data; 
          } else{ // cache occupied
             if(CE.update)
                if(!writeToDatabase(CE)) // error occured
                   return null;
             DBE.update=false;
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
   writeAccesses++;
   int cacheIndex = (int)(key%cacheSize);
   Entry CE = cache[cacheIndex];
   if(CE!=null && CE.key==key){
      cache[cacheIndex]=null;
      cached--;
   }
   return deleteFromDatabase(key); 
}


/**
Prints some statistics 

*/
public void printStatistics(){
   System.out.println("*******************************");
   System.out.println("CacheSize: " + cacheSize);
   System.out.println("cached   : " + cached);
   System.out.println("Read     : " + readAccesses);
   System.out.println("DB-read  : " + dbreadAccesses);
   System.out.println("write    : " + writeAccesses);
   System.out.println("DB-write : " + dbwriteAccesses);
   System.out.println("*******************************");
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
     DBDir.deleteOnExit();
     File EnvDir = DBDir;
     System.out.println("Creating new Directory "+EnvDir.getName()+" for DBArrays");
     EnvDir.mkdir();  // create a directory if not exists
     EnvironmentConfig envConf = new EnvironmentConfig();
     envConf.setTransactional(true); 
     envConf.setAllowCreate(true);
     environment=new Environment(EnvDir,envConf);
     // thread removing the temporal directory for the environment.
     rmEnv = new RemoveEnv(environment);
     Thread t = new Thread(rmEnv);
     Runtime.getRuntime().addShutdownHook(t);
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
     rmEnv.addDatabase(database);
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
      // we con't allow duplicates in the database
      // so, the old value is overwritten without
      // deleting it before
      // database.delete(null,k);
       dbwriteAccesses++;
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
     dbreadAccesses++;
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
    dbwriteAccesses++;
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

private long readAccesses=0;
private long writeAccesses=0;
private long dbreadAccesses=0;
private long dbwriteAccesses=0;
private int cached=0;


/** The Database environment */
private static Environment environment=null;
private static RemoveEnv rmEnv;
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
   boolean update; // flag specifying whether on cache are newer data 
                   // than in the database
} 

static class RemoveEnv implements Runnable {

    RemoveEnv(Environment env){
      this.env = env;
      databases = new Vector();
    }

    public void addDatabase(Database db){
        if(!databases.contains(db))
           databases.add(db);
    }
    
    public void run() {
       // first, close all databases
       try{
        for(int i=0;i<databases.size();i++)
           ((Database)databases.get(i)).close();
       } catch(Exception e){
         e.printStackTrace();
       }


       try{
           List DBNs = env.getDatabaseNames();
           for(int i=0;i<DBNs.size();i++){
              env.removeDatabase(null,(String)DBNs.get(i)); 
            }
       }catch(Exception e){
         e.printStackTrace();
       }
       File file = null;
       try{
          file = env.getHome();
          env.close();
       } catch(Exception e){
          e.printStackTrace();
       }
       if(file!=null){
         if(!deleteFile(file)){
             System.out.println("Cannot delete the temporal directory : "+ file);
          }
       }else{
          System.err.println("Error in removing database environment");
       }
    }

   // deletes F with subdirectories if F is a directory
   public static boolean deleteFile(File F){
       boolean ok;
       if(F.isDirectory()){
          File[] content = F.listFiles();
          for(int i=0;i<content.length;i++)
              deleteFile(content[i]);
       }
       return F.delete();
   }

    private Environment env;
    private Vector databases;
}

}
