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
class MetricWrapper;
/*
1.3 Class SplitPolicy

This class contains all avaliable promote and partition functions, which both
together form the split policy. The split policy describes, how the entries in
a node are splittet into two nodes.

*/
class SplitPolicy
{
 public:
  SplitPolicy( MetricWrapper* metric,
               PROMFUN promFunId, PARTFUN partFunId );
/*
Constructor.

*/

  inline void Promote(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2 )
  {
    promFun( entries, prom1, prom2, metric );
  }

  void Partition(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2 )
  {
    partFun(entries, prom1, prom2, entries1, rad1, entries2, rad2, metric );
  }

 private:
  void (*promFun)(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        MetricWrapper *metric );
/*
Contains the selected promote function.

*/

  void (*partFun)(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2,
        MetricWrapper *metric );
/*
Contains the selected partition function.

*/

  MetricWrapper* metric;
/*
Contains the distance function wrapper for the respective metric

*/

/*
Promote functions

These methods promote two objects in the entries list and return them in prom1
and prom2. Furthermore the entries are deletet from entries-list.

*/

  static void RAND_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        MetricWrapper* metric );
/*
This method promtes two randomly selected elements.

*/

  static void MRAD_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        MetricWrapper *metric );
/*
TODO enter method description

*/


  static void MMRAD_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        MetricWrapper *metric );
/*
TODO enter method description

*/

  static void MLB_Prom(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        MetricWrapper *metric );
/*
TODO enter method description

*/

/*
Partition functions

*/

  static void HyperplanePart(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2,
        MetricWrapper *metric );
/*
TODO enter method description

*/

  static void BalancedPart(
        list<MTreeEntry*> &entries,
        MTreeEntry *prom1, MTreeEntry *prom2,
        list<MTreeEntry*> &entries1, double &rad1,
        list<MTreeEntry*> &entries2, double &rad2,
        MetricWrapper *metric );
/*
TODO enter method description

*/
};

/*
1.4 Struct ~MTreeData~

This class implements the data member im class ~MTreeEntry~

*/
struct MTreeData
{
  virtual ~MTreeData()
  {}
/*
Virtual destructor.

*/

  virtual void Read( char *buffer, int &offset ) = 0;
/*
Reads an data object from the buffer. Offset is increased.

*/

  virtual void Write( char *buffer, int &offset ) = 0;
/*
Writes an data object from the buffer. Offset is increased.

*/
};

/*
1.5 Struct MTreeDataRef

...

*/
struct MTreeData_Ref : public MTreeData
{
  TupleId tupleId; // tuple identifier

  inline MTreeData_Ref( char* buffer, int& offset)
  {
    Read( buffer, offset );
  }
/*
Constructor (reads the data values from buffer).

*/

  inline MTreeData_Ref( TupleId tid ) :
    tupleId ( tid ) 
  {}
/*
Standard Constructor.

*/

  inline void Read( char* buffer, int& offset )
  {
    memcpy( &tupleId, buffer+offset, sizeof(TupleId) );
    offset += sizeof(TupleId);
  }
/*
Reads an data object from the buffer. Offset is increased.

*/

  inline void Write( char* buffer, int& offset )
  {
    memcpy( buffer+offset, &tupleId, sizeof(TupleId) );
    offset += sizeof(TupleId);
  }
/*
Writes an data object from the buffer. Offset is increased.

*/

  static int Size()
  {
    return sizeof(TupleId);
  }
/*
Returns the size of the entry in disk (without data member)

*/
};

/*
1.6 Struct MTreeDataInt

*/
struct MTreeData_Int : public MTreeData
{
  char* datastr; // data string for distance computations
  size_t len;    // number of chars stored in the data string

  inline MTreeData_Int( char* buffer, int& offset )
  { Read( buffer, offset ); }
/*
Constructor (reads the data values from buffer).

*/

  inline MTreeData_Int( char* datastr, size_t len ) :
    datastr ( datastr ),
    len ( len ) {}
/*
Standard Constructor.

*/

  inline ~MTreeData_Int()
  { delete[] datastr; }
/*
Destructor.

*/

   inline void Read( char* buffer, int& offset )
  {
    memcpy( &len, buffer+offset, sizeof(size_t) );
    offset += sizeof(size_t);

    datastr = new char[len];
    memcpy( datastr, buffer+offset, len );
    offset += len;
  }
/*
Reads an data object from the buffer. Offset is increased.

*/

  inline void Write( char* buffer, int& offset )
  {
    memcpy( buffer+offset, &len, sizeof(size_t) );
    offset += sizeof(size_t);

    memcpy( buffer+offset, datastr, len );
    offset += len;
  }
/*
Writes an data object from the buffer. Offset is increased.

*/

  static int StaticSize()
  { return sizeof(size_t); }
};

/*
1.7 Struct MTreeDataExt

*/
struct MTreeData_Ext : public MTreeData
{
  SmiRecordId dataRec;

  inline MTreeData_Ext( char* buffer, int& offset )
{
  Read( buffer, offset );
}
/*
Constructor (reads the data values from buffer).

*/

  inline MTreeData_Ext( SmiRecordId dataRec ) :
    dataRec ( dataRec ) {}
/*
...

*/

  inline void Read( char* buffer, int& offset )
  {
    memcpy( &dataRec, buffer+offset, sizeof(SmiRecordId) );
    offset += sizeof(SmiRecordId);
  }
/*
Writes an data object from the buffer. Offset is increased.

*/

  inline void Write( char* buffer, int& offset )
  {
    memcpy( buffer+offset, &dataRec, sizeof(SmiRecordId) );
    offset += sizeof(SmiRecordId);
  }
/*
Reads an data object from the buffer. Offset is increased.

*/

  static int Size()
  { return sizeof(SmiRecordId); }
};

/*
1.8 struct ~MTreeEntry~

*/
struct MTreeEntry
{
  double dist;       // distance to parent node
  MTreeData* data;  // data representation of the tuple

  inline virtual ~MTreeEntry()
  { delete data; }
/*
The virtual destructor.

*/

  virtual void Read( char *buffer, int &offset, STORAGE_TYPE storageType ) = 0;
/*
Reads an entry from the buffer. Offset is increased.

*/

  virtual void Write( char *buffer, int &offset ) = 0;
/*
Writes an entry to the buffer. Offset is increased.

*/
};

/*
1.9 Class ~MetricWrapper~

This class encapsulates the metrics defined in ~MetricRegistry~ from their usage
in m-tree. Due to the various possibilities for the data stored in the m-tree (
reference, internal or external) there are various ways to extract the data from
the internal storage to get the appropriate parameters for the metric. This is
done by the static methods of this class. Which implementation will be used is
selected by the ~storage~ parameter in the constructor.

*/
class MetricWrapper
{
 public:
  inline MetricWrapper(Metric metric, Relation* rel, int attrIndex,
                       SmiRecordFile* file, STORAGE_TYPE storage )
  {
    supp = new MWSupp( metric, rel, attrIndex, file );

    switch( storage )
    {
      case REFERENCE:
        metricImpl = DistRef;
        break;
      case INTERNAL:
        metricImpl = DistInt;
        break;
      case EXTERNAL:
        metricImpl = DistExt;
        break;
    }
  }
/*
The constructor selects the appropriate implementation and initiates the
~MWSupp~ object (see below).

*/

  ~MetricWrapper()
  { delete supp; }
/*
Destructor.

*/

  inline void Distance( const MTreeEntry* e1, const MTreeEntry* e2, double&
result )
  {
    metricImpl( e1, e2, result, supp );
  }
/*
This method calls the previously selected implementation.

*/

private:
  struct MWSupp
  {
    MWSupp( Metric metric, Relation* rel, int attrIndex, SmiRecordFile* file) :
      metric ( metric ),
      rel ( rel ),
      attrIndex ( attrIndex ),
      file ( file )
    {}

    Metric metric;
    Relation* rel;
    int attrIndex;
    SmiRecordFile* file;
  } *supp;
/*
This struct contains all elements, which are needed from at least one
implementation. A pointer to the supp object is assigned as additional
parameter to the wrapper implementation methods.

*/

  void (*metricImpl)( const MTreeEntry* e1, const MTreeEntry* e2,
                      double& result, MWSupp* supp );
/*
Pointer to the used wrapper implementation method

*/

  static void DistRef( const MTreeEntry* e1, const MTreeEntry* e2,
                       double& result, MWSupp* supp );
/*
Wrapper implementation used for calls with storage type REFERENCE

*/

  static void DistInt( const MTreeEntry* e1, const MTreeEntry* e2,
                       double& result, MWSupp* supp );
/*
Wrapper implementation used for calls with storage type INTERNAL

*/

  static void DistExt( const MTreeEntry* e1, const MTreeEntry* e2,
                       double& result, MWSupp* supp );
/*
Wrapper implementation used for calls with storage type EXTERNAL

*/
};

#endif
