/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of main memory data representation

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

[TOC]

1 Includes, Constants, Globals, Enumerations

*/
#ifndef RELALG_PERSISTENT  

using namespace std;

#include "RelationAlgebra.h"

int ccTuplesCreated = 0;
int ccTuplesDeleted = 0;
int ccRelsCreated = 0;
int ccRelsDeleted = 0;
int ccRelITsCreated = 0;
int ccRelITsDeleted = 0;
int ccTupleAttributesInfoCreated = 0;
int ccTupleAttributesInfoDeleted = 0;

void CloseRecFile (CcRel* r) { return; };
void CloseDeleteRecFile (CcRel* r) { return; };
void CloseLobFile (CcRel* r) { return; };
void CloseDeleteLobFile (CcRel* r) { return; };

//static NestedList* nl;
//static QueryProcessor* qp;

//enum RelationType { rel, tuple, stream, ccmap, ccbool, error };

TupleAttributesInfo::TupleAttributesInfo (ListExpr typeInfo, ListExpr value)
{
  tupleType = 0;
  attrTypes = 0;
  ccTupleAttributesInfoCreated++;
};

TupleAttributesInfo::TupleAttributesInfo (ListExpr value, int attrno)
{
  tupleType = 0;
  attrTypes = 0;
  ccTupleAttributesInfoCreated++;
};
  
TupleAttributesInfo::TupleAttributesInfo (TupleAttributes* ta, AttributeType* at)
{
  tupleType = 0;
  attrTypes = 0;
  ccTupleAttributesInfoCreated++;
}

TupleAttributesInfo::~TupleAttributesInfo ()
{
  ccTupleAttributesInfoDeleted++;
};

AttributeType* TupleAttributesInfo::GetAttributesTypeInfo () {return 0;};

TupleAttributes* TupleAttributesInfo::GetTupleTypeInfo () { return 0; };

AttributeType* CloneAttributesType ( AttributeType* attrt, int attrno )
{
  return 0;
}

TupleAttributes* CloneTupleType ( AttributeType* attrt, int attrno )
{
  return 0;
}

TupleAttributes* CloneTupleTypeInfo ( TupleAttributesInfo* tuai, int attrno )
{
  return 0;
}

//TupleAttributes* TupleAttributesInfo::GetTupleAttributesInfo () {return 0;};
/*

3 Type constructors of the Algebra

1.3 Type constructor ~tuple~

The list representation of a tuple is:

----	(<attrrep 1> ... <attrrep n>)
----

Typeinfo is:

----	(<NumericType(<type exression>)> <number of attributes>)
----


For example, for

----	(tuple
		(
			(name string)
			(age int)))
----

the typeinfo is

----	(
	    	(2 2)
			(
				(name (1 4))
				(age (1 1)))
		2)
----

The typeinfo list consists of three lists. The first list is a
pair (AlgebraID, Constructor ID). The second list represents the
attributelist of the tuple. This list is a sequence of pairs (attribute
name (AlgebraID ConstructorID)). Here the ConstructorID is the identificator
of a standard data type, e.g. int. The third list is an atom and counts the
number of the tuple's attributes.

1.3.1 Type property of type constructor ~tuple~

*/
ListExpr TupleProp ()
{
  return (nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("plus"),
          nl->TwoElemList(nl->SymbolAtom("ident"), nl->SymbolAtom("DATA"))),
          nl->SymbolAtom("TUPLE")));
}
/*

1.3.1 Main memory representation

Each instance of the class defined below will be the main memory
representation of a value of type ~tuple~.

(Figure needs to be redrawn. It doesn't display or print properly.)

Figure 1: Main memory representation of a tuple (class ~CcTuple~) [tuple.eps]

*/
CcTuple::CcTuple ()
{
  NoOfAttr = 0;
  for (int i=0; i < MaxSizeOfAttr; i++)
    AttrList[i] = 0;
  ccTuplesCreated++;
};

CcTuple::CcTuple ( TupleAttributes* attributes, AttributeType* at )
{
  NoOfAttr = 0;
  ccTuplesCreated++;
};

//CcTuple::CcTuple (TupleAttributes* attributes) {};

CcTuple::~CcTuple ()
{
  ccTuplesDeleted++;
};

Tuple* CcTuple::GetTuple () { return 0; };

TupleAttributes* CcTuple::GetTupleAttributes () { return 0; };

AttributeType* CcTuple::GetAttributeType () { return 0; };

Attribute* CcTuple::Get (int index) {return AttrList[index];};

void  CcTuple::Put (int index, Attribute* attr) {AttrList[index] = attr;};

void  CcTuple::SetNoAttrs (int noattr) {NoOfAttr = noattr;};

int   CcTuple::GetNoAttrs () {return NoOfAttr;};

bool CcTuple::IsFree() { return isFree; }

void CcTuple::SetFree(bool b) { isFree = b; }

SmiRecordId CcTuple::GetId()
{
  return id;
}

void CcTuple::SetId(SmiRecordId id)
{
  this->id = id;
}

CcTuple* CcTuple::Clone()
{
  CcTuple* result = new CcTuple();
  result->SetFree(true);
  result->SetNoAttrs(GetNoAttrs());
  for(int i = 0; i < GetNoAttrs(); i++)
  {
    Attribute* attr = ((Attribute*)Get(i))->Clone();
    result->Put(i, attr);
  }
  return result;
}

CcTuple* CcTuple::CloneIfNecessary()
{
  if(IsFree())
  {
    return this;
  }
  else
  {
    return Clone();
  }
}

void CcTuple::DeleteIfAllowed()
{
  if(IsFree())
  {
    for(int i = 0; i < GetNoAttrs(); i++)
    {
      Attribute* attr = (Attribute*)Get(i);
      delete attr;
    }
    delete this;
  }
}
/*

The next function supports writing objects of class CcTuple to standard
output. It is only needed for internal tests.

*/
ostream& operator<<(ostream& os, CcTuple t)
{
  TupleElement* attr;

  os << "(";
  for (int i=0; i < t.GetNoAttrs(); i++)
  {
    attr = (TupleElement*)t.Get(i);
    attr->Print(os);
    if (i < (t.GetNoAttrs() - 1)) os << ",";
  }
  os << ")";
  return os;
}
/*

The lexicographical order on CcTuple. To be used in conjunction with
STL algorithms.

*/
bool LexicographicalCcTupleCmp::operator()(const CcTuple* aConst, const CcTuple* bConst) const
{
  CcTuple* a = (CcTuple*)aConst;
  CcTuple* b = (CcTuple*)bConst;


  for(int i = 0; i < a->GetNoAttrs(); i++)
  {
    if(((Attribute*)a->Get(i))->Compare(((Attribute*)b->Get(i))) < 0)
    {
      return true;
    }
    else
    {
      if(((Attribute*)a->Get(i))->Compare(((Attribute*)b->Get(i))) > 0)
      {
        return false;
      }
    }
  }
  return false;
}

string
ReportTupleStatistics()
{
  ostringstream buf;
  buf << ccTuplesCreated << " tuples created, "
      << ccTuplesDeleted << " tuples deleted, difference is "
      << (ccTuplesCreated - ccTuplesDeleted) << "." << endl;

  ccTuplesCreated = 0;
  ccTuplesDeleted = 0;
  return buf.str();
}

string
ReportRelStatistics()
{
  ostringstream buf;
  buf << ccRelsCreated << " relations created, "
      << ccRelsDeleted << " relations deleted, difference is "
      << (ccRelsCreated - ccRelsDeleted) << "." << endl;

  ccRelsCreated = 0;
  ccRelsDeleted = 0;
  return buf.str();
}

string
ReportRelITStatistics()
{
  ostringstream buf;
  buf << ccRelITsCreated << " relationits created, "
      << ccRelITsDeleted << " relationits deleted, difference is "
      << (ccRelITsCreated - ccRelITsDeleted) << "." << endl;

  ccRelITsCreated = 0;
  ccRelITsDeleted = 0;
  return buf.str();
}

string
ReportTupleAttributesInfoStatistics()
{
  ostringstream buf;
  buf << ccTupleAttributesInfoCreated << " tupleattributesinfos created, "
      << ccTupleAttributesInfoDeleted << " tupleattributesinfos deleted, difference is "
      << (ccTupleAttributesInfoCreated - ccTupleAttributesInfoDeleted) << "." << endl;

  ccTupleAttributesInfoCreated = 0;
  ccTupleAttributesInfoDeleted = 0;
  return buf.str();
}
/*
5.6.1 Help Function ~Concat~

Copies the attribute values of two tuples
(words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void Concat (Word r, Word s, Word& t)
{
  int rnoattrs, snoattrs, tnoattrs;
  Attribute* attr;

  rnoattrs = ((CcTuple*)r.addr)->GetNoAttrs();
  snoattrs = ((CcTuple*)s.addr)->GetNoAttrs();
  if ((rnoattrs + snoattrs) > MaxSizeOfAttr)
  {
    tnoattrs = MaxSizeOfAttr;
  }
  else
  {
    tnoattrs = rnoattrs + snoattrs;
  }

  ((CcTuple*)t.addr)->SetNoAttrs(tnoattrs);
  for (int i = 1; i <= rnoattrs; i++)
  {
    attr = ((CcTuple*)r.addr)->Get(i - 1);
    ((CcTuple*)t.addr)->Put((i - 1), ((StandardAttribute*)attr)->Clone());
  }
  for (int j = (rnoattrs + 1); j <= tnoattrs; j++)
  {
    attr = ((CcTuple*)s.addr)->Get(j - rnoattrs - 1);
    ((CcTuple*)t.addr)->Put((j - 1), ((StandardAttribute*)attr)->Clone());
  }
}
/*

1.3.2 ~Out~-function of type constructor ~tuple~

The ~out~-function of type constructor ~tuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested list
format and a pointer to a tuple value, stored in main memory.
The function returns the tuple value from main memory storage
in nested list format.

*/
ListExpr OutTuple (ListExpr typeInfo, Word  value)
{
  int attrno, algebraId, typeId;
  ListExpr l, lastElem, attrlist, first, valuelist;
  CcTuple* tupleptr;

  tupleptr = (CcTuple*)value.addr;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  attrlist = nl->Second(nl->First(typeInfo));
  attrno = 0;
  l = nl->TheEmptyList();
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));
    valuelist = (algM->OutObj(algebraId, typeId))(nl->Rest(first),
                  SetWord(tupleptr->Get(attrno)));
    attrno++;
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(valuelist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, valuelist);
  }
  return l;
}
/*

1.3.4 ~Destroy~-function of type constructor ~tuple~

A type constructor's ~destroy~-function is used by the query processor in order
to deallocate memory occupied by instances of Secondo objects. They may have
been created in two ways:

  * as return values of operator calls

  * by calling a type constructor's ~create~-function.

The corresponding function of type constructor ~tuple~ is called ~DeleteTuple~.

*/
void DeleteTuple(Word& w)
{
  CcTuple* tupleptr;
  int attrno;
  tupleptr = (CcTuple*)w.addr;
  attrno = tupleptr->GetNoAttrs();
  for (int i = 0; i <= (attrno - 1); i++)
  {
    delete (TupleElement*)tupleptr->Get(i);
  }
  delete tupleptr;
}
/*

1.3.4 ~Check~-function of type constructor ~tuple~

Checks the specification:

----	(ident x DATA)+		-> TUPLE	tuple
----

with the additional constraint that all identifiers used (attribute names)
must be distinct. Hence a tuple type has the form:

----	(tuple
	    (
		(age x)
		(name y)))
----

and ~x~ and ~y~ must be types of kind DATA. Kind TUPLE introduces the
following error codes:

----	(... 1) 	Empty tuple type
	(... 2 x)  	x is not an attribute list, but an atom
	(... 3 x)	Doubly defined attribute name x
	(... 4 x)	Invalid attribute name x
	(... 5 x)	Invalid attribute definition x (x is not a pair)
	(... 6 x)	Attribute type does not belong to kind DATA
----

*/
bool CheckTuple(ListExpr type, ListExpr& errorInfo)
{
  vector<string> attrnamelist;
  ListExpr attrlist, pair;
  string attrname;
  bool correct, ckd;
  int unique;
  vector<string>::iterator it;
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && (nl->IsEqual(nl->First(type), "tuple",
       true)))
  {
    attrlist = nl->Second(type);
    if (nl->IsEmpty(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->ThreeElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(1)));
      return false;
    }
    if (nl->IsAtom(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(2),
        attrlist));
      return false;
    }
    algMgr = SecondoSystem::GetAlgebraManager();
    attrnamelist.resize(MaxSizeOfAttr);
    it = attrnamelist.begin();
    unique = 0;
    correct = true;
    while (!nl->IsEmpty(attrlist))
    {
      pair = nl->First(attrlist);
      attrlist = nl->Rest(attrlist);
      if (nl->ListLength(pair) == 2)
      {
        if ((nl->IsAtom(nl->First(pair))) &&
          (nl->AtomType(nl->First(pair)) == SymbolType))
        {
          attrname = nl->SymbolValue(nl->First(pair));
          unique = std::count(attrnamelist.begin(), attrnamelist.end(),
                         attrname);
          if (unique > 0)
          {
            errorInfo = nl->Append(errorInfo,
             nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
               nl->IntAtom(3), nl->First(pair)));
            correct = false;
          }
          *it = attrname;
          ckd =  algMgr->CheckKind("DATA", nl->Second(pair), errorInfo);
          if (!ckd)
          {
            errorInfo = nl->Append(errorInfo,
              nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
                nl->IntAtom(6),nl->Second(pair)));
          }
          correct = correct && ckd;
        }
        else
        {
          errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(4),nl->First(pair)));
          correct = false;
        }
      }
      else
      {
        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
          nl->IntAtom(5),pair ));
        correct = false;
      }
      it++;
    }
    return correct;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("TUPLE"), type));
    return false;
  }
}

/*

3.2.5 ~Cast~-function of type constructor ~tuple~

*/
void* CastTuple(void* addr)
{
  return ( 0 );
}
/*

1.3.3 ~Create~-function of type constructor ~tuple~

The function is used to allocate memory sufficient for keeping one instance
of ~tuple~. The ~Size~-parameter is not evaluated.

*/
Word CreateTuple(int Size)
{
  CcTuple* tup;
  tup = new CcTuple();
  return (SetWord(tup));
}
/*

3.2.5 ~Model~-functions of type constructor ~tuple~

*/
Word TupleInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr TupleOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word TupleValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word TupleValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}
/*

1.4 TypeConstructor ~rel~

The list representation of a relation is:

----	(<tuplerep 1> ... <tuplerep n>)
----

Typeinfo is:

----	(<NumericType(<type exression>)>)
----

For example, for

----	(rel (tuple ((name string) (age int))))
----

the type info is

----	((2 1) ((2 2) ((name (1 4)) (age (1 1)))))
----

1.3.1 Type property of type constructor ~rel~

*/
ListExpr RelProp ()
{
  return (nl->TwoElemList(nl->OneElemList(nl->SymbolAtom("TUPLE")),
          nl->SymbolAtom("REL")));
}

TupleAttributesInfo* CcRel::globreltai = 0;

CcRel::CcRel () 
{
  currentId = 1;
  NoOfTuples = 0; 
  TupleList = new CTable<CcTuple*>(100);
  ccRelsCreated++;
};

CcRel::CcRel ( ListExpr ti, ListExpr v ) 
{
  currentId = 1;
  NoOfTuples = 0; 
  TupleList = new CTable<CcTuple*>(100);
  ccRelsCreated++;
};

CcRel::CcRel ( int rfi, int lfi, TupleAttributesInfo* t, int noTuples )
{
  currentId = 1;
  NoOfTuples = 0; 
  TupleList = new CTable<CcTuple*>(100);
  ccRelsCreated++;
};

CcRel::~CcRel () 
{ 
  delete TupleList;
  ccRelsDeleted++; 
};

void CcRel::SetRelTupleAttributesInfo ( TupleAttributesInfo* ta )
{
  return;
}

void CcRel::CloseRecFile () //dummy function
{
  return;
}

void CcRel::CloseLobFile () //dummy function
{
  return;
}


void CcRel::AppendTuple (CcTuple* t)
{
  t->SetId(currentId);
  currentId++;
  TupleList->Add(t);
  NoOfTuples++;
};

void CcRel::Empty()
{
  CTable<CcTuple*>::Iterator iter = TupleList->Begin();
  Word w;

  while(iter != TupleList->End())
  {
    w = SetWord(*iter);
    DeleteTuple(w);
    ++iter;
  }
  delete TupleList;

  currentId = 1;
  NoOfTuples = 0;
  TupleList = new CTable<CcTuple*>(100);
}

SmiRecordFile* CcRel::GetRecFile() { return 0; };

CcRelIT* CcRel::MakeNewScan()
{
  return new CcRelIT(TupleList->Begin(), this);
}

CcTuple* CcRel::GetTupleById(SmiRecordId id)
{
  return (*TupleList)[id];
}

void CcRel::SetNoTuples (int notuples) 
{
  NoOfTuples = notuples;
};

int CcRel::GetNoTuples () 
{
  return NoOfTuples;
};

CcRelIT::CcRelIT (CTable<CcTuple*>::Iterator rs, CcRel* r)
{
  this->rs = rs;
  this->r = r;
}

CcRelIT::~CcRelIT () {};

CcRel* CcRelIT::GetRel()
{
  return r;
}

CcTuple* CcRelIT::GetTuple() {return ((CcTuple*)(*rs));};

void CcRelIT::Next() { rs++; };

bool CcRelIT::EndOfScan() { return ( rs == (r->TupleList)->End() ); };

CcRelIT& CcRelIT::operator=(CcRelIT& right)
{
  rs = right.rs;
  r = right.r;
  return (*this);

};

CcTuple* CcRelIT::GetNextTuple()
{
  if( rs == (r->TupleList)->End() )
  {
    return 0;
  }
  else
  {
    CcTuple* result = *rs;
    rs++;
    return result;
  }
}
/*

1.4.2 ~Out~-function of type constructor ~rel~

*/
ListExpr OutRel(ListExpr typeInfo, Word  value)
{
  CcTuple* t;
  ListExpr l, lastElem, tlist, TupleTypeInfo;
  
  CcRel* r = (CcRel*)(value.addr);

  CcRelIT* rit = r->MakeNewScan();
  l = nl->TheEmptyList();

  //cout << "OutRel " << endl;
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
	  nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    tlist = OutTuple(TupleTypeInfo, SetWord(t));
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(tlist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, tlist);
  }
  return l;
  delete rit;
}
/*

1.3.3 ~Create~-function of type constructor ~rel~

The function is used to allocate memory sufficient for keeping one instance
of ~rel~. The ~Size~-parameter is not evaluated.

*/
Word CreateRel(int Size)
{
  CcRel* rel = new CcRel();
  return (SetWord(rel));
}
/*

1.3.4 ~Destroy~-function of type constructor ~rel~


The corresponding function of type constructor ~rel~ is called ~DeleteRel~.

*/
/*void DeleteRel(Word& w)
{
  if(w.addr == 0)
  {
    return;
  }

  CcTuple* t;
  CcRel* r;
  Word v;

  r = (CcRel*)w.addr;
  //cout << "DeleteRel " << endl;
  CcRelIT* rit = r->MakeNewScan();
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    v = SetWord(t);
    DeleteTuple(v);
  }
  delete rit;
  delete r;
}*/
/*

4.3.8 ~Check~-function of type constructor ~rel~

Checks the specification:

----    TUPLE   -> REL          rel
----

Hence the type expression must have the form

----    (rel x)
----

and ~x~ must be a type of kind TUPLE.

*/
bool CheckRel(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "rel"))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return (algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("REL"), type));
    return false;
  }
}
/*

3.2.5 ~Cast~-function of type constructor ~rel~

*/
void* CastRel(void* addr)
{
  return ( 0 );
}
/*

3.2.5 ~PersistFunction~ of type constructor ~rel~

This is a slightly modified version of the function ~DefaultPersistValue~ (from
~Algebra~) which creates the relation from the SmiRecord only if it does not
yet exist.

The idea is to maintain a cache containing the relation representations that
have been built in memory. The cache basically stores pairs (recordId, relation
value). If the record Id passed to this function is found, the cached relation
value is returned instead of building a new one.

*/
/*bool
RelPersistValue( const PersistDirection dir,
    SmiRecord& valueRecord,
    const ListExpr typeInfo,
    Word& value )
{
  NestedList* nl = SecondoSystem::GetNestedList();
  ListExpr valueList;
  string valueString;
  int valueLength;
  
  if ( dir == ReadFrom )
  {
    SmiKey mykey;
    SmiRecordId recId;
    mykey = valueRecord.GetKey();
    if ( ! mykey.GetKey(recId) ) cout << "
	RelPersistValue: Couldn't get the key!" << endl;

    // cout << "the record number is: " << recId << endl;

    static bool firsttime = true;
    const int cachesize = 20;
    static int current = 0;
    static SmiRecordId key[cachesize];
    static Word cache[cachesize];

    // initialize

    if ( firsttime ) {
      for ( int i = 0; i < cachesize; i++ ) { key[i] = 0; }
      firsttime = false;
    }

    // check whether value was cached

    bool found = false;
    int pos;
    for ( int j = 0; j < cachesize; j++ )
      if ( key[j]  == recId ) {
	found = true;
	pos = j;
        break;
      }
    
    if ( found ) {value = cache[pos]; return true;}

    // prepare to cache the value constructed from the list

    if ( key[current] != 0 ) { 
      	// cout << "I do delete!" << endl;
      DeleteRel(cache[current]);
    }

    key[current] = recId;

    ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
    bool correct;
    valueRecord.Read( &valueLength, sizeof( valueLength ), 0 );
    char* buffer = new char[valueLength];
    valueRecord.Read( buffer, valueLength, sizeof( valueLength ) );
    valueString.assign( buffer, valueLength );
    delete []buffer;
    nl->ReadFromString( valueString, valueList );
    value = InRel( nl->First(typeInfo), nl->First(valueList), 1, errorInfo,
	correct); 

    cache[current++] = value;
    if ( current == cachesize ) current = 0;
        
    if ( errorInfo != 0 )     {
      nl->Destroy( errorInfo );
    }
  }
  else // WriteTo
  {
    valueList = OutRel( nl->First(typeInfo), value );
    valueList = nl->OneElemList( valueList );
    nl->WriteToString( valueString, valueList );
    valueLength = valueString.length();
    valueRecord.Write( &valueLength, sizeof( valueLength ), 0 );
    valueRecord.Write( valueString.data(), valueString.length(), sizeof( valueLength ) );

    value = SetWord(Address(0));
  }
  nl->Destroy( valueList );
  return (true);
}*/
/*

3.2.5 ~Model~-functions of type constructor ~rel~

*/
Word RelInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr RelOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word RelValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word RelValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}
/*

5 Defnition of type constructor ~tuple~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type constructor's
name and the eleven functions previously defined.

*/
TypeConstructor cpptuple( "tuple",           TupleProp,
                          OutTuple,          InTuple,     CreateTuple,
                          DeleteTuple,       CastTuple,   CheckTuple,
			  0,                 0,
			  TupleInModel,      TupleOutModel,
			  TupleValueToModel, TupleValueListToModel );
/*

5 Definition of type constructor ~rel~

Eventually a type constructor is created by defining an instance of
class ~TypeConstructor~. Constructor's arguments are the type constructor's
name and the eleven functions previously defined.

*/
TypeConstructor cpprel( "rel",           RelProp,
                        OutRel,          InRel,   CreateRel,
                        DeleteRel,       CastRel,   CheckRel,
			RelPersistValue, 0,
			RelInModel,      RelOutModel,
			RelValueToModel, RelValueListToModel );

#endif

























