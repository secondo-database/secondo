/*
//paragraph [1] Title:	[{\Large \bf ]	[}]

[1] Secondo Standardalgebra

Nov 1998. Friedhelm Becker.

August 16, 2000. RHG Changed includes to show dependencies more clearly.

March 2002. Ulrich Telle Port to C++

November 9, 2002. RHG Added operators ~randint~ and ~log~. Some other slight revisions.

February 2004. F. Hoffmann added operators ~relcount~ and ~relcount2~.

April 28, 2004. M. Spiekermann added operators ~nextint~ and ~randmax~. The calculation of
random numbers in an specified range was revised according to the recommendations documented 
in the rand() manpage.

July 08, 2004. M. Spiekermann changed the IN-function of data type ~real~. Integer atoms are now also
accepted. 

\begin{center}
\footnotesize
\tableofcontents
\end{center}


1 Overview

In  this algebra the standard types and functions are defined. Standard types are
~int~, ~real~, ~bool~ and ~string~. These types  are represented
In Classes of C++ whith a boolean flag which shows whether this value is defined
or not (e.g it could be undefined because it is the result of a divivion by zero).


Following operators are defined:

  * + (add)

----    int x int --> int
        int x real --> real
        real x int --> real
        real x real --> real
----

  * - (subtract)

----    int x int --> int
        int x real --> real
        real x int --> real
        real x real --> real
----

  * [*] (multiply)

----    int x int --> int
        int x real --> real
        real x int --> real
        real x real --> real
----

  * / (divide)

----    int x int --> real
        int x real --> real
        real x int --> real
        real x real --> real
----

  * mod (remainder)

----    int x int --> int
----

  * div (integer division)

----	int x int --> int
----

  * randint
  * randmax

----	int -> int
            -> int
----

Computes a random integer within the range [0, arg-1]. The argument must be
greater than 0. Otherwise it is set to 2. 

  * seqinit
  * seqnext

----	int -> bool 
            -> int
----
The seqinit operator can be used to create
sequences of numbers starting by arg. The n-th call of seqnext will return arg+n-1 

  * log

----	int -> int
----

Computes the base 2 logarithm of the first argument (i.e., the integer part of
it).

  * \verb+<+ , \verb+>+ , =, \verb+<=+ , \verb+>=+ , \#

----    int  x int  --> bool
        int  x real --> bool
        real x int  --> bool
        real x real --> bool
        bool x bool --> bool
        string x string --> bool
----


  * starts, contains

----    string x string --> bool
----

  * not

----    bool  --> bool
----

  * or, and

----    bool x bool --> bool
----


2 Includes

In addition to the normal CC includes we have to include header files
for QueryProcessor and Tuplemanager and the file which contains the
definitions of our four classes: ~CcInt~, ~CcReal~, ~CcBool~, ~CcString~.

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoSystem.h" //operator queries
#include <iostream>
#include <string>
#include <math.h>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>

extern NestedList* nl;
extern QueryProcessor *qp;

/*
4.1 Type investigation auxiliaries

Within this algebra module, we have to handle with values of four different
types: ~ccint~ and ~ccreal~, ~ccbool~ and ~CcString~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of the four types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~CcType~, containing the four types, and a function,
~TypeOfSymbol~, taking a nested list as  argument and returning the
corresponding ~CcType~ type name.

*/

enum CcType { ccint, ccreal, ccerror, ccbool, ccstring, ccconst, ccset };


CcType
TypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "int"    ) return (ccint);
    if ( s == "real"   ) return (ccreal);
    if ( s == "bool"   ) return (ccbool);
    if ( s == "string" ) return (ccstring);
    if ( s == "const"  ) return (ccconst);
    if ( s == "set"    ) return (ccset);
  }
  return (ccerror);
}

/*
3 Type constructors

A type constructor is created by defining an instance of class
~TypeConstructor~. Before this instance definition may take place we have
to define some functions which are passed as constructor arguments
during ~TypeConstructor~ instantiation.

3.1 Type constructor *CcInt*

Each instance of below defined class CcInt will be the main memory
representation of a
value of type ~ccint~. It consists of a boolean flag, ~defined~,
and an integer value, ~intval~. ~defined~ may be used to indicate whether
an instance of class CcInt represents a valid value or not. E.g., a
division by zero might result in a CcInt object with ~defined~ set to false.
Within this implementation, we don't use the flag but maintain it in order
to demonstrate how to handle complex  objects.

*/
long CcInt::intsCreated = 0;
long CcInt::intsDeleted = 0;

CcInt::CcInt() { intsCreated++; };
CcInt::CcInt( bool d, int v ) { defined = d; intval = v; intsCreated++; };
CcInt::~CcInt() { intsDeleted++; };
void   CcInt::Set( int v ) { defined = true, intval = v; };
void   CcInt::Set( bool d, int v ) { defined = d, intval = v; };
int    CcInt::GetIntval() { return (intval); };
bool   CcInt::IsDefined() const { return (defined); };
void   CcInt::SetDefined(bool defined) { this->defined = defined; };
CcInt* CcInt::Clone() { return (new CcInt( this->defined, this->intval )); };
size_t CcInt::HashValue() { return (defined ? intval : 0); };

void CcInt::CopyFrom(StandardAttribute* right)
{
  CcInt* r = (CcInt*)right;
  defined = r->defined;
  intval = r->intval;
}

int    CcInt::Compare( Attribute* arg )
{
  if(!IsDefined() && !arg->IsDefined())
  {
    return 0;
  }
  if(!IsDefined())
  {
    return -1;
  }
  if(!arg->IsDefined())
  {
    return 1;
  }

  CcInt* p =  (CcInt*)(arg);
  if ( !p )                 return (-2);
  if ( intval < p->intval ) return (-1);
  if ( intval > p->intval)  return (1);
  return (0);
};
/*

The next function defines the type property of type constructor ~CcInt~.

*/
ListExpr
CcIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("int"),
			     nl->StringAtom("(<intvalue>)"),
			     nl->StringAtom("12 or -32 or 0"))));
}

bool CcInt::Adjacent( Attribute* arg )
{
  int a = GetIntval(),
      b = ((CcInt *)arg)->GetIntval();

  return( a == b || a == b + 1 || b == a + 1 );
}

/*

Now we define a function, ~OutInt~, that takes as inputs a type description
and a pointer to a value of this type. The representation of this value in
nested list format is returned.

For the simple types int, real, string, bool we don't use the type description at all. We will
need it in the case of more complex type constructors,
e.g. to be able to compute the nested list
representation of a tuple value we must know the types of the
respective attribute values.

*/

ListExpr
OutCcInt( ListExpr typeinfo, Word value )
{
  if( ((CcInt*)value.addr)->IsDefined() )
  {
    return (nl->IntAtom( ((CcInt*)value.addr)->GetIntval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*

The function ~InInt~ provides a functionality complementary to ~OutInt~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters.

*/

Word
InCcInt( ListExpr typeInfo, ListExpr value,
         int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == IntType )
  {
    correct = true;
    return (SetWord( new CcInt( true, nl->IntValue( value ) ) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcInt( false, 0) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*

*/

Word
CreateCcInt( const ListExpr typeInfo )
{
  return (SetWord( new CcInt( false, 0 ) ));
}

void
DeleteCcInt( Word& w )
{
  delete (CcInt*) w.addr;
  w.addr = 0;
}

void
CloseCcInt( Word& w )
{
  delete (CcInt*) w.addr;
  w.addr = 0;
}

Word
CloneCcInt( const Word& w )
{
  return SetWord( ((CcInt*)w.addr)->Clone() );
}

int
SizeOfCcInt()
{
  return sizeof(CcInt);
}

/*
3.2.5 {\em Cast}-function of type constructor {\tt ccint}

*/

void*
CastInt( void* addr )
{
  CcInt::intsCreated--;
  return (new (addr) CcInt);
}

/*
3.2.6 {\em Type check} function of type constructor {\tt ccint}

*/

bool
CheckInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "int" ));
}

/*
A type constructor is created by constructing an instance of
class ~TypeConstructor~. Constructor arguments are the type constructor's name
and the four functions previously defined.

*/

/****************************************************************************
1.9 Models

1.9.1 Model for Set of Integers

*/

enum ConstOrSet { Const, Set };

struct IntSetModel
{
  ConstOrSet constOrSet;
  union
  {
    int value;
    struct
    {
      int min;
      int max;
      int card;
    } t;
  };
};

/*
The list representation for this is:

----	()			NIL, the undefined model

	(const 3)		a constant 3

	(set 0 1000 40)		min = 0, max = 1000 card = 40
----

where in the second case the numbers 0, 1000, and 40 give the minimal,
and maximal value and the cardinality (i.e. the number of distinct values)
of the set of integers to be described. It is required that ~min~ [<] ~max~;
otherwise we have a single value which is represented as a constant.

We need conversion functions to be able to store this in list format:

*/

ListExpr
OutIntSetModel( ListExpr typeExpr, Word inModel )
{
  IntSetModel* model = (IntSetModel*) inModel.addr;
  if ( model == 0 )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    if ( model->constOrSet == Const )
    {
      return (nl->TwoElemList(
                nl->SymbolAtom( "const" ),
                nl->IntAtom( model->value ) ));
    }
    else if ( model->constOrSet == Set )
    {
      return (nl->FourElemList(
                nl->SymbolAtom( "set" ),
                nl->IntAtom( model->t.min ),
                nl->IntAtom( model->t.max ),
    	        nl->IntAtom( model->t.card ) ));
    }
    else
    {
      return (nl->TheEmptyList());
    }
  }
  return (nl->TheEmptyList());
}

Word
InIntSetModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  if ( nl->IsEmpty( list ) )
  {
    return (SetWord( Address( 0 ) ));
  }
  else
  {
    IntSetModel* model = new IntSetModel;
    if ( TypeOfSymbol( nl->First( list ) ) == ccconst )
    {
      model->constOrSet = Const;
      model->value = nl->IntValue( nl->Second( list ) );
    }
    else
    {
      model->constOrSet = Set;
      model->t.min = nl->IntValue( nl->Second( list ) );
      model->t.max = nl->IntValue( nl->Third( list ) );
      model->t.card = nl->IntValue( nl->Fourth( list ) );
    }
    return (SetWord( model ));
  }
}

Word
IntToIntSetModel( ListExpr typeExpr, Word value )
{
  IntSetModel* model = new IntSetModel;
  model->constOrSet = Const;
  model->value = ((CcInt*)value.addr)->GetIntval();
  return (SetWord( model ));
}

Word
IntListToIntSetModel( const ListExpr typeExpr, const ListExpr valueList,
                      const int errorPos, ListExpr& errorInfo, bool& correct )
{
  IntSetModel* model = new IntSetModel;
  model->constOrSet = Const;
  model->value = nl->IntValue( valueList );
  correct = true;
  return (SetWord( model ));
}

/*

*/

TypeConstructor ccInt( "int",            CcIntProperty,
                       OutCcInt,         InCcInt,
                       0,                0,
                       CreateCcInt,      DeleteCcInt,
                       0,        0,      CloseCcInt, CloneCcInt,
                       CastInt,          SizeOfCcInt, CheckInt,
                       0,
                       InIntSetModel,    OutIntSetModel,
                       IntToIntSetModel, IntListToIntSetModel );

/*
3.2 Type constructor *ccreal*

The following type constructor, ~ccreal~, is defined in the same way as
~ccint~.

*/
long CcReal::realsCreated = 0;
long CcReal::realsDeleted = 0;

CcReal::CcReal(){ realsCreated++; };
CcReal::CcReal( bool d, float v ) { defined = d; realval = v; realsCreated++; };
CcReal::~CcReal() { realsDeleted++; };
bool    CcReal::IsDefined() const { return (defined); };
void    CcReal::SetDefined(bool defined) { this->defined = defined; };
float   CcReal::GetRealval() { return (realval);};
void    CcReal::Set( float v ) { defined = true, realval = v; };
void    CcReal::Set( bool d, float v ) { defined = d, realval = v; };
CcReal* CcReal::Clone() { return (new CcReal(this->defined, this->realval)); };

size_t CcReal::HashValue()
{
  if(!defined)
  {
    return 0;
  }

  unsigned long h = 0;
  char* s = (char*)&realval;
  for(unsigned int i = 1; i <= sizeof(float) / sizeof(char); i++)
  {
    h = 5 * h + *s;
    s++;
  }
  return size_t(h);
}

void CcReal::CopyFrom(StandardAttribute* right)
{
  CcReal* r = (CcReal*)right;
  defined = r->defined;
  realval = r->realval;
}

int     CcReal::Compare( Attribute * arg )
{
  if(!IsDefined() && !arg->IsDefined())
  {
    return 0;
  }
  if(!IsDefined())
  {
    return -1;
  }
  if(!arg->IsDefined())
  {
    return 1;
  }

  CcReal* p = (CcReal* )(arg);
  if ( !p )                    return (-2);
  if ( realval < p-> realval ) return (-1);
  if ( realval > p-> realval ) return (1);
  return (0);
};
/*

The next function defines the type property of type constructor ~CcReal~.

*/
ListExpr
CcRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("real"),
			     nl->StringAtom("(<realvalue>)"),
			     nl->StringAtom("12.0 or -1.342 or 14e-3 "
			     "or .23"))));
}

bool CcReal::Adjacent( Attribute *arg )
{
  return( GetRealval() == ((CcReal *)arg)->GetRealval() );
}

ListExpr
OutCcReal( ListExpr typeinfo, Word value )
{
  if( ((CcReal*)value.addr)->IsDefined() )
  {
    return (nl->RealAtom( ((CcReal*)value.addr)->GetRealval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

Word
InCcReal( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  bool isAtom = nl->IsAtom( value );
  NodeType nodeType = nl->AtomType( value );

  if ( isAtom && nodeType == RealType )
  {
    correct = true;
    return (SetWord( new CcReal( true, nl->RealValue( value ) )));
  }
  else if ( isAtom && nodeType == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcReal( false, 0.0) ));
  } 
  else if ( isAtom && nodeType == IntType  )
  {
    return ( SetWord( new CcReal( true, 1.0 * nl->IntValue(value) )));
    correct = true;    
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

Word
CreateCcReal( const ListExpr typeInfo )
{
  return (SetWord( new CcReal( false, 0 ) ));
}

void
DeleteCcReal( Word& w )
{
  delete (CcReal*)w.addr;
  w.addr = 0;
}

void
CloseCcReal( Word& w )
{
  delete (CcReal*)w.addr;
  w.addr = 0;
}

Word
CloneCcReal( const Word& w )
{
  return SetWord( ((CcReal*)w.addr)->Clone() );
}

int
SizeOfCcReal()
{
  return sizeof(CcReal);
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

void*
CastReal( void* addr )
{
  CcReal::realsCreated--;
  return new (addr) CcReal;
}


/*
3.3.7 {\em Type check} function of type constructor {\tt ccreal}

*/

bool
CheckReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "real" ));
}


TypeConstructor ccReal( "real",       CcRealProperty,
                        OutCcReal,    InCcReal,
                        0,            0,
                        CreateCcReal, DeleteCcReal,
                        0,            0,
                        CloseCcReal, CloneCcReal,
                        CastReal,   SizeOfCcReal, CheckReal );

/*
3.3 Type constructor *ccbool*

Each instance of below defined class CcBool will be the main memory
representation of a
value of type ~ccbool~. It consists of a boolean flag, ~defined~,
and an boolean value, ~boolval~. ~defined~ may be used to indicate whether
an instance of class CcBool represents a valid value or not. E.g., a
division by zero might result in a CcBool object with ~defined~ set to false.
Within this implementation, we don't use the flag but maintain it in order
to demonstrate how to handle complex  objects.

*/
long CcBool::boolsCreated = 0;
long CcBool::boolsDeleted = 0;

CcBool::CcBool(){ boolsCreated++; };
CcBool::CcBool( bool d, int v ){ defined  = d; boolval = v; boolsCreated++; };
CcBool::~CcBool() { boolsDeleted++; };
void    CcBool::Set( bool d, bool v ){ defined = d, boolval = v; };
bool    CcBool::IsDefined() const { return defined; };
void    CcBool::SetDefined(bool defined) { this->defined = defined; };
bool    CcBool::GetBoolval() { return boolval; };
CcBool* CcBool::Clone() { return new CcBool(this->defined, this->boolval); };
size_t CcBool::HashValue() { return (defined ? boolval : false); };

void CcBool::CopyFrom(StandardAttribute* right)
{
  CcBool* r = (CcBool*)right;
  defined = r->defined;
  boolval = r->boolval;
}

int     CcBool::Compare( Attribute* arg )
{
  if(!IsDefined() && !arg->IsDefined())
  {
    return 0;
  }
  if(!IsDefined())
  {
    return -1;
  }
  if(!arg->IsDefined())
  {
    return 1;
  }

  CcBool* p = (CcBool*)(arg);
  if ( !p )                    return (-2);
  if ( boolval < p-> boolval ) return (-1);
  if ( boolval > p-> boolval ) return (1);
  return (0);
};
/*

The next function defines the type property of type constructor ~CcBool~.

*/
ListExpr
CcBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("bool"),
			     nl->StringAtom("(<boolvalue>)"),
			     nl->StringAtom("TRUE or FALSE"))));
}

bool CcBool::Adjacent( Attribute* arg )
{
  return 1;
}

/*
Now we define a function, ~OutBool~, that takes as inputs a type description
and a pointer to a value of this type. The representation of this value in
nested list format is returned.

*/

ListExpr
OutCcBool( ListExpr typeinfo, Word value )
{
  if( ((CcBool*)value.addr)->IsDefined() )
  {
    return (nl->BoolAtom( ((CcBool*)value.addr)->GetBoolval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
The function ~InBool~ provides a functionality complementary to ~OutBool~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters. Again, we don't need the type information in this example due to
simplicity of types used.

The next three parameters are used to return error information.

*/

Word
InCcBool( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == BoolType )
  {
    correct = true;
    return (SetWord( new CcBool( true, nl->BoolValue( value ) ) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcBool( false, false) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

Word
CreateCcBool( const ListExpr typeInfo )
{
  return (SetWord( new CcBool( false, 0 ) ));
}

void
DeleteCcBool( Word& w )
{
  delete (CcBool*)w.addr;
  w.addr = 0;
}

void
CloseCcBool( Word& w )
{
  delete (CcBool*)w.addr;
  w.addr = 0;
}

Word
CloneCcBool( const Word& w )
{
  return SetWord( ((CcBool*)w.addr)->Clone() );
}

int
SizeOfCcBool()
{
  return sizeof(CcBool);
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

void*
CastBool( void* addr )
{
  CcBool::boolsCreated--;
  return (new (addr) CcBool);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt ccreal}

*/

bool
CheckBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "bool" ));
}

/*
1.9.2 Model for Set of Booleans

*/

typedef float BoolSetModel;

/*
The list representation for a ~BoolSetModel~ is just the one for a real number.
Here 0.0 represents the constant FALSE, 1.0 the constant TRUE. Numbers between
0 and 1 represent the expected fraction of TRUE values in the set. The undefined
model is represented by a negative number such as -1.0.

*/

ListExpr
OutBoolSetModel( ListExpr typeExpr, Word model )
{
  BoolSetModel modelValue = *((BoolSetModel*)model.addr);
  if ( modelValue < 0.0 )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    return (nl->RealAtom( modelValue ));
  }
}

Word
InBoolSetModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  BoolSetModel* result = new BoolSetModel;
  if ( nl->IsEmpty( list ) )
  {
    *result = -1.0;
  }
  else
  {
    *result = nl->RealValue( list );
  }
  return (SetWord( result ));
}

Word
BoolToBoolSetModel( ListExpr typeExpr, Word value )
{
  BoolSetModel* result = new BoolSetModel;
  *result = (((CcBool*)value.addr)->GetBoolval()) ? 1.0 : 0.0;
  return (SetWord( result ));
}

Word
BoolListToBoolSetModel( const ListExpr typeExpr, const ListExpr valueList,
                        const int errorPos, ListExpr& errorInfo, bool& correct )
{
  BoolSetModel* result = new BoolSetModel;
  *result = (nl->BoolValue( valueList )) ? 1.0 : 0.0;
  correct = true;
  return (SetWord( result ));
}

/*

*/

TypeConstructor ccBool( "bool",             CcBoolProperty,
                        OutCcBool,          InCcBool,
                        0,                  0,
                        CreateCcBool,       DeleteCcBool,
                        0,                  0,
                        CloseCcBool,        CloneCcBool,
                        CastBool,           SizeOfCcBool,  CheckBool,
                        0,
                        InBoolSetModel,     OutBoolSetModel,
                        BoolToBoolSetModel, BoolListToBoolSetModel );

/*
3.5 Type constructor *CcString*

*/
long CcString::stringsCreated = 0;
long CcString::stringsDeleted = 0;

CcString::CcString() { stringsCreated++; };
CcString::CcString( bool d, const STRING* v ) { defined = d; strcpy( stringval, *v); stringsCreated++; };
CcString::~CcString() { stringsDeleted++; };
bool      CcString::IsDefined() const { return (defined); };
void      CcString::SetDefined(bool defined) { this->defined = defined; };
STRING*   CcString::GetStringval() { return (&stringval); };
CcString* CcString::Clone() { return (new CcString( this->defined, &this->stringval )); };
void CcString::Set( bool d, const STRING* v ) { defined = d; strcpy( stringval, *v); };

size_t
CcString::HashValue()
{
  if(!defined)
  {
    return 0;
  }

  unsigned long h = 0;
  char* s = stringval;
  while(*s != 0)
  {
    h = 5 * h + *s;
    s++;
  }
  return size_t(h);
}

void CcString::CopyFrom(StandardAttribute* right)
{
  CcString* r = (CcString*)right;
  defined = r->defined;
  strcpy(stringval, r->stringval);
}

int       CcString::Compare( Attribute* arg )
{
  if(!IsDefined() && !arg->IsDefined())
  {
    return 0;
  }
  if(!IsDefined())
  {
    return -1;
  }
  if(!arg->IsDefined())
  {
    return 1;
  }

  CcString* p = (CcString*)(arg);
  if ( !p ) return (-2);
  if ( strcmp(stringval , p->stringval) < 0) return (-1);
  if ( !strcmp(stringval , p->stringval)) return (0);
  return (1);
  // return (stringval.compare( p->stringval ));
};
/*

The next function defines the type property of type constructor ~CcString~.

*/
ListExpr
CcStringProperty()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist, "\"A piece of text up to 48 "
		 "characters\"");
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
	                     nl->StringAtom("Example Type List"),
			     nl->StringAtom("List Rep"),
			     nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
	                     nl->StringAtom("string"),
			     nl->StringAtom("(<stringvalue>)"),
			     examplelist)));
}

bool CcString::Adjacent( Attribute* arg )
{
  STRING *a = GetStringval(),
         *b = ((CcString *)arg)->GetStringval();

  if( strcmp( *a, *b ) == 0 )
    return 1;

  if( strlen( *a ) == strlen( *b ) )
  {
    if( strncmp( *a, *b, strlen( *a ) - 1 ) == 0 )
    {
      char cha = (*a)[strlen(*a)-1],
           chb = (*b)[strlen(*b)-1];
      return( cha == chb + 1 || chb == cha + 1 );
    }
  }
  else if( strlen( *a ) == strlen( *b ) + 1 )
  {
    return( strncmp( *a, *b, strlen( *b ) ) == 0 &&
            ( (*a)[strlen(*a)-1] == 'a' || (*a)[strlen(*a)-1] == 'A' ) );
  }
  else if( strlen( *a ) + 1 == strlen( *b ) )
  {
    return( strncmp( *a, *b, strlen( *a ) ) == 0 &&
            ( (*b)[strlen(*b)-1] == 'a' || (*b)[strlen(*b)-1] == 'A' ) );
  }

  return 0;
}

/*

*/

ListExpr
OutCcString( ListExpr typeinfo, Word value )
{
  if( ((CcString*)value.addr)->IsDefined() )
  {
    return (nl->StringAtom( *((CcString*)value.addr)->GetStringval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*

*/

Word
InCcString( ListExpr typeInfo, ListExpr value,
            int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == StringType )
  {
    correct = true;
    string s = nl->StringValue( value );
    return (SetWord( new CcString( true, (STRING*)s.c_str() ) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcString( false, (STRING*)"" ) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*

*/

Word
CreateCcString( const ListExpr typeInfo )
{
  char p[49] = "";
  return (SetWord( new CcString( false, (STRING*)&p ) ));
}

void
DeleteCcString( Word& w )
{
  delete (CcString*)w.addr;
  w.addr = 0;
}

void
CloseCcString( Word& w )
{
  delete (CcString*)w.addr;
  w.addr = 0;
}

Word
CloneCcString( const Word& w )
{
  return SetWord( ((CcString*)w.addr)->Clone() );
}

int
SizeOfCcString()
{
  return sizeof(CcString);
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

void*
CastString( void* addr )
{
  CcString::stringsCreated--;
  return (new (addr) CcString);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt ccreal}

*/
bool
CheckString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "string" ));
}

TypeConstructor ccString( "string",       CcStringProperty,
                          OutCcString,    InCcString,
                          0,              0,
                          CreateCcString, DeleteCcString,
                          0,              0,
                          CloseCcString,  CloneCcString,
                          CastString,     SizeOfCcString, CheckString );

/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.2 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.2.1 Type mapping function CcMathTypeMap

It is for the operators +, - and [*].

*/

ListExpr
CcMathTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    {
      return (nl->SymbolAtom( "int" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
    {
      return (nl->SymbolAtom( "real" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.2 Type mapping function CcMathTypeMapdiv

It is for the operators /. the only difference between CCMathTypeMap and
CcMathTypeMapdiv is that the latter give as resulttype ccreal if the input type
is ccint ("normal division of int with result real", the other division
of int is called div in this program).

*/

ListExpr
CcMathTypeMapdiv( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
      return (nl->SymbolAtom( "real" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.3 Type mapping function CcMathTypeMap1

It is for the operators mod and div which have ~int~ as input and ~int~ as result.

*/

ListExpr
CcMathTypeMap1( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.4 Type mapping function CcMathTypeMap2
>>>>>>> 1.18

It is for the operators ~intersection~ and ~minus~.

*/
ListExpr
CcMathTypeMap2( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "int" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
      return (nl->SymbolAtom( "string" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.4 Type mapping functions IntInt, IntBool and EmptyInt

Used for operators ~randint~, ~randmax~, ~initseq~, ~nextseq~ and ~log~.

*/

ListExpr
IntInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "int" ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
IntBool( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "int" ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
EmptyInt( ListExpr args )
{
  if ( !nl->IsEmpty( args ) ) {
    return (nl->SymbolAtom( "typeerror" ));
  } else {
    return (nl->SymbolAtom( "int" ));
  }
}



/*
4.2.5 Type mapping function CcMathTypeMapBool

It is for the Compare operators which have ~bool~ as resulttype.

*/

ListExpr
CcMathTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring)
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.6 Type mapping function CcMathTypeMapBool1

It is for the  operator ~not~ which have ~bool~ as input and resulttype.

*/

ListExpr
CcMathTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.7 Type mapping function CcMathTypeMapBool2

It is for the  operators and and or  which have bool as input and resulttype.

*/

ListExpr
CcMathTypeMapBool2( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.8 Type mapping function CcMathTypeMapBool3

It is for the  operators ~starts~ and ~contains~  which have ~string~ as input and ~bool~ resulttype.

*/

ListExpr
CcMathTypeMapBool3( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.9 Type mapping function CcMathTypeMapBool4

It is for the  operators ~isempty~ which have ~bool~, ~int~, ~real~, and ~string~ as input and ~bool~ resulttype.

*/

ListExpr
CcMathTypeMapBool4( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccint )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.10 Type mapping function for the ~upper~ operator:

string ---> string.

*/

ListExpr
CcStringMapCcString( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccstring )
      return (nl->SymbolAtom( "string" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.11 Type mapping function for the ~relcount~ operator:

string ---> int.

*/

static ListExpr
CcStringMapCcInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccstring )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.12 Type mapping function for the ~keywords~ operator:

Type mapping for ~keywords~ is

----	(string) -> (stream string)
----

*/
ListExpr
keywordsType( ListExpr args ){
  ListExpr arg;
  
  if ( nl->ListLength(args) == 1 )
  {
    arg = nl->First(args);
    if ( nl->IsEqual(arg, "string") )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("string"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.3 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

*/

/*
4.3.1 Selection function  CcMathSelectCompute

It is used for the  operators +, - [*] and / .

*/

int
CcMathSelectCompute( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    return (3);
  return (-1); // This point should never be reached
}

/*
4.3.3 Selection function  CcMathSelectCompare

It is used for the  all compare operators .

*/

int
CcMathSelectCompare( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    return (3);
  if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
    return (4);
  if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
    return (5);
  return (-1); // This point should never be reached
}

/*
4.3.4 Selection function  CcMathSelectSet

It is used for the set operators ~intersection~ and ~minus~.

*/

int
CcMathSelectSet( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
    return (3);
  return (-1); // This point should never be reached
}


/*
4.3.3 Selection function  CcMathSelectIsEmpty

It is used for the ~isempty~ operator.

*/
int
CcMathSelectIsEmpty( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  if ( TypeOfSymbol( arg1 ) == ccbool )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccint )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccreal )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccstring )
    return (3);
  return (-1); // This point should never be reached
}

/*
4.4 Value mapping functions of operator ~+~

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators --- like in
this example --- there are several value mapping functions, one for each
possible combination of input parameter types. We have to provide
four functions for each of the operators ~+~, ~-~,  ~[*]~ ....., since
each of them accepts four input parameter combinations: ~ccint~ $\times$
~ccint~, ~ccint~ $\times$ ~ccreal~,  ~ccreal~ $\times$ ~ccint~, and
~ccreal~ $\times$ ~ccreal~.

*/

int
CcPlus_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->Set( true, ((CcInt*)args[0].addr)->GetIntval() +
                                   ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() +
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcReal*)args[0].addr)->IsDefined() &&
      ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() +
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() +
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.5 Value mapping functions of operator ~-~

*/

int
CcMinus_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() -
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcMinus_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() -
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcMinus_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() -
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() -
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.6 Value mapping functions of operator ~[*]~

Value mapping functions of operator ~[*]~ are defined in the same way as
value mapping functions of operator ~+~.

*/

int
CcProduct_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() *
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcProduct_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() *
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcProduct_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() *
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcProduct_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() *
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.7 Value mapping functions of operator ~/~

*/

int
CcDivision_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((float )((CcInt*)args[0].addr)->GetIntval()) /
                          ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcDivision_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->GetRealval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() /
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcDivision_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() /
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcDivision_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() /
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.8 Value mapping functions of operator ~mod~

*/

int
CcMod( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() %
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.8 Value mapping functions of operator ~div~

*/

int
CcDiv( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() &&
      ((CcInt*)args[1].addr)->IsDefined() &&
      ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcInt *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() /
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}




/*
4.8 Value mapping function of operators ~randint~, ~maxrand~, ~initseq~ and ~nextseq~

*/

int randint(int u)    	//Computes a random integer in the range 0..u-1,
			//for u >= 2
{
  if ( u < 2 ) {u=2;}
  // rand creates a value between [0,RAND_MAX]. The calculation procedure below is recommended
  // in the manpage of the rand() function. Using rand() % u will yield poor results.
  return (int) ( (float)u * rand()/(RAND_MAX+1.0) );
}


int
RandInt( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->
      Set( true, randint(((CcInt*)args[0].addr)->GetIntval()) );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
MaxRand( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)-> Set( true, RAND_MAX );
  return (0);
}


static int SequenceCounter = 0;


int
InitSeq( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->Set( true, true );
      SequenceCounter = ((CcInt*)args[0].addr)->GetIntval();
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
NextSeq( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)-> Set( true, SequenceCounter );
  SequenceCounter++;
  return (0);
}


/*
4.8 Value mapping function of operator ~log~

*/

int intlog(int n)
{
  int i = 0;
  while (n > 1) {n >>= 1; i++;}
  return i;
}


int
LogFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() &&
      ((CcInt*)args[0].addr)->GetIntval() )
  {
    int n = intlog(((CcInt*)args[0].addr)->GetIntval());
    ((CcInt *)result.addr)-> Set( true, n );
  }
  else
  {
    ((CcInt *)result.addr)-> Set( false, 0 );
  }
  return (0);
}


/*
4.9 Value mapping functions of operator  $ < $

*/

int
CcLess_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() <
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLess_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() <
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLess_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() <
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLess_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() <
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLess_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcBool*)args[0].addr)->GetBoolval() <
                 ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLess_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    int cmp = ((CcString*)args[0].addr)->Compare((CcString*)args[1].addr);
    if (cmp < 0)
      ((CcBool *)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.10 Value mapping functions of operator $ <= $

*/

int
CcLessEqual_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() <=
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLessEqual_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() <=
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLessEqual_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() <=
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLessEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() <=
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLessEqual_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcBool*)args[0].addr)->GetBoolval() <=
                 ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcLessEqual_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    int cmp = ((CcString*)args[0].addr)->Compare((CcString*)args[1].addr);
    if (cmp <= 0)
      ((CcBool *)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return 0;
}

/*
4.11 Value mapping functions of operator $ > $

*/

int
CcGreater_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() >
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreater_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() >
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreater_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() >
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreater_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() >
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreater_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcBool*)args[0].addr)->GetBoolval() >
                 ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreater_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    int cmp = ((CcString*)args[0].addr)->Compare((CcString*)args[1].addr);
    if (cmp > 0)
      ((CcBool *)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );

  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.12 Value mapping functions of operator $ >= $

*/

int
CcGreaterEqual_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() >=
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreaterEqual_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() >=
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreaterEqual_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() >=
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreaterEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() >=
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreaterEqual_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcBool*)args[0].addr)->GetBoolval() >=
                 ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcGreaterEqual_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    int cmp = ((CcString*)args[0].addr)->Compare((CcString*)args[1].addr);
    if (cmp >= 0)
      ((CcBool *)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.13 Value mapping functions of operator ~=~

*/

int
CcEqual_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() ==
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcEqual_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() ==
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcEqual_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() ==
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() ==
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcEqual_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((CcBool*)args[0].addr)->GetBoolval() ==
                 ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcEqual_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    int cmp = ((CcString*)args[0].addr)->Compare((CcString*)args[1].addr);
    if (cmp == 0)
      ((CcBool *)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.14 Value mapping functions of operator ~\#~

*/

int
CcDiff_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, !(((CcInt*)args[0].addr)->GetIntval() ==
                   ((CcInt*)args[1].addr)->GetIntval()) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcDiff_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, !(((CcInt*)args[0].addr)->GetIntval() ==
                   ((CcReal*)args[1].addr)->GetRealval()) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcDiff_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, !(((CcReal*)args[0].addr)->GetRealval() ==
                   ((CcInt*)args[1].addr)->GetIntval()) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcDiff_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, !(((CcReal*)args[0].addr)->GetRealval() ==
                   ((CcReal*)args[1].addr)->GetRealval()) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcDiff_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, !(((CcBool*)args[0].addr)->GetBoolval() ==
                   ((CcBool*)args[1].addr)->GetBoolval()) );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
CcDiff_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    int cmp = ((CcString*)args[0].addr)->Compare((CcString*)args[1].addr);
    if (cmp != 0)
      ((CcBool *)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.14 Value mapping functions of operator ~starts~

*/

int
StartsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    string str1 = (string)(char*)((CcString*)args[0].addr)->GetStringval();
    string str2 = (string)(char*)((CcString*)args[1].addr)->GetStringval();
    char* cstr1 = (char*)((CcString*)args[0].addr)->GetStringval();
    char* cstr2 = (char*)((CcString*)args[1].addr)->GetStringval();
    ((CcBool *)result.addr)->
    Set (true, strncmp(cstr1, cstr2, str2.length()) == 0 );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.14 Value mapping functions of operator ~contains~

*/

int
ContainsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    string str1 = (string)(char*)((CcString*)args[0].addr)->GetStringval();
    string str2 = (string)(char*)((CcString*)args[1].addr)->GetStringval();
    ((CcBool *)result.addr)->
    Set (true, str1.find(str2) != string::npos);

    //((CcBool *)result.addr)->
      //Set( true, ((CcString*)args[0].addr)->GetStringval()->find(
                //*((CcString*)args[1].addr)->GetStringval() ) != string::npos );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.15 Value mapping functions of operator ~not~

*/

int
NotFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->Set( true, !((CcBool*)args[0].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.16 Value mapping functions of operator ~and~

*/

int
AndFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( (((CcBool*)args[0].addr)->IsDefined() &&
        ((CcBool*)args[1].addr)->IsDefined()) )
  {
    ((CcBool*)result.addr)->Set( true, ((CcBool*)args[0].addr)->GetBoolval() &&
                                       ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.17 Value mapping functions of operator ~or~

*/

int
OrFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcBool*)args[0].addr)->IsDefined() &&
      ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, ((CcBool*)args[0].addr)->GetBoolval() ||
                                       ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.17 Value mapping functions of operator ~isempty~

*/

int
IsEmpty_b( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcBool*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}

int
IsEmpty_i( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}

int
IsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcReal*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}

int
IsEmpty_s( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcString*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}

/*
4.18 Value mapping functions of operator ~upper~

*/

int
Upper( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  char * orgStr = (char*)((CcString*)args[0].addr)->GetStringval();
  char newStr[256];
  char *lastChar;

  if( ((CcString*)args[0].addr)->IsDefined() )
  {
      strcpy(newStr, orgStr);
      if (strlen(newStr)>0)
      {
           lastChar= newStr + strlen(newStr) -1;
           *lastChar=*lastChar+1;
       }
      ((CcString *)result.addr)->Set( true, (STRING *)&newStr );
  }
  else
  {
      ((CcString *)result.addr)->Set( false, ((CcString*)args[0].addr)->GetStringval());
  }
  return (0);
}

/*
4.19 Value mapping functions of operator ~intersection~

*/

int
CcSetIntersection_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    if( ((CcInt*)args[0].addr)->GetIntval() == ((CcInt*)args[1].addr)->GetIntval() )
    {
      ((CcInt *)result.addr)->Set( true, ((CcInt*)args[0].addr)->GetIntval() );
      return (0);
    }
  }
  ((CcInt *)result.addr)->Set( false, 0 );
  return (0);
}

int
CcSetIntersection_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    if( ((CcReal*)args[0].addr)->GetRealval() == ((CcReal*)args[1].addr)->GetRealval() )
    {
      ((CcReal *)result.addr)->Set( true, ((CcReal*)args[0].addr)->GetRealval() );
      return (0);
    }
  }
  ((CcReal *)result.addr)->Set( false, 0.0 );
  return (0);
}

int
CcSetIntersection_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    if( ((CcBool*)args[0].addr)->GetBoolval() == ((CcBool*)args[1].addr)->GetBoolval() )
    {
      ((CcBool*)result.addr)->Set( true, ((CcBool*)args[0].addr)->GetBoolval() );
      return (0);
    }
  }
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

int
CcSetIntersection_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    if( strcmp( *((CcString*)args[0].addr)->GetStringval(), *((CcString*)args[1].addr)->GetStringval() ) == 0 )
    {
      ((CcString*)result.addr)->Set( true, ((CcString*)args[0].addr)->GetStringval() );
      return (0);
    }
  }
  STRING nullStr = "";
  ((CcString*)result.addr)->Set( false, &nullStr );
  return (0);
}

/*
4.19 Value mapping functions of operator ~minus~

*/

int
CcSetMinus_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    if( ((CcInt*)args[0].addr)->GetIntval() == ((CcInt*)args[1].addr)->GetIntval() )
    {
      ((CcInt *)result.addr)->Set( false, 0 );
      return (0);
    }
  }
  ((CcInt *)result.addr)->Set( true, ((CcInt*)args[0].addr)->GetIntval() );
  return (0);
}

int
CcSetMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    if( ((CcReal*)args[0].addr)->GetRealval() == ((CcReal*)args[1].addr)->GetRealval() )
    {
      ((CcReal *)result.addr)->Set( false, 0.0 );
      return (0);
    }
  }
  ((CcReal *)result.addr)->Set( true, ((CcReal*)args[0].addr)->GetRealval() );
  return (0);
}

int
CcSetMinus_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    if( ((CcBool*)args[0].addr)->GetBoolval() == ((CcBool*)args[1].addr)->GetBoolval() )
    {
      ((CcBool *)result.addr)->Set( false, false );
      return (0);
    }
  }
  ((CcBool*)result.addr)->Set( true, ((CcBool*)args[0].addr)->GetBoolval() );
  return (0);
}

int
CcSetMinus_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    if( strcmp( *((CcString*)args[0].addr)->GetStringval(), *((CcString*)args[1].addr)->GetStringval() ) == 0 )
    {
      STRING nullStr = "";
      ((CcString*)result.addr)->Set( false, &nullStr );
      return (0);
    }
  }
  ((CcString*)result.addr)->Set( true, ((CcString*)args[0].addr)->GetStringval() );
  return (0);
}

/*
4.20 Value mapping function of operator ~relcount~

*/

static int
RelcountFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ListExpr resultType, queryList, resultList, valueList;
  QueryProcessor* qpp;
  OpTree tree;
  AlgebraLevel level;
  char* relname;
  bool correct      = false;
  bool evaluable    = false;
  bool defined      = false;
  bool isFunction   = false;
  
  result = qp->ResultStorage( s );

  if ( ((CcString*)args[0].addr)->IsDefined() )
  {
    // create the query list (count (feed <relname>))
    relname = (char*)(((CcString*)args[0].addr)->GetStringval());
    string querystring = "(count(feed " + (string)relname + "))";
    nl->ReadFromString(querystring, queryList);
   
    // construct the operator tree within a new query processor instance
    // NOTE: variable name for this instance must differ from qp 
    qpp = new QueryProcessor( nl, SecondoSystem::GetAlgebraManager() );
    level = SecondoSystem::GetAlgebraLevel();
    qpp->Construct( level, queryList, correct, 
      evaluable, defined, isFunction, tree, resultType );
    if ( !defined )
    {
      cout << "object value is undefined" << endl;         
    }
    else if ( correct )
    {
      if ( evaluable )
      {
	// evaluate the operator tree
        qpp->Eval( tree, result, 1 );

	// create the result list ( type, value )
        valueList = SecondoSystem::GetCatalog( level )->
          OutObject( resultType, result );
        resultList = nl->TwoElemList( resultType, valueList );

	// set the result value and destroy the operator tree
        ((CcInt *)result.addr)->Set ( true, 
	  nl->IntValue(nl->Second(resultList)) );
	qpp->Destroy( tree, false );
      }
      else cout << "Operator query not evaluable" << endl;
    }
    else cout << "Error in operator query" << endl;
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  delete qpp;
  return (0);
}

/*
4.21 Value mapping function of operator ~relcount2~

*/

static int
RelcountFun2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  char* relname;
  string querystring;
  Word resultword;
  
  result = qp->ResultStorage( s );

  if ( ((CcString*)args[0].addr)->IsDefined() )
  {
    // create the query list (count (feed <relname>))
    relname = (char*)(((CcString*)args[0].addr)->GetStringval());
    querystring = "(count(feed " + (string)relname + "))";

    // the if statement can be simply reduced to:
    // if ( QueryProcessor::ExecuteQuery(querystring, result) )
    if ( QueryProcessor::ExecuteQuery(querystring, resultword) )
    {
      ((CcInt *)result.addr)->Set( true, 
      ((CcInt*)resultword.addr)->GetIntval() );
    }
    else cout << "Error in executing operator query" << endl;
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.21 Value mapping function of operator ~keywords~

The following auxiliary function ~trim~ removes any kind of space
characters from the end of a string.

*/

int trim (char s[])
{
  int n;
  
  for(n = strlen(s) - 1; n >= 0; n--)
   if ( !isspace(s[n]) ) break;
  s[n+1] = '\0';
  return n;
}

int
keywordsFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Creates a stream of strings containing the single words
of the origin string, on the assumption, that words in a string
are separated by a space character.

*/
{
  struct Subword {int start, nochr, strlength; STRING* subw;}* subword;

  CcString* elem, *str;
  //CcString* str;
  int i;
  string teststr, tmpstr;
  STRING outstr;
  Word arg0;

  switch( message )
  {
    case OPEN:
      // cout << "open" << endl;
      qp->Request(args[0].addr, arg0);

      str = ((CcString*)arg0.addr);

      subword = new Subword;
      subword->start = 0;
      subword->nochr = 0;
      
      subword->subw = (STRING*)malloc(strlen(*str->GetStringval()) + 1);
      // copy input string to allocated memory      
      strcpy(*subword->subw, *str->GetStringval());
      trim( *subword->subw ); //remove spaces from the end of the string
      
      subword->strlength = strlen(*subword->subw);
      (*subword->subw)[subword->strlength] = '\0';
      // get the necessary values to determine the first single word in the 
      // string, if it is not empty or contains only space characters. 
      if ( subword->strlength > 0) {
        i=0;
        while ( isspace(*subword->subw[i]) ) i++;     
        subword->start = i;
      
        i=subword->start;
        while ( (!(isspace(((*subword->subw)[i]))) && 
	        ((*subword->subw)[i]) != '\0') ) i++;
        subword->nochr = i - subword->start;
      }
      local.addr = subword;
      return 0;

    case REQUEST:
      //  cout << "request" << endl;
      subword = ((Subword*) local.addr);
      // another single word in the string still exists
      if ( (subword->strlength > 0) && (subword->start < subword->strlength) )
      { 
        tmpstr = (((string)(*subword->subw)).substr(subword->start,subword->nochr));
	strcpy(outstr, (char*)tmpstr.c_str());
        elem = new CcString(true, &outstr);
	result.addr = elem;
        // determine the necessary values to get the next word in the string,
	// if there is any.
	subword->start += subword->nochr;
        i = subword->start;
	if (i < subword->strlength ) {
          while ( isspace((*subword->subw)[i]) ) i++; 

          subword->start = i;	
          while ( (!isspace((*subword->subw)[i])) && 
	          (((*subword->subw)[i]) != '\0') )  i++;
          subword->nochr = i - subword->start + 1;
	}		
	local.addr = subword;

	return YIELD;
      }
      // no more single words in the string
      else
      {
        // string is empty or contains only space characters
        if ( subword->strlength == 0 ) {
	  outstr[0] = '\0';
          elem = new CcString(true, &outstr);
	  result.addr = elem;
	  subword->start = subword->strlength = 1;
	  local.addr = subword;
	  return YIELD;
        }	  
      return CANCEL;
      }
       
    case CLOSE:
      // cout << "close" << endl;
      subword = ((Subword*) local.addr);
      free(subword->subw);
      delete subword;
      return 0;
  }
  /* should not happen */
  return -1;
}

/*
1.10 Operator Model Mappings

*/

Word
IiGreaterModel( ArgVector arg, Supplier opTreeNode )
{
  Word modelWord1, modelWord2;
  float* result = new float; // Should this be a CcReal ???

  qp->RequestModel( arg[0].addr, modelWord1 );
  qp->RequestModel( arg[1].addr, modelWord2 );

  IntSetModel* model1 = (IntSetModel*) modelWord1.addr;
  IntSetModel* model2 = (IntSetModel*) modelWord2.addr;

  if ( model1->constOrSet == Const )
  {
    if ( model2->constOrSet == Const )
    {
      /* both models have constants */
      *result = (model1->value > model2->value ) ? 1.0 : 0.0;
    }
    else
    {
      /* model2 has a set, model1 a constant */
      if ( model1->value <= model2->t.min )
      {
        *result = 0.0;
      }
      else if ( model1->value > model2->t.max )
      {
        *result = 1.0;
      }
      else
      {
        *result = (float)((model1->value - 1) - (model2->t.min - 1)) /
                  (float)(model2->t.max - (model2->t.min - 1));
      }
    }
  }
  else
  {
    if ( model2->constOrSet == Const )
    {
      /* model1 has a set, model2 a constant */
      if ( model1->t.min > model2->value )
      {
        *result = 1.0;
      }
      else if ( model1->t.max <= model2->value )
      {
        *result = 0.0;
      }
      else
      {
        *result = (float)(model1->t.max - model2->value) /
                  (float)(model1->t.max - (model1->t.min - 1));
      }
    }
    else
    {
      /* both models describe sets: more tricky, to be solved later */
      *result = 0.5;
    }
  }
  return (SetWord( result ));
}

/*
The dummy model mapping:

*/

Word
CcNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

/*
5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

*/

ValueMapping ccplusmap[] = { CcPlus_ii, CcPlus_ir, CcPlus_ri, CcPlus_rr };
ValueMapping ccminusmap[] = { CcMinus_ii, CcMinus_ir, CcMinus_ri, CcMinus_rr };
ValueMapping ccproductmap[] = { CcProduct_ii, CcProduct_ir, CcProduct_ri, CcProduct_rr };
ValueMapping ccdivisionmap[] = { CcDivision_ii, CcDivision_ir, CcDivision_ri, CcDivision_rr };

ValueMapping ccmodmap[] = { CcMod };
ValueMapping ccdivmap[] = { CcDiv };

ValueMapping cclessmap[] = { CcLess_ii, CcLess_ir, CcLess_ri, CcLess_rr,
                             CcLess_bb, CcLess_ss};
ValueMapping cclessequalmap[] = { CcLessEqual_ii, CcLessEqual_ir, CcLessEqual_ri,
                                  CcLessEqual_rr, CcLessEqual_bb, CcLessEqual_ss };
ValueMapping ccgreatermap[] = { CcGreater_ii, CcGreater_ir, CcGreater_ri,
                                CcGreater_rr, CcGreater_bb, CcGreater_ss };
ValueMapping ccgreaterequalmap[] = { CcGreaterEqual_ii, CcGreaterEqual_ir,
                                     CcGreaterEqual_ri, CcGreaterEqual_rr,
                                     CcGreaterEqual_bb, CcGreaterEqual_ss };
ValueMapping ccequalmap[] = { CcEqual_ii, CcEqual_ir, CcEqual_ri, CcEqual_rr,
                              CcEqual_bb,  CcEqual_ss };
ValueMapping ccdiffmap[] = { CcDiff_ii, CcDiff_ir, CcDiff_ri, CcDiff_rr,
                             CcDiff_bb, CcDiff_ss };

ValueMapping ccstartsmap[] = { StartsFun };
ValueMapping cccontainsmap[] = { ContainsFun };
ValueMapping ccandmap[] = { AndFun };
ValueMapping ccormap[] = { OrFun };
ValueMapping ccnotmap[] = { NotFun };
ValueMapping ccisemptymap[] = { IsEmpty_b, IsEmpty_i, IsEmpty_r, IsEmpty_s };
ValueMapping ccsetintersectionmap[] = { CcSetIntersection_ii, CcSetIntersection_rr, CcSetIntersection_bb, CcSetIntersection_ss };
ValueMapping ccsetminusmap[] = { CcSetMinus_ii, CcSetMinus_rr, CcSetMinus_bb, CcSetMinus_ss };
ValueMapping ccoprelcountmap[] = { RelcountFun };
ValueMapping ccoprelcountmap2[] = { RelcountFun2 };
ValueMapping cckeywordsmap[] = { keywordsFun };

ModelMapping ccnomodelmap[] = { CcNoModelMapping, CcNoModelMapping, CcNoModelMapping,
                                CcNoModelMapping, CcNoModelMapping, CcNoModelMapping };
ModelMapping ccgtmodelmap[] = { IiGreaterModel, CcNoModelMapping, CcNoModelMapping,
                                CcNoModelMapping, CcNoModelMapping, CcNoModelMapping };


const string CCSpecAdd  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> int, (int real) -> real, "
			  "(real int)"
			  " -> real, (real real) -> real</text--->"
			       "<text>_ + _</text--->"
			       "<text>Addition.</text--->"
			       "<text>query -1.2 + 7</text--->"
			      ") )";

const string CCSpecSub  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> int, (int real) -> real, "
			  "(real int) -> real, (real real) -> real</text--->"
			       "<text>_ - _</text--->"
			       "<text>Subtraction</text--->"
			       "<text>query -.2 - 4</text--->"
			      ") )";

const string CCSpecMul  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> int, (int real) -> real, "
			  "(real int) -> real, (real real) -> real</text--->"
			       "<text>_ * _</text--->"
			       "<text>Multiplication.</text--->"
			       "<text>query 5 * 1.4 </text--->"
			      ") )";

const string CCSpecDiv  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> real, (int real) -> real, "
			  "(real int) -> real, (real real) -> real</text--->"
			       "<text>_ / _</text--->"
			       "<text>Division.</text--->"
			       "<text>query 5 / 2 </text--->"
			      ") )";

const string CCSpecMod  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(int int) -> int</text--->"
			       "<text>_ mod _</text--->"
			       "<text>Modulo.</text--->"
			       "<text>query 8 mod 3 </text--->"
			      ") )";

const string CCSpecDiv2  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" )"
                             "( <text>(int int) -> int</text--->"
			       "<text>_ div _</text--->"
			       "<text>Integer Division.</text--->"
			       "<text>query 5 div 2 </text--->"
			      ") )";

const string CCSpecRandInt  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>int -> int </text--->"
			       "<text>randint ( _ )</text--->"
			       "<text>Returns a random integer between 0 and "
			       "arg-1, the argument must be at least 2 otherwise it is set to 2. </text--->"
			       "<text>query randint (9)</text--->"
			      ") )";

const string CCSpecMaxRand  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text> -> int </text--->"
			       "<text>maxrand</text--->"
			       "<text>Returns the value of MAX_RAND </text--->"
			       "<text>query maxrand</text--->"
			      ") )";


const string CCSpecInitSeq  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>int -> bool </text--->"
			       "<text>intitseq ( _ ) </text--->"
			       "<text>Returns true and sets the start value "
                               " of the sequence to the argument value</text--->"
			       "<text>query initseq (100)</text--->"
			      ") )";

const string CCSpecNextSeq  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text> -> int </text--->"
			       "<text>nextseq ()</text--->"
			       "<text>Returns s+n-1 at the n-th call when the sequence"
                               " was initialzed with intiseq (s) otherwise s defaults to 0.</text--->"
			       "<text>query nextseq ()</text--->"
			      ") )";

const string CCSpecLog  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>int -> int </text--->"
			       "<text>log ( _ )</text--->"
			       "<text>Computes the logarithmus of base 2."
			       " The argument must be greater"
			       "than 0.</text--->"
			       "<text>query log (256)</text--->"
			      ") )";

const string CCSpecLT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
			 "(real int) -> bool, (real real) -> bool, (bool bool)"
			 " -> bool, (string string) -> bool</text--->"
			       "<text>_ < _</text--->"
			       "<text>Less.</text--->"
			       "<text>query \"house\" < \"hotel\"</text--->"
			      ") )";

const string CCSpecLE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
			 "(real int) -> bool, (real real) -> bool, (bool bool)"
			 " -> bool, (string string) -> bool</text--->"
			       "<text>_ <= _</text--->"
			       "<text>Less or equal.</text--->"
			       "<text>query 8.2 <= 8.2</text--->"
			      ") )";

const string CCSpecGT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
			 "(real int) -> bool, (real real) -> bool, (bool bool)"
			 " -> bool, (string string) -> bool</text--->"
			       "<text>_ > _</text--->"
			       "<text>Greater.</text--->"
			       "<text>query 3 > 4.1</text--->"
			      ") )";

const string CCSpecGE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
			 "(real int) -> bool, (real real) -> bool, (bool bool)"
			 " -> bool, (string string) -> bool</text--->"
			       "<text>_ >= _</text--->"
			       "<text>Greater or equal.</text--->"
			       "<text>query 3 >= 5</text--->"
			      ") )";

const string CCSpecEQ  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
			 "(real int) -> bool, (real real) -> bool, (bool bool)"
			 " -> bool, (string string) -> bool</text--->"
			       "<text>_ = _</text--->"
			       "<text>Equal.</text--->"
			       "<text>query 2.1 = 2.01</text--->"
			      ") )";

const string CCSpecNE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
			 "(real int) -> bool, (real real) -> bool, (bool bool)"
			 " -> bool, (string string) -> bool</text--->"
			       "<text>_ # _</text--->"
			       "<text>Not equal.</text--->"
			       "<text>query 2.1 # 2.01</text--->"
			      ") )";

const string CCSpecBeg  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(string string) -> bool</text--->"
			       "<text>_ starts _</text--->"
			       "<text>Starts.</text--->"
			       "<text>query \"starts\" starts \"st\"</text--->"
			      ") )";

const string CCSpecCon  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(string string) -> bool</text--->"
			       "<text>_ contains _</text--->"
			       "<text>Contains.</text--->"
			    "<text>query \"contains\" contains \"tai\""
			    "</text--->"
			      ") )";

const string CCSpecNot  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>bool -> bool</text--->"
			       "<text>not ( _ )</text--->"
			       "<text>Logical Not.</text--->"
			       "<text>query not ( 4=4 )</text--->"
			     " ) )";

const string CCSpecAnd  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(bool bool) -> bool</text--->"
			       "<text>_ and _</text--->"
			       "<text>Logical And.</text--->"
			       "<text>query (8 = 8) and (3 < 4)</text--->"
			      ") )";

const string CCSpecOr  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(bool bool) -> bool</text--->"
			       "<text>_ or _</text--->"
			       "<text>Logical Or.</text--->"
			       "<text>query (3 <= 4) or (\"hotel\" > "
			       "\"house\")"
			       "</text--->"
			       ") )";

const string CCSpecIsEmpty  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>bool -> bool, int -> bool, real -> bool,"
			     " string -> bool</text--->"
			     "<text>isempty ( _ )</text--->"
			     "<text>Returns whether the value is defined or "
			     "not.</text--->"
			       "<text>query isempty ( 8 )</text--->"
			      ") )";

const string CCSpecUpper  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>string -> string</text--->"
			       "<text>upper ( _ )</text--->"
			       "<text>Returns a string immediately upper to"
			       " the original one.</text--->"
			       "<text>query upper ( \"hello\" )</text--->"
			       ") )";

const string CCSpecSetIntersection  = "( ( \"Signature\" \"Meaning\" )"
                             "( <text> (int int) -> int, (real real) -> real, "
			     "(bool bool) -> bool, (string string) -> "
			     "string</text--->"
			     "<text> Set intersection. </text--->"
			     ") )";

const string CCSpecSetMinus  = "( ( \"Signature\" \"Meaning\" )"
                             "( <text> (int int) -> int, (real real) -> real, "
			     "(bool bool) -> bool, (string string) -> "
			     "string</text--->"
			     "<text> Set minus. </text--->"
			       			      ") )";
                        
const string CCSpecRelcount  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>string -> int</text--->"
             "<text>_ relcount</text--->"
             "<text>Counts the number of tuples of a relation, "
             "which is specified by its objectname"
             " of type string.</text--->"
             "<text>query \"Staedte\" relcount</text--->"
             ") )";

const string CCSpecRelcount2  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>string -> int</text--->"
             "<text>_ relcount2</text--->"
             "<text>Counts the number of tuples of a relation, "
             "which is specified by its objectname"
             " of type string. Uses static method ExecuteQuery"
             " from class QueryProcessor.</text--->"
             "<text>query \"Orte\" relcount2</text--->"
             ") )";
	                            
const string CCSpecKeywords  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(string) -> (stream string)</text--->"
             "<text>_ keywords</text--->"
             "<text>Creates a stream of strings containing the single words"
             " of the origin string, on the assumption, that words in a string"
             " are separated by a space character.</text--->"
             "<text>query ten feed extendstream(name: mystring keywords) consume</text--->"
             ") )"; 

Operator ccplus( "+", CCSpecAdd, 4, ccplusmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMap );
Operator ccminus( "-", CCSpecSub, 4, ccminusmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMap );
Operator ccproduct( "*", CCSpecMul, 4,ccproductmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMap );
Operator ccdivision( "/", CCSpecDiv, 4, ccdivisionmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMapdiv );
Operator ccmod( "mod", CCSpecMod, 1, ccmodmap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMap1 );
Operator ccdiv( "div", CCSpecDiv2, 1, ccdivmap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMap1 );
Operator ccrandint( "randint", CCSpecRandInt, RandInt, Operator::DummyModel, Operator::SimpleSelect, IntInt );
Operator ccrandmax( "randmax", CCSpecMaxRand, MaxRand, Operator::DummyModel, Operator::SimpleSelect, EmptyInt );
Operator ccseqinit( "seqinit", CCSpecInitSeq, InitSeq, Operator::DummyModel, Operator::SimpleSelect, IntBool );
Operator ccseqnext( "seqnext", CCSpecNextSeq, NextSeq, Operator::DummyModel, Operator::SimpleSelect, EmptyInt );
Operator cclog( "log", CCSpecLog, LogFun, Operator::DummyModel, Operator::SimpleSelect, IntInt );
Operator ccless( "<", CCSpecLT, 6, cclessmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator cclessequal( "<=", CCSpecLE, 6, cclessequalmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccgreater( ">", CCSpecGT, 6, ccgreatermap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccgreaterequal( ">=", CCSpecGE, 6, ccgreaterequalmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccequal( "=", CCSpecEQ, 6, ccequalmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccdiff( "#", CCSpecNE, 6, ccdiffmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccstarts( "starts", CCSpecBeg, 1, ccstartsmap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMapBool3 );
Operator cccontains( "contains", CCSpecCon, 1, cccontainsmap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMapBool3 );
Operator ccnot( "not", CCSpecNot, 1, ccnotmap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMapBool1 );
Operator ccand( "and", CCSpecAnd, 1, ccandmap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMapBool2 );
Operator ccor( "or", CCSpecOr, 1, ccormap, ccnomodelmap, Operator::SimpleSelect, CcMathTypeMapBool2 );
Operator ccisempty( "isempty", CCSpecIsEmpty, 4, ccisemptymap, ccnomodelmap, CcMathSelectIsEmpty, CcMathTypeMapBool4 );
Operator ccuper( "upper", CCSpecUpper, Upper, Operator::DummyModel, Operator::SimpleSelect, CcStringMapCcString );
Operator ccsetintersection( "intersection", CCSpecSetIntersection, 4, ccsetintersectionmap, ccnomodelmap, CcMathSelectSet, CcMathTypeMap2 );
Operator ccsetminus( "minus", CCSpecSetMinus, 4, ccsetminusmap, ccnomodelmap, CcMathSelectSet, CcMathTypeMap2 );
Operator ccoprelcount( "relcount", CCSpecRelcount, 1, ccoprelcountmap, ccnomodelmap, Operator::SimpleSelect, CcStringMapCcInt );
Operator ccoprelcount2( "relcount2", CCSpecRelcount2, 1, ccoprelcountmap2, ccnomodelmap, Operator::SimpleSelect, CcStringMapCcInt );
Operator ccopkeywords( "keywords", CCSpecKeywords, 1, cckeywordsmap, ccnomodelmap, Operator::SimpleSelect, keywordsType );

/*
6 Class ~CcAlgebra~

The last steps in adding an algebra to the Secondo system are

  * Associating value mapping functions with their operators

  * ``Bunching'' all
type constructors and operators in one instance of class ~Algebra~.

Therefore, a new subclass ~CcAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~ccalgebra1~ is defined.

*/

class CcAlgebra1 : public Algebra
{
 public:
  CcAlgebra1() : Algebra()
  {
    AddTypeConstructor( &ccInt );
    AddTypeConstructor( &ccReal );
    AddTypeConstructor( &ccBool );
    AddTypeConstructor( &ccString );

    ccInt.AssociateKind( "DATA" );
    ccReal.AssociateKind( "DATA" );
    ccBool.AssociateKind( "DATA" );
    ccString.AssociateKind( "DATA" );

    ccInt.AssociateKind( "BASE" );
    ccReal.AssociateKind( "BASE" );
    ccBool.AssociateKind( "BASE" );
    ccString.AssociateKind( "BASE" );

    AddOperator( &ccplus );
    AddOperator( &ccminus );
    AddOperator( &ccproduct );
    AddOperator( &ccdivision );
    AddOperator( &ccmod );
    AddOperator( &ccdiv );
    AddOperator( &ccrandint );
    AddOperator( &ccrandmax );
    AddOperator( &ccseqinit );
    AddOperator( &ccseqnext );
    AddOperator( &cclog );
    AddOperator( &ccless );
    AddOperator( &cclessequal );
    AddOperator( &ccgreater );
    AddOperator( &ccgreaterequal );
    AddOperator( &ccequal );
    AddOperator( &ccdiff );
    AddOperator( &ccstarts );
    AddOperator( &cccontains );
    AddOperator( &ccnot );
    AddOperator( &ccand );
    AddOperator( &ccor );
    AddOperator( &ccisempty );
    AddOperator( &ccuper );
    AddOperator( &ccsetintersection );
    AddOperator( &ccsetminus );
    AddOperator( &ccoprelcount );
    AddOperator( &ccoprelcount2 );
    AddOperator( &ccopkeywords );
  }
  ~CcAlgebra1() {};
};

CcAlgebra1 ccalgebra1;

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
InitializeStandardAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&ccalgebra1);
}

/*
8 Query examples

Now we are ready to
compile this algebra implementation and link the resulting object
file with the Secondo library to a Secondo executable containing our
example algebra.

Here are some example queries:

  * "(list operators)"[1]

    Among others, the operators ~+~, ~-~, ~[*]~, and ~/~ are listed.

  * "(list type constructors)"[1]

    ~int~ , ~real~, ~bool~and ~string~ are part of the output.


  * "(query (+ 4 5))"[1]

    Result value 9 is returned.

  * "(query ([*] (+ 4 5.7)(- 2.5 4)))"[1]

    Result value -14.55 is returned.

  * "(query (or ( $ < $  4 5.7)( $ > $  2.5 4)))"[1]

    Result value TRUE is returned.

*/

ostream& ShowStandardTypesStatistics( const bool reset, ostream& o )
{
  o << "CcInts    created: " << CcInt::intsCreated << " / deleted: " << CcInt::intsDeleted << endl
    << "CcReals   created: " << CcReal::realsCreated << " / deleted: " << CcReal::realsDeleted << endl
    << "CcBools   created: " << CcBool::boolsCreated << " / deleted: " << CcBool::boolsDeleted << endl
    << "CcStrings created: " << CcString::stringsCreated << " / deleted: " << CcString::stringsDeleted << endl;
  if( reset )
  {
    CcInt::intsCreated = 0; CcInt::intsDeleted = 0;
    CcReal::realsCreated = 0; CcReal::realsDeleted = 0;
    CcBool::boolsCreated = 0; CcBool::boolsDeleted = 0;
    CcString::stringsCreated = 0; CcString::stringsDeleted = 0;
  }
  return o;
}

