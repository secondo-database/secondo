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

//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[11]	title:		[{\large \bf ] [}]
//paragraph	[12]	title:		[{\normalsize \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Secondo Parser

March 2002 Ulrich Telle

1.1 Overview

"Secondo"[3] offers a fixed set of commands for database management, catalog
inquiries, access to types and objects, queries, and transaction control.
Some of these commands require type expression, value expression, or
identifier arguments. Whether a type expression or value expression is
valid or not is determined by means of the specifications provided by the
active algebra modules, while validity of an identifier depends on the
contents of the actual database.

"Secondo"[3] acccepts two different forms of user input: Queries in nested list
syntax and queries following a syntax defined by the active algebra modules.
Both forms have positive and negative aspects. On the one hand, nested list
syntax remains the same, regardless of the actual set of operators provided
by active algebra modules. On the other hand, queries in nested list syntax
tend to contain a lot of parentheses, thereby getting hard to formulate and
read. This is the motivation for offering a second level of query syntax
with two important features:

  * Reading and writing type expressions is simplified.

  * For each operator of an algebra module, the algebra implementor can
specify syntax properties like infix or postfix notation. If this feature
is used carefully, value expressions can be much more readable.

User level syntax is provided to support the formulation of interactive
user queries in a more intuitive and less error-prone way. Compared to
nested list syntax, writing "Secondo"[3] commands in user level syntax
essentially has three effects:

  * There is no need to enclose commands by parentheses. The string list
type constructors, for instance, is valid input.

  * Formulation of type expressions is simplified and more straightforward.

  * "Secondo"[3] enables the algebra implementor to define syntactic properties
of value expressions using the algebra's operators.

Internally the system uses always the nested list representation. Therefore
a method for translating the user level syntax into nested list syntax is
necessary. The "Secondo"[3] parser class provides such a translation feature.

1.2 Interface methods

This module offers the following methods:

[22]	Creation/Removal & Parsing   \\
	[--------]
	SecParser        & Text2List \\
	[tilde]SecParser &           \\

1.3 Class "SecParser"[1]

The class ~SecParser~ implements a parser for translating Secondo commands
in text form into textual nested list representation.

*/

#ifndef SEC_PARSER_H
#define SEC_PARSER_H

class SecParser
{
 public:
  SecParser();
/*
Creates a Secondo command parser.

*NOTE*: The parser is not reentrant.

*/
  ~SecParser();
/*
Destroys a Secondo parser.

*/
  int Text2List( const string& inputString,
                 string& outputString );
/*
Parses the Secondo command in ~inputString~ and returns a nested list
representation of the command in ~outputString~. 
Returns an error code as follows:

  * 0 -- parsing was successful

  * 1 -- parsing was aborted due to errors

  * 2 -- parsing was aborted due to stack overflow

*/
};

#endif

