/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Header File of Module FLOB

Victor Almeida, 24/04/03. Adapting the class to accept standalone objects
and objects inside tuples transparently.

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Stefan Dieker, 03/05/98

1 Defines

*/

#ifndef FLOB_H
#define FLOB_H

/*

2 Includes

*/
#include "SecondoSMI.h"

#ifdef PERSISTENT_FLOB
enum FLOB_Type {Destroyed, InMemory, InDiskSmall, InDiskLarge, InDiskMemory};
#else
enum FLOB_Type {Destroyed, InMemory, InDiskSmall, InDiskLarge};
#endif

/*
3 class FLOB

This class implements a FLOB which define a new
large object abstraction that offers most access
methods of the original one.
It defines a threshold size namely SWITCH\_THRESHOLD
as a static attribute. This value defines a limit
between "small" and "large" values referring its
size.
A small FLOB is stored in a main memory structure
whereas a large FLOB is stored into a seperate
SmiRecord in an SmiFile.

*/

class FLOB
{

  public:

    static const int SWITCH_THRESHOLD;
/*
This is the tresholdsize for a FLOB.  Whenever the size of this FLOB exceeds
the thresholdsize the data will stored in a separate file for lobs.

*/

    FLOB() {}
/*
This constructor should not be used.

3.1 Constructor

Create a new FLOB from scratch.

*/
    FLOB( int sz );

/*
3.3 Destructor

Deletes the FLOB instance.

*/
    ~FLOB();


/*
3.4 BringToMemory

Brings a disk lob to memory, i.e., converts a flob in ~InDiskLarge~
state to a ~InMemory~ state.

*/ 
    void BringToMemory();

/*
3.5 Get

Read by copying

*/
    void Get( int offset, int length, void *target );

/*
3.6	Put

Write Flob data into source.

*/
    void Put( int offset, int length, const void *source );

/*
3.7 Size

Returns the size of a FLOB.

*/
    int Size() const;


/*
3.8 Resize

Resizes the FLOB.

*/
    void Resize( int size );

/*
3.8 Clear 

Clears the FLOB.

*/
    void Clear();


/*
3.8 Destroy 

Destroys the physical representation of the FLOB.

*/
    void Destroy();

/*
3.9 Restore

Restore from byte string.

*/
    void Restore( void *address );

/*
3.10 SaveToLob

*/
    void SaveToLob( SmiRecordFile& lobFile, SmiRecordId lobId = 0 );

/*
3.10 SetLobFile

*/
    void SetLobFile( SmiRecordFile* lobFile );

/*
3.11 SaveToTupleRecord

*/
    void SaveToExtensionTuple( void *extensionTuple );

/*
3.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/
    bool IsLob() const;

/*
3.11 GetType

*/
    FLOB_Type GetType() const;


  protected:

    FLOB_Type type;
    int size;

    union FLOB_Descriptor
    {
      struct InMemory
      {
        char *buffer;
        bool freeBuffer;
      } inMemory;
      struct InDiskLarge
      {
    	SmiRecordFile *lobFile;
        SmiRecordId lobId;
      } inDiskLarge;
#ifdef PERSISTENT_FLOB
      struct InDiskMemory
      {
        SmiRecordFile *lobFile;
        SmiRecordId lobId;
        char *pageBuffer;
        int pageId;
      } inDiskMemory;
#endif
    } fd;
};

#endif
