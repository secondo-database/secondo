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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[i] [\'{\i}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Display TTY

May 1998 Friedhelm Becker

December 1998 Miguel Rodr[i]guez Luaces Ported for use in the client 
server version of "Secondo"[3]

May 2002 Ulrich Telle Port to new "Secondo"[3] version

August 2009 M. Spiekermann. Introduction of struct Displayfunction. 
A specific Displayfunction
must now be a child of this base class, refer to DisplayTTY.cpp for examples.

1.1 Overview

There must be exactly one TTY display function of type ~DisplayFunction~
for every  type constructor provided by any of the algebra modules
which are loaded by the ~AlgebraManager~. The first parameter is the
original type expression in nested list format, the second parameter is
the type expression in numeric nested list format describing the value
which is going to be displayed by the display function. The third
parameter is this value in nested list format.

1.2 Includes and Defines

*/
#ifndef DISPLAY_TTY_H
#define DISPLAY_TTY_H

#include <string>
#include <map>

#include "NestedList.h"

struct DisplayFunction {

  DisplayFunction() {}
  virtual ~DisplayFunction() {} 

  virtual void Display( ListExpr type,
                        ListExpr value    ) = 0;


  double GetNumeric(ListExpr value, bool &err);
  void DisplayResult( ListExpr type, ListExpr value );
  void CallDisplayFunction( const std::string& name,
                            ListExpr type,
                            ListExpr value );


  static NestedList*       nl; // Ref. to nested list container

  int MaxAttributLength( ListExpr type );
};

/*
The class above is a base class and abstraction for display functions.
User defined display functions need to inherit from this class.

*/

/*
1.2 Class "DisplayTTY"[1]

This class manages the display functions in the "Secondo"[3] TTY interface.
 he map ~displayFunctions~ holds all existing display functions. It
is indexed by a string consisting of the ~algebraId~ and the ~typeId~ of the
corresponding type constructor.

Display functions are used to  transform a nested list value into a pretty
printed output in text format. Display functions which are called with a
value of compound type usually call recursively the display functions of
the subtypes, passing the subtype and subvalue, respectively.

*/

class DisplayTTY
{
 public:
  static void Initialize();
/*
Initializes the display function mapping.

*/

 static void Finish();
/*
Removes the instance for displaying objects. 

*/

  

  bool DisplayResult( ListExpr type, ListExpr value );
/*
Displays a ~value~ of ~type~ using the defined display functions. Both
paramaters are given as nested lists.

*/

  void CallDisplayFunction( const std::string& name,
                            ListExpr type,
                            ListExpr value );
/*
The method ~CallDisplayFunction~ uses its first argument ~idPair~ ---
consisting of the two-elem-list $<$algebraId, typeId$>$ --- to find
the right display function in the map ~displayFunctions~. The arguments
are simply passed to this display function.

*/

   double GetNumeric(ListExpr value, bool &err);
/* 
Returns the numeric value of a ListExpr containing a IntAtom,
a RealAtom or a list representing a rational number 

*/

   inline static DisplayTTY& GetInstance() 
   { 
     if (!dtty)
       dtty = new DisplayTTY();
     return *dtty;
   }  

/*
Create an instance and initialize the dtty pointer if necessary.

*/   
   void Insert( const std::string& name, DisplayFunction* df );

   static void Set_NL(NestedList* NL) 
   { 
     nl = NL; 
     DisplayFunction::nl = NL;
   }

   static int maxIndent;
  

static void DisplayDescriptionLines( ListExpr value, int  maxNameLen);
/*
Displays a single type constructor or operator formatted, similar 
display function for relations.

*/

 protected:
    
           
 private:
  static DisplayTTY* dtty;

  DisplayTTY() {};
  ~DisplayTTY();
/*
The method ~Insert~ inserts the display function ~df~ given as
the second argument into the map ~displayFunctions~ at the index which is
determined by the type constructor ~name~ given as first argument.

*/
   void DisplayGeneric( ListExpr type, ListExpr value );
/*
Is a generic display function used as default for types to which not special
display function was assigned:

*/


  static NestedList*       nl; // Ref. to nested list container


  static std::string getStr(ListExpr e);

  typedef std::map<std::string,DisplayFunction*> DisplayMap; 
  DisplayMap displayFunctions;

};

#endif

