#ifndef OLD_RELATION_ALGEBRA__H
#define OLD_RELATION_ALGEBRA_H

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
#include "OldRelationAlgebraInfo.h"

enum CcRelationType { mrel, mtuple, stream, ccmap, ccbool, error };

const int MaxSizeOfAttr = 35;  //changed by DZM, original value: 20

int CcFindAttribute( ListExpr list, string attrname, ListExpr& attrtype, NestedList* nl);
bool CcIsTupleDescription(ListExpr a, NestedList* nl);

ListExpr CcTupleProp ();

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

string ReportCcTupleStatistics();

ListExpr OutCcTuple (ListExpr, Word);

ListExpr SaveToListCcTuple (ListExpr, Word);

Word InCcTuple(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct);

Word RestoreFromListCcTuple(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct);

void DeleteCcTuple(Word&);

bool CheckCcTuple(ListExpr, ListExpr&);

void* CastCcTuple(void*);

Word CreateCcTuple(const ListExpr);

Word CcTupleInModel( ListExpr, ListExpr, int);

ListExpr CcTupleOutModel( ListExpr, Word);

Word CcTupleValueToModel( ListExpr, Word);

Word CcTupleValueListToModel( const ListExpr, const ListExpr,
                       const int, ListExpr&, bool& );

ListExpr CcRelProp ();

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
    void Empty();

    CcRelIT* MakeNewScan();

    void    SetNoTuples (int);
    int     GetNoTuples ();

};

ListExpr OutCcRel(ListExpr, Word);

ListExpr SaveToListCcRel(ListExpr, Word);

Word CreateCcRel(const ListExpr);

Word InCcRel(ListExpr, ListExpr,
          int, ListExpr&, bool&);

Word RestoreFromListCcRel(ListExpr, ListExpr,
          int, ListExpr&, bool&);

void DeleteCcRel(Word&);

bool CheckCcRel(ListExpr, ListExpr&);

void* CastCcRel(void*);

bool OpenCcRel( SmiRecord&,
              const ListExpr,
              Word& );

bool SaveCcRel( SmiRecord&,
              const ListExpr,
              Word& );
    
Word CcRelInModel( ListExpr, ListExpr, int );

ListExpr CcRelOutModel( ListExpr, Word );

Word CcRelValueToModel( ListExpr, Word );

Word CcRelValueListToModel( const ListExpr, const ListExpr,
                       const int, ListExpr&, bool& );

#endif /* OLD_RELATION_ALGEBRA_H */
