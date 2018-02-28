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

[1] Implementation of datatype TupleDescr and operators.

[toc]

1 TupleDescr class implementation
see TupleDescr.cpp for details.

*/

#ifndef __TUPLEDESCR_H__
#define __TUPLEDESCR_H__

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "StandardTypes.h"
#include "Algebras/Record/Record.h"
#include "Algebras/Collection/CollectionAlgebra.h"
#include "VTree.h"

#include <string>

class Tuple;
class TupleType;

namespace cstream {
    class VTuple;

    typedef enum {TUPLE, RECORD, VECTOR} enumMode;

    class TNode {
    public:

        TNode(NodeType ntype) : _type(ntype) {};
        NodeType _type;
        std::list<TNode*> _children;
    };

    class TupleDescr : public Attribute {
    public:
        TupleDescr();
        TupleDescr(const std::string& s);
        TupleDescr(const ListExpr& list);
        TupleDescr(const TupleDescr& td);
        ~TupleDescr();

        /* Secondo methods */
        static void* Cast(void* addr);
        static int SizeOf();
        static bool TypeCheck(ListExpr type, ListExpr& errorInfo);
        static const std::string BasicType();
        static const bool CheckType(const ListExpr list);
        static ListExpr Property();
        static Word In( const ListExpr typeInfo, const ListExpr instance, 
                        const int errorPos, ListExpr&errorInfo, bool& correct);
        static ListExpr Out(ListExpr typeInfo, Word value);
        static ListExpr Out(TupleDescr* td);
        static Word Create(const ListExpr typeInfo);
        static void Delete(const ListExpr typeInfo, Word& w);
        static void Close(ListExpr typeInfo, Word& w);        
        static Word CloneTD(const ListExpr typeInfo, const Word& w);

        /* Attribute methods */
        virtual int NumOfFLOBs() const;
        virtual Flob* GetFLOB(const int i);
        int Compare(const Attribute* arg) const;
        bool Adjacent(const Attribute* arg) const;
        size_t Sizeof() const;
        size_t HashValue() const;
        void CopyFrom(const Attribute* arg);
        Attribute* Clone() const;

        /* Helper methods */
        char* Get() const;
        const std::string GetString() const;
        bool Import(const ListExpr& list); 
        bool Import(const char* s, std::string& error);

        bool BuildTree(std::string& error);
        TupleType* CreateTupleType();
        ListExpr GetTupleTypeExpr();
        int FindAttribute(const Node* parent, Node* attrnode);
        void TraverseMatchTree(Node* n1, Node* n2, bool& match);
        bool IsProjectable(TupleDescr* td);
                
        void GetTypeStringRec(Node* parent, Node* n, std::string& s);
        std::string GetTypeString(Node* n);

        Attribute* GetAttribute(enumMode mode, void* src, int idx, int vecidx);
        
        void SetNewAttribute(enumMode mode, void* dst, int idx, Attribute* a,
            const std::string& type, const std::string& name);
        
        void CopyAttribute( enumMode mode, void* dsrc, void* ddst,
            int srcidx, int dstidx,
            const std::string& type, const std::string& name, int vecidx);
        
        void ProjectTupleRec(Node* nsrc, Node* ndst, void* dsrc, void* ddst,
            enumMode mode, int vecidx, bool& match);
        Tuple* ProjectTuple(VTuple* vtsrc, bool& projected);

    private:
        
        Node* _root;
        int _max_depth;
        int _num_nodes;
        Flob _data;
    };

};


#endif
