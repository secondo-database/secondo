/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

1 Header file of ~MTree~ datastructure

November 2007 Mirko Dibbert

[TOC]

1.1 Overview

TODO: enter MTree datastructure discription

1.2 Includes and Defines

*/

#ifndef __MTREE_H__
#define __MTREE_H__

#include "RelationAlgebra.h"
#include "MetricAttribute.h"
#include "MetricRegistry.h"
#include "MTreeTools.h"
#include "WinUnix.h"
#include <list>

const size_t PAGESIZE = ( WinUnix::getPageSize() - 100 );
/*


*/

const int MIN_NODE_CAPACITY = 2;
/*
Should be at least 2 and is used, if a data string is stored:
If there does not fit at least MIN NODE CAPACITY entries into every node the
m-tree will use external storage for the strings.

*/

/*
1.8 struct ~MTreeLeafEntry~

*/
struct MTreeLeafEntry : public MTreeEntry
{
  TupleId tupleId; // tuple identifier

  inline MTreeLeafEntry( char* buffer, int& offset, STORAGE_TYPE storageType )
  {
    Read( buffer, offset, storageType );
  }

/*
Read constructor.

*/

  inline MTreeLeafEntry( MTreeData* data, TupleId tid, double dist )
  {
    this->tupleId = tid;
    this->data = data;
    this->dist = dist;
  }
/*
Standard constructor.

*/

  static int StaticSize()
  {
    return sizeof(TupleId) +  // tupleId
           sizeof(double);    // dist
  }
/*
Returns the size of the entry in disk (without data member)

*/

  void Read( char* buffer, int& offset, STORAGE_TYPE storageType );
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char* buffer, int& offset );
/*
Writes an entry from the buffer. Offset is increased.

*/

};

/*
1 struct ~MTreeRoutingEntry~

*/
struct MTreeRoutingEntry : public MTreeEntry
{
  SmiRecordId *chield; // pointer to covering tree
  double r;            // covering radius

  inline MTreeRoutingEntry( char* buffer, int& offset,
                            STORAGE_TYPE storageType )
  {
    Read( buffer, offset, storageType  );
  }

/*
Read constructor.

*/

  inline MTreeRoutingEntry(
        MTreeEntry* e, 
        SmiRecordId *chield, 
        double dist, 
        double r )
  {
    this->data = e->data;
    this->chield = chield;
    this->dist = dist;
    this->r = r;
  }
/*
Constructor.

*/

  inline MTreeRoutingEntry(
        MTreeData* data, 
        SmiRecordId *chield, 
        double dist, 
        double r )
  {
    this->data = data;
    this->chield = chield;
    this->dist = dist;
    this->r = r;
  }
/*


*/

  inline ~MTreeRoutingEntry()
  {
    cout << "MTreeRoutingEntry destructor called\n";
    delete data;
  }

  static int StaticSize()
  {
    return sizeof(SmiRecordId) + // chield
           sizeof(double) +      // dist
           sizeof(double);       // r
  }
/*
Returns the size of the entry in disk (without data member)

*/

  void Read( char *buffer, int &offset, STORAGE_TYPE storageType );
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int &offset );
/*
Writes an entry from the buffer. Offset is increased.

*/
};
/*
1 Class ~MTreeNode~

*/
class MTreeNode
{
 public:
  MTreeNode( bool leaf, size_t maxEntries, STORAGE_TYPE storageType );
  ~MTreeNode();

  static inline int SizeOfEmptyNode()
  {
    return sizeof(size_t); // count of entries
  }

  void Read( SmiRecordFile &file, const SmiRecordId page );
  void Read( SmiRecord &record );

  void Write( SmiRecordFile &file, const SmiRecordId page );
  void Write( SmiRecord &record );

  void InsertEntry( MTreeEntry* entry );

  list<MTreeEntry*>::iterator GetEntryIterator()
  {
     return entries.begin();
  }

  inline bool IsLeaf() { return isLeaf; }
  inline bool IsFull() { return !(entries.size() < maxEntries); }
  inline int GetEntryIter() { return entries.size(); }
  inline int GetEntryCount() { return entries.size(); }

 private:
  list<MTreeEntry*> entries; // list containing the entries
  bool isLeaf;     // true, if the node is a leaf
  bool modified;   // true, if the node has been modified
  size_t maxEntries;
  STORAGE_TYPE storageType;
};

/*
1 Struct ~MTreeHeader~

*/
class MTreeHeader
{
 public:
  MTreeHeader() :
    headerInitialized ( false ),
    root ( 0 ),
    promFun ( STD_PROMFUN ),
    partFun ( STD_PARTFUN ),
    storage ( REFERENCE ),
    datafileId ( 0 ),
    dataLength ( 0 ),
    maxLeafEntries ( 0 ),
    maxRoutingEntries ( 0 ),
    attrIndex ( 0 ),
    algebraId ( 0 ),
    typeId ( 0 ),
    height ( 0 ),
    nodeCount ( 0 ),
    entryCount ( 0 )
  {}

  inline void Write( SmiRecord &record )
  {
    assert( record.Write( this, sizeof( MTreeHeader ), 0 )
            == sizeof(MTreeHeader) );
  }

  inline void Read( SmiRecord &record )
  {
    assert( record.Read( this, sizeof( MTreeHeader ), 0 )
    == sizeof(MTreeHeader) );
  }

  void Print( void )
  {
    cout << "\n"
         << "Header Statistics:\n"
         << "==================================================\n"
         << "      header size = " << sizeof( MTreeHeader ) << "\n"
         << "headerInitialized = " << headerInitialized << "\n"
         << "             root = " << root << "\n"
         << "          promFun = " << promFun << "\n"
         << "          partfun = " << partFun << "\n"
         << "          storage = " << storage << "\n"
         << "       datafileId = " << datafileId << "\n"
         << "   maxLeafEntries = " << maxLeafEntries << "\n"
         << "maxRoutingEntries = " << maxRoutingEntries << "\n"
         << "        attrIndex = " << attrIndex << "\n"
         << "        algebraId = " << algebraId << "\n"
         << "           typeId = " << typeId << "\n"
         << "           height = " << height << "\n"
         << "        nodeCount = " << nodeCount << "\n"
         << "       entryCount = " << entryCount << "\n"
         << "==================================================\n\n";
  }

  bool headerInitialized;
  SmiRecordId root;          // page of the root node
  PROMFUN promFun;             // index of used promotion function
  PARTFUN partFun;             // index of used partition function
  STORAGE_TYPE storage;
  SmiFileId datafileId;
  size_t dataLength;            // length of the data string
  int maxLeafEntries;    // maximum number of entries in a node
  int maxRoutingEntries; // maximum number of entries in a node
  int attrIndex;             // used attribute index
  int algebraId;
  int typeId;
  int height;                // height of the tree
  int nodeCount;
  int entryCount;
};

/*
1 Class ~MTree~

*/
class MTree
{
 public:
  MTree();
/*
Constructor (creates a new tree)

*/

  MTree( const SmiFileId fileid );
/*
Constructor (opens an existing tree)

*/

  ~MTree();
/*
Destructor

*/

  void Initialize( Relation *rel, Tuple *tuple, int attrIndex,
                  PROMFUN promFun = STD_PROMFUN,
                  PARTFUN partFun = STD_PARTFUN);
/*
This method is needed to set a pointer to the relation and to store the
attribute index. This will be needed when the objects are only stored as
reference (tupleId) to get the objects for distance computations.

The tuple is needed to get one reference object which is needed to get the
datastring length and with it the storage method: If the GetMDatalength
returns 0, the object will be stored as reference. Otherwise the objects
string representation will be stored direktly in the nodes, if the data string
length is small enough to store at least two entries per node, or in a seperate
SmiRecordFile (Member ~datafile~) otherwhise. 

For every type of storage (REFERENCE, INTNERNAL, EXTERNAL) there exist a
correspondent class for the data member in ~MTreeEntry~ and a respective
wrapper class for the distance function.

*/

  inline void Initialize( Relation *rel )
  {
    InitializeFromHeader( rel );
  }
/*
This method is used to initialize the MTree as above, but the required values
are obtained from the previously stored header.

*/

  void DeleteFile();
/*
This Method deletes the file, used in ~DeleteMTree~ function (MTreeAlgebra.cpp)

*/

  inline bool IsInitialized() { return initialized; }
/*
Returns true if the Initialize function has been called previously.

*/

  void Insert( TupleId tupleId );
/*
Inserts a new entry into the tree.

*/

  inline SmiFileId GetFileId() { return file.GetFileId(); }
/*
This method returns the FileId of the SmiFile containing the m-tree

*/

 private:

  void InitializeFromHeader(Relation *rel);

  void ReadHeader();
  void WriteHeader();

  void Split();

  SmiRecordFile file; // the file, which is containing the tree
  SmiRecordFile* datafile; // the file, which is containing the tree
  MTreeHeader header; // contains some informations about the tree
  bool initialized;   // true, if initialize function has been called
  MetricWrapper* metric;
  SplitPolicy *splitpol;
  Relation* rel;

//    inline bool IsLeaf( int level ) const
//    {
//      return ( level == header.height );
//    }
/*
Returns the maximum number of entries per node.

*/

};

#endif
