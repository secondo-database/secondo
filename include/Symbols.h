/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and Computer Science,
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

May 2007, M. Spiekermann. Initial version.

This file should contain all symbols in nested lists which are
types or reserved words.

Below we define some string constants which correspond to the symbols for type
constructors used in SECONDO algebra modules. These constants will
be used throughout the code below to avoid redundant use of string constants in
the code. This is important (i) to avoid strange runtime errors, e.g. in the
type mapping, which may be caused by a misspelled type name and (ii) to make
type renaming easier.

*/

#ifndef SEC_SYMBOLS_H
#define SEC_SYMBOLS_H

#include <string>
/*
The following class provides strings used as indentifiers for ~kinds~ in Secondo.
By Associating a ~type constructor~ with a certain kind, the implementor asserts
that the data type provides functionality required by the according kind.

Use, e.g. Kind::DATA() to get the according kind name for kind DATA, which is
the kind for all types that can be used as attributes.

*/
class Kind{
  public:
    static const std::string ARRAY()         { return "ARRAY"; }
    static const std::string BASE()          { return "BASE"; }
    static const std::string CSVEXPORTABLE() { return "CSVEXPORTABLE"; }
    static const std::string CSVIMPORTABLE() { return "CSVIMPORTABLE"; }
    static const std::string SQLEXPORTABLE() { return "SQLEXPORTABLE"; }
    static const std::string DATA()          { return "DATA"; }
    static const std::string DUALGRAPH()     { return "DUALGRAPH"; }
    static const std::string FILE()          { return "FILE"; }
    static const std::string HIERARCHICAL()  { return "HIERARCHICAL"; }
    static const std::string INDEXABLE()     { return "INDEXABLE"; }
    static const std::string INDOORGRAPH()   { return "INDOORGRAPH"; }
    static const std::string MREL()          { return "MREL"; }
    static const std::string MTUPLE()        { return "MTUPLE"; }
    static const std::string NETWORK()       { return "NETWORK"; }
    static const std::string JNETWORK()      { return "JNETWORK"; }
    static const std::string PTUPLE()        { return "PTUPLE"; }
    static const std::string RANGE()         { return "RANGE"; }
    static const std::string REL()           { return "REL"; }
    static const std::string SHPEXPORTABLE() { return "SHPEXPORTABLE"; }
    static const std::string SIMPLE()        { return "SIMPLE"; }
    static const std::string SPATIAL2D()     { return "SPATIAL2D"; }
    static const std::string SPATIAL3D()     { return "SPATIAL3D"; }
    static const std::string SPATIAL4D()     { return "SPATIAL4D"; }
    static const std::string SPATIAL8D()     { return "SPATIAL8D"; }
    static const std::string TEMPORAL()      { return "TEMPORAL"; }
    static const std::string TUPLE()         { return "TUPLE"; }
    static const std::string UNCERTAIN()     { return "UNCERTAIN"; }
    static const std::string UNIT()          { return "UNIT"; }
    static const std::string VISUALGRAPH()   { return "VISUALGRAPH"; }
    static const std::string DELIVERABLE()   { return "DELIVERABLE"; }
};

/*
Class ~Symbol~ defines functions providing general indentifiers used by the
QueryProcessor, in type mapping functions, counters or runtime flags.

*/
#undef ERROR
class Symbol{
  public:
    static const std::string APPEND()    { return "APPEND"; }
    static const std::string ERROR()     { return "ERROR"; }
    static const std::string ERRORS()    { return "ERRORS"; }
    static const std::string MAP()       { return "map"; }
    static const std::string STREAM()    { return "stream"; }
    static const std::string TYPEERROR() { return "typeerror"; }
    static const std::string UNDEFINED() { return "undefined"; }

    // COUNTER NAMES:
    static const std::string CTR_CreatedTuples() { return "RA:CreatedTuples"; }
    static const std::string CTR_DeletedTuples() { return "RA:DeletedTuples"; }
    static const std::string CTR_MaxmemTuples() { return "RA:MaxTuplesInMem"; }
    static const std::string CTR_MemTuples() { return "RA:TuplesInMem"; }

    static const std::string CTR_INT_Created() { return "STD:INT_created"; }
    static const std::string CTR_INT_Deleted() { return "STD:INT_deleted"; }
    static const std::string CTR_REAL_Created() { return "STD:REAL_created"; }
    static const std::string CTR_REAL_Deleted() { return "STD:REAL_deleted"; }
    static const std::string CTR_BOOL_Created() { return "STD:BOOL_created"; }
    static const std::string CTR_BOOL_Deleted() { return "STD:BOOL_deleted"; }
    static const std::string CTR_STR_Created() { return "STD:STRING_created"; }
    static const std::string CTR_STR_Deleted() { return "STD:STRING_deleted"; }

    static const std::string CTR_ATTR_BASIC_OPS() { return "RA:Attr:BasicOps"; }
    static const std::string CTR_ATTR_HASH_OPS() { return "RA:Attr:HashOps"; }
    static const std::string CTR_ATTR_COMPARE_OPS()
                                                { return "RA:Attr:CompareOps"; }
    static const std::string CTR_INT_COMPARE() { return "CcInt::Compare"; }
    static const std::string CTR_INT_EQUAL() { return "CcInt::Equal"; }
    static const std::string CTR_INT_LESS() { return "CcInt::Less"; }
    static const std::string CTR_INT_HASH() { return "CcInt::HashValue"; }
    static const std::string CTR_INT_ADJACENT() { return "CcInt::Adjacent"; }

    static const std::string CTR_TBUF_BYTES_W()
                                          { return "RA:TupleBuf:Write:Bytes"; }
    static const std::string CTR_TBUF_PAGES_W()
                                          { return "RA:TupleBuf:Write:Pages"; }
    static const std::string CTR_TBUF_BYTES_R()
                                          { return "RA:TupleBuf:Read:Bytes"; }
    static const std::string CTR_TBUF_PAGES_R()
                                          { return "RA:TupleBuf:Read:Pages"; }
};

#endif
