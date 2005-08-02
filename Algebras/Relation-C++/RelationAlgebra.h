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

March 2003 Victor Almeida created the new Relational Algebra organization

Oct 2004 M. Spiekermann changed some frequently called ~small~ functions into
inline functions implemented in the header file. This reduced code redundance
since the same code written in RelationMainMemory and RelationPersistent can be
kept together here and may improve performance when the code is compiled with
optimization flags.

June-July 2004 M. Spiekermann. Changes in class ~Tuple~ and ~TupleBuffer~. Storing the
attribute array as member variable in class Tuple reduces processing overhead. Moreover the
array is needed for both implementations "persistent" and "memory" hence it should not be
maintained in class ~privateTuple~. The TupleBuffer was extended. The TupleBuffer constructor
was extended by a new boolean parameter which indicates whether the tuples in it are stored as
"free" or "non-free" tuples. 

[TOC]


1 Overview

The Relational Algebra basically implements two type constructors, namely ~tuple~ and ~rel~.
The type system of the Relational Algebra can be seen below.

\begin{displaymath}
\begin{array}{lll}
& \to \textrm{DATA} & {\underline{\smash{\mathit{int}}}}, {\underline{\smash{\mathit{real}}}},
	{\underline{\smash{\mathit{bool}}}}, {\underline{\smash{\mathit{string}}}} \\
({\underline{\smash{\mathit{ident}}}} \times \textrm{DATA})^{+} & \to \textrm{TUPLE} &
	{\underline{\smash{\mathit{tuple}}}} \\
\textrm{TUPLE} & \to \textrm{REL} & {\underline{\smash{\mathit{rel}}}}
\end{array}
\end{displaymath}

The DATA kind should be incremented with more complex data types such as, for example,
${\underline{\smash{\mathit{point}}}}$, ${\underline{\smash{\mathit{points}}}}$,
${\underline{\smash{\mathit{line}}}}$, and ${\underline{\smash{\mathit{region}}}}$
of the ROSE Algebra.

As an example, a relation cities should be described as

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

This file will contain an interface of the memory representation structures (~classes~) for these
two type constructors, namely ~Tuple~ and ~Relation~, and some additional ones that will be needed
for the Relational Algebra, namely ~TupleId~, ~RelationIterator~, ~TupleType~, ~Attribute~
(which is defined inside the file Attribute.h), ~AttributeType~, and ~RelationDescriptor~.

It is intended to have two implementation of these classes, one with a persistent representation and
another with a main memory representation. We will call these two Persistent Relation Algebra and
Main Memory Relational Algebra, respectively. This can be seen in the architecture of the Relational
Algebra implementation figure below.

                Figure 1: Relational Algebra implementation architecture. [RelationAlgebraArchitecture.eps]

2 Defines, includes, and constants

*/
#ifndef _RELATION_ALGEBRA_H_
#define _RELATION_ALGEBRA_H_

#include <iostream>
#include <map>

#include "Algebra.h"
#include "StandardAttribute.h"
#include "NestedList.h"

#define MAX_NUM_OF_ATTR 20

class CcTuple;

/*
3 Type Constructor ~tuple~

3.1 Struct ~TupleId~

This class will implement the unique identification for tuples inside a relation.

*/
typedef long TupleId;

/*
3.2 Class ~Attribute~

This abstract class ~Attribute~ is inside the file Attribute.h and contains a set
of functions necessary to the management of attributes. All type constructors of the
kind DATA must be a sub-class of ~Attribute~.

3.3 Struct ~AttributeType~

This ~AttributeType~ struct implements the type of each attribute inside a tuple.
To identify a data type in the Secondo system the ~algebraId~ and the ~typeId~
are necessary. The size of the attribute is also necessary to previously know
how much space will be necessary to store an instance of such attribute's data type.

*/
struct AttributeType
{
  AttributeType()
    {}
/*
This constructor should not be used.

*/
  AttributeType( const int algId, const int typeId, const int size ):
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

A ~TupleType~ is a collection (an array) of all attribute types (~AttributeType~)
of the tuple. This structure contains the metadata of a tuple attributes.

*/
class TupleType
{
  public:
    TupleType( const ListExpr typeInfo );
/*
The first constructor. Creates a tuple type from a ~typeInfo~ list expression. It sets
all member variables, including the total size.

*/
    TupleType( const TupleType& tupleType );
/*
The second constructor. Creates a tuple type which is a copy of ~tupleType~.

*/
    ~TupleType();
/*
The destructor.

*/
    inline const int GetNoAttributes() const { return noAttributes; }
/*
Returns the number of attributes of the tuple type.

*/
    inline const int GetTotalSize() const { return totalSize; }
/*
Returns the total size of the tuple.

*/
    inline const AttributeType& GetAttributeType( const int index ) const
    {
      assert( index >= 0 && index < noAttributes );
      return attrTypeArray[index];
    }
/*
Returns the attribute type at ~index~ position.

*/
    inline void PutAttributeType( const int index, const AttributeType& attrType )
    {
      assert( index >= 0 && index < noAttributes );
      attrTypeArray[index] = attrType;
    }
/*
Puts the attribute type ~attrType~ in the position ~index~.

*/
  private:

    TupleType( const int noAttr, AttributeType *attrs );
/*
A private constructor that receives directly the array of tuple attributes. It
needs to set the ~totalSize~ attribute.

*/
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

};

/*
3.5 Class ~Tuple~

This class implements the memory representation of the type constructor ~tuple~.

*/

#ifdef RELALG_PERSISTENT 
#include "RelationPersistent.h"
#else
#include "RelationMainMemory.h"
#endif

/*
Declaration of the struct ~PrivateTuple~. This struct contains the
private attributes of the class ~Tuple~ and is defined differently
for the Main Memory Relational Algebra and for the Persistent Relational Algebra.

*/

class Tuple
{
  public:

    Tuple( const TupleType& tupleType, const bool isFree = false );
/*
The constructor. It contructs a tuple with the metadata passed in the ~tupleType~
as argument.

*/
    Tuple( const ListExpr typeInfo, const bool isFree = false );
/*
A similar constructor as the above, unless that it takes a list expression ~typeInfo~
and first convert it to a ~TupleType~.

*/
    static Tuple *In( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct );
/*
Creates a tuple from the ~typeInfo~ and ~value~ information.
Corresponds to the ~In~-function of type constructor ~tuple~.

*/
    static Tuple *RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct );
/*
Acts as the ~In~ function, but uses internal representation for the objects.

*/
    ListExpr Out( ListExpr typeInfo );
/*
Writes a tuple into a ~ListExpr~ format.
Corresponds to the ~Out~-function of type constructor ~tuple~.

*/
    ListExpr SaveToList( ListExpr typeInfo );
/*
Acts as the ~Out~ function, but uses internal representation for the objects.

*/
    ~Tuple();
/*
The destructor.

*/
    static void ShowTupleStatistics( const bool reset = false );
/*
Shows tuple statistics. If ~reset~ is set to true, the values of the tuple statistics are
set to zero.

*/
    const TupleId& GetTupleId() const;
/*
Returns the unique ~id~ of the tuple.

*/
    void SetTupleId( const TupleId& tupleId );
/*
Sets the tuple unique ~id~ of the tuple. This function is necessary because at the
construction time, the tuple does not know its id.

*/
    inline Attribute* GetAttribute( const int index ) const 
    {
      return (Attribute *)attributes[index];
    }
/*
Returns the attribute at position ~index~ inside the tuple.

*/
    void PutAttribute( const int index, Attribute* attr );
/*
Puts an attribute in the position ~index~ inside the tuple.

*/

    void UpdateAttributes( const vector<int>& changedIndices,const vector<Attribute*>& newAttrs );
/*
Puts the attributes from 'newAttrs' at the corresponding position from 'changedIndices' into the tuple. Destroys the
physical representations of the old attributes and saves the new tuple to disk.

*/
    const int GetMemorySize();
/*
Returns the size of the memory (in bytes) used by the tuple.

*/
    const int GetTotalSize();
/*
Returns the total size of the tuple taking into consideration the tuple and the
LOBs.

*/
    inline const int GetNoAttributes() const 
    {
      return noAttributes;
    }
/*
Returns the number of attributes of the tuple.

*/
    inline const TupleType& GetTupleType() const 
    {
#ifdef RELALG_PERSISTENT 
      return privateTuple->tupleType;
#else
      return *(privateTuple->tupleType);
#endif
    }

    inline void SetTupleType(const TupleType& type)
    {
      privateTuple->tupleType = type;
      noAttributes = type.GetNoAttributes();
    }


/*
Returns the tuple type.

*/
    inline const bool IsFree() const 
    {
      return isFree;
    }
/*
Returns if a tuple is free.
*Need some more explanations about why it is used.*

*/
    inline void SetFree( const bool onoff ) 
    {
      if (!protectState)
        isFree = onoff;
    }
/*
Turns the tuple free (or not) for deletion.

*/
    Tuple *Clone( const bool isFree = true ) const;
/*
Create a new tuple which is a clone of this tuple.

*/
    CcTuple* CloneToMemoryTuple( const bool isFree = true ) const;
/*
Creates a new memory tuple which is a clone of this tuple.

*/
    inline Tuple *CloneIfNecessary() 
    {
      if( isFree )
        return this;
      else
        return this->Clone( false );
    }
/*
Calls the ~Clone~ function if it is a free tuple.

*/
    inline void DeleteIfAllowed() 
    {
      if( isFree && !keepInMemory )
        delete this;
    }

/*
The function below will protect the tuple for deletion. If called
with value ~true~ the function ~DeleteIfAllowed~ will never delete the
tuple.


*/
    inline void KeepInMemory( const bool onoff )
    {
      keepInMemory = onoff; 
    }

/*
If ProtectState(true) was called  SetFree will have no effect.

*/
    inline void ProtectState( const bool onoff )
    {
      protectState = onoff;
    }

/*
Deletes the tuple if it is allowed, i.e., a free tuple.

*/
    inline PrivateTuple *GetPrivateTuple()
    { 
      return privateTuple; 
    }

/*
Function to give outside access to the private part of the tuple class.

*/

  private:
    static long tuplesCreated;
    static long tuplesDeleted;
    static long tuplesInMemory;
    static long maximumTuples;

    inline void InitAttrArray()
    {
      for( int i = 0; i < noAttributes; i++ )
        attributes[i] = 0;
    }

    inline void Init(bool IsFree, int NoAttr, PrivateTuple* pt)
    {
      isFree = IsFree;
      keepInMemory = false;
      protectState = false;
      recomputeTotalSize = true;
      recomputeMemSize = true;

      noAttributes = NoAttr;
      pt->attributes = &attributes[0];

      InitAttrArray();

      tupleMemSize = 0;
      tupleTotalSize = 0;

      tuplesCreated++;
      tuplesInMemory++;
      if( tuplesInMemory > maximumTuples ) {
	maximumTuples = tuplesInMemory;
      }
    }


/*
Variables used for tuple statistics.

*/

    bool isFree;
    bool keepInMemory;
    bool protectState;
    bool recomputeMemSize;
    bool recomputeTotalSize;

    int noAttributes;
    int tupleMemSize;
    int tupleTotalSize;

    TupleElement* attributes[MAX_NUM_OF_ATTR];

/*
Two flags that tells if a tuple is free for deletion. If a tuple is free, then a stream receiving
the tuple can delete or reuse it. By default ~deleteAllowed~ is true, but in some situations this
is not useful, e.g. if you want to use a TupleBuffer as input for a function several times, hence
we can switch off the ~normal~ deletion procedure.

*/

    PrivateTuple *privateTuple;
/*
The private implementation dependent attributes of the class ~Tuple~.

*/
};

ostream& operator <<( ostream& o, Tuple& t );
/*
The print function for tuples. Used for debugging purposes

*/

/*
3.7 Class ~LexicographicalTupleCompare~

This is a class used in the sort algorithm that specify the lexicographical
comparison function between two tuples.

*/

class LexicographicalTupleCompare
{
  public:
    inline bool operator()(const Tuple* aConst, const Tuple* bConst) const
    {
      Tuple* a = (Tuple*)aConst;
      Tuple* b = (Tuple*)bConst;

      for(int i = 0; i < a->GetNoAttributes(); i++)
      {
        if(((Attribute*)a->GetAttribute(i))->Compare(((Attribute*)b->GetAttribute(i))) < 0)
        {
          return true;
        }
        else
        {
          if(((Attribute*)a->GetAttribute(i))->Compare(((Attribute*)b->GetAttribute(i))) > 0)
          {
            return false;
          }
        }
      }
      return false;
    }
};

/*
3.8 Class ~TupleCompareBy~

This is a class used in the sort algorithm that specify the comparison
function between two tuples using a set of attributes specified in
~SortOrderSpecification~, which is a vector of pairs containing the
index of the attribute and a bool flag telling if the ordering is
ascendant or not (descendant).

*/
typedef vector< pair<int, bool> > SortOrderSpecification;

class TupleCompareBy
{
  public:
    TupleCompareBy( const SortOrderSpecification &spec ):
      spec( spec )
      {}

    inline bool operator()(const Tuple* aConst, const Tuple* bConst) const
    {
      Tuple* a = (Tuple*)aConst;
      Tuple* b = (Tuple*)bConst;

      SortOrderSpecification::const_iterator iter = spec.begin();
      while(iter != spec.end())
      {
        const int pos = iter->first-1;
        Attribute* aAttr = (Attribute*) a->GetAttribute(pos);
        Attribute* bAttr = (Attribute*) b->GetAttribute(pos);
        int cmpValue = aAttr->Compare( bAttr );

        if(  cmpValue < 0 ) // aAttr < bAttr ?
        {
          return iter->second;
        }
        else
        {
          if( cmpValue > 0) // aAttr > bAttr ?
          {
            return !(iter->second);
          }
        }
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

*/
class GenericRelationIterator
{
  public:
    virtual ~GenericRelationIterator() {};
    virtual Tuple *GetNextTuple() = 0;
    virtual TupleId GetTupleId() const = 0;
};

/*
3.9 Class ~GenericRelation~

*/
class GenericRelation
{
  public:
    virtual ~GenericRelation() {};
    virtual const int GetNoTuples() const = 0;
    virtual const double GetTotalSize() const = 0;
    virtual void Clear() = 0;
    virtual void AppendTuple( Tuple *t ) = 0;
    //virtual bool DeleteTuple( Tuple *t ) = 0;
    virtual Tuple *GetTuple( const TupleId& id ) const = 0;
    virtual GenericRelationIterator *MakeScan() const = 0;
};

/*
3.10 Class ~TupleBufferIterator~

This class is an iterator for the ~TupleBuffer~ class.

*/
class TupleBuffer;
/*
Forward declaration.

*/

struct PrivateTupleBufferIterator;
/*
Forward declaration of the struct ~PrivateTupleBufferIterator~. This struct will contain
the private attributes of the class ~TupleBufferIterator~ and will be defined later
differently for the Main Memory Relational Algebra and for the Persistent Relational
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
Returns the next tuple of the buffer. Returns 0 if the end of the buffer is reached.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identification of the current tuple.

*/

  private:
    PrivateTupleBufferIterator *privateTupleBufferIterator;
    bool isFreeStorageType;
};

/*
3.9 Class ~TupleBuffer~

This class is used to collect tuples for sorting, for example, or
to do a cartesian product.

*/
struct PrivateTupleBuffer;
/*
Forward declaration of the struct ~PrivateTupleBuffer~. This struct will contain the
private attributes of the class ~TupleBuffer~ and will be defined later differently
for the Main Memory Relational Algebra and for the Persistent Relational Algebra.

*/
class TupleBuffer : public GenericRelation
{
  public:
    TupleBuffer( const size_t maxMemorySize = 4 * 1024 * 1024, bool isFree = false );
/*
The constructor. Creates an empty tuple buffer.

*/
    ~TupleBuffer();
/*
The destructor. It is not the intention of the buffer to delete
tuples.

*/
    const int GetNoTuples() const;
/*
Returns the number of tuples in the buffer.

*/
    const double GetTotalSize() const;
/*
Returns the total size of the relation in bytes.

*/
    const bool IsEmpty() const;
/*
Checks if the tuple buffer is empty or not.

*/
    void Clear();
/*
Deletes (if allowed) all tuples and also clears the buffer.

*/
    void AppendTuple( Tuple *t );
/*
Appends a tuple to the buffer. Returns the size in bytes occupied by the tuple.

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
    bool isFreeStorageType;
};

/*
4 Type constructor ~rel~

4.2 Class ~RelationIterator~

This class is used for scanning (iterating through) relations.

*/

class Relation;
/*
Forward declaration.

*/

struct PrivateRelationIterator;
/*
Forward declaration of the struct ~PrivateRelationIterator~. This struct will contain the
private attributes of the class ~RelationIterator~ and will be defined later differently
for the Main Memory Relational Algebra and for the Persistent Relational Algebra.

*/

class RelationIterator : public GenericRelationIterator
{
  public:
    RelationIterator( const Relation& relation );
/*
The constructor. Creates a ~RelationIterator~ for a given ~relation~ and positions the
cursor in the first tuple, if exists.

*/
    ~RelationIterator();
/*
The destructor.

*/
    Tuple *GetNextTuple();
/*
Retrieves the tuple in the current position of the iterator and moves the cursor forward
to the next tuple. Returns NULL if the cursor is in the end of a relation.

*/
    TupleId GetTupleId() const;
/*
Returns the tuple identifier of the current tuple.

*/
    const bool EndOfScan();
/*
Tells if the cursor is in the end of a relation.

*/
  private:
    PrivateRelationIterator *privateRelationIterator;
/*
The private attributes of the class ~RelationIterator~.

*/
};

/*
4.1 Class ~Relation~

This class implements the memory representation of the type constructor ~rel~.

*/

struct RelationDescriptor;
class RelationDescriptorCompare;
/*
Forward declaration of the struct ~RelationDescriptor~ and the comparison class
~RelationDescriptorCompare~. These classes will contain the necessary information 
for opening a relation.

*/

struct PrivateRelation;
/*
Forward declaration of the struct ~PrivateRelation~. This struct will contain the
private attributes of the class ~Relation~ and will be defined later differently
for the Main Memory Relational Algebra and for the Persistent Relational Algebra.

*/
class Relation : public GenericRelation
{
  public:
    Relation( const ListExpr typeInfo, const bool isTemporary = false );
/*
The first constructor. It creates an empty relation from a ~typeInfo~. The flag ~isTemporary~
can be used to create temporary relations.

*/
    Relation( const TupleType& tupleType, const bool isTemporary = false );
/*
The second constructor. It creates an empty relation from a ~tupleType~. The flag ~isTemporary~
can be used to create temporary relations.

*/
    Relation( const TupleType& tupleType, const RelationDescriptor& relDesc, const bool isTemporary = false );
/*
The third constructor. It opens a previously created relation. The flag ~isTemporary~
can be used to open temporary created relations.

*/
    Relation( const ListExpr typeInfo, const RelationDescriptor& relDesc, const bool isTemporary = false );
/*
The fourth constructor. It opens a previously created relation using the ~typeInfo~ instead of
the ~tupleType~. The flag ~isTemporary~ can be used to open temporary created relations.

*/
    ~Relation();
/*
The destructor.

*/
    static Relation *GetRelation( const RelationDescriptor& d );
/*
Given a relation descriptor, finds if there is an opened relation with that descriptor and retrieves 
its memory representation pointer. 

*/
    static Relation *In( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct );
/*
Creates a relation from the ~typeInfo~ and ~value~ information.
Corresponds to the ~In~-function of type constructor ~rel~.

*/
    static Relation *RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct );
/*
Acts as the ~In~ function, but uses internal representation for the objects.

*/
    ListExpr Out( ListExpr typeInfo );
/*
Writes a relation into a ~ListExpr~ format.
Corresponds to the ~Out~-function of type constructor ~rel~.

*/
    ListExpr SaveToList( ListExpr typeInfo );
/*
Acts as the ~Out~ function, but uses internal representation for the objects.

*/
    static Relation *Open( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo );
/*
Opens a relation.
Corresponds to the ~Open~-function of type constructor ~rel~.

*/
    bool Save( SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo );
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
Appends a tuple to the relation. Returns the size in bytes occupied by the tuple.

*/
    bool DeleteTuple( Tuple *tuple );
/*
Deletes the tuple from the relation. Returns false if the tuple could not be deleted.

*/
    void UpdateTuple( Tuple *tuple, const vector<int>& changedIndices, const vector<Attribute *>& newAttrs );
/*
Updates the tuple by putting the new attributes at the positions given by 'changedIndices' and adjusts the physical representation.

*/
    Tuple* GetTuple( const TupleId& tupleId ) const;
/*
Returns the tuple identified by ~tupleId~.

*/
    const TupleType& GetTupleType() const;
/*
Returns the tupletype of the tuples of the relation.

*/
    void Clear();
/*
Clears (empties) a relation removing all its tuples.

*/
    const int GetNoTuples() const;
/*
Gets the number of tuples in the relation.

*/
    const double GetTotalSize() const;
/*
Returns the total size of the relation in bytes.

*/
    RelationIterator *MakeScan() const;
/*
Returns a ~RelationIterator~ for a relation scan.

*/
    PrivateRelation *GetPrivateRelation()
      { return privateRelation; }
/*
Function to give outside access to the private part of the relation class.

*/

    friend class RelationIterator;
    friend struct PrivateRelationIterator;

  private:

    PrivateRelation *privateRelation;
/*
The private attributes of the class ~Relation~.

*/
    static map<RelationDescriptor, Relation*, RelationDescriptorCompare> pointerTable;
/*
A table containing all opened relations indexed by relation descriptors.

*/

};

/*
4 Auxiliary functions' interface

4.1 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is not one
of these symbols, then the value ~error~ is returned.

*/
enum RelationType { rel, tuple, stream, ccmap, ccbool, error };
RelationType TypeOfRelAlgSymbol (ListExpr symbol);

/*
3.2 Function ~FindAttribute~

Here ~list~ should be a list of pairs of the form (~name~,~datatype~).
The function ~FindAttribute~ determines whether ~attrname~ occurs as one of
the names in this list. If so, the index in the list (counting from 1)
is returned and the corresponding datatype is returned in ~attrtype~.
Otherwise 0 is returned. Used in operator ~attr~, for example.

*/
int FindAttribute( ListExpr list, string attrname, ListExpr& attrtype);

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

Copies the attribute values of two tuples ~r~ and ~s~ into tuple ~t~.

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
