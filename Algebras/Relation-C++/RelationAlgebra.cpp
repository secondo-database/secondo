/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra Main Memory

[1] Using Storage Manager Berkeley DB 

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann

[TOC]

1 Includes, Constants, Globals, Enumerations

*/
using namespace std;

#include "Algebra.h"
#include "AlgebraManager.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "DynamicLibrary.h"
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

static NestedList* nl;
static QueryProcessor* qp;

const int MaxSizeOfAttr = 10;

enum RelationType { rel, tuple, stream, ccmap, ccbool, error };
/*

2 Basic Functions

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
class CcTuple 
{
  private:

  int NoOfAttr;
  void* AttrList [MaxSizeOfAttr];

  public:

  CcTuple () {NoOfAttr = 0;};
  ~CcTuple () {};
  void* Get (int index) {return AttrList[index];};
  void Put (int index, void* attr) {AttrList[index] = attr;};
  void SetNoAttrs (int noattr) {NoOfAttr = noattr;};
  int GetNoAttrs () {return NoOfAttr;};
};
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
  attrlist =  nl->Second(nl->First(typeInfo));
  valuelist = value;
  // cout << nl->WriteToFile("/dev/tty",attrlist) << endl;
  // cout << nl->WriteToFile("/dev/tty",valuelist) << endl;
  correct = 1;
  if (nl->IsAtom(valuelist))
  {
    correct = 0;
    errorInfo = nl->Append(errorInfo,
	nl->FourElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(1),
	nl->IntAtom(errorPos)));
  }
  else
  {
	
    tupleaddr = new CcTuple();
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
	correct = 0;
	errorInfo = nl->Append(errorInfo, 
	  nl->FourElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(2),
	    nl->IntAtom(errorPos)));
      }
      else
      {
 	firstvalue = nl->First(valuelist);
	valuelist = nl->Rest(valuelist);
        attr = (algM->InObj(algebraId, typeId))(nl->Rest(first), 
                 firstvalue, attrno, errorInfo, valueCorrect);
	if (valueCorrect)
	{
	  correct = 1;
          tupleaddr->Put(attrno - 1, attr.addr);
          noOfAttrs++;
	}
	else
	{
	  correct = 0;
	  errorInfo = nl->Append(errorInfo,
	    nl->FiveElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(3),
		nl->IntAtom(errorPos), nl->IntAtom(attrno)));
   	}
      }
    }
      if (!nl->IsEmpty(valuelist))
      {
	correct = 0;
	errorInfo = nl->Append(errorInfo,
	  nl->FourElemList(nl->IntAtom(71), nl->SymbolAtom("tuple"), nl->IntAtom(4),
	    nl->IntAtom(errorPos)));
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
  const char* typname; 
  
  tupleptr = (CcTuple*)w.addr;
  attrno = tupleptr->GetNoAttrs();
  for (int i = 1; i <= attrno; i++)
  {
    typname = typeid(*(tupleptr->Get(i))).name();
    cout << typname << endl;
    
    // delete ((typeid(*(tupleptr->Get(i))).name())tupleptr->Get(i));
  }
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

  // cout << nl->WriteToFile("/dev/tty",type) << endl;
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
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("TUPLEXX"), type));
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
  return (SetWord( Address( 0 ) ));
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
  errorInfo = 0;
  return (SetWord( Address( 0 ) ));
} 
/*

1.3.5 Defnition of type constructor ~tuple~

Eventually a type constructor is created by defining an instance of class ~TypeConstructor~. Constructor's arguments are the type constructor's name and the eleven functions previously defined.

*/
TypeConstructor cpptuple( "tuple",       TupleProp,
                        OutTuple,    InTuple,   CreateTuple,
                        DeleteTuple, CastTuple,   CheckTuple,
			0,           0,
			TupleInModel, TupleOutModel,
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

*/
typedef CTable<CcTuple>* Relation;
/*

1.4.2 ~Out~-function of type constructor ~rel~

*/
ListExpr OutRel(ListExpr typeInfo, Word  value) 
{
  CTable<CcTuple>::Iterator rs;
  CcTuple t;
  ListExpr l, lastElem, tlist, TupleTypeInfo;

  // cout << nl->WriteToFile("/dev/tty",typeInfo) << endl;
  
  if (nl->IsEqual(nl->First(nl->First(typeInfo)), "rel"))
  {
    
    typeInfo = nl->First(SecondoSystem::GetCatalog(ExecutableLevel)->NumericType(typeInfo));
  }
  
  // cout << nl->WriteToFile("/dev/tty",typeInfo) << endl;
  
  Relation r = (Relation)(value.addr);
  rs = r->Begin();
  l = nl->TheEmptyList();
 
  while (rs != r->End())
  {
    t = (CcTuple)*rs;
    TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
	  nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    tlist = OutTuple(TupleTypeInfo, SetWord(&t));
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(tlist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, tlist);
    rs++;
  }
  cout << nl->WriteToFile("/dev/tty",l);
  return l;
  
}
/*

1.3.3 ~Create~-function of type constructor ~rel~

The function is used to allocate memory sufficient for keeping one instance of ~rel~. The ~Size~-parameter is not evaluated.

*/
static Word CreateRel(int Size)
{
  CTable<CcTuple>* rel;
  rel = new CTable<CcTuple>(100);
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
  Relation rel;
  CcTuple* tupleaddr;
  int tupleno;
  bool tupleCorrect;
  
  correct = true;
  cout << nl->WriteToFile("/dev/tty",typeInfo) << endl;
  cout << nl->WriteToFile("/dev/tty",value) << endl;
  if (nl->IsEqual(nl->First(nl->First(typeInfo)), "rel"))
  {
    typeInfo = nl->First(SecondoSystem::GetCatalog(ExecutableLevel)->NumericType(typeInfo));
  }
  // cout << nl->WriteToFile("/dev/tty",typeInfo) << endl;
    
  rel = (Relation)((CreateRel(50)).addr);
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
  {
    while (!nl->IsEmpty(tuplelist))
    {
      first = nl->First(tuplelist);
      tuplelist = nl->Rest(tuplelist);
      tupleno++;
      tupleaddr = (CcTuple*)(InTuple(TupleTypeInfo, first, tupleno, 
        errorInfo, tupleCorrect).addr);
      //cout << (*(CcInt*)(tupleaddr->Get(0))).GetIntval() << endl;
 
      if (tupleCorrect)
      {
        rel->Add(*((CcTuple*)tupleaddr)); 
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
    
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[1])).Get(0))).GetIntval() << endl;
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[2])).Get(0))).GetIntval() << endl;
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[3])).Get(0))).GetIntval() << endl;
    //std::cout << (*(CcInt*)(((CcTuple)((*rel)[4])).Get(0))).GetIntval() << endl;

    return (SetWord((void*)rel));
  }
  
}
/*

1.3.4 ~Destroy~-function of type constructor ~rel~


The corresponding function of type constructor ~rel~ is called ~DeleteRel~.

*/
void DeleteRel(Word& w)
{
  ;
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
  errorInfo = 0;
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




