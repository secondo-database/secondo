#ifndef RELATION_ALGEBRA_PERSISTENCE_H
#define RELATION_ALGEBRA_PERSISTENCE_H

#include "Algebra.h"
#include "AlgebraManager.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <iostream>
#include <string>
#include <deque>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <typeinfo>
#include "RelationAlgebraInfo.h"
#include "Tuple.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern int ccTuplesCreated;
extern int ccTuplesDeleted;
extern int ccRelsCreated;
extern int ccRelsDeleted;

extern TypeConstructor cpptuple;
extern TypeConstructor cpprel;
extern ListExpr AttrTypeList;


class CcRel;
void CloseRecFile (CcRel*);
void CloseDeleteRecFile (CcRel*);
void CloseLobFile (CcRel*);
void CloseDeleteLobFile (CcRel*);

enum RelationType { rel, tuple, stream, ccmap, ccbool, error };

const int MaxSizeOfAttr = 20;

int findattr( ListExpr list, string attrname, ListExpr& attrtype, NestedList* nl);
bool IsTupleDescription(ListExpr a, NestedList* nl);

ListExpr TupleProp ();

class TupleAttributesInfo
{

    TupleAttributes* tupleType;
    AttributeType* attrTypes;

  public:

    TupleAttributesInfo (ListExpr, ListExpr);
    TupleAttributesInfo (ListExpr);
    TupleAttributesInfo (ListExpr, int);
    TupleAttributesInfo (TupleAttributes*, AttributeType*);
    ~TupleAttributesInfo ();
    TupleAttributes* GetTupleTypeInfo ();
    AttributeType* GetAttributesTypeInfo ();

};

extern TupleAttributesInfo* tai;

class CcTuple
{
  #ifndef RELALG_PERSISTENT
  
  private:

    int NoOfAttr;
    Attribute* AttrList [MaxSizeOfAttr];

    /* if a tuple is free, then a stream receiving the tuple can delete or
       reuse it */
    bool isFree;
    SmiRecordId id;
    
  #else
  
  private:

    int NoOfAttr;
    Tuple* AttrList;
    bool isFree;
    SmiRecordId id;
    AttributeType* attrTypes;
    TupleAttributes* tupleType;
    
  #endif
  
  public:

    CcTuple ();
    CcTuple ( TupleAttributes*, AttributeType* );
    CcTuple ( Tuple*, int, AttributeType*, TupleAttributes* );

    virtual ~CcTuple ();
    
    void PutTuple(Tuple*);
    void PutAttrTypes(AttributeType*);
    void PutTupleType(TupleAttributes*);  
    Tuple* GetTuple ();
    TupleAttributes* GetTupleAttributes();
    void SetAttrType(int, int, AttributeType*);
    AttributeType* GetAttributeType();
    Attribute* Get (int);
    void  Put (int, Attribute*);
    void  SetNoAttrs (int);
    int   GetNoAttrs ();
    bool IsFree();
    void SetFree(bool);

    CcTuple* CloneIfNecessary();
    
    CcTuple* Clone();

    void DeleteIfAllowed();
    
    SmiRecordId GetId();
    void SetId(SmiRecordId id);


    friend
    ostream& operator<<(ostream& s, CcTuple t);
};

class LexicographicalCcTupleCmp
{

  public:

    bool operator()(const CcTuple*, const CcTuple*) const;
};

AttributeType* CloneAttributesTypeInfo ( TupleAttributesInfo*, int );
AttributeType* CloneAttributesType ( AttributeType*, int );
TupleAttributes* CloneTupleTypeInfo ( TupleAttributesInfo*, int );
TupleAttributes* CloneTupleType ( AttributeType*, int );

string ReportTupleStatistics();
string ReportRelStatistics();
string ReportRelITStatistics();
string ReportTupleAttributesInfoStatistics();

ListExpr OutTuple (ListExpr, Word);

Word InTuple(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct);

void DeleteTuple(Word&);

bool CheckTuple(ListExpr, ListExpr&);

void* CastTuple(void*);

Word CreateTuple(int);

Word TupleInModel( ListExpr, ListExpr, int);

ListExpr TupleOutModel( ListExpr, Word);

Word TupleValueToModel( ListExpr, Word);

Word TupleValueListToModel( const ListExpr, const ListExpr,
                       const int, ListExpr&, bool& );

ListExpr RelProp ();

/*

1.3.1 Main memory representation

(Figure needs to be redrawn, doesn't work.)

Figure 2: Main memory representation of a relation (~Compact Table~) [relation.eps]

*/
typedef CTable<CcTuple*>* Relation;

class CcRel;

class CcRelIT
{ 
  friend class CcRel;
  
  #ifndef RELALG_PERSISTENT
  
  CTable<CcTuple*>::Iterator rs;
  
  #else
  
  SmiRecordFileIterator* rs;
  SmiRecord actualrec;
  SmiRecordId actualrecid;
  
  #endif
   
  CcRel* r;
   
  public :
    
  //CcRelIT (CTable<CcTuple*>::Iterator rs, CcRel* r);
  //CcRelIT (SmiRecordFileIterator* rs, CcRel* r);
  CcRelIT ();
  CcRelIT (SmiRecordFileIterator*, CcRel*, SmiRecordId, SmiRecord);
  CcRelIT (CTable<CcTuple*>::Iterator rs, CcRel* r);
  ~CcRelIT ();
  CcRelIT& operator=(CcRelIT& right);
  
  CcRel* GetRel();  
  CcTuple* GetTuple();
  void Next();
  bool EndOfScan(); 
  CcTuple* GetNextTuple();
    
};

#ifdef RELALG_PERSISTENT
class PrefetchingRelIterator
{ 
  friend class CcRel;
    
  PrefetchingIterator* iter;
  CcRel* r;
   
  PrefetchingRelIterator(CcRel* r);
    
public :
    
  ~PrefetchingRelIterator();
  
  CcRel* GetRel();  
  CcTuple* GetCurrentTuple();
  bool Next();
};
#endif /* RELALG_PERSISTENT */

class CcRel
{
  friend class CcRelIT;
  friend class PrefetchingRelIterator;

  #ifndef RELALG_PERSISTENT
  
  private:

    int NoOfTuples;
    Relation TupleList;
    SmiRecordId currentId;
    
  #else
    
  private:
  
    int NoOfTuples;
    TupleAttributesInfo* reltai;
    TupleAttributes* tupleType;
    Relation TupleList;
    SmiRecordFile* recFile;
    int recFileId;
    SmiRecordFile* lobFile;
    int lobFileId;
    
  #endif

  public:

    CcRel ();
    CcRel ( ListExpr, ListExpr );
    CcRel ( int, int, TupleAttributesInfo*, int );
    ~CcRel ();
    
    static TupleAttributesInfo* globreltai;
    
    bool OpenRecFile ();
    bool OpenLobFile ();
    
    void CloseRecFile ();
    void CloseLobFile ();
    
    CcTuple* GetTupleById(SmiRecordId id);    
    void AppendTuple (CcTuple*);
    void Empty();
    
    TupleAttributesInfo* GetTupleAttributesInfo ();
    void SetRelTupleAttributesInfo ( TupleAttributesInfo* );
        
    SmiRecordFile* GetRecFile();
    int GetRecFileId();
    
    SmiRecordFile* GetLobFile();
    int GetLobFileId();

    CcRelIT* MakeNewScan();

#ifdef RELALG_PERSISTENT
    PrefetchingRelIterator* MakeNewPrefetchedScan();
#endif /* RELALG_PERSISTENT */

    void SetNoTuples (int);
    int GetNoTuples ();

};

void Concat ( Word, Word, Word& );

ListExpr OutRel(ListExpr, Word);

Word CreateRel(int);

Word InRel(ListExpr, ListExpr,
          int, ListExpr&, bool&);

void DeleteRel(Word&);

bool CheckRel(ListExpr, ListExpr&);

void* CastRel(void*);

bool RelPersistValue( const PersistDirection,
    SmiRecord&,
    const ListExpr,
    Word& );
    
Word RelInModel( ListExpr, ListExpr, int );

ListExpr RelOutModel( ListExpr, Word );

Word RelValueToModel( ListExpr, Word );

Word RelValueListToModel( const ListExpr, const ListExpr,
                       const int, ListExpr&, bool& );

#endif /* RELATION_ALGEBRA_PERSISTENCE_H */
