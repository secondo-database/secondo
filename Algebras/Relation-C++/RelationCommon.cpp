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

[1] Implementation of the Common Relation Algebra

March 2003 Victor Almeida created the new Relational Algebra
organization.

Nov 2004 M. Spiekermann. Implementation of the ~Comparison~
Classes moved to RelationAlgebra.h in order to declare the
operator() as inline function. Moreover some uninitialzed
variables were set to avoid warning when compiling with
optimization flag -O2.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

[TOC]

1 Overview

The Relational Algebra basically implements two type constructors,
namely ~tuple~ and ~rel~.

More information about the Relational Algebra can be found in the
RelationAlgebra.h header file.

Some functionalities are the same for both the Main Memory and the
Persistent Relational Algebra. These common functionalities belongs
to this implementation file.

2 Defines, includes, and constants

*/
#include "LogMsg.h"
#include "CharTransform.h"

#include "RelationAlgebra.h"
#include "OldRelationAlgebra.h"
#include "SecondoSystem.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "Counter.h"
#include "Symbols.h"

#include <set>
#include <string>
#include <sstream>

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

long Tuple::tuplesCreated = 0;
long Tuple::tuplesDeleted = 0;
long Tuple::maximumTuples = 0;
long Tuple::tuplesInMemory = 0;

SmiSize Tuple::extensionLimit = 256;

/*
These variables are used for tuple statistics.

3 Implementation of the class ~TupleType~

A ~TupleType~ is a collection (an array) of all attribute types
(~AttributeType~) of the tuple. This structure contains the
metadata of a tuple attributes.

*/

TupleType::TupleType( const ListExpr typeInfo ):
noAttributes(0), attrTypeArray(0),totalSize(0),refs(1),coreSize(0)
{
  int i = 0;
  size_t offset = sizeof(uint16_t);

  try {
    const string errMsg("TupleType: Wrong list format! Line ");
    if ( nl->ListLength(typeInfo) != 2) { // ( <tuple> <attr_list> )
      throw SecondoException(errMsg + int2Str(__LINE__));
    }
    noAttributes = nl->ListLength( nl->Second( typeInfo ) );
    ListExpr rest = nl->Second( typeInfo );

    attrTypeArray = new AttributeType[noAttributes];

    while( !nl->IsEmpty( rest ) )
    {
      ListExpr list = nl->First( rest );

      if (nl->ListLength(list) != 2){ //( <attr_name> <attr_desc> )
        throw SecondoException(errMsg + int2Str(__LINE__));
      }

      //list = (a b ...)
      ListExpr b = nl->Second( list );
      rest = nl->Rest( rest );

      int algId=0, typeId=0, clsSize=0;
      if( nl->IsAtom(b) ){
        throw SecondoException(errMsg + int2Str(__LINE__));
      }

      ListExpr b1 = nl->First( b );
      if( nl->IsAtom( b1 ) ) //b = (b1 b2 ...)
      {
        if ( nl->ListLength(b) < 2 ){
          throw SecondoException(errMsg + int2Str(__LINE__));
        }
        //b = (algid typeid ...)
        algId = nl->IntValue( nl->First( b ) ),
        typeId = nl->IntValue( nl->Second( b ) ),
        clsSize = (am->SizeOfObj(algId, typeId))();

      }
      else
      {
        if ( nl->ListLength(b1) < 2 ){
          throw SecondoException(errMsg + int2Str(__LINE__));
        }
        //b1 = ((algid typeid ...) ...)
        algId = nl->IntValue( nl->First(b1) );
        typeId = nl->IntValue( nl->Second(b1) );
        clsSize = (am->SizeOfObj(algId, typeId))();
      }

      int currentCoreSize = 0;
      TypeConstructor* tc = am->GetTC(algId, typeId);

      int numOfFlobs = tc->NumOfFLOBs();
      bool extStorage = false;
      if ( tc->GetStorageType() == Attribute::Extension )
      {
        currentCoreSize = sizeof(uint32_t);
        extStorage = true;
      }
      else if ( tc->GetStorageType() == Attribute::Default )
      {
        currentCoreSize = clsSize;
      }
      else
      {
        currentCoreSize = tc->SerializedFixSize();
      }

      //totalSize += clsSize;
      totalSize += currentCoreSize;
      attrTypeArray[i++] = AttributeType( algId, typeId, numOfFlobs,
                                          clsSize, currentCoreSize,
                                          extStorage, offset      );
      coreSize += currentCoreSize;
      offset += currentCoreSize;
    }
  }
  catch (SecondoException e) {
    cerr << e.msg() << endl;
    cerr << "Input list: " << nl->ToString(typeInfo) << endl;
    cerr << "Assuming list: "
         << "(a1 (algid typeid) a2 (algid typeid) ....) or" << endl;
    cerr << "               "
         << "(a1 ((algid typeid)) a2 ((algid typeid)) ....) " << endl;
  }
}

/*
4 Implementation of the class ~Tuple~

*/
Tuple *Tuple::Clone() const
{
  Counter::getRef("RA:ClonedTuples")++;
  Tuple *result = new Tuple( this->GetTupleType() );
  for( int i = 0; i < this->GetNoAttributes(); i++ )
  {
    Attribute* tmp = GetAttribute(i);
    Attribute *attr = tmp?tmp->Clone():0;
    result->PutAttribute( i, attr );
  }
  return result;
}


void Tuple::InitCounters(const bool val)
{
  tuplesCreated = 0;
  tuplesDeleted = 0;
  maximumTuples = 0;
  tuplesInMemory = 0;

  Counter::getRef(Symbol::CTR_CreatedTuples()) = 0;
  Counter::getRef(Symbol::CTR_DeletedTuples()) = 0;
  Counter::getRef(Symbol::CTR_MaxmemTuples()) = 0;
  Counter::getRef(Symbol::CTR_MemTuples()) = 0;

  Counter::reportValue(Symbol::CTR_CreatedTuples(), val );
  Counter::reportValue(Symbol::CTR_DeletedTuples(), val);
  Counter::reportValue(Symbol::CTR_MaxmemTuples(), val);
  Counter::reportValue(Symbol::CTR_MemTuples(), val);
}

void Tuple::SetCounterValues()
{
  Counter::getRef(Symbol::CTR_CreatedTuples()) = tuplesCreated;
  Counter::getRef(Symbol::CTR_DeletedTuples()) = tuplesDeleted;
  Counter::getRef(Symbol::CTR_MaxmemTuples()) = maximumTuples;
  Counter::getRef(Symbol::CTR_MemTuples()) = tuplesInMemory;
}



ostream &operator<< (ostream &os, Attribute &attrib)
{
  return attrib.Print(os);
}

ostream& operator <<( ostream& o, const Tuple& t )
{
  o << "<";
  for( int i = 0; i < t.GetNoAttributes(); i++)
  {
    o << *t.GetAttribute(i);
    if (i < t.GetNoAttributes() - 1)
      o << ", ";
  }
  return o << ">";
}

Tuple *Tuple::In( ListExpr typeInfo, ListExpr value, int errorPos,
                  ListExpr& errorInfo, bool& correct )
{
  int  attrno, algebraId, typeId, noOfAttrs;
  Word attr;
  bool valueCorrect;
  ListExpr first, firstvalue, valuelist, attrlist;

  attrno = 0;
  noOfAttrs = 0;
  Tuple *t = new Tuple( nl->First( typeInfo ) );

  attrlist =  nl->Second(nl->First(typeInfo));
  valuelist = value;

  correct = true;
  if (nl->IsAtom(valuelist))
  {
    correct = false;

    cout << "Error in reading tuple: an atom instead of a list "
         << "of values." << endl;
    cout << "Tuple no." << errorPos << endl;
    cout << "The tuple is: " << endl;
    nl->WriteListExpr(value);

    errorInfo = nl->Append(errorInfo,
      nl->FourElemList(
        nl->IntAtom(71),
        nl->SymbolAtom(Tuple::BasicType()),
        nl->IntAtom(1),
        nl->IntAtom(errorPos)));
    delete t;
    return 0;
  }
  else
  {
    while (!nl->IsEmpty(attrlist))
    {
      first = nl->First(attrlist);
      attrlist = nl->Rest(attrlist);

      algebraId =
        t->GetTupleType()->GetAttributeType( attrno ).algId;
      typeId =
        t->GetTupleType()->GetAttributeType( attrno ).typeId;
      attrno++;
      if (nl->IsEmpty(valuelist))
      {
        correct = false;

        cout << "Error in reading tuple: list of values is empty."
             << endl;
        cout << "Tuple no." << errorPos << endl;
        cout << "The tuple is: " << endl;
        nl->WriteListExpr(value);

        errorInfo = nl->Append(errorInfo,
          nl->FourElemList(
            nl->IntAtom(71),
            nl->SymbolAtom(Tuple::BasicType()),
            nl->IntAtom(2),
            nl->IntAtom(errorPos)));
        delete t;
        return 0;
      }
      else
      {
        firstvalue = nl->First(valuelist);
        valuelist = nl->Rest(valuelist);

        attr = (am->InObj(algebraId, typeId))
          (nl->First(nl->Rest(first)), firstvalue,
           attrno, errorInfo, valueCorrect);
        if (valueCorrect)
        {
          correct = true;

          t->PutAttribute(attrno - 1, (Attribute *)attr.addr);
          noOfAttrs++;
        }
        else
        {
          correct = false;

          cout << "Error in reading tuple: "
               << "wrong attribute value representation." << endl;
          cout << "Tuple no." << errorPos << endl;
          cout << "The tuple is: " << endl;
          nl->WriteListExpr(value);
          cout << endl << "The attribute is: " << endl;
          nl->WriteListExpr(firstvalue);
          cout << endl;

          errorInfo = nl->Append(errorInfo,
            nl->FiveElemList(
              nl->IntAtom(71),
              nl->SymbolAtom(Tuple::BasicType()),
              nl->IntAtom(3),
              nl->IntAtom(errorPos),
              nl->IntAtom(attrno)));
          delete t;
          return 0;
        }
      }
    }
    if (!nl->IsEmpty(valuelist))
    {
      correct = false;

      cout << "Error in reading tuple: "
           << "too many attribute values." << endl;
      cout << "Tuple no." << errorPos << endl;
      cout << "The tuple is: " << endl;
      nl->WriteListExpr(value);

      errorInfo = nl->Append(errorInfo,
        nl->FourElemList(
          nl->IntAtom(71),
          nl->SymbolAtom(Tuple::BasicType()),
          nl->IntAtom(4),
          nl->IntAtom(errorPos)));
      delete t;
      return 0;
    }
  }
  return t;
}

/*
Expects a nested List expression with format
((tuple ((ident type)..(ident type))))

*/

ListExpr Tuple::Out( ListExpr typeInfo )
{
  int attrno=0, algebraId=0, typeId=0;
  ListExpr l = nl->TheEmptyList();
  ListExpr lastElem=l, attrlist=l, first=l, valuelist=l;

  attrlist = nl->Second(nl->First(typeInfo));
  attrno = 0;
  l = nl->TheEmptyList();
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);

    algebraId = GetTupleType()->GetAttributeType( attrno ).algId;
    typeId = GetTupleType()->GetAttributeType( attrno ).typeId;
    Attribute *attr = GetAttribute( attrno );
    valuelist = (am->OutObj(algebraId, typeId))
      (nl->First(nl->Rest(first)), SetWord(attr));
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
5 Implementation of the class ~Relation~

*/
Tuple *Relation::GetTuple( const TupleId& id,
                           const int attrIndex,
                           const vector< pair<int, int> >& intervals,
                           const bool dontReportError ) const
{
  Tuple *t = 0;
  if( (t = GetTuple( id, dontReportError )) != 0 )
    t->GetAttribute( attrIndex )->Restrict( intervals );
  return t;
}

GenericRelation *Relation::In( ListExpr typeInfo, ListExpr value,
                        int errorPos, ListExpr& errorInfo,
                        bool& correct, bool tupleBuf /*=false*/)
{
  ListExpr tuplelist, TupleTypeInfo, first;
  GenericRelation* rel;
  Tuple* tupleaddr;
  int tupleno, count;
  bool tupleCorrect;

  correct = true;
  count = 0;

  if (tupleBuf)
    rel = new TupleBuffer;
  else
    rel = new Relation( typeInfo );


  tuplelist = value;
  TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
    nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
  tupleno = 0;
  if (nl->IsAtom(value))
  {
    correct = false;
    errorInfo = nl->Append(errorInfo,
    nl->ThreeElemList(
      nl->IntAtom(70),
      nl->SymbolAtom(Relation::BasicType()),
      tuplelist));
    return rel;
  }
  else
  { // increase tupleno
    while (!nl->IsEmpty(tuplelist))
    {
      first = nl->First(tuplelist);
      tuplelist = nl->Rest(tuplelist);
      tupleno++;
      tupleaddr = Tuple::In(TupleTypeInfo, first, tupleno,
                            errorInfo, tupleCorrect);

      if (tupleCorrect)
      {
        rel->AppendTuple(tupleaddr);
        tupleaddr->DeleteIfAllowed();

        count++;
      }
      else
      {
        correct = false;
      }
    }

    if (!correct)
    {
      errorInfo =
        nl->Append(errorInfo,
          nl->TwoElemList(
          nl->IntAtom(72),
          nl->SymbolAtom(Relation::BasicType())));
      delete rel;
      return 0;
    }
    else
      return rel;
  }
}

ListExpr Relation::Out( ListExpr typeInfo, GenericRelationIterator* rit )
{
  Tuple* t=0;
  ListExpr l=nl->TheEmptyList();
  ListExpr lastElem=l, tlist=l, tupleTypeInfo=l;

  //RelationIterator* rit = MakeScan();

  //cerr << "OutRel " << endl;
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    tupleTypeInfo = nl->TwoElemList(
      nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    tlist = t->Out(tupleTypeInfo);
    //cout << "REL:" << nl->ToString(tlist) << endl;
    t->DeleteIfAllowed();
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(tlist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, tlist);
  }
  delete rit;
  return l;
}

 std::ostream& Relation::Print(std::ostream& os) const
 {
   os << "Start of Relation: " << endl;
   int i = 0;
   Tuple* t = 0;
   GenericRelationIterator* it = MakeScan();
   while ((t = it->GetNextTuple()) != 0)
   {
     os << i << ".Tuple: ";
     t->Print(os);
     t->DeleteIfAllowed();
     t = 0;
     i++;
     os << endl;
   }
   delete it;
   it = 0;
   os << "End of Relation." << endl;
   return os;
 }
/*
6 Implementation of class ~TupleBuffer~

*/
Tuple *TupleBuffer::GetTuple( const TupleId& id,
                              const int attrIndex,
                              const vector< pair<int, int> >& intervals,
                              const bool dontReportError ) const
{
  Tuple *t = 0;
  if( (t = GetTuple( id, dontReportError )) != 0 )
    t->GetAttribute( attrIndex )->Restrict( intervals );
  return t;
}

/*
6 Auxilary Functions

6.1 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is
not one of these symbols, then the value ~error~ is returned.

*/
RelationType TypeOfRelAlgSymbol (ListExpr symbol)
{
  string s;

  if (nl->AtomType(symbol) == SymbolType)
  {
    s = nl->SymbolValue(symbol);
    if (s == Relation::BasicType()   ) return rel;
    if (s == TempRelation::BasicType()  ) return trel;
    if (s == Tuple::BasicType() ) return tuple;
    if (s == Symbol::STREAM()) return stream;
    if (s == Symbol::MAP()   ) return ccmap;
    if (s == CcBool::BasicType()  ) return ccbool;
  }
  return error;
}

/*
6.2 Function ~FindAttribute~

Here ~list~ should be a list of pairs of the form
(~name~,~datatype~). The function ~FindAttribute~ determines
whether ~attrname~ occurs as one of the names in this list. If so,
the index in the list (counting from 1) is returned and the
corresponding datatype is returned in ~attrtype~. Otherwise 0 is
returned. Used in operator ~attr~, for example.

*/
int
FindAttribute( ListExpr list, string attrname, ListExpr& attrtype)
{
  ListExpr first, rest;
  int j;
  string  name;

  if (nl->IsAtom(list))
    return 0;
  rest = list;
  j = 1;
  while (!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);
    if ((nl->ListLength(first) == 2) &&
       (nl->AtomType(nl->First(first)) == SymbolType))
    {
      name = nl->SymbolValue(nl->First(first));
      if (name == attrname)
      {
        attrtype = nl->Second(first);
        return j;
      }
    }
    else
      return 0; // typeerror
    j++;
  }
  return 0; // attrname not found
}

/*
6.3 Function ~ConcatLists~

Concatenates two lists.

*/
ListExpr ConcatLists( ListExpr list1, ListExpr list2)
{
  if (nl->IsEmpty(list1))
    return list2;
  else
    return nl->Cons(nl->First(list1),
                    ConcatLists(nl->Rest(list1), list2));
}

/*
6.4 Function ~AttributesAreDisjoint~

Checks wether two ListExpressions are of the form
((a1 t1) ... (ai ti)) and ((b1 d1) ... (bj dj))
and whether the ai and the bi are disjoint.

*/

bool insertArelNames (set<string>& aNames, ListExpr a)
{
  ListExpr rest = nl->Second(nl->Second(a));
  ListExpr current;
  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2))
    {
      if ((nl->IsAtom(nl->First(current)))
           && (nl->AtomType(nl->First(current)) == SymbolType)
           && (((nl->IsAtom(nl->Second(current)))
           && (nl->AtomType(nl->Second(current)) == SymbolType))))
        aNames.insert(nl->SymbolValue(nl->First(current)));
      else if (((nl->IsAtom(nl->First(nl->Second(current))))
          && (nl->AtomType(nl->First(nl->Second(current))) == SymbolType))
          && (nl->SymbolValue(nl->First(nl->Second(current))) == "arel"))

      {
        aNames.insert(nl->SymbolValue(nl->First(current)));
        if (!insertArelNames ( aNames, nl->Second(current)))
          return false;
      }
    }
    else
      return false;
  }
  return true;
}

bool checkArelNames (set<string>& aNames, ListExpr b)
{
  ListExpr rest = nl->Second(nl->Second(b));
  ListExpr current;
  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType))
    {
      if ((nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
      {
        if(aNames.find(nl->SymbolValue(nl->First(current))) !=
         aNames.end())
           return false;
      }
      else if ((nl->IsAtom(nl->First(nl->Second(current))))
          && (nl->AtomType(nl->First(nl->Second(current))) == SymbolType)
          && (nl->SymbolValue(nl->First(nl->Second(current))) == "arel"))
      {
        if(aNames.find(nl->SymbolValue(nl->First(current))) !=
           aNames.end())
          return false;
        if(!checkArelNames(aNames, nl->Second(current)))
          return false;
      }
    }
    else
      return false;
  }
  return true;
}

bool AttributesAreDisjoint(ListExpr a, ListExpr b)
{
  set<string> aNames;
  ListExpr rest = a;
  ListExpr current;

  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2) && (nl->IsAtom(nl->First(current)))
           && (nl->AtomType(nl->First(current)) == SymbolType))
    {
      if ((nl->IsAtom(nl->Second(current)))
           && (nl->AtomType(nl->Second(current)) == SymbolType))
        aNames.insert(nl->SymbolValue(nl->First(current)));
      else if ((nl->IsAtom(nl->First(nl->Second(current))))
          && (nl->AtomType(nl->First(nl->Second(current))) == SymbolType)
          && (nl->SymbolValue(nl->First(nl->Second(current))) == "arel"))
      {
        aNames.insert(nl->SymbolValue(nl->First(current)));
        if (!insertArelNames ( aNames, nl->Second(current)))
          return false;
      }
    }
    else
      return false;
  }
  rest = b;
  while(!nl->IsEmpty(rest))
  {
    ListExpr current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType))
    {
      if ((nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
      {
        if(aNames.find(nl->SymbolValue(nl->First(current))) !=
         aNames.end())
           return false;
      }
      else if ((nl->IsAtom(nl->First(nl->Second(current))))
          && (nl->AtomType(nl->First(nl->Second(current))) == SymbolType)
          && (nl->SymbolValue(nl->First(nl->Second(current))) == "arel"))
      {
        if(aNames.find(nl->SymbolValue(nl->First(current))) !=
           aNames.end())
          return false;
        if(!checkArelNames(aNames, nl->Second(current)))
          return false;
      }
    }
    else
      return false;
  }
  return true;
}


/*
6.6 Function ~CompareNames~

*/
bool CompareNames(ListExpr list)
{
  vector<string> attrnamestrlist;
  vector<string>::iterator it;
  ListExpr attrnamelist;
  int unique;
  string attrname;

  attrnamelist = list;
  attrnamestrlist.resize(nl->ListLength(list));
  it = attrnamestrlist.begin();
  while (!nl->IsEmpty(attrnamelist))
  {
    attrname = nl->SymbolValue(nl->First(nl->First(attrnamelist)));
    attrnamelist = nl->Rest(attrnamelist);
    unique = std::count(attrnamestrlist.begin(),
                        attrnamestrlist.end(),
                        attrname);
    *it =  attrname;
    if (unique) return false;
    it++;
  }
  return true;
}

/*
6.7 Function ~IsTupleDescription~

Checks wether a ListExpression is of the form
((a1 t1) ... (ai ti)).

*/
bool IsTupleDescription( ListExpr a )
{
  ListExpr rest = a;
  ListExpr current;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ErrorInfo"));

  if(nl->AtomType(a)!=NoAtom){
     ErrorReporter::ReportError("Not a valid tuple type, must be a list.");
     return  false;
  }
  if(nl->IsEmpty(a)){
     ErrorReporter::ReportError("An empty attribute list is not allowed.");
     return false;
  }

  set<string> attrnames;
  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if(! (    (nl->ListLength(current) == 2)
           && (nl->IsAtom(nl->First(current)))
           && (nl->AtomType(nl->First(current)) == SymbolType)
           && am->CheckKind(Kind::DATA(),nl->Second(current),errorInfo))
           && attrnames.find(nl->SymbolValue(nl->First(current)))==
                 attrnames.end()) {
      if(nl->ListLength(current!=2)){
         ErrorReporter::ReportError("Attribut description must have length 2");
      }
      if(!nl->IsAtom(nl->First(current)) ||
         (nl->AtomType(nl->First(current)) != SymbolType)){
         ErrorReporter::ReportError("Attribute name must be a symbol.");
      }
      if(! am->CheckKind(Kind::DATA(),nl->Second(current),errorInfo)){
         ErrorReporter::ReportError("Attribute type is not of kind DATA.");
      }
      return false;
    } else {
      attrnames.insert(nl->SymbolValue(nl->First(current)));
    }
  }
  return true;
}

/*
6.7 Function ~IsRelDescription~

Checks wether a ListExpression is of the form
(rel (tuple ((a1 t1) ... (ai ti)))).

*/
bool IsRelDescription( ListExpr relDesc, bool trel /*=false*/ )
{
  if( nl->ListLength(relDesc) != 2 ){
    ErrorReporter::ReportError("Relation description must have length 2.");
    return false;
  }

  ListExpr relSymbol = nl->First(relDesc);
  ListExpr tupleDesc = nl->Second(relDesc);

  const string relStr = trel ? TempRelation::BasicType():Relation::BasicType();

  if( !nl->IsAtom(relSymbol) ||
      nl->AtomType(relSymbol) != SymbolType ||
      nl->SymbolValue(relSymbol) != relStr ){
    ErrorReporter::ReportError("Symbol '" + relStr + "' expected");
    return false;
  }

  if( nl->ListLength(tupleDesc) != 2 ){
    ErrorReporter::ReportError("Tuple description must have length 2.");
    return false;
  }

  ListExpr tupleSymbol = nl->First(tupleDesc);;
  ListExpr attrList = nl->Second(tupleDesc);

  if( !nl->IsAtom(tupleSymbol) ||
      nl->AtomType(tupleSymbol) != SymbolType ||
      nl->SymbolValue(tupleSymbol) != Tuple::BasicType() ){
    ErrorReporter::ReportError("The first element of a tuple description"
                               " must be the symbol 'tuple'.");
    return false;
  }

  if( !IsTupleDescription(attrList) )
    return false;

  return true;
}

/*
6.7 Function ~IsStreamDescription~

Checks wether a ListExpression is of the form
(stream (tuple ((a1 t1) ... (ai ti)))).

*/
bool IsStreamDescription( ListExpr streamDesc )
{
  if( nl->ListLength(streamDesc) != 2 ){
    ErrorReporter::ReportError("A stream description must have length 2.");
    return false;
  }

  ListExpr streamSymbol = nl->First(streamDesc);
  ListExpr tupleDesc = nl->Second(streamDesc);

  if( !nl->IsAtom(streamSymbol) ||
      nl->AtomType(streamSymbol) != SymbolType ||
      nl->SymbolValue(streamSymbol) != Symbol::STREAM() ){
    ErrorReporter::ReportError("Symbol 'stream' expected");
    return false;
  }

  if( nl->ListLength(tupleDesc) != 2 ){
    ErrorReporter::ReportError("Tuple description must have length 2.");
    return false;
 }

  ListExpr tupleSymbol = nl->First(tupleDesc);;
  ListExpr attrList = nl->Second(tupleDesc);

  if( !nl->IsAtom(tupleSymbol) ||
      nl->AtomType(tupleSymbol) != SymbolType ||
      nl->SymbolValue(tupleSymbol) != Tuple::BasicType() ){
    ErrorReporter::ReportError("The first element of a tuple description"
                               " must be the symbol 'tuple'.");
    return false;
  }

  if( !IsTupleDescription(attrList) )
    return false;

  return true;
}

/*
6.8 Function ~GetTupleResultType~

This function returns the tuple result type as a list expression
given the Supplier ~s~.

*/
ListExpr GetTupleResultType( Supplier s )
{
  ListExpr result = qp->GetType( s ),
           first = nl->First( result );

  switch( TypeOfRelAlgSymbol( first ) )
  {
    case ccmap:
    {
      result = nl->Third( result );
      break;
    }
    case stream:
    {
      // the result already corresponds to the stream.
      break;
    }
    default:
      assert( false );
  }

  first = nl->First( result );
  return SecondoSystem::GetCatalog()->NumericType( result );
}

/*
6.9 Function ~CompareSchemas~

This function receives two relation types and compare their schemas.
It returns true if they are equal, and false otherwise.

*/
bool CompareSchemas( ListExpr r1, ListExpr r2 )
{
  ListExpr attrList1 = nl->Second( nl->Second( r1 ) ),
           attrList2 = nl->Second( nl->Second( r2 ) );

  if( nl->ListLength( attrList1 ) != nl->ListLength( attrList2 ) )
    return false;

  for( int i = 0; i < nl->ListLength( attrList1 ); i++ )
  {
    ListExpr first1 = nl->First( attrList1 ),
             first2 = nl->First( attrList2 );
    attrList1 = nl->Rest( attrList1 );
    attrList2 = nl->Rest( attrList2 );

    if( nl->SymbolValue( nl->Second( first1 ) ) !=
        nl->SymbolValue( nl->Second( first2 ) ) )
      return false;
  }
  return true;
}

/*
7 Function returning the type name for ~temporal relation~

*/
const string TempRelation::BasicType() { return "trel"; }
