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
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: GArray

Version: 0.7

August 2002 RHG

1.1 Overview

This module offers a generic persistent array implemented on top of the
SecondoSMI interface.



1.2 Interface methods

This module offers the following methods:

[23]	Creation/Removal 	& Access   	& Inquiries	\\ 	
	[--------]
	GArray        		& Get 		& Size		\\  	
	[tilde]GArray		& Put		& Id		\\
	MarkDelete		&		& 		\\

Operations have to follow the protocol shown below:

		Figure 1: Protocol [Protocol.eps]

1.3 Class ~GArray~

An instance of the class is a handle to a persistent array of fixed size with
elements of type ~T~.

*/

#ifndef GARRAY_H
#define GARRAY_H

template<class T>
class GArray
{
 public:

  virtual ~GArray() {}
  virtual void MarkDelete() = 0;
  virtual void Put(int const index, const T& elem) = 0;
  virtual void Get(int const index, T& elem) = 0;
  virtual void Clear() = 0;
  virtual void Sort( bool (*cmp)(const T&, const T&) ) = 0;
  virtual const int Size() const = 0;
  virtual const SmiRecordId Id() const = 0;
};

#endif // GARRAY_H

