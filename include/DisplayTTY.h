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

December 1998 Miguel Rodr[i]guez Luaces Ported for use in the client server version of "Secondo"[3]

May 2002 Ulrich Telle Port to new "Secondo"[3] version

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

#include "SecondoInterface.h"
#include "NestedList.h"

typedef void (*DisplayFunction)( ListExpr type,
                                 ListExpr numType,
                                 ListExpr value );
/*
Is the type definition for references to display functions.

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
  static void Initialize( SecondoInterface* secondoInterface );
/*
Initializes the display function mapping for the "Secondo"[3] interface
given by ~secondoInterface~.

*/
  static void DisplayResult( ListExpr type, ListExpr value );
/*
Displays a ~value~ of ~type~ using the defined display functions. Both
paramaters are given as nested lists.

*/

static void DisplayResult2( ListExpr value );
/*
Displays all type constructors and operators, currently registered, in formatted manner.
Displays all type constructors and operators for a single, included algebra in
formatted manner.

*/
 protected:
 private:
  DisplayTTY();
  ~DisplayTTY();
  static void InsertDisplayFunction( const string& name,
                                     DisplayFunction df );
/*
The method ~InsertDisplayFunction~ inserts the display function ~df~ given as
the second argument into the map ~displayFunctions~ at the index which is
determined by the type constructor ~name~ given as first argument.

*/
  static void CallDisplayFunction( const ListExpr idPair,
                                   ListExpr type,
                                   ListExpr numType,
                                   ListExpr value );
/*
The method ~CallDisplayFunction~ uses its first argument ~idPair~ ---
consisting of the two-elem-list $<$algebraId, typeId$>$ --- to find
the right display function in the map ~displayFunctions~. The arguments
are simply passed to this display function.

*/
  static void DisplayGeneric( ListExpr type,
                              ListExpr numType,
                              ListExpr value );
/*
Is a generic display function used as default for types to which not special
display function was assigned:

*/
  static void DisplayRelation( ListExpr type,
                               ListExpr numType,
                               ListExpr value );
/*
Is a display function for relations.

*/
// concatenates two lists
static ListExpr ConcatLists ( ListExpr l1, ListExpr l2 );
static int  MaxHeaderLength( ListExpr type );

static void DisplayDescriptionLines( ListExpr value, int  maxNameLen);
/*
Displays a single type constructor or operator formatted, similar display function for relations.

*/

  static int  MaxAttributLength( ListExpr type );
  static void DisplayTuple( ListExpr type,
                            ListExpr numType,
                            ListExpr value,
                            const int maxNameLen );
  static void DisplayTuples( ListExpr type,
                             ListExpr numType,
                             ListExpr value );
/*
Are display functions for tuples.

*/
  static void DisplayInt( ListExpr type,
                          ListExpr numType,
                          ListExpr value );

  static void DisplayReal( ListExpr type,
                           ListExpr numType,
                           ListExpr value );

  static void DisplayBoolean( ListExpr list,
                              ListExpr numType,
                              ListExpr value );

  static void DisplayString( ListExpr type,
                             ListExpr numType,
                             ListExpr value );

  static void DisplayText( ListExpr type,
                           ListExpr numType,
                           ListExpr value );

  static void DisplayXPoint( ListExpr type,
                             ListExpr numType,
                             ListExpr value );

  static void DisplayPoint( ListExpr type,
                             ListExpr numType,
                             ListExpr value );

  static void DisplayRect( ListExpr type,
                           ListExpr numType,
                           ListExpr value );

  static void DisplayRect3( ListExpr type,
                            ListExpr numType,
                            ListExpr value );

  static void DisplayRect4( ListExpr type,
                            ListExpr numType,
                            ListExpr value );
                            
  static void DisplayRect8( ListExpr type,
                            ListExpr numType,
                            ListExpr value );

  static void DisplayBinfile( ListExpr type,
                              ListExpr numType,
                              ListExpr value );
 
  static void DisplayArray( ListExpr type,
                            ListExpr numType,
                            ListExpr value );

  static void DisplayMP3( ListExpr type,
                          ListExpr numType,
                          ListExpr value );

  static void DisplayID3( ListExpr type,
                          ListExpr numType,
                          ListExpr value );

  static void DisplayLyrics( ListExpr type,
                             ListExpr numType,
                             ListExpr value );

  static void DisplayMidi( ListExpr type,
                           ListExpr numType,
                           ListExpr value );

  static void DisplayInstant( ListExpr type,
                              ListExpr numType,
                              ListExpr value );

  static void DisplayDuration( ListExpr type,
                               ListExpr numType,
                               ListExpr value );

  static void DisplayTid( ListExpr type,
                          ListExpr numType,
                          ListExpr value );

/* Returns the numeric value of a ListExpr containing a IntAtom, a RealAtom
   or a list representing a rational number */
   static double getNumeric(ListExpr value, bool &err);

/*
Are display functions for the types of the standard algebra.

*/
  static void DisplayFun( ListExpr type,
                          ListExpr numType,
                          ListExpr value );
/*
Is a display function for functions.

*/
  static void DisplayDate( ListExpr type,
                          ListExpr numType,
                          ListExpr value );
/*
Is a display function for date.

*/
 static void DisplayHtml( ListExpr type,
                          ListExpr numType,
                          ListExpr value );
/*
Is a display function for html.

*/

 static void DisplayPage( ListExpr type,
                          ListExpr numType,
                          ListExpr value );
/*
 Is a display function for page.

*/

  static void DisplayUrl( ListExpr type,
                          ListExpr numType,
                          ListExpr value );
/*
Is a display function for url.

*/


  static SecondoInterface* si; // Ref. to Secondo interface
  static NestedList*       nl; // Ref. to nested list container
  static map<string,DisplayFunction> displayFunctions;

  static int maxIndent; // used for multi line text values inside tuples

};

#endif

