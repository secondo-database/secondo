#ifndef DISPLAY_TTY_H
#define DISPLAY_TTY_H

#include <string>
#include <map>

#include "SecondoInterface.h"
#include "NestedList.h"

typedef void (*DisplayFunction)( ListExpr type, ListExpr numType, ListExpr value );
/*
There must be exactly one TTY display function of type DisplayFunction
for every type constructor provided by any of the algebra modules which
are loaded by *AlgebraManager*. The first parameter is the type
expression describing the value which is going to be displayed by the
display function. The second parameter is the type expression again,
but now in numeric nested list format. The third parameter is this
value in nested list format.

*/

class DisplayTTY
{
 public:
  static void Initialize( SecondoInterface* secondoInterface );
  static void DisplayResult( ListExpr type, ListExpr value );
 protected:
 private:
  DisplayTTY();
  ~DisplayTTY();
  static void InsertDisplayFunction( const string& name,
                                     DisplayFunction df );
  static void CallDisplayFunction( const ListExpr idPair,
                                   ListExpr type,
                                   ListExpr numType,
                                   ListExpr value );
  static void DisplayGeneric( ListExpr type, ListExpr numType,
                              ListExpr value );
  static void DisplayRelation( ListExpr type, ListExpr numType,
                               ListExpr value );
  static int  MaxAttributLength( ListExpr type );
  static void DisplayTuple( ListExpr type, ListExpr numType,
                            ListExpr value, const int maxNameLen );
  static void DisplayTuples( ListExpr type, ListExpr numType,
                             ListExpr value );
  static void DisplayInt( ListExpr type, ListExpr numType,
                          ListExpr value );
  static void DisplayReal( ListExpr type, ListExpr numType,
                           ListExpr value );
  static void DisplayBoolean( ListExpr list, ListExpr numType,
                              ListExpr value );
  static void DisplayString( ListExpr type, ListExpr numType,
                             ListExpr value );
  static void DisplayFun( ListExpr type, ListExpr numType,
                          ListExpr value );
  static SecondoInterface* si;
  static NestedList*       nl;
  static map<string,DisplayFunction> displayFunctions;
};

#endif

