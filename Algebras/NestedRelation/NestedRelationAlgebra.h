/*
1 Preliminaries
1.1 Includes and global declarations

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h" 
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "ConstructorTemplates.h" 
#include "StandardTypes.h"
#include "QueryProcessor.h"
#include "DBArray.h"
#include "Attribute.h"
#include "RelationAlgebra.h"

#include <vector>

extern NestedList* nl;
extern QueryProcessor *qp;

/*
2 Typ arel
2.1 Class AttributeRelation

This class implements the representation of the type
constructor ~arel~. An attribute relation is a relation,
that can be used as an attribute within a nested relation.
*/

class AttributeRelation : public Attribute
{
   public:
        AttributeRelation( const ListExpr typeInfo, bool ownRelation );
        /*
        The first constructor. Constructs an empty AttributeRelation from the 
        metadata passed in typeInfo. 
        */
        
        AttributeRelation( const SmiFileId fileId );
        /*
        The second constructor. Sets pointer to the relation used for storing tuples to the
        relation with tupleFileId fileId. 
        */
        
        ~AttributeRelation();
        /*
        The destructor
        */
        
        ListExpr getArelType();
        /*
        Returns the type information of this arel-type
        */
        
        void setOwnRelation(bool b);
        /*
        Sets the value of ownRelation to b. True means that the relation
        used for saving tuples is created by AttributeRelation. False means
        that the relation was created by NestedRelation.
        */
        
        bool hasOwnRelation();
        /*
        Returns the value of ownRelation.
        */
        
        const bool isEmpty() const;
        /*
        Returns true if DBArray tupleIds is empty, false otherwise.
        */
                
        void Append(const TupleId& tupleId);
        /*
        Appends a tupleId to the DBArray tupleIds.
        */
        
        void setFileId(SmiFileId id);
        /*
        Sets tupleFile to if and rel to the relation with this SmiFileId, if such a relation
        is currently open.
        */
        
        SmiFileId getFileId();
        /*
        Returns the value of tupleFile.
        */   
        
        DBArray<TupleId>* getTupleIds(); 
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
        
        int NumOfFLOBs() const;
        
        FLOB *GetFLOB(const int i);
        
        int Compare(const Attribute*) const;
        
        bool Adjacent(const Attribute*) const;
        
        AttributeRelation *Clone() const;
        
        bool IsDefined() const;
        
        void SetDefined( bool defined );
        
        size_t Sizeof() const;
        
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
        
       
   private:        
        friend class ConstructorFunctions<AttributeRelation>; 
        DBArray<TupleId> tupleIds;              
        /*
        Saves the TupleIds of the tuples pertaining to this instance
        of AttributeRelation
        */
        
        bool tupleFileSet;
        /*
        True means that the SmiFileId tupleFile is set, false means that is not
        yet set.
        */
        
        Relation* rel;
        /*
        Pointer to the relation which is used to save the tuples pertaining to
        an instance of AttributeRelation.
        */
        
        ListExpr arelType;
        /*
        The type information for AttributeRelation.
        */
        
        SmiFileId tupleFile; 
        /*
        The SmiFileId of the relation which is used to save tuples.
        */
        
        bool ownRelation;
        /*
        True means that the relation used for saving tuples was created
        by AttributeRelation. False means that the relation was created by NestedRelation.
        */
        
        inline AttributeRelation(){};
        /*
        The empty contructor.
        */       
};

/*
2.7 Typ nrel
2.7.1 struct SubRelation

This struct contains information about any subrelations contained in
an nrel-type.
*/
struct SubRelation
{
       
       SubRelation(Relation* ptr, const string n, SmiFileId id, ListExpr tI):
                             rel(ptr),
                             name(n),
                             fileId(id),
                             typeInfo(tI)
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
3.2 Class NestedRelation


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
          NestedRelation( ListExpr typeInfo, Relation* ptr, vector<SubRelation*>& sR );
          /*
          The copy-constructor
          */
          ~NestedRelation() {};
          /*
          The destructor
          */
   
          void InsertSubRelations(const ListExpr typeInfo);
          /*
          Reads typeInfo, creates a SubRelation for every arel type in typeInfo and appends it to 
          vector subRels.
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
          typeInfo is expected to be the description of an arel-
          attribute. The returned list expression contains the description
          of the arel-attribute and, appended to it, the SmiFileId of the 
          relation which is to be used for saving tuples.
          */
          
          SubRelation* getSubRel(string name);
          /*
          returns a pointer to the Subrelation with Name name, if such a Relation exists,
          nil otherwise.
          */
          
          ListExpr getTupleType();
          /*
          returns a pointer to tupleTypeInfo
          */
          
          static int getTypeId(int algId, string typeName);
          /*
          returns the typeId of the type with name typeName from algebra with
          id algId
          */
      
          static bool NamesUnique(ListExpr type);
          /*
          Returns true, if all attribute names in the type are unique, false
          otherwise.
          */
          
          static ListExpr unnestedList(ListExpr typeInfo);
          /*
          creates an unnested list from typeInfo. Is needed to check that all 
          attribute names are unique.
          */
            
          static bool saveString (string& s, SmiRecord& valueRecord, size_t& offset);
          /*
          Auxiliary function for saving an instance of NestedRelation.
          */
      
          static bool readString (string& s, SmiRecord& valueRecord, size_t& offset);
          /*
          Auxiliary function for opening an instance of NestedRelation.
          */
          
          void     Delete ();
          /*
          Used to delete the primary relation and all subrelations.
          */
          
          void Close();
          /*
          Used to close the primary relation and all subrelation.
          */
          
          NestedRelation* Clone(ListExpr typeInfo);
          /*
          Used to clone a nested relation
          */
          
          vector<SubRelation*>* getSubRels();
          /*
          Returns a pointer to subrels.
          */
          
                    
          static Word     In( const ListExpr typeInfo, const ListExpr instance,
                          const int errorPos, ListExpr& errorInfo, bool& correct );
    
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
          
          static ListExpr Property();
   private:
        NestedRelation(){}
        Relation* primary;
        vector<SubRelation*> subRels;
        ListExpr tupleTypeInfo;
        ListExpr primaryTypeInfo;
};
