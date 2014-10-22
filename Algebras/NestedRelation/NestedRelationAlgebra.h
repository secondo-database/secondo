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

[1] Header file of the Nested Relation Algebra

August 2009 Klaus Teufel

[TOC]


1 Overview

The Nested Relation Algebra implements two type constructors,
namely ~nrel~ and ~arel~. nrel implements a nested relation, i.e.
a relation that can have subrelations as attributes. arel implements
an attribute relation, i.e. a relation that can be the attribute
of a nested relation. arel can have attributes of type arel again, so
that the Nested Relation Algebra allows for several levels of nesting.

Both nrel and arel rely heavily on the types and functions implemented
by the Relation Algebra module.

As an example, a nested relation ~publisher~ with two levels of
nesting could be described as

\begin{displaymath}
{\underline{\smash{\mathit{nrel}}}}
  ({\underline{\smash{\mathit{tuple}}}}
    (
      (\textrm{publisher}, {\underline{\smash{\mathit{string}}}}),
      (\textrm{publications}, {\underline{\smash{\mathit{arel}}}}
      ({\underline{\smash{\mathit{tuple}}}}
        (
          (\textrm{title}, {\underline{\smash{\mathit{string}}}}),
\end{displaymath}
\begin{displaymath}
          (\textrm{authors}, {\underline{\smash{\mathit{arel}}}}
          ({\underline{\smash{\mathit{tuple}}}}
            (
              (\textrm{name}, {\underline{\smash{\mathit{string}}}})
            )
          ))
        )
      ))
    )
  )
\end{displaymath}

This file will contain an interface of classes for these two type
constructors, namely ~AttributeRelation~ and ~NestedRelation~.

2 Defines, includes, and constants

*/
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"
// #include "DBArray.h"
#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "RelationAlgebra.h"

#include <vector>

extern NestedList* nl;
extern QueryProcessor *qp;

/*
3 Types arel and nrel

3.1 Class AttributeRelation

This class implements the representation of the type
constructor ~arel~. An attribute relation is a relation,
that can be used as an attribute within a nested relation.

*/

class NestedRelation;
/*
Forward declaration of class NestedRelation.

*/

class AttributeRelation : public Attribute
{
   public:
        AttributeRelation( const ListExpr typeInfo, bool nrel, int n = 0 );
/*
The first constructor. Constructs an empty AttributeRelation from the 
metadata passed in typeInfo. The DBArray tupleIds is initialized with size n. 
        
*/
        
        AttributeRelation( const SmiFileId id, const ListExpr typeInfo, 
                                               int n = 0 );
/*
The second constructor. Sets pointer to the relation used for storing tuples 
to the relation with tupleFileId fileId. The DBArray tupleIds is initialized 
with size n.
        
*/
        
        ~AttributeRelation();
/*
The destructor
        
*/
        
        ListExpr getArelType();
/*
Returns the type information of this arel-type
        
*/
        
        void setPartOfNrel(bool b);
/*
Sets the value of partOfNrel to b. True means that this arel-instance was 
created as an attribute of an nrel-instance. False means
that this arel-instance was created independently of nrel.
        
*/
       
        bool isPartOfNrel();
/*
Returns the value of partOfNrel.
        
*/
        
        const bool isEmpty() const;
/*
Returns true if DBArray tupleIds is empty, false otherwise.
        
*/
                
        void Append(const TupleId& tupleId);
/*
Appends a tupleId to the DBArray tupleIds.
        
*/
        
        void setRelId(SmiFileId id);
/*
Sets tupleFile to if and rel to the relation with this SmiFileId, if such a 
relation is currently open.
        
*/
        
        SmiFileId getRelId() const;
/*
Returns the value of tupleFile.
        
*/   
        
         DbArray<TupleId>* getTupleIds(); 
/*
Returns a pointer to DBArray fileIds.
        
*/
        
        Relation* getRel();
/*
Returns a pointer to rel.
        
*/
        
        void setRel(Relation* r);
/*
Sets rel to r.
        
*/
        
        void Destroy();
/*
Destroys the DBArray tupleIds
        
*/

        void CopyTuplesToRel(Relation* r);
/*
Appends the tuples, the Ids of which are saved in tupleIds, to rel.
Precondition is that the tuples are not nested.

*/

        void CopyTuplesToNrel(NestedRelation* nrel);
/*
Appends the tuples, the Ids of which are saved in tupleIds, to nrel.

*/
        
        int NumOfFLOBs() const;
        
        Flob *GetFLOB(const int i);
        
        int Compare(const Attribute* attr) const;
        
        bool Adjacent(const Attribute* attr) const;
        
        AttributeRelation *Clone() const;
        
        bool IsDefined() const;
        
        size_t Sizeof() const;
        
        size_t HashValue() const;
  
        void CopyFrom(const Attribute* right);
        
        static Word     In( const ListExpr typeInfo, const ListExpr value,
                        const int errorPos, ListExpr& errorInfo, 
                        bool& correct );
        
        static ListExpr Out( ListExpr typeInfo, Word value );
        
        static Word     Create( const ListExpr typeInfo );
        
        static void     Delete( const ListExpr typeInfo, Word& w );
        
        static void     Close( const ListExpr typeInfo, Word& w );
        
        static bool     Open( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );
        
        static bool     Save( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );
        
        static Word     Clone( const ListExpr typeInfo, const Word& w );
        
        static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
        
        static int      SizeOfObj();  
        
        static void* Cast(void* addr);

        static const string BasicType(){
             return "arel";
        }
        
       
   private:        
        friend struct ConstructorFunctions<AttributeRelation>; 
       
        DbArray<TupleId> tupleIds;              
/*
Saves the TupleIds of the tuples pertaining to this instance
of AttributeRelation
        
*/
      
        ListExpr arelType;
/*
The type information for AttributeRelation.
        
*/
    
        bool partOfNrel;
/*
True means that the arel-instance was created within the context of an nrel-
instance. False means that the arel-instance was created independently of
an nrel-instance.
        
*/

        bool relDelete;
/*
True means that the relation used for saving tuples should be deleted when
closing or deleting the arel instance. This is the case, if the relation for 
saving tuples was created by arel and not by the corresponding nrel. False 
means, that the relation should not be deleted, as it was created by a 
corresponding nrel instance. 

*/
        
        Relation* rel;
/*
Pointer to the relation which is used to save the tuples pertaining to
an instance of AttributeRelation.
        
*/
      
        SmiFileId relId; 
/*
The SmiFileId of the relation which is used to save tuples. relId is used
by arel to identify its relation among all open relations.
        
*/

        AttributeRelation(){}
/*
The empty constructor.

*/
};

/*
3.2 struct SubRelation

This struct contains information about any subrelations contained in
an nrel-type.

*/
struct SubRelation
{
       
       SubRelation(Relation* ptr, const string n, SmiFileId id, 
                                ListExpr tI):
                             name(n),
                             typeInfo(tI),
                             rel(ptr),
                             fileId(id)
                             {}
                             
       ~SubRelation()
       {     
       }
              
       string name;
       
       ListExpr typeInfo;
           
       Relation* rel;
       
       SmiFileId fileId;
};

/*
3.3 Class NestedRelation

This class implements the representation of the type
constructor ~nrel~. A nested relation is a relation,
that can have relations as attributes.

*/
class NestedRelation
{
   public:
          NestedRelation(ListExpr typeInfo);
/*
The first constructor. Creates an instance of NestedRelation
from the meta-data passed in typeInfo
          
*/

          NestedRelation( ListExpr typeInfo, Relation* ptr, 
                          vector<SubRelation*>& sR );
/*
The second constructor, used in the Open-function.
          
*/

           NestedRelation(){}
/*
The empty constructor.

*/
          
          ~NestedRelation(){}
/*
The destructor. 
          
*/
   
          void insertSubRelations(const ListExpr typeInfo);
/*
Reads typeInfo, creates a SubRelation for every arel type in typeInfo and 
appends it to vector subRels.
          
*/
          
          Relation* getPrimary();              
/*
Returns a point to the primary relation of the nested relation.
          
*/
          
          void append (SubRelation* srel);
/*
Appends srel to vector subRels
          
*/
      
          void setTupleTypeInfo( ListExpr typeInfo );
/*
appends the SmiFileId of the respective relation to all arel-types .
          
*/
          
          ListExpr getSubRelationInfo( ListExpr typeInfo );        
/*
typeInfo is expected to be the description of an 
arel-attribute. The returned list expression contains the description
of the arel-attribute and, appended to it, the SmiFileId of the 
relation which is to be used for saving tuples.
          
*/
          
          SubRelation* getSubRel(string name);
/*
returns a pointer to the Subrelation with Name name, if such a Relation exists,
nil otherwise.
          
*/
          
          static int getTypeId(int algId, string typeName);
/*
returns the typeId of the type with name typeName from algebra with
id algId
          
*/
      
          static bool namesUnique(ListExpr type, string& s);
/*
Returns true, if all attribute names in the type are unique, false
otherwise.
          
*/
          
          static ListExpr unnestedList(ListExpr typeInfo);
/*
creates an unnested list from typeInfo. Is needed to check that all 
attribute names are unique.
          
*/
            
          static bool saveString (string& s, SmiRecord& valueRecord, 
                                 size_t& offset);
/*
Auxiliary function for saving an instance of NestedRelation.
          
*/
      
          static bool readString (string& s, SmiRecord& valueRecord, 
                                 size_t& offset);
/*
Auxiliary function for opening an instance of NestedRelation.
          
*/
 
            void     Delete ();
/*
Used to delete the primary relation and all subrelations.
         
*/         
          
          NestedRelation* Clone(ListExpr typeInfo);


          size_t GetNoTuples() const{
            return primary->GetNoTuples();
          }
/*
Returns the number of tuples in the primary relation.

*/


/*
Used to clone a nested relation
          
*/
          
          vector<SubRelation*>* getSubRels();
/*
Returns a pointer to subrels.
          
*/
        
          AttributeRelation* storeSubRel(AttributeRelation* a, int& i);
/*
Auxiliary function for AppendTuples. It receives as first argument an 
AttributeRelation ~a~, the tuples of which are to be copied to a new 
AttributeRelation-instance ~arel~. The second argument is an integer ~i~, 
which denotes the index of the SubRelation in vector ~subRels~, that 
corresponds to the newly created AttributeRelation ~arel~. The function 
retrieves the tuples, the ids of which are saved in the DBArray of ~a~. Those 
tuplesare then appended to the SubRelation pointed to by ~i~, and the tupleIds 
are appended to ~arel~. The function returns the new AttributeRelation ~arel~.

*/

          void AppendTuple(Tuple* tuple);
/*
Appends nested tuples to the nested relation. The top level tuple is saved in
primary, the subtuples in the corresponding subrelation. Precondition is that
the type of tuple corresponds to the type of the primary relation.

*/
          
            
          static Word     In( const ListExpr typeInfo, const ListExpr instance,
                          const int errorPos, ListExpr& errorInfo, 
                          bool& correct );
    
          static ListExpr Out( ListExpr typeInfo, Word value );
    
          static Word     Create( const ListExpr typeInfo );
    
          static void     Delete( const ListExpr typeInfo, Word& w );
              
          static void     Close( const ListExpr typeInfo, Word& w );
                  
          static Word     Clone( const ListExpr typeInfo, const Word& w );
    
          static bool     Open( SmiRecord& valueRecord, 
                            size_t& offset, const ListExpr typeInfo, 
                            Word& value );
    
          static bool     Save( SmiRecord& valueRecord, size_t& offset, 
                            const ListExpr typeInfo, Word& value );
                            
          static bool     KindCheck( ListExpr type, ListExpr& errorInfo ); 

          static const string BasicType(){
              return "nrel";
          }

          static const bool checkType(const ListExpr list){
            return listutils::isRelDescription2(list, BasicType());
          }


          
   private:
        Relation* primary;
        vector<SubRelation*> subRels;
        ListExpr tupleTypeInfo;
        ListExpr primaryTypeInfo;
};
