/*
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
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Standard Attribute

May 1998 Stefan Dieker

December 1998 Friedhelm Becker

April 2002 Ulrich Telle Adjustments for the new Secondo version

1.1 Overview

The data types in the standard algebra are classes derived from the
class ~StandardAttribute~ which defines a pure virtual method ~GetValue~.
The method ~GetValue~ is important for the delivery of object values to
the "Secondo"[3] query processor.

*/

#ifndef STANDARDATTRIBUTE_H
#define STANDARDATTRIBUTE_H

#include "Attribute.h"

class StandardAttribute : public Attribute
{
 public:
  virtual size_t HashValue() = 0;

/*

1.2 Function ~CopyFrom~

Copies the contents of ~right~ into ~this~. Assumes that ~this~ and
~right~ are of the same type. This can be ensured by the type checking functions
in the algebras.

*/
  virtual void CopyFrom(StandardAttribute* right) = 0;
};

#endif

