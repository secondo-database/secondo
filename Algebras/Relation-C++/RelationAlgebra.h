/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

[1] Header file  of the Relational Algebra

March 2003 Victor Almeida created the new Relational Algebra
organization

Oct 2004 M. Spiekermann changed some frequently called ~small~
functions into inline functions implemented in the header file. This
reduced code redundance since the same code written in
RelationMainMemory and RelationPersistent can be kept together here
and may improve performance when the code is compiled with
optimization flags.

June-July 2004 M. Spiekermann. Changes in class ~Tuple~ and
~TupleBuffer~. Storing the attribute array as member variable in
class Tuple reduces processing overhead. Moreover the array is
needed for both implementations "persistent" and "memory" hence it
should not be maintained in class ~privateTuple~. The TupleBuffer
was extended. The TupleBuffer constructor was extended by a new
boolean parameter which indicates whether the tuples in it are
stored as "free" or "non-free" tuples.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

May 2006 M. Spiekermann. Changes in ~TupleCompareBy~. The number of compare
operations was antisymmetric, e.g. recognizing that two attributes fulfill
$A > B$ needed one integer comparison more than $A < B$. Now first $A = B$ is
tested and determining one of the remaining cases needs only one extra
comparison. Hence in the average we will use less ${=,<}$ operations than
before.

April 2007, T. Behr, M. Spiekermann. Removal of the main memory implementation
of this algebmain memory implementation
of this algebra module.

September 2007, M. Spiekermann. Dependencies to algebra OldRelationAlgebra removed.

June 2009, S. Jungnickel. Added classes ~TupleFile~ and ~TupleFileIterator~.
New methods ~Save~ and ~Open~ in class ~Tuple~ to save and restore tuples to/from
a ~TupleFile~.

Sept 2009. M. Spiekermann. Due to problems with too many open files communicated
by S. Jungnickel several codelines of the classes FLOBCache, Relation and in the
StorageManagement need to be changed. Moreover, the class ~PrivateRelation~ has been
merged into class Relation. The relation itself will now request the FLOBCache to create
LOB-files if necessary. Before, files were created on the fly indicated by a zero
lobFile-id which was not a satisfying solution for all situations.

[TOC]


1 Overview

The Relational Algebra basically implements two type constructors,
namely ~tuple~ and ~rel~. The type system of the Relational Algebra
can be seen below.

\begin{displaymath}
\begin{array}{lll}
&
\to \textrm{DATA} &
{\underline{\smash{\mathit{int}}}},
{\underline{\smash{\mathit{real}}}},
{\underline{\smash{\mathit{bool}}}},
{\underline{\smash{\mathit{string}}}}
\\
({\underline{\smash{\mathit{ident}}}} \times \textrm{DATA})^{+} &
\to \textrm{TUPLE} &
{\underline{\smash{\mathit{tuple}}}}
\\
\textrm{TUPLE} &
\to \textrm{REL} &
{\underline{\smash{\mathit{rel}}}}
\end{array}
\end{displaymath}

The DATA kind should be incremented with more complex data types
such as, for example, ${\underline{\smash{\mathit{point}}}}$,
${\underline{\smash{\mathit{points}}}}$,
${\underline{\smash{\mathit{line}}}}$,
and ${\underline{\smash{\mathit{region}}}}$ of the Spatial Algebra,
meaning that these data types can be inserted into relations.

As an example, a relation ~cities~ could be described as

\begin{displaymath}
{\underline{\smash{\mathit{rel}}}}
  ({\underline{\smash{\mathit{tuple}}}}
    (<
      (\textrm{name}, {\underline{\smash{\mathit{string}}}}),
      (\textrm{country}, {\underline{\smash{\mathit{string}}}}),
      (\textrm{pop}, {\underline{\smash{\mathit{int}}}}),
      (\textrm{pos}, {\underline{\smash{\mathit{point}}}})
    >)
  )
\end{displaymath}

This file will contain an interface of the memory representation
structures (~classes~) for these two type constructors, namely
~Tuple~ and ~Relation~, and some additional ones that are needed
for the Relational Algebra, such as ~TupleId~, ~RelationIterator~,
~TupleType~, ~Attribute~ (which is defined inside the file
Attribute.h), ~AttributeType~, and ~RelationDescriptor~.


                Figure 1: Relational Algebra implementation
                architecture. [RelationAlgebraArchitecture.eps]

2 Defines, includes, and constants

*/
#ifndef _RELATION_ALGEBRA_H_
#define _RELATION_ALGEBRA_H_

#include <iostream>
#include <map>
#include <list>
#include <deque>

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include "Trace.h"
#include "Algebra.h"
#include "Attribute.h"
#include "NestedList.h"
#include "Counter.h"
#include "../TupleIdentifier/TupleIdentifier.h"

#define MAX_NUM_OF_ATTR 10

extern AlgebraManager* am;


/*
3.2 Class ~Attribute~

This abstract class ~Attribute~ is inside the file Attribute.h and
contains a set of functions necessary to the management of
attributes. All type constructors of the kind DATA must be a
sub-class of ~Attribute~.

3.3 Struct ~AttributeType~

This ~AttributeType~ struct implements the type of each attribute
inside a tuple. To identify a data type in the Secondo system the
~algebraId~ and the ~typeId~ are necessary. The size of the
attribute is also necessary to previously know how much space will
be necessary to store an instance of such attribute's data type.

*/
struct AttributeType
{
  friend ostream& operator<<(ostream& o, AttributeType& at);

  AttributeType():
    algId(0), typeId(0), numOfFlobs(0),size(0),
    coreSize(0),extStorage(0),offset(0)
    {}
/*
This constructor should not be used.

*/
  AttributeType( int in_algId,
                 int in_typeId,
                 int in_numOfFlobs,
                 int in_size,
                 int in_coreSize,
                 bool in_extStorage,
                 size_t in_offset = 0 ):
    algId( in_algId ),
    typeId( in_typeId ),
    numOfFlobs( in_numOfFlobs ),
    size( in_size ),
    coreSize( in_coreSize ),
    extStorage( in_extStorage ),
    offset( in_offset )
    {}
/*
The constructor.

*/
  int algId;
/*
The data type's algebra ~id~ of the attribute.

*/
  int typeId;
/*
The data type's ~id~ of the attribute.

*/
  int numOfFlobs;
  int size;
  int coreSize;
  bool extStorage;
/*
Size of attribute instance in bytes.

*/
  size_t offset;
};

ostream& operator<<(ostream& o, AttributeType& at);


/*
3.4 Class ~TupleType~

A ~TupleType~ is a collection (an array) of all attribute types
(~AttributeType~) of the tuple. This structure contains the metadata of a tuple attributes.

*/
class TupleType
{
  friend ostream& operator<<(ostream& o, const TupleType& tt);
  public:
    TupleType( const ListExpr typeInfo );
/*
The first constructor. Creates a tuple type from a ~typeInfo~ list
expression. It sets all member variables, including the total size.

*/

    ~TupleType()
    {
      if(attrTypeArray){
         delete []attrTypeArray;
         attrTypeArray = 0;
      }
    }
/*
The destructor.

*/
    inline bool DeleteIfAllowed()
    {
      assert( refs > 0 );
      refs--;
      if( refs == 0 ){
        delete this;
        return true;
      } else {
         return false;
      }
    }
/*
Deletes the tuple type if allowed, i.e. there are no more
references to it.

*/
    inline void IncReference()
    {
      refs++;
    }
/*
Increment the reference count of this tuple type.

*/
    inline int GetNoAttributes() const
    {
      return noAttributes;
    }
/*
Returns the number of attributes of the tuple type.

*/
    inline int GetTotalSize() const
    {
      return totalSize;
    }
/*
Returns the total size of the tuple.

*/
    inline int GetCoreSize() const
    {
      return coreSize;
    }
/*
Returns the core size of the tuple.

*/

    inline const AttributeType& GetAttributeType( int index ) const
    {
      return attrTypeArray[index];
    }
/*
Returns the attribute type at ~index~ position.

*/
    inline void PutAttributeType( int index,
                                  const AttributeType& attrType )
    {
      attrTypeArray[index] = attrType;
    }
/*
Puts the attribute type ~attrType~ in the position ~index~.

*/
    inline int NumOfFlobs() const
    {
      int n = 0;
      for(int i = 0; i < noAttributes; i++) {
        n += attrTypeArray[i].numOfFlobs;
      }
      return n;
    }



  private:

    int noAttributes;
/*
Number of attributes.

*/
   AttributeType* attrTypeArray;
/*
Array of attribute type descriptions.

*/
    int totalSize;
/*
Sum of all attribute sizes.

*/
    int refs;
/*
A reference counter.

*/

    int coreSize;
};

ostream& operator<<(ostream& o, const TupleType& tt);

/*
4.2 Struct ~RelationDescriptor~

This struct contains necessary information for opening a relation.

*/
struct RelationDescriptor
{
  friend ostream& operator<<(ostream& o, const RelationDescriptor& rd);
  inline RelationDescriptor( TupleType* tupleType, bool b=false ):
    isTemp(b),
    tupleType( tupleType ),
    attrExtSize( tupleType->GetNoAttributes() ),
    attrSize( tupleType->GetNoAttributes() )
    {
      tupleType->IncReference();
      init();
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      {
        attrExtSize[i] = 0.0;
        attrSize[i] = 0.0;
      }
    }

  inline
  RelationDescriptor( const ListExpr typeInfo, bool b=false ):
    isTemp(b),
    tupleType( new TupleType( nl->Second( typeInfo ) ) ),
    attrExtSize( 0 ),
    attrSize( 0 )
    {
      // tuple type was created, no need to increment the reference counter
      init();
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
    isTemp(false),
    tupleType( tupleType ),
    attrExtSize( attrExtSize ),
    attrSize( attrSize )
    {
      tupleType->IncReference();
      init(noTuples, totalExtSize, totalSize, tId, lId);
    }

  inline
  RelationDescriptor( const ListExpr typeInfo,
                      int noTuples,
                      double totalExtSize, double totalSize,
                      const vector<double>& attrExtSize,
                      const vector<double>& attrSize,
                      const SmiFileId tId, const SmiFileId lId ):
    isTemp(false),
    tupleType( new TupleType( nl->Second( typeInfo ) ) ),
    attrExtSize( attrExtSize ),
    attrSize( attrSize )
    {
      // tuple type was created, no need to increment the reference counter
      init(noTuples, totalExtSize, totalSize, tId, lId);
    }

/*
The first constructor.

*/
  inline RelationDescriptor( const RelationDescriptor& d ):
    isTemp( d.isTemp ),
    tupleType( d.tupleType ),
    attrExtSize( d.attrExtSize ),
    attrSize( d.attrSize )
    {
      tupleType->IncReference();
      init(d.noTuples, d.totalExtSize, d.totalSize,
           d.tupleFileId, d.lobFileId);
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

    init(d.noTuples, d.totalExtSize, d.totalSize,
         d.tupleFileId, d.lobFileId);
    attrExtSize = d.attrExtSize;
    attrSize = d.attrSize;
    isTemp = d.isTemp;

    return *this;
  }
/*
Definition of the assignment operator.

*/


  inline void init(  int in_noTuples = 0,
                     double in_totalExtSize = 0.0,
                     double in_totalSize = 0.0,
                     int in_tupleFileId = 0,
                     int in_lobFileId = 0 )
  {
    noTuples = in_noTuples;
    totalExtSize = in_totalExtSize;
    totalSize = in_totalSize;
    tupleFileId = in_tupleFileId;
    lobFileId = in_lobFileId;
  }
/*
Initialization of some members which are defined in the same way
for all constructor variants.

*/




  bool isTemp;
/*
A flag telling whether the relation is temporary.

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


ostream& operator<<(ostream& o, const RelationDescriptor& rd);


class TupleFile;
class TupleFileIterator;

/*
Necessary forward declarations for class ~Tuple~.

*/

/*
3.5 Class ~Tuple~

This class implements the representation of the type
constructor ~tuple~.

*/

/*
Declaration of the struct ~PrivateTuple~. This struct contains the
private attributes of the class ~Tuple~.

*/

class Tuple
{
  public:

    inline Tuple( TupleType* _tupleType ) :
    refs(1), recomputeExtSize(true), recomputeSize(true),
    tupleExtSize(0), tupleSize(0),noAttributes(0), attributes(0),
    tupleId(0), tupleType( _tupleType ), lobFileId(0), tupleFile(0)
    {
      tupleType->IncReference();
      Init( tupleType->GetNoAttributes());
      DEBUG_MSG("Constructor Tuple(TupleType *tupleType) called.")
    }

/*
The constructor. It contructs a tuple with the metadata passed in
the ~tupleType~ as argument.

*/
    inline Tuple( const ListExpr typeInfo ) :
    refs(1), recomputeExtSize(true), recomputeSize(true),
    tupleExtSize(0), tupleSize(0),noAttributes(0), attributes(0),
    tupleId(0), tupleType( new TupleType( typeInfo ) ) ,
    lobFileId(0), tupleFile(0)
        {
      Init( tupleType->GetNoAttributes());
      DEBUG_MSG("Constructor Tuple(const ListExpr typeInfo) called.")
    }
/*
A similar constructor as the above, but taking a list
expression ~typeInfo~ as argument.

*/
  ~Tuple();
/*
The destructor.

*/
    static Tuple *In( const ListExpr typeInfo, ListExpr value,
                      int errorPos, ListExpr& errorInfo,
                      bool& correct );
/*
Creates a tuple from the ~typeInfo~ and ~value~ information.
Corresponds to the ~In~-function of type constructor ~tuple~.

*/
    static Tuple *RestoreFromList( const ListExpr typeInfo,
                                   ListExpr value,
                                   int errorPos,
                                   ListExpr& errorInfo,
                                   bool& correct );
/*
Acts as the ~In~ function, but uses internal representation for
the objects.

*/
    ListExpr Out( ListExpr typeInfo );
/*
Writes a tuple into a ~ListExpr~ format.
Corresponds to the ~Out~-function of type constructor ~tuple~.

*/
    ListExpr SaveToList( ListExpr typeInfo );
/*
Acts as the ~Out~ function, but uses internal representation for
the objects.

*/

    static const string BasicType(){
      return "tuple";
    }
    static const bool checkType(const ListExpr type){
      return listutils::isTupleDescription(type);
    }


    static void InitCounters(bool visible);
    static void SetCounterValues();
/*
Initialize tuple counters and assign values.

*/
    const TupleId& GetTupleId() const;
/*
Returns the unique ~id~ of the tuple.

*/
    void SetTupleId( const TupleId& tupleId );
/*
Sets the tuple unique ~id~ of the tuple. This function is necessary
because at the construction time, the tuple does not know its ~id~.

*/
    inline Attribute* GetAttribute( int index ) const
    {
      return attributes[index];
    }
/*
Returns the attribute at position ~index~ inside the tuple.

*/
    void PutAttribute( int index, Attribute* attr );
/*
Puts an attribute in the position ~index~ inside the tuple.

*/
    void UpdateAttributes( const vector<int>& changedIndices,
                           const vector<Attribute*>& newAttrs,
                           double& extSize, double& size,
                           vector<double>& attrExtSize,
                           vector<double>& attrSize );
/*
Puts the attributes from ~newAttrs~ at the corresponding position
from ~changedIndices~ into the tuple. Destroys the physical
representations of the old attributes and saves the new tuple to
disk. The implementation of this function is found in the
Update Relation Algebra.

*/

    inline void PinAttributes(){
      for(unsigned i=0;i<noAttributes;i++){
        attributes[i]->Pin();
      }
    }

    inline int GetRootSize() const
    {
      return tupleType->GetCoreSize();
    }
/*
Returns the size of the tuple's root part.

*/
    inline int GetRootSize( int i ) const
    {
      return tupleType->GetAttributeType(i).coreSize;
    }
/*
Returns the size of the attribute's root part.

*/
    inline int GetExtSize() const
    {
      if ( !recomputeExtSize )
        return tupleExtSize;

      tupleExtSize = 0;
      for( int i = 0; i < noAttributes; i++)
      {
        tupleExtSize += GetExtSize(i);
      }
      recomputeExtSize = false;
      return tupleExtSize;
    }
/*
Returns the size of the tuple taking into account the extension
part, i.e. the small FLOBs.

*/
    inline int GetExtSize( int i ) const
    {
      int attrExtSize = GetRootSize( i );
  //cout << "GetExtSize(i).GetRootSize(i)" << attrExtSize << endl;
  //cout << "size = " << tupleType->GetAttributeType(i).size;
  //cout << "coreSize = " << tupleType->GetAttributeType(i).coreSize;
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        Flob *tmpFlob = attributes[i]->GetFLOB(j);

        if( tmpFlob->getSize() < extensionLimit ) {
          attrExtSize += tmpFlob->getSize();
        }
      }

      if (tupleType->GetAttributeType(i).extStorage) {
        attrExtSize += attributes[i]->SerializedSize();
      }
      return attrExtSize;
    }
/*
Returns the size of attribute i including its extension part.

*/
   inline size_t GetMemSize() const{
      size_t tupleMemSize = sizeof(*this);
      if(noAttributes > MAX_NUM_OF_ATTR){ // do not use standard
        tupleMemSize += noAttributes + sizeof(Attribute*);
      }

      for( int i = 0; i < noAttributes; i++)
      {
        tupleMemSize += GetMemSize(i);
      }
      return tupleMemSize;
   }

   // returns the size in memory of attribute i
   inline size_t  GetMemSize(int i) const {
      if(attributes[i]){
       return attributes[i]->Sizeof() +
              attributes[i]->getUncontrolledFlobSize();
      } else {
          return 0;
      }
   }



    inline int GetSize() const
    {
      if ( !recomputeSize )
        return tupleSize;

      tupleSize = 0;
      for( int i = 0; i < noAttributes; i++)
      {
        tupleSize += GetSize(i);
      }
      recomputeSize = false;
      return tupleSize;
    }
/*
Returns the total size of the tuple taking into account the
all FLOB sizes.

*/
    inline int GetSize( int i ) const
    {
      int attrSize = GetRootSize(i);
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
        attrSize += attributes[i]->GetFLOB(j)->getSize();

      if (tupleType->GetAttributeType(i).extStorage) {
        attrSize += attributes[i]->SerializedSize();
      }
      return attrSize;
    }
/*
Returns the total size of attribute i taking
into account all FLOB sizes.

*/
    inline size_t HashValue(int i)
    {
      return static_cast<Attribute*>( GetAttribute(i) )->HashValue();
    }

/*
Returns the hash value for attribute number i.

*/



    inline int GetNoAttributes() const
    {
      return noAttributes;
    }
/*
Returns the number of attributes of the tuple.

*/
    inline TupleType* GetTupleType() const
    {
      return tupleType;
    }
/*
Returns the tuple type.

*/
    Tuple *Clone() const;
/*
Create a new tuple which is a clone of this tuple.

*/
    inline bool DeleteIfAllowed()
    {
      assert(refs>0);
      refs--;
      if( refs == 0 ){
        delete this;
        return true;
      } else{
        return false;
      }
    }
/*
Deletes the tuple if it is allowed, i.e., there are no references
(attribute ~refs~) to it anymore.

*/
    inline void IncReference()
    {
      refs++;
    }


    int GetNumOfRefs() const
    {
      return refs;
    }
/*
Returns the number of references

*/

/*
~CopyAttribute~

This function is used to copy attributes from tuples to tuples
without cloning attributes.

*/
  inline void CopyAttribute( int sourceIndex,
                             const Tuple *source,
                             int destIndex )
  {
    if( attributes[destIndex] != 0 ){
      // remove reference from an old attribute
      (attributes[destIndex]->DeleteIfAllowed());
    }
    attributes[destIndex] = source->attributes[sourceIndex]->Copy();
  }

/*
Saves a tuple into ~tuplefile~ and ~lobfile~. Returns the
sizes of the tuple saved.

*/

  void Save( SmiRecordFile *tuplefile, const SmiFileId& lobFileId,
             double& extSize, double& size,
             vector<double>& attrExtSize, vector<double>& attrSize,
             bool ignorePersistentLOBs=false );

/*
Saves a tuple into a temporary ~tuplefile~.

*/

  void Save( TupleFile& tuplefile ) ;

/*
Saves a tuple with updated attributes and reuses the old
record.

*/
  void UpdateSave( const vector<int>& changedIndices,
                   double& extSize, double& size,
                   vector<double>& attrExtSize,
                   vector<double>& attrSize );


  void SaveOrel(SmiRecord* record, SmiFileId& lobFileId,
                double& extSize, double& size,
                vector<double>& attrExtSize,
                vector<double>& attrSize,
                bool ignorePersistentLOBs, TupleId tupleId);

  bool OpenOrel( SmiFileId lobfileId,
                 SmiRecord& record, TupleId tupleId );

  bool OpenOrel(SmiFileId lobFileId,
                PrefetchingIterator* iter, TupleId tupleId);

  bool OpenPartialOrel( TupleType* newtype,
                        const list<int>& attrList,
                        SmiFileId lobfileId,
                        PrefetchingIterator* iter,
                        TupleId tupleId);

  void UpdateAttributesOrel( const vector<int>& changedIndices,
                             const vector<Attribute*>& newAttrs );




/*
Opens a tuple from ~tuplefile~(~rid~) and ~lobfile~.

*/
  bool Open( SmiRecordFile *tuplefile,
             SmiFileId lobfileId,
             SmiRecordId rid,
             const bool dontReportError );

/*
Opens a tuple from ~tuplefile~ and ~lobfile~ reading from
~record~.

*/
  bool Open( SmiRecordFile *tuplefile, SmiFileId lobfileId,
             SmiRecord *record,
             const bool dontReportError );

/*
Opens a tuple from ~tuplefile~ and ~lobfile~ reading the
current record of ~iter~.

*/
  bool Open( SmiRecordFile *tuplefile, SmiFileId lobfileId,
             PrefetchingIterator *iter );

  bool OpenPartial( TupleType* newtype, const list<int>& attrList,
                    SmiRecordFile *tuplefile,
                    SmiFileId lobfileId,
                    PrefetchingIterator *iter );

/*
Opens a tuple from a temporary ~TupleFile~ reading the
current record of ~iter~.

*/
  bool Open( TupleFileIterator *iter );

/*
Transform the tuple value to a Base 64 code string

*/

  string WriteToBinStr();

/*
Read a tuple value from a Base 64 code string

*/
  void ReadFromBinStr(string binStr);

/*
Write a complete tuple into an allocated binary block,
including its big flobs whose sizes are larger than ~extensionLimit~.
The flobs will be written after a normal tuple block.
At the same time, a new head value used to indicate how big
the extended memory block is.

*/
  void WriteToBin(char* buf,
      size_t coreSize, size_t extensionSize, size_t flobSize);

/*
Write a tuple into an allocated binary block,
but without its big flob.
Therefore, it has only one length heading.

*/
  void WriteTupleToBin(char* buf,
      size_t coreSize, size_t extensionSize);

  void WriteToDivBlock( char* buf,
      size_t coreSize,
      size_t extensionSize,
      size_t flobSize,
      SmiFileId flobFileId,
      SmiRecordId sourceDS,
      SmiSize& flobBlockOffset,
      map<pair<SmiFileId, SmiRecordId>, SmiSize>& flobIdCache) const;
/*
Write a tuple into an allocated binary block,
with or without its Flob data, decided by the caontainsLob value.

If contains the Flob, then it should generates the same result as the
~WriteToBin~ function.
If not, then its generated memory block contains two parts,
the first is the tuple data, while the second is the Flob data.
The blockSize recorded in the first tuple data does not count the flob data size.

*/


/*
Read a tuple from a binary block written by ~WriteToBin~.

*/
  u_int32_t ReadFromBin(char* buf, u_int32_t bSize = 0);

/*
Read a tuple from a binary block written by ~WriteToBin~,
but leave the FLOB data untouched

*/
  u_int32_t ReadTupleFromBin(char* buf, u_int32_t bSize,
                        string flobFile, size_t flobOffset);

/*
Read a tuple from a binary block written by ~WriteTupleToBin~,
the block contains only the tuple data, while the Flob data is kept in its
original persistent record file.
The flob mode is set 3 if the pDS is non-nagtive,
or else the flob is kept locally and the can be read as usual.

*/
  void ReadTupleFromBin(char* buf);

/*
Read the data from the disk file to a Flob with mode 1

*/

  void readLocalFlobFile(const string flobFilePath);

/*
Return the size that a complete binary tuple block needs.

*/
  size_t GetBlockSize(size_t& coreSize,
                         size_t& extensionSize,
                         size_t& flobSize,
                         vector<double>* attrExtSize = 0,
                         vector<double>* attrSize = 0) const;

  static SmiSize extensionLimit;
/*
Defines the maximum size used to store Flob data inside
a tuple

*/


  ostream& Print(ostream& out) const{
    out << "Tuple: (" << endl;
    for(int i = 0; i < noAttributes; i++) {
       GetAttribute(i)->Print(out);
       cout << endl;
    }
    out << "       )" << endl;
    return out;
  }

  private:

    static long tuplesCreated;
    static long tuplesDeleted;
    static long tuplesInMemory;
    static long maximumTuples;

/*
Some statistics about tuples.

*/


    inline void InitAttrArray()
    {
      for( int i = 0; i < noAttributes; i++ )
        attributes[i] = 0;
    }
/*
Initializes the attributes array with zeros.

*/
    inline void Init( int NoAttr)
    {
      noAttributes = NoAttr;

      refs = 1;
      tupleId = 0;
      tupleSize = 0;
      tupleFile = 0;
      tupleExtSize = 0;
      lobFileId =  0;

      recomputeExtSize = true;
      recomputeSize = true;

      if ( noAttributes > MAX_NUM_OF_ATTR ) {
        attributes = new Attribute*[noAttributes];
      } else {
        attributes = defAttributes;
      }
      InitAttrArray();

      tuplesCreated++;
      tuplesInMemory++;
      if( tuplesInMemory > maximumTuples ) {
        maximumTuples = tuplesInMemory;
      }
    }
/*
Initializes a tuple.

*/
    void ChangeTupleType(TupleType* newtype,
                         const list<int>& attrIds) {

        tupleType->DeleteIfAllowed();
        newtype->IncReference();
        tupleType = newtype;

        recomputeExtSize = true;
        recomputeSize = true;

        // save the current addresses stored in attributes
        Attribute** tmp_attributes = new Attribute*[noAttributes];
        for (int i = 0; i<noAttributes; i++) {
           tmp_attributes[i] = attributes[i];
        }

        // free old attribute array if necessary
        if (noAttributes > MAX_NUM_OF_ATTR){
          delete [] attributes;
        }

        // allocate new attribute array if necessary
        noAttributes = newtype->GetNoAttributes();
        if ( noAttributes > MAX_NUM_OF_ATTR ) {
          attributes = new Attribute*[noAttributes];
        } else {
          attributes = defAttributes;
        }

        // copy the old addresses in the given order into the
        // new attribute array
        int i = 0;
        list<int>::const_iterator iter = attrIds.begin();
        while ( iter != attrIds.end() ) {
           attributes[i] = tmp_attributes[*iter];
           iter++;
           i++;
        }

        delete [] tmp_attributes;
    }


    uint32_t refs;
/*
The reference count of this tuple. There can exist several tuple
pointers pointing to the same tuple and sometimes we want to prevent
deleting tuples. As an example, in some operators buffers that
store tuples in memory are used. Until the memory is kept in the
buffer it cannot be deleted by other pointers. For this case,
before appending a tuple in the buffer, the reference count of the
tuple is increased. To delete tuples we always call the function
~DeleteIfAllowed~, which first checks if ~refs~ = 0 to really
delete the tuple. This number is initialized with 0 so that normally
the tuple will be deleted.

*/

    mutable bool recomputeExtSize;
    mutable bool recomputeSize;
/*
These two flags together with the next two attributes are used
in order to avoid re-computing the extension and the total sizes of
tuples. The first time these sizes are computed they are stored.

*/

    mutable uint16_t tupleExtSize;
/*
Stores the size of the tuples taking into account the extension
part of the tuple, i.e. the small FLOBs.

*/
    mutable uint32_t tupleSize;
/*
Stores the total size of the tuples taking into account the
FLOBs.

*/
    uint8_t noAttributes;
/*
Store the number of attributes of the tuple.

*/

    Attribute** attributes;
    Attribute* defAttributes[MAX_NUM_OF_ATTR];
/*
The attribute array. If it contains less than ~MAX\_NUM\_OF\_ATTR~
entries, the it is statically constructed and ~attributes~ point
to ~defAttributes~, otherwise it is dinamically constructed.

*/


  static const size_t MAX_TUPLESIZE = 65535;
  static char tupleData[MAX_TUPLESIZE];

  char* WriteToBlock( size_t attrSizes,
                      size_t extensionSize,
                      bool ignoreLOBs,
                      SmiRecordFile* file,
                      const SmiFileId& lobFileId) const;

  void WriteToBlock( char* buf,
                     size_t coreSize,
                     size_t extensionSize,
                     bool ignoreLOBs,
                     SmiRecordFile* file,
                     const SmiFileId& lobFileId,
                     bool containLOBs = false) const;
/*
Write a tuple into a memory block, the size of the tuple is decided
by function ~CalculateBlockSize~.

----
tupleSize = coreSize + extensionSize + sizeof(TUPLESIZE-TYPE)
----

By default, Secondo only write a tuple's ~core~ , ~extension~
and small flobs into the memory block.
But in some situation, like in RemoteStreamAlgebra, the memory block
should contain the tuple's complete data, and the big flobs are
also written into the ~extension~ part.
In this case, the default TUPLESIZE-TYPE which is u\_int16\_t,
is not big enough to express the actual size of the tuple.
Therefore ~containLOBs~ is set to indicate whether the big flobs should
also be written into the memory block.
when ~containLOBs~ is true, the TUPLESIZE-TYPE will be set to u\_int32\_t,
and it's big enough to express the tuple's size

*/

  size_t CalculateBlockSize( size_t& attrSizes,
                             double& extSize,
                             double& size,
                             vector<double>& attrExtSize,
                             vector<double>& attrSize
                           ) const;


  char* GetSMIBufferData(SmiRecord& r, uint16_t& rootSize);
  char* GetSMIBufferData(PrefetchingIterator* iter,
                         uint16_t& rootSize);

  void InitializeAttributes(char* src, bool containLOBs = false);

/*
In ~WriteToBlock~, the big flobs may also be written into the memory
block, which is decided by ~containLOBs~.
Then when read a tuple back from a memory block, it also use ~containLOBs~
to indicate whether the big flobs are inside the memory block.
If it's true, then it will invoke Flob::createFromBlock function to read
big flobs back from the memory.

However, in the comment of Flob::createFromBlock, it said this function
does some evil thing, but it's not clear enough.

*/

  void InitializeSomeAttributes( const list<int>& attrList,
                                 char* src);

  void InitializeNoFlobAttributes(
    char* src, string flobFile = "", size_t flobOffset = 0);
/*
It reads the block data created by ~WriteToBlock~  with no FLOB data.
If the flobFile and the flobOffset are given, then set the flob to a local disk file.
Or else, just change the flob mode to 3.

*/

  bool ReadFrom(SmiRecord& record);



/*
The unique identification of the tuple inside a relation.

*/
  SmiRecordId tupleId;

/*
Stores the tuple type.

*/
  mutable TupleType* tupleType;

/*
Reference to an ~SmiRecordFile~ which contains LOBs.

*/
  SmiFileId lobFileId;
/*
Reference to an ~SmiRecordFile~ which contains the tuple.

*/
  SmiRecordFile* tupleFile;

/*
The members below are useful for debugging:

Profiling turned out that a separate member storing the number
of attributes makes sense since it reduces calls for TupleType::NoAttributes

*/


/*
Debugging stuff

*/
#ifdef MALLOC_CHECK_
  void free (void* ptr) { cerr << "freeing ptr " << ptr << endl; ::free(ptr); }
#endif

};

ostream& operator<<(ostream &os, Attribute &attrib);
ostream& operator<<( ostream& o, const Tuple& t );
/*
The print function for tuples. Used for debugging purposes

*/

/*
3.7 Class ~LexicographicalTupleCompare~

This is a class used in the sort algorithm that specifies the
lexicographical comparison function between two tuples.

*/

// use this one for sorting
class LexicographicalTupleSmaller
{
  public:
    inline bool operator()( const Tuple* aConst,
                            const Tuple* bConst ) const
    {
      Tuple* a = (Tuple*)aConst;
      Tuple* b = (Tuple*)bConst;

      for( int i = 0; i < a->GetNoAttributes(); i++ )
      {
        int cmp =
          ((Attribute*)a->GetAttribute(i))->Compare(
            ((Attribute*)b->GetAttribute(i)));
        if( cmp < 0)
          return true;
        if( cmp > 0)
          return false;
      }
      return false;
    }
};

class LexicographicalTupleGreater : public LexicographicalTupleSmaller
{
  public:
    inline bool operator()( const Tuple* a,
                            const Tuple* b ) const
    {
      return !LexicographicalTupleSmaller::operator()(a, b);
    }
};



// use this one to remove duplicates
class LexicographicalTupleSmallerAlmost
{
  public:
    inline bool operator()( const Tuple* aConst,
                            const Tuple* bConst ) const
    {
      for( int i = 0; i < aConst->GetNoAttributes(); i++ )
      {
        int cmp =
          ((Attribute*)aConst->GetAttribute(i))->CompareAlmost(
            ((Attribute*)bConst->GetAttribute(i)));
        if( cmp < 0)
          return true;
        if( cmp > 0)
          return false;
      }
      return false;
    }
};



class LexicographicalTupleCmp
{
  public:
    inline int operator()( const Tuple* a,
                            const Tuple* b) const
    {
      for( int i = 0; i < a->GetNoAttributes(); i++ )
      {
        int cmp =
          ((Attribute*)a->GetAttribute(i))->Compare(
            ((Attribute*)b->GetAttribute(i)));
        if( cmp != 0){
           return cmp;
        }
      }
      return 0;
    }
};


class LexicographicalTupleCmpAlmost
{
  public:
    inline int operator()( const Tuple* a,
                            const Tuple* b) const
    {
      for( int i = 0; i < a->GetNoAttributes(); i++ )
      {
        int cmp =
          ((Attribute*)a->GetAttribute(i))->CompareAlmost(
            ((Attribute*)b->GetAttribute(i)));
        if( cmp != 0){
           return cmp;
        }
      }
      return 0;
    }
};


/*
3.8 Class ~TupleCompareBy~

This is a class used in the sort algorithm that specifies the
comparison function between two tuples using a set of attributes
specified in ~SortOrderSpecification~, which is a vector of pairs
containing the index of the attribute and a boolean flag telling
whether the ordering is ascendant or not (descendant).

*/

// use this one to sort
typedef vector< pair<int, bool> > SortOrderSpecification;

class TupleCompareBy
{
  public:
    TupleCompareBy( const SortOrderSpecification &spec ):
      spec( spec ),
      len( spec.size() )
      {}

    inline bool operator()( const Tuple* a,
                            const Tuple* b ) const
    {
      if (len > 1)
      {

        SortOrderSpecification::const_iterator iter = spec.begin();
        while( iter != spec.end() )
        {
          const int pos = iter->first-1;
          const Attribute* aAttr = (const Attribute*) a->GetAttribute(pos);
          const Attribute* bAttr = (const Attribute*) b->GetAttribute(pos);
          const int cmpValue = aAttr->Compare( bAttr );

          if( cmpValue !=  0 )
          {
            // aAttr < bAttr ?
            return (cmpValue < 0) ? iter->second : !(iter->second);
          }
          // the current attribute is equal
          iter++;
        }
        // all attributes are equal
        return false;
      }
      else
      {
        const int pos = spec[0].first-1;
        const Attribute* aAttr = (const Attribute*) a->GetAttribute(pos);
        const Attribute* bAttr = (const Attribute*) b->GetAttribute(pos);
        return aAttr->Less(bAttr) ? spec[0].second : !spec[0].second;
      }
  }

  private:
    SortOrderSpecification spec;
    const size_t len;
};


class TupleCompareBy_Reverse : public TupleCompareBy
{
  public:
    TupleCompareBy_Reverse( const SortOrderSpecification& spec )
    : TupleCompareBy(spec)
    {}

    inline bool operator()( const Tuple* a,
                            const Tuple* b ) const
    {
      return !TupleCompareBy::operator()(a, b);
    }
};



// use this one to remove duplicates
class TupleCompareByAlmost
{
  public:
    TupleCompareByAlmost( const SortOrderSpecification &spec ):
      spec( spec ),
      len( spec.size() )
      {}

    inline bool operator()( const Tuple* a,
                            const Tuple* b ) const
    {
      if (len > 1)
      {

        SortOrderSpecification::const_iterator iter = spec.begin();
        while( iter != spec.end() )
        {
          const int pos = iter->first-1;
          const Attribute* aAttr = (const Attribute*) a->GetAttribute(pos);
          const Attribute* bAttr = (const Attribute*) b->GetAttribute(pos);
          const int cmpValue = aAttr->CompareAlmost( bAttr );

          if( cmpValue !=  0 )
          {
            // aAttr < bAttr ?
            return (cmpValue < 0) ? iter->second : !(iter->second);
          }
          // the current attribute is equal
          iter++;
        }
        // all attributes are equal
        return false;
      }
      else
      {
        const int pos = spec[0].first-1;
        const Attribute* aAttr = (const Attribute*) a->GetAttribute(pos);
        const Attribute* bAttr = (const Attribute*) b->GetAttribute(pos);
        return aAttr->Less(bAttr) ? spec[0].second : !spec[0].second;
      }
  }

  private:
    SortOrderSpecification spec;
    const size_t len;
};


class TupleFile;

/*
3.5 Class ~TupleFileIterator~

This class implements a sequential iterator for a ~TupleFile~.

*/

class TupleFileIterator
{
public:

  TupleFileIterator(TupleFile& f);
/*
The constructor. Opens the tuple file ~f~ for reading and
initializes a sequential scan of ~f~.

*/

  ~TupleFileIterator();
/*
The destructor. Closes the tuple file.

*/

  inline bool MoreTuples() { return ( data != 0 ); }
/*
Test if the are more tuples to read in the corresponding ~TupleFile~.

*/

  Tuple* GetNextTuple();
/*
Returns the next tuple from the corresponding ~TupleFile~. If all
tuples have been read the method returns 0;

*/

  friend class Tuple;
/*
Class ~Tuple~ already implements an internal method for
unpacking a tuple and its attributes from a memory block.
In order to reuse this code class ~Tuple~ is declared as a friend
class of ~TupleFileIterator~. ~TupleFileIterator~ simply reads
the next memory block from the ~TupleFile~ and delegates the
unpacking to class ~Tuple~.

*/

protected:

  char* ReadNextTuple(size_t& size);
/*
Returns a pointer to the current data block and its ~size~ and
prefetches the next tuple data block.

*/

  char* readData(size_t& size);
/*
Reads the next tuple data block into memory. Returns a pointer
and the ~size~ of the data block. Used to prefetch the next
data block.

*/

private:

  TupleFile& tupleFile;
/*
Reference to a tuple file.

*/
  char* data;
/*
Pointer to the current data block.

*/

  size_t size;
/*
Size of the current data block.

*/

};

/*
3.5 Class ~TupleFile~

This class implements a temporary operating system file used
to collect tuples from temporary relations.

*/

class TupleFile
{
public:

  TupleFile(TupleType* tupleType, const size_t bufferSize);
/*
First constructor. Creates a ~TupleFile~ for tuple type ~tupleType~
and sets the I/O Buffer to ~bufferSize~. The filename is created
automatically from the current working directory. The file is stored
in subfolder /tmp and starts with prefix ~TF~.

*/

  TupleFile(TupleType* tupleType, string pathName, const size_t bufferSize);
/*
Second constructor. Creates a ~TupleFile~ for tuple type ~tupleType~
with the specified filename ~pathName~ and sets the I/O Buffer
to ~bufferSize~.

*/

  ~TupleFile();
/*
The destructor. Closes and deletes the tuple file if necessary.

*/

  bool Open();
/*
Opens the tuple file for writing.

*/

  void Close();
/*
Closes the tuple file.

*/

  void Append(Tuple* t);
/*
Appends a tuple to the tuple file.

*/

  inline int GetNoTuples() const { return tupleCount; }
/*
Returns the number of tuples stored in the file.

*/

  inline size_t GetTotalExtSize() const { return totalExtSize; }
/*
Returns the total size of the extension part (including
small FLOBs) of all tuples stored in the file.

*/

  inline size_t GetTotalSize() const { return totalSize; }
/*
Returns the total size of all tuples stored in the file.

*/

  inline const string GetPathName() const { return pathName; }
/*
Returns the file path of the tuple file.

*/

  TupleFileIterator* MakeScan();
/*
Starts a sequential scan of the tuples stored in the tuple file.
Returns a pointer to a ~TupleFileIterator~.

*/

  friend class Tuple;
  friend class TupleFileIterator;
/*
Class ~Tuple~ already implements an internal method for
packing a tuple and its attributes into a memory block.
In order to reuse this code class ~Tuple~ is declared as a friend
class of ~TupleFile~. ~TupleFile~ delegates calls to Append() to
~Tuple~. ~TupleFileIterator~ gains in this way access to
the internal stream object ~stream~.

*/

protected:

  void Append(char *data, size_t core, size_t ext);
/*
Appends the tuple data block ~data~ to the end of the tuple file.
The sizes ~core~ and ~ext~ are used to calculate the total
memory block size. The total size is prepended to the written
data block. This method is called from class ~Tuple~ after
the tuple and its attributes have been packed into a
memory block.

*/

private:

  TupleType* tupleType;
/*
Tuple type of this temporary file.

*/

  string pathName;
/*
File path.

*/

  const size_t bufferSize;
/*
I/O Buffer Size for read and write operations.

*/

  FILE* stream;
/*
Stream object.

*/

  size_t tupleCount;
  size_t totalSize;
  size_t totalExtSize;
/*
Some statistics about number of tuples and tuple sizes.

*/

  static bool traceMode;
/*
Flag to control the trace mode. If set to true, some log
messages on the standard output will be produced.

*/

};


/*
3.10 Class ~GenericRelationIterator~

This abstract class forces its two son classes ~RelationIterator~
and ~TupleBufferIterator~ to implement some functions so that
both classes can be used in a generic way with polymorphism.

*/
class GenericRelationIterator
{
  public:
    virtual ~GenericRelationIterator() {};
/*
The virtual destructor.

*/
    virtual Tuple* GetNextTuple() = 0;
    virtual Tuple* GetNextTuple( const list<int>& attrList ) { return 0; }
/*
The function to retrieve the next tuple.

*/
    virtual TupleId GetTupleId() const = 0;
/*
The function to retrieve the current tuple ~id~.

*/

 virtual bool EndOfScan() const{ return false;}

};

/*
3.9 Class ~GenericRelation~

This abstract class forces its two son classes ~Relation~ and
~TupleBuffer~ to implement some functions so that both classes
can be used in a generic way with polymorphism.

*/
class GenericRelation
{
  public:
    virtual ~GenericRelation() {};
/*
The virtual destructor.

*/
    virtual int GetNoTuples() const = 0;
/*
The function to return the number of tuples.

*/
    virtual double GetTotalRootSize() const = 0;
/*
The function to return the total size of the relation
in bytes, taking into account only the root part of the
tuples.

*/
    virtual double GetTotalRootSize(int i) const = 0;
/*
The function to return the total size of and attribute
of the relation in bytes, taking into account only the
root part of the tuples.

*/
    virtual double GetTotalExtSize() const = 0;
/*
The function to return the total size of the relation
in bytes, taking into account the root part of the
tuples and the extension part, i.e. the small FLOBs.

*/
    virtual double GetTotalExtSize( int i ) const = 0;
/*
The function to return the total size of an attribute
of the relation in bytes, taking into account the root
part of the tuples and the extension part, i.e. the
small FLOBs.

*/
    virtual double GetTotalSize() const = 0;
/*
The function to return the total size of the relation
in bytes.

*/
    virtual double GetTotalSize( int i ) const = 0;
/*
The function to return the total size of an attribute
of the relation in bytes, taking into account the root
part of the tuples and the extension part, i.e. the
small FLOBs.

*/
    virtual void Clear() = 0;
/*
The function that clears the set.

*/
    virtual void AppendTuple( Tuple *t ) = 0;
/*
The function to append a tuple into the set.

*/
    virtual Tuple *GetTuple( const TupleId& id,
                             const bool dontReportError) const = 0;
/*
The function that retrieves a tuple given its ~id~.

*/
    virtual
    Tuple *GetTuple( const TupleId& id,
                     const int attrIndex,
                     const vector< pair<int, int> >& intervals,
                     const bool dontReportError ) const = 0;
/*
The function is similar to the last one, but instead of only
retrieving the tuple, it restricts the first FLOB (or DBArray)
of the attribute indexed by ~attrIndex~ to the positions given
by ~intervals~. This function is used in Double Indexing
(operator ~gettuplesdbl~).

*/
    virtual GenericRelationIterator *MakeScan() const = 0;
    virtual GenericRelationIterator *MakeScan(TupleType* tt) const { return 0; }
/*
The function to initialize a scan returning the iterator.

*/
    virtual bool GetTupleFileStats( SmiStatResultType &result ) = 0;
    virtual bool GetLOBFileStats( SmiStatResultType &result ) = 0;
/*
This two functions return statistics on the files used to store the core
tuple data, resp. LOB data of the relation.

Return value will be false, if an error occured.

*/

   virtual bool DeleteTuple(Tuple* t) = 0;
/*
  The function that deletes the given Tuple from the relation

*/
  virtual void UpdateTuple( Tuple *tuple,
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs ) = 0;


};

/*
3.10 Class ~TupleBufferIterator~

This class is an iterator for the ~TupleBuffer~ class.

*/
class TupleBuffer;
/*
Forward declaration of the ~TupleBuffer~ class.

*/

class TupleBufferIterator : public GenericRelationIterator
{
  public:
    TupleBufferIterator( const TupleBuffer& buffer );
    TupleBufferIterator( const TupleBuffer& buffer, TupleType* tt );
/*
The constructor.

*/
    ~TupleBufferIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple();
    Tuple *GetNextTuple( const list<int>& attrList );
/*
Returns the next tuple of the buffer. Returns 0 if the end of the
buffer is reached.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identification of the current tuple.

*/

  private:

  long& readData_Bytes;
  long& readData_Pages;

  const TupleBuffer& tupleBuffer;
/*
A pointer to the tuple buffer.

*/
  size_t currentTuple;
/*
The current tuple if it is in memory.

*/
  GenericRelationIterator *diskIterator;
/*
The iterator if it is not in memory.

*/
  TupleType* outtype;
/*
The type of the tuples in the TupleBuffer.
Required for the feedproject operator.

*/

};

/*
3.9 Class ~TupleBuffer~

This class is a collection of tuples in memory, if they fit.
Otherwise it acts like a relation. The size of memory used is
passed in the constructor.


*/
class Relation;

/*
Forward declaration of class ~Relation~.

*/

class TupleBuffer : public GenericRelation
{
  public:
    TupleBuffer( size_t maxMemorySize = 16 * 1024 * 1024 );
/*
The constructor. Creates an empty tuple buffer.

*/
    ~TupleBuffer();
/*
The destructor. Deletes (if allowed) all tuples.

*/

    virtual Tuple* GetTuple( const TupleId& tupleId,
                             const bool dontReportError ) const;

    virtual Tuple *GetTuple( const TupleId& id,
                     const int attrIndex,
                     const vector< pair<int, int> >& intervals,
                     const bool dontReportError ) const;

    virtual void   Clear();
    virtual int    GetNoTuples() const;
    virtual double GetTotalRootSize() const;
    virtual double GetTotalRootSize(int i) const;
    virtual double GetTotalExtSize() const;
    virtual double GetTotalExtSize( int i ) const;
    virtual double GetTotalSize() const;
    virtual double GetTotalSize( int i ) const;
    virtual void   AppendTuple( Tuple *t );
    virtual bool GetTupleFileStats( SmiStatResultType &result ) ;
    virtual bool GetLOBFileStats( SmiStatResultType &result ) ;


    virtual GenericRelationIterator *MakeScan() const;
    virtual GenericRelationIterator *MakeScan(TupleType* tt) const;

/*
inherited ~virtual~ functions.

Note: The function ~AppendTuple~ does not copy the physical representation of
FLOBS which are in state "InDiskLarge", since TupleBuffers should only be used
for intermediate result sets needed in operator implementations.

*/


    bool IsEmpty() const;
/*
Checks if the tuple buffer is empty or not.

*/
    size_t FreeBytes() const;
/*
Returns the number of Free Bytes

*/

    inline bool InMemory() const { return inMemory; }
/*
Checks if the tuple buffer is empty or not.

*/


   virtual bool DeleteTuple(Tuple* t){
      bool implemented = false;
      assert(implemented);
      return false;
   }
/*
  The function that deletes the given Tuple from the relation

*/
  virtual void UpdateTuple( Tuple *tuple,
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs ){
     bool implemented = false;
     assert(implemented);
  }


    friend class TupleBufferIterator;

  private:

  typedef vector<Tuple*> TupleVec;

  const size_t MAX_MEMORY_SIZE;
/*
The maximum size of the memory in bytes. 32 MBytes being used.

*/
  TupleVec memoryBuffer;
/*
The memory buffer which is a ~vector~ from STL.

*/
  Relation* diskBuffer;
/*
The buffer stored on disk.

*/
  bool inMemory;
/*
A flag that tells if the buffer fit in memory or not.

*/
  const bool traceFlag;
/*
Switch trace messages on or off

*/
  double totalMemSize;
/*
The total size of occupied main memory;

*/


  double totalExtSize;

/*
The total size occupied by the tuples in the buffer,
taking into account the small FLOBs.

*/
  double totalSize;
/*
The total size occupied by the tuples in the buffer,
taking into account the FLOBs.

*/
  void clearAll();

  void updateDataStatistics();


    Tuple* GetTupleAtPos( const size_t pos ) const;
    bool SetTupleAtPos( const size_t pos, Tuple* t);
/*
Retrieves or assigns a tuple to the memory buffer. If the
position is out of range or the buffer is not in state inMemory,
a null pointer or false will be returned.

For a valid position the stored tuple pointer will be returned or assigned.
Positions start at zero.

*/



};


class CircularTupleBuffer : public GenericRelation
{
  public:
    CircularTupleBuffer(bool _inMemory, TupleType* tT, unsigned int noTuples);
/*
The constructor. Creates an empty tuple buffer.

*/
    ~CircularTupleBuffer();
/*
The destructor. Deletes (if allowed) all tuples.

*/

    virtual Tuple* GetTuple( const TupleId& tupleId,
                             const bool dontReportError ) const;

    virtual Tuple *GetTuple( const TupleId& id,
                     const int attrIndex,
                     const vector< pair<int, int> >& intervals,
                     const bool dontReportError ) const;

    virtual void   Clear();
    virtual int    GetNoTuples() const;
    virtual double GetTotalRootSize() const;
    virtual double GetTotalRootSize(int i) const;
    virtual double GetTotalExtSize() const;
    virtual double GetTotalExtSize( int i ) const;
    virtual double GetTotalSize() const;
    virtual double GetTotalSize( int i ) const;
    virtual void   AppendTuple( Tuple *t );
    virtual bool GetTupleFileStats( SmiStatResultType &result ) ;
    virtual bool GetLOBFileStats( SmiStatResultType &result ) ;


    virtual GenericRelationIterator *MakeScan() const;
    virtual GenericRelationIterator *MakeScan(TupleType* tt) const;

/*
This two functions return statistics on the files used to store the core
tuple data, resp. LOB data of the relation.

Return value will be false, if an error occured.

*/

    virtual bool DeleteTuple(Tuple* t);
/*
  The function that deletes the given Tuple from the relation

*/
    virtual void UpdateTuple( Tuple *tuple,
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs );


/*
inherited ~virtual~ functions.

Note: The function ~AppendTuple~ does not copy the physical representation of
FLOBS which are in state "InDiskLarge", since TupleBuffers should only be used
for intermediate result sets needed in operator implementations.

*/

    inline bool InMemory() const { return inMemory; }
/*
Checks if the tuple buffer is empty or not.

*/


    friend class CircularTupleBufferIterator;

  private:

  bool inMemory;
/*
A flag that tells if the buffer fit in memory or not.

*/

  deque<Tuple*> memoryBuffer;
/*
The in-memory representation of the buffer. Used only if inMemory == true

*/

  Relation* diskBuffer;
  GenericRelationIterator *writeIterator;

/*
The buffer stored on disk.

*/
  bool overWrite;
/*
A flag that tells if the first write cycle is finished. This is used only when
the buffer is on disk. Initially it is false. Once it is true, all writes to
the buffer are overwriting of existing tuples.

*/
  unsigned int maxNoTuples;
/*
The capacity of the buffer.

*/

  double totalExtSize;

/*
The total size occupied by the tuples in the buffer,
taking into account the small FLOBs.

*/
  double totalSize;
/*
The total size occupied by the tuples in the buffer,
taking into account the FLOBs.

*/

  double totalMemSize;
/*
The total size of occupied main memory;

*/

  void clearMemory();

  void updateDataStatistics();


  Tuple* GetTupleAtPos( const size_t pos ) const;
  bool SetTupleAtPos( const size_t pos, Tuple* t);
/*
Retrieves or assigns a tuple to the memory buffer. If the
position is out of range or the buffer is not in state inMemory,
a null pointer or false will be returned.

For a valid position the stored tuple pointer will be returned or assigned.
Positions start at zero.

*/

};


class CircularTupleBufferIterator : public GenericRelationIterator
{
  public:
    CircularTupleBufferIterator( const CircularTupleBuffer& buffer );
    CircularTupleBufferIterator(
        const CircularTupleBuffer& buffer, TupleType* tt );
/*
The constructor.

*/
    ~CircularTupleBufferIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple();
    Tuple *GetNextTuple( const list<int>& attrList );
/*
Returns the next tuple of the buffer. Returns 0 if the end of the
buffer is reached.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identification of the current tuple.

*/

  private:

  const CircularTupleBuffer& tupleBuffer;
/*
A pointer to the tuple buffer.

*/
  deque<Tuple*>::const_iterator currentTuple;
/*
The current tuple if it is in memory.

*/
  TupleType* outtype;
/*
The type of the tuples in the TupleBuffer.
Required for the feedproject operator.

*/

};

/*
The class ~RandomTBuf~ provides a special kind of tuple buffer
which draws a random subset of N tuples for all tuples which
are given to it.

Only the selected tuples are stored there, rejected or released
tuples must be handled elsewhere

*/


class RandomTBuf
{
  public:
  RandomTBuf( size_t subsetSize = 500 );
  ~RandomTBuf() {};

  Tuple* ReplacedByRandom(Tuple* in, size_t& idx, bool& replaced);

  typedef vector<Tuple*> Storage;
  typedef Storage::iterator iterator;

  iterator begin() { return memBuf.begin(); }
  iterator end()   { return memBuf.end(); }

  void copy2TupleBuf(TupleBuffer& tb);

  private:
    size_t subsetSize;
    size_t streamPos;
    int run;
    bool trace;
    Storage memBuf;
};

/*
4 Type constructor ~rel~

4.2 Class ~RelationIterator~

This class is used for scanning (iterating through) relations.

*/


class RelationIterator : public GenericRelationIterator
{
  public:
    RelationIterator( const Relation& relation, TupleType* tt = 0 );
/*
The constructor. Creates a ~RelationIterator~ for a given ~relation~
and positions the cursor in the first tuple, if exists.

*/
    ~RelationIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple();
    Tuple* GetNextTuple( const list<int>& attrList );
/*
Retrieves the tuple in the current position of the iterator and moves
the cursor forward to the next tuple. Returns 0 if the cursor is in
the end of a relation.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identifier of the current tuple.

*/
    bool EndOfScan() const;
/*
Tells whether the cursor is in the end of a relation.

*/
  protected:
  PrefetchingIterator *iterator;
/*
The iterator.

*/
  const Relation& relation;
/*
A reference to the relation.

*/
  bool endOfScan;
/*
Stores the state of the iterator.

*/
  TupleId currentTupleId;
/*
Stores the identification of the current tuple.

*/
  TupleType* outtype;
/*
  optional output tuple type. This will be used for a scan which
  already applies a projection

*/

};


class RandomRelationIterator : protected RelationIterator
{
  public:
    RandomRelationIterator( const Relation& relation );
/*
The constructor. Creates a ~RandomRelationIterator~ for a given ~relation~
and positions the cursor in the first tuple, if exists.

*/
    ~RandomRelationIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple(int step=1);
/*
Retrieves the tuple in the current position of the iterator and moves
the cursor forward to the next tuple. Returns 0 if the cursor is in
the end of a relation.

*/
};

/*
4.1 Class ~Relation~

This class implements the memory representation of the type
constructor ~rel~.

*/


class Relation : public GenericRelation
{
  public:
    Relation( const ListExpr typeInfo, bool isTemp = false );
/*
The first constructor. It creates an empty relation from a
~typeInfo~. The flag ~isTemp~ can be used to create temporary
relations.

*/
    Relation( TupleType *tupleType, bool isTemp = false );
/*
The second constructor. It creates an empty relation from a
~tupleType~. The flag ~isTemp~ can be used to create temporary
relations.

*/
    Relation( const RelationDescriptor& relDesc);
/*
The third constructor. It opens a previously created relation.
The flag ~isTemporary~ can be used to open temporary created
relations.

*/
    ~Relation();
/*
The destructor. Deletes the memory part of an relation object.

*/

    std::ostream& Print(std::ostream& os) const;

/*
Return the type name used for a persistent relation in Secondo.
To get the type name of a temporary relation, use TempRelation::BasicType().

*/
    inline static const string BasicType() { return "rel"; }

    static const bool checkType(const ListExpr type){
      return listutils::isRelDescription(type);
    }

    static Relation *GetRelation (const SmiFileId fileId );
    SmiFileId GetFileId() { return tupleFile.GetFileId(); }
/*

Given a relation descriptor, ~GetRelation~ finds if there is an opened relation with that
descriptor and retrieves its memory representation pointer.This function is
used to avoid opening several times the same relation. A table indexed by
descriptors containing the relations is used for this purpose.

Instead of the complete descriptor the file id of the tuple file can also be used.
for this purpose. The file id can be retrieved by ~GetFileId~.

*/
    static GenericRelation *In( ListExpr typeInfo, ListExpr value,
                         int errorPos, ListExpr& errorInfo,
                         bool& correct, bool tupleBuf = false );
/*
Creates a relation from the ~typeInfo~ and ~value~ information.
Corresponds to the ~In~-function of type constructor ~rel~.

*/
    static Relation *RestoreFromList( ListExpr typeInfo,
                                      ListExpr value, int errorPos,
                                      ListExpr& errorInfo,
                                      bool& correct );
/*
Acts like the ~In~ function, but uses internal representation for
the objects. Corresponds to the ~RestoreFromList~-function of type
constructor ~rel~.

*/
    static ListExpr Out( ListExpr typeInfo, GenericRelationIterator* rit );
/*
Writes a relation into a ~ListExpr~ format.
Corresponds to the ~Out~-function of type constructor ~rel~.

*/
    ListExpr SaveToList( ListExpr typeInfo );
/*
Acts like the ~Out~ function, but uses internal representation for
the objects. Corresponds to the ~SaveToList~-function of type
constructor ~rel~.

*/
    static Relation *Open( SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo );
/*
Opens a relation.
Corresponds to the ~Open~-function of type constructor ~rel~.

*/
    bool Save( SmiRecord& valueRecord, size_t& offset,
               const ListExpr typeInfo );
/*
Saves a relation.
Corresponds to the ~Save~-function of type constructor ~rel~.

*/
    void Close();
/*
Closes a relation.
Corresponds to the ~Close~-function of type constructor ~rel~.

*/
    void Delete();
/*
Deletes a relation.
Corresponds to the ~Delete~-function of type constructor ~rel~.

*/
    void DeleteAndTruncate();
/*
Deletes a relation and truncates the corresponding record files.
This releases used disk memory.

*/
    Relation *Clone() const;
/*
Clones a relation.
Corresponds to the ~Clone~-function of type constructor ~rel~.

*/
    bool DeleteTuple( Tuple *tuple );
/*
Deletes the tuple from the relation. Returns false if the tuple
could not be deleted. The implementation of this function belongs
to the Update Relational Algebra.

*/
    void UpdateTuple( Tuple *tuple,
                      const vector<int>& changedIndices,
                      const vector<Attribute *>& newAttrs );
/*
Updates the tuple by putting the new attributes at the positions
given by ~changedIndices~ and adjusts the physical representation.
The implementation of this function belongs to the Update
Relational Algebra.

*/
    TupleType *GetTupleType() const;
/*
Returns the tuple type of the tuples of the relation.

*/

    virtual Tuple* GetTuple( const TupleId& tupleId,
                             const bool dontReportError ) const;

    virtual Tuple *GetTuple( const TupleId& id,
                     const int attrIndex,
                     const vector< pair<int, int> >& intervals,
                     const bool dontReportError ) const;

    virtual void   Clear();
    virtual int    GetNoTuples() const;
    virtual double GetTotalRootSize() const;
    virtual double GetTotalRootSize(int i) const;
    virtual double GetTotalExtSize() const;
    virtual double GetTotalExtSize( int i ) const;
    virtual double GetTotalSize() const;
    virtual double GetTotalSize( int i ) const;
    virtual void   AppendTuple( Tuple *tuple );

    virtual GenericRelationIterator *MakeScan() const;
    virtual GenericRelationIterator *MakeScan(TupleType* tt) const;
    virtual bool GetTupleFileStats( SmiStatResultType &result ) ;
    virtual bool GetLOBFileStats( SmiStatResultType &result ) ;

/*
Inherited ~virtual~ functions

*/

    void AppendTupleNoLOBs( Tuple *tuple );

/*
A special variant of ~AppendTuple~ which does not copy the LOBs.

*/

    RandomRelationIterator *MakeRandomScan() const;
/*
Returns a ~RandomRelationIterator~ which returns tuples in random order.
Currently this is not implemented, but first of all it this iterator skips
some tuples in the scan, hence it can iterate faster. It is used by the
sample operator.

*/
    //inline PrivateRelation *GetPrivateRelation() { return privateRelation; }
/*
Function to give outside access to the private part of the
relation class.

*/

    friend class RelationIterator;
    friend class RandomRelationIterator;
    ////friend struct PrivateRelationIterator;


  private:
    void InitFiles( bool open = false );
    void ErasePointer();
/*
Removes the current Relation from the table of opened relations.

*/

  RelationDescriptor relDesc;
/*
Stores the descriptor of the relation.

*/
  mutable SmiRecordFile tupleFile;
/*
Stores a handle to the tuple file.

*/


/*
The private attributes of the class ~Relation~.

*/
    static map<SmiFileId, Relation*> pointerTable;
/*
A table containing all opened relations indexed by 
fileId.

*/
};

/*
4 Auxiliary functions' interface

4.1 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is
not one of these symbols, then the value ~error~ is returned.

*/
enum RelationType { rel, trel, tuple, stream, ccmap, ccbool, error };
RelationType TypeOfRelAlgSymbol (ListExpr symbol);

/*
3.2 Function ~FindAttribute~

Here ~list~ should be a list of pairs of the form
(~name~,~datatype~). The function ~FindAttribute~ determines
whether ~attrname~ occurs as one of the names in this list. If so,
the index in the list (counting from 1) is returned and the
corresponding datatype is returned in ~attrtype~. Otherwise 0 is
returned. Used in operator ~attr~, for example.

*/
int FindAttribute( ListExpr list,
                   string attrname,
                   ListExpr& attrtype);

/*
3.3 Function ~ConcatLists~

Concatenates two lists.

*/
ListExpr ConcatLists( ListExpr list1, ListExpr list2);

/*
3.5 Function ~AttributesAreDisjoint~

Checks wether two ListExpressions are of the form
((a1 t1) ... (ai ti)) and ((b1 d1) ... (bj dj))
and wether the ai and the bi are disjoint.

*/
bool AttributesAreDisjoint(ListExpr a, ListExpr b);

/*
3.6 Function ~Concat~

Copies the attribute values of two tuples ~r~ and ~s~ into
tuple ~t~.

*/
void Concat (Tuple *r, Tuple *s, Tuple *t);

/*
3.7 Function ~CompareNames~

*/
bool CompareNames(ListExpr list);

/*

5.6 Function ~IsTupleDescription~

Checks whether a ~ListExpr~ is of the form
((a1 t1) ... (ai ti)).

*/
bool IsTupleDescription( ListExpr tupleDesc );

/*

5.6 Function ~IsRelDescription~

Checks whether a ~ListExpr~ is of the form
( {rel|trel} (tuple ((a1 t1) ... (ai ti)))).

*/
bool IsRelDescription( ListExpr relDesc, bool trel = false );

/*

5.6 Function ~IsStreamDescription~

Checks whether a ~ListExpr~ is of the form
(stream (tuple ((a1 t1) ... (ai ti)))).

*/
bool IsStreamDescription( ListExpr relDesc );

/*
5.7 Function ~GetTupleResultType~

This function returns the tuple result type as a list expression
given the Supplier ~s~.

*/
ListExpr GetTupleResultType( Supplier s );

/*
5.8 Function ~CompareSchemas~

This function takes two relations types and compare their schemas.
It returns true if they are equal, and false otherwise.

*/
bool CompareSchemas( ListExpr r1, ListExpr r2 );

/*
Namespace for temporal relation (trel), that does not have its own class.

*/
namespace TempRelation {
  const string BasicType();
}

#endif // _RELATION_ALGEBRA_H_
