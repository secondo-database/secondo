/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file  of the Relational Algebra

March 2003 Victor Almeida created the new Relational Algebra organization


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
#include "Algebra.h"
#include "StandardAttribute.h"
#include "NestedList.h"

/*
3 Type Constructor ~tuple~

3.1 Struct ~TupleId~

This class will implement the unique identification for tuples inside a relation. 

*/
struct TupleId;

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
    const int GetNoAttributes() const;
/*
Returns the number of attributes of the tuple type.

*/
    const int GetTotalSize() const;
/*
Returns the total size of the tuple.

*/
    const AttributeType& GetAttributeType( const int index ) const;
/*
Returns the attribute type at ~index~ position.

*/
    void PutAttributeType( const int index, const AttributeType& attrType );
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
    AttributeType *attrTypeArray;
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
struct PrivateTuple;
/*
Forward declaration of the struct ~PrivateTuple~. This struct will contain the
private attributes of the class ~Tuple~ and will be defined later differently 
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
    ListExpr Out( ListExpr typeInfo );
/*
Writes a tuple into a ~ListExpr~ format.
Corresponds to the ~Out~-function of type constructor ~tuple~.

*/
    ~Tuple();
/*
The destructor.

*/
    static ostream& ShowTupleStatistics( const bool reset = false, ostream& o = cout );
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
    Attribute* GetAttribute( const int index ) const;
/*
Returns the attribute at position ~index~ inside the tuple.

*/
    void PutAttribute( const int index, Attribute* attr );
/*
Puts an attribute in the position ~index~ inside the tuple.

*/
    const int GetNoAttributes() const;
/*
Returns the number of attributes of the tuple.

*/
    const TupleType& GetTupleType() const;
/*
Returns the tuple type.

*/
    const bool IsFree() const;
/*
Returns if a tuple is free.
*Need some more explanations about why it is used.*

*/
    Tuple *Clone( const bool isFree = true ) const;
/*
Create a new tuple which is a clone of this tuple.

*/
    Tuple *CloneIfNecessary();
/*
Calls the ~Clone~ function if the flag if it is necessary. 
*Need some more explanations about whether it is necessary or not.*

*/
    void DeleteIfAllowed();
/*
Deletes the tuple if it is allowed.
*Need some more explanations about whether it is allowed or not.*

*/
    void Delete();
/*
Deletes the tuple.
*Need some more explanations.*

*/
    PrivateTuple *GetPrivateTuple()
      { return privateTuple; }
/*
Function to give outside access to the private part of the tuple class.

*/
  private:
    static long tuplesCreated;
    static long tuplesDeleted;
    static long tuplesInMemory;
    static long maximumTuples;
/*
Variables used for tuple statistics.

*/

    PrivateTuple *privateTuple;
/*
The private attributes of the class ~Tuple~.

*/
};

ostream& operator <<( ostream& o, Tuple& t );
/*
The print function for tuples. Used for debugging purposes

3.6 Class ~TupleCompare~

This abstract class is used for tuple comparisons, for example,
for ordering a relation.

*/
class TupleCompare
{
  public:
    virtual ~TupleCompare() {};
    virtual bool operator()(const Tuple* a, const Tuple* b) const = 0;
/*
This operator compares two tuples ~a~ and ~b~.

*/
};

/*
3.7 Class ~LexicographicalTupleCompare~

This is a specialization of the abstract class ~TupleCompare~ which 
compares tuples by their lexicographical order.

*/
class LexicographicalTupleCompare : public TupleCompare
{
  public:
    bool operator()(const Tuple* aConst, const Tuple* bConst) const;
};

/*
4 Type constructor ~rel~

4.1 Class ~Relation~

This class implements the memory representation of the type constructor ~rel~.

*/

struct RelationDescriptor;
/*
Forward declaration of the struct ~RelationDescriptor~. This struct will contain
the necessary information for opening a relation.

*/

struct PrivateRelation;
/*
Forward declaration of the struct ~PrivateRelation~. This struct will contain the
private attributes of the class ~Relation~ and will be defined later differently 
for the Main Memory Relational Algebra and for the Persistent Relational Algebra.

*/

class RelationIterator;
/*
Forward declaration of the class ~RelationIterator~ which is needed in the class
~Relation~.

*/

class Relation
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
    Relation( const ListExpr typeInfo, const RelationDescriptor& relDesc );
/*
The third constructor. It opens a previously created relation.

*/
    static Relation *In( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct );
/*
Creates a relation from the ~typeInfo~ and ~value~ information.
Corresponds to the ~In~-function of type constructor ~rel~.

*/
    ListExpr Out( ListExpr typeInfo );
/*
Writes a relation into a ~ListExpr~ format.
Corresponds to the ~Out~-function of type constructor ~rel~.

*/
    static bool Open( SmiRecord& valueRecord, const ListExpr typeInfo, Relation*& value );
/*
Opens a relation.
Corresponds to the ~Open~-function of type constructor ~rel~.

*/
    bool Save( SmiRecord& valueRecord, const ListExpr typeInfo );
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
    ~Relation();
/*
The destructor.

*/
    void AppendTuple( Tuple *tuple );
/*
Appends a tuple to the relation.

*/
//    Tuple* GetTuple( const TupleId& tupleId ) const;
/*
Returns the tuple identified by ~tupleId~.

*/
    void Clear();
/*
Clears (empties) a relation removing all its tuples.

*/
    const int GetNoTuples() const;
/*
Gets the number of tuples in the relation.

*/
    RelationIterator *MakeScan() const;
/*
Returns a ~RelationIterator~ for a relation scan.

*/
    RelationIterator *MakeSortedScan( const TupleCompare* tupleCompare ) const;
/*
Returns a ~RelationIterator~ for a relation scan sorted by the criterias defined in
~tupleCompare~.

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
};

/*
4.2 Class ~RelationIterator~

This class is used for scanning (iterating through) relations.

*/
struct PrivateRelationIterator;
/*
Forward declaration of the struct ~PrivateRelationIterator~. This struct will contain the
private attributes of the class ~RelationIterator~ and will be defined later differently 
for the Main Memory Relational Algebra and for the Persistent Relational Algebra.

*/

class RelationIterator
{
  public:
    RelationIterator( const Relation& relation, const TupleCompare *tupleCompare = 0 );
/*
The constructor. Creates a ~RelationIterator~ for a given ~relation~ and positions the
cursor in the first tuple, if exists. If ~tupleCompare~ is defined, the iterator will
iterate in a sorted way specified by the ~tupleCompare~ criteria.

*/
    ~RelationIterator();
/*
The destructor.

*/
//    Tuple *GetTuple();
/*
Retrieves the tuple in the current position of the iterator. Returns NULL if the cursor 
is in the end of a relation.

*/
    Tuple *GetNextTuple();
/*
Retrieves the tuple in the current position of the iterator and moves the cursor forward 
to the next tuple. Returns NULL if the cursor is in the end of a relation.

*/
//    void Next();
/*
Moves the cursor forward to the next tuple. 

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
4 Auxiliary functions' interface

4.1 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is not one
of these symbols, then the value ~error~ is returned.

*/
enum RelationType { rel, tuple, stream, ccmap, ccbool, error };
RelationType TypeOfRelAlgSymbol (ListExpr symbol);

/*
4.2 Macro ~CHECK\_COND~

This macro makes reporting errors in type mapping functions more conveniently.

*/
#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

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

Checks wether a ~ListExpr~ is of the form
((a1 t1) ... (ai ti)).

*/
bool IsTupleDescription( ListExpr tupleDesc );

#endif // _RELATION_ALGEBRA_H_
