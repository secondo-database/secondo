/*
//paragraph [1] Title:	[{\Large \bf ]	[}]

[1] Secondo Standardalgebra
 
Friedhelm Becker, Nov. 1998

August 16, 2000 RHG Changed includes to show dependencies more clearly.

March 2002 Ulrich Telle Port to C++

November 9, 2002 RHG Added operators ~randint~ and ~log~. Some other slight revisions.
 
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

----	int -> int
----

Computes a random integer within the range [0, arg-1]. The argument must be
greater than 0. Otherwise it is set to 2.

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
#include <iostream>
#include <string>
#include <math.h>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>

static NestedList* nl;
static QueryProcessor* qp;

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


static CcType
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

3.1 Type properties

*/

static ListExpr
CcProperty()
{
  return (nl->TwoElemList(
            nl->OneElemList( nl->SymbolAtom( "" ) ),
            nl->SymbolAtom( "DATA" ) ));
}

/*
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

CcInt::CcInt() {};
CcInt::CcInt( bool d, int v ) { defined = d; intval = v; };
CcInt::~CcInt() {};
void   CcInt::Set( int v ) { defined = true, intval = v; };
void   CcInt::Set( bool d, int v ) { defined = d, intval = v; };
int    CcInt::GetIntval() { return (intval); };
void*  CcInt::GetValue() { return (void *)intval;};
bool   CcInt::IsDefined() { return (defined); };
int    CcInt::Sizeof() { return (sizeof(CcInt)); };
CcInt* CcInt::Clone() { return (new CcInt( *this )); };
size_t CcInt::HashValue() { return (defined ? intval : 0); };

void CcInt::CopyFrom(StandardAttribute* right)
{
  CcInt* r = (CcInt*)right;
  defined = r->defined;
  intval = r->intval;
}

int    CcInt::Compare( Attribute* arg )
{
//        CcInt* p = dynamic_cast< CcInt* >(arg);
  CcInt* p =  (CcInt*)(arg);
  if ( !p )                 return (-2);
  if ( intval < p->intval ) return (-1);
  if ( intval > p->intval)  return (1);
  return (0);
};

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

static ListExpr
OutCcInt( ListExpr typeinfo, Word value )
{
  return (nl->IntAtom( ((CcInt*)value.addr)->GetIntval() ));
}

/*
The function ~InInt~ provides a functionality complementary to ~OutInt~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters.

*/

static Word
InCcInt( ListExpr typeInfo, ListExpr value,
         int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == IntType )
  {
    correct = true;
    return (SetWord( new CcInt( true, nl->IntValue( value ) ) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*

*/

static Word
CreateCcInt( int Size )
{
  return (SetWord( new CcInt( false, 0 ) ));
}

static void
DeleteCcInt( Word& w )
{
  delete (CcInt*) w.addr;
  w.addr = 0;
}

/*
3.2.5 {\em Cast}-function of type constructor {\tt ccint}

*/

static void*
CastInt( void* addr )
{
  return (new (addr) CcInt);
}

/*
3.2.6 {\em Type check} function of type constructor {\tt ccint}

*/

static bool
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

static ListExpr
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

static Word
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

static Word
IntToIntSetModel( ListExpr typeExpr, Word value )
{
  IntSetModel* model = new IntSetModel;
  model->constOrSet = Const;
  model->value = ((CcInt*)value.addr)->GetIntval();
  return (SetWord( model ));
}

static Word
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

TypeConstructor ccInt( "int",            CcProperty,
                       OutCcInt,         InCcInt,        CreateCcInt,
                       DeleteCcInt,      CastInt,        CheckInt,
                       0,                0,
                       InIntSetModel,    OutIntSetModel,
                       IntToIntSetModel, IntListToIntSetModel );

/*
3.2 Type constructor *ccreal*

The following type constructor, ~ccreal~, is defined in the same way as
~ccint~.

*/

CcReal::CcReal(){};
CcReal::CcReal( bool d, float v ) { defined = d; realval = v; };
CcReal::~CcReal(){};
bool    CcReal::IsDefined() { return (defined); };
float   CcReal::GetRealval() { return (realval);};
void*   CcReal::GetValue() { return ((void*)-1); };
void    CcReal::Set( float v ) { defined = true, realval = v; };
void    CcReal::Set( bool d, float v ) { defined = d, realval = v; };
int     CcReal::Sizeof() { return (sizeof(CcReal)); };
CcReal* CcReal::Clone() { return (new CcReal(*this)); };

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
   //     CcReal* p = dynamic_cast< CcReal* >(arg);
  CcReal* p = (CcReal* )(arg);
  if ( !p )                    return (-2);
  if ( realval < p-> realval ) return (-1);
  if ( realval > p-> realval ) return (1);
  return (0);
};

static ListExpr
OutCcReal( ListExpr typeinfo, Word value )
{
  return (nl->RealAtom( ((CcReal*)value.addr)->GetRealval() ));
}

static Word
InCcReal( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == RealType )
  {
    correct = true;
    return (SetWord( new CcReal( true, nl->RealValue( value )) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

static Word
CreateCcReal( int size )
{
  return (SetWord( new CcReal( false, 0 ) ));
}

static void
DeleteCcReal( Word& w )
{
  delete (CcReal*)w.addr;
  w.addr = 0;
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}
 
*/

static void*
CastReal( void* addr )
{
  return new (addr) CcReal;
}


/*
3.3.7 {\em Type check} function of type constructor {\tt ccreal}

*/

static bool
CheckReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "real" ));
}


TypeConstructor ccReal( "real",       CcProperty,
                        OutCcReal,    InCcReal,   CreateCcReal,
                        DeleteCcReal, CastReal,   CheckReal );

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

CcBool::CcBool(){};
CcBool::CcBool( bool d, int v ){ defined  = d; boolval = v; };
CcBool::~CcBool(){};
void    CcBool::Set( bool d, bool v ){ defined = d, boolval = v; };
bool    CcBool::IsDefined() { return defined; };
bool    CcBool::GetBoolval() { return boolval; };
void*   CcBool::GetValue() { return (void *)boolval; };
int     CcBool::Sizeof() { return sizeof(CcBool); };
CcBool* CcBool::Clone() { return new CcBool(*this); };
size_t CcBool::HashValue() { return (defined ? boolval : false); };

void CcBool::CopyFrom(StandardAttribute* right)
{
  CcBool* r = (CcBool*)right;
  defined = r->defined;
  boolval = r->boolval;
}

int     CcBool::Compare(Attribute* arg)
{
   //     CcBool* p = dynamic_cast< CcBool* >(arg);
  CcBool* p = (CcBool*)(arg);
  if ( !p )                    return (-2);
  if ( boolval < p-> boolval ) return (-1);
  if ( boolval > p-> boolval ) return (1);
  return (0);
};

/*
Now we define a function, ~OutBool~, that takes as inputs a type description
and a pointer to a value of this type. The representation of this value in
nested list format is returned.

*/

static ListExpr
OutCcBool( ListExpr typeinfo, Word value )
{
  return (nl->BoolAtom( ((CcBool*)value.addr)->GetBoolval() ));
}

/*
The function ~InBool~ provides a functionality complementary to ~OutBool~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters. Again, we don't need the type information in this example due to
simplicity of types used.

The next three parameters are used to return error information.

*/

static Word
InCcBool( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == BoolType )
  {
    correct = true;
    return (SetWord( new CcBool( true, nl->BoolValue( value ) ) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

static Word
CreateCcBool( int size )
{
  return (SetWord( new CcBool( false, 0 ) ));
}

static void
DeleteCcBool( Word& w )
{
  delete (CcBool*)w.addr;
  w.addr = 0;
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

static void*
CastBool( void* addr )
{
  return (new (addr) CcBool);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt ccreal}

*/

static bool
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

static ListExpr
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

static Word
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

static Word
BoolToBoolSetModel( ListExpr typeExpr, Word value )
{
  BoolSetModel* result = new BoolSetModel;
  *result = (((CcBool*)value.addr)->GetBoolval()) ? 1.0 : 0.0;
  return (SetWord( result ));
}

static Word
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

TypeConstructor ccBool( "bool",             CcProperty,
                        OutCcBool,          InCcBool,        CreateCcBool,
                        DeleteCcBool,       CastBool,        CheckBool,
                        0,                  0,
                        InBoolSetModel,     OutBoolSetModel,
                        BoolToBoolSetModel, BoolListToBoolSetModel );

/*
3.5 Type constructor *CcString*

*/

CcString::CcString() {};
CcString::CcString( bool d, const STRING* v ) { defined = d; strcpy( stringval, *v); };
CcString::~CcString() {};
bool      CcString::IsDefined() { return (defined); };
STRING*   CcString::GetStringval() { return (&stringval); };
void*     CcString::GetValue() { return ((void*) &stringval); };
int       CcString::Sizeof() { return (sizeof(CcString)); };
CcString* CcString::Clone() { return (new CcString( *this )); };

size_t CcString::HashValue()
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
   //     CcString* p = dynamic_cast< CcString* >(arg);
  CcString* p = (CcString*)(arg);
  if ( !p ) return (-2);
  if ( strcmp(stringval , p->stringval) < 0) return (-1);
  if ( !strcmp(stringval , p->stringval)) return (0);
  return (1);
  // return (stringval.compare( p->stringval ));
};

/*

*/

static ListExpr
OutCcString( ListExpr typeinfo, Word value )
{
  return (nl->StringAtom( *((CcString*)value.addr)->GetStringval() ));
}

/*

*/

static Word
InCcString( ListExpr typeInfo, ListExpr value,
            int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == StringType )
  {
    correct = true;
    string s = nl->StringValue( value );
    return (SetWord( new CcString( true, (STRING*)s.c_str() ) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*

*/

static Word
CreateCcString( int size )
{
  char p[49] = "";
  return (SetWord( new CcString( true, (STRING*)&p ) ));
}

static void
DeleteCcString( Word& w )
{
  delete (CcString*)w.addr;
  w.addr = 0;
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

static void*
CastString( void* addr )
{
  return (new (addr) CcString);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt ccreal}
 
*/
 
static bool
CheckString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "string" ));
}

TypeConstructor ccString( "string",       CcProperty,
                          OutCcString,    InCcString, CreateCcString,
                          DeleteCcString, CastString, CheckString );

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

static ListExpr
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

static ListExpr
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

static ListExpr
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
4.2.x Type mapping function IntInt

Used for operators ~randint~ and ~log~.

*/

static ListExpr
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



/*
4.2.4 Type mapping function CcMathTypeMapBool

It is for the Compare operators which have ~bool~ as resulttype.

*/

static ListExpr
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
4.2.5 Type mapping function CcMathTypeMapBool1

It is for the  operator ~not~ which have ~bool~ as input and resulttype.

*/

static ListExpr
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
4.2.6 Type mapping function CcMathTypeMapBool2

It is for the  operators and and or  which have bool as input and resulttype.

*/

static ListExpr
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
4.2.7 Type mapping function CcMathTypeMapBool3

It is for the  operators ~starts~ and ~contains~  which have ~string~ as input and ~bool~ resulttype.

*/

static ListExpr
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

static int
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
4.3.2 Selection function SimpleSelect

Is used for all non-overloaded operators.

*/

static int
SimpleSelect( ListExpr args ) {return (0);}

/*
4.3.3 Selection function  CcMathSelectCompare

It is used for the  all compare operators .

*/

static int
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

static int
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

static int
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

static int
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

static int
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

static int
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

static int
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

static int
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

static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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

static int
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
4.8 Value mapping function of operator ~randint~

*/

int randint(int u)    	//Computes a random integer in the range 0..u-1,
			//for u >= 2
{
  int r = rand();
  if ( u < 2 ) {u=2;}
  return (r % u);
}


static int
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


/*
4.8 Value mapping function of operator ~log~

*/

int intlog(int n)
{
  int i = 0;
  while (n > 1) {n >>= 1; i++;}
  return i;
}


static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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

static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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
 
static int
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

static int
StartsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    string str1 = (string)(char*)((CcString*)args[0].addr)->GetStringval();
    string str2 = (string)(char*)((CcString*)args[1].addr)->GetStringval();
    ((CcBool *)result.addr)->
    Set (true, str1.compare(str2, 0, str2.length()) == 0 );
    //((CcBool *)result.addr)->
      //Set( true, ( ((CcString*)args[0].addr)->GetStringval()->compare( 
                  //*((CcString*)args[1].addr)->GetStringval(), 0,
                   //((string)((CcString*)args[1].addr)->GetStringval()).length() ) == 0) );
		   //teststr.length() ) == 0 );
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

static int
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

static int
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

static int
AndFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( (((CcBool*)args[0].addr)->IsDefined() &&
      (!((CcBool*)args[0].addr)->GetBoolval())) ||
        ((CcBool*)args[1].addr)->IsDefined() &&
      (!((CcBool*)args[1].addr)->GetBoolval()) )
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  else
  {
    if ( ((CcBool*)args[0].addr)->IsDefined() &&
         ((CcBool*)args[1].addr)->IsDefined() )
    {
      ((CcBool *)result.addr)->Set( true, true );
    }
    else
    {
      ((CcBool *)result.addr)->Set( false, false );
    }
  }
  return (0);
}
//if(((CcBool*)args[0])->IsDefined() && ((CcBool*)args[1])->IsDefined())
//  ((CcBool *)*result)->Set(1, ((CcBool*)args[0])->GetBoolval() &&
//                          ((CcBool*)args[1])->GetBoolval());

/*
4.17 Value mapping functions of operator ~or~

*/

static int
OrFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( (((CcBool*)args[0].addr)->IsDefined() &&
        ((CcBool*)args[0].addr)->GetBoolval()) ||
        ((CcBool*)args[1].addr)->IsDefined() &&
        ((CcBool*)args[1].addr)->GetBoolval() )
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  else
  {
    if ( ((CcBool*)args[0].addr)->IsDefined() &&
         ((CcBool*)args[1].addr)->IsDefined() )
    {
      ((CcBool *)result.addr)->Set( true, false );
    }
    else
    {
      ((CcBool *)result.addr)->Set( false, false );
    }
  }
  return (0);
}
// if(((CcBool*)args[0])->IsDefined() && ((CcBool*)args[1])->IsDefined())
//  ((CcBool *)*result)->Set(1, ((CcBool*)args[0])->GetBoolval() ||
//                          ((CcBool*)args[1])->GetBoolval());

/*
1.10 Operator Model Mappings

*/

static Word
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

static Word
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

ModelMapping ccnomodelmap[] = { CcNoModelMapping, CcNoModelMapping, CcNoModelMapping,
                                CcNoModelMapping, CcNoModelMapping, CcNoModelMapping };
ModelMapping ccgtmodelmap[] = { IiGreaterModel, CcNoModelMapping, CcNoModelMapping,
                                CcNoModelMapping, CcNoModelMapping, CcNoModelMapping };

const string CCSpecAdd  = "(<text> (int int) -> int, (int real) -> real, (real int) -> real, (real real) -> real </text---><text> Addition. </text--->)";
const string CCSpecSub  = "(<text> (int int) -> int, (int real) -> real, (real int) -> real, (real real) -> real</text---><text> Subtraction. </text--->)";
const string CCSpecMul  = "(<text> (int int) -> int, (int real) -> real, (real int) -> real, (real real) -> real</text---><text> Multiplication. </text--->)";
const string CCSpecDiv  = "(<text> (int int) -> real, (int real) -> real, (real int) -> real, (real real) -> real</text---><text> Division. </text--->)";
const string CCSpecMod  = "(<text> (int int) -> int</text---><text> Modulo. </text--->)";
const string CCSpecDiv2 = "(<text> (int int) -> int</text---><text> Integer Division. </text--->)";
const string CCSpecRandInt  = "(<text> int -> int </text---><text> Returns a random integer between 0 and arg-1, for arg > 1.</text--->)";
const string CCSpecLog  = "(<text> int -> int </text---><text> Returns (floor of) the base 2 logarithm of the argument.</text--->)";
const string CCSpecLT   = "(<text> (int int) -> bool, (int real) -> bool, (real int) -> bool, (real real) -> bool, (bool bool) -> bool, (string string) -> bool</text---><text> Less. </text--->)";
const string CCSpecLE   = "(<text> (int int) -> bool, (int real) -> bool, (real int) -> bool, (real real) -> bool, (bool bool) -> bool, (string string) -> bool</text---><text> Less or equal. </text--->)";
const string CCSpecGT   = "(<text> (int int) -> bool, (int real) -> bool, (real int) -> bool, (real real) -> bool, (bool bool) -> bool, (string string) -> bool</text---><text> Greater. </text--->)";
const string CCSpecGE   = "(<text> (int int) -> bool, (int real) -> bool, (real int) -> bool, (real real) -> bool, (bool bool) -> bool, (string string) -> bool</text---><text> Greater or equal. </text--->)";
const string CCSpecEQ   = "(<text> (int int) -> bool, (int real) -> bool, (real int) -> bool, (real real) -> bool, (bool bool) -> bool, (string string) -> bool</text---><text> Equal. </text--->)";
const string CCSpecNE   = "(<text> (int int) -> bool, (int real) -> bool, (real int) -> bool, (real real) -> bool, (bool bool) -> bool, (string string) -> bool</text---><text> Not equal. </text--->)";
const string CCSpecBeg  = "(<text> (string string) -> bool</text---><text> Starts. </text--->)";
const string CCSpecCon  = "(<text> (string string) -> bool</text---><text> Contains. </text--->)";
const string CCSpecNot  = "(<text> (bool bool) -> bool</text---><text> Logical Not. </text--->)";
const string CCSpecAnd  = "(<text> (bool bool) -> bool</text---><text> Logical And. </text--->)";
const string CCSpecOr   = "(<text> (bool bool) -> bool</text---><text> Logical Or. </text--->)";

Operator ccplus( "+", CCSpecAdd, 4, ccplusmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMap );
Operator ccminus( "-", CCSpecSub, 4, ccminusmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMap );
Operator ccproduct( "*", CCSpecMul, 4,ccproductmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMap );
Operator ccdivision( "/", CCSpecDiv, 4, ccdivisionmap, ccnomodelmap, CcMathSelectCompute, CcMathTypeMapdiv );
Operator ccmod( "mod", CCSpecMod, 1, ccmodmap, ccnomodelmap, SimpleSelect, CcMathTypeMap1 );
Operator ccdiv( "div", CCSpecDiv2, 1, ccdivmap, ccnomodelmap, SimpleSelect, CcMathTypeMap1 );
Operator ccrandint( "randint", CCSpecRandInt, RandInt, Operator::DummyModel, SimpleSelect, IntInt );
Operator cclog( "log", CCSpecLog, LogFun, Operator::DummyModel, SimpleSelect, IntInt );
Operator ccless( "<", CCSpecLT, 6, cclessmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator cclessequal( "<=", CCSpecLE, 6, cclessequalmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccgreater( ">", CCSpecGT, 6, ccgreatermap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccgreaterequal( ">=", CCSpecGE, 6, ccgreaterequalmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccequal( "=", CCSpecEQ, 6, ccequalmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccdiff( "#", CCSpecNE, 6, ccdiffmap, ccnomodelmap, CcMathSelectCompare, CcMathTypeMapBool );
Operator ccstarts( "starts", CCSpecBeg, 1, ccstartsmap, ccnomodelmap, SimpleSelect, CcMathTypeMapBool3 );
Operator cccontains( "contains", CCSpecCon, 1, cccontainsmap, ccnomodelmap, SimpleSelect, CcMathTypeMapBool3 );
Operator ccnot( "not", CCSpecNot, 1, ccnotmap, ccnomodelmap, SimpleSelect, CcMathTypeMapBool1 );
Operator ccand( "and", CCSpecAnd, 1, ccandmap, ccnomodelmap, SimpleSelect, CcMathTypeMapBool2 );
Operator ccor( "or", CCSpecOr, 1, ccormap, ccnomodelmap, SimpleSelect, CcMathTypeMapBool2 );

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

    AddOperator( &ccplus );
    AddOperator( &ccminus );
    AddOperator( &ccproduct );
    AddOperator( &ccdivision );
    AddOperator( &ccmod );
    AddOperator( &ccdiv );
    AddOperator( &ccrandint );
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

