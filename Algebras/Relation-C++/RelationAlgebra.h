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

extern TypeConstructor cpptuple;
extern TypeConstructor cpprel;

enum RelationType { rel, tuple, stream, ccmap, ccbool, error };

const int MaxSizeOfAttr = 10;

int findattr( ListExpr list, string attrname, ListExpr& attrtype, NestedList* nl);
bool IsTupleDescription(ListExpr a, NestedList* nl);

ListExpr TupleProp ();

class CcTuple
{
  private:

    int NoOfAttr;
    Attribute* AttrList [MaxSizeOfAttr];

    /* if a tuple is free, then a stream receiving the tuple can delete or
       reuse it */
    bool isFree;
    SmiRecordId id;

  public:

    CcTuple ();

    virtual ~CcTuple ();

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

string ReportTupleStatistics();

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
  CTable<CcTuple*>::Iterator rs;
  CcRel* r;
  public :

  CcRelIT (CTable<CcTuple*>::Iterator rs, CcRel* r);
  ~CcRelIT ();
  CcRelIT& operator=(CcRelIT& right);

  CcTuple* GetTuple();
  void Next();
  bool EndOfScan();
  CcTuple* GetNextTuple();

};

class CcRel
{
  friend class CcRelIT;

  private:

    int NoOfTuples;
    Relation TupleList;
    SmiRecordId currentId;

  public:

    CcRel ();
    ~CcRel ();

    CcTuple* GetTupleById(SmiRecordId id);
    void    AppendTuple (CcTuple*);

    CcRelIT* MakeNewScan();

    void    SetNoTuples (int);
    int     GetNoTuples ();

};

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
