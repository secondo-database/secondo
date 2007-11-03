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

//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

1 Header file of ~MTreeTools~

November 2007 Mirko Dibbert

[TOC]

1.1 Overview

TODO: enter discription

1.2 Includes and Defines

*/

#ifndef __MTREE_TOOLS_H__
#define __MTREE_TOOLS_H__

#include <list>

enum PROMFUN
{
  RANDOM, m_RAD, mM_RAD, M_LB_DIST
};
/*
Constants for the implemented promote functions.

*/

enum PARTFUN
{
  GENERALIZED_HYPERPLANE, BALANCED
};
/*
Constants for the implemented partition functions.

*/

const PROMFUN STD_PROMFUN = RANDOM;
const PARTFUN STD_PARTFUN = GENERALIZED_HYPERPLANE;

enum STORAGE_TYPE
{
  INTERNAL, EXTERNAL, REFERENCE
};
/*
Constants for the type of data which is stored (data string or tuple id).

*/

struct MTreeEntry;
class DistanceFunction;

/*
1.8 Class SplitPolicy

This class contains all avaliable promote and partition functions, which both
together form the split policy. The split policy describes, how the entries in
a node are splittet into two nodes.

*/
class SplitPolicy
{
 public:
  SplitPolicy( DistanceFunction *distFun, 
               PROMFUN promFunId, PARTFUN partFunId );
/*
Constructor.

*/

  inline void Promote(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 )
  {
    promFun(entries, prom1, prom2);
  }

  void Partition(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2 )
  {
    partFun(entries, prom1, prom2, entries1, rad1, entries2, rad2, distFun );
  }

 private:
  void (*promFun)(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 );
/*
Contains the selected promote function.

*/

  void (*partFun)(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2,
        DistanceFunction *distFun );
/*
Contains the selected partition function.

*/

  DistanceFunction *distFun;
/*
Contains the distance function wrapper for the respective metric

*/

/*
1.8.1 Promote functions

These methods promote two objects in the entries list and return them in prom1
and prom2. Furthermore the entries are deletet from entries-list.

*/
  static void RAND_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 );
/*
This method promtes two randomly selected elements.

*/

  static void MRAD_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 );
/*
TODO enter method description

*/


  static void MMRAD_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 );
/*
TODO enter method description

*/

  static void MLB_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 );
/*
TODO enter method description

*/

/*
1.8.2 Partition functions

*/
  static void HyperplanePart(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2,
        DistanceFunction *distFun );
/*
TODO enter method description

*/

  static void BalancedPart(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2,
        DistanceFunction *distFun );
/*
TODO enter method description

*/
};

struct MTreeData
{
  virtual ~MTreeData()
  {}
/*
Virtual destructor.

*/

  virtual void Read( char *buffer, int &offset ) = 0;
/*
...

*/
  virtual void Write( char *buffer, int &offset ) = 0;
/*
...

*/
};

struct MTreeDataRef : public MTreeData
{
  MTreeDataRef( TupleId tid ) :
    tupleId ( tid ) 
  {}
/*
...

*/

  inline void Read( char *buffer, int &offset )
  {
    memcpy( &tupleId, buffer+offset, sizeof(TupleId) );
    offset += sizeof(TupleId);
  }
/*
...

*/

  inline void Write( char *buffer, int &offset )
  {
    memcpy( buffer+offset, &tupleId, sizeof(TupleId) );
    offset += sizeof(TupleId);
  }
/*
...

*/

  static int Size()
  {
    return sizeof(TupleId);
  }
/*
Returns the size of the entry in disk (without data member)

*/

  TupleId tupleId; // tuple identifier
};

struct MTreeDataInternal : public MTreeData
{
  inline MTreeDataInternal( char* datastr, size_t len ) :
    datastr ( datastr ),
    len ( len ) 
  {}
/*


*/
  inline void Read( char *buffer, int &offset )
  {
    memcpy( &len, buffer+offset, sizeof(int) );
    offset += sizeof(int);

    memcpy( datastr, buffer+offset, len );
    offset += len;
  }
/*
...

*/

  inline void Write( char *buffer, int &offset )
  {
    memcpy( buffer+offset, &len, sizeof(int) );
    offset += sizeof(int);

    memcpy( buffer+offset, datastr, len );
    offset += len;
  }
/*
...

*/

  static int StaticSize()
  {
    return sizeof(size_t);
  }

  inline int Size()
  {
    return sizeof(size_t) + len;
  }

  char* datastr; // data string for distance computations
  size_t len;    // number of chars stored in the data string
};

struct MTreeDataExternal : public MTreeData
{
  inline MTreeDataExternal( SmiRecordId dataRec ) :
    dataRec ( dataRec ) 
  {}
/*
...

*/


  inline void Read( char *buffer, int &offset )
  {
    memcpy( &dataRec, buffer+offset, sizeof(SmiRecordId));
    offset += sizeof(SmiRecordId);
  }
/*
...

*/

  inline void Write( char *buffer, int &offset )
  {
    memcpy( buffer+offset, &dataRec, sizeof(SmiRecordId));
    offset += sizeof(SmiRecordId);
  }
/*
...

*/

  static int Size()
  {
    return sizeof(SmiRecordId);
  }
  SmiRecordId dataRec;
};

/*
1.3 struct ~MTreeEntry~

*/
struct MTreeEntry
{
  double dist;      // distance to parent node
  MTreeData* data;  // data representation of the tuple

  virtual ~MTreeEntry()
  {}
/*
The virtual destructor.

*/

  virtual void Read( char *buffer, int &offset ) = 0;
/*
Reads an entry from the buffer. Offset is increased.

*/
  virtual void Write( char *buffer, int &offset ) = 0;
/*
Writes an entry to the buffer. Offset is increased.

*/
};

/*
1.8 struct ~MTreeLeafEntry~

*/
struct MTreeLeafEntry : public MTreeEntry
{
  TupleId tupleId; // tuple identifier

  inline MTreeLeafEntry( char *buffer, int offset )
  {
    Read( buffer, offset );
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

  void Read( char *buffer, int &offset );
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int &offset );
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

  inline MTreeRoutingEntry( char *buffer, int offset )
  {
    Read( buffer, offset );
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

  static int StaticSize()
  {
    return sizeof(SmiRecordId) + // chield
           sizeof(double) +      // dist
           sizeof(double);       // r
  }
/*
Returns the size of the entry in disk (without data member)

*/

  void Read( char *buffer, int &offset );
/*
Reads an entry from the buffer. Offset is increased.

*/

  void Write( char *buffer, int &offset );
/*
Writes an entry from the buffer. Offset is increased.

*/
};

/*
1.4 class ~DistanceFunction~

Wrapper class to allow distance computations on ~MTreeEntry~ objects directly.

*/
class DistanceFunction
{
 public:
  inline virtual ~DistanceFunction()
  {}
/*
The virtual destructor.

*/

  virtual double Distance( const MTreeEntry *e1,
                           const MTreeEntry *e2 ) const = 0;
/*
This method should return the distance beetween e1 and e2

*/
};

/*
1.5 Class ~DFReference~

This wrapper class is used, if the metric needs the Objects itself

*/
class DFReference : public DistanceFunction
{
 public:
  inline DFReference( Relation *rel, int attrIndex, Metric distFun )
  {
    this->rel = rel;
    this->attrIndex = attrIndex;
    this->distFun = distFun;
  }

  inline double Distance( const MTreeEntry *e1, const MTreeEntry *e2 ) const
  {
    cout << "Distance function wrapper for REFERENCE representation called.\n";
    TupleId tid1 = ((MTreeDataRef*)e1->data)->tupleId;
    TupleId tid2 = ((MTreeDataRef*)e2->data)->tupleId;
    StandardMetricalAttribute *m1 = ( StandardMetricalAttribute *)
         ( rel->GetTuple(tid1) )->GetAttribute( attrIndex );
    StandardMetricalAttribute *m2 = ( StandardMetricalAttribute*)
         ( rel->GetTuple(tid2) )->GetAttribute( attrIndex );
    return distFun( m1,m2 );
  }

 private:
  int attrIndex;
  Relation *rel;
  Metric distFun;
};

/*
1.6 Class ~DFInternal~

This wrapper class is used, if the metric needs small data strings (at least
two entries must fit into one node)

*/
class DFInternal : public DistanceFunction
{
 public:
  inline DFInternal( Metric distFun )
  {
    this->distFun = distFun;
  }

  inline double Distance( const MTreeEntry *e1, const MTreeEntry *e2 ) const
  {
    cout << "Distance function wrapper for INTERNAL representation called.\n";
    char* data1 = ((MTreeDataInternal*)e1->data)->datastr;
    char* data2 = ((MTreeDataInternal*)e2->data)->datastr;
    return distFun( data1, data2 );
  }

 private:
  Metric distFun;
};

/*
1.7 Class ~DFExternal~

This wrapper class is used, if the metric needs long data strings. In this
case the strings are stored in seperate pages.

*/
class DFExternal : public DistanceFunction
{
 public:
  inline DFExternal( Metric distFun, SmiRecordFile* file )
  {
    this->distFun = distFun;
    this->file = file;
  }

  inline double Distance( const MTreeEntry *e1, const MTreeEntry *e2 ) const
  {
    cout << "Distance function wrapper for EXTERNAL representation called.\n";
    SmiRecordId page1 = ((MTreeDataExternal*)e1->data)->dataRec;
    SmiRecordId page2 = ((MTreeDataExternal*)e2->data)->dataRec;

    SmiRecord record;
    int size = file->GetRecordLength();
    char* data1[size];
    char* data2[size];

    assert( file->SelectRecord( page1, record, SmiFile::ReadOnly ));
    assert( record.Read( data1, size, 0 ));

    assert( file->SelectRecord( page2, record, SmiFile::ReadOnly ));
    assert( record.Read( data2, size, 0 ));

    return distFun( data1, data2 );
  }

 private:
  SmiRecordFile* file;
  Metric distFun;
};
#endif
