/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra Main Memory

[1] Using Storage Manager Berkeley DB 

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

[TOC]

1 Includes, Constants, Globals, Enumerations

*/
using namespace std;

#include "Algebra.h"
#include "AlgebraManager.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
// #include "DynamicLibrary.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <typeinfo>
#include "Tuple.h"

static NestedList* nl;
static QueryProcessor* qp;

const int MaxSizeOfAttr = 10;

enum RelationType { rel, tuple, stream, ccmap, ccbool, error };
/*

2 Auxilary Functions

1.2 Function ~TypeOfRelAlgSymbol~

Transforms a list expression ~symbol~ into one of the values of type ~RelationType~. ~Symbol~ is allowed to be any list. If it is not one of these symbols, then the value ~error~ is returned.

*/
static RelationType TypeOfRelAlgSymbol (ListExpr symbol) {

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

5.6 Function ~findattr~

Here ~list~ should be a list of pairs of the form (~name~,~datatype~). The function ~findattr~ determines whether ~attrname~ occurs as one of the names in this list. If so, the index in the list (counting from 1) is returned and the corresponding datatype is returned in ~attrtype~. Otherwise 0 is returned. Used in operator ~attr~. 
*/
int findattr( ListExpr list, string attrname, ListExpr& attrtype) 
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

5.6 Function ~ConcatLists~

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

The typeinfo list consists of three lists. The first list is a pair (AlgebraID, Constructor ID). The second list represents the attributelist of the tuple. This list is a sequence of pairs (attribute name (AlgebraID ConstructorID)). Here the ConstructorID is the identificator of a standard data type, e.g. int. The third list is an atom and counts the number of the tuple's attributes. 

1.3.1 Type property of type constructor ~tuple~

*/
static ListExpr TupleProp ()
{
  return (nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("plus"),
          nl->TwoElemList(nl->SymbolAtom("ident"), nl->SymbolAtom("DATA"))),
          nl->SymbolAtom("TUPLE")));
}
/*

1.3.1 Main memory representation

Each instance of the class defined below will be the main memory representation of a value of type ~tuple~.

		Figure 1: Main memory representation of a tuple (class ~CcTuple~) [tuple.eps]

*/
class TupleAttributesInfo
{
  private:
   
    TupleAttributes* tupleType;
    static TupleAttributesInfo tupleTypeInfo;
    
  public:
  
    TupleAttributesInfo (ListExpr typeInfo, ListExpr value)
    {
      ListExpr attrlist, valuelist,first,firstvalue, errorInfo;
      Word attr;
      int algebraId, typeId, noofattrs;
      AttributeType attrTypes[nl->ListLength(value)];
      AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
      bool valueCorrect;
      
      attrlist = typeInfo;
      valuelist = value;
      noofattrs = 0;
      
      while (!nl->IsEmpty(attrlist))
      {
        first = nl->First(attrlist);
        attrlist = nl->Rest(attrlist);

        algebraId = nl->IntValue(nl->First(nl->Second(first)));
        typeId = nl->IntValue(nl->Second(nl->Second(first)));
      
 	firstvalue = nl->First(valuelist);
	valuelist = nl->Rest(valuelist);
        attr = (algM->InObj(algebraId, typeId))(nl->Rest(first), 
                 firstvalue, 0, errorInfo, valueCorrect);
	if (valueCorrect)
	{
	  AttributeType attrtype = { algebraId, 0, ((Attribute*)attr.addr)->Sizeof() }; 
	  attrTypes[noofattrs] = attrtype;
	  noofattrs++;
	}
      }
      tupleType = new TupleAttributes(noofattrs, attrTypes);	  
    }
};

class CcTuple 
{
  private:

    int NoOfAttr;
    void* AttrList [MaxSizeOfAttr];

  public:

    CcTuple () { NoOfAttr = 0;
                 for (int i=0; i < MaxSizeOfAttr; i++) 
		   AttrList[i] = 0;
               };
    ~CcTuple () {};
    void* Get (int index) {return AttrList[index];};
    void  Put (int index, void* attr) {AttrList[index] = attr;};
    void  SetNoAttrs (int noattr) {NoOfAttr = noattr;};
    int   GetNoAttrs () {return NoOfAttr;};
  
    friend
    ostream& operator<<(ostream& s, CcTuple t);
};
/*

The next function supports writing objects of class CcTuple to standard output. It is only needed for internal tests.

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

1.3.2 ~Out~-function of type constructor ~tuple~

The ~out~-function of type constructor ~tuple~ takes as inputs a type description (~typeInfo~) of the tuples attribute structure in nested list format and a pointer to a tuple value, stored in main memory. The function returns the tuple value from main memory storage in nested list format. 

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

1.3.2 ~In~-function of type constructor ~tuple~

The ~in~-function of type constructor ~tuple~ takes as inputs a type description (~typeInfo~) of the tuples attribute structure in nested list format and the tuple value in nested list format. The function retrurns a pointer to atuple value, stored in main memory in accordance to the tuple value in nested list format. 

Error handling in ~InTuple~: ~Correct~ is only true if there is the right number of attribute values and all values have correct list representations. Otherwise the following error messages are added to ~errorInfo~:

----	(71 tuple 1 <errorPos>)		        atom instead of value list
	(71 tuple 2 <errorPos>)		        not enough values
	(71 tuple 3 <errorPos> <attrno>) 	wrong attribute value in 
					        attribute <attrno>
	(71 tuple 4 <errorPos>)		        too many values
----

is added to ~errorInfo~. Here ~errorPos~ is the number of the tuple in the relation list (passed by ~InRelation~).


*/
static Word InTuple(ListExpr typeInfo, ListExpr value,
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
  //cout << "InTuple: -> " << tupleaddr << endl;
  attrlist =  nl->Second(nl->First(typeInfo));
  valuelist = value;
  // cout << nl->WriteToFile("/dev/tty",attrlist) << endl;
  // cout << nl->WriteToFile("/dev/tty",valuelist) << endl;
  correct = true;
  if (nl->IsAtom(valuelist))
  {
    correct = false;
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
          tupleaddr->Put(attrno - 1, attr.addr);
          noOfAttrs++;
	}
	else
	{
	  correct = false;
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

1.3.4 ~Destroy~-function of type constructor ~tuple~

A type constructor's ~destroy~-function is used by the query processor in order to deallocate memory occupied by instances of Secondo objects. They may have been created in two ways:

  * as return values of operator calls

  * by calling a type constructor's ~create~-function.

The corresponding function of type constructor ~tuple~ is called ~DeleteTuple~.

*/
void DeleteTuple(Word& w)
{
  CcTuple* tupleptr;
  int attrno;
  // const char* typname;
  tupleptr = (CcTuple*)w.addr;
  attrno = tupleptr->GetNoAttrs();
  for (int i = 0; i <= (attrno - 1); i++)
  {
    // typname = typeid(*(tupleptr->Get(i))).name();
    // cout << typeid(*(tupleptr->Get(i))).name() << endl;
    
    // delete &(typeid(*(tupleptr->Get(i))));
    delete (TupleElement*)tupleptr->Get(i);
    //if (typeid(*(tupleptr->Get(i))) == typeid(CcInt))
    //{
      //cout << "Class CcInt" << endl;
      //delete (CcInt*)tupleptr->Get(i);
    //}
  }
  //cout << "DeleteTuple: -> " << tupleptr << endl;
  delete tupleptr;
}
/*

1.3.4 ~Check~-function of type constructor ~tuple~

Checks the specification:

----	(ident x DATA)+		-> TUPLE	tuple
----

with the additional constraint that all identifiers used (attribute names) must be distinct. Hence a tuple type has the form:

----	(tuple 
	    (
		(age x)
		(name y)))
----

and ~x~ and ~y~ must be types of kind DATA. Kind TUPLE introduces the following error codes:

----	(... 1) 	Empty tuple type
	(... 2 x)  	x is not an attribute list, but an atom
	(... 3 x)	Doubly defined attribute name x
	(... 4 x)	Invalid attribute name x
	(... 5 x)	Invalid attribute definition x (x is not a pair)
	(... 6 x)	Attribute type does not belong to kind DATA
----

*/
static bool CheckTuple(ListExpr type, ListExpr& errorInfo)
{
  vector<string> attrnamelist;
  ListExpr attrlist, pair;
  string attrname;
  bool correct, ckd;
  int unique;
  vector<string>::iterator it;
  AlgebraManager* algMgr;

  //cout << nl->WriteToFile("/dev/tty",type) << endl;
  if ((nl->ListLength(type) == 2) && (nl->IsEqual(nl->First(type), "tuple",
       true)))
  {
    attrlist = nl->Second(type);
    if (nl->IsEmpty(attrlist))
    {
      errorInfo = nl->Append(errorInfo,
	nl->ThreeElemList(nl->IntAtom(61), nl->SymbolAtom("TUPLE"),
	nl->IntAtom(1)));
      // cout << nl->WriteToFile("/dev/tty",errorInfo) << endl;
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
static void* CastTuple(void* addr)
{
  return ( 0 );
}
/*

1.3.3 ~Create~-function of type constructor ~tuple~

The function is used to allocate memory sufficient for keeping one instance of ~tuple~. The ~Size~-parameter is not evaluated.

*/
static Word CreateTuple(int Size)
{
  // cout << "CreateTuple" <<endl;
  CcTuple* tup;
  
  tup = new CcTuple();
  //cout << "CreateTuple: -> " << tup << endl;
  return (SetWord(tup));
}
/*

3.2.5 ~Model~-functions of type constructor ~tuple~
 
*/
static Word TupleInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

static ListExpr TupleOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

static Word TupleValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

static Word TupleValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  //errorInfo = 0;
  return (SetWord( Address( 0 ) ));
} 
/*

1.3.5 Defnition of type constructor ~tuple~

Eventually a type constructor is created by defining an instance of class ~TypeConstructor~. Constructor's arguments are the type constructor's name and the eleven functions previously defined.

*/
TypeConstructor cpptuple( "tuple",         TupleProp,
                        OutTuple,          InTuple,     CreateTuple,
                        DeleteTuple,       CastTuple,   CheckTuple,
			0,                 0,
			TupleInModel,      TupleOutModel,
			TupleValueToModel, TupleValueListToModel );
/*

1.4 Type constructor ~rel~

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
static ListExpr RelProp ()
{
  return (nl->TwoElemList(nl->OneElemList(nl->SymbolAtom("TUPLE")),
          nl->SymbolAtom("REL")));
}
/* 

1.3.1 Main memory representation

		Figure 2: Main memory representation of a relation (~Compact Table~) [relation.eps]

*/
typedef CTable<CcTuple*>* Relation;

class CcRel 
{
  private:

    int NoOfTuples;
    Relation TupleList;
    CTable<CcTuple*>::Iterator rs;

  public:

    CcRel () {NoOfTuples = 0; TupleList = new CTable<CcTuple*>(100);};
    ~CcRel () {};
    // void* Get (int index) {return AttrList[index];};
    // void  Put (int index, void* attr) {AttrList[index] = attr;};
    void    NewScan() {rs = TupleList->Begin();};
    bool    EndOfScan() {return (rs == TupleList->End());};
    void    NextScan() {(rs)++;};
    CcTuple* GetTuple() {return ((CcTuple*)(*rs));};
    void    AppendTuple (CcTuple* t) {TupleList->Add(t);};
    void    SetNoTuples (int notuples) {NoOfTuples = notuples;};
    int     GetNoTuples () {return NoOfTuples;};
  
};
/*

1.4.2 ~Out~-function of type constructor ~rel~

*/
ListExpr OutRel(ListExpr typeInfo, Word  value) 
{
  // CTable<CcTuple>::Iterator rs;
  CcTuple* t;
  ListExpr l, lastElem, tlist, TupleTypeInfo;
  
  // cout << "OutRel" << endl;

  // cout << nl->WriteToFile("/dev/tty",typeInfo) << endl;
      
  // Relation r = (Relation)(value.addr);
  CcRel* r = (CcRel*)(value.addr);

  // rs = r->Begin();
  
    //CcTuple tup;
    //r->NewScan();
    //for (int i = 1; i <= 4;i++)
    //{
      //tup = r->GetTuple();
      // cout << tup << endl;
      //r->NextScan();
    //}

  r->NewScan();
  l = nl->TheEmptyList();
 
  // while (rs != r->End())
  while (!r->EndOfScan())
  {
    // t = (CcTuple)*rs;
    t = r->GetTuple();
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
    // rs++;
    r->NextScan();
  }
  // cout << nl->WriteToFile("/dev/tty",l);
  return l;
  
}
/*

1.3.3 ~Create~-function of type constructor ~rel~

The function is used to allocate memory sufficient for keeping one instance of ~rel~. The ~Size~-parameter is not evaluated.

*/
static Word CreateRel(int Size)
{

  // cout << "CreateRel" << endl;
  // CTable<CcTuple>* rel;
  CcRel* rel = new CcRel();
  // rel = new CTable<CcTuple>(100);
  //cout << "CreateRel: -> " << rel << endl;
  return (SetWord(rel));
}
/*

1.4.2 ~In~-function of type constructor ~rel~

~value~ is the list representation of the relation. The structure of ~typeInfol~ and ~value~ are described above. Error handling in ~InRel~:

The result relation will contain all tuples that have been converted correctly (have correct list expressions). For all other tuples, an error message containing the position of the tuple within this relation (list) is added to ~errorInfo~. (This is done by procedure ~InTuple~ called by ~InRel~). If any tuple representation is wrong, then ~InRel~ will return ~correct~ as FALSE and will itself add an error message of the form

----	(InRelation <errorPos>)
----

to ~errorInfo~. The value in ~errorPos~ has to be passed from the environment; probably it is the position of the relation object in the list of database objects.

*/
static Word InRel(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct)
{
  ListExpr tuplelist, TupleTypeInfo, first;
  // Relation rel;
  CcRel* rel;
  CcTuple* tupleaddr;
  int tupleno, count;
  bool tupleCorrect;
  
  correct = true;
  count = 0;
  
  // cout << "InRel" << endl;
  
  // cout << nl->WriteToFile("/dev/tty",typeInfo) << endl;
  // cout << nl->WriteToFile("/dev/tty",value) << endl;
    
  // rel = (Relation)((CreateRel(50)).addr);
  //rel = (CcRel*)((CreateRel(50)).addr);
  rel = new CcRel();
  //cout << "InRel: -> " << rel << endl;
  tuplelist = value;
  TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
	  nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
  tupleno = 0;
  if (nl->IsAtom(value))
  {
    correct = false;
    errorInfo = nl->Append(errorInfo,
	nl->ThreeElemList(nl->IntAtom(70), nl->SymbolAtom("rel"), tuplelist));
    return SetWord(rel);
  }
  else
  { // increase tupleno
    while (!nl->IsEmpty(tuplelist))
    {
      first = nl->First(tuplelist);
      tuplelist = nl->Rest(tuplelist);
      tupleno++;
      tupleaddr = (CcTuple*)(InTuple(TupleTypeInfo, first, tupleno, 
        errorInfo, tupleCorrect).addr);
      //cout << "InRelTuples1 -> " << tupleaddr << endl;

      //cout << (*(CcInt*)(tupleaddr->Get(0))).GetIntval() << endl;
 
      if (tupleCorrect)
      {
        // rel->Add(*((CcTuple*)tupleaddr));
	rel->AppendTuple(tupleaddr);
	count++;
        // delete (CcTuple*)tupleaddr;
      }
      else
      {
	correct = false;
      }
    }
    if (!correct)
    {
      errorInfo = nl->Append(errorInfo,
	nl->TwoElemList(nl->IntAtom(72), nl->SymbolAtom("rel")));
    }
    else rel->SetNoTuples(count);
    
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[1])).Get(0))).GetIntval() << endl;
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[2])).Get(0))).GetIntval() << endl;
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[3])).Get(0))).GetIntval() << endl;
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[4])).Get(0))).GetIntval() << endl;
    // CcTuple tup;
    rel->NewScan();
    //for (int i = 1; i <= 4;i++)
    while (! rel->EndOfScan())
    {
      //tup = rel->GetTuple();
      //cout << "InRelTuples2 -> " << rel->GetTuple() << endl;
      rel->NextScan();
    }
    return (SetWord((void*)rel));
  }
  
}
/*

1.3.4 ~Destroy~-function of type constructor ~rel~


The corresponding function of type constructor ~rel~ is called ~DeleteRel~.

*/
void DeleteRel(Word& w)
{

  // CTable<CcTuple>::Iterator rs;
  CcTuple* t;
  // Relation r;
  CcRel* r;
  Word v;
    
  // r = (Relation)w.addr;
  r = (CcRel*)w.addr;
  // rs = r->Begin();
  r->NewScan();
  // while (rs != r->End())
  while (!r->EndOfScan())
  {
    //cout << "while" << endl;
    // t = (CcTuple)*rs;
    t = r->GetTuple();
    //cout << "DeleteRelbeforeTuple: -> " << t << endl;

    v = SetWord(t);
    DeleteTuple(v);
    // rs++;
    r->NextScan();
  }
  //cout << "DeleteRel: -> " << r << endl;
  delete r;
}
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
static bool CheckRel(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;
  
  // cout << nl->WriteToFile("/dev/tty", type);
  
  // cout << "CheckRel" << endl;
  
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
static void* CastRel(void* addr)
{
  return ( 0 );
}
/*

3.2.5 ~Model~-functions of type constructor ~rel~
 
*/
static Word RelInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

static ListExpr RelOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

static Word RelValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

static Word RelValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  //errorInfo = 0;
  return (SetWord( Address( 0 ) ));
}
/*

1.3.5 Defnition of type constructor ~tuple~

Eventually a type constructor is created by defining an instance of class ~TypeConstructor~. Constructor's arguments are the type constructor's name and the eleven functions previously defined.

*/
TypeConstructor cpprel( "rel",           RelProp,
                        OutRel,          InRel,   CreateRel,
                        DeleteRel,       CastRel,   CheckRel,
			0, 0,
			RelInModel,      RelOutModel,
			RelValueToModel, RelValueListToModel );
/*

4 Operators

4.2 Selection function for non-overloaded operators

For non-overloaded operators, the set of value mapping functions consists
of exactly one element.  Consequently, the selection function of such an
operator always returns 0.
 
*/
static int simpleSelect (ListExpr args) { return 0; }
/*

6.1 Type Operator ~TUPLE~

Type operators are used only for inferring argument types of parameter functions. They have a type mapping but no evaluation function.
 
6.1.1 Type mapping function of operator ~TUPLE~
 
Extract tuple type from a stream or relation type given as the first argument.

----    ((stream x) ...)                -> x
        ((rel x)    ...)                -> x
----
 
*/
ListExpr TUPLETypeMap(ListExpr args)
{
  ListExpr first;
  // cout << nl->WriteToFile("/dev/tty",args);
  if(nl->ListLength(args) >= 1)
  {
    first = nl->First(args);
    if(nl->ListLength(first) == 2  )
    {
      if ((TypeOfRelAlgSymbol(nl->First(first)) == stream)  ||
          (TypeOfRelAlgSymbol(nl->First(first)) == rel))
        return nl->Second(first);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.3 Specification of operator ~TUPLE~

*/
const string TUPLESpec =
  "(<text>((stream x)...) -> x, ((rel x)...) -> x</text---><text>Extract tuple type from a stream or relation type given as the first argument.</text--->)";
/*

4.1.3 Definition of operator ~TUPLE~

*/
Operator TUPLE (
         "TUPLE",              // name
         TUPLESpec,            // specification
         0,                    // no value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         TUPLETypeMap          // type mapping
);
/*

6.1 Type Operator ~TUPLE2~

6.1.1 Type mapping function of operator ~TUPLE2~
 
Extract tuple type from a stream or relation type given as the second argument.

----    ((stream x) (stream y) ...)          -> y
        ((rel x) (rel y) ...)                -> y
----
 
*/
ListExpr TUPLE2TypeMap(ListExpr args)
{
  ListExpr second;
  // cout << nl->WriteToFile("/dev/tty",args);
  if(nl->ListLength(args) >= 2)
  {
    second = nl->Second(args);
    if(nl->ListLength(second) == 2  )
    {
      if ((TypeOfRelAlgSymbol(nl->First(second)) == stream)  ||
          (TypeOfRelAlgSymbol(nl->First(second)) == rel))
        return nl->Second(second);
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.3 Specification of operator ~TUPLE2~

*/
const string TUPLE2Spec =
  "(<text>((stream x) (stream y) ...) -> y, ((rel x) (rel y) ...) -> y</text---><text>Extract tuple type from a stream or relation type given as the second argument.</text--->)";
/*

4.1.3 Definition of operator ~TUPLE2~

*/
Operator TUPLE2 (
         "TUPLE2",             // name
         TUPLE2Spec,           // specification
         0,                    // no value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         TUPLE2TypeMap         // type mapping
);
/*

4.1 Operator ~feed~

Produces a stream from a relation by scanning the relation tuple by tuple.

4.1.1 Type mapping function of operator ~feed~

A type mapping function takes a nested list as argument. Its contents are 
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

Result type of feed operation.

----	((rel x))		-> (stream x)
---- 

*/
static ListExpr FeedTypeMap(ListExpr args)
{
  ListExpr first ;

  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);
    if(nl->ListLength(first) == 2)
    {
      if (TypeOfRelAlgSymbol(nl->First(first)) == rel) 
        return nl->Cons(nl->SymbolAtom("stream"), nl->Rest(first));
    }
  } 
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~feed~

*/
static int
Feed(Word* args, Word& result, int message, Word& local, Supplier s)
{
 //CTable<CcTuple>::Iterator* rs;
 CcRel* r;

  switch (message)
  {
    case OPEN :

      //rs = new CTable<CcTuple>::Iterator::Iterator();
      //*rs = (*((Relation)args[0].addr)).Begin();
      r = ((CcRel*)args[0].addr);
      r->NewScan();
      local.addr = r;
      return 0;

    case REQUEST :
    
      //rs = (CTable<CcTuple>::Iterator*)local.addr;
      r = (CcRel*)local.addr;
      //if (!((*rs).EndOfScan()))
      if (!(r->EndOfScan()))
      {
        //result.addr = &(**rs);
	result = SetWord(r->GetTuple());
        //(*rs)++;
	r->NextScan();
        return YIELD;
      }
      else return CANCEL;
     
    case CLOSE :
    
      //rs = (CTable<CcTuple>::Iterator*)local.addr;
      // rs = *((CTable<CCTuple>::Iterator**)local);
      //delete rs;
      return 0;
  }
}
/*

4.1.3 Specification of operator ~feed~

*/
const string FeedSpec =
  "(<text>(rel x) -> (stream x)</text---><text>Produces a stream from a relation by scanning the relation tuple by tuple.</text--->)";
/*

4.1.3 Definition of operator ~feed~

Non-overloaded operators are defined by constructing a new instance of class~Operator~, passing all operator functions as constructor arguments. 

*/
Operator feed (
         "feed",                // name
	 FeedSpec,              // specification
	 Feed,                  // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 FeedTypeMap           // type mapping
);
/*
4.1 Operator ~consume~

Collects objects from a stream into a relation.

4.1.1 Type mapping function of operator ~consume~

Operator ~consume~ accepts a stream of tuples and returns a relation.


----    (stream  x)                 -> ( rel x)
----

*/
ListExpr ConsumeTypeMap(ListExpr args)
{
  ListExpr first ;
  
  if(nl->ListLength(args) == 1)
  {
    first = nl->First(args);
    if(nl->ListLength(first) == 2)
    {
      if (TypeOfRelAlgSymbol(nl->First(first)) == stream) 
        return nl->Cons(nl->SymbolAtom("rel"), nl->Rest(first));
    }
  } 
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~consume~

*/
static int
Consume(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word actual;
  //Relation rel;
  CcRel* rel;
  // int catentry;
  // int* catentryptr;
   
  //rel = (Relation)(CreateRel(50).addr);
  rel = new CcRel();
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    //rel->Add(*((CcTuple*)actual.addr));
    rel->AppendTuple(((CcTuple*)actual.addr));
    qp->Request(args[0].addr, actual);
  }
  // catentry = ctreldb.Add((void*)rel);
  // catentryptr = new int(catentry);
  
  result = SetWord((void*) rel);

  // *result = (void*)catentryptr;
  qp->Close(args[0].addr);
  return 0;  
}
/*

4.1.3 Specification of operator ~consume~

*/
const string ConsumeSpec =
  "(<text>(stream x) -> (rel x)</text---><text>Collects objects from a stream into a relation.</text--->)";
/*

4.1.3 Definition of operator ~consume~

*/
Operator consume (
         "consume",            // name
	 ConsumeSpec,          // specification
	 Consume,              // value mapping
	 Operator::DummyModel, // dummy model mapping, defines in Algebra.h
	 simpleSelect,         // trivial selection function
	 ConsumeTypeMap        // type mapping
);
/*

7.1 Operator ~attr~

7.1.1 Type mapping function of operator ~attr~

Result type attr operation.

----
    ((tuple ((x1 t1)...(xn tn))) xi)    -> ti
                            APPEND (i) ti)
----
This type mapping uses a special feature of the query processor, in that if requests to append a further argument to the given list of arguments, namely, the index of the attribute within the tuple. This indes is computed within the type mapping  function. The request is given through the result expression of the type mapping which has the form, for example,

---- 

    (APPEND (1) string)

----

The keyword ~APPEND~ occuring as a first element of a returned type expression tells the query processor to add the elements of the following list - the second element of the type expression - as further arguments to the operator (as if they had been written in the query). The third element  of the query is then used as the real result type. In this case 1 is the index of the attribute determined in this procedure. The query processor, more precisely the procedure ~anotate~ there, will produce the annotation for the constant 1, append it to the list of annotated arguments, and then use "string" as the result type of the ~attr~ operation.

*/
ListExpr AttrTypeMap(ListExpr args)
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;
  // cout << nl->WriteToFile("/dev/tty",args);
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);
            
    if((nl->ListLength(first) == 2  ) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == tuple)  &&
        (nl->IsAtom(second)) &&
        (nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = findattr(nl->Second(first), attrname, attrtype);
      if (j)
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                  nl->OneElemList(nl->IntAtom(j)), attrtype);
    }
    return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~attr~

The argument vector ~arg~ contains in the first slot ~args[0]~ the tuple and in ~args[2]~ the position of the attribute as a number. Returns as ~result~ the value of an attribute at the given position ~args[2]~ in a tuple object. The attribute name is argument 2 in the query and is used in the function ~AttributeTypeMap~ to determine the attribute number ~args[2]~ .

*/
static int
Attr(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CcTuple* tupleptr;
  int index;
  
  tupleptr = (CcTuple*)args[0].addr;
  index = (int)((StandardAttribute*)args[2].addr)->GetValue();
  if ((1 <= index) && (index <= tupleptr->GetNoAttrs()))   
  {
    result = SetWord(tupleptr->Get(index - 1));
    return 0;
  }
  else
  { 
    cout << "attribute: index out of range !";
    return -1;
  }         
}
/*

4.1.3 Specification of operator ~attr~

*/
const string AttrSpec =
  "(<text>((tuple ((x1 t1)...(xn tn))) xi)  -> ti)</text---><text>Returns the value of an attribute at a given position in a tuple object.</text--->)";
/*

4.1.3 Definition of operator ~attr~

*/
Operator attr (
         "attr",           // name
     AttrSpec,        // specification
     Attr,            // value mapping
     Operator::DummyModel, // dummy model mapping, defines in Algebra.h
     simpleSelect,         // trivial selection function
     AttrTypeMap      // type mapping
);
/*

7.3 Operator ~filter~

Only tuples, fulfilling a certain condition are passed on to the output stream.

7.3.1 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream x) (map x bool))       -> (stream x)
----

*/
ListExpr FilterTypeMap(ListExpr args)
{
  ListExpr first, second;
  // cout << nl->WriteToFile("/dev/tty",args);
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);

    if ((nl->ListLength(first) == 2 && nl->ListLength(second) == 3 ) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream)  &&
    (TypeOfRelAlgSymbol(nl->First(second)) == ccmap)    &&
    (TypeOfRelAlgSymbol(nl->Third(second)) == ccbool)  &&
    (nl->Equal(nl->Second(first),nl->Second(second))))
      return first;
    else
      return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~filter~

*/
static int
Filter(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool found;
  Word elem, funresult;
  ArgVectorPointer funargs;

  switch ( message )
  {

    case OPEN:

      qp->Open (args[0].addr);
      return 0;

    case REQUEST:

      funargs = qp->Argument(args[1].addr);
      qp->Request(args[0].addr, elem);
      found = false;
      while (qp->Received(args[0].addr) && !found)
      {    
        (*funargs)[0] = elem;
        qp->Request(args[1].addr, funresult);
        if (((StandardAttribute*)funresult.addr)->IsDefined())
          found = (bool)((StandardAttribute*)funresult.addr)->GetValue();
        if (!found)
        {
        // delete(elem);
        qp->Request(args[0].addr, elem);
        }
      }
      if (found)
      {
        result = elem;
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE:

      qp->Close(args[0].addr);
      return 0;
  }
}
/*

4.1.3 Specification of operator ~filter~

*/
const string FilterSpec =
  "(<text>((stream x) (map x bool)) -> (stream x)</text---><text>Only tuples, fulfilling a certain condition are passed on to the output stream.</text--->)";
/*

4.1.3 Definition of operator ~filter~

*/
Operator tfilter (
         "tfilter",            // name
         FilterSpec,           // specification
         Filter,               // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         FilterTypeMap         // type mapping
);
/*

7.3 Operator ~project~

7.3.1 Type mapping function of operator ~filter~

Result type of project operation.

----	((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik))	-> 

		(APPEND
			(k (i1 ... ik))
			(stream (tuple ((ai1 Ti1) ... (aik Tik))))
		)
----

The type mapping computes the number of attributes and the list of attribute numbers for the given projection attributes and asks the query processor to append it to the given arguments.

*/
ListExpr ProjectTypeMap(ListExpr args)
{
  bool firstcall;
  int noAttrs, j;
  ListExpr first, second, first2, attrtype, newAttrList, lastNewAttrList, 
           lastNumberList, numberList, outlist;
  string attrname;
  
  firstcall = true;
  // cout << nl->WriteToFile("/dev/tty",args) << endl;
  if (nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second = nl->Second(args);
    
    if ((nl->ListLength(first) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
	(nl->ListLength(nl->Second(first)) == 2) &&
	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
	(!nl->IsAtom(second)) &&
	(nl->ListLength(second) > 0))
    {
      noAttrs = nl->ListLength(second);
      while (!(nl->IsEmpty(second)))
      {
        first2 = nl->First(second);
	second = nl->Rest(second);
	if (nl->AtomType(first2) == SymbolType)
	{ 
	  attrname = nl->SymbolValue(first2);
	}
	else return nl->SymbolAtom("typeerror");
	j = findattr(nl->Second(nl->Second(first)), attrname, attrtype);
	if (j)
	{
	  if (firstcall)
	  {
	    firstcall = false;
	    newAttrList = nl->OneElemList(nl->TwoElemList(first2, attrtype));
	    lastNewAttrList = newAttrList;
	    numberList = nl->OneElemList(nl->IntAtom(j));
	    lastNumberList = numberList;
	  }
	  else
	  {
	    lastNewAttrList =
	      nl->Append(lastNewAttrList, nl->TwoElemList(first2, attrtype));
	    lastNumberList =  
	      nl->Append(lastNumberList, nl->IntAtom(j));
	  }
	}
	else return nl->SymbolAtom("typeerror");
      }
      // Check whether all new attribute names are distinct
      // - not yet implemented
      outlist = nl->ThreeElemList(
                 nl->SymbolAtom("APPEND"),
		 nl->TwoElemList(nl->IntAtom(noAttrs), numberList),
		 nl->TwoElemList(nl->SymbolAtom("stream"),
		               nl->TwoElemList(nl->SymbolAtom("tuple"),
			                     newAttrList)));
      // cout << nl->WriteToFile("/dev/tty",outlist) << endl;
      return outlist;
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~project~

*/
static int
Project(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem1, elem2;
  int noOfAttrs, index;
  Supplier son;
  void* attr;
  CcTuple* t;
  

  switch (message)
  {
    case OPEN :
      
      qp->Open(args[0].addr);
      return 0;
      
    case REQUEST :
    
      qp->Request(args[0].addr, elem1);
      if (qp->Received(args[0].addr))
      {
        //result = qp->ResultStorage(s);
	t = new CcTuple();
        noOfAttrs = ((CcInt*)args[2].addr)->GetIntval();
        //((CcTuple*)result.addr)->SetNoAttrs(noOfAttrs);
	t->SetNoAttrs(noOfAttrs);
        for (int i=1; i <= noOfAttrs; i++)
        {
          son = qp->GetSupplier(args[3].addr, i-1);
          qp->Request(son, elem2);
          index = ((CcInt*)elem2.addr)->GetIntval();
	  attr = ((CcTuple*)elem1.addr)->Get(index-1);
          //((CcTuple*)result.addr)->Put(i-1, attr);
	  t->Put(i-1, attr); 
        }
	result = SetWord(t);
        return YIELD;
      }
      else return CANCEL;

    case CLOSE :

      qp->Close(args[0].addr);
      return 0;
  }
}
/*

4.1.3 Specification of operator ~project~

*/
const string ProjectSpec =
  "(<text>((stream (tuple ((x1 T1) ... (xn Tn)))) (ai1 ... aik)) -> (stream (tuple ((ai1 Ti1) ... (aik Tik))))</text---><text>Produces a projection tuple for each tuple of its input stream.</text--->)";
/*

4.1.3 Definition of operator ~project~

*/
Operator project (
         "project",            // name
         ProjectSpec,          // specification
         Project,              // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         ProjectTypeMap        // type mapping
);
/*

7.3 Operator ~product~

5.6.1 Help Function ~Concat~

Copies the attribute values of two tuples (words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void Concat (Word r, Word s, Word& t)
{
  int rnoattrs, snoattrs, tnoattrs;
  void* attr;
  
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
    ((CcTuple*)t.addr)->Put((i - 1), attr);
  }
  for (int j = (rnoattrs + 1); j <= tnoattrs; j++)
  {
    attr = ((CcTuple*)s.addr)->Get(j - rnoattrs - 1);
    ((CcTuple*)t.addr)->Put((j - 1), attr);
  }
}  
/*

7.3.1 Type mapping function of operator ~product~

Result type of product operation.

----	((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym))))
	
	-> (stream (tuple (x1 ... xn y1 ... ym)))
---- 

*/
ListExpr ProductTypeMap(ListExpr args)
{
  ListExpr first, second, list, list1, list2, outlist;
  
  if (nl->ListLength(args) == 2) 
  {
    first = nl->First(args); second = nl->Second(args);

    // Check first argument and extract list1 
    if (nl->ListLength(first) == 2) 
    {
      if (TypeOfRelAlgSymbol(nl->First(first)) == stream) 
      {
        if (nl->ListLength(nl->Second(first)) == 2)
	{
          if (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) 
	  {
            list1 = nl->Second(nl->Second(first));
	  }
          else return nl->SymbolAtom("typeerror");
	}
        else return nl->SymbolAtom("typeerror");
      }
      else return nl->SymbolAtom("typeerror");
    }
    else return nl->SymbolAtom("typeerror");
  
    // Check second argument and extract list2 
    if (nl->ListLength(second) == 2) 
    {
      if (TypeOfRelAlgSymbol(nl->First(second)) == stream) 
      {
        if (nl->ListLength(nl->Second(second)) == 2)
	{
          if (TypeOfRelAlgSymbol(nl->First(nl->Second(second))) == tuple) 
	  {
            list2 = nl->Second(nl->Second(second));
	  }
          else return nl->SymbolAtom("typeerror");
	}
        else return nl->SymbolAtom("typeerror");
      }
      else return nl->SymbolAtom("typeerror");
    }
    else return nl->SymbolAtom("typeerror");
    
    list = ConcatLists(list1, list2);
    // Check whether all new attribute names are distinct
    // - not yet implemented
    
    outlist = nl->TwoElemList(nl->SymbolAtom("stream"), 
      nl->TwoElemList(nl->SymbolAtom("tuple"), list));
    return outlist;
  }
  else return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~product~

*/
static int
Product(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, u, t;
  
  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);
      qp->Request(args[0].addr, r);
      local = (qp->Received(args[0].addr)) ? r : SetWord(Address(0));
      return 0;
      
    case REQUEST :
    
      if (local.addr == 0)
      {
        qp->Close(args[1].addr);
	return CANCEL;
      }
      else
      {
        r = local;
	qp->Request(args[1].addr, u);
	if (qp->Received(args[1].addr))
	{
	  //t = qp->ResultStorage(s);
	  t = SetWord(new CcTuple());
	  Concat(r, u, t);
	  result = t;
	  return YIELD;
	}
	else
	// second stream exhausted and closed now; must get a
	// new tuple from the first stream and restart second stream
	{
	  qp->Request(args[0].addr, r);
          if (qp->Received(args[0].addr))
	  {
	    local = r;
	    qp->Open(args[1].addr);
	    qp->Request(args[1].addr, u);
	    if (!qp->Received(args[1].addr)) // second stream is empty
	    {
	      qp->Close(args[0].addr);
	      return CANCEL;
	    }
	    else
	    {
	      //t = qp->ResultStorage(s);
	      t = SetWord(new CcTuple());
	      Concat(r, u, t);
	      result = t;
	      return YIELD;
	    }
	  }
	  else return CANCEL; // first stream exhausted
	}
      }
      
    case CLOSE :
    
      qp->Close(args[0].addr); 
      qp->Close(args[1].addr);
      return 0;
  }   
}
/*

4.1.3 Specification of operator ~product~

*/
const string ProductSpec =
  "(<text>((stream (tuple (x1 ... xn))) (stream (tuple (y1 ... ym)))) -> (stream (tuple (x1 ... xn y1 ... ym)))</text---><text>Computes a Cartesian product stream from its two argument streams.</text--->)";
/*

4.1.3 Definition of operator ~product~

*/
Operator product (
         "product",            // name
         ProductSpec,          // specification
         Product,              // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         ProductTypeMap        // type mapping
);
/*

7.3 Operator ~cancel~

Transmits tuple from its input stream to its output stream until a tuple arrives fulfilling some condition.

7.3.1 Type mapping function of operator ~cancel~

Type mapping for ~cancel~ is the same, as type mapping for operator ~flter~.
Result type of cancel operation.

----    ((stream x) (map x bool)) -> (stream x)
----

4.1.2 Value mapping function of operator ~cancel~

*/
static int
Cancel(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  bool found;
  ArgVectorPointer vector;
  
  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :
    
      qp->Request(args[0].addr,t);
      found= false;
      if (qp->Received(args[0].addr))
      {
        vector = qp->Argument(args[1].addr);
	(*vector)[0] = t;
	qp->Request(args[1].addr, value);
	found = ((CcBool*)value.addr)->GetBoolval();
	if (found)
	{
	  qp->Close(args[0].addr);
	  return CANCEL;
	}
	else
	{
	  result = t;
	  return YIELD;
	}
      }
      else return CANCEL;
      
    case CLOSE :
    
      qp->Close(args[0].addr);
      return 0;
  }
}
/*

4.1.3 Specification of operator ~cancel~

*/
const string CancelSpec =
  "(<text>((stream x) (map x bool)) -> (stream x)</text---><text>Transmits tuple from its input stream to its output stream until a tuple arrives fulfilling some condition.</text--->)";
/*

4.1.3 Definition of operator ~cancel~

*/
Operator cancel (
         "cancel",             // name
         CancelSpec,           // specification
         Cancel,               // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         FilterTypeMap         // type mapping
);
/*

7.3 Operator ~tcount~

Count the number of tuples within a stream of tuples.

7.3.1 Type mapping function of operator ~tcount~

Type mapping for ~tcount~ is

----	((stream (tuple x))) -> int
----

*/
static ListExpr
TCountTypeMap( ListExpr args )
{
  ListExpr arg11, arg12;
  
  if ( nl->ListLength(args) == 1 )
  {
    arg11 = nl->First(nl->First(args));
    arg12 = nl->Second(nl->First(args));
    if (nl->IsEqual(arg11, "stream") &&
          nl->IsEqual(nl->First(arg12), "tuple"))
      return nl->SymbolAtom("int");
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~tcount~

*/
static int
TCount(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  int count = 0;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) )
  {
    count++;
    qp->Request(args[0].addr, elem);
  }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);		
  qp->Close(args[0].addr);
  return 0;
}
/*

4.1.3 Specification of operator ~tcount~

*/
const string TCountSpec =
  "(<text>((stream (tuple x))) -> int</text---><text>Count number of tuples within a stream of tuples.</text--->)";
/*

4.1.3 Definition of operator ~tcount~

*/
Operator tcount (
         "tcount",             // name
         TCountSpec,           // specification
         TCount,               // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         TCountTypeMap         // type mapping
);
/*

7.3 Operator ~rename~

Renames all attribute names by adding them with the postfix passed as parameter.

7.3.1 Type mapping function of operator ~rename~

Type mapping for ~rename~ is

----	((stream (tuple([a1:d1, ... ,an:dn)))ar) -> (stream (tuple([a1ar:d1, ... ,anar:dn)))
----

*/
static ListExpr
RenameTypeMap( ListExpr args )
{
  ListExpr first, first2, second, attrtype, rest, listn, lastlistn;
  string  attrname;
  string  attrnamen;
  bool firstcall = true;
  //nl->WriteToFile("/dev/tty",args);
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);
			
    if((nl->ListLength(first) == 2  ) &&
    	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
	(TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
       	nl->IsAtom(second) &&
	(nl->AtomType(second) == SymbolType)) 
    {
      rest = nl->Second(nl->Second(first));
      while (!(nl->IsEmpty(rest)))
      {
	first2 = nl->First(rest);
	rest = nl->Rest(rest);
	nl->SymbolValue(nl->First(first));
	attrname = nl->SymbolValue(nl->First(first2));
	attrnamen = nl->SymbolValue(second);
	attrname.append(attrnamen);
			
	if (!firstcall)
	{
	  lastlistn  = 
	    nl->Append(lastlistn,nl->TwoElemList(nl->SymbolAtom(attrname), nl->Second(first2)));
	}
	else
	{
	  firstcall = false;			
 	  listn = nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(attrname),nl->Second(first2)));
	  lastlistn = listn;
	}
      }
      //nl->WriteToFile("/dev/tty",listn);
      return 
        nl->TwoElemList(nl->SymbolAtom("stream"),
		nl->TwoElemList(nl->SymbolAtom("tuple"),
		listn));
    }
    return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~rename~

*/
static int
Rename(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
    
  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :
    
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        result = t;
	return YIELD;
      }
      else return CANCEL;
      
    case CLOSE :
    
      qp->Close(args[0].addr);
      return 0;
  }
}
/*

4.1.3 Specification of operator ~rename~

*/
const string RenameSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn)))ar) -> (stream (tuple([a1ar:d1, ... ,anar:dn)))</text---><text>Renames all attribute names by adding them with the postfix passed as parameter. NOTE: parameter must be of symbol type.</text--->)";
/*

4.1.3 Definition of operator ~rename~

*/
Operator cpprename (
         "rename",             // name
         RenameSpec,           // specification
         Rename,               // value mapping
         Operator::DummyModel, // dummy model mapping, defines in Algebra.h
         simpleSelect,         // trivial selection function
         RenameTypeMap         // type mapping
);
/*

7.3 Operator ~extract~

This operator has a stream of tuples and the name of an attribut as input and returns the value of this attribute
from the first tuple of the input stream. If the input stream is empty a run time error occurs. In this case value -1 will be returned.

7.3.1 Type mapping function of operator ~extract~

Type mapping for ~extract~ is

----	((stream (tuple ((x1 t1)...(xn tn))) xi) 	-> ti
							APPEND (i) ti)
----

*/
static ListExpr
ExtractTypeMap( ListExpr args )
{
  ListExpr first, second, attrtype;
  string  attrname;
  int j;
  
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);
			
    if((nl->ListLength(first) == 2  ) &&
       (TypeOfRelAlgSymbol(nl->First(first)) == stream)  &&
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple)  &&
	(nl->IsAtom(second)) &&
       	(nl->AtomType(second) == SymbolType))
    {
      attrname = nl->SymbolValue(second);
      j = findattr(nl->Second(nl->Second(first)), attrname, attrtype);
      if (j)
	return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
			       nl->OneElemList(nl->IntAtom(j)), attrtype);
    }
    return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~extract~

The argument vector ~args~ contains in the first slot ~args[0]~ the tuple and in ~args[2]~ the position of the attribute as a number. Returns as ~result~ the value of an attribute at the given position ~args[2]~ in a tuple object.

*/
static int
Extract(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t;
  CcTuple* tupleptr;
  int index;
  
  qp->Open(args[0].addr);
  
  qp->Request(args[0].addr,t);
  
  if (qp->Received(args[0].addr))
  {  
    tupleptr = (CcTuple*)t.addr;
    index = (int)((StandardAttribute*)args[2].addr)->GetValue();
    if ((1 <= index) && (index <= tupleptr->GetNoAttrs()))   
    {
      result = SetWord(tupleptr->Get(index - 1));
      return 0;
    }
    else
    { 
      cout << "extract: index out of range !";
      return -1;
    }
  }
  else
    return -1;
}
/*

4.1.3 Specification of operator ~extract~

*/
const string ExtractSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text---><text>Returns the value of attribute ai of the first tuple in the input stream.</text--->)";
/*

4.1.3 Definition of operator ~extract~

*/
Operator cppextract (
         "extract",             // name
         ExtractSpec,           // specification
         Extract,               // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         simpleSelect,          // trivial selection function
         ExtractTypeMap         // type mapping
);
/*

7.3 Operator ~extend~

Extends each input tuple by new attributes as specified in the parameter list.

7.3.1 Type mapping function of operator ~extend~

Type mapping for ~extract~ is

----	 ((stream (tuple (x1 ... xn))) ((fun x y1) ... (fun x ym))
 
        -> (stream (tuple (x1 ... xn y1 ... ym)))
----

*/
static ListExpr
ExtendTypeMap( ListExpr args )
{
  ListExpr first, second, third, rest, listn, errorInfo, 
           lastlistn, first2, second2, firstr, outlist;
  bool loopok;
  AlgebraManager* algMgr;

  algMgr = SecondoSystem::GetAlgebraManager();
  
  nl->WriteToFile("/dev/tty", args);
  
  if(nl->ListLength(args) == 2)
  {
    first = nl->First(args);
    second  = nl->Second(args);			
    if((nl->ListLength(first) == 2)  &&
	(TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
	(!nl->IsAtom(second)) &&
	(nl->ListLength(second) > 0))
    {
      rest = nl->Second(nl->Second(first));
      listn = nl->OneElemList(nl->First(rest));
      lastlistn = listn;
      rest = nl->Rest(rest);
      while (!(nl->IsEmpty(rest)))
      {
   	lastlistn = nl->Append(lastlistn,nl->First(rest)); 
   	rest = nl->Rest(rest);
      }
      loopok = true;
      rest = second;
      while (!(nl->IsEmpty(rest)))
      {
	firstr = nl->First(rest);
	rest = nl->Rest(rest);
	first2 = nl->First(firstr);
	second2 = nl->Second(firstr);
	if ((nl->IsAtom(first2)) &&
	    (nl->ListLength(second2) == 3) &&
	    (nl->AtomType(first2) == SymbolType) &&
	    (TypeOfRelAlgSymbol(nl->First(second2)) == ccmap) &&
	    (algMgr->CheckKind("DATA", nl->Third(second2), errorInfo)) &&
            (nl->Equal(nl->Second(first),nl->Second(second2))))
	{
	  lastlistn = nl->Append(lastlistn,
	  	(nl->TwoElemList(first2,nl->Third(second2))));
	}
	else{
	  loopok = false;
	}
      }
      //if ((loopok) && (comparenames(listn))){	
      if ( loopok ){
        outlist = nl->TwoElemList(nl->SymbolAtom("stream"),
			nl->TwoElemList(nl->SymbolAtom("tuple"),listn));
	nl->WriteToFile("/dev/tty", outlist);	
        return outlist;
      }
    }
  }
   return nl->SymbolAtom("typeerror");
}
/*

4.1.2 Value mapping function of operator ~extend~

*/
static int
Extend(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, value;
  CcTuple* tup;
  Supplier supplier, supplier2, supplier3;
  int noofoldattrs, nooffun, noofsons;
  ArgVectorPointer funargs;
    
  switch (message)
  {
    case OPEN :

      qp->Open(args[0].addr);
      return 0;

    case REQUEST :
    
      qp->Request(args[0].addr,t);
      if (qp->Received(args[0].addr))
      {
        tup = (CcTuple*)t.addr;
	noofoldattrs = tup->GetNoAttrs();
	supplier = args[1].addr;
	nooffun = qp->GetNoSons(supplier);
	for (int i=0; i < nooffun;i++)
	{
	  supplier2 = qp->GetSupplier(supplier, i);
	  noofsons = qp->GetNoSons(supplier2);
	  supplier3 = qp->GetSupplier(supplier2, 1);
          funargs = qp->Argument(supplier3);
          (*funargs)[0] = SetWord(tup);
          qp->Request(supplier3,value);
	  tup->SetNoAttrs(noofoldattrs+i+1);
	  tup->Put(noofoldattrs+i,((StandardAttribute*)value.addr)->Clone());
	}
	//cout << "Extend Anz funs : " << nooffun << endl;
        result = SetWord(tup);
	//cout << *tup << endl;
	return YIELD;
      }
      else return CANCEL;
      
    case CLOSE :
    
      qp->Close(args[0].addr);
      return 0;
  }
}
/*

4.1.3 Specification of operator ~extend~

*/
const string ExtendSpec =
  "(<text>((stream (tuple([a1:d1, ... ,an:dn]))) x ai) -> di</text---><text>Returns the value of attribute ai of the first tuple in the input stream.</text--->)";
/*

4.1.3 Definition of operator ~extract~

*/
Operator cppextend (
         "extend",              // name
         ExtendSpec,            // specification
         Extend,                // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         simpleSelect,          // trivial selection function
         ExtendTypeMap          // type mapping
);

/*

6 Class ~RelationAlgebra~

A new subclass ~RelationAlgebra~ of class ~Algebra~ is declared. The only 
specialization with respect to class ~Algebra~ takes place within the 
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~RelationAlgebra~ is defined.

*/

class RelationAlgebra : public Algebra
{
 public:
  RelationAlgebra() : Algebra()
  {
    AddTypeConstructor( &cpptuple );
    AddTypeConstructor( &cpprel );
    
    AddOperator(&feed);
    AddOperator(&consume);
    AddOperator(&TUPLE);
    AddOperator(&TUPLE2);
    AddOperator(&attr);
    AddOperator(&tfilter);
    AddOperator(&project);
    AddOperator(&product);
    AddOperator(&cancel);
    AddOperator(&tcount);
    AddOperator(&cpprename);
    AddOperator(&cppextract);
    AddOperator(&cppextend);

    cpptuple.AssociateKind( "TUPLE" );
    cpptuple.AssociateKind( "REL" );

  }
  ~RelationAlgebra() {};
};

RelationAlgebra relationalgebra;
/*

7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeRelationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&relationalgebra);
}




