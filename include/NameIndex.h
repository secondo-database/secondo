/*
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]

1 Header File: Name Index

October 1995 Michael Endemann

November 1996 RHG Revision

March 2002 Ulrich Telle Port to C++ (using the ~map~ container of the standard C++ library)

1.1 Overview

A name index is conceptually a list of pairs of the form (~name~, ~Cardinal~).
The order of elements in the list is not specified. 

		Figure 1: Concept of a name index [NameIndex.eps]

The purpose is to allow efficient access to objects by name. The objects
themselves will be kept in a (compact) table. In contrast to lists and tables,
this one is not a generic structure; all components are known. The name index
is implemented as a specialization of the ~map~ template class of the C++
standard library.

Furthermore a scan over the elements of the name index is offered by ~map iterators~.

Dictionary-operations will be done in logarithmic time with this implementation.

1.2 Includes, Types

*/

#ifndef NAME_INDEX_H
#define NAME_INDEX_H

#include <string>
#include <map>

typedef unsigned long Cardinal;
typedef map<string,Cardinal> NameIndex;

#endif

