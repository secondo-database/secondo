/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Header File of Module TupleManager

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Stefan Dieker, 03/05/98

\tableofcontents

1 Overview

This file essentially defines two classes: *Tuple* and *TupleElem*. (In the
rest of this documentation we denote tuple elements by the term *attributes*).
They provide efficient access to persistent data types in a very easy manner.
Classes implementing attribute data types have to be subtypes of class
attribute. Whatever the shape of such derived attribute classes might be,
their instances can be aggregated and made persistent via instances of class
TMTuple, while the user is (almost) not aware of the additional management
actions arising from persistency. An example of how to use attributes and
tuples can be found in ../UserInterfaces/SecondoTestFrame.cpp.

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

2 Defines

*/
#ifndef TUPLEMANAGER_H
#define TUPLEMANAGER_H


/*
3 Includes

*/

#include "RelationAlgebra.h"
#include "TupleElement.h"
#include "SecondoSMI.h"
#include "Algebra.h"

/*
4 Basic Types

4.1 Open Mode

Existing tuples can be opened for read-only or write purposes, which must be
indicated by a parameter of type OpenMode.

*/

enum OpenMode {ReadAccess, WriteAccess};

/*
5 Class TMTuple

*/

class TMTuple {

/*
  5.1 Private members:

*/
  

  friend class FLOB;

private:
	
  /* An AttributeInfo manages one attribute of a tuple,
     specifically its deleting. */
  struct AttributeInfo {
    /* This pointers refer to the AttributeInfo's of other
       tuples if exist. See AttrPut method of TMTuple. */
    AttributeInfo *prev;
    AttributeInfo *next;
    
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
    AttributeInfo();
    
    /* increase RefCount in all AttributeInfos 
       referring to the TupleElement *value */
    void incRefCount();
    
    /* decrease RefCount in all AttributeInfos
       referring to the TupleElement *value */
    void decRefCount();
    
    /* delete value in all AttributeInfos referring 
       to the TupleElement *value. */
    void deleteValue(); 
  };
  
  enum TupleState {Fresh, SolidWrite, SolidRead };
  
  struct TupleHeader{
    int size;
  };
  
  // Record that persistently holds the tuple value.
  SmiRecord diskTuple;
  // The ID of above record according to the underlying DB.
  SmiRecordId diskTupleId;
  // Reference to an SMIRecordFile which contains LOBs. 
  SmiRecordFile* lobFile;
  // Reference to an SMIRecordFile which contains the diskTuple.
  SmiRecordFile* recFile;
  // number of attributes.
  int attrNum;
  // Information about the data stored in the components of a tuple (size..)
  AttributeInfo *attribInfo;
  // Reference to the AlgebraManager.
  AlgebraManager* algM;
  // State of the tuple (Fresh, SolidWrite, SolidRead)
  TupleState state;
  // Size of the amount of core tuple.
  int memorySize;
  // Used to load temporarily the disk tuple into memory. The amount of used
  // bytes is stored in memorySize.
  char *memoryTuple;
  // Size of the amount of extension tuple.
  int extensionSize;
  // Used to load temporarily the extension tuple of a disk tuple into memory.
  // The amount of used bytes is stored in extension size.
  char *extensionTuple;
  
  /* initialisation of member variables */
  void Init(const TupleType& attributes);
  
  /* Determine the size of FLOB data stored in lobFile. */
  int CalcSizeOfFLOBData();
  
  /* move FLOB data to extension tuple. */
  char *MoveFLOBDataToExtensionTuple();
  
  /* move external attribue values to memory tuple */
  void MoveExternalAttributeToMemoryTuple();
  
public:
  /* true if a tuple could not read from SmiFile. */
  bool error;
  
/*
5.1 Constructors

There are two constructors. The first one creates a 
new tuple instance based on a tuple type description.
The result of this constructor is a fresh tuple.

*/
    
    TMTuple(const TupleType& type);

/*
  The second constructor creates a tuple from a given ~record~. 
  The type description of the tuple also has to be supplied in ~type~. 
  After this tuple creation, the tuple can be changed or not depending on ~mode~.

*/

  TMTuple(SmiRecordFile *recfile, SmiRecordId rid, SmiRecordFile *lobfile, 
	const TupleType& attributes, SmiFile::AccessType mode);
 
  TMTuple(SmiRecordFile *recfile, SmiRecordId rid, SmiRecord& record, 
          SmiRecordFile *lobfile, const TupleType& attributes);

  TMTuple(SmiRecordFile* recfile, PrefetchingIterator* iter, SmiRecordFile *lobfile,
	     const TupleType& attributes);
 
/*
5.2 Destructor

Calling the destructor releases the main memory occupied by the tuple instance.
It does *not* destroy the underlying persistent record, if any such record
exists.

*/
    ~TMTuple();

/*
5.3 SaveTo

A persistent record is created for the tuple in the file ~tupleFile~.
The actual contents of a tuple are made persistent by storing them into
the underlying record.
This method can only used for fresh tuples. Use the save method for 
solid tuples instead.

*/

    bool SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile);

/*
5.4 Save

The actual contents of a tuple are made persistent by storing them into the
underlying record.
This method can only used for solid tuples. Use the saveTo method for
fresh tuples instead.

*/

    bool Save();

/*
5.5 Destroy

The persistent representation of a tuple is destroyed.
This belongs NOT to the actual representation in memory.

*/

    bool Destroy();

/*
5.6 GetPersistentId

The persistent id of a tuple is returned.

*/

    SmiRecordId GetPersistentId(){ return diskTupleId; }

/*
5.7 Put

*Put()* is used to set the ~attrno~-th attribute to the value pointed to by
~value~. It is the caller's responsibility that this value exists until a call
of *Save()* or *SaveTo()* takes place. The caller also has to take care of
the eventual deletion of the respective value.

*/


  bool Put(int attrno, TupleElement *value);

/*
5.8 DelPut

*DelPut()* is similar to *Put()* but performs a ~delete value~ when
appropriate. If a value is set to a tuple by this method the caller 
MUST NOT delete the respective value.

*/
  
  bool DelPut(int attrno, TupleElement *value);

/*
5.9 AttrPut

Again like *Put()*. The difference is that the assigned value is not existing
on its own, but part of another tuple. A special mechanism guarantees that the
memory of the attribute value is not released as long as the actual tuple
refers to that value.

*/

  bool AttrPut(int attrno_to, TMTuple *tup, int attrno_from);

/*
5.10 Get

Get a pointer to the attribute value with number ~attrno~.

*/
    
  TupleElement *Get(int attrno);

/*
5.11 destruct

Get a boolean value which defines whether the attribute  with number ~attrno~ will be destroyed when the tuple is destroyed

*/

  bool destruct(int attrno);

/*
5.12 GetAttrNum

Returns the number of attributes.

*/
 
  int GetAttrNum();
  
  
/*
5.13 GetSize

Returns the size of the tuple value, i.e. the sum of all attribute's sizes.

*/

  int GetSize();


/*
5.14 Print

Print a textual representation of the tuple value to ~os~. Only functional
if each contained attribute class provides its own *Print()* method.

*/

  ostream &Print(ostream& os);

};

ostream& operator<< (ostream &os, TMTuple &tup);

#endif

/*
6 References

[G[ue]t97] R. H. G[ue]ting.\\
Secondo Notes 4: Database Arrays, Tuple Manager\\
and Attribute Data Type Implementation.\\
PI LG IV Fernuniversit[ae]t Hagen, 1997.

*/
