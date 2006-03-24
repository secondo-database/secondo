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

It is intended to have two implementation of these classes, one 
with a persistent representation and another with a main memory 
representation. We will call these two Persistent Relation Algebra 
and Main Memory Relational Algebra, respectively. This can be seen 
in the architecture of the Relational Algebra implementation 
figure below.

                Figure 1: Relational Algebra implementation 
                architecture. [RelationAlgebraArchitecture.eps]

2 Defines, includes, and constants

*/
#ifndef _RELATION_ALGEBRA_H_
#define _RELATION_ALGEBRA_H_

#include <iostream>
#include <map>

#include "Algebra.h"
#include "StandardAttribute.h"
#include "NestedList.h"

#define MAX_NUM_OF_ATTR 10 

class CcTuple;

/*
3 Type Constructor ~tuple~

3.1 ~TupleId~

This class implements the unique identification for tuples inside a 
relation.

*/
typedef long TupleId;

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
  AttributeType()
    {}
/*
This constructor should not be used.

*/
  AttributeType( int algId, int typeId, int size ):
    algId( algId ),
    typeId( typeId ),
    size( size )
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
  int size;
/*
Size of attribute instance in bytes.

*/
};

/*
3.4 Class ~TupleType~

A ~TupleType~ is a collection (an array) of all attribute types 
(~AttributeType~) of the tuple. This structure contains the metadata of a tuple attributes.

*/
class TupleType
{
  public:
    TupleType( const ListExpr typeInfo );
/*
The first constructor. Creates a tuple type from a ~typeInfo~ list 
expression. It sets all member variables, including the total size.

*/

    ~TupleType()
    {
      delete []attrTypeArray;
    }
/*
The destructor.

*/
    inline void DeleteIfAllowed() 
    {
      assert( refs > 0 );
      refs--;
      if( refs == 0 )
        delete this;
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
};

/*
3.5 Class ~Tuple~

This class implements the memory representation of the type 
constructor ~tuple~.

*/

#ifdef RELALG_PERSISTENT 
#include "RelationPersistent.h"
#else
#include "RelationMainMemory.h"
#endif
/*
Declaration of the struct ~PrivateTuple~. This struct contains the
private attributes of the class ~Tuple~ and is defined differently
for the Main Memory Relational Algebra (RelationMainMemory.h)
and for the Persistent Relational Algebra (RelationPersistent.h).

*/

class Tuple
{
  public:

    inline Tuple( TupleType* tupleType ):
    privateTuple( new PrivateTuple( tupleType ) )
    {
      Init( tupleType->GetNoAttributes(), privateTuple );
    }
/*
The constructor. It contructs a tuple with the metadata passed in 
the ~tupleType~ as argument.

*/
    inline Tuple( const ListExpr typeInfo ):
    privateTuple( new PrivateTuple( typeInfo ) )
    {
      Init( privateTuple->tupleType->GetNoAttributes(), privateTuple);
    }
/*
A similar constructor as the above, but taking a list 
expression ~typeInfo~ as argument.

*/
    inline ~Tuple()
    {
      tuplesDeleted++;
      tuplesInMemory--;
      delete privateTuple;
      if (noAttributes > MAX_NUM_OF_ATTR)
        delete [] attributes;
    }
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
    static Tuple *RestoreFromList( const ListExpr typeInfo, ListExpr value,
                                   int errorPos, ListExpr& errorInfo,
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
    static void SetCounterReport( bool val );
/*
Shows tuple statistics if invoekd with ~true~ 

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
      return (Attribute *)attributes[index];
    }
/*
Returns the attribute at position ~index~ inside the tuple.

*/
    void PutAttribute( int index, Attribute* attr );
/*
Puts an attribute in the position ~index~ inside the tuple.

*/
    void UpdateAttributes( const vector<int>& changedIndices,
                           const vector<Attribute*>& newAttrs );
/*
Puts the attributes from ~newAttrs~ at the corresponding position 
from ~changedIndices~ into the tuple. Destroys the physical 
representations of the old attributes and saves the new tuple to 
disk. The implementation of this function is found in the
Update Relation Algebra.

*/
    inline int GetRootSize() const
    {
      return privateTuple->tupleType->GetTotalSize();
    }
/*
Returns the size of the tuple's root part.

*/
    inline int GetRootSize( int i ) const
    {
      return privateTuple->tupleType->GetAttributeType(i).size;
    }
/*
Returns the size of the tuple's root part.

*/

    inline int GetExtSize() const
    {
      if ( !recomputeExtSize ) 
        return tupleExtSize;

      tupleExtSize = privateTuple->tupleType->GetTotalSize();
      for( int i = 0; 
           i < privateTuple->tupleType->GetNoAttributes(); i++)
      {
        for( int j = 0; 
             j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
        {
          FLOB *tmpFLOB = privateTuple->attributes[i]->GetFLOB(j);
          if( !tmpFLOB->IsLob() )
            tupleExtSize += tmpFLOB->Size();
        }
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
      tupleExtSize = GetRootSize( i );
      for( int j = 0;
           j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
      {
        FLOB *tmpFLOB = privateTuple->attributes[i]->GetFLOB(j);
        if( !tmpFLOB->IsLob() )
          tupleExtSize += tmpFLOB->Size();
      }
      return tupleExtSize;
    }
/*
Returns the size of an attribute of the tuple taking into account 
the extension part, i.e. the small FLOBs.

*/

    inline int GetSize() const
    {
      if ( !recomputeSize ) 
        return tupleSize;

      tupleSize = privateTuple->tupleType->GetTotalSize();
      for( int i = 0; 
           i < privateTuple->tupleType->GetNoAttributes(); i++)
      {
        for( int j = 0; 
             j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
          tupleSize += 
            privateTuple->attributes[i]->GetFLOB(j)->Size();
      }
      recomputeSize = false;
      return tupleSize;
    }
/*
Returns the total size of the tuple taking into account  
the FLOBs.

*/
    inline int GetSize( int i ) const
    {
      tupleSize = GetRootSize(i);
      for( int j = 0;
           j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
        tupleSize +=
          privateTuple->attributes[i]->GetFLOB(j)->Size();
      return tupleSize;
    }
/*
Returns the total size of an attribute of the tuple taking 
into account the FLOBs.

*/

    inline void CopyAttribute( int sourceIndex, 
                               const Tuple *source, 
                               int destIndex )
    {
      privateTuple->CopyAttribute( sourceIndex, 
                                   source->privateTuple, 
                                   destIndex );
    }
/*
This function is used to copy attributes from tuples to tuples 
without cloning attributes.

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
      return privateTuple->tupleType;
    }
/*
Returns the tuple type.

*/
    Tuple *Clone() const;
/*
Create a new tuple which is a clone of this tuple.

*/
    CcTuple* CloneToMemoryTuple() const;
/*
Creates a new memory tuple which is a clone of this tuple.

*/
    inline void DeleteIfAllowed() 
    {
      if( refs == 0 )
        delete this;
    }
/*
Deletes the tuple if it is allowed, i.e., there are no references
(attribute ~refs~) to it anymore.

*/
    inline void IncReference()
    {
      refs++;
    }
/*
Increses the reference count of this tuple.

*/
    inline void DecReference()
    {
      refs--;
    }
/*
Increses the reference count of this tuple.

*/
    inline PrivateTuple *GetPrivateTuple()
    { 
      return privateTuple; 
    }
/*
Function to give outside access to the private part of the tuple 
class.

*/

  private:

    static long& tuplesCreated;
    static long& tuplesDeleted;
    static long& tuplesInMemory;
    static long& maximumTuples;
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
    inline void Init( int NoAttr, PrivateTuple* pt )
    {
      recomputeExtSize = true;
      recomputeSize = true;

      noAttributes = NoAttr;
      
      if ( noAttributes > MAX_NUM_OF_ATTR ) 
        attributes = new Attribute*[noAttributes];
      else 
        attributes = defAttributes;

      refs = 0;

      pt->attributes = attributes;
      InitAttrArray();

      tupleExtSize = 0;
      tupleSize = 0;

      tuplesCreated++;
      tuplesInMemory++;
      if( tuplesInMemory > maximumTuples ) 
        maximumTuples = tuplesInMemory;
    }
/*
Initializes a tuple.

*/

    int refs;
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

    mutable int tupleExtSize;
/*
Stores the size of the tuples taking into account the extension
part of the tuple, i.e. the small FLOBs.

*/
    mutable int tupleSize;
/*
Stores the total size of the tuples taking into account the 
FLOBs.

*/
    int noAttributes;
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
    mutable PrivateTuple *privateTuple;
/*
The private implementation dependent attributes of the class 
~Tuple~.

*/
};

ostream& operator <<( ostream& o, Tuple& t );
/*
The print function for tuples. Used for debugging purposes

*/

/*
3.7 Class ~LexicographicalTupleCompare~

This is a class used in the sort algorithm that specifies the 
lexicographical comparison function between two tuples.

*/
class LexicographicalTupleCompare
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

/*
3.8 Class ~TupleCompareBy~

This is a class used in the sort algorithm that specifies the 
comparison function between two tuples using a set of attributes 
specified in ~SortOrderSpecification~, which is a vector of pairs 
containing the index of the attribute and a boolean flag telling 
whether the ordering is ascendant or not (descendant).

*/
typedef vector< pair<int, bool> > SortOrderSpecification;

class TupleCompareBy
{
  public:
    TupleCompareBy( const SortOrderSpecification &spec ):
      spec( spec )
      {}

    inline bool operator()( const Tuple* aConst, 
                            const Tuple* bConst ) const
    {
      Tuple* a = (Tuple*)aConst;
      Tuple* b = (Tuple*)bConst;

      SortOrderSpecification::const_iterator iter = spec.begin();
      while( iter != spec.end() )
      {
        int pos = iter->first-1;
        Attribute* aAttr = (Attribute*) a->GetAttribute(pos);
        Attribute* bAttr = (Attribute*) b->GetAttribute(pos);
        int cmpValue = aAttr->Compare( bAttr );

        if( cmpValue < 0 ) // aAttr < bAttr ?
          return iter->second;
        if( cmpValue > 0) // aAttr > bAttr ?
          return !(iter->second);
        
        // the current attribute is equal
        iter++;
      }
      // all attributes are equal  
      return false;
  }

  private:
    SortOrderSpecification spec;
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
    virtual Tuple *GetNextTuple() = 0;
/*
The function to retrieve the next tuple.

*/
    virtual TupleId GetTupleId() const = 0;
/*
The function to retrieve the current tuple ~id~.

*/
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
    virtual double GetTotalExtSize() const = 0;
/*
The function to return the total size of the relation
in bytes, taking into account the root part of the 
tuples and the extension part, i.e. the small FLOBs. 

*/
    virtual double GetTotalSize() const = 0;
/*
The function to return the total size of the relation
in bytes.

*/
    virtual void Clear() = 0;
/*
The function that clears the set.

*/
    virtual void AppendTuple( Tuple *t ) = 0;
/*
The function to append a tuple into the set.

*/
    virtual Tuple *GetTuple( const TupleId& id ) const = 0;
/*
The function that retrieves a tuple given its ~id~.

*/
    virtual GenericRelationIterator *MakeScan() const = 0;
/*
The function to initialize a scan returning the iterator.

*/
};

/*
3.10 Class ~TupleBufferIterator~

This class is an iterator for the ~TupleBuffer~ class.

*/
class TupleBuffer;
/*
Forward declaration of the ~TupleBuffer~ class.

*/

struct PrivateTupleBufferIterator;
/*
Forward declaration of the struct ~PrivateTupleBufferIterator~. 
This struct will contain the private attributes of the class 
~TupleBufferIterator~ and will be defined later differently for 
the Main Memory Relational Algebra and for the Persistent Relational
Algebra.

*/
class TupleBufferIterator : public GenericRelationIterator
{
  public:
    TupleBufferIterator( const TupleBuffer& buffer );
/*
The constructor.

*/
    ~TupleBufferIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple();
/*
Returns the next tuple of the buffer. Returns 0 if the end of the 
buffer is reached.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identification of the current tuple.

*/

  private:

    PrivateTupleBufferIterator *privateTupleBufferIterator;
/*
The reference to the private attributes of this class.

*/
};

/*
3.9 Class ~TupleBuffer~

This class is a collection of tuples in memory, if they fit. 
Otherwise it acts like a relation. The size of memory used is
passed in the constructor.

This concept is only interesting in the Persistent Relational
Algebra, where a relation is stored on disk. In the Main Memory
Relation Algebra it acts like a relation anyway and the maximum
memory utilization is ignored.

*/
struct PrivateTupleBuffer;
/*
Forward declaration of the struct ~PrivateTupleBuffer~. This struct 
will contain the private attributes of the class ~TupleBuffer~ and 
will be defined later differently for the Main Memory Relational 
Algebra and for the Persistent Relational Algebra.

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
    int GetNoTuples() const;
/*
Returns the number of tuples in the buffer.

*/
    double GetTotalRootSize() const;
/*
The function to return the total size of the buffer
in bytes, taking into account only the root part of the
tuples.

*/
    double GetTotalExtSize() const;
/*
The function to return the total size of the buffer
in bytes, taking into account the root part of the
tuples and the extension part, i.e. the small FLOBs.

*/
    double GetTotalSize() const;
/*
The function to return the total size of the buffer
in bytes.

*/
    bool IsEmpty() const;
/*
Checks if the tuple buffer is empty or not.

*/
    void Clear();
/*
Deletes (if allowed) all tuples and also clears the buffer.

*/
    void AppendTuple( Tuple *t );
/*
Appends a tuple to the buffer. Returns the size in bytes occupied 
by the tuple.

*/
    Tuple* GetTuple( const TupleId& tupleId ) const;
/*
Returns the tuple identified by ~tupleId~.

*/
    TupleBufferIterator *MakeScan() const;
/*
Returns a ~TupleBufferIterator~ for a new scan.

*/

    friend class TupleBufferIterator;
    friend struct PrivateTupleBufferIterator;

  private:

    PrivateTupleBuffer *privateTupleBuffer;
/*
A reference to the private attributes.

*/
};

/*
4 Type constructor ~rel~

4.2 Class ~RelationIterator~

This class is used for scanning (iterating through) relations.

*/

class Relation;
/*
Forward declaration of class ~Relation~.

*/

struct PrivateRelationIterator;
/*
Forward declaration of the struct ~PrivateRelationIterator~. This 
struct will contain the private attributes of the class 
~RelationIterator~ and will be defined later differently
for the Main Memory Relational Algebra and for the Persistent 
Relational Algebra.

*/

class RelationIterator : public GenericRelationIterator
{
  public:
    RelationIterator( const Relation& relation );
/*
The constructor. Creates a ~RelationIterator~ for a given ~relation~
and positions the cursor in the first tuple, if exists.

*/
    ~RelationIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple();
/*
Retrieves the tuple in the current position of the iterator and moves
the cursor forward to the next tuple. Returns 0 if the cursor is in 
the end of a relation.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identifier of the current tuple.

*/
    bool EndOfScan();
/*
Tells whether the cursor is in the end of a relation.

*/
  private:
    PrivateRelationIterator *privateRelationIterator;
/*
The private attributes of the class ~RelationIterator~.

*/
};

/*
4.1 Class ~Relation~

This class implements the memory representation of the type 
constructor ~rel~.

*/

struct RelationDescriptor;
class RelationDescriptorCompare;
/*
Forward declaration of the struct ~RelationDescriptor~ and the 
comparison class ~RelationDescriptorCompare~. These classes will 
contain the necessary information for opening a relation.

*/

struct PrivateRelation;
/*
Forward declaration of the struct ~PrivateRelation~. This struct 
will contain the private attributes of the class ~Relation~ and will 
be defined later differently for the Main Memory Relational Algebra 
and for the Persistent Relational Algebra.

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
    Relation( const RelationDescriptor& relDesc, 
              bool isTemp = false );
/*
The third constructor. It opens a previously created relation. 
The flag ~isTemporary~ can be used to open temporary created 
relations.

*/
    ~Relation();
/*
The destructor.

*/
    static Relation *GetRelation( const RelationDescriptor& d );
/*
Given a relation descriptor, finds if there is an opened relation 
with that descriptor and retrieves its memory representation pointer.This function is used to avoid opening several times the same 
relation. A table indexed by descriptors containing the relations
is used for this purpose. 

*/
    static Relation *In( ListExpr typeInfo, ListExpr value, 
                         int errorPos, ListExpr& errorInfo, 
                         bool& correct );
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
    ListExpr Out( ListExpr typeInfo );
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
    Relation *Clone();
/*
Clones a relation.
Corresponds to the ~Clone~-function of type constructor ~rel~.

*/
    void AppendTuple( Tuple *tuple );
/*
Appends a tuple to the relation. Returns the size in bytes occupied 
by the tuple.

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
    Tuple* GetTuple( const TupleId& tupleId ) const;
/*
Returns the tuple identified by ~tupleId~.

*/
    TupleType *GetTupleType() const;
/*
Returns the tuple type of the tuples of the relation.

*/
    void Clear();
/*
Clears (empties) a relation removing all its tuples.

*/
    int GetNoTuples() const;
/*
Gets the number of tuples in the relation.

*/
    double GetTotalRootSize() const;
/*
The function to return the total size of the relation
in bytes, taking into account only the root part of the
tuples.

*/
    double GetTotalRootSize(int i) const;
/*
The function to return the total size of and attribute 
of the relation in bytes, taking into account only the 
root part of the tuples.

*/
    double GetTotalExtSize() const;
/*
The function to return the total size of the relation
in bytes, taking into account the root part of the
tuples and the extension part, i.e. the small FLOBs.

*/
    double GetTotalExtSize( int i ) const;
/*
The function to return the total size of an attribute 
of the relation in bytes, taking into account the root 
part of the tuples and the extension part, i.e. the 
small FLOBs.

*/
    double GetTotalSize() const;
/*
The function to return the total size of the relation
in bytes.

*/
    double GetTotalSize( int i ) const;
/*
The function to return the total size of an attribute 
of the relation in bytes.

*/

    RelationIterator *MakeScan() const;
/*
Returns a ~RelationIterator~ for a relation scan.

*/
    inline PrivateRelation *GetPrivateRelation()
    { 
      return privateRelation; 
    }
/*
Function to give outside access to the private part of the 
relation class.

*/

    friend class RelationIterator;
    friend struct PrivateRelationIterator;

  private:

    PrivateRelation *privateRelation;
/*
The private attributes of the class ~Relation~.

*/
    static map<RelationDescriptor, Relation*, 
               RelationDescriptorCompare> pointerTable;
/*
A table containing all opened relations indexed by relation 
descriptors.

*/
};

/*
4 Auxiliary functions' interface

4.1 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is 
not one of these symbols, then the value ~error~ is returned.

*/
enum RelationType { rel, tuple, stream, ccmap, ccbool, error };
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
(rel (tuple ((a1 t1) ... (ai ti)))).

*/
bool IsRelDescription( ListExpr relDesc );

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

#endif // _RELATION_ALGEBRA_H_
