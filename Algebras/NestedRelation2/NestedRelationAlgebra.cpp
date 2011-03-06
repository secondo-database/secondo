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
but WITHOUT ANY WARRANTY; without even the implied warranty ofn
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Nested Relation Algebra

August 2009 Klaus Teufel  

[TOC]

1 Overview

The Nested Relation Algebra implements two type constructors,
namely ~nrel~ and ~arel~. nrel implements a nested relation, i.e.
a relation that can have subrelations as attributes. arel implements
an attribute relation, i.e. a relation that can be the attribute
of a nested relation. Both nrel and arel rely heavily on the
types and functions implemented by the Relation Algebra module.

2 Defines, includes, and constants

*/


#include "NestedRelationAlgebra.h"
#include "ListUtils.h"
#include "Progress.h"
#include "../../Tools/Flob/DbArray.h"
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>


using namespace std;


  









/*

3 Implementation of class AttributeRelation

3.1 Constructors and Destructor

*/
AttributeRelation::AttributeRelation( ListExpr typeInfo, bool nrel, 
                                                         int n):
 tupleIds( n ),
 arelType( typeInfo ),
 partOfNrel( nrel ),
 relDelete( false )
{
   if (!(nl->IsEmpty( typeInfo )))
      {
      if ((nl->ListLength( typeInfo ) == 3))
/*
In this case nrel has added the SmiFileId of the corresponding relation

*/
                                              
      {
         relId = nl->IntValue (nl -> Third( typeInfo )); 
         rel = Relation::GetRelation( relId );
      }
   }
}

AttributeRelation::AttributeRelation( const SmiFileId id, const ListExpr
                                      typeInfo, int n) :
   tupleIds( n ),
   arelType( typeInfo ),
   partOfNrel( true ),
   relDelete( false )
{
   setRelId(id);
}

AttributeRelation::~AttributeRelation() 
{
  if (!partOfNrel && relDelete)
  {
    relDelete = false;
    Relation* rel = Relation::GetRelation( getRelId() );
    if (!(rel == 0))
    {
      rel->Delete();
    }
  }                                      
}

/*
3.2 Auxiliary Functions

*/

ListExpr AttributeRelation::getArelType()
{
   return arelType;
}

void AttributeRelation::setPartOfNrel(bool b)
{
     partOfNrel = b;
     relDelete = !b;
}

bool AttributeRelation::isPartOfNrel()
{
     return partOfNrel;
}

const bool AttributeRelation::isEmpty() const
{
     return tupleIds.Size() == 0;
}

void AttributeRelation::Append(const TupleId& tupleId)
{
     tupleIds.Append(tupleId);
}

void AttributeRelation::setRelId(SmiFileId id)
{
     relId = id;
     rel = Relation::GetRelation( relId );
}

SmiFileId AttributeRelation::getRelId() const
{
   return relId;
}


DbArray<TupleId>* AttributeRelation::getTupleIds()
{
   return &tupleIds;
}

Relation* AttributeRelation::getRel()
{
   return rel;
}

void AttributeRelation::setRel(Relation* r)
{
   rel = r;
}

void AttributeRelation::Destroy()
{
  tupleIds.Destroy();
}

void AttributeRelation::CopyTuplesToRel(Relation* r)
{
  Relation* temp = Relation::GetRelation(relId);
  Tuple* t;
  TupleId tid;
  for (int i = 0; i < tupleIds.Size(); i++)  
    {
      tupleIds.Get(i, tid);  
      t = temp->GetTuple(tid,false);
      if(t){
        r->AppendTuple(t);
        t->DeleteIfAllowed();
      }
    }
}

void AttributeRelation::CopyTuplesToNrel(NestedRelation* nrel)
{
  Relation* temp = Relation::GetRelation(relId);
  Tuple* t;
  TupleId tid;
  for (int i = 0; i < tupleIds.Size(); i++)  
    {
      tupleIds.Get(i, tid);  
      t = temp->GetTuple(tid, false);
      if(t){
        nrel->AppendTuple(t);
        t->DeleteIfAllowed();
      }
    }
}

/*
3.3 Functions implementing virtual functions from class 
Attribute 

*/
int AttributeRelation::NumOfFLOBs() const
{
  return 1;
}

Flob *AttributeRelation::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &tupleIds;
}

int AttributeRelation::Compare(const Attribute* attr) const
{
  AttributeType atype;
  const Attribute* a1, *a2;
  int cmp;
  Relation* rel1 = Relation::GetRelation(relId);
  TupleType* tt = rel1->GetTupleType();
  const AttributeRelation* arel = static_cast
                                <const AttributeRelation*> (attr);
  Relation* rel2;
  if (relId == arel->getRelId())
    rel2 = rel1;
  else
    rel2 = Relation::GetRelation(arel->getRelId());
  const DbArray<TupleId>* arelTids = &(arel->tupleIds);
  Tuple *t1, *t2;
  TupleId tid1, tid2;
  int i = 0;  
  while (i < tupleIds.Size() && i < arelTids->Size())
  {
    tupleIds.Get(i, tid1);
    arelTids->Get(i, tid2);
    t2 = rel2->GetTuple(tid2,false);
    t1 = rel1->GetTuple(tid1,false);
    
    for (int j = 0; j < tt->GetNoAttributes(); j++)
    {
      a1 = t1->GetAttribute(j);
      a2 = t2->GetAttribute(j);
      atype = tt->GetAttributeType(j);
      cmp = a1->Compare(a2);
      if (cmp != 0)
      {
        t1->DeleteIfAllowed();
        t2->DeleteIfAllowed();
        return cmp;
      }
    }
    i++;
    t1->DeleteIfAllowed();
    t2->DeleteIfAllowed();
  }
  
  if (i < tupleIds.Size())
    return 1;
  if (i < arelTids->Size())
    return -1;    
  return 0;
}

bool AttributeRelation::Adjacent(const Attribute* attr) const
{
  return 0;
}

AttributeRelation *AttributeRelation::Clone() const
{
  AttributeRelation *arel = new AttributeRelation( this->arelType, 
                                  false, this->tupleIds.Size() );
  arel->setRelId(this->relId);
  TupleId tid;
  for (int i = 0; i < this->tupleIds.Size(); i++){
      this->tupleIds.Get(i, tid);
      arel->Append(tid);
  }
  return arel;          
}
 
bool AttributeRelation::IsDefined() const
{
  return true;
}

size_t AttributeRelation::Sizeof() const
{
  return sizeof( *this );
}
 
size_t AttributeRelation::HashValue() const
{
  size_t value = 0;
  AttributeType atype;
  const Attribute* a1;
  Relation* rel1 = Relation::GetRelation(relId);
  TupleType* tt = rel1->GetTupleType();
  tt->IncReference();
  Tuple *t1;
  TupleId tid1;
  if (tupleIds.Size() > 0)
  {
    tupleIds.Get(0, tid1);
    t1 = rel1->GetTuple(tid1,false);
    for (int j = 0; j < tt->GetNoAttributes(); j++)
    {
      a1 = (Attribute*)t1->GetAttribute(j);
      atype = tt->GetAttributeType(j);
      value = a1->HashValue();
    }
  t1->DeleteIfAllowed();
  }  
  tt->DeleteIfAllowed();    
  cout << "value: " << value;
  return value; 
}
  
void AttributeRelation::CopyFrom(const Attribute* right)
{
  AttributeRelation* arel = (AttributeRelation*) right;
  partOfNrel = false;
  relDelete = false;
  relId = arel->getRelId();
  DbArray<TupleId>* tids = arel->getTupleIds(); 
  TupleId tid;
  if (!isEmpty())
    tupleIds.clean();
  for (int i = 0; i < tids->Size(); i++)
  {
    tids->Get(i, tid);  
    Append(tid);
  }
}
    
/*
3.4 The mandatory set of algebra support functions

3.4.1 In-function 
 
*/ 
Word AttributeRelation::In(const ListExpr typeInfo, const ListExpr 
                    value, const int errorPos, ListExpr& errorInfo, 
                    bool& correct)
{
   Word result = SetWord( Address(0) );
   correct = true;
   AttributeRelation* arel = 0;
   ListExpr tuplelist, TupleTypeInfo, first;
   Relation* rel = 0;
   
   if (!(nl->ListLength( typeInfo ) == 3) ) //no file Id attached
   {
      int relAlgId = am->GetAlgebraId("RelationAlgebra");
      int relId = NestedRelation::getTypeId(relAlgId, "rel");
      ListExpr relType = nl->TwoElemList(nl->TwoElemList
          (nl->IntAtom(relAlgId), nl->IntAtom(relId)),
           nl->Second(typeInfo)); 
      rel = new Relation( relType );
      arel = new AttributeRelation (-1, typeInfo, 
                                  nl->ListLength(value));
      arel->setPartOfNrel(false);
   }
   else
   {
     arel = new AttributeRelation( typeInfo, true, nl->ListLength(value) );
     rel = arel->rel;
   }
   Tuple* tupleaddr;
   int tupleno, count;
   bool tupleCorrect;
   tuplelist = value;
   tupleno = 0;
   count = 0;
   
   TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
    nl->IntAtom(nl->ListLength(nl->Second(nl->Second
    (typeInfo)))));
    
   if (nl->IsAtom(value))
   {
      correct = false;
      errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
       nl->IntAtom(70),
       nl->SymbolAtom("arel"),
       tuplelist));
      if (!arel->isPartOfNrel()){
        rel->Delete();
      }
      delete arel;
      return result;
   }
   else
   { 
      first = nl->First(tuplelist);
      if (nl->IsAtom(first))
         tuplelist = nl->Rest(tuplelist); 
      while (!nl->IsEmpty(tuplelist))
      {
/*
Disregard a possibly existing Int-Atom which indicates whether to 
show tupleIds or tuple-values. 

*/
         first = nl->First(tuplelist);
         tuplelist = nl->Rest(tuplelist);
         tupleno++;
         tupleaddr = Tuple::In(TupleTypeInfo, first, tupleno,
                            errorInfo, tupleCorrect);

         if (tupleCorrect)
         {
            rel->AppendTuple(tupleaddr);
            arel->Append(tupleaddr->GetTupleId());
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
            nl->SymbolAtom("arel")));
         if (!arel->isPartOfNrel()){
           rel->Delete();
         }
         delete arel;
         return result;
      }
      else
      {
         if (!arel->isPartOfNrel()){
           rel->Delete();
         }
         result.addr = arel;    
         return result; 
      }
   }
}

/*
3.4.2 Out-function

*/
ListExpr AttributeRelation::Out( ListExpr typeInfo, Word value )
{
  AttributeRelation* arel = (AttributeRelation*)value.addr;
  DbArray<TupleId>* tids = arel->getTupleIds();
  TupleId tid;
  if (arel->isPartOfNrel())
  {
    Relation* rel = Relation::GetRelation(arel->getRelId());
    arel->setRel(rel); 
    Tuple* t=0;
    ListExpr l= nl->Cons(nl->IntAtom(0), nl->TheEmptyList());
    ListExpr lastElem=l, tlist=l;
    ListExpr tupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));  
    for (int i = 0; i < tids->Size(); i++)  
    {
      tids->Get(i, tid);  
      t = rel->GetTuple(tid, false);
      tlist = t->Out(tupleTypeInfo);
      lastElem = nl->Append(lastElem, tlist);
      t->DeleteIfAllowed();
    }
    return l;
  }
  else
  {
    NList outlist;
    outlist.append(nl->IntAtom(1));
    ListExpr val;
    for (int i = 0; i < tids->Size(); i++)
    {
      tids->Get(i, tid);
      val = nl->IntAtom(tid);
      outlist.append(NList(val));
    }
    return outlist.listExpr();          
  }
}

/*
3.4.3 Create-function

*/
Word AttributeRelation::Create(const ListExpr typeInfo)
{
  return ( SetWord(new AttributeRelation (typeInfo, false) ) );
}

/*
3.4.4 Delete-function

*/
void AttributeRelation::Delete( const ListExpr typeInfo, Word& w )
{
     AttributeRelation* arel = (AttributeRelation*) w.addr;
     arel->Destroy();
     arel->rel = 0;
     delete arel;
     w.addr = 0;
}

/*
3.4.5 Close- function

*/
void AttributeRelation::Close( const ListExpr typeInfo, Word& w )
{
     AttributeRelation* arel = static_cast
                                <AttributeRelation *> (w.addr); 
     arel->rel = 0;
     delete arel;
     w.addr = 0;
}               

/*
3.4.6 Open-function

*/
bool
AttributeRelation::Open( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  AttributeRelation *arel = (AttributeRelation*)Attribute::
                            Open( valueRecord, offset, typeInfo );
  value.setAddr( arel );
  return true;
}

/*
3.4.7 Save-function

*/
bool
AttributeRelation::Save( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  AttributeRelation *arel = (AttributeRelation *)value.addr;
  if (!arel->isPartOfNrel() && arel->relDelete)
  {
    arel->relDelete = false;
    Relation* rel = Relation::GetRelation( arel->getRelId() );
    if (!(rel == 0))
    {
      rel->Delete();
      rel = 0;
    }
  }
  Attribute::Save( valueRecord, offset, typeInfo, arel );
  return true; 
}

/*
3.4.8 Clone-function

*/
Word AttributeRelation::Clone(const ListExpr typeInfo, const Word& w)
{
  return SetWord( ((AttributeRelation*)w.addr)->Clone() );
}

/*
3.4.9 KindCheck

*/
bool
AttributeRelation::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  if ((nl->ListLength(type) == 2) &&
      nl->IsEqual(nl->First(type), "arel"))
  {
    return am->CheckKind("TUPLE", nl->Second(type), errorInfo);
  }
  else
  {
    return false;
  }
}

/*
3.4.10 SizeOf-function

*/
int AttributeRelation::SizeOfObj()
{
  return sizeof(AttributeRelation);
}

/*
3.4.11 Cast-function

*/
void* AttributeRelation::Cast(void* addr)
{
  return (new (addr) AttributeRelation);
}

/*
3.6 Type Description

*/
  
struct attributeRelationInfo : ConstructorInfo {

  attributeRelationInfo() {

    name         = "arel";
    signature    = "TUPLE ->  DATA";
    typeExample  = "arel";
    listRep      =  "arel(tuple([b: int]))";
    valueExample = "((2) (4) (232))";
  }
};

/*
3.7 Creation of the Type Constructor Instance

*/

struct attributeRelationFunctions : 
       ConstructorFunctions <AttributeRelation> 
{
  attributeRelationFunctions()
  {
    // re-assign function pointers
    in = AttributeRelation::In;
    out = AttributeRelation::Out; 
    create = AttributeRelation::Create;
    deletion = AttributeRelation::Delete; 
    close = AttributeRelation::Close;
    open = AttributeRelation::Open;
    save = AttributeRelation::Save;
    clone = AttributeRelation:: Clone;
    kindCheck = AttributeRelation:: KindCheck;
    sizeOf = AttributeRelation:: SizeOfObj;
    cast = AttributeRelation:: Cast;
  }  
};    
 
attributeRelationInfo ari;
attributeRelationFunctions arf;
TypeConstructor attributeRelationTC( ari, arf );



/*
4 Implementation of class NestedRelation

4.1 Constructors and Destructor

*/

NestedRelation::NestedRelation(ListExpr typeInfo) :
                                        subRels(0)
{
   
   int relAlgId = am->GetAlgebraId("RelationAlgebra");
   int relId = getTypeId(relAlgId, "rel");
   primaryTypeInfo = nl->TwoElemList(nl->TwoElemList(nl->IntAtom
                             (relAlgId), nl->IntAtom(relId)), 
                              nl->Second(typeInfo));
   primary = new Relation( primaryTypeInfo );
   insertSubRelations( nl->Second(nl->Second(typeInfo)));
   setTupleTypeInfo(typeInfo); 
}

NestedRelation::NestedRelation( ListExpr typeInfo, Relation* ptr, 
                                vector<SubRelation*>& sR ) :
  primary(ptr),
  subRels(sR)
{
   int relAlgId = am->GetAlgebraId("RelationAlgebra");
   int relId = getTypeId(relAlgId, "rel");
   ListExpr primaryInfo = nl->TwoElemList(nl->TwoElemList(nl->IntAtom
                               (relAlgId), nl->IntAtom(relId)), nl->
                                Second(typeInfo));
   primaryTypeInfo = primaryInfo;
   setTupleTypeInfo(typeInfo);
}

/*
4.2 Auxiliary functions

*/
void NestedRelation::insertSubRelations( ListExpr typeInfo )
{
   int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
   int relAlgId = am->GetAlgebraId("RelationAlgebra");
   int arelId, relId;
   arelId = getTypeId(nrelAlgId, "arel");
   relId = getTypeId(relAlgId, "rel");
   NList attrList, first, first1, first2;
   NList rest(typeInfo);
   while (!rest.isEmpty())
   { 
      first = rest.first();
      first1 = first.first();
      first2 = first.second();
      rest.rest();
      //check whether attribute is of type arel
      if (!first2.first().isAtom() && first2.first().hasLength(2) && 
        first2.first().first().isInt() && first2.first().first()
        .intval() == nrelAlgId && first2.first().second().isInt() && 
        first2.first().second().intval() == arelId)
      {
         //create a Subrelation and call insertSubRelation
         ListExpr subRelInfo = nl->TwoElemList(nl->TwoElemList
                             (nl->IntAtom(relAlgId), nl->IntAtom(relId)), 
                              nl->Second(first2.listExpr()));
         Relation *subRel = new Relation(subRelInfo);
         SubRelation *s = new SubRelation(subRel, first1.str(), 
                          subRel->GetFileId(), 
                          subRelInfo);
         append(s);
        
         insertSubRelations( first2.second().second().listExpr() );
      }
   }               
}


Relation* NestedRelation::getPrimary()
{
   return primary;
}

void NestedRelation::append (SubRelation* srel)
  {
       subRels.push_back(srel);
  }

void NestedRelation::setTupleTypeInfo( ListExpr typeInfo )
{
   int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
   int arelId = getTypeId(nrelAlgId, "arel");  
   
   NList tupleAttrList, first, first1, first2;
   NList type( typeInfo);
   NList attrList( type.second().second());
   while ( !attrList.isEmpty() )
   {
      first = attrList.first();
      first1 = first.first();
      first2 = first.second();
      attrList.rest();
      if (!first2.first().isAtom() && first2.first().hasLength(2) 
        && first2.first().first().isInt() && first2.first().
        first().intval() == nrelAlgId && first2.first().
        second().isInt() && first2.first().second().intval() 
        == arelId) 
      {
         ListExpr subRelInfo = getSubRelationInfo(first.listExpr());
         tupleAttrList.append(NList(subRelInfo));   
      }
      else
         tupleAttrList.append(first);
   }
   tupleTypeInfo = nl->TwoElemList(nl->TwoElemList
      (type.second().first().listExpr(), tupleAttrList.listExpr()), 
       nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
}

ListExpr NestedRelation::getSubRelationInfo( ListExpr typeInfo )
{
   int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
   int arelId = getTypeId(nrelAlgId, "arel");
   long fileId;
   
   NList attrList, arelAttrList, first, first1, first2;
   NList type(typeInfo);
   NList ident (type.first());
   NList arel ( type.second().first());
   NList tuple ( type.second().second().first());
   string name = type.first().str();
   attrList = type.second().second().second();
   while(!attrList.isEmpty())
   {
      first = attrList.first();
      first1 = first.first();
      first2 = first.second();
      attrList.rest();
      if (!first2.first().isAtom() && first2.first().hasLength(2) && 
        first2.first().first().isInt() && first2.first().first().intval() 
        == nrelAlgId && first2.first().second().isInt() && 
        first2.first().second().intval() == arelId) 
      {
         ListExpr subArelInfo = getSubRelationInfo(first.listExpr());
         arelAttrList.append(NList(subArelInfo));   
      }
      else
         arelAttrList.append(first);
   }
   fileId = getSubRel(name)->fileId;
   ListExpr temp = nl->TwoElemList(tuple.listExpr(), arelAttrList.listExpr());
   ListExpr temp1 = nl->ThreeElemList (arel.listExpr(), temp, 
                                       nl->IntAtom(fileId));
   ListExpr result = nl->TwoElemList(ident.listExpr(), temp1);
   return result;
} 

SubRelation* NestedRelation::getSubRel(string name)
{
   for ( unsigned int i = 0; i < subRels.size(); i++ )
   {
      SubRelation* srel = subRels[i];
      if ( srel->name == name )
         return srel; 
   }
   return 0;
}

int NestedRelation::getTypeId(int algId, string typeName)
{
    int result = -1;
    for (int i = 0; i < am->ConstrNumber(algId); i++){
        if (am->GetTC(algId, i)->Name() == typeName){
           result = i;
       }
    }
    return result;
}
 
bool NestedRelation::namesUnique(ListExpr type, string& s)
{
  vector<string> attrnamelist;
  ListExpr attrlist, pair;
  string attrname;
  int unique;
  attrlist = nl->Second(type);
  unique = 0;
  while (!nl->IsEmpty(attrlist))
  {
    pair = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    attrname = nl->SymbolValue(nl->First(pair));
    unique = std::count(attrnamelist.begin(),
                              attrnamelist.end(),
                              attrname);
    if (unique > 0)
    {
      s = attrname;
      return false;
    }
    attrnamelist.push_back(attrname);
  }
  return true;
}

ListExpr NestedRelation::unnestedList(ListExpr typeInfo)
{
  NList first, first2;
  NList type(typeInfo);
  NList rest(nl->Second(typeInfo));
  NList attributes;
  while(!rest.isEmpty())
  {
    first = rest.first();
    first2 = first.second();
    if (!(first2.isAtom())&& first2.first().isSymbol() && 
      first2.first().str() == "arel")
    {
      attributes.append(first);
      ListExpr temp = unnestedList(first.second().second().listExpr());
      NList tempRest(nl->Second(temp));
      while(!tempRest.isEmpty())
      {
        attributes.append(tempRest.first());
        tempRest.rest();
      }
    }  
    else
      attributes.append(first);
    rest.rest();
  }
  return nl->TwoElemList(nl->SymbolAtom("tuple"), attributes.listExpr());   
}

bool NestedRelation::saveString( string& s, SmiRecord& valueRecord, 
                                 size_t& offset )
{
   int size = s.size();
   bool ok = true;
   ok = ok && valueRecord.Write( &size, sizeof( int ), offset);
   offset += sizeof( int );
   for (int i = 0; i < size; i++)
   {
      ok = ok && valueRecord.Write( &s.at(i), sizeof( char ), offset);
      offset += sizeof( char );
   }
   
   return ok;
}

bool NestedRelation::readString( string& s, SmiRecord& valueRecord, 
                                 size_t& offset )
{
   int size;
   string temp;
   char c;
   bool ok = true;
   ok = ok && valueRecord.Read( &size, sizeof( int ), offset);
   offset += sizeof( int );
   for (int i = 0; i < size; i++)
   {
      ok = ok && valueRecord.Read( &c, sizeof( char ), offset);
      temp.push_back(c);
      offset += sizeof( char );
   }
   s = temp;
   return ok;
}

void NestedRelation::Delete()
{
   this->primary->Delete();
   this->primary = 0;
   for (unsigned int i = 0; i < this->subRels.size(); i++)
   {
      SubRelation* srel = this->subRels[i];
      srel->rel->Delete();
      srel->rel = 0;
      delete srel;
   }
}

NestedRelation *NestedRelation::Clone(ListExpr typeInfo)
{
   NestedRelation *nrel = new NestedRelation( typeInfo );
   Tuple* t;
   GenericRelationIterator *iter = primary->MakeScan();
   while( (t = iter->GetNextTuple()) != 0 )
      {
         nrel->AppendTuple( t );
         t->DeleteIfAllowed();
      }
   delete iter;
   return nrel;
}


vector<SubRelation*>* NestedRelation::getSubRels()
{
   return &subRels;
} 


AttributeRelation* NestedRelation::storeSubRel(AttributeRelation* a, int& i)
{
   int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
   int arelId = NestedRelation::getTypeId(nrelAlgId, "arel"); 
   Relation* relOld = Relation::GetRelation(a->getRelId());
   SmiFileId id = getSubRels()->at(i)->fileId; 
   Relation* r = getSubRels()->at(i)->rel;
   AttributeRelation* arel = new AttributeRelation (id, a->getArelType(),
                                                a->getTupleIds()->Size());
   arel->setRel(r);
   arel->setPartOfNrel(true);
   int j = i;
   DbArray<TupleId>* tids = a->getTupleIds();
   TupleId tid;
   for (int i1 = 0; i1 < tids->Size(); i1++)
   {   
      i = j;
      Tuple* t = 0;
      Tuple* newTuple;
      tids->Get(i1, tid);  
      t = relOld->GetTuple(tid, false);
      newTuple = new Tuple(t->GetTupleType());
      for (int i2 = 0; i2 < t->GetTupleType()->GetNoAttributes(); 
           i2++)
      {
         if (t->GetTupleType()->GetAttributeType(i2).algId ==  
         nrelAlgId && t->GetTupleType()->
         GetAttributeType(i2).typeId == arelId)
         {
            i = i + 1;
            AttributeRelation* ar = (AttributeRelation*)
                                   (t->GetAttribute(i2));
            AttributeRelation* b = storeSubRel(ar, i);
            newTuple->PutAttribute(i2, b);       
         }
         else
         {
              newTuple->CopyAttribute( i2, t, i2 );
         }
      }
      r->AppendTuple(newTuple);
      arel->Append(newTuple->GetTupleId());
      t->DeleteIfAllowed();
      newTuple->DeleteIfAllowed();
   }   
   return arel;          
}


void NestedRelation::AppendTuple(Tuple* tuple)
{
  int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
  int arelId = NestedRelation::getTypeId(nrelAlgId, "arel"); 
  int i = -1;
  Tuple* newTuple = new Tuple(tuple->GetTupleType());
  for (int i2 = 0; i2 < tuple->GetTupleType()->
        GetNoAttributes(); i2++)
  {
    if (tuple->GetTupleType()->GetAttributeType(i2).algId == 
        nrelAlgId && tuple->GetTupleType()->GetAttributeType(i2)
        .typeId == arelId)
    {
      i = i + 1;
      AttributeRelation* temp1 = (AttributeRelation*)
                                       (tuple->GetAttribute(i2));
      AttributeRelation* b = storeSubRel(temp1, i);
      newTuple->PutAttribute(i2, b);
    } 
    else
      {
        newTuple->CopyAttribute(i2, tuple, i2);
      }
  }
  primary->AppendTuple(newTuple);
  newTuple->DeleteIfAllowed();
}
                  
    
/*
4.3 The mandatory set of algebra support functions

4.3.1 In-function

*/

Word
NestedRelation::In( const ListExpr typeInfo, const ListExpr value,
                const int errorPos, ListExpr& errorInfo,  
                bool& correct )
{
   ListExpr tuplelist, first;
   Tuple* tupleaddr;
   int tupleno, count;
   bool tupleCorrect;
   Word result = SetWord( Address(0) );
   NestedRelation* nrel = new NestedRelation( typeInfo );
   tuplelist = value;
   tupleno = 0;
   count = 0;
   correct = true;
   if (nl->IsAtom(value))
   {
      correct = false;
      errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
       nl->IntAtom(70),
       nl->SymbolAtom("nrel"),
       tuplelist));
      nrel->Delete();
      delete nrel;
      return result;
   }
   else
   { 
      while (!nl->IsEmpty(tuplelist))
      {
         first = nl->First(tuplelist);
         tuplelist = nl->Rest(tuplelist);
         tupleno++;
         tupleaddr = Tuple::In(nrel->tupleTypeInfo, first, tupleno,
                            errorInfo, tupleCorrect);
         if (tupleCorrect)
         {
            nrel->primary->AppendTuple(tupleaddr);
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
            nl->SymbolAtom("nrel")));
         nrel->Delete();
         delete nrel;
         return result;
      }
      else
      {  
         result.addr = nrel;
         return result;
      }
   }
}

/*
4.3.2 Out-function

*/

ListExpr
NestedRelation::Out( ListExpr typeInfo, Word value )
{
  NestedRelation* nrel = (NestedRelation*)value.addr;
  Tuple* t=0;
  ListExpr l=nl->TheEmptyList();
  ListExpr lastElem=l, tlist=l, tupleTypeInfo=l;
  
  GenericRelationIterator* rit = nrel->primary->MakeScan();

  while ( (t = rit->GetNextTuple()) != 0 )
  {
    tupleTypeInfo = nrel->tupleTypeInfo;
    tlist = t->Out(tupleTypeInfo);
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
 
/*
4.3.3 Create-function

*/
Word
NestedRelation::Create( const ListExpr typeInfo )
{
  return (SetWord( new NestedRelation ( typeInfo ) ));
}

/*
4.3.4 Delete-function

*/
void NestedRelation::Delete( const ListExpr typeInfo, Word& w )
{
  
   NestedRelation* nR = static_cast<NestedRelation*>(w.addr);
   nR->Delete();
   delete nR;
   w.addr = 0;
}

/*
4.3.5 Close-function

*/
void NestedRelation::Close( const ListExpr typeInfo, Word& w )
{
   NestedRelation* nR = static_cast<NestedRelation*>(w.addr);
   nR->primary->Close();
   nR->primary = 0;
   for (unsigned int i = 0; i < nR->subRels.size(); i++)
   {
      SubRelation* srel = nR->subRels[i];
      srel->rel->Close();
      srel->rel = 0;
      delete srel;
   }
   delete nR;
}

/*
4.3.6 Clone-function

*/   
Word NestedRelation::Clone( const ListExpr typeInfo, const Word& w )
{
   return SetWord( ((NestedRelation*)w.addr)->Clone(typeInfo) );
}   

/*
4.3.7 Open-function

*/   
bool NestedRelation::Open( SmiRecord& valueRecord, size_t& offset, 
                           const ListExpr typeInfo, Word& value )
{
   int relAlgId = am->GetAlgebraId("RelationAlgebra");
   int relId = getTypeId(relAlgId, "rel");
   ListExpr primaryInfo = nl->TwoElemList(nl->TwoElemList
                        (nl->IntAtom(relAlgId), nl->IntAtom(relId)), 
                         nl->Second(typeInfo));
    
   bool ok = true;
   Relation* ptr = Relation::Open(valueRecord, offset, primaryInfo);
   vector<SubRelation*> srel;
   int vsize = 0;
   Relation* rel;
   SubRelation* subRel;
   string name;
   SmiFileId fileId;
   string subType;
   ListExpr subTypeInfo;
   ok = ok && valueRecord.Read (&vsize, sizeof( int ), offset);
   offset += sizeof( int );
   //set the vector srel
   for (int i = 0; i < vsize; i++)
   {
      ok = ok && NestedRelation::readString (subType, valueRecord, 
                                             offset);
      nl->ReadFromString(subType, subTypeInfo);
      rel = Relation::Open(valueRecord, offset, subTypeInfo);
      ok = ok && NestedRelation::readString (name, valueRecord, 
                                             offset);
      ok = ok && valueRecord.Read(&fileId, sizeof( SmiFileId ), 
                                  offset);
      offset += sizeof(SmiFileId);
      subRel = new SubRelation (rel, name, fileId, subTypeInfo);
      srel.push_back(subRel);
   }
   value.addr = new NestedRelation(typeInfo, ptr, srel);
   return ok;
      
}

/*
4.3.8 Save-function

*/   
bool NestedRelation::Save( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo, Word& value )
{
   int size;
   bool ok = true;
   NestedRelation* nr = static_cast<NestedRelation*>( value.addr );
   //Save primary relation
   ok = nr->primary->Save(valueRecord, offset, nr->primaryTypeInfo );
   //Save subrelations
   vector<SubRelation*>* sV = nr ->getSubRels();
   size = sV->size();
   ok = ok && valueRecord.Write(&size, sizeof(int), offset);
   offset += sizeof(int);
   string typeString;
  for ( unsigned int i = 0; i < sV->size(); i++)
  {
      SubRelation* sR = sV->at(i);
      nl->WriteToString(typeString, sR->typeInfo);
      ok = ok && NestedRelation::saveString(typeString, valueRecord,  
                                            offset);
      ListExpr typeInfo = sR->typeInfo;
      Relation* r = sR->rel;
      ok = ok && r->Save(valueRecord, offset, typeInfo);
      ok = ok && NestedRelation::saveString(sR->name, valueRecord, 
                                            offset );
      ok = ok && valueRecord.Write(&sR->fileId, sizeof(SmiFileId), 
                                   offset );
      offset += sizeof(SmiFileId);      
   }

   return ok;                     
}


/*
4.3.9 KindCheck

Incoming list should have the form nrel(tuple ((a1 t1)(a2 t2)...
         (a3 (arel(tuple(b1 t1)...))))...(ai ti))).

*/
bool
NestedRelation::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  bool correct;
  if ((nl->ListLength(type) == 2) &&
      nl->IsEqual(nl->First(type), "nrel"))
  {
    correct = am->CheckKind("TUPLE", nl->Second(type), errorInfo);
    if (!correct)
    {
      return false;
    }
    ListExpr temp = unnestedList(nl->Second(type));
    string s;
    correct = namesUnique(temp, s);
    if (!correct)
    {
      errorInfo = nl->Append(errorInfo,
             nl->FourElemList(
               nl->IntAtom(61),
               nl->SymbolAtom("TUPLE"),
               nl->IntAtom(3),
               nl->SymbolAtom(s)));
      return false;
    }
    else
      return correct;             
  }
  else
  {
    return false;
  }
}

/*
4.4 Type Description

*/
struct nestedRelationInfo : ConstructorInfo {

  nestedRelationInfo() {

    name         = "nrel";
    signature    = "TUPLE -> NREL";
    typeExample  = "nrel";
    listRep      =  "nrel(tuple([a: int]))";
    valueExample = "((1)(2))";
    remarks      = "Tuples can have arel as attributes";
  }
};

struct nestedRelationFunctions : 
       ConstructorFunctions<NestedRelation> 
{
  nestedRelationFunctions()
  {
    // re-assign function pointers
    in = NestedRelation::In;
    out = NestedRelation::Out; 
    create = NestedRelation::Create;
    deletion = NestedRelation::Delete; 
    close = NestedRelation::Close;
    open = NestedRelation::Open;
    save = NestedRelation::Save;
    clone = NestedRelation:: Clone;
    kindCheck = NestedRelation::KindCheck;
  }  
};    
 
nestedRelationInfo nri;
nestedRelationFunctions nrf;
TypeConstructor nestedRelationTC( nri, nrf );


/*
5 Operators

5.1 Operator ~feed~ for nrel. 

Creates a stream of tuples from nrel:

----    (nrel x)                -> (stream  x) 
----

5.1.1 Type Map for ~feed~

*/
ListExpr feedTypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 1,
    "Operator feed expects a list of length one.");

  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND(
    nl->ListLength(first) == 2 && ( nl->IsAtom(nl->First(first)) && 
    nl->AtomType(nl->First(first)) == SymbolType &&
    nl->SymbolValue(nl->First(first)) == "nrel" )
    && ( !(nl->IsAtom(nl->Second(first)) || nl->IsEmpty
    (nl->Second(first)))&& (TypeOfRelAlgSymbol(nl->First
    (nl->Second(first))) == tuple) ),
  "Operator feed expects an argument of type nested relation, "
  "(nrel(tuple((a1 t1)...(an tn)))).\n"
  "Operator feed gets an argument of type '" + argstr + "'."
  " Nested relation name not known in the database ?");

  return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
}

/*
5.1.2 Value mapping function of operator ~feed~

*/
int
feed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  NestedRelation* nr;
  Relation* r;
  GenericRelationIterator* rit;
  switch (message)
  {
    case OPEN :
      nr = (NestedRelation*)args[0].addr;
      r = nr->getPrimary();
      rit = r->MakeScan();
      local.addr = rit;
      return 0;

    case REQUEST :
      rit = (GenericRelationIterator*)local.addr;
      Tuple *t;
      if ((t = rit->GetNextTuple()) != 0)
      {  
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      if(local.addr)
      {
         rit = (GenericRelationIterator*)local.addr;
         delete rit;
         local.addr = 0;
      }
      return 0;
  }
  return 0;
}

/*
5.1.3 Specification of operator ~feed~

*/
struct feedInfo : OperatorInfo {

  feedInfo() : OperatorInfo()
  {
    name =      "feed";
    signature = "(nrel x) -> (stream x)";
    syntax =    "_ feed";
    meaning =   "Produces a stream from a nested relation by "
                "scanning the nested relation tuple by tuple.";
    example =   "query authors feed consume";
  }
};

/*
5.2 Operator ~consume~

Collects objects from a stream into a nested relation.

5.2.1 Type mapping function of operator ~consume~

Operator ~consume~ accepts a stream of tuples and returns 
a nested relation.

----    (stream  x)                 -> ( nrel x)
----

*/
ListExpr consumeTypeMap(ListExpr args)
{
  ListExpr first, tup, tupFirst, type ;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 1,
  "Operator consume expects a list of length one.");

  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND(
    ((nl->ListLength(first) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream)) &&
    (!(nl->IsAtom(nl->Second(first)) ||
       nl->IsEmpty(nl->Second(first))) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)),
  "Operator consume expects an argument of type (stream(tuple"
  "((a1 t1)...(an tn)))).\n"
  "Operator consume gets an argument of type '" + argstr + "'.");
  
  tup = nl->Second(nl->Second(first));
  bool containsArel = false;
  while (!(nl->IsEmpty(tup)) && !containsArel)
  {
    tupFirst = nl->First(tup);
    type = nl->Second(tupFirst);
    if (!(nl->IsAtom(type)))
    {
      type = nl->First(type); 
      if (nl->IsAtom(type))
        if(nl->SymbolValue(type) == "arel")
          containsArel = true;
    }
    tup = nl->Rest(tup);
  }
  CHECK_COND(containsArel, "Operator consume of nrel expects nested tuples as"
                           " arguments.");
  
  ListExpr temp = NestedRelation::unnestedList(nl->Second(first));
  string s;
  CHECK_COND(NestedRelation::namesUnique(temp, s), 
              "The attributename '" + s + "' in the incoming "
              "stream is not unique.\n" 
              "Please make sure that there are no duplicates.");
  return nl->Cons(nl->SymbolAtom("nrel"), nl->Rest(first));
}

/*
5.2.2 Value mapping function of operator ~consume~

*/
int
consume(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  
  Word actual;
  NestedRelation* nrel = (NestedRelation*)(qp->ResultStorage(s).addr);
  Relation* rel = nrel->getPrimary();
  if(rel->GetNoTuples() > 0)
  {
    rel->Clear();
  }
  vector<SubRelation*>* srels = nrel->getSubRels();
  for (unsigned int k = 0; k < srels->size(); k++)
  {
    if(srels->at(k)->rel->GetNoTuples() > 0) 
    {
      srels->at(k)->rel->Clear();
    } 
  }
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  Tuple* tuple;
  while (qp->Received(args[0].addr))
  {     
     tuple = (Tuple*)actual.addr;
     nrel->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
     qp->Request(args[0].addr, actual); 
  }
  result.setAddr(nrel);
  qp->Close(args[0].addr);
  return 0;
}

/*
5.2.3 Specification of operator ~consume~

*/
struct consumeInfo : OperatorInfo {

  consumeInfo() : OperatorInfo()
  {
    name =      "consume";
    signature = "(stream x) -> (nrel x)";
    syntax =    "_ consume";
    meaning =   "Collects objects from a stream into a nested relation.";
    example =   "query books feed consume";
  }

};

/*
5.3 Operator ~afeed~ 

5.3.1 Type mapping function of operator ~afeed~

Operator afeed creates a stream of tuples out of arel: 

----    (arel x)                 -> ( stream x)
----

*/
ListExpr aFeedTypeMap(ListExpr args)
{
  ListExpr first;
  string argstr;

  CHECK_COND(nl->ListLength(args) == 1,
    "Operator afeed expects a list of length one.");

  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND(
    nl->ListLength(first) == 2
      && ( nl->IsAtom(nl->First(first)) && nl->
      AtomType(nl->First(first)) == SymbolType && 
      nl->SymbolValue(nl->First(first)) == "arel" )
      && ( !(nl->IsAtom(nl->Second(first)) || 
      nl->IsEmpty(nl->Second(first)))
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) ),
  "Operator afeed expects an argument of type attribute relation, "
  "(arel(tuple((a1 t1)...(an tn)))).\n"
  "Operator afeed gets an argument of type '" + argstr + "'.");

  return nl->Cons(nl->SymbolAtom("stream"),nl->Rest(first));
}

/*
5.3.2 Value mapping function of operator ~afeed~

*/
struct AfInfo 
{
  AfInfo (int i) : index(i){}
  int index;
};

int
aFeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  AfInfo* info;
  AttributeRelation* arel;
  Relation* r;
  Tuple* t;
  switch (message)
  {
    case OPEN :
      info = new AfInfo(0);
      local.addr = info;
      return 0;

    case REQUEST :
      info = (AfInfo*)local.addr;
      arel = (AttributeRelation*) args[0].addr;
      if (!arel->isPartOfNrel() && (int)arel->getRelId() == -1)
      {
        delete info;
        local.setAddr(0);
        cout << endl << "WARNING: arel is not an attribute "
                        "of a nested relation. Result of operation" 
                        " will be empty" << endl << endl;
        return CANCEL;
      }                         
      r = Relation::GetRelation( arel->getRelId() );
      TupleId tid;
      if (info->index < arel->getTupleIds()->Size())
      { 
        arel->getTupleIds()->Get(info->index, tid); 
        t = r->GetTuple(tid, false);
        info->index++;
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
      return 0;

    case CLOSE :
      if(local.addr)
      {
         info = (AfInfo*)local.addr;
         delete info;
         local.addr = 0;
      }
      return 0;
  }
  return 0;
}

/*
5.3.3 Specification of operator ~afeed~

*/
struct aFeedInfo : OperatorInfo {

  aFeedInfo() : OperatorInfo()
  {
    name =      "afeed";
    signature = "(arel x) -> (stream x)";
    syntax =    "_ afeed";
    meaning =   "Produces a stream from an attribute relation.";
    example =   "query authors feed extend [no_papers: .papers afeed" 
                "count] consume";
  }
};

/*
5.4 Operator ~aconsume~

5.4.1 Type mapping function of operator ~aconsume~

Operator aconsume creates an attribute relation out of a stream of tuples: 

----    (stream x)          -> (arel x)
----

*/
ListExpr aConsumeTypeMap(ListExpr args)
{
  ListExpr first ;
  string argstr;
  
  CHECK_COND(nl->ListLength(args) == 1,
  "Operator aconsume expects a list of length one.");

  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND(
    ((nl->ListLength(first) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream)) &&
    (!(nl->IsAtom(nl->Second(first)) ||
       nl->IsEmpty(nl->Second(first))) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)),
  "Operator aconsume expects an argument of type (stream(tuple"
  "((a1 t1)...(an tn)))).\n"
  "Operator aconsume gets an argument of type '" + argstr + "'.");
 
  ListExpr temp = NestedRelation::unnestedList(nl->Second(first));
  string s;
  CHECK_COND(NestedRelation::namesUnique(temp, s), 
              "The attributename '" + s
              + "' in the incoming stream is not unique.\n" 
              "Please make sure that there are no duplicates.");   
  return nl->Cons(nl->SymbolAtom("arel"), nl->Rest(first));
}

/*
5.4.2 Value mapping function of operator ~aconsume~

*/
struct AconInfo 
{
  Relation* rel;
  bool set;
};

int
aConsume(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  Relation* rel; 
  AconInfo* acon;
  switch (message)
  {
    case REQUESTPROGRESS :
    {
      return CANCEL;      //no progress available
    }

    case CLOSEPROGRESS :
    {
      if (local.addr)
      {
        acon = (AconInfo*) local.addr;
        rel = acon->rel;
        if (rel)
          rel->Delete();
        delete acon;
        local.setAddr(0);
      }
      return 0;
    }
  }
  
  Word actual;
  AttributeRelation* arel = (AttributeRelation*)
                              (qp->ResultStorage(s).addr);
  arel->getTupleIds()->clean();
  int relAlgId = am->GetAlgebraId("RelationAlgebra");
  int relId = NestedRelation::getTypeId(relAlgId, "rel");
  if (!local.addr)
  {
    ListExpr relType = nl->TwoElemList(nl->TwoElemList(
                       nl->IntAtom(relAlgId), nl->IntAtom(relId)),
                       nl->Second(arel->getArelType())); 
    acon = new AconInfo;
    acon->set = true;
    rel = new Relation(relType);
    acon->rel = rel;
    arel->setRel(rel);
    arel->setRelId(rel->GetFileId());
    local.addr = acon;
  }
  else 
  {
    acon = (AconInfo*)local.addr;
    if (acon->set)
    {
      rel = acon->rel;
      arel->setRelId(rel->GetFileId());
    }
    else
    {
      delete acon;
      acon = new AconInfo;
      acon->set = true;
      ListExpr relType = nl->TwoElemList(nl->TwoElemList(nl->IntAtom
                         (relAlgId), nl->IntAtom(relId)),
                           nl->Second(arel->getArelType())); 
      rel = new Relation(relType);
      acon->rel = rel;
      arel->setRel(rel);
      arel->setRelId(rel->GetFileId());
      local.addr = acon;
    }
  }     
  Tuple* tuple;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    tuple = (Tuple*)actual.addr;
    rel->AppendTuple(tuple);
    arel->Append(tuple->GetTupleId());
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, actual); 
  }
  result.setAddr(arel);
  qp->Close(args[0].addr);
  return 0;
}

/*
5.4.3 Specification of operator ~aconsume~

*/
struct aConsumeInfo : OperatorInfo {

  aConsumeInfo() : OperatorInfo()
  {
    name =      "aconsume";
    signature = "(stream x) -> (arel x)";
    syntax =    "_ aconsume";
    meaning =   "Collects objects from a stream.";
    example =   "query papers feed extend [authors_new: .authors afeed "
                "aconsume] consume";
  }
};

Operator aconsume (aConsumeInfo(), aConsume, aConsumeTypeMap);

/*
5.5 Operator ~nest~

This operator accepts a stream of  tuples and transfers all attributes not 
mentioned in the first argument into a subrelation (of type arel), 
the name of which is specified by the second argument.

5.5.1 Type mapping function of operator ~nest~

Type mapping for ~nest~ is

----     ((stream (tuple ((x1 t1)...(xn tn)))) ((xi1) ... (xij)) (xo))
              -> (stream (tuple ((xi1 ti1)...(xij tij) 
                 (xo arel(tuple((xik tik)...(xin tin))))) 
                 APPEND (j n-j (i1 i2 ... ij) (ik ... in))
----

*/
ListExpr nestTypeMap( ListExpr args )
{
   
   NList argList(args);
   NList first, second, third, rest,
           lastlistn, first1, first2, second2, firstr,
           primary, subreltuple, numberlist1, numberlist2;
  
   ListExpr errorInfo, attrtype, outlist;
   bool allArel = true;  
   errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
   string argstr, argstr2, attrname, type;
   CHECK_COND(nl->ListLength(args) == 3,
    "Operator nest expects a list of length three.");
   first = argList.first();
   second = argList.second();
   third = argList.third();
   CHECK_COND(third.isSymbol(), 
   "Operator nest expects a valid attribute name as third argument."); 
  
   nl->WriteToString(argstr, first.listExpr());
   CHECK_COND(nl->ListLength(first.listExpr()) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first.listExpr())) 
             == stream) && (nl->ListLength(nl->Second
             (first.listExpr())) == 2) &&(TypeOfRelAlgSymbol
             (nl->First(nl->Second(first.listExpr()))) 
             == tuple) &&(nl->ListLength(nl->Second
             (first.listExpr())) == 2) && (IsTupleDescription
             (nl->Second(nl->Second(first.listExpr())))),
     "Operator nest expects as first argument a list with structure "
     "(stream (tuple ((a1 t1)...(an tn))))\n"
     "Operator nest gets as first argument '" + argstr + "'." );

   string arelName = third.str();
   NList unnested(nl->Second(NestedRelation::unnestedList(
                        first.second().listExpr())));
   bool unique = true;
   while (!(unnested.isEmpty()) && unique)
   {
     if (arelName == unnested.first().first().str())
       unique = false;
     unnested.rest();
   }
   CHECK_COND(unique, "The name for the new arel-Attribute " 
                      + arelName + " is already assigned to another "
                      "attribute. Please choose a new name and try again.");
   CHECK_COND((nl->ListLength(second.listExpr()) > 0), 
   "Operator nest: Second argument list may not be empty" );
  
/*
check that all attributes named in second argument appear in the first 
argument, collect attributes of second argument in primary.

*/ 
   int j;
   rest = second;
   set<string> attrNames;
   while (!(rest.isEmpty()))
   {
      first2 = rest.first();
      rest.rest();
      if (first2.isSymbol())
      {
         attrname = first2.str();
      }
      else
      {
         ErrorReporter::ReportError(
         "Attributename in the list is not of symbol type.");
         return nl->SymbolAtom("typeerror");
      }
       if(attrNames.find(attrname)!= attrNames.end()){
          ErrorReporter::ReportError("names within the nest "
                                  "list are not unique");
          return nl->TypeError();
      } else {
          attrNames.insert(attrname);
      }
      j = FindAttribute(first.second().second().listExpr(),
                      attrname, attrtype); 
      
      if (j)
      {
         primary.append(NList(first2, NList(attrtype)));
         nl->WriteToString( type, attrtype);
         
         numberlist1.append(NList(nl->IntAtom(j)));
         if (!(type == "arel"))
            allArel = false;
      }
      else
      {
         ErrorReporter::ReportError(
          "Operator nest: Attributename '" + attrname +
          "' is not a known attributename in the tuple stream.");
         return nl->SymbolAtom("typeerror");
      }
   }
   if (allArel)
   {
      ErrorReporter::ReportError(
          "Operator nest: There must be at least one attribute other than "
          "arel in the first argument list.");
         return nl->SymbolAtom("typeerror");
   }
   CHECK_COND (!(primary.length() == first.second().second().length()),
     "Operator nest: there must be at least one attribute that should be "
     "nested in a subrelation.");    

/*
check, if attributes in first argument exist in primary. If not append to subrel.

*/
   rest = first.second().second();
   int i = 1;
   while (!(rest.isEmpty()))
   {
      first1 = rest.first();
  
      attrname = first1.first().str();
      rest.rest();
      j = FindAttribute (primary.listExpr(), attrname, attrtype);
      if (!j)
      {
         subreltuple.append(first1);
         numberlist2.append(nl->IntAtom(i));
      }
      i++;
   }
   //create typeinfo for the resulting nested relation
   ListExpr temp1 = nl -> TwoElemList( nl -> SymbolAtom( "arel"), 
    nl -> TwoElemList ( nl -> SymbolAtom("tuple"), subreltuple.listExpr()));
   ListExpr temp2 = nl -> TwoElemList(nl -> SymbolAtom (third.str()), temp1);
   NList subrel(temp2);
   primary.append(subrel);
   ListExpr numbers = nl -> FourElemList (nl -> IntAtom(numberlist1.length()), 
     nl -> IntAtom(numberlist2.length()), numberlist1.listExpr(), 
     numberlist2.listExpr());
   ListExpr streamdescription = nl -> TwoElemList (nl->SymbolAtom( "stream" ),
      nl -> TwoElemList ( nl -> SymbolAtom( "tuple" ), primary.listExpr()));
   outlist = nl -> ThreeElemList (nl -> SymbolAtom ("APPEND"), numbers, 
     streamdescription);   
   return outlist;
}

/*
5.5.2 Auxiliary struct for function ~nest~

*/
struct NestInfo
{
  bool endOfStream;
  SmiFileId fileId;
  Tuple* lastTuple;
  Relation* subrel;
  int primaryLength;
  int subrelLength;
  bool firstCall;
  TupleType* subTupleType;
  TupleType* tupleType;
  AttributeRelation* arel;
  ListExpr relInfo;
  ListExpr arelInfo;
};

/*
5.5.3 Value mapping function of operator ~nest~

*/
int
nestValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s)
{ 
   NestInfo* info;
   Supplier son;
   Word elem1, elem2;
   Tuple* current;
   int index;
   Tuple* tuple;
   Tuple* subtuple;
   switch (message)
   {
     case OPEN :
     {
       info = (NestInfo*)local.addr;
       if (!info) 
       {
         info = new NestInfo();
         ListExpr resultType = GetTupleResultType( s );
         info->relInfo = resultType;
         info->tupleType = new TupleType(nl->Second(info->relInfo));
         info->primaryLength = ((CcInt*)args[3].addr)->GetIntval();
         //search for arel type information
         NList rest = NList(nl->Second(nl->Second(resultType)));
         for (int i = 0; i < info->primaryLength; i++)
           rest.rest();
         info->arelInfo = rest.first().second().listExpr();
         info->subTupleType = new TupleType(
                                rest.first().second().second().listExpr());
         info->subrelLength =  ((CcInt*)args[4].addr)->GetIntval();  
         
         info->subrel = new Relation(info->subTupleType);
         info->fileId = info->subrel->GetFileId();
         local.addr = info;
       }
       info->firstCall = true;
       info->endOfStream = false;
       qp->Open(args[0].addr);
       return 0;
     }  
     case REQUEST :
     {       
       info = (NestInfo*)local.addr;
       if (!(info->endOfStream))
       {
         bool equal = true;
         if ((info->firstCall))  
         {  
           qp->Request(args[0].addr, elem1);
           if (qp->Received(args[0].addr))
           {
             current = (Tuple*)elem1.addr;
             info->lastTuple =(Tuple*)elem1.addr;
             info->firstCall = false;
             info->arel = new AttributeRelation(info->fileId, info->arelInfo);
             tuple = new Tuple( info->tupleType );
             subtuple = new Tuple (info->subTupleType);  
           }
           else
           {
             return CANCEL;
           }
         }    
         else
         {
           info->arel = new AttributeRelation(info->fileId, info->arelInfo);
           current = info->lastTuple; 
           tuple = new Tuple( info->tupleType );
           subtuple = new Tuple (info->subTupleType);      
         }
         assert( tuple->GetNoAttributes() == info->primaryLength + 1);
         assert( subtuple->GetNoAttributes() == info->subrelLength);
         for (int i = 0; i < info->primaryLength; i++)
         {
           son = qp->GetSupplier(args[5].addr, i);
           qp->Request(son, elem2);
           index = ((CcInt*)elem2.addr)->GetIntval(); 
           tuple->CopyAttribute(index-1, current, i);
         }
         for (int i = 0; i < info->subrelLength; i++)
         {
           son = qp->GetSupplier(args[6].addr, i);
           qp->Request(son, elem2);
           index = ((CcInt*)elem2.addr)->GetIntval();
           subtuple->CopyAttribute(index-1, current, i);
         }
         info->subrel->AppendTuple(subtuple);
         info->arel->Append(subtuple->GetTupleId());
         subtuple->DeleteIfAllowed();
         tuple->PutAttribute(info->primaryLength, info->arel);
         qp->Request(args[0].addr, elem1);
         if (qp->Received(args[0].addr))
         {
           current = (Tuple*)elem1.addr;
           Attribute* attr1;
           Attribute* attr2;
           for (int i = 0; i < info->primaryLength; i++)
           {
             son = qp->GetSupplier(args[5].addr, i);
             qp->Request(son, elem2);
             index = ((CcInt*)elem2.addr)->GetIntval();     
             attr1 = info->lastTuple->GetAttribute(index - 1);
             attr2 = current->GetAttribute(index - 1);
             if (!(attr1->Compare(attr2) == 0))
             { 
               equal = false;
               break;
             }
           }
           while(equal)
           {
             subtuple = new Tuple (info->subTupleType);
             for (int i = 0; i < info->subrelLength; i++)
             {
               son = qp->GetSupplier(args[6].addr, i);
               qp->Request(son, elem2);
               index = ((CcInt*)elem2.addr)->GetIntval();
               subtuple->CopyAttribute(index-1, current, i);
             }  
             info->subrel->AppendTuple(subtuple);
             info->arel->Append(subtuple->GetTupleId());
             subtuple->DeleteIfAllowed();
             current->DeleteIfAllowed();
             current = 0;
             qp->Request(args[0].addr, elem1);
             if (qp->Received(args[0].addr))
             {
               current = (Tuple*)elem1.addr;
               Attribute* attr1;
               Attribute* attr2;
               for (int i = 0; i < info->primaryLength; i++)
               {
                 son = qp->GetSupplier(args[5].addr, i);
                 qp->Request(son, elem2);
                 index = ((CcInt*)elem2.addr)->GetIntval();     
                 attr1 = info->lastTuple->GetAttribute(index - 1);
                 attr2 = current->GetAttribute(index - 1);
                 if (!(attr1->Compare(attr2) == 0))
                 { 
                   equal = false;
                 }
               }
             }
             else
             {
                info->endOfStream = true;
                result.setAddr(tuple);
                return YIELD;
             }
           }
           info->lastTuple->DeleteIfAllowed();
           info->lastTuple = current;
           info->arel->getTupleIds()->TrimToSize();
           result.setAddr(tuple);
           return YIELD;
         }
         else
         {
           info->endOfStream = true;
           result.setAddr(tuple);
           return YIELD;
         }
       }
       else
       {   
         return CANCEL;  
       }
     }    
     case CLOSE :
     {
       if(local.addr)
       {
         info = (NestInfo*)local.addr;
         if (info->lastTuple)
           info->lastTuple->DeleteIfAllowed();
       }
       qp->Close(args[0].addr);
       return 0;
     }
     case CLOSEPROGRESS :
     {
       if(local.addr)
       {
         info = (NestInfo*)local.addr;
         info->tupleType->DeleteIfAllowed();
         info->subTupleType->DeleteIfAllowed();
         info->subrel->Delete();
         delete info;
         local.setAddr(0);
         return 0;
       }
     }
     case REQUESTPROGRESS :
     {
       return CANCEL;
     }                              
   }
return 0;  
}

/*
5.5.4 Specification of operator ~nest~

*/
struct nestInfo : OperatorInfo {

  nestInfo() : OperatorInfo()
  {
    name =      "nest";
    signature = "(stream x) -> (stream y)";
    syntax =    "_ nest [xi1,..., xij; x0]";
    meaning =   "Creates a nested tuple stream from a tuple stream. The "
                "stream should be sorted by the attributes that are "
                "to appear in the primary relation." ;
    example =   "query documents feed sortby [publisher] nest[publisher; "
                "publications]";
  }
};

Operator nest (nestInfo(), nestValueMap, nestTypeMap);

/*
5.6 Operator ~unnest~

5.6.1 Type mapping function of operator ~unnest~


----  ((stream (tuple ((x1 T1) ... (xn Tn)))) xj)  ->
        (APPEND
          (j n o)
          (stream (tuple ((x1 T1) ... (xi Ti)(xk Tk)...(xn Tn)(a1 t1)...
          (ao to))))
        )
----
Where xj: arel(tuple([a1: t1,...,ao: to]))

*/
ListExpr unnestTypeMap(ListExpr args)
{
  int j = 0;
  // initialize local ListExpr variables
  ListExpr first=nl->TheEmptyList();
  ListExpr second, arelAttr, tuple,
           attrtype, attr;
  ListExpr outlist, numbers;
  string attrname="", argstr="";
  int numberAttr, noAttrArel;

  CHECK_COND(nl->ListLength(args) == 2,
    "Operator unnest expects a list of length two.");

  first = nl->First(args);
  second = nl->Second(args);

  nl->WriteToString(argstr, first);
  CHECK_COND(
    nl->ListLength(first) == 2 &&
    TypeOfRelAlgSymbol(nl->First(first)) == stream &&
    nl->ListLength(nl->Second(first)) == 2 &&
    nl->SymbolValue(nl->First(nl->Second(first))) == "tuple" &&
    IsTupleDescription(nl->Second(nl->Second(first))),
    "Operator unnest expects a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator unnest gets a list with structure '" + argstr + "'.");

  nl->WriteToString(argstr, second);
  NList temp (second);
  CHECK_COND(
    nl->IsAtom(second) && temp.isSymbol(),
    "Operator unnest expects one attribute name as argument "
    "Operator unnest gets '" + argstr + "' as argument.");
  
  attrname = nl->SymbolValue(second);  
  attr = nl->Second(nl->Second(first));
  numberAttr = nl->ListLength(attr);
  j = FindAttribute(attr, attrname, attrtype);
  if (j)
  {
    nl->WriteToString(argstr, attrtype);
  
    CHECK_COND(
      (nl->ListLength(attrtype)==2) && 
      (nl->SymbolValue(nl->First(attrtype)) == "arel"),
      "Operator unnest expects an attribute of type arel as argument "
      "Operator unnest gets an attribute of type '" + argstr + 
      "' as argument.");
  }
  else
  {
     ErrorReporter::ReportError("Operator unnest: Attributename '" + attrname 
               + "' is not a known attributename in the tuple stream.");
       return nl->TypeError();
  }
  arelAttr = nl->Second(nl->Second(attrtype));
  NList temp1;
  NList rest(attr);
  for (int i = 0; i < numberAttr; i++)
  {
    if (!(i == (j - 1)))
      temp1.append(rest.first());
    rest.rest();
  }
  noAttrArel = (nl->ListLength(arelAttr));
  rest = NList(arelAttr);
  for (int i = 0; i < noAttrArel; i++)
  {
    temp1.append(rest.first());
    rest.rest();
  } 
  tuple = nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          temp1.listExpr());
  
  
  numbers = nl->ThreeElemList(nl->IntAtom(j), nl->IntAtom(numberAttr), 
          nl->IntAtom(noAttrArel));
  outlist =
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
        numbers,
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        tuple));
  return outlist;            
}

struct UnnestInfo
{
  int index;
  Tuple* lastTuple;
  AttributeRelation* arel;
  TupleType *tupleType;
};

/*
5.6.2 Value mapping for ~unnest~

*/ 
int
unnestValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  UnnestInfo* info;
  DbArray<TupleId>* tidArray;
  Relation* rel;
  switch (message)
  {
    case OPEN :
    {
      info = new UnnestInfo;
      info->index = -1;
      ListExpr resultType = GetTupleResultType( s );
      info->tupleType = new TupleType(nl->Second(resultType));
      local.addr = info;
      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST :
    {
      Word elem1;
      TupleId tid;
      Tuple* current;
      int arelIndex = (((CcInt*)args[2].addr)->GetIntval()) - 1;
      int noOfAttrs = ((CcInt*)args[3].addr)->GetIntval();
      int noAttrArel = ((CcInt*)args[4].addr)->GetIntval();
      info = (UnnestInfo *)local.addr;
      if (info->index == -1)
      {
        qp->Request(args[0].addr, elem1);
        if (qp->Received(args[0].addr))
        {
          current = (Tuple*)(elem1.addr);
          Tuple *tuple = new Tuple( info->tupleType );
          for( int i = 0; i < noOfAttrs; i++)
          {
            if (i < arelIndex)
              tuple->CopyAttribute(i, current, i);
            else
              if (i > arelIndex)
                tuple->CopyAttribute(i, current, i - 1);   
          }
          info->arel = (AttributeRelation*)(current->GetAttribute(arelIndex));
          tidArray = info->arel->getTupleIds();
          tidArray->Get(0, tid);
          rel = Relation::GetRelation(info->arel->getRelId());
          Tuple* arelTuple = rel->GetTuple(tid, false);
          for (int i = 0; i < noAttrArel; i++)
            tuple->CopyAttribute(i, arelTuple, i + noOfAttrs - 1);       
          info->lastTuple = current;
          info->index = 1;
          result.setAddr(tuple);
          arelTuple->DeleteIfAllowed();
          return YIELD;
        }
        else
        {
          return CANCEL;
        }
      }
      else
      {  
        info = (UnnestInfo *)local.addr;
        Tuple *tuple = new Tuple( info->tupleType );
        tidArray = info->arel->getTupleIds();
        if (info->index < tidArray->Size())
        {
          for( int i = 0; i < noOfAttrs; i++)
          {
            if (i < arelIndex)
              tuple->CopyAttribute(i, info->lastTuple, i);
            else
              if (i > arelIndex)
                tuple->CopyAttribute(i, info->lastTuple, i - 1);   
          }
          tidArray->Get(info->index, tid);
          rel = Relation::GetRelation(info->arel->getRelId());
          Tuple* arelTuple = rel->GetTuple(tid, false);
          info->index++;
          for (int i = 0; i < noAttrArel; i++)
            tuple->CopyAttribute(i, arelTuple, i + noOfAttrs - 1);       
          result.setAddr(tuple);
          arelTuple->DeleteIfAllowed();
          return YIELD;
        }
        else
        {
          qp->Request(args[0].addr, elem1);
          if (qp->Received(args[0].addr))
          {
            current = (Tuple*)elem1.addr;
            for( int i = 0; i < noOfAttrs; i++)
            {
              if (i < arelIndex)
                tuple->CopyAttribute(i, current, i);
              else
                if (i > arelIndex)
                  tuple->CopyAttribute(i, current, i - 1);   
            }
            info->arel = (AttributeRelation*)(current->GetAttribute
                                                       (arelIndex));
            tidArray = info->arel->getTupleIds();
            tidArray->Get(0, tid);
            rel = Relation::GetRelation(info->arel->getRelId());
            Tuple* arelTuple = rel->GetTuple(tid, false);   
            for (int i = 0; i < noAttrArel; i++)
              tuple->CopyAttribute(i, arelTuple, i + noOfAttrs -1);       
            info->lastTuple->DeleteIfAllowed();
            info->lastTuple = current;
            info->index = 1;
            result.setAddr(tuple);
            arelTuple->DeleteIfAllowed();
            return YIELD;
          }
          else
          {
            tuple->DeleteIfAllowed();
            info->lastTuple->DeleteIfAllowed();
            info->lastTuple = 0;
            return CANCEL;
          }                      
        }
      }
    }  
    case CLOSE :
    {
      qp->Close(args[0].addr);
      if(local.addr)
      {
        info = (UnnestInfo*)local.addr;
        if (info->lastTuple)
          info->lastTuple->DeleteIfAllowed();
        info->tupleType->DeleteIfAllowed();
        delete info;
        local.setAddr(0);
      }
      return 0;
    }
  }
  return 0;
} 

/*
5.6.3 Specification of operator ~unnest~

*/
struct unnestOperatorInfo : OperatorInfo {

  unnestOperatorInfo() : OperatorInfo()
  {
    name =      "unnest";
    signature = "(stream x) -> (stream y)";
    syntax =    "_ unnest [xi] (where xi is of type arel)";
    meaning =   "Unnests an attribute relation in a tuple stream.";
    example =   "query books feed unnest [authors] consume";
  }
};

/*
5.7 Operator ~rename~

Renames all attribute names, including those in arel-tuples, by adding 
the postfix passed as parameter.

5.7.1 Type mapping function of operator ~rename~

Type mapping for ~rename~ is

----  ((stream (tuple([a1:d1, ... ,an:dn)))ar) ->
           (stream (tuple([a1ar:d1, ... ,anar:dn)))
----

*/
ListExpr
renameArelAttrs( ListExpr first, string& attrnamen)
{
  ListExpr first2, rest,
           listn, lastlistn;
  string  attrname="";
  bool firstcall = true;
  
  rest = nl->Second(nl->Second(first));
  while (!(nl->IsEmpty(rest)))
  {
    first2 = nl->First(rest);
    rest = nl->Rest(rest);
    nl->SymbolValue(nl->First(first));
    attrname = nl->SymbolValue(nl->First(first2));
    attrname.append("_");
    attrname.append(attrnamen);

    if (!firstcall)
    {
      NList firstSecond(nl->Second(first2));
      if (!firstSecond.isAtom() && firstSecond.first().isSymbol() 
          && firstSecond.first().str() == "arel")
      {
        lastlistn  = nl->Append(lastlistn,
        nl->TwoElemList(nl->SymbolAtom(attrname), 
                      renameArelAttrs(firstSecond.listExpr(), attrnamen)));
      }
      else
      {  
        lastlistn  = nl->Append(lastlistn,
        nl->TwoElemList(nl->SymbolAtom(attrname), nl->Second(first2)));
      }
    }
    else
    {
      firstcall = false;
      NList firstSecond(nl->Second(first2));
      if (!firstSecond.isAtom() && firstSecond.first().isSymbol() 
          && firstSecond.first().str() == "arel")
      {
        listn  = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname), 
                      renameArelAttrs(firstSecond.listExpr(), attrnamen)));
      }
      else
      {  
                 
        listn = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname), 
                        nl->Second(first2)));
      }
      lastlistn = listn;
    }
  }
  return
    nl->TwoElemList(nl->SymbolAtom("arel"),
    nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
}  

ListExpr
nestedRenameTypeMap( ListExpr args )
{
  ListExpr first=nl->TheEmptyList();
  ListExpr first2=first, second=first, 
           rest=first, listn=first, lastlistn=first, tup, tupFirst, type;
  string  attrname="", argstr="";
  string  attrnamen="";
  bool firstcall = true;

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator rename expects a list of length two.");

  first = nl->First(args);
  second  = nl->Second(args);

  nl->WriteToString(argstr, first);
  if (!IsStreamDescription(first)) {
    ErrorReporter::ReportError(
    "Operator rename expects a valid tuple stream "
    "Operator rename gets a list with structure '" + argstr + "'.");
    return nl->TypeError();
  }

  nl->WriteToString(argstr, second);
  CHECK_COND( nl->IsAtom(second) &&
    nl->AtomType(second) == SymbolType,
    "Operator rename expects as second argument a symbol "
    "atom (attribute suffix) "
    "Operator rename gets '" + argstr + "'.");

  tup = nl->Second(nl->Second(first));
  bool containsArel = false;
  while (!(nl->IsEmpty(tup)) && !containsArel)
  {
    tupFirst = nl->First(tup);
    type = nl->Second(tupFirst);
    if (!(nl->IsAtom(type)))
    {
      type = nl->First(type); 
      if (nl->IsAtom(type))
        if(nl->SymbolValue(type) == "arel")
          containsArel = true;
    }
    tup = nl->Rest(tup);
  }
  CHECK_COND(containsArel, " ");
  
  rest = nl->Second(nl->Second(first));
  while (!(nl->IsEmpty(rest)))
  {
    first2 = nl->First(rest);
    rest = nl->Rest(rest);
    nl->SymbolValue(nl->First(first));
    attrname = nl->SymbolValue(nl->First(first2));
    attrnamen = nl->SymbolValue(second);
    attrname.append("_");
    attrname.append(attrnamen);

    if (!firstcall)
    {
      NList firstSecond(nl->Second(first2));
      if (!firstSecond.isAtom() && firstSecond.first().isSymbol() 
          && firstSecond.first().str() == "arel")
      {
        lastlistn  = nl->Append(lastlistn,
        nl->TwoElemList(nl->SymbolAtom(attrname), 
                      renameArelAttrs(firstSecond.listExpr(), attrnamen)));
      }
      else
      {  
        lastlistn  = nl->Append(lastlistn,
        nl->TwoElemList(nl->SymbolAtom(attrname), nl->Second(first2)));
      }
    }  
    else
    {
      firstcall = false;
      NList firstSecond(nl->Second(first2));
      if (!firstSecond.isAtom() && firstSecond.first().isSymbol() 
          && firstSecond.first().str() == "arel")
      {
        listn  = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname), 
                      renameArelAttrs(firstSecond.listExpr(), attrnamen)));
      }
      else
      {  
                 
        listn = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname), 
                        nl->Second(first2)));
      }
      lastlistn = listn;
    }
  }
  return
    nl->TwoElemList(nl->SymbolAtom("stream"),
    nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
}

/*
5.7.2 Value mapping function of operator ~rename~

*/
int
nestedRename(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  Word t;
  Tuple* tuple;

  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :

      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tuple = (Tuple*)t.addr;
        result.setAddr(tuple);
        return YIELD;
      }
      else return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;
      
  }
  return 0;
}

/*
5.7.3 Definition of operator ~rename~

*/
struct nestedRenameInfo : OperatorInfo {

  nestedRenameInfo() : OperatorInfo()
  {
    name =      "rename";
    signature = "(stream x) -> (stream y)";
    syntax =    "_ rename [ _ ]";
    meaning =   "Renames all attribute names by adding"
                " them with the postfix passed as parameter. ";
    example =   "query publishers feed {s} consume";
  }

};


/*
5.8 Operator ~extract~

This operator has a stream of tuples and the name of an arel-attribute 
as input. It returns either a nested relation or a relation containing all 
the tuples saved in the arel-Attribute of the first tuple of the input stream. 

5.8.1 Type mapping function of operator ~extract~

Type mapping for ~extract~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> rel(tup)
              APPEND (i) ti 1)
       or
       
      ((stream (tuple ((x1 t1)...(xn tn))) xi)  -> nrel(tup)
              APPEND (i) ti 0) 
      
      whith ti of the form:
            
            arel(tup)
      
      
----

*/
ListExpr extractTypeMap( ListExpr args )
{
  if(nl->ListLength(args)!=2)
  {
    return listutils::typeError("two arguments expected");
  }

  ListExpr stream = nl->First(args);
  ListExpr attrname = nl->Second(args);
  string err = "(stream( tuple[ a1 : t1, .., an : tn ])) x a_i expected";
  if(!listutils::isTupleStream(stream) ||
     nl->AtomType(attrname)!=SymbolType)
  {
    return listutils::typeError(err);
  }

  string attr = nl->SymbolValue(attrname); 
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  int j = listutils::findAttribute(attrList, attr, attrType);
  if (j) 
  {
    if (!(nl->ListLength(attrType) == 2 && nl->SymbolValue
          (nl->First(attrType)) == "arel"))
    {
      return listutils::typeError("Operator extract only defined for"
                                  " attribute-type arel.");
    }
    else
    {
      bool containsArel = false;
      NList attr (nl->Second(nl->Second(attrType)));
      NList first;
      while (!attr.isEmpty())
      {
            first = attr.first().second();
            attr.rest();
            if (first.hasLength(2) && first.first().str() == "arel")
              containsArel = true;
      }        
      if (containsArel)
      {
        //0 as third argument means, that the result type is nrel
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->TwoElemList(nl->IntAtom(j), nl->IntAtom(0)), 
             nl->TwoElemList(nl->SymbolAtom("nrel"), nl->Second(attrType)));
      }
      else
      {
        //1 as third argument means, that the result type rel
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->TwoElemList(nl->IntAtom(j), nl->IntAtom(1)), 
              nl->TwoElemList(nl->SymbolAtom("rel"), nl->Second(attrType)));   
      }
    }  
  } 
  else 
  {
    return listutils::typeError("Attribute name " + attr + 
                                " not known in the tuple");
  }
}


/*
5.8.2 Value mapping function of operator ~extract~

The argument vector ~args~ contains in the first slot ~args[0]~ the tuple,
in ~args[2]~ the position of the attribute as a number and in ~args[3]~ an 
integer value (0 indicates that the result type is nrel, 1 indicates that
the result type is rel). Returns as ~result~ a relation or a nested relation.

*/
int extractValueMap(Word* args, Word& result, int message, Word& local, 
                 Supplier s)
{
  Word t;
  Tuple* tupleptr;
  int index;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr,t);
  int i = ((CcInt*)args[3].addr)->GetIntval();
  if (i == 1)
  {
    Relation* res = (Relation*)((qp->ResultStorage(s)).addr);
    if (qp->Received(args[0].addr))
    {
      tupleptr = (Tuple*)t.addr;
      index = ((CcInt*)args[2].addr)->GetIntval();
      AttributeRelation* arel = 
              (AttributeRelation*)tupleptr->GetAttribute(index - 1);
      arel->CopyTuplesToRel(res);  
      tupleptr->DeleteIfAllowed(); 
      result.setAddr(res);
    }
    else
    {
      result.setAddr (0);
    }
  }  
  else
  {
    NestedRelation* res = (NestedRelation*)((qp->ResultStorage(s)).addr);
    if (qp->Received(args[0].addr)) 
    {
      tupleptr = (Tuple*)t.addr;
      index = ((CcInt*)args[2].addr)->GetIntval();
      AttributeRelation* arel = 
              (AttributeRelation*)tupleptr->GetAttribute(index - 1);
      arel->CopyTuplesToNrel(res);  
      tupleptr->DeleteIfAllowed(); 
      result.setAddr(res);
    }
    else
    {
      result.setAddr (0);
    }
  }
  qp->Close(args[0].addr);
  return 0;
}

/*
5.8.3 Specification of operator ~extract~

*/

struct extractInfo : OperatorInfo {

  extractInfo() : OperatorInfo()
  {
    name =      "extract";
    signature = "stream(x) -> rel(y)";
    appendSignature ("stream(x) -> nrel(y)"); 
    syntax =    "_ extract [xi] (where xi is of type arel)";
    meaning =   "Unnests an attribute relation in a tuple stream.";
    example =   "query publishers feed extract [publications]";
  }
};



/*
5.9 Operator ~gettuplesnrel~

5.9.1 Type mapping function of operator ~gettuplesnrel~

*/
ListExpr GetTuplesTypeMap_nrel(ListExpr args)
{
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();

  // check for correct parameter list
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError(
      "\nExpects exactly 2 arguments.");
  }
  // Split arguments into two parts
  ListExpr streamDescription = nl->First(args),
           nrelDescription = nl->Second(args);

  // Handle the stream part of arguments
  if(!listutils::isTupleStream(streamDescription)){
    return listutils::typeError("Expects a valid tuplestream as 1st argument.");
  }

  // Handle the nrel part of arguments
  if(!((nl->ListLength(nrelDescription)==2) && 
    nl->IsEqual(nl->First(nrelDescription), "nrel"))){
    return listutils::typeError("Expects a nrel as argument");
  }  
  
  

  // Check for existence of a single tid-attribute
  int tidIndex = 0;
  string tidAttrName = "";
  tidIndex = listutils::findType(nl->Second(nl->Second(streamDescription)),
                                 nl->SymbolAtom("tid"),
                                 tidAttrName);
  if( tidIndex <= 0 ){
    return listutils::typeError("Stream must contain an attribute of type "
                                "'tid'.");
  }
  else if( tidIndex > 0 ){
     int tidIndex2 = 0;
     string tidAttrName2 = "";
     tidIndex2 = listutils::findType(nl->Second(nl->Second(streamDescription)),
                                     nl->SymbolAtom("tid"),
                                     tidAttrName2,
                                     tidIndex+1);
     if (tidIndex2 != 0) {
         return listutils::typeError("Stream must contain at most one attribute"
                                     " of type 'tid'.");
     }
  }

  // remove tid-attribute from stream-attrlist
  set<string> k;
  k.insert(tidAttrName);
  ListExpr tmp, tmpL;
  int noRemovedAttrs = 0;
  noRemovedAttrs =
      listutils::removeAttributes(nl->Second(nl->Second(streamDescription)),
                                  k,
                                  tmp,
                                  tmpL);
  if(noRemovedAttrs != 1){
    return listutils::typeError("Stream must contain at most one attribute of "
                                "type 'tid'.");
  }

  // append rel-attrlist to modified stream-attrlist
  ListExpr newAttrList =
     listutils::concat(tmp, nl->Second(nl->Second(nrelDescription)));
  //listutils::typeError("after concat");
  // check whether result attrlist is valid
  if (!listutils::isAttrList(newAttrList)){
    return listutils::typeError("Result after merging tuples is not a "
                               "valid attribute list (Possible reasons: "
                               "duplicate attribute names or an attribute "
                               "type is not of kind DATA).");
  }
    

  // return resulttype and APPEND tid-attr index in stream-attrlist
  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(
        nl->IntAtom(tidIndex)),
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          newAttrList)));
}

/*
5.9.2 Value mapping function of operator ~gettuplesnrel~

The template parameter ~TidIndexPos~ specifies the argument number, where
the attribute index for the tid is stored within the stream argument's
tuple type. For gettuplesnrel, it is 2.

*/



#ifndef USE_PROGRESS

// standard version

                 
struct GetTuplesLocalInfo
{
  NestedRelation* nrel;
  Relation* rel = nrel->getPrimary();
  int tidIndex;
  TupleType *resultTupleType;
};

template<int TidIndexPos>
    int GetTuples( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{ 
  GetTuplesLocalInfo *localInfo;
  static MessageCenter* msg = MessageCenter::GetInstance();

  switch (message)
  {
    case OPEN :
    {
      assert( TidIndexPos == 2 || TidIndexPos == 3);
      qp->Open(args[0].addr);
      localInfo = new GetTuplesLocalInfo();
      localInfo->nrel=(NestedRelation*)(qp->ResultStorage(s).addr); 
      localInfo->rel=nrel->getPrimary() ;
      localInfo->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
      localInfo->tidIndex = ((CcInt*)args[TidIndexPos].addr)->GetIntval() - 1;
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (GetTuplesLocalInfo*)local.addr;

      Word wTuple;
      qp->Request(args[0].addr, wTuple);
      while( qp->Received(args[0].addr) )
      {
        Tuple* sTuple = (Tuple*)wTuple.addr;
        Tuple* resultTuple = new Tuple( localInfo->resultTupleType );
        Tuple* relTuple = localInfo->rel->
            GetTuple(((TupleIdentifier *)sTuple->
            GetAttribute(/*localInfo->tidIndex*/4))->GetTid());

        if(!relTuple){
          NList msg_list(NList("simple") ,
                    NList("Warning: invalid tuple id"));
          msg->Send(msg_list);
          qp->Request(args[0].addr, wTuple);
        } else {
          int j = 0;

          // Copy the attributes from the stream tuple
          for( int i = 0; i < sTuple->GetNoAttributes(); i++ )
          {
            if( i != localInfo->tidIndex )
              resultTuple->CopyAttribute( i, sTuple, j++ );
          }
          sTuple->DeleteIfAllowed();

          for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
          {
            resultTuple->CopyAttribute( i, relTuple, j++ );
          }
          relTuple->DeleteIfAllowed();

          result.setAddr( resultTuple );
          return YIELD;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      if(local.addr)
      {
        localInfo = (GetTuplesLocalInfo*)local.addr;
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
  
}

# else

// progress version


struct GetTuplesLocalInfo: public ProgressLocalInfo
{
  //NestedRelation* nrel;//=(NestedRelation*)(qp->ResultStorage(s).addr);
  Relation* rel;//=nrel->getPrimary();
  int tidIndex;
  TupleType *resultTupleType;
};

template<int TidIndexPos>
    int GetTuples( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{    
  NestedRelation* nrel=(NestedRelation*)(qp->ResultStorage(s).addr);
  GetTuplesLocalInfo *localInfo;
  localInfo = (GetTuplesLocalInfo*)local.addr;
  static MessageCenter* msg = MessageCenter::GetInstance();

  switch (message)
  {
    case OPEN :
    {
    
      assert( TidIndexPos == 2 || TidIndexPos == 3);
      qp->Open(args[0].addr);

      if ( !localInfo )  // first time
      {
        localInfo = new GetTuplesLocalInfo();
        localInfo->rel=nrel->getPrimary() ;
        localInfo->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
        localInfo->tidIndex = ((CcInt*)args[TidIndexPos].addr)->GetIntval() - 1;

        local.setAddr(localInfo);
      }
      return 0;
    }

    case REQUEST :
    {
      
      Word wTuple;
      qp->Request(args[0].addr, wTuple);
      while( qp->Received(args[0].addr) )
      {
        localInfo->read++;

        Tuple* sTuple = (Tuple*)wTuple.addr;
        Tuple* resultTuple = new Tuple( localInfo->resultTupleType );
        Tuple* relTuple = localInfo->rel->
            GetTuple(((TupleIdentifier *)sTuple->
            GetAttribute(localInfo->tidIndex))->GetTid(),true);

        if(!relTuple){
          NList msg_list(NList("simple") ,
                    NList("Warning: invalid tuple id"));
          msg->Send(msg_list);
          qp->Request(args[0].addr, wTuple);
        } else {
          int j = 0;

          // Copy the attributes from the stream tuple
          for( int i = 0; i < sTuple->GetNoAttributes(); i++ )
          {
            if( i != localInfo->tidIndex )
              resultTuple->CopyAttribute( i, sTuple, j++ );
          }
          sTuple->DeleteIfAllowed();

          for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
          {
            resultTuple->CopyAttribute( i, relTuple, j++ );
          }
          relTuple->DeleteIfAllowed();

          result.setAddr( resultTuple );

          localInfo->returned++;
          return YIELD;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      
      qp->Close(args[0].addr);
      return 0;
    }

   
    case CLOSEPROGRESS :
    {
  
      if ( localInfo )
      {
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
      }
      local.setAddr(Address(0));
      return 0;
    }
    
    case REQUESTPROGRESS :
    {

      ProgressInfo p1;
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      //Experiments described in file Cost functions
      const double uTuple = 0.061;    //milliseconds per tuple
      const double vByte = 0.0000628;  //milliseconds per byte

      if( !localInfo || !qp->RequestProgress(args[0].addr, &p1) ) {
        // ask stream argument
        return CANCEL;
      }
      localInfo->sizesChanged =
                            (!localInfo->sizesInitialized || p1.sizesChanged);
      if ( localInfo->sizesChanged ){
          localInfo->total = (int) p1.Card;
          localInfo->noAttrs =
            nl->ListLength(nl->Second(nl->Second(qp->GetType(s))));

          if(!localInfo->sizesInitialized){
             localInfo->attrSize = new double[localInfo->noAttrs];
             localInfo->attrSizeExt = new double[localInfo->noAttrs];
          }
          // ordering of attributes is:
          //  attributes from first argument (without the one at tidIndex)
          //  then relation attributes
          int no_stream_attrs = p1.noAttrs;
          int no_rel_attrs = localInfo->noAttrs - (no_stream_attrs - 1);
          // copy first part of stream attrs
          int j = 0;
          for (int i = 0;  i < localInfo->tidIndex; i++) {
            localInfo->attrSize[j]    = p1.attrSize[i];
            localInfo->Size          += p1.attrSize[i];
            localInfo->attrSizeExt[j] = p1.attrSizeExt[i];
            localInfo->SizeExt       += p1.attrSizeExt[i];
            j++;
          }
          // copy second part of stream attrs
          for (int i = localInfo->tidIndex+1;  i < no_stream_attrs; i++) {
            localInfo->attrSize[j]    = p1.attrSize[i];
            localInfo->Size          += p1.attrSize[i];
            localInfo->attrSizeExt[j] = p1.attrSizeExt[i];
            localInfo->SizeExt       += p1.attrSizeExt[i];
            j++;
          }
          // copy rel attrs
          for ( int i = 0;  i < no_rel_attrs; i++) {
            localInfo->attrSize[j] = localInfo->rel->GetTotalSize(i)
                                  / (localInfo->total + 0.001);
            localInfo->attrSizeExt[j] = localInfo->rel->GetTotalExtSize(i)
                                  / (localInfo->total + 0.001);
            localInfo->Size += localInfo->attrSize[j];
            localInfo->SizeExt += localInfo->attrSizeExt[j];
            j++;
          }
          localInfo->sizesInitialized = true;
      }
      pRes->CopySizes(localInfo);
      pRes->Card = p1.Card;
      pRes->Time = p1.Time + p1.Card * (uTuple + vByte * localInfo->SizeExt);
      if ( p1.BTime < 0.1 && pipelinedProgress ) { //non-blocking,
                                                   //use pipelining
        pRes->Progress = p1.Progress;
      } else {
        pRes->Progress =   ((p1.Progress * p1.Time)
                         + (localInfo->read / p1.Card)
                         * (uTuple + vByte * localInfo->SizeExt))
                         / pRes->Time;
      }
      pRes->CopyBlocking(p1);
      return YIELD;
      
    } // case REQUESTPROGRESS
  } // switch  
  return 0;
  
} 

#endif


/*
5.1.5 Specification of operator ~gettuplesnrel~

*/
const string gettuplesSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>(stream (tuple ((id tid) (x1 t1)...(xn tn)))) x"
      " (nrel (tuple ((y1 t1)...(yn tn)))) ->"
      " (stream (tuple ((x1 t1)...(xn tn) (y1 t1)...(yn tn))))"
      "</text--->"
      "<text>_ _ gettuples_nrel</text--->"
      "<text>Retrieves the tuples in the relation in the second "
      "argument given by the tuple id in first argument stream. "
      "The result tuple type is a concatenation of both types "
      "without the 'tid' attribute.</text--->"
      "<text>query citiesInd windowintersectsS[r] cities gettuples; "
      "where citiesInd is e.g. created with 'letidAttrName2t citiesInd = "
      "cities creatertree [pos]'</text--->"
      ") )";

/*
5.1.6 Definition of operator ~gettuplesnrel~

*/
struct gettuples_nrelInfo : OperatorInfo {

  gettuples_nrelInfo() : OperatorInfo()
  {
    name =      "gettuples";
    signature = "stream(x) -> rel(y)";
    appendSignature ("stream(x) -> nrel(y)"); 
    syntax =    "_ _ gettuples_nrel ";
    meaning =   "  ";
    example =   " ";
  }
};
Operator gettuples_nrel (
         "gettuples_nrel",            // name
         gettuplesSpec,          // specification
         GetTuples<2>,           // value mapping
         Operator::SimpleSelect, // trivial selection function
         GetTuplesTypeMap_nrel        // type mapping
);



/*
5.10.1 Typemapping of operator ~terms~

*/
ListExpr TermsTypeMap(ListExpr args)
{
  ListExpr first, tup, tupFirst, type ;
  string argstr;

   if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 1){
    return listutils::typeError(
      "\nExpects exactly 1 arguments.");
  }

  first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND(
    ((nl->ListLength(first) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream)) &&
    (!(nl->IsAtom(nl->Second(first)) ||
       nl->IsEmpty(nl->Second(first))) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)),
  "Operator expects an argument of type (stream(tuple"
  "((a1 t1)...(an tn)))).\n"
  "Operator gets an argument of type '" + argstr + "'.");
  
  tup = nl->Second(nl->Second(first));
  bool containsArel = false;
  while (!(nl->IsEmpty(tup)) && !containsArel)
  {
    tupFirst = nl->First(tup);
    type = nl->Second(tupFirst);
    if (!(nl->IsAtom(type)))
    {
      type = nl->First(type); 
      if (nl->IsAtom(type))
        if(nl->SymbolValue(type) == "arel")
          containsArel = true;
    }
    tup = nl->Rest(tup);
  }
  CHECK_COND(containsArel, "Operator expects nested tuples as"
                           " arguments.");
  
  ListExpr temp = NestedRelation::unnestedList(nl->Second(first));
  string s;
  CHECK_COND(NestedRelation::namesUnique(temp, s), 
              "The attributename '" + s + "' in the incoming "
              "stream is not unique.\n" 
              "Please make sure that there are no duplicates.");
  //tid needs to be searched and found for further saving in the array as docid
  // Check for existence of a single tid-attribute
  // Split arguments into two parts
  ListExpr streamDescription = nl->First(args),
           nrelDescription = nl->Second(args);  
int tidIndex = 0;
  string tidAttrName = "";
  tidIndex = listutils::findType(nl->Second(nl->Second(streamDescription)),
                                 nl->SymbolAtom("tid"),
                                 tidAttrName);
  if( tidIndex <= 0 ){
    return listutils::typeError("Stream must contain an attribute of type "
                                "'tid'.");
  }
  else if( tidIndex > 0 ){
     int tidIndex2 = 0;
     string tidAttrName2 = "";
     tidIndex2 = listutils::findType(nl->Second(nl->Second(streamDescription)),
                                     nl->SymbolAtom("tid"),
                                     tidAttrName2,
                                     tidIndex+1);
     if (tidIndex2 != 0) {
         return listutils::typeError("Stream must contain at most one attribute"
                                     " of type 'tid'.");
     }
  }

  // remove tid-attribute from stream-attrlist
  set<string> k;
  k.insert(tidAttrName);
  ListExpr tmp, tmpL;
  int noRemovedAttrs = 0;
  noRemovedAttrs =
      listutils::removeAttributes(nl->Second(nl->Second(streamDescription)),
                                  k,
                                  tmp,
                                  tmpL);
  if(noRemovedAttrs != 1){
    return listutils::typeError("Stream must contain at most one attribute of "
                                "type 'tid'.");
  }

  // append rel-attrlist to modified stream-attrlist
  ListExpr newAttrList =
     listutils::concat(tmp, nl->Second(nl->Second(nrelDescription)));
  //listutils::typeError("after concat");
  // check whether result attrlist is valid
  if (!listutils::isAttrList(newAttrList)){
    return listutils::typeError("Result after merging tuples is not a "
                               "valid attribute list (Possible reasons: "
                               "duplicate attribute names or an attribute "
                               "type is not of kind DATA).");
  }
    

  // return resulttype and APPEND tid-attr index in stream-attrlist
  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(
        nl->IntAtom(tidIndex)),
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          newAttrList)));
  
  
  
  
}





/*
5.10.2 Value mapping of operator ~terms~

*/
/*{ 
  switch (message)
  {
    case OPEN :
    {
    }

    case REQUEST :
    {
    }

    case CLOSE :
    {
    }
  }
  return 0;
  
}*/



/*
5.10.5 Specificaion of operator ~terms~

*/
const string termsSpec  =
      "";


/*
5.10.6 Definition of operator ~terms~

*/
struct termsInfo : OperatorInfo {

  termsInfo() : OperatorInfo()
  {
    name =      "";
    signature = "";
    appendSignature (""); 
    syntax =    "";
    meaning =   "";
    example =   "";
  }
};





/*
6 NestedRelationAlgebra

*/
class NestedRelationAlgebra : public Algebra
{
  public:
    NestedRelationAlgebra() : Algebra()
    {
      AddTypeConstructor( &attributeRelationTC );
      AddTypeConstructor( &nestedRelationTC );
      AddOperator (feedInfo(), feed, feedTypeMap);
      AddOperator (consumeInfo(), consume, consumeTypeMap);
      AddOperator (aFeedInfo(), aFeed, aFeedTypeMap);
      AddOperator (&aconsume);
      AddOperator (&nest);
      AddOperator (unnestOperatorInfo(), unnestValueMap, unnestTypeMap);
      AddOperator (nestedRenameInfo(), nestedRename, nestedRenameTypeMap);
      AddOperator (extractInfo(), extractValueMap, extractTypeMap); 
      AddOperator (&gettuples_nrel);
      attributeRelationTC.AssociateKind( "DATA" );
#ifdef USE_PROGRESS
      nest.EnableProgress();
      aconsume.EnableProgress();
#endif      
    }
    ~NestedRelationAlgebra() {};
};

/*
7 Data Structures

7.1. Struct Doclist
*/



/*
7.2 Class Doclist

*/
// moved to header file


/*
7.3 needs to be named

*/
Doclist::Doclist()
{}

Doclist::Doclist(bool Defined, string Term, Docidlist& Doc, DoclistState State)
{
  defined=Defined;
  term=Term;
  doclists.Append(Doc);
  state=State;
}

string Doclist::GetTerm() const
{
  return term;
}



void Doclist::AppendID(const Docidlist& doc)
{
  assert (state == partial);
  doclists.Append(doc);
  //doclists.Sort; needs to be used
}

void Doclist::Complete()
{
  assert( state == partial );
  state = complete;
}


void Doclist::Destroy()
{
  assert (state==complete);
  doclists.Destroy();
}

int Doclist::NumOfFLOBs() const
{
  return 1;
}

Flob *Doclist::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &doclists;
}

int Doclist::Compare(const Attribute* attr) const
{
 const Doclist * d = (const Doclist* )(attr);
 if ( !d ) return (-1);

 if (!IsDefined() && !(attr->IsDefined()))  return 0;
 else if (!IsDefined())  return -1;
    else  if (!(attr->IsDefined())) return 1;
       else
       {
         if ((this->GetTerm()) > (d->GetTerm())) return 1;
         else if ((this->GetTerm()) < (d->GetTerm())) return -1;
         else return 0;
       }
}

bool Doclist::Adjacent(const Attribute* attr) const
{
  return 0;
}

Doclist *Doclist::Clone() const
{
  return (new Doclist( *this));
}

bool Doclist::IsDefined() const
{
  return true;
}
 
size_t Doclist::Sizeof() const
{
  return sizeof( *this );
}

size_t Doclist::HashValue() const
{
  if( !IsDefined() ){
    return 0;
  }
  return 1 ;
}
    
void Doclist::CopyFrom(const Attribute* attr)
{
  const Doclist * d = (const Doclist*)attr;
  defined = d->defined;
  term = d->term;
  state = d->state;
  doclists = d->doclists;
}

DoclistState Doclist::GetState()
{
  return state;
}


//Auxiliary Functions for Doclist
void Doclist::invert(string text)
{
	//Länge des eingabetextes
	int laenge=text.length();

	//char array mit Länge von Eingabetext initialisieren
	char *textarray=new char[laenge];

	//Eingabetext zeichenweise in Array kopieren
	strcpy(textarray,text.c_str());
	
	int i;
	int zaehler=0;
	string wort="";
	
	
	//vector als dynamisches array initialisieren
	vector<wortpos> wpvector;
	
	//Array zeichenweise durchlaufen
	for (i=0; i<=laenge; i++)
	{
		//satzzeichen und leerzeichen rausnehmen
		if (!(textarray[i]==' ' || textarray[i]==',' || 
    textarray[i]=='.' || textarray[i]==':' || 
    textarray[i]==';' || textarray[i]=='!' || 
    textarray[i]=='?'||i==laenge))
		{
			wort=wort+textarray[i];
		}

		//mehrfache satz und leerzeichen rausnehmen
		else if (!(wort==""))
		{
			cout << "Position:" << zaehler << " Wort:" 
      << wort << endl;
			
			//pos und wort in struct schreiben
			wp.wort=wort;
			wp.pos=zaehler;

			//struct in vector speichern
			wpvector.push_back(wp);
			zaehler=i+1;
			wort="";
		}
		//weiter nummerieren bei mehrfachen satz und leerzeichen
		else 
		{
			zaehler=i+1;
			wort="";
		}
	}
	sort(wpvector.begin(), wpvector.end(),wortpos());
/*	
	int j;
	int k=wpvector.size();
	for (j=0; j<k;j++)
	{
		cout << wpvector[j].wort << " " << wpvector[j].pos <<endl;
	}
*/
}




/*
8 Initialization

*/

extern "C"
Algebra*
InitializeNestedRelationAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new NestedRelationAlgebra());
}                                                                               
