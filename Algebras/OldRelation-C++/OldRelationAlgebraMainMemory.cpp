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

[1] Implementation of Module Relation Algebra

[1] Separate part of main memory data representation

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

1 Includes, Constants, Globals, Enumerations

*/

using namespace std;

#include "OldRelationAlgebra.h"

int ccTuplesCreated = 0;
int ccTuplesDeleted = 0;

extern NestedList* nl;
extern QueryProcessor* qp;

/*

3 Type constructors of the Algebra

1.3 Type constructor ~mtuple~

The list representation of a tuple is:

----	(<attrrep 1> ... <attrrep n>)
----

Typeinfo is:

----	(<NumericType(<type exression>)> <number of attributes>)
----


For example, for

----	(mtuple
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

1.3.1 Type property of type constructor ~mtuple~

*/
ListExpr CcTupleProp ()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"(\"Myers\" 53)");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("(ident x DATA)+ -> MTUPLE"),
	                     nl->StringAtom("(mtuple((name string)(age int)))"),
			     nl->StringAtom("(<attr1> ... <attrn>)"),
			     examplelist)));
}
/*

1.3.1 Main memory representation

Each instance of the class defined below will be the main memory
representation of a value of type ~mtuple~.

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

CcTuple::~CcTuple ()
{
  ccTuplesDeleted++;
};

Attribute* CcTuple::Get (int index) {return AttrList[index];};

void  CcTuple::Put (int index, Attribute* attr)
{
  assert(index < MaxSizeOfAttr);
  AttrList[index] = attr;
};

void  CcTuple::SetNoAttrs (int noattr)
{
  assert(noattr <= MaxSizeOfAttr);
  NoOfAttr = noattr;
};

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
  Attribute* attr;

  os << "(";
  for (int i=0; i < t.GetNoAttrs(); i++)
  {
    attr = (Attribute*)t.Get(i);
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
ReportCcTupleStatistics()
{
  ostringstream buf;
  buf << ccTuplesCreated << " tuples created, "
      << ccTuplesDeleted << " tuples deleted, difference is "
      << (ccTuplesCreated - ccTuplesDeleted) << "." << endl;

  ccTuplesCreated = 0;
  ccTuplesDeleted = 0;
  return buf.str();
}
/*

1.3.2 ~Out~-function of type constructor ~mtuple~

The ~out~-function of type constructor ~mtuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested list
format and a pointer to a tuple value, stored in main memory.
The function returns the tuple value from main memory storage
in nested list format.

*/
ListExpr OutCcTuple (ListExpr typeInfo, Word  value)
{
  int attrno = 0, algebraId = 0, typeId = 0;
  
  ListExpr l, lastElem, first, valuelist;
  l = lastElem = first = valuelist = nl->TheEmptyList();

  ListExpr attrlist = nl->Second(nl->First(typeInfo));

  CcTuple* tupleptr = (CcTuple*)value.addr;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  
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

1.3.2 ~SaveToList~-function of type constructor ~mtuple~

The ~SaveToList~-function of type constructor ~mtuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested list
format and a pointer to a tuple value, stored in main memory.
The function returns the tuple value from main memory storage
in nested list format. The difference between this function and the ~Out~-
function is that it uses an internal structure and does not make correctness
tests.

*/
ListExpr SaveToListCcTuple (ListExpr typeInfo, Word  value)
{
  int attrno = 0, algebraId = 0, typeId = 0;

  ListExpr l, lastElem, first, valuelist;
  l = lastElem = first = valuelist = nl->TheEmptyList();
  ListExpr attrlist = nl->Second(nl->First(typeInfo));

  CcTuple* tupleptr = (CcTuple*)value.addr;
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    valuelist = (algM->SaveToListObj(algebraId, typeId))(nl->Rest(first),
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

1.3.2 ~In~-function of type constructor ~mtuple~

The ~in~-function of type constructor ~mtuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested
list format and the tuple value in nested list format. The function
returns a pointer to atuple value, stored in main memory in accordance to
the tuple value in nested list format.

Error handling in ~InTuple~: ~Correct~ is only true if there is the right
number of attribute values and all values have correct list representations.
Otherwise the following error messages are added to ~errorInfo~:

----	(71 mtuple 1 <errorPos>)		        atom instead of value list
	(71 mtuple 2 <errorPos>)		        not enough values
	(71 mtuple 3 <errorPos> <attrno>) 	wrong attribute value in
					        attribute <attrno>
	(71 mtuple 4 <errorPos>)		        too many values
----

is added to ~errorInfo~. Here ~errorPos~ is the number of the tuple in the
relation list (passed by ~InRelation~).


*/
Word InCcTuple(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  int  attrno, algebraId, typeId, noOfAttrs;
  Word attr;
  CcTuple* tupleaddr;
  bool valueCorrect;
  ListExpr first, firstvalue, valuelist, attrlist;

  attrno = 0;
  noOfAttrs = 0;
  tupleaddr = new CcTuple();

  attrlist =  nl->Second(nl->First(typeInfo));
  valuelist = value;
  correct = true;
  if (nl->IsAtom(valuelist))
  {
    correct = false;

    cout << "Error in reading tuple: an atom instead of a list of values." << endl;
    cout << "Tuple no." << errorPos << endl;
    cout << "The tuple is: " << endl;
    nl->WriteListExpr(value);

    errorInfo = nl->Append(errorInfo,
      nl->FourElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(1),
      nl->IntAtom(errorPos)));
    delete tupleaddr;
    return SetWord(Address(0));
  }
  else
  {

    AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
    while (!nl->IsEmpty(attrlist))
    {
      first = nl->First(attrlist);
      attrlist = nl->Rest(attrlist);
      attrno++;
      algebraId = nl->IntValue(nl->First(nl->Second(first)));
      typeId = nl->IntValue(nl->Second(nl->Second(first)));
      if (nl->IsEmpty(valuelist))
      {
        correct = false;

        cout << "Error in reading tuple: list of values is empty." << endl;
        cout << "Tuple no." << errorPos << endl;
        cout << "The tuple is: " << endl;
        nl->WriteListExpr(value);

        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(2),
            nl->IntAtom(errorPos)));
        delete tupleaddr;
        return SetWord(Address(0));

      }
      else
      {
        firstvalue = nl->First(valuelist);
        valuelist = nl->Rest(valuelist);

        attr = (algM->InObj(algebraId, typeId))(nl->Rest(first),
                 firstvalue, attrno, errorInfo, valueCorrect);

        if (valueCorrect)
        {
          correct = true;
          tupleaddr->Put(attrno - 1, (Attribute*)attr.addr);
          noOfAttrs++;
        }
        else
        {
          correct = false;

	  cout << "Error in reading tuple: wrong attribute value representation." << endl;
	  cout << "Tuple no." << errorPos << endl;
	  cout << "The tuple is: " << endl;
	  nl->WriteListExpr(value);
	  cout << endl << "The attribute is: " << endl;
	  nl->WriteListExpr(firstvalue);
	  cout << endl;

          errorInfo = nl->Append(errorInfo,
            nl->FiveElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(3),
          nl->IntAtom(errorPos), nl->IntAtom(attrno)));
          delete tupleaddr;
          return SetWord(Address(0));
        }
      }
    }
    if (!nl->IsEmpty(valuelist))
    {
      correct = false;

      cout << "Error in reading tuple: too many attribute values." << endl;
      cout << "Tuple no." << errorPos << endl;
      cout << "The tuple is: " << endl;
      nl->WriteListExpr(value);

      errorInfo = nl->Append(errorInfo,
      nl->FourElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(4),
      nl->IntAtom(errorPos)));
      delete tupleaddr;
      return SetWord(Address(0));
    }
  }
  tupleaddr->SetNoAttrs(noOfAttrs);
  return (SetWord(tupleaddr));
}

/*

1.3.2 ~In~-function of type constructor ~mtuple~

The ~in~-function of type constructor ~mtuple~ takes as inputs a type
description (~typeInfo~) of the tuples attribute structure in nested
list format and the tuple value in nested list format. The function
returns a pointer to a tuple value, stored in main memory in accordance to
the tuple value in nested list format. The difference between this function
and the ~In~-function is that it uses an internal structure and does not
make correctness tests.

*/
Word RestoreFromListCcTuple(ListExpr typeInfo, ListExpr value,
                            int errorPos, ListExpr& errorInfo, bool& correct)
{
  int  attrno, algebraId, typeId, noOfAttrs;
  Word attr;
  CcTuple* tupleaddr;
  bool valueCorrect;
  ListExpr first, firstvalue, valuelist, attrlist;

  attrno = 0;
  noOfAttrs = 0;
  tupleaddr = new CcTuple();

  attrlist =  nl->Second(nl->First(typeInfo));
  valuelist = value;
  correct = true;

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    attrno++;
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    firstvalue = nl->First(valuelist);
    valuelist = nl->Rest(valuelist);

    attr = (algM->RestoreFromListObj(algebraId, typeId))(nl->Rest(first),
            firstvalue, attrno, errorInfo, valueCorrect);

    assert(valueCorrect);
    tupleaddr->Put(attrno - 1, (Attribute*)attr.addr);
    noOfAttrs++;
  }
  tupleaddr->SetNoAttrs(noOfAttrs);
  correct = true;
  return (SetWord(tupleaddr));
}

/*

1.3.4 ~Destroy~-function of type constructor ~mtuple~

A type constructor's ~destroy~-function is used by the query processor in order
to deallocate memory occupied by instances of Secondo objects. They may have
been created in two ways:

  * as return values of operator calls

  * by calling a type constructor's ~create~-function.

The corresponding function of type constructor ~tuple~ is called ~DeleteTuple~.

*/
void DeleteCcTuple(Word& w)
{
  CcTuple* tupleptr;
  int attrno;
  tupleptr = (CcTuple*)w.addr;
  attrno = tupleptr->GetNoAttrs();
  for (int i = 0; i <= (attrno - 1); i++)
  {
    delete (Attribute*)tupleptr->Get(i);
  }
  delete tupleptr;
}
/*

1.3.4 ~Check~-function of type constructor ~mtuple~

Checks the specification:

----	(ident x DATA)+		-> MTUPLE	mtuple
----

with the additional constraint that all identifiers used (attribute names)
must be distinct. Hence a tuple type has the form:

----	(mtuple
	    (
		(age x)
		(name y)))
----

and ~x~ and ~y~ must be types of kind DATA. Kind MTUPLE introduces the
following error codes:

----	(... 1) 	Empty tuple type
	(... 2 x)  	x is not an attribute list, but an atom
	(... 3 x)	Doubly defined attribute name x
	(... 4 x)	Invalid attribute name x
	(... 5 x)	Invalid attribute definition x (x is not a pair)
	(... 6 x)	Attribute type does not belong to kind DATA
----

*/
bool CheckCcTuple(ListExpr type, ListExpr& errorInfo)
{
  vector<string> attrnamelist;
  ListExpr attrlist, pair;
  string attrname;
  bool correct, ckd;
  int unique;
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && (nl->IsEqual(nl->First(type), "mtuple",
       true)))
  {
    attrlist = nl->Second(type);
    if (nl->IsEmpty(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->ThreeElemList(nl->IntAtom(61), nl->SymbolAtom("MTUPLE"),
          nl->IntAtom(1)));
      return false;
    }
    if (nl->IsAtom(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
        nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("MTUPLE"),
          nl->IntAtom(2),
        attrlist));
      return false;
    }
    algMgr = SecondoSystem::GetAlgebraManager();

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
             nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("MTUPLE"),
               nl->IntAtom(3), nl->First(pair)));
            correct = false;
          }
          attrnamelist.push_back(attrname);
          ckd =  algMgr->CheckKind("DATA", nl->Second(pair), errorInfo);
          if (!ckd)
          {
            errorInfo = nl->Append(errorInfo,
              nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("MTUPLE"),
                nl->IntAtom(6),nl->Second(pair)));
          }
          correct = correct && ckd;
        }
        else
        {
          errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("MTUPLE"),
          nl->IntAtom(4),nl->First(pair)));
          correct = false;
        }
      }
      else
      {
        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(nl->IntAtom(61), nl->SymbolAtom("MTUPLE"),
          nl->IntAtom(5),pair ));
        correct = false;
      }
    }
    return correct;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("MTUPLE"), type));
    return false;
  }
}

/*

3.2.5 ~Cast~-function of type constructor ~mtuple~

*/
void* CastCcTuple(void* addr)
{
  return ( 0 );
}
/*

1.3.3 ~Create~-function of type constructor ~mtuple~

The function is used to allocate memory sufficient for keeping one instance
of ~mtuple~.

*/
Word CreateCcTuple(const ListExpr typeInfo)
{
  CcTuple* tup;
  tup = new CcTuple();
  return (SetWord(tup));
}

/*

1.4 TypeConstructor ~mrel~

The list representation of a relation is:

----	(<tuplerep 1> ... <tuplerep n>)
----

Typeinfo is:

----	(<NumericType(<type exression>)>)
----

For example, for

----	(mrel (mtuple ((name string) (age int))))
----

the type info is

----	((2 1) ((2 2) ((name (1 4)) (age (1 1)))))
----

1.3.1 Type property of type constructor ~mrel~

*/
ListExpr CcRelProp ()
{
  ListExpr listreplist = nl->TextAtom();
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(listreplist,"(<mtuple>*)where <mtuple> is "
  "(<attr1> ... <attrn>)");
  nl->AppendText(examplelist,"((\"Myers\" 53)(\"Smith\" 21))");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("MTUPLE -> MREL"),
	               nl->StringAtom("(mrel(mtuple((name string)(age int))))"),
		       listreplist,
		       examplelist)));
}

CcRel::CcRel ()
{
  currentId = 1;
  NoOfTuples = 0;
  TupleList = new CTable<CcTuple*>(100);
};

CcRel::~CcRel ()
{
  delete TupleList;
};

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
    DeleteCcTuple(w);
    ++iter;
  }
  delete TupleList;

  currentId = 1;
  NoOfTuples = 0;
  TupleList = new CTable<CcTuple*>(100);
}

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

1.4.2 ~Out~-function of type constructor ~mrel~

*/
ListExpr OutCcRel(ListExpr typeInfo, Word  value)
{
  CcTuple* t = 0;

  ListExpr l, lastElem, tlist, TupleTypeInfo;
  l = lastElem = tlist = TupleTypeInfo = nl->TheEmptyList();

  CcRel* r = (CcRel*)(value.addr);
  CcRelIT* rit = r->MakeNewScan();

  //cerr << "OutRel " << endl;
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
	  nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

    tlist = OutCcTuple(TupleTypeInfo, SetWord(t));

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

1.4.2 ~SaveToList~-function of type constructor ~mrel~

*/
ListExpr SaveToListCcRel(ListExpr typeInfo, Word value)
{
  CcTuple* t = 0;
  
  ListExpr l, lastElem, tlist, TupleTypeInfo;
  l = lastElem = tlist = TupleTypeInfo = nl->TheEmptyList();

  CcRel* r = (CcRel*)(value.addr);
  CcRelIT* rit = r->MakeNewScan();

  //cerr << "OutRel " << endl;
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
          nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

    tlist = SaveToListCcTuple(TupleTypeInfo, SetWord(t));

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

1.3.3 ~Create~-function of type constructor ~mrel~

The function is used to allocate memory sufficient for keeping one instance
of ~mrel~.

*/
Word CreateCcRel(const ListExpr typeInfo)
{
  //cerr << "CreateRel " << endl;
  CcRel* rel = new CcRel();
  return (SetWord(rel));
}
/*

1.4.2 ~In~-function of type constructor ~mrel~

~value~ is the list representation of the relation. The structure of
~typeInfol~ and ~value~ are described above. Error handling in ~InRel~:

The result relation will contain all tuples that have been converted
correctly (have correct list expressions). For all other tuples, an error
message containing the position of the tuple within this relation (list) is
added to ~errorInfo~. (This is done by procedure ~InTuple~ called by ~InRel~).
If any tuple representation is wrong, then ~InRel~ will return ~correct~ as
FALSE and will itself add an error message of the form

----	(InRelation <errorPos>)
----

to ~errorInfo~. The value in ~errorPos~ has to be passed from the environment;
probably it is the position of the relation object in the list of
database objects.

*/
Word InCcRel(ListExpr typeInfo, ListExpr value,
             int errorPos, ListExpr& errorInfo, bool& correct)
{
  ListExpr tuplelist, TupleTypeInfo, first;
  CcRel* rel;
  CcTuple* tupleaddr;
  int tupleno, count;
  bool tupleCorrect;

  correct = true;
  count = 0;
  rel = new CcRel();

  //cerr << "InRel " << endl;
  tuplelist = value;
  TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
    nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
  tupleno = 0;
  if (nl->IsAtom(value))
  {
    correct = false;
    errorInfo = nl->Append(errorInfo,
    nl->ThreeElemList(nl->IntAtom(70), nl->SymbolAtom("mrel"), tuplelist));
    return SetWord(rel);
  }
  else
  { // increase tupleno
    while (!nl->IsEmpty(tuplelist))
    {
      first = nl->First(tuplelist);
      tuplelist = nl->Rest(tuplelist);
      tupleno++;
      tupleaddr = (CcTuple*)(InCcTuple(TupleTypeInfo, first, tupleno,
        errorInfo, tupleCorrect).addr);

      if (tupleCorrect)
      {
        tupleaddr->SetFree(false);
        rel->AppendTuple(tupleaddr);
        count++;
      }
      else
      {
        correct = false;
      }
    }
    if (!correct)
    {
      errorInfo = nl->Append(errorInfo,
      nl->TwoElemList(nl->IntAtom(72), nl->SymbolAtom("mrel")));
    }
    else rel->SetNoTuples(count);
    return (SetWord((void*)rel));
  }
}
/*

1.4.2 ~RestoreFromList~-function of type constructor ~mrel~

*/
Word RestoreFromListCcRel(ListExpr typeInfo, ListExpr value,
                          int errorPos, ListExpr& errorInfo, bool& correct)
{
  ListExpr tuplelist, TupleTypeInfo, first;
  CcRel* rel;
  CcTuple* tupleaddr;
  int tupleno, count;
  bool tupleCorrect;

  correct = true;
  count = 0;
  rel = new CcRel();

  //cerr << "InRel " << endl;
  tuplelist = value;
  TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
    nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
  tupleno = 0;

  // increase tupleno
  while (!nl->IsEmpty(tuplelist))
  {
    first = nl->First(tuplelist);
    tuplelist = nl->Rest(tuplelist);
    tupleno++;
    tupleaddr = (CcTuple*)(RestoreFromListCcTuple(TupleTypeInfo, first, tupleno,
                                                 errorInfo, tupleCorrect).addr);

    assert(tupleCorrect);
    tupleaddr->SetFree(false);
    rel->AppendTuple(tupleaddr);
    count++;
  }
  rel->SetNoTuples(count);
  correct = true;
  return (SetWord((void*)rel));
}


/*

1.3.4 ~Destroy~-function of type constructor ~mrel~


The corresponding function of type constructor ~mrel~ is called ~DeleteRel~.

*/
void DeleteCcRel(Word& w)
{
  if(w.addr == 0)
  {
    return;
  }

  CcTuple* t;
  CcRel* r;
  Word v;

  r = (CcRel*)w.addr;
  //cerr << "DeleteRel " << endl;
  CcRelIT* rit = r->MakeNewScan();
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    v = SetWord(t);
    DeleteCcTuple(v);
  }
  delete rit;
  delete r;
}
/*

4.3.8 ~Check~-function of type constructor ~mrel~

Checks the specification:

----    MTUPLE   -> MREL          mrel
----

Hence the type expression must have the form

----    (mrel x)
----

and ~x~ must be a type of kind MTUPLE.

*/
bool CheckCcRel(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if ((nl->ListLength(type) == 2) && nl->IsEqual(nl->First(type), "mrel"))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return (algMgr->CheckKind("MTUPLE", nl->Second(type), errorInfo));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("MREL"), type));
    return false;
  }
}
/*

3.2.5 ~Cast~-function of type constructor ~mrel~

*/
void* CastCcRel(void* addr)
{
  return ( 0 );
}
/*

3.2.5 ~PersistFunction~ of type constructor ~mrel~

This is a slightly modified version of the function ~DefaultPersistValue~ (from
~Algebra~) which creates the relation from the SmiRecord only if it does not
yet exist.

The idea is to maintain a cache containing the relation representations that
have been built in memory. The cache basically stores pairs (recordId, relation
value). If the record Id passed to this function is found, the cached relation
value is returned instead of building a new one.

*/
bool
OpenCcRel( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  NestedList* nl = SecondoSystem::GetNestedList();
  ListExpr valueList = nl->TheEmptyList();
  string valueString = "";
  int valueLength = 0;

  SmiKey mykey;
  SmiRecordId recId = 0;
  mykey = valueRecord.GetKey();
  assert( mykey.GetKey(recId) );

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
  int pos = 0;
  for ( int j = 0; j < cachesize; j++ )
    if ( key[j]  == recId ) {
      found = true;
      pos = j;
      break;
    }

  if ( found ) {value = cache[pos]; return true;}

  // prepare to cache the value constructed from the list
  if ( key[current] != 0 ) {
    DeleteCcRel(cache[current]);
  }

  key[current] = recId;

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool correct;
  valueRecord.Read( &valueLength, sizeof( valueLength ), offset );
  offset += sizeof( valueLength );
  char* buffer = new char[valueLength];
  valueRecord.Read( buffer, valueLength, offset );
  offset += valueLength;
  valueString.assign( buffer, valueLength );
  delete []buffer;
  nl->ReadFromString( valueString, valueList );
  value = RestoreFromListCcRel( typeInfo, nl->First(valueList), 1, errorInfo, correct);

  cache[current++] = value;
  if ( current == cachesize ) current = 0;

  if ( errorInfo != 0 )     {
    nl->Destroy( errorInfo );
  }
  nl->Destroy( valueList );
  return (true);
}

bool
SaveCcRel( SmiRecord& valueRecord,
           size_t& offset,
           const ListExpr typeInfo,
           Word& value )
{
  NestedList* nl = SecondoSystem::GetNestedList();
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = SaveToListCcRel( typeInfo, value );
  valueList = nl->OneElemList( valueList );
  nl->WriteToString( valueString, valueList );
  valueLength = valueString.length();
  valueRecord.Write( &valueLength, sizeof( valueLength ), offset );
  offset += sizeof( valueLength );
  valueRecord.Write( valueString.data(), valueString.length(), offset );
  offset += valueString.length();

  value = SetWord(Address(0));

  nl->Destroy( valueList );
  return (true);
}


