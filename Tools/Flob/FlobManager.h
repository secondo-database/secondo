/*


1 Class FlobManager

The only class using the FlobManager class is Flob. For this reason,
all functions are declared as private and Flob is declared to be a friend
of that class.

*/

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

      static const FlobManager& getInstance();	


/*
~getData~

The getData function retrieves Data from a Flob. 
The __dest__ buffer must be provided by the caller. The requested content is copyied
to that buffer. 

*/
	
      void getData(const Flob& flob,            // Flob containing the data
                   char* dest,                  // destination buffer
                   const SmiSize  offset,     // offset within the Flob 
                   const SmiSize  size) const;  // requested data size


/*
~destroy~

Frees all resources occupied by the Flob. After destroying a flob. No data can be 
accessed.

*/
      void destroy(Flob& victim) const;


/*
~saveTo~

Save the content of a flob into a file at a specific position (given as record and 
offset). This will result in another Flob which is returned as the result of this 
function.


*/
      Flob saveTo(const Flob& src,   // Flob tp save
                  const SmiFileId fileId,  // target file id
                  const SmiRecordId recordId, // target record id
                  const SmiSize offset) const;    // offset within the record  


/*
~saveTo~

Saves a Flob into a specific file. The flob is saved into a new created record
in the file at offset 0. By saving the old Flob, a new Flob is created which
is the result of this function.

*/

      Flob saveTo(const Flob& src,             // flob to save
                  const SmiFileId fileId) const;  // target file id


/*
~putData~

Puts data into a flob. 

*/

      void putData(const Flob& dest,         // destination flob
                         const char* buffer, // source buffer
                         const SmiSize targetoffset,  // offset within the Flob
                         const SmiSize length) const;  // data size

/*
~create~

Creates a new Flob with a given size which is assigned to a temporal file.


*/

      Flob create(const SmiSize size) const;  // result flob

/*
~create~

Creates a new flob with given file and position.

*/

      Flob create(const SmiFileId fileId,        // target file
                  const SmiRecordId recordId,    // target record id
                  const SmiSize offset,      // offset within the record
                  const SmiSize size);       // initial size of the Flob

/*
~constructor~

Because Flobmanager is a singleton, the constructor should only be used
by the FlobManager class itself.

*/

      FlobManager();

/*
~instance~

The only instance of the Flobmanager, never use this member directly!

*/
      static FlobManager* instance;

/*
~nativeFlobs~

File id for freshly created Flobs. Only to use by the Flobmanager class itself.

*/
      SmiFileId nativeFLOBS; // for in memory Flobs

};


