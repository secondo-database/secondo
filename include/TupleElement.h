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
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Tuple Element

May 1998 Stefan Dieker

April 2002 Ulrich Telle Adjustments for the new Secondo version

1.1 Overview

The ~Tuple Manager~ is an important support component for the relational
algebra. Relations consist of tuples, tuples consist of tuple elements.
Classes implementing attribute data types have to be subtypes of class
attribute. Whatever the shape of such derived attribute classes might be,
their instances can be aggregated and made persistent via instances of class
~Tuple~, while the user is (almost) not aware of the additional management
actions arising from persistency.

1.1 Types

*/

#ifndef TUPLE_ELEMENT_H
#define TUPLE_ELEMENT_H

#ifndef TYPE_ADDRESS_DEFINED
#define TYPE_ADDRESS_DEFINED
typedef void* Address;
#endif

#include "FLOB.h"
/*
#ifndef TYPE_FLOB_DEFINED
#define TYPE_FLOB_DEFINED
typedef void FLOB;
#endif
*/

/*
Are type definitions for a generic address pointer and for a ~fake large object~.

*/

/*
1.1 Class "TupleElement"[1]

This class defines several virtual methods which are essential for the
~Tuple Manager~.

*/
class TupleElement // renamed, previous name: TupleElem
{
 public:
  TupleElement(){};
  virtual ~TupleElement() {};
  virtual int      NumOfFLOBs() { return (0); };
  virtual FLOB*    GetFLOB( int ){ return (0); };
  virtual ostream& Print( ostream& os ) { return (os << "??"); };
};

ostream& operator<< (ostream &os, TupleElement &attrib);

#endif

