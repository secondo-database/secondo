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
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

December 07, 2017

Author: Nicolas Napp

\tableofcontents

1 Header File: BinaryTuple.h

*/

#ifndef BINARYTUPLE_H_
#define BINARYTUPLE_H_

#include <cstdint>

/*
~BinaryTuples~ are needed to store the extracted ~relevant~ data from the argument tuple blocks, which are currently materialized in the ~ccPartHashJoin~ operator. The size of a ~binary tuple~ is determined dynamically at runtime. They come in three different sizes:

\begin{itemize}
\item binaryTupleSmall (4 Byte),
\item binaryTupleMedium (8 Byte), or 
\item binaryTupleLarge (16 Byte).
\end{itemize}

The goal is to use the smallest size possible and still be able to encode a reference to any tuple (i.e. the tuple block and row number) in the field ~tupelref~.

*/
namespace CRel2Algebra {

struct binaryTupleSmall    // size = 4 Bytes
{
  binaryTupleSmall() :
      tupleref { 0 }, hashvalue { 0 }
  {
  }
  uint16_t tupleref;       // exactly 16 Bit; needs <cstdint>
  uint16_t hashvalue;
};

struct binaryTupleMedium   // size = 8 Bytes
{
  binaryTupleMedium() :
      tupleref { 0 }, hashvalue { 0 }
  {
  }
  uint32_t tupleref;       // exactly 32 Bit; needs <cstdint>
  uint32_t hashvalue;
};

struct binaryTupleLarge    // size = 16 Bytes
{
  binaryTupleLarge() :
      tupleref { 0 }, hashvalue { 0 }
  {
  }
  uint64_t tupleref;       // exactly 64 Bit; needs <cstdint>
  uint64_t hashvalue;
};

} /* namespace CRel2Algebra */

#endif /* BINARYTUPLE_H_ */
