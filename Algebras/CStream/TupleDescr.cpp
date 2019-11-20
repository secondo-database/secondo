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

*/

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "ConstructorTemplates.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace collection;

namespace cstream {

/*
1.1 Constrcutors

*/
TupleDescr::TupleDescr() : _root(NULL) {
}


TupleDescr::TupleDescr(const std::string& s) :
Attribute(false), _root(NULL), _max_depth(0), 
_num_nodes(0), _data(0) {
    std::string error = "";
    if (!Import(s.c_str(), error)) {
        //std::string serr = "TupleDescr: falsches Format: " + s;
        throw SecondoException(error.c_str());
    }
}


TupleDescr::TupleDescr(const ListExpr& list) :
Attribute(false), _root(NULL), _max_depth(0), 
_num_nodes(0), _data(0) {
    if (!Import(list)) {
        std::string serr = "TupleDescr: falsches Format: ";
        throw SecondoException(serr.c_str());
    }
}

TupleDescr::TupleDescr(const TupleDescr& td) :
Attribute(td.IsDefined()),
_max_depth(0), _num_nodes(0),
_data(td._data.getSize()) {
    _data.copyFrom(td._data);
}

TupleDescr::~TupleDescr() {
    if (_root != NULL)
        VTree::Inst().Delete(_root);
    SetDefined(false);
}


/*
1.2 Secondo-Methods

*/
void* TupleDescr::Cast(void* addr) {
    return (new (addr) TupleDescr); 
}

int TupleDescr::SizeOf() {
    return sizeof(TupleDescr);
} 

bool TupleDescr::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, TupleDescr::BasicType());   
}
    
const std::string TupleDescr::BasicType() { 
    return "tupledescr"; 
}

const bool TupleDescr::CheckType(const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
}

ListExpr TupleDescr::Property() {
    return( nl->TwoElemList (
        nl->FourElemList (
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List")),
        nl->FourElemList (
            nl->StringAtom("-> DATA"),
            nl->StringAtom(TupleDescr::BasicType()),
            nl->StringAtom("(tupledescr) = (td)"),
            nl->StringAtom("(Plz int)")
    )));
}

Word TupleDescr::In(const ListExpr typeInfo, const ListExpr instance, 
                    const int errorPos, ListExpr&errorInfo, bool& correct) {
    ListExpr first;
    if (nl->ListLength(instance) == 1)
        first = nl->First(instance);
    else
        first = instance;
    
    if (nl->IsAtom(first) && nl->AtomType(first) == SymbolType
        && listutils::isSymbolUndefined(first)) {
        TupleDescr* td = new TupleDescr("");
        correct = true;
        return SetWord(td);
    }

    if (nl->IsAtom(first) && nl->AtomType(first) == TextType) {
        std::string s;
        nl->Text2String(first, s);
        TupleDescr* td = NULL;
        try {
            td = new TupleDescr(s.c_str());
        } catch (SecondoException& e) {
            errorInfo = listutils::typeError(e.msg());
            correct = false;
            return SetWord(Address(0));
        }
        correct = true;
        return SetWord(td);
    }

    correct = false;
    return SetWord(Address(0));
}

ListExpr TupleDescr::Out(ListExpr typeInfo, Word value) {
    TupleDescr* td = (TupleDescr*)(value.addr);
    if (!td->IsDefined())
        return listutils::getUndefined();
    std::string s = td->GetString();
    std::string error;
    td->BuildTree(error);
    return nl->TextAtom(s);
}

ListExpr TupleDescr::Out(TupleDescr* td) {
    if (!td->IsDefined())
        return listutils::getUndefined();
    return nl->TextAtom(td->GetString());
}

Word TupleDescr::Create(const ListExpr typeInfo) {
    Word w;
    w.addr = (new TupleDescr(""));
    return w;
}

void TupleDescr::Delete(const ListExpr typeInfo, Word& w) {
    TupleDescr* td = (TupleDescr*)w.addr;    
    if (td->_root != NULL)
        td->DeleteIfAllowed();
    w.addr = 0;
}

void TupleDescr::Close(ListExpr typeInfo, Word& w) {
    TupleDescr::Delete(typeInfo, w);
}


Word TupleDescr::CloneTD(const ListExpr typeInfo, const Word& w) {
    TupleDescr* td = (TupleDescr*)w.addr;
    Word res(td->Clone());    
    return res;
}

int TupleDescr::NumOfFLOBs() const {
    return 1;
}

Flob* TupleDescr::GetFLOB(const int i) {
    assert(i == 0);
    return &_data;
}

bool TupleDescr::Adjacent(const Attribute* arg) const {
    return false;
}

int TupleDescr::Compare(const Attribute* arg) const {
    if (!IsDefined())
        return arg->IsDefined() ? -1 : 0;
    if (!arg->IsDefined())
        return 1;
    TupleDescr* td = (TupleDescr*)arg;
    return this->GetString().compare(td->GetString());
}

size_t TupleDescr::Sizeof() const {
    return sizeof(*this);
}

size_t TupleDescr::HashValue() const {
    if (!IsDefined())
        return 0;
    boost::hash<std::string> string_hash;
    return string_hash(GetString());
}

void TupleDescr::CopyFrom(const Attribute* arg) {
    TupleDescr* td = (TupleDescr*)arg;
    if (!td->IsDefined()) {
        SetDefined(false);
        return;
    }
    std::string error;
    Import(td->GetString().c_str(), error);
}

Attribute* TupleDescr::Clone() const {
    TupleDescr* td = new TupleDescr(this->GetString());
    if (!this->IsDefined())
        td->SetDefined(false);
    return td;
}

/*
Caller is responsible for delete

*/
char* TupleDescr::Get() const {
    assert(IsDefined());
    SmiSize sz = _data.getSize();
    if (sz == 0) {
        char* s = new char[1];
        s[0] = 0;
        return s;
    }
    char* s = new char[sz];
    bool ok = _data.read(s, sz);
    assert(ok);
    return s;
}

const std::string TupleDescr::GetString() const {
    char* s = Get();
    std::string res(s);
    delete[] s;
    return res;
}

/*
1.3.1 Import

Builds the internal tree through ListExpr.

*/
bool TupleDescr::Import(const ListExpr& list) {
    std::string s;
    bool b = nl->WriteToString(s, list);
    if (!b)
        return false;
    std::string error;
    return Import(s.c_str(), error);
}

/*
1.3.2 Import

Builds the internal tree through string.

*/
bool TupleDescr::Import(const char* s, std::string& error) {
    _data.clean();
    if (s[0] == 0) {
        char d = 0;
        _data.write(&d, 1);
        return true;
    }

    SetDefined(true);
    SmiSize sz = strlen(s) + 1;
    assert(s[sz-1] == 0);
    bool b = _data.write(s, sz);
    assert(b);
    
    b = BuildTree(error);
    if (!b)
        return false;

    return b;
}

/*
1.3.4 BuildTree

Builds the internal tree.

*/
bool TupleDescr::BuildTree(std::string& error) {
    if (_root)
        return true;
    ListExpr list;
    nl->ReadFromString(this->GetString(), list);
    _root = VTree::Inst().Create(list, error, _max_depth, _num_nodes);
    if (!error.empty())
        return false;    
    //VTree::Inst().Print(_root, "|");
    return true;
}

/*
1.3.5 CreateTupleType

Creates a TupleType which is then needed for
constructing a tuple.

*/
TupleType* TupleDescr::CreateTupleType() {
    ListExpr attrlist = GetTupleTypeExpr();
    ListExpr resultTupleType = nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),attrlist);
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    ListExpr numResultTupleType = sc->NumericType(resultTupleType);
    return new TupleType(numResultTupleType);
}

/*
1.3.6 GetTupleTypeExpr

Converts the internal string in ListExpr.

*/
ListExpr TupleDescr::GetTupleTypeExpr() {
    ListExpr resulttype;
    nl->ReadFromString(GetString(), resulttype);
    return resulttype;
}

/*
1.3.7 FindAttribute

Search a tree-node for a given attribute.
Returns the index, -1 if not found.

*/
int TupleDescr::FindAttribute(const Node* parent, Node* attrnode) {
    for (size_t i=0; i<parent->_children.size(); ++i) {
        Node* n = parent->_children[i];
        if (n->_name == attrnode->_name && n->_type == attrnode->_type)
            return i;
        /*if (n->_type == "vector" && attrnode->_type == "vector")
            return i;*/
    }
    return -1;
}

/*
1.3.8 TraverseMatchTree

Recursive functions which checks if a tree (n2) is
projectable on another tree (n1).

*/
void TupleDescr::TraverseMatchTree(Node* n1, Node* n2, bool& match) {
    if (!match)
        return;

    for (size_t i=0; i<n1->_children.size(); ++i) {
        Node* n = n1->_children[i];

        int srcidx = this->FindAttribute(n2, n);
        if (srcidx == -1) {
            match = false;
            return;
        }
        Node* fn = n2->_children[srcidx];

        // collection attributes
        if (n->_type == "record" || n->_type == "vector") { 
            TraverseMatchTree(n, fn, match);
        } else { // simple Attribute
        }
    }
}

/*
1.3.9 IsProjectable

Checks if a given TupleDescr is projectable on
this TupleDescr.

*/
bool TupleDescr::IsProjectable(TupleDescr* td) {
    std::string error;
    if (!BuildTree(error))
        return false;
    if (!td->BuildTree(error))
        return false;
    
    // td projectable on this ? 
    // conditions:
    // 1. max_depth(td) <= max_depth(this)
    // 2. num_nodes(td) <= num_nodes(this)
    if (td->_max_depth > this->_max_depth)
        return false;
    if (td->_num_nodes > this->_num_nodes)
        return false;
    bool match = true;
    TraverseMatchTree(td->_root, this->_root, match);
    return match;
}

/*
1.3.10 GetTypeStringRec

Recursive function used by GetTypeString.
Constructs a type string for a given node.

*/
void TupleDescr::GetTypeStringRec(Node* parent, Node* n, std::string& s) {
    if (n->_type == "vector" || n->_type == "record") {
        if (n->_type == "record")
            s += "(";
        s += n->_type + " ";
        for (size_t i = 0; i<n->_children.size(); ++i) {
            GetTypeStringRec(n, n->_children[i], s);
        } 
        if (n->_type == "record")
            s += ")";
    } else {
        if (n->_name != "")
            s += "(" + n->_name + " " + n->_type + ")";
        else {
            if (parent != NULL && parent->_type == "vector")
                s += n->_type;
            else
                s += "(" + n->_type + ")";
        }
    }
}

/*
1.3.11 GetTypeString

Returns the type string for a given node.
Is used for constructing a Collection::vector.

*/
std::string TupleDescr::GetTypeString(Node* n) {
    std::string s;
    GetTypeStringRec(NULL, n, s);
    s = "(" + s + ")";
    return s;
}

/*
1.3.12 GetAttribute

Returns an Attribute at a given index depending on the
parent type (Tuple, Record, Collection).

*/
Attribute* TupleDescr::GetAttribute(enumMode mode, void* src,
    int idx, int vecidx) {
    switch (mode) {
        case TUPLE:
        {
            Tuple* tsrc = (Tuple*)src;
            return tsrc->GetAttribute(idx);
        }
        break;

        case RECORD:
        {
            Record* rsrc = (Record*)src;
            return rsrc->GetElement(idx);
        }
        break;

        case VECTOR:
        {
            Collection* csrc = (Collection*)src;
            return csrc->GetComponent(vecidx);
        }
        break;
    }
    return NULL;
}

/*
1.3.13 SetNewAttribute

Sets an Attribute at a given index depending on the
destination type (Tuple, Record, Collection).

*/
void TupleDescr::SetNewAttribute(enumMode mode, void* dst, int idx,
    Attribute* a, const std::string& type, const std::string& name) {
    switch (mode) {
        case TUPLE:
        {
            Tuple* tdst = (Tuple*)dst;
            tdst->PutAttribute(idx, a);
        }
        break;

        case RECORD:
        {
            Record* rdst = (Record*)dst;
            rdst->SetElement(idx, a, type, name);
        }
        break;

        case VECTOR:
        {
            Collection* cdst = (Collection*)dst;
            cdst->Insert(a, 1);
            cdst->SetDefined(true);
        }
        break;
    }
}

/*
1.3.14 CopyAttribute

Copies/inserts an Attribute at a given index depending
on the destination type (Tuple, Record, Collection).

*/
void TupleDescr::CopyAttribute(enumMode mode, void* dsrc, void* ddst,
    int srcidx, int dstidx,
    const std::string& type, const std::string& name, int vecidx) {
    switch (mode) {
        case TUPLE:
        {
            Tuple* tsrc = (Tuple*)dsrc;
            Tuple* tdst = (Tuple*)ddst;
            tdst->CopyAttribute(srcidx, tsrc, dstidx);
        }
        break;

        case RECORD:
        {
            Record* rsrc = (Record*)dsrc;
            Record* rdst = (Record*)ddst;
            Attribute* asrc = rsrc->GetElement(srcidx);
            rdst->SetElement(dstidx, asrc, type, name);
        }
        break;

        case VECTOR:
        {
            Collection* csrc = (Collection*)dsrc;
            Collection* cdst = (Collection*)ddst;
            Attribute* asrc = csrc->GetComponent(vecidx);
            cdst->Insert(asrc, 1);
        }
        break;
    }
}

/*
1.3.15 ProjectTupleRec

Recursive function used by ProjectTuple.
Projects dst to src.

*/
void TupleDescr::ProjectTupleRec(Node* nsrc, Node* ndst,
        void* dsrc, void* ddst,
        enumMode mode, int vecidx, bool& match) {
    if (!match)
        return;
    for (size_t i = 0; i < ndst->_children.size(); ++i) {
        Node* n = ndst->_children[i];
        int srcidx = FindAttribute(nsrc, n);
        if (srcidx == -1) {
            match = false;
            return;
        }
        Node* fn = nsrc->_children[srcidx];

        if (n->_type == "record") {
            Record* rsrc = (Record*)GetAttribute(mode, dsrc, srcidx, vecidx);
            Record* rdst = new Record(n->_children.size());
            ProjectTupleRec(fn, n, rsrc, rdst, RECORD, 0, match);
            if (!match) {
                rdst->DeleteIfAllowed();
                return;
            }
            SetNewAttribute(mode, ddst, i, rdst, n->_type, n->_name);
        } 
        else if (n->_type == "vector") {
            Collection* csrc = (Collection*)GetAttribute(
                mode, dsrc, srcidx, vecidx);
            std::string svtype;
            svtype = GetTypeString(n);
            ListExpr vtype;
            nl->ReadFromString(svtype, vtype);
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numvtype = sc->NumericType(vtype);
            Collection* cdst = new Collection(collection::vector, numvtype);
            
            for (int vidx = 0; vidx < csrc->GetNoComponents(); ++vidx) {
                ProjectTupleRec(fn, n, csrc, cdst, VECTOR, vidx, match);
                if (!match) {
                    cdst->DeleteIfAllowed();
                    return;
                }
            }
            cdst->Finish();
            SetNewAttribute(mode, ddst, i, cdst, n->_type, n->_name);
        }
        else { // Attribute
            CopyAttribute(mode, dsrc, ddst, srcidx, i, n->_type,
                n->_name, vecidx);
        }
    }
}

/*
1.3.16 ProjectTuple

Constructs a new (projected) Tuple based on the given
source VTuple and this TupleDescr.
Projects this to vtsrc.
3 possible results:
1) projected=false, result=NULL -> vtsrc not compatible
2) projected=false, result=vtsrc.tuple -> vtsrc 100% compatible
3) projected=true, result=projected tuple, vtsrc projectable

*/
Tuple* TupleDescr::ProjectTuple(VTuple* vtsrc, bool& projected) {
    Tuple* vtres = NULL;
    projected = false;
    TupleDescr* tdsrc = vtsrc->getTupleDescr();
    if (this->GetString() == tdsrc->GetString()) { // same TupleType
        //vtres = vtsrc->getTuple()->Clone();
        vtres = vtsrc->getTuple();
        vtres->IncReference();
    } else { // check projection here
        if ((tdsrc->_max_depth < this->_max_depth) ||
            (tdsrc->_num_nodes < this->_num_nodes))
            return NULL;
        TupleType* tt = CreateTupleType();
        vtres = new Tuple(tt);
        tt->DeleteIfAllowed();
        bool match = true;
        
        /*LOG << "SRC-Tree" << ENDL;
        VTree::Print(tdsrc->_root, "|");
        LOG << "DST-Tree" << ENDL;
        VTree::Print(this->_root, "|");
        LOG << ENDL;*/

        ProjectTupleRec(tdsrc->_root, this->_root,
            vtsrc->getTuple(), vtres, TUPLE,
            0, match);
        if (!match) {
            vtres->DeleteIfAllowed();
            return NULL;
        } else
            projected = true;
    }
    return vtres;
}

TypeConstructor TupleDescrTC(
    TupleDescr::BasicType(),
    TupleDescr::Property,
    TupleDescr::Out,
    TupleDescr::In,
    0,0,
    TupleDescr::Create,
    TupleDescr::Delete,
    OpenAttribute<TupleDescr>,
    SaveAttribute<TupleDescr>,
    TupleDescr::Close,
    TupleDescr::CloneTD,
    TupleDescr::Cast,
    TupleDescr::SizeOf,
    TupleDescr::TypeCheck
);



} /* namespace cstream */
