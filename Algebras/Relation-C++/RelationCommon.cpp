/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Common Relation Algebra

March 2003 Victor Almeida created the new Relational Algebra organization.

[TOC]

1 Overview

The Relational Algebra basically implements two type constructors, namely ~tuple~ and ~rel~.
More information about the Relational Algebra can be found in the RelationAlgebra.h header
file.

Some functionalities are the same for both the Main Memory and the Persistent Relational
Algebra. These common functionalities belongs to this implementation file.

2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "SecondoSystem.h"
#include <set>

extern NestedList *nl;

/*
3 Implementation of the class ~TupleType~

A ~TupleType~ is a collection (an array) of all attribute types (~AttributeType~)
of the tuple. This structure contains the metadata of a tuple attributes.

*/
TupleType::TupleType( const ListExpr typeInfo ):
  noAttributes( nl->ListLength( nl->Second( typeInfo ) ) ),
  attrTypeArray( new AttributeType[noAttributes] ),
  totalSize( 0 )
{
  static AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  ListExpr rest = nl->Second( typeInfo );
  int i = 0;

  while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

    int algId = nl->IntValue( nl->First( nl->Second( first ) ) ),
        typeId = nl->IntValue( nl->Second( nl->Second( first ) ) ),
        size = (algM->SizeOfObj(algId, typeId))();

    totalSize += size;
    attrTypeArray[i++] = AttributeType( algId, typeId, size );
  }
  assert( i == noAttributes );
}

TupleType::TupleType( const TupleType& tupleType ):
  noAttributes( tupleType.GetNoAttributes() ),
  attrTypeArray( new AttributeType[tupleType.GetNoAttributes()] ),
  totalSize( 0 )
{
  for( int i = 0; i < noAttributes; i++ )
  {
    attrTypeArray[i] = tupleType.GetAttributeType( i );
    totalSize += attrTypeArray[i].size;
  }
}

TupleType::TupleType( const int noAttrs, AttributeType *attrs ):
  noAttributes( noAttrs ),
  attrTypeArray( attrs ),
  totalSize( 0 )
{
  for( int i = 0; i < noAttrs; i++ )
  {
    totalSize += attrs[i].size;
  }
}

TupleType::~TupleType()
{
  delete []attrTypeArray;
}

const int TupleType::GetNoAttributes() const
{
  return noAttributes;
}

TupleType *TupleType::Concat( const TupleType& t )
{
  int noAttrs = this->GetNoAttributes() + t.GetNoAttributes();
  AttributeType *attrTypes = new AttributeType[noAttrs]; 

  for( int i = 0; i < this->GetNoAttributes(); i++ )
    attrTypes[i] = this->GetAttributeType( i );
  for( int i = 0; i < t.GetNoAttributes(); i++ )
    attrTypes[this->GetNoAttributes()+i] = t.GetAttributeType( i );

  return new TupleType( noAttrs, attrTypes );
}

const AttributeType& TupleType::GetAttributeType( const int index ) const
{
  assert( index >= 0 && index < noAttributes );
  return attrTypeArray[index];
}

void TupleType::PutAttributeType( const int index, const AttributeType& attrType )
{
  assert( index >= 0 && index < noAttributes );
  attrTypeArray[index] = attrType;
}

/*
4 Auxilary Functions

4.1 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of
type ~RelationType~. ~Symbol~ is allowed to be any list. If it is not one
of these symbols, then the value ~error~ is returned.

*/
RelationType TypeOfRelAlgSymbol (ListExpr symbol)
{
  string s;

  if (nl->AtomType(symbol) == SymbolType)
  {
    s = nl->SymbolValue(symbol);
    if (s == "rel"   ) return rel;
    if (s == "tuple" ) return tuple;
    if (s == "stream") return stream;
    if (s == "map"   ) return ccmap;
    if (s == "bool"  ) return ccbool;
  }
  return error;
}

/*
3.2 Function ~FindAttribute~

Here ~list~ should be a list of pairs of the form (~name~,~datatype~).
The function ~FindAttribute~ determines whether ~attrname~ occurs as one of
the names in this list. If so, the index in the list (counting from 1)
is returned and the corresponding datatype is returned in ~attrtype~.
Otherwise 0 is returned. Used in operator ~attr~, for example.

*/
int FindAttribute( ListExpr list, string attrname, ListExpr& attrtype)
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
3.3 Function ~ConcatLists~

Concatenates two lists.

*/
ListExpr ConcatLists( ListExpr list1, ListExpr list2)
{
  if (nl->IsEmpty(list1))
  {
    return list2;
  }
  else
  {
    return nl->Cons(nl->First(list1), ConcatLists(nl->Rest(list1), list2));
  }
}

/*
3.5 Function ~AttributesAreDisjoint~

Checks wether two ListExpressions are of the form
((a1 t1) ... (ai ti)) and ((b1 d1) ... (bj dj))
and wether the ai and the bi are disjoint.

*/
bool AttributesAreDisjoint(ListExpr a, ListExpr b)
{
  set<string> aNames;
  ListExpr rest = a;
  ListExpr current;

  while(!nl->IsEmpty(rest))
  {
    current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType)
      && (nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
    {
      aNames.insert(nl->SymbolValue(nl->First(current)));
    }
    else
    {
      return false;
    }
  }
  rest = b;
  while(!nl->IsEmpty(rest))
  {
    ListExpr current = nl->First(rest);
    rest = nl->Rest(rest);
    if((nl->ListLength(current) == 2)
      && (nl->IsAtom(nl->First(current)))
      && (nl->AtomType(nl->First(current)) == SymbolType)
      && (nl->IsAtom(nl->Second(current)))
      && (nl->AtomType(nl->Second(current)) == SymbolType))
    {
      if(aNames.find(nl->SymbolValue(nl->First(current))) != aNames.end())
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  return true;
}

/*
3.6 Function ~Concat~

Copies the attribute values of two tuples
(words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void Concat (Word r, Word s, Word& t)
{
  int rnoattrs, snoattrs, tnoattrs;
  Attribute* attr;

  rnoattrs = ((Tuple*)r.addr)->GetNoAttributes();
  snoattrs = ((Tuple*)s.addr)->GetNoAttributes();
  tnoattrs = rnoattrs + snoattrs;

//VTA
  assert( ((Tuple*)t.addr)->GetNoAttributes() == tnoattrs );
//  ((Tuple*)t.addr)->SetNoAttributes(tnoattrs);

  for( int i = 0; i < rnoattrs; i++)
  {
    attr = ((Tuple*)r.addr)->GetAttribute(i);
    ((Tuple*)t.addr)->PutAttribute((i), ((StandardAttribute*)attr)->Clone());
  }
  for (int j = rnoattrs; j < tnoattrs; j++)
  {
    attr = ((Tuple*)s.addr)->GetAttribute(j - rnoattrs);
    ((Tuple*)t.addr)->PutAttribute((j), ((StandardAttribute*)attr)->Clone());
  }
}

/*
3.7 Function ~CompareNames~

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
    unique = std::count(attrnamestrlist.begin(), attrnamestrlist.end(),
                               attrname);
    *it =  attrname;
    if (unique) return false;
    it++;
  }
  return true;
}


