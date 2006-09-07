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

November 2004. M. Spiekermann

June 2005 M. Spiekermann. The attributes array will now be a 
private member of the class ~Tuple~. Moreover it will be a member 
variable and calls for new and delete are saved. 

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

*/

#ifndef INC_RELALG_PERSISTENT_H
#define INC_RELALG_PERSISTENT_H

#ifdef RELALG_PERSISTENT
/*
This ~RELALG\_PERSISTENT~ defines which kind of relational algebra
is to be compiled. If it is set, the persistent version of the 
relational algebra will be compiled, and otherwise, the main memory 
version will be compiled.

*/

using namespace std;

#include "SecondoSMI.h"

extern AlgebraManager* am;

/*
3 Type constructor ~tuple~

3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/
enum TupleState {Fresh, Solid};

struct PrivateTuple
{
  PrivateTuple( TupleType *tupleType ):
    tupleId( 0 ),
    tupleType( tupleType ),
    attributes( 0 ),
    lobFileId( 0 ),
    tupleFile( 0 ),
    memoryTuple( 0 ),
    extensionTuple( 0 ),
    state( Fresh )
    {
      if (debug)	    
        cerr << "Tuple " << (void*)this 
             << ": Constructed by 1st function."  << endl;
      tupleType->IncReference();
    }
/*
The first constructor. It creates a fresh tuple from a ~tupleType~.

*/
  PrivateTuple( const ListExpr typeInfo ):
    tupleId( 0 ),
    tupleType( new TupleType( typeInfo ) ),
    attributes( 0 ),
    lobFileId( 0 ),
    tupleFile( 0 ),
    memoryTuple( 0 ),
    extensionTuple( 0 ),
    state( Fresh )
    {
      if (debug)	    
        cerr << "Tuple " << (void*)this 
             << ": Constructed by 2nd function."  << endl;
    }
/*
The second constructor. It creates a fresh tuple from a ~typeInfo~.

*/
  ~PrivateTuple()
  {
    if (debug)	   
      cerr << "Tuple " << (void*)this 
	   << ": about to destruct. state = " << state 
           << " mem = " << (void*)memoryTuple 
           << " ext = " << (void*)extensionTuple << endl;   
    if( state == Solid )
    {
    	    
      assert( memoryTuple != 0 );
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      {
        attributes[i]->Finalize();
        for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
        {
          FLOB *flob = attributes[i]->GetFLOB(j);
          if( flob->GetType() != Destroyed )
            flob->Clean();
        }
      }
      free( memoryTuple );
      if( extensionTuple != 0 )
        free( extensionTuple );
    }
    else
    {
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
        if( attributes[i] != 0 )
          attributes[i]->DeleteIfAllowed();
    }
    tupleType->DeleteIfAllowed();
  }
/*
The destructor.

*/
    inline void CopyAttribute( int sourceIndex, 
                               const PrivateTuple *source, 
                               int destIndex )
    {
      assert( state == Fresh );
      if( attributes[destIndex] != 0 )
        attributes[destIndex]->DeleteIfAllowed();

      if( source->state == Fresh )
        attributes[destIndex] = source->attributes[sourceIndex]->Copy();
      else
        attributes[destIndex] = source->attributes[sourceIndex]->Clone();
    }
/*
This function is used to copy attributes from tuples to tuples 
without cloning attributes.

*/
  void Save( SmiRecordFile *tuplefile, SmiFileId& lobFileId,
             double& extSize, double& size,
             vector<double>& attrExtSize, vector<double>& attrSize );
/*
Saves a fresh tuple into ~tuplefile~ and ~lobfile~. Returns the 
sizes of the tuple saved.

*/
  void UpdateSave( const vector<int>& changedIndices,
                   double& extSize, double& size,
                   vector<double>& attrExtSize,
                   vector<double>& attrSize );
/*
Saves a solid tuple with updated attributes and reuses the old 
record. This function is implemented in the Update Relation Algebra.

*/
  bool Open( SmiRecordFile *tuplefile, SmiFileId lobfileId,
             SmiRecordId rid );
/*
Opens a solid tuple from ~tuplefile~(~rid~) and ~lobfile~.

*/
  bool Open( SmiRecordFile *tuplefile, SmiFileId lobfileId,
             PrefetchingIterator *iter );
/*
Opens a solid tuple from ~tuplefile~ and ~lobfile~ reading the 
current record of ~iter~.

*/
  bool Open( SmiRecordFile *tuplefile, SmiFileId lobfileId,
             SmiRecord *record );                
/*
Opens a solid tuple from ~tuplefile~ and ~lobfile~ reading from 
~record~.

*/

  SmiRecordId tupleId;
/*
The unique identification of the tuple inside a relation.

*/
  mutable TupleType *tupleType;
/*
Stores the tuple type.

*/
  Attribute **attributes;
/*
The attributes pointer array. The tuple information is kept in 
memory.

*/
  SmiFileId lobFileId;
/*
Reference to an ~SmiRecordFile~ which contains LOBs.

*/
  SmiRecordFile* tupleFile;
/*
Reference to an ~SmiRecordFile~ which contains the tuple.

*/
  char *memoryTuple;
/*
The representation of the attributes in the solid tuple.

*/
  char *extensionTuple;
/*
The representation of the small flobs in the solid tuple.

*/
  TupleState state;
/*
State of the tuple (Fresh, Solid).

The members below are useful for debugging:

*/
#ifdef MALLOC_CHECK_  
  void free (void* ptr) { cerr << "freeing ptr " << ptr << endl; ::free(ptr); }
#endif
  static bool debug;
};

/*
4.2 Struct ~RelationDescriptor~

This struct contains necessary information for opening a relation.

*/
struct RelationDescriptor
{
  inline
  RelationDescriptor( TupleType* tupleType ):
    tupleType( tupleType ),
    noTuples( 0 ),
    totalExtSize( 0.0 ),
    totalSize( 0.0 ),
    attrExtSize( tupleType->GetNoAttributes() ),
    attrSize( tupleType->GetNoAttributes() ),
    tupleFileId( 0 ),
    lobFileId( 0 )
    {
      tupleType->IncReference();
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      {
        attrExtSize[i] = 0.0;
        attrSize[i] = 0.0;
      }
    }

  inline
  RelationDescriptor( const ListExpr typeInfo ):
    tupleType( new TupleType( nl->Second( typeInfo ) ) ),
    noTuples( 0 ),
    totalExtSize( 0.0 ),
    totalSize( 0.0 ),
    attrExtSize( 0 ),
    attrSize( 0 ),
    tupleFileId( 0 ),
    lobFileId( 0 )
    {
      attrExtSize.resize( tupleType->GetNoAttributes() );
      attrSize.resize( tupleType->GetNoAttributes() );
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      {
        attrExtSize[i] = 0.0;
        attrSize[i] = 0.0;
      }
    }
/*
The simple constructors.

*/
  inline
  RelationDescriptor( TupleType *tupleType,
                      int noTuples,
                      double totalExtSize, double totalSize,
                      const vector<double>& attrExtSize,
                      const vector<double>& attrSize,
                      const SmiFileId tId, const SmiFileId lId ):
    tupleType( tupleType ),
    noTuples( noTuples ),
    totalExtSize( totalExtSize ),
    totalSize( totalSize ),
    attrExtSize( attrExtSize ),
    attrSize( attrSize ),
    tupleFileId( tId ),
    lobFileId( lId )
    {
      tupleType->IncReference();
    }

  inline
  RelationDescriptor( const ListExpr typeInfo,
                      int noTuples,
                      double totalExtSize, double totalSize,
                      const vector<double>& attrExtSize,
                      const vector<double>& attrSize,
                      const SmiFileId tId, const SmiFileId lId ):
    tupleType( new TupleType( nl->Second( typeInfo ) ) ),
    noTuples( noTuples ),
    totalExtSize( totalExtSize ),
    totalSize( totalSize ),
    attrExtSize( attrExtSize ),
    attrSize( attrSize ),
    tupleFileId( tId ),
    lobFileId( lId )
    {
    }

/*
The first constructor.

*/
  inline
  RelationDescriptor( const RelationDescriptor& desc ):
    tupleType( desc.tupleType ),
    noTuples( desc.noTuples ),
    totalExtSize( desc.totalExtSize ),
    totalSize( desc.totalSize ),
    attrExtSize( desc.attrExtSize ),
    attrSize( desc.attrSize ),
    tupleFileId( desc.tupleFileId ),
    lobFileId( desc.lobFileId )
    {
      tupleType->IncReference();
    }
/*
The copy constructor.

*/
  inline
  ~RelationDescriptor()
  {
    tupleType->DeleteIfAllowed();
  }
/*
The destructor.

*/
  inline RelationDescriptor& operator=( const RelationDescriptor& d )
  {
    tupleType->DeleteIfAllowed();
    tupleType = d.tupleType;
    tupleType->IncReference();
    noTuples = d.noTuples;
    totalExtSize = d.totalExtSize;
    totalSize = d.totalSize;
    tupleFileId = d.tupleFileId;
    lobFileId = d.lobFileId;
    attrExtSize = d.attrExtSize;
    attrSize = d.attrSize;

    return *this;
  }
/*
Redefinition of the assignement operator.

*/

  TupleType *tupleType;
/*
Stores the tuple type of every tuple in the relation.

*/
  int noTuples;
/*
The quantity of tuples inside the relation.

*/
  double totalExtSize;
/*
The total size occupied by the tuples in the relation taking
into account the small FLOBs, i.e. the extension part of
the tuples.

*/
  double totalSize;
/*
The total size occupied by the tuples in the relation taking
into account all parts of the tuples, including the FLOBs.

*/
  vector<double> attrExtSize;
/*
The total size occupied by the attributes in the relation
taking into account the small FLOBs, i.e. the extension part
of the tuples.

*/
  vector<double> attrSize;
/*
The total size occupied by the attributes in the relation
taking into account all parts of the tuples, including the
FLOBs.

*/
  SmiFileId tupleFileId;
/*
The tuple's file identification.

*/
  SmiFileId lobFileId;
/*
The LOB's file identification.

*/
};

/*
4.2 Class ~RelationDescriptorCompare~

*/
class RelationDescriptorCompare
{
  public:
    inline bool operator()( const RelationDescriptor& d1, 
                            const RelationDescriptor d2 )
    {
      if( d1.tupleFileId < d2.tupleFileId )
        return true;
      else if( d1.tupleFileId == d2.tupleFileId &&
               d1.lobFileId == d2.lobFileId )
        return true;
      else
        return false;
    }
};

/*
4.1 Struct ~PrivateRelation~

This struct contains the private attributes of the class ~Relation~.

*/
struct PrivateRelation
{
  PrivateRelation( const ListExpr typeInfo, bool isTemp ):
    relDesc( new TupleType( nl->Second( typeInfo ) ) ),
    tupleFile( false, 0, isTemp ),
    isTemp( isTemp )
    {
      if( !tupleFile.Create() )
      {
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
        assert( false );
      }
      relDesc.tupleFileId = tupleFile.GetFileId();
    }
/*
The first constructor. Creates an empty relation from a ~typeInfo~.

*/
  PrivateRelation( TupleType *tupleType, bool isTemp ):
    relDesc( tupleType ),
    tupleFile( false, 0, isTemp ),
    isTemp( isTemp )
    {
      tupleType->IncReference();
      if( !tupleFile.Create() )
      {
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
        assert( false );
      }
      relDesc.tupleFileId = tupleFile.GetFileId();
    }
/*
The second constructor. Creates an empty relation from a ~tupleType~.

*/
  PrivateRelation( const RelationDescriptor& relDesc, 
                   bool isTemp ):
    relDesc( relDesc ),
    tupleFile( false, 0, isTemp ),
    isTemp( isTemp )
    {
      if( !tupleFile.Open( relDesc.tupleFileId ) )
      {
        string error;
        SmiEnvironment::GetLastErrorCode( error );
        cout << error << endl;
        assert( false );
      }
      this->relDesc.tupleFileId = tupleFile.GetFileId();
    }
/*
The third constructor. Opens a previously created relation.

*/
  ~PrivateRelation()
  {
    tupleFile.Close();
  }
/*
The destuctor.

*/
  RelationDescriptor relDesc;
/*
Stores the descriptor of the relation.

*/
  SmiRecordFile tupleFile;
/*
Stores a handle to the tuple file.

*/
  bool isTemp;
/*
A flag telling whether the relation is temporary.

*/
};

#endif // RELALG_PERSISTENT
#endif // INC_RELALGPERSISTENT_H
