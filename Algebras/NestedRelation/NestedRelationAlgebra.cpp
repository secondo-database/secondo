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
/*

3 Implementation of class AttributeRelation

3.1 Constructors and Destructor

*/
AttributeRelation::AttributeRelation( ListExpr typeInfo, bool nrel):
 tupleIds(0),
 arelType( typeInfo ),
 partOfNrel( nrel ),
 relDelete( false ),
 tupleFileSet(false)
{
   if (!(nl->IsEmpty( typeInfo )))
      {
      if ((nl->ListLength( typeInfo ) == 3))
      {
         tupleFile = nl->IntValue (nl -> Third( typeInfo )); 
         rel = Relation::GetRelation( tupleFile );
         tupleFileSet = true;
      }
   }
}

AttributeRelation::AttributeRelation( const SmiFileId fileId):
   tupleIds( 0 ),
   arelType( 0 ),
   partOfNrel( true ),
   relDelete( false )
{
   setFileId( fileId );
}

AttributeRelation::~AttributeRelation() 
{}

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

void AttributeRelation::setRelDelete(bool b)
{
     relDelete = b;
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

void AttributeRelation::setFileId(SmiFileId id)
{
   tupleFile = id;
   tupleFileSet = true;
   if (!(tupleFile == 0))
   {
      rel = Relation::GetRelation( tupleFile );
   } 
}

SmiFileId AttributeRelation::getFileId() const
{
   return tupleFile;
}


DBArray<TupleId>* AttributeRelation::getTupleIds()
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

/*
3.3 Functions implementing virtual functions from class Attribute 

*/
int AttributeRelation::NumOfFLOBs() const
{
  return 1;
}

FLOB *AttributeRelation::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &tupleIds;
}

int AttributeRelation::Compare(const Attribute* attr) const
{
  AttributeType atype;
  const Attribute* a1, *a2;
  int cmp;
  Relation* rel1 = Relation::GetRelation(tupleFile);
  TupleType* tt = rel1->GetTupleType();
  const AttributeRelation* arel = static_cast<const AttributeRelation*> (attr);
  Relation* rel2;
  if (tupleFile == arel->getFileId())
    rel2 = rel1;
  else
    rel2 = Relation::GetRelation(arel->getFileId());
  const DBArray<TupleId>* arelTids = &(arel->tupleIds);
  Tuple *t1, *t2;
  const TupleId* tid1, *tid2;
  int i = 0;  
  while (i < (&tupleIds)->Size() && i < arelTids->Size())
  {
    (&tupleIds)->Get(i, tid1);
    arelTids->Get(i, tid2);
    t2 = rel2->GetTuple(*tid2);
    t1 = rel1->GetTuple(*tid1);
    
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
  }
  t1->DeleteIfAllowed();
  t2->DeleteIfAllowed();
  if (i < (&tupleIds)->Size())
    return 1;
  if (i < arelTids->Size())
    return -1;    
  return 0;
}

bool AttributeRelation::Adjacent(const Attribute*) const
{
  return 0;
}

AttributeRelation *AttributeRelation::Clone() const
{
  AttributeRelation *arel = new AttributeRelation( this->arelType, false );
  arel->setFileId(this->tupleFile);
  const TupleId *tid;
  for (int i = 0; i < this->tupleIds.Size(); i++){
      this->tupleIds.Get(i, tid);
      arel->Append(*tid);
  }
  return arel;          
}

bool AttributeRelation::IsDefined() const
{
  return true;
}

void AttributeRelation::SetDefined( bool defined )
{
}

size_t AttributeRelation::Sizeof() const
{
  return sizeof( *this );
}

size_t AttributeRelation::HashValue() const
{
  size_t value = 0;
  AttributeType atype;
  const StandardAttribute* a1;
  Relation* rel1 = Relation::GetRelation(tupleFile);
  TupleType* tt = rel1->GetTupleType();
  tt->IncReference();
  Tuple *t1;
  const TupleId* tid1;
  int i = 0;  
  while (i < (&tupleIds)->Size())
  {
    (&tupleIds)->Get(i, tid1);
    t1 = rel1->GetTuple(*tid1);
    for (int j = 0; j < tt->GetNoAttributes(); j++)
    {
      a1 = (StandardAttribute*)t1->GetAttribute(j);
      atype = tt->GetAttributeType(j);
      value = a1->HashValue();
    }
  i++;
  }  
  t1->DeleteIfAllowed();
  tt->DeleteIfAllowed();    
  return value; 
}
  

void AttributeRelation::CopyFrom(const StandardAttribute* right)
{
  AttributeRelation* arel = (AttributeRelation*) right;
  partOfNrel = false;
  relDelete = false;
  tupleFile = arel->getFileId();
  DBArray<TupleId>* tids = arel->getTupleIds(); 
  const TupleId* tid;
  if (!isEmpty())
    tupleIds.Clear();
  for (int i = 0; i < tids->Size(); i++)
  {
    tids->Get(i, tid);  
    Append(*tid);
  }
}
    
/*
3.4 The mandatory set of algebra support functions

3.4.1 In-function 
 
*/ 
Word AttributeRelation::In(const ListExpr typeInfo, const ListExpr value, 
                           const int errorPos, ListExpr& errorInfo, 
                           bool& correct)
{
   Word result = SetWord( Address(0) );
   correct = true;
   AttributeRelation* arel;
   ListExpr tuplelist, TupleTypeInfo, first;
   Relation* rel;
   
   if (!(nl->ListLength( typeInfo ) == 3) ) //no file Id attached
   {
      int relAlgId = am->GetAlgebraId("RelationAlgebra");
      int relId = NestedRelation::getTypeId(relAlgId, "rel");
      ListExpr relType = nl->TwoElemList(nl->TwoElemList(nl->IntAtom(relAlgId),
                      nl->IntAtom(relId)),nl->Second(typeInfo)); 
      rel = new Relation( relType );
      arel = new AttributeRelation (rel->GetFileId());
      arel->setPartOfNrel(false);
   }
   else
   {
     arel = new AttributeRelation( typeInfo, true );
     rel = arel->rel;
   }
   Tuple* tupleaddr;
   int tupleno, count;
   bool tupleCorrect;
   tuplelist = value;
   tupleno = 0;
   count = 0;
   
   TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
    nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    
   if (nl->IsAtom(value))
   {
      correct = false;
      errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
       nl->IntAtom(70),
       nl->SymbolAtom("arel"),
       tuplelist));
      if (!arel->isPartOfNrel())
        rel->Delete();
      delete arel;
      return result;
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
         if (!arel->isPartOfNrel())
           rel->Delete();
         delete arel;
         return result;
      }
      else
      {
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
  DBArray<TupleId>* tids = arel->getTupleIds();
  const TupleId *tid;
  if (arel->isPartOfNrel())
  {
    arel->setRel(Relation::GetRelation(arel->getFileId()));
    Relation* rel = arel->getRel();
    Tuple* t=0;
    ListExpr l= nl->Cons(nl->IntAtom(0), nl->TheEmptyList());
    ListExpr lastElem=l, tlist=l;
    ListExpr tupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));  
    for (int i = 0; i < tids->Size(); i++)  
    {
      tids->Get(i, tid);  
      t = rel->GetTuple(*tid);
      tlist = t->Out(tupleTypeInfo);
      lastElem = nl->Append(lastElem, tlist);
    }
    return l;
  }
  else
  {
    arel->setRel(0);
    NList outlist;
    outlist.append(nl->IntAtom(1));
    ListExpr val;
    for (int i = 0; i < tids->Size(); i++)
    {
      tids->Get(i, tid);
      val = nl->IntAtom(*tid);
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
  string s;
  nl->WriteToString(s, typeInfo);
  return ( SetWord(new AttributeRelation (typeInfo, false) ) );
}

/*
3.4.4 Delete-function

*/
void AttributeRelation::Delete( const ListExpr typeInfo, Word& w )
{
     AttributeRelation* arel = (AttributeRelation*) w.addr;
     if (!arel->isPartOfNrel() && arel->relDelete)
     {
        arel->relDelete = false;
        Relation* rel = Relation::GetRelation( arel->getFileId() );
        if (!(rel == 0))
        {
          rel->Delete();
        }
     }
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
     AttributeRelation* arel = static_cast<AttributeRelation *> (w.addr); 
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
  AttributeRelation *arel = (AttributeRelation*)Attribute::Open( valueRecord, 
                                                            offset, typeInfo );
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
    Relation* rel = Relation::GetRelation( arel->getFileId() );
    if (!(rel == 0))
    {
      rel->Delete();
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

struct attributeRelationFunctions : ConstructorFunctions<AttributeRelation> {

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

4.1 Constructors

*/

NestedRelation::NestedRelation(ListExpr typeInfo) :
                                        subRels(0)
{
   
   int relAlgId = am->GetAlgebraId("RelationAlgebra");
   int relId = getTypeId(relAlgId, "rel");
   primaryTypeInfo = nl->TwoElemList(nl->TwoElemList(nl->IntAtom(relAlgId), 
                                     nl->IntAtom(relId)), nl->Second
                                     (typeInfo));
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
        first2.first().first().isInt() && first2.first().first().intval() == 
        nrelAlgId && first2.first().second().isInt() && 
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
      if (!first2.first().isAtom() && first2.first().hasLength(2) && 
        first2.first().first().isInt() && first2.first().first().intval() 
        == nrelAlgId && first2.first().second().isInt() && 
        first2.first().second().intval() == arelId) 
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

int NestedRelation::getSubRelIndex(string name)
{
   for ( unsigned int i = 0; i < subRels.size(); i++ )
   {
      SubRelation* srel = subRels[i];
      if ( srel->name == name )
         return i; 
   }
   return -1;   
}

ListExpr NestedRelation::getTupleType()
{
   return tupleTypeInfo;
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
   Relation* primaryClone = nrel->getPrimary();
   while( (t = iter->GetNextTuple()) != 0 )
      {
         primaryClone->AppendTuple( t );
         t->DeleteIfAllowed();
      }
   delete iter;
   
   vector<SubRelation*>* srels = nrel->getSubRels();
   for ( unsigned int i = 0; i < subRels.size(); i++ )
   {
      SubRelation* srel = subRels[i];
      SubRelation* srelClone = srels->at(i);
      iter = srel->rel->MakeScan();
      while( (t = iter->GetNextTuple()) != 0 )
      {
         srelClone->rel->AppendTuple( t );
         t->DeleteIfAllowed();
      }
      delete iter;
   }
   return nrel;
}


vector<SubRelation*>* NestedRelation::getSubRels()
{
   return &subRels;
}                   
    
/*
4.3 The mandatory set of algebra support functions

4.3.1 In-function

*/

Word
NestedRelation::In( const ListExpr typeInfo, const ListExpr value,
                const int errorPos, ListExpr& errorInfo, bool& correct )
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
      return result;
   }
   else
   { // increase tupleno
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

  //cerr << "OutRel " << endl;
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
      ok = ok && NestedRelation::readString (subType, valueRecord, offset);
      nl->ReadFromString(subType, subTypeInfo);
      rel = Relation::Open(valueRecord, offset, subTypeInfo);
      ok = ok && NestedRelation::readString (name, valueRecord, offset);
      ok = ok && valueRecord.Read(&fileId, sizeof( SmiFileId ), offset);
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
      ok = ok && NestedRelation::saveString(typeString, valueRecord, offset);
      ListExpr typeInfo = sR->typeInfo;
      Relation* r = sR->rel;
      ok = ok && r->Save(valueRecord, offset, typeInfo);
      ok = ok && NestedRelation::saveString(sR->name, valueRecord, offset );
      ok = ok && valueRecord.Write(&sR->fileId, sizeof(SmiFileId), offset );
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

struct nestedRelationFunctions : ConstructorFunctions<NestedRelation> {

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
    && ( !(nl->IsAtom(nl->Second(first)) || nl->IsEmpty(nl->Second(first)))
    && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) ),
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

Operator ~consume~ accepts a stream of tuples and returns a nested relation.

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
  CHECK_COND(containsArel, " ");
  
  ListExpr temp = NestedRelation::unnestedList(nl->Second(first));
  string s;
  CHECK_COND(NestedRelation::namesUnique(temp, s), "The attributename '" + s + 
              "' in the incoming stream is not unique.\n" 
              "Please make sure that there are no duplicates.");                      
  return nl->Cons(nl->SymbolAtom("nrel"), nl->Rest(first));
}



/*
5.2.2 Value mapping function of operator ~consume~

5.2.2.1 Auxiliary function storeSubRel for value map of consume

*/
AttributeRelation* storeSubRel(AttributeRelation* a, int& i, 
                               NestedRelation* nr)
{
   int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
   int arelId = NestedRelation::getTypeId(nrelAlgId, "arel"); 
   Relation* relOld = Relation::GetRelation(a->getFileId());
   SmiFileId fileId = nr->getSubRels()->at(i)->fileId;
   AttributeRelation* arel = new AttributeRelation (fileId);                     
   arel->setPartOfNrel(true);
   Relation* r = nr->getSubRels()->at(i)->rel;
   int j = i;
   DBArray<TupleId>* tids = a->getTupleIds();
   const TupleId* tid;
   for (int i1 = 0; i1 < tids->Size(); i1++)
   {   
      i = j;
      Tuple* t = 0;
      Tuple* newTuple;
      tids->Get(i1, tid);  
      t = relOld->GetTuple(*tid);
      newTuple = new Tuple(t->GetTupleType());
      for (int i2 = 0; i2 < t->GetTupleType()->GetNoAttributes(); i2++)
      {
         if (t->GetTupleType()->GetAttributeType(i2).algId == nrelAlgId && 
         t->GetTupleType()->GetAttributeType(i2).typeId == arelId)
         {
            i = i + 1;
            AttributeRelation* ar = (AttributeRelation*)(t->GetAttribute(i2));
            AttributeRelation* b = storeSubRel(ar, i, nr);
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

/*
5.2.2.2 Main value mapping function for operator ~consume~

*/
int
consume(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  
  int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
  int arelId = NestedRelation::getTypeId(nrelAlgId, "arel"); 
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
     srels->at(k)->rel->Clear();
     
  }
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
     int i = -1;
     Tuple* tuple = (Tuple*)actual.addr;
     Tuple* newTuple = new Tuple(tuple->GetTupleType());
     for (int i2 = 0; i2 < tuple->GetTupleType()->GetNoAttributes(); i2++)
     {
        if (tuple->GetTupleType()->GetAttributeType(i2).algId == nrelAlgId && 
        tuple->GetTupleType()->GetAttributeType(i2).typeId == arelId)
        {
           i = i + 1;
           AttributeRelation* temp1 = (AttributeRelation*)
                                       (tuple->GetAttribute(i2));
           AttributeRelation* b = storeSubRel(temp1, i, nrel);
           newTuple->PutAttribute(i2, b);
        } 
        else
        {
            newTuple->CopyAttribute(i2, tuple, i2);
        }
     }
     rel->AppendTuple(newTuple);
     tuple->DeleteIfAllowed();
     newTuple->DeleteIfAllowed();
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
      && ( nl->IsAtom(nl->First(first)) && nl->AtomType(nl->First(first)) == 
      SymbolType && nl->SymbolValue(nl->First(first)) == "arel" )
      && ( !(nl->IsAtom(nl->Second(first)) || nl->IsEmpty(nl->Second(first)))
      && (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) ),
  "Operator afeed expects an argument of type attribute relation, "
  "(arel(tuple((a1 t1)...(an tn)))).\n"
  "Operator afeed gets an argument of type '" + argstr + "'.");

  return nl->Cons(nl->SymbolAtom("stream"),nl->Rest(first));
}
/*
5.3.2 Value mapping function of operator ~afeed~

*/
int
aFeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  int* index;
  AttributeRelation* arel;
  Relation* r;
  Tuple* t;
  switch (message)
  {
    case OPEN :
      index = new int(0);
      local.addr = index;
      return 0;

    case REQUEST :
      index = (int*)local.addr;
      arel = (AttributeRelation*) args[0].addr;
      arel->setFileId(arel->getFileId());
      r = arel->getRel();
      const TupleId* tid;
      if (*index < arel->getTupleIds()->Size())
      { 
        arel->getTupleIds()->Get(*index, tid); 
        t = r->GetTuple(*tid);
        int i = *index;
        i++;
        delete index;
        index = new int(i);
        local.addr = index;
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
         index = (int*)local.addr;
         delete index;
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
  CHECK_COND(NestedRelation::namesUnique(temp, s), "The attributename '" + s + 
              "' in the incoming stream is not unique.\n" 
              "Please make sure that there are no duplicates.");   
  return nl->Cons(nl->SymbolAtom("arel"), nl->Rest(first));
}
/*
5.4.2 Value mapping function of operator ~aconsume~

*/
int
aConsume(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  
  Word actual;
  AttributeRelation* arel = (AttributeRelation*)(qp->ResultStorage(s).addr);
  arel->getTupleIds()->Clear();
  int relAlgId = am->GetAlgebraId("RelationAlgebra");
  int relId = NestedRelation::getTypeId(relAlgId, "rel");
  Relation* rel; 
  if (!local.addr)
  {
     ListExpr relType = nl->TwoElemList(nl->TwoElemList(nl->IntAtom(relAlgId),
                      nl->IntAtom(relId)),nl->Second(arel->getArelType())); 
     rel = new Relation(relType);
     arel->setRel(rel);
     arel->setFileId(rel->GetFileId());
     arel->setPartOfNrel(false);
     local.addr = rel;
  }
  else
  {
      rel = (Relation*)local.addr;
      arel->setFileId(rel->GetFileId());
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
/*
5.5 Operator ~nest~

This operator accepts a stream of tuples and transfers all attributes not 
mentioned in the first argument into a subrelation (of type arel), 
the name of which is specified by the second argument. The outcome of this
operator is a nested relation.

5.5.1 Type mapping function of operator ~nest~

Type mapping for ~nest~ is

----     ((stream (tuple ((x1 t1)...(xn tn)))) ((xi1) ... (xij)) (xo))
              -> (nrel (tuple ((xi1 ti1)...(xij tij) 
                 (xo arel(tuple((xik tik)...(xin tin))))) 
                 APPEND (j n-j (i1 i2 ... ij) (ik ... in) "xo")
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
             (TypeOfRelAlgSymbol(nl->First(first.listExpr())) == stream) &&
             (nl->ListLength(nl->Second(first.listExpr())) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second
             (first.listExpr()))) == tuple) &&(nl->ListLength
             (nl->Second(first.listExpr())) == 2) && (IsTupleDescription
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
                      + arelName + " is already assigned to another attribute. "
                      "Please choose a new name and try again.");        
   CHECK_COND((nl->ListLength(second.listExpr()) > 0), 
   "Operator nest: Second argument list may not be empty" );
  
   //check that all attributes named in second argument appear in
   //the first argument, collect attributes of second 
   //argument in primary 
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
          "Operator nest: There must be at least one attribute other than arel "
          "in the first argument list.");
         return nl->SymbolAtom("typeerror");
   }
   CHECK_COND (!(primary.length() == first.second().second().length()),
     "Operator nest: there must be at least one attribute that should be nested"
     " in a subrelation.");    
   //check, if attributes in first argument exist in primary.
   //if not append to subrel
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
   ListExpr numbers = nl -> FiveElemList (nl -> IntAtom(numberlist1.length()), 
     nl -> IntAtom(numberlist2.length()), numberlist1.listExpr(), 
     numberlist2.listExpr(), nl->StringAtom(arelName));
   ListExpr nreldescription = nl -> TwoElemList (nl->SymbolAtom( "nrel" ),
      nl -> TwoElemList ( nl -> SymbolAtom( "tuple" ), primary.listExpr()));
   outlist = nl -> ThreeElemList (nl -> SymbolAtom ("APPEND"), numbers, 
             nreldescription);   
   return outlist;
}
/*
5.5.2 Value mapping function of operator ~nest~

*/
int
nestValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
   Supplier son;
   Word elem1, elem2;
   AttributeRelation* arel;
   NestedRelation* nrel = (NestedRelation*)(qp->ResultStorage(s).addr);
   nrel->getPrimary()->Clear();
   vector<SubRelation*>* vs = nrel->getSubRels();
   for (unsigned int i = 0; i < vs->size(); i++)
   {
     SubRelation* srel = vs->at(i);
     srel->rel->Clear();
   }
   Tuple* current;
   Tuple* lastTuple = 0;
   int primaryLength = ((CcInt*)args[3].addr)->GetIntval();   
   int subrelLength = ((CcInt*)args[4].addr)->GetIntval();
   string arelName = ((CcString*)args[7].addr)->GetValue();
   SubRelation* subrel = nrel->getSubRel(arelName);
   ListExpr tupletype = nl->Second(subrel->typeInfo);
   TupleType* currentType;
   int index;
   int nrelAlgId = am->GetAlgebraId("NestedRelationAlgebra");
   int arelId = NestedRelation::getTypeId(nrelAlgId, "arel"); 
   Tuple* tuple;
   qp->Open(args[0].addr);
   qp->Request(args[0].addr, elem1);
   while (qp->Received(args[0].addr))
   {
      int relIndex = -1;
      bool equal = true;
      current = (Tuple*)elem1.addr; 
      currentType = current->GetTupleType();   
      Tuple* subtuple = new Tuple (tupletype);
      if (lastTuple == 0)
      {
         tuple = new Tuple( nl->First(nrel->getTupleType()) );
         assert( tuple->GetNoAttributes() == primaryLength + 1);                    
         for (int i = 0; i < primaryLength; i++)
         {
           son = qp->GetSupplier(args[5].addr, i);
           qp->Request(son, elem2);
           index = ((CcInt*)elem2.addr)->GetIntval(); 
           if (currentType->GetAttributeType(index-1).algId == nrelAlgId 
               && currentType->GetAttributeType(index-1).typeId == arelId)
           {
             relIndex = relIndex + 1;
             AttributeRelation* temp1 = (AttributeRelation*)
                                      (current->GetAttribute(index - 1));
             AttributeRelation* b = storeSubRel(temp1, relIndex, nrel);
             tuple->PutAttribute(i, b);
           }
           else
           {
             tuple->CopyAttribute(index-1, current, i);  
           }
         }
         for (int i = 0; i < subrelLength; i++)
         {
            son = qp->GetSupplier(args[6].addr, i);
            qp->Request(son, elem2);
            index = ((CcInt*)elem2.addr)->GetIntval();
            if (currentType->GetAttributeType(index-1).algId == nrelAlgId 
                && currentType->GetAttributeType(index-1).typeId == arelId)
            {
               relIndex = nrel->getSubRelIndex(arelName) + 1;
               AttributeRelation* temp1 = (AttributeRelation*)
                                      (current->GetAttribute(index - 1));
               AttributeRelation* b = storeSubRel(temp1, relIndex, nrel);
               subtuple->PutAttribute(i, b);
            }
            else
            {
               subtuple->CopyAttribute(index-1, current, i);  
            }
         }
         arel = new AttributeRelation(subrel->fileId);
         arel->setPartOfNrel(true);
         subrel->rel->AppendTuple(subtuple);
         arel->Append(subtuple->GetTupleId());
         tuple->PutAttribute(primaryLength, arel);
         subtuple->DeleteIfAllowed();
         subtuple = 0;
         lastTuple = current;
      }
      else
      {
         Attribute* attr1;
         Attribute* attr2;
         for (int i = 0; i < primaryLength; i++)
         {
            son = qp->GetSupplier(args[5].addr, i);
            qp->Request(son, elem2);
            index = ((CcInt*)elem2.addr)->GetIntval();     
            attr1 = lastTuple->GetAttribute(index - 1);
            attr2 = current->GetAttribute(index - 1);
            if (!(attr1->Compare(attr2) == 0))
            { 
               equal = false;
               break;
            }
         }
         if (equal)
         {
            ListExpr tupletype = nl->Second(subrel->typeInfo);
            Tuple* subtuple = new Tuple (tupletype);
            for (int i = 0; i < subrelLength; i++)
            {
               son = qp->GetSupplier(args[6].addr, i);
               qp->Request(son, elem2);
               index = ((CcInt*)elem2.addr)->GetIntval();
               if (currentType->GetAttributeType(index-1).algId == nrelAlgId 
                && currentType->GetAttributeType(index-1).typeId == arelId)
               {
                 relIndex = nrel->getSubRelIndex(arelName) + 1;
                 AttributeRelation* temp1 = (AttributeRelation*)
                                      (current->GetAttribute(index - 1));
                 AttributeRelation* b = storeSubRel(temp1, relIndex, nrel);
                 subtuple->PutAttribute(i, b);
               }
               else
               {
                  subtuple->CopyAttribute(index-1, current, i);  
               }
            }  
            arel = (AttributeRelation*)(tuple->GetAttribute(primaryLength));
            subrel->rel->AppendTuple(subtuple);
            arel->Append(subtuple->GetTupleId());
            subtuple->DeleteIfAllowed();
            subtuple = 0;
            current->DeleteIfAllowed();
            current = 0;
         }
         else
         {
            nrel->getPrimary()->AppendTuple(tuple);
            tuple->DeleteIfAllowed();
            tuple = new Tuple( nl->First(nrel->getTupleType()));
            ListExpr tupletype = nl->Second(subrel->typeInfo);
            Tuple* subtuple = new Tuple (tupletype);
            assert( tuple->GetNoAttributes() == primaryLength + 1);                    
            assert( subtuple->GetNoAttributes() == subrelLength);
            for (int i = 0; i < primaryLength; i++)
            {
               son = qp->GetSupplier(args[5].addr, i);
               qp->Request(son, elem2);
               index = ((CcInt*)elem2.addr)->GetIntval(); 
               if (currentType->GetAttributeType(index-1).algId == nrelAlgId 
                && currentType->GetAttributeType(index-1).typeId == arelId)
               {
                 relIndex = relIndex + 1;
                 AttributeRelation* temp1 = (AttributeRelation*)
                                      (current->GetAttribute(index - 1));
                 AttributeRelation* b = storeSubRel(temp1, relIndex, nrel);
                 tuple->PutAttribute(i, b);
               }
               else
               {
                  tuple->CopyAttribute(index-1, current, i);  
               }
            }
            for (int i = 0; i < subrelLength; i++)
            {
               son = qp->GetSupplier(args[6].addr, i);
               qp->Request(son, elem2);
               index = ((CcInt*)elem2.addr)->GetIntval();
               if (currentType->GetAttributeType(index-1).algId == nrelAlgId 
                && currentType->GetAttributeType(index-1).typeId == arelId)
               {
                 relIndex = nrel->getSubRelIndex(arelName) + 1;
                 AttributeRelation* temp1 = (AttributeRelation*)
                                      (current->GetAttribute(index - 1));
                 AttributeRelation* b = storeSubRel(temp1, relIndex, nrel);
                 subtuple->PutAttribute(i, b);
               }
               else
               {
                  subtuple->CopyAttribute(index-1, current, i);  
               }
            }
            arel = new AttributeRelation(subrel->fileId);
            arel->setPartOfNrel(true);
            subrel->rel->AppendTuple(subtuple);
            arel->Append(subtuple->GetTupleId());
            subtuple->DeleteIfAllowed();
            subtuple = 0;
            tuple->PutAttribute(primaryLength, arel);
            lastTuple->DeleteIfAllowed();
            lastTuple = current;
         }   
      }
      qp->Request(args[0].addr, elem1);   
   }
  
   nrel->getPrimary()->AppendTuple(tuple);
   tuple->DeleteIfAllowed();
   if (!(lastTuple == 0))
     lastTuple->DeleteIfAllowed();
   result.setAddr(nrel);
   qp->Close(args[0].addr);
   return 0;         
}
/*
5.5.3 Specification of operator ~nest~

*/
struct nestInfo : OperatorInfo {

  nestInfo() : OperatorInfo()
  {
    name =      "nest";
    signature = "(stream x) -> (nrel y)";
    syntax =    "_ nest [xi1,..., xij; x0]";
    meaning =   "Creates a nested relation from a tuple stream. The stream "
                "should be sorted by the attributes that are to appear in the "
                "primary relation." ;
    example =   "query documents feed sortby [publisher] nest [publisher; "
                "publications]";
  }
};

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
     ErrorReporter::ReportError("Operator unnest: Attributename '" + attrname +
                    "' is not a known attributename in the tuple stream.");
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
  DBArray<TupleId>* tidArray;
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
      const TupleId* tid;
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
          rel = Relation::GetRelation(info->arel->getFileId());
          Tuple* arelTuple = rel->GetTuple(*tid);
          for (int i = 0; i < noAttrArel; i++)
            tuple->CopyAttribute(i, arelTuple, i + noOfAttrs - 1);       
          info->lastTuple = current;
          info->index = 1;
          result.setAddr(tuple);
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
          rel = Relation::GetRelation(info->arel->getFileId());
          Tuple* arelTuple = rel->GetTuple(*tid);           
          info->index++;
          for (int i = 0; i < noAttrArel; i++)
            tuple->CopyAttribute(i, arelTuple, i + noOfAttrs - 1);       
          result.setAddr(tuple);
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
            rel = Relation::GetRelation(info->arel->getFileId());
            Tuple* arelTuple = rel->GetTuple(*tid);   
            for (int i = 0; i < noAttrArel; i++)
              tuple->CopyAttribute(i, arelTuple, i + noOfAttrs -1);       
            info->lastTuple->DeleteIfAllowed();
            info->lastTuple = current;
            info->index = 1;
            result.setAddr(tuple);
            return YIELD;
          }
          else
          {
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
NestedRenameTypeMap( ListExpr args )
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

5.12.2 Value mapping function of operator ~rename~

*/
int
NestedRename(Word* args, Word& result, int message,
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

5.12.3 Specification of operator ~rename~

*/
const string RenameSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((stream (tuple([a1:d1, ... ,"
  "an:dn)))ar) -> (stream (tuple([a1ar:d1, "
  "... ,anar:dn)))</text--->"
  "<text>_ rename [ _ ] or just _ { _ }"
  "</text--->"
  "<text>Renames all attribute names by adding"
  " them with the postfix passed as parameter. "
  "NOTE: parameter must be of symbol type."
  "</text--->"
  "<text>query ten feed rename [ r1 ] consume "
  "or query ten feed {r1} consume, the result "
  "has format e.g. n_r1</text--->"
  ") )";

/*

5.12.4 Definition of operator ~rename~

*/
Operator nrelalgrename (
         "rename",               // name
         RenameSpec,             // specification
         NestedRename,                 // value mapping
         Operator::SimpleSelect, // trivial selection function
         NestedRenameTypeMap           // type mapping
);


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
      AddOperator (aConsumeInfo(), aConsume, aConsumeTypeMap);
      AddOperator (nestInfo(), nestValueMap, nestTypeMap);
      AddOperator (unnestOperatorInfo(), unnestValueMap, unnestTypeMap);
      AddOperator(&nrelalgrename);
      attributeRelationTC.AssociateKind( "DATA" );
    }
    ~NestedRelationAlgebra() {};
};

/*
7 Initialization

*/

extern "C"
Algebra*
InitializeNestedRelationAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new NestedRelationAlgebra());
}                                                                               
