/*


1 Class FlobManager

The only class using the FlobManager class is Flob. For this reason,
all functions are declared as private and Flob is declared to be a friend
of that class.

*/
#ifndef SEC_FLOBMGR_H
#define SEC_FLOBMGR_H


#include <map>
#include "SecondoSMI.h"


// forward declaration of the FLOB class
class Flob;

class FlobManager{
   friend class Flob;
   private:
/*
 
The FlobManager is realized to be a singleton. So, not the contructor
but this getInstance function is to call to get the only one
FlobManager instance.

*/    

      static FlobManager& getInstance() ;

/*
~destroyInstance~

Destroys the only instance of the FlobManager. This function must be called
before Secondo exits to be sure that the nativ Flob file is closed correctly.
After calling that function all nativ flobs (flobs not explicitely stored into
file) are not longer available.

*/
      static bool destroyInstance();

/*
~getData~

The getData function retrieves Data from a Flob. 
The __dest__ buffer must be provided by the caller. The requested content is copyied
to that buffer. 

*/

      bool getData(const Flob& flob,            // Flob containing the data
                   char* dest,                  // destination buffer
                   const SmiSize&  offset,     // offset within the Flob 
                   const SmiSize&  size) ;  // requested data size


/*
~destroy~

Frees all resources occupied by the Flob. After destroying a flob. No data can be 
accessed.

*/
      void destroy(Flob& victim);


/*
~saveTo~

Save the content of a flob into a file at a specific position (given as record and 
offset). This will result in another Flob which is returned as the result of this 
function.


*/
      bool saveTo(const Flob& src,   // Flob tp save
                  const SmiFileId fileId,  // target file id
                  const SmiRecordId recordId, // target record id
                  const SmiSize offset,
                  Flob& result);    // offset within the record  


/*
~saveTo~

Saves a Flob into a specific file. The flob is saved into a new created record
in the file at offset 0. By saving the old Flob, a new Flob is created which
is the result of this function.

*/

      bool saveTo(const Flob& src,             // flob to save
                  const SmiFileId fileId,
                  Flob& result);  // target file id


/*
~putData~

Puts data into a flob. 

*/

      bool putData(const Flob& dest,         // destination flob
                         const char* buffer, // source buffer
                         const SmiSize& targetoffset,  // offset within the Flob
                         const SmiSize& length);  // data size

/*
~create~

Creates a new Flob with a given size which is assigned to a temporal file.


*/

      bool create(const SmiSize& size, Flob& result);  // result flob

/*
~create~

Creates a new flob with given file and position.

*/

      bool create(const SmiFileId& fileId,        // target file
                  const SmiRecordId& recordId,    // target record id
                  const SmiSize& offset,      // offset within the record
                  const SmiSize& size,
                  Flob& result);       // initial size of the Flob

/*
~createFrom~

return a Flob with persistent storage allocated and defined elsewhere

*/      

      static Flob createFrom( const SmiFileId& fid,
                              const SmiRecordId& rid,
                              const SmiSize& offset, 
                              const SmiSize& size); 


      bool copyData(const Flob& src, Flob& dest);

/*
~constructor~

Because Flobmanager is a singleton, the constructor should only be used
by the FlobManager class itself.

*/

      FlobManager();

/*
~Destructor~

The destructor should never be called from outside. Instead the destroyInstance
function should be called. By calling the destructor, all open files are closed
and the nativFlobFile is deleted.


*/
     ~FlobManager();


/*
~instance~

The only instance of the Flobmanager, never use this member directly!

*/
      static FlobManager* instance;

/*
~nativeFlobs~

File id for freshly created Flobs. Only to use by the Flobmanager class itself.

*/
     SmiFileId nativeFlobs; // for in memory Flobs
     SmiRecordFile* nativeFlobFile;

/*
~open Files~

*/
    map<SmiFileId, SmiRecordFile*> openFiles;

/*
~getFile~

Return the file to a fileid;

*/
   SmiRecordFile* getFile(const SmiFileId& fileId);



};



#endif
