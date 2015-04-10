/*


1 Class FlobManager

The only class using the ~FlobManager~ class is ~Flob~. For this reason,
all functions are declared as private and Flob is declared to be a friend
of this class.

*/
#ifndef SEC_FLOBMGR_H
#define SEC_FLOBMGR_H


#include "SecondoSMI.h"

#ifdef THREAD_SAVE
#include <boost/thread.hpp>
#endif


class Flob;


class FlobManager{
   friend class Flob;
   friend class FlobCache;
   friend class NativeFlobCache;
   friend class PersistentFlobCache;
   private:
/*

The FlobManager is realized to be a singleton. So, not the constructor
but this ~getInstance~ function is to call to get the only one
FlobManager instance.

*/

      static FlobManager& getInstance() ;

/*
~destroyInstance~

Destroys the singleton ~FlobManager~ instance. This function must be called
before Secondo exits to make sure that the native Flob file is closed correctly.
After calling the function all native Flobs (Flobs not explicitely stored into
a given file) are not longer available.

*/
      static bool destroyInstance();

/*
~getData~

The getData function retrieves data from a Flob.
The __dest__ buffer must be provided by the caller. The requested content is copied
to that buffer.

*/

      bool getData(const Flob& flob,            // Flob containing the data
                   char* dest,                  // destination buffer
                   const SmiSize&  offset,     // offset within the Flob
                   const SmiSize&  size,
                   const bool ignoreCache = false) ;  // requested data size


/*
~destroy~

Frees all resources occupied by a Flob. This deletes the data persistently.
So, after destroying the Flob, its data cannot be accessed any longer.

*/
      bool destroy(Flob& victim);


/*
~destroyIfNonPersistent~

Detroys the content of the Flob, but only if it is a native Flob.
Usually used to free disk and chache space from a temporary stored Flob.

*/
      bool destroyIfNonPersistent(Flob& victim);



/*
~saveTo~

Copy the content referenced by a Flob into a file using a specific position
(given as record and offset). The Flob's content is copied to the file and the
return value is a new Flob which is a valid reference to the copy of the Flob-
argument's content stored within the given file.

*/
      bool saveTo(const Flob& src,   // Flob tp save
                  const SmiFileId fileId,  // target file id
                  const SmiRecordId recordId, // target record id
                  const SmiSize offset,       // offset within the record
                  const char mode,          // environment
                  Flob& result);    // offset within the record


      bool saveTo(const Flob& src,   // Flob tp save
                  SmiRecordFile* file,  // target file id
                  const SmiRecordId& recordId, // target record id
                  const SmiSize& offset,
                  Flob& result);    // offset within the record

/*
~saveTo~

Saves a Flob into a specific file. The Flob is saved into a newly created record
in the file at offset 0. By saving the old Flob, a new Flob referencing the stored
data is created which is the result of this function.

*/

      bool saveTo(const Flob& src,             // flob to save
                  const SmiFileId fileId,      // target file id
                  const char mode,           // target environment
                  Flob& result);  // target file id


     bool saveTo(const Flob& src,             // flob to save
                 SmiRecordFile* file,  // target file
                 Flob& result);  // the new flob position

/*
~putData~

Copies data from a given buffer into the Flob's storage. The Flob's size is
adjusted to the size of the copied data.

*/

      bool putData(Flob& dest,         // destination flob
                         const char* buffer, // source buffer
                         const SmiSize& targetoffset,  // offset within the Flob
                         const SmiSize& length,
                         const bool ignoreCache = false);  // data size


/*
~putData~

Puts data into a Flob without checking or changing sizes.


*/
      bool putData(const FlobId& id,         // destination flob
                         const char* buffer, // source buffer
                         const SmiSize& targetoffset,  // offset within the Flob
                         const SmiSize& length
                         );

/*
~setExFile~

Link Flob to an external file.

*/
      bool setExFile(Flob& flob, const string& flobFile,
          const SmiSize length, const SmiSize flobOffset);

/*
~SwitchToMode1~

Read the data to Flob with mode 1

*/

      bool SwitchToMode1(Flob& flob, const string& flobFile,
          const SmiSize length, const SmiSize flobOffset);

/*
~changeMode~

Reset the Flob mode.

*/
      void changeMode(Flob* flob, const char mode);

/*
~create~

Creates a new, empty Flob with a given size within a temporal file
(native Flob file).

*/

      bool create(const SmiSize size, Flob& result);  // result flob

/*
~create~

Creates a new, empty Flob with given file and position.

*/

      bool create(const SmiFileId& fileId,        // target file
                  const SmiRecordId& recordId,    // target record id
                  const SmiSize& offset,      // offset within the record
                  const char mode,              // environment
                  const SmiSize& size,
                  Flob& result);       // initial size of the Flob

/*
~createFrom~

Returns a Flob with persistent storage allocated and defined elsewhere.

*/

      static Flob createFrom( const SmiFileId& fid,
                              const SmiRecordId& rid,
                              const SmiSize& offset,
                              const char mode,
                              const SmiSize& size);


/*
~copyData~

Copies the referenced data (content) from one Flob to another.

*/
      bool copyData(const Flob& src, Flob& dest);


/*
~resize~

Changes the size of a given Flob to the specified size.

*/
     bool resize(Flob& flob, const SmiSize& newSize,
                 const bool ignoreCache=false);



/*
~constructor~

Because FlobManager is a singleton class, the constructor should only be used
by the FlobManager class itself.

*/

      FlobManager();

/*
~dropfile~

If Flob data is stored into a file, the FlobManager will
create a new File from the file id and keep the open file.
If the file should be deleted or manipulated by other code,
the FlobManger has to give up the control over that file.
This can be realized by calling the ~dropFile~ function.

*/
     bool dropFile(const SmiFileId& id, const char mode);

/*
~dropFiles~

Will drop all files except the native flob file.

*/
     bool dropFiles();


/*
~killNativeFlobs~

destroys all native Flobs

Can be called after executing a command to drop temporary data.

*/
    void killNativeFlobs();


/*
~killCashes~

clears all caches.

Can be called when opeing and closing a database.

*/
   void clearCaches();


/*
SetNativeCache, SetPersistentCache

Sets sizes for the native flob cache/ persistent Flob cache.
The slotsize must be smaller than the maxSize.
The avgSize must be smaller or equal to the slotSize.
Can only be called before any other flob operation,
i.e. when ~instance~ does not yet exist.

*/
   static void SetNativeCache(const size_t maxSize,
                              const size_t slotSize,
                              const size_t avgSize);


   static void SetPersistentCache(const size_t maxSize,
                              const size_t slotSize,
                              const size_t avgSize);


/*
~Destructor~

The destructor should never be called from outside. Instead, the destroyInstance
function should be called. By calling the destructor, all open files are closed
and the native Flob file ~nativeFlobFile~ is deleted.


*/
     ~FlobManager();


/*
~instance~

The only instance of the FlobManager, never use this member directly!

*/
      static FlobManager* instance;

/*
~nativeFlobs~

File id for freshly created Flobs. Only to be used by the Flobmanager class itself.

*/
     SmiFileId nativeFlobs; // for in memory Flobs
     SmiRecordFile* nativeFlobFile;

/*
~open Files~

This map is used to manage all file ids and files opened by the FlobManager.

*/
    map< pair<SmiFileId, bool>, SmiRecordFile*> openFiles;


/*
~getFile~

Return the file belonging to a given fileid;

*/
   SmiRecordFile* getFile(const SmiFileId& fileId, const char mode);

/*
~createFromBlock~

Does the evil thing. Loads a Flob's content from a file into a fixed main
memory buffer.
This is used to speed-up processing of small Flobs, and is only applied to
non-native Flobs.

*/
 void createFromBlock(Flob& result,const char* buffer, const SmiSize& size,
                      const bool autodestroy);


/*
~makeControllable~

This converts an ~uncontrollable Flob~ (see ~createFromBlock~) into a regular
Flob, that does not use fixed main memory buffer.

*/
 bool makeControllable(Flob& flob);


/*
~mutex member~

If the implementation should be thread save, all accesses to 
global instances (flob cashes) must be synchronized.

*/
#ifdef THREAD_SAVE
  boost::mutex ncmtx;
  boost::mutex pcmtx;
  boost::mutex omtx;  // access to other variables
#endif






};




#endif
