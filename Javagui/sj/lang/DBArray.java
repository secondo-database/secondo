package sj.lang;

import java.io.File;
import java.io.RandomAccessFile;
import java.util.List;
import java.util.Vector;


/**
This Class provides an array of inifinity size

*/

public class DBArray{


/** Creates a new Array with the given properties **/
public DBArray(int cacheSize, byte maxDataSize){
   this(cacheSize,maxDataSize,maxDataSize*4);
}


/**
Create a new DBArray with given cachesize. The cachsize determines 
not the size of the cahc in bytes bit in entries. Data greater than 
maxDataSize are not cached.

*/
public DBArray(int cacheSize, byte maxDataSize, int dataSizeSecondStage){
    if(cacheSize<0)  cacheSize=1;
    // if this is the first dbarray => initialize a environment
	if(tempDir==null){
        initEnvironment();
    }
    initDatabase();
    this.cacheSize = cacheSize;
    this.maxDataSize = Math.max(maxDataSize+1,9);
    this.secondDataSize = Math.max(2*this.maxDataSize,dataSizeSecondStage);
    this.cache = new Entry[cacheSize];
    // init the cache
    for(int i=0;i<cacheSize;i++)
      cache[i]=null;

    fileEntries1 = (int) (FILESIZE/(maxDataSize+4));
    fileEntries2 = (int) (FILESIZE/secondDataSize);
    connector1 = new byte[maxDataSize+4];
    connector2 = new byte[secondDataSize];
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
     tempDir = DBDir;
     System.out.println("Creating new Directory "+tempDir.getName()+" for DBArrays");
     tempDir.mkdir();  // create a directory if not exists
     rmEnv = new RemoveEnv(tempDir);
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
   DBName = "DB_Array_"+DBNumber;
   DBNumber++; 
   return true; 
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
     long FileNumber = key / fileEntries1;
     String FileName = tempDir.getAbsolutePath()+File.separator+DBName+"_1_"+FileNumber;
     if(lastUsedFile1==null || FileNumber!=lastUsedFileNumber1){
        if(lastUsedFile1!=null)
           lastUsedFile1.close();
        lastUsedFile1 = new RandomAccessFile(FileName,"rw");
        lastUsedFileNumber1 = FileNumber;
        if(lastUsedFile1.length()!=FILESIZE)
           lastUsedFile1.setLength(FILESIZE);
     }
     int length=data.length; 
     if(length<maxDataSize){
        // level 1 store the data in the first file
        // put the length into the array
        connector1[3]= (byte) (length&255);
        length = length >> 8;
        connector1[2] = (byte) (length&255);
        length = length >> 8;
        connector1[1] = (byte) (length&255);
        length = length >> 8;
        connector1[0] = (byte) (length&255);
        // put the data in the array
        System.arraycopy(data,0,connector1,4,data.length);
        // write to file
        long fileoffset = (key%fileEntries1)*connector1.length;
        lastUsedFile1.seek(fileoffset);
        lastUsedFile1.write(connector1);
     }
     else if(length<secondDataSize){
        // first, write the entry into the first stage
        // we use a negative value for indicating the 
        // second stage
        length = -length;
        connector1[3]= (byte) (length&255);
        length = length >> 8;
        connector1[2] = (byte) (length&255);
        length = length >> 8;
        connector1[1] = (byte) (length&255);
        length = length >> 8;
        connector1[0] = (byte) (length&255);
        // write to file
        long fileoffset = (key%fileEntries1)*connector1.length;
        lastUsedFile1.seek(fileoffset);
        lastUsedFile1.write(connector1);
        long FileNumber2 = key / fileEntries2;
        String FileName2 = tempDir.getAbsolutePath()+File.separator+DBName+"_2_"+FileNumber2;
        if(lastUsedFile2==null || FileNumber2!=lastUsedFileNumber2){
           if(lastUsedFile2!=null)
              lastUsedFile2.close();
           lastUsedFile2 = new RandomAccessFile(FileName2,"rw");
           lastUsedFileNumber2 = FileNumber2;
        }
        if(lastUsedFile2.length()!=FILESIZE)
           lastUsedFile2.setLength(FILESIZE);
        long fileoffset2 = (key%fileEntries2)*connector2.length;
        lastUsedFile2.seek(fileoffset2);
        lastUsedFile2.write(data);
     }
     else {
       // we use -1 for indicating the third stage
       // this is not conflicting with stage 2 because the
       // length in the second stage must be greater than 1
        length = -1;
        connector1[3]= (byte) (length&255);
        length = length >> 8;
        connector1[2] = (byte) (length&255);
        length = length >> 8;
        connector1[1] = (byte) (length&255);
        length = length >> 8;
        connector1[0] = (byte) (length&255);
        // write to file
        long fileoffset = (key%fileEntries1)*connector1.length;
        lastUsedFile1.seek(fileoffset);
        lastUsedFile1.write(connector1);
        // write the data into a single file
        RandomAccessFile file3 = new RandomAccessFile(tempDir.getAbsolutePath()+File.separator+DBName+"_3_"+key,"rw");
        file3.seek(0);
        file3.write(data);
        file3.close();
     }
     return true;     
   }catch(Exception e){
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
    // first read the entry in the first stage
    long FileNumber = key / fileEntries1;
    String FileName = tempDir.getAbsolutePath()+File.separator+DBName+"_1_"+FileNumber;
    if(lastUsedFile1==null || FileNumber!=lastUsedFileNumber1){
       if(lastUsedFile1!=null)
          lastUsedFile1.close();
       lastUsedFile1 = new RandomAccessFile(FileName,"rw");
       lastUsedFileNumber1=FileNumber;
    }
    if(lastUsedFile1.length()!=FILESIZE){
        return null;
    }
    long fileoffset1 =  (key%fileEntries1)*connector1.length;
    lastUsedFile1.seek(fileoffset1);
    lastUsedFile1.readFully(connector1);
    // extract the length information from the byteblock;
    int length = getPositiveInt(connector1[0]);
    length = length << 8;
    length = length | getPositiveInt(connector1[1]);
    length = length << 8;
    length = length | getPositiveInt(connector1[2]);
    length = length << 8;
    length = length | getPositiveInt(connector1[3]);
    if(length == -1){ // third level
        RandomAccessFile F = new RandomAccessFile(tempDir.getAbsolutePath()+File.separator+DBName+"_3_"+key,"r");
        byte[] data = new byte[(int)F.length()];
        F.readFully(data);
        F.close();
        return new Entry(key,data);
    }
    if(length >=0){ // first level
       byte[] data = new byte[length];
       System.arraycopy(connector1,4,data,0,length);
       return new Entry(key,data);
    }
    // second level
    long FileNumber2 = key/fileEntries2;
    String FileName2 = tempDir.getAbsolutePath()+File.separator+DBName+"_2_"+FileNumber2;
    if(lastUsedFile2==null || FileNumber2!=lastUsedFileNumber2){
        if(lastUsedFile2!=null)
            lastUsedFile2.close();
        lastUsedFile2 = new RandomAccessFile(FileName2,"rw");
        lastUsedFileNumber2 = FileNumber2;
    }
    long fileoffset2 = (key%fileEntries2)*connector2.length;
    lastUsedFile2.seek(fileoffset2);
    byte[] data = new byte[-length];
    lastUsedFile2.readFully(data);
    return new Entry(key,data);
   }catch(Exception e){
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
    long FileNumber = key / fileEntries1;
    String FileName = tempDir.getAbsolutePath()+File.separator+DBName+"_1_"+FileNumber;
    if(lastUsedFile1==null || FileNumber!=lastUsedFileNumber1){
       if(lastUsedFile1!=null)
          lastUsedFile1.close();
       lastUsedFile1 = new RandomAccessFile(FileName,"rw");
       lastUsedFileNumber1=FileNumber;
    }
    if(lastUsedFile1.length()!=FILESIZE){
        return false;
    }
    long fileoffset1 =  (key%fileEntries1)*connector1.length;
    lastUsedFile1.seek(fileoffset1);
    lastUsedFile1.readFully(connector1);
    // extract the length information from the byteblock;
    int length = getPositiveInt(connector1[0]);
    length = length << 8;
    length = length | getPositiveInt(connector1[1]);
    length = length << 8;
    length = length | getPositiveInt(connector1[2]);
    length = length << 8;
    length = length | getPositiveInt(connector1[3]);
    connector1[0]=0;
    connector1[1]=0;
    connector1[2]=0;
    connector1[3]=0;
    if(length == -1){ // third level
        File F = new File(tempDir.getAbsolutePath()+File.separator+DBName+"_3_"+key);
        if(!F.exists())
            return false;
        F.delete();
    }
    lastUsedFile1.seek(fileoffset1);
    lastUsedFile1.write(connector1); 
    return true;
   }catch(Exception e){
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
private int secondDataSize;
private String DBName;

private boolean DEBUG_MODE=true;
private Entry[] cache; 

private long readAccesses=0;
private long writeAccesses=0;
private long dbreadAccesses=0;
private long dbwriteAccesses=0;
private int cached=0;


private static final long FILESIZE = 4195304;
private int fileEntries1;
private int fileEntries2;
private RandomAccessFile lastUsedFile1;
private long lastUsedFileNumber1;
private RandomAccessFile lastUsedFile2;
private long lastUsedFileNumber2;
/** Byte array used for exchange data between file and memory 
  * in the first stage
  **/
private byte[] connector1;
/** Bytearray for data exchange between files and memory in the
  * second stage
  **/
private byte[] connector2;

private static File tempDir = null;

private static int getPositiveInt(byte b){
  if(b<0)
     return 256+b;
  else
     return b;
 }
 

/** The Database environment */
private static RemoveEnv rmEnv;

private static int DBNumber = 0;

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

    RemoveEnv(File dir){
      this.directory = dir;
    }

    public void run() {
       deleteFile(directory);
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

    private File directory;
}

}
