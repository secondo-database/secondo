/*
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

1 Header File of Module TupleManager

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Stefan Dieker, 03/05/98

\tableofcontents

1.1 Overview

This file essentially defines two classes: *Tuple* and *TupleElem*. (In the
rest of this documentation we denote tuple elements by the term *attributes*).
They
provide efficient access to persistent data types in a very easy manner.
Classes implementing attribute data types have to be subtypes of class
attribute. Whatever the shape of such derived attribute classes might be,
their instances can be aggregated and made persistent via instances of class
Tuple, while the user is (almost) not aware of the additional management
actions arising from persistency. An example of how to use attributes and
tuples can be found in ../Tests/Tuple.cc. 

A tuple can exist in different states:

\begin{description}
\item[Fresh] A new tuple has been constructed
\item[SolidRead] An already existing tuple has been opened for read-only
access
\item[SolidWrite] An already existing tuple has been opened for 
write access
\end{description}
\begin{figure}[thb]
\centerline{\fbox{\epsfig{file=Figures/tuplestates.eps,width=12cm}}}
\caption{\label{FigStates} States and operations of tuple instances}
\end{figure}
Figure \ref{FigStates} illustrates the relationship of states and operations.

1.2 Defines

*/
#ifndef TUPLEMANAGER_H
#define TUPLEMANAGER_H

#define MAXATTR 128
#define MAXDBARRAYS 64

/*
1.3 Includes

*/

#include "TupleElement.h"
#include "SecondoSMI.h"
#include "Algebra.h"

/*
1.4 Basic Types

1.4.1 Open Mode

Existing tuples can be opened for read-only or write purposes, which must be
indicated by a parameter of type OpenMode.

*/

enum OpenMode {ReadAccess, WriteAccess};

/*
1.4.2 AttrType

When opening a tuple, a description of each attribute type
has to be passed. It is stored in a variable of type *AttrType*.

*/

struct AttributeType {
    int algId;  // Unique algebra id
    int typeId; // Unique type id
    int size;   // Size of attribute instance in bytes
};

/*
1.4.3 TupleType

When opening a tuple, a description of the tuple type has to be passed. It is
stored in a variable of type *TupleType*.

*/

struct TupleAttributes {
    int totalNumber;      // Number of attributes
    AttributeType* type;  // Array of attribute type descriptors
    int totalSize;        // sum of all attrib sizes
    // Constructor. Sets all member variables, including size
    TupleAttributes(int noattrs, AttributeType *attrtypes);
};

/*
1.5 Class TupleElement

Each type constructor which is going to be an attribute data type must be
a derived class of class *Attribute*. Its pure virtual
function *Compare()* as well as the destructor have to be defined.
The virtual function *Print()* should be overloaded.

*/

/*
1.5 Class Tuple

*/

class Tuple {

/*
  1.5.1 Private members:

*/


	friend class FLOB;

private:
	
	/* An AttributeInfo manages one attribute of a tuple,
		specifically its deleting. */
    struct AttributeInfo {
    	/* This pointer refers to the "parent" AttributeInfo of another
			tuple if exists. See AttrPut method of Tuple. */
		AttributeInfo *parent;
		
		/* keine Ahnung. */
		//bool defined;
		
		/* destruct determines wheather the TupleElement* value should 
			deleted if there is no reference at all. */
		bool destruct;
		
		/* changed determines wheather this attribute differ from its 
			persistent delineation. */
        bool changed;
	
		/* refCount determines the number of tuples using this attribute. */
		int refCount;
		
		/* the value of this tuple component. */
		TupleElement *value;

		/* the size of this tuple component. */		
		int size;

		/* simple Constructor. */
		AttributeInfo(){
			destruct = changed = false; 
			//defined = destruct = changed = false;
			parent = 0; value = 0; size = 0;
			refCount = 0;
		}
		
		/* increase RefCount in all AttributeInfos 
			referring to the TupleElement *value */
		void incRefCount() {
			refCount++;
			if (parent != 0) parent->incRefCount();
		}
		
		/* decrease RefCount in all AttributeInfos
			referring to the TupleElement *value */
		void decRefCount() {
			refCount--;
			if (parent != 0) parent->decRefCount();
		}
		
		/* delete value in all AttributeInfos referring 
			to the TupleElement *value. */
		void deleteValue() {
			delete value;
			if (parent != 0) parent->deleteValue2();
		}
		
		/* A help method for above one. */
		void deleteValue2() {
			// The value pointer points to a memory area witch is already deleted.
			value = 0;
			if (parent != 0) parent->deleteValue2();
		}
    };
    
    enum TupleState {Fresh, SolidWrite, SolidRead };

    struct TupleHeader{
	    int size;
	    SmiFileId lobFileId; 
    };

    SmiRecord diskTuple;           	// Membervar which represents an disktuple.
    SmiRecordId diskTupleId; 		// Record that persistently holds the tuple value.
    SmiRecordFile* lobFile;     	// Reference to a File which contains LOBs.
    SmiRecordFile* recFile;			// Reference to a File which contains tuples.  
    bool lobFileOpened;				// Is the lobfile open?
    bool lobFileAllocated;			// Was the lobFile allocated by the Tuple manager?
    int attrNum;                	// Number of attribs. 
    AttributeInfo *attribInfo;  	// Sizes of attrib values. 
    AlgebraManager* algM;       	// Reference to Algebramanagers. 
    TupleState state;    			// State of the tuple (Fresh, SolidWrite, SolidRead)
    int memorySize;					// Size of the amount of core tuple.
    char *memoryTuple;				// Used to load temporarily the disk tuple into memory.
   	int extensionSize;				// Size of the amount of extension tuple.
	char *extensionTuple;			// Used to load temporarily the extension 
									// of a disk tuple into memory.

   	/* initialisation of member variables */
    void Init(const TupleAttributes *attributes);

	/* Determine the size of FLOB data stored in lobFile. */
	int CalcSizeOfFLOBData();

	/* move FLOB data to extension tuple. */
	char *moveFLOBDataToExtensionTuple();

	/* move external attribue values to memory tuple */
	void MoveExternalAttributeToMemoryTuple();

public:
	/* true if a tuple could not read from SmiFile. */
	bool error;

/*
1.5.1 Constructors

There are two constructors. The first one creates a 
new tuple instance based on a tuple type description.

*/
    
    Tuple(const TupleAttributes *type);

/*
  The second constructor creates a tuple from a given ~record~. 
  The type description of the tuple also has to be supplied in ~type~. 
  After this tuple creation, the tuple can be changed or not depending on ~mode~.

*/
	Tuple(SmiRecordFile* recfile, SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode);

 
/*
1.5.2 Destructor

Calling the destructor releases the main memory occupied by the tuple instance.
It does *not* destroy the underlying persistent record, if any such record
exists.

*/
    ~Tuple();

/*
1.5.3 SaveTo

A persistent record is created for the tuple in the file ~tupleFile~. 
If the tuple contains any database arrays to be stored externally,

*/

    bool SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile);

/*
1.5.4 Save

The actual contents of a tuple are made persistent by storing them into the
underlying record.

*/

    bool Save();

/*
1.5.5 Destroy

The persistent representation of a tuple is destroyed.

*/

    bool Destroy();

/*
1.5.5 GetPersistentId

The persistent id of a tuple is returned.

*/

    SmiRecordId GetPersistentId(){ return diskTupleId; }

/*
1.5.6 Put

*Put()* is used to set the ~attrno~-th attribute to the value pointed to by
~value~. It is the caller's responsibility that this value exists until a call
of *Save()* or *SaveTo()* takes place. The caller also has to take care of
the eventual deletion of the respective value.

*/


  	bool Put(int attrno, TupleElement *value);

/*
1.5.7 DelPut

*DelPut()* is similar to *Put()* but performs a ~delete value~ when
appropriate.

*/
  
	bool DelPut(int attrno, TupleElement *value);

/*
1.5.8 AttrPut

Again like *Put()*. The difference is that the assigned value is not existing
on its own, but part of another tuple. A special mechanism guarantees that the
memory of the attribute value is not released as long as the actual tuple
refers to that value.

*/

	bool AttrPut(int attrno_to, Tuple *tup, int attrno_from);

/*
1.5.8 Get

Get a pointer to the attribute value with number ~attrno~.

*/
    
	TupleElement *Get(int attrno);

/*
1.5.9 destruct

Get a boolean value which defines whether the attribute  with number ~attrno~ will be destroyed when the tuple is destroyed

*/

	bool destruct(int attrno);

/*
1.5.9 GetAttrNum

Returns the number of attributes.

*/
 
	int GetAttrNum();


/*
1.5.9 GetSize

Returns the size of the tuple value, i.e. the sum of all attribute's sizes.

*/

	int GetSize();


/*
1.5.9 Print

Print a textual representation of the tuple value to ~os~. Only functional
if each contained attribute class provides its own *Print()* method.

*/

    ostream &Print(ostream& os);

};

	ostream& operator<< (ostream &os, Tuple &tup);

#endif

/*
2 References

[G[ue]t97] R. H. G[ue]ting.\\
Secondo Notes 4: Database Arrays, Tuple Manager\\
and Attribute Data Type Implementation.\\
PI LG IV Fernuniversit[ae]t Hagen, 1997.

*/
