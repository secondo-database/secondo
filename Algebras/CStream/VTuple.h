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

//[_][\_]
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of datatype VTuple and operators.

[toc]

1 Overview

This header file essentially contains the definition of the VTuple class.

With this class type named VTuple is implemented into
SECONDO. One element of an already existing type Tuple and one element of type
TupleDesrc can be summerized to a new VTuple.

For detailed information refer to ~VTuple.cpp~.

2 Defines and includes


*/


#ifndef __VTUPLE_H__
#define __VTUPLE_H__

#include "TupleDescr.h"

class Tuple;

namespace cstream
{

/*
3 class ~VTuple~

A VTuple has the member [_]tuple of type Tuple and the member [_]tupledescr 
of type TupleDescr

*/

    class VTuple {
    public:
        
/*
3.1 Constructors and Destructor

*/
        VTuple();
        VTuple(Tuple* tuple, TupleDescr* tupledescr);
        VTuple(const VTuple& vt);
        ~VTuple();
        bool IsDefined();
        bool DeleteIfAllowed();
        void IncReference();
        int GetNumOfRefs() const;

/*
3.2 get functions

3.2.1 getTuple

*/
        Tuple* getTuple() const;
/*
3.2.2 getTupleDescr

*/
        TupleDescr* getTupleDescr() const;

/*
3.2.3 Secondo functions

*/
        static ListExpr Property();
        static Word In(const ListExpr typeInfo, const ListExpr instance,  
            const int errorPos, ListExpr& errorInfo,  bool& correct);
        static ListExpr Out(ListExpr typeInfo, Word value);
        static Word Create(const ListExpr typeInfo);
        static void Delete(const ListExpr typeInfo, Word& w);
        static void Close(const ListExpr typeInfo, Word& w);
        static bool Save(SmiRecord& valueRecord, size_t& offset, 
            const ListExpr typeInfo, Word& value);
        static bool Open(SmiRecord& valueRecord, size_t& offset, 
            const ListExpr typeInfo, Word& value);
        static Word Clone(const ListExpr typeInfo, const Word& w);
        static void* Cast(void* addr);
        static const std::string BasicType();
        static bool TypeCheck(ListExpr type, ListExpr& errorInfo);
        static int SizeOf();
        static const bool CheckType(const ListExpr list);

    private:

/*
3.3 Attributes

*/
        Tuple* _tuple;
        TupleDescr* _tupledescr;
        bool _defined;
        uint32_t refs;
    };
};

#endif
