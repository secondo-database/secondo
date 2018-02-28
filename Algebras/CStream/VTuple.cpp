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

[1] Implementation of datatype VTuple and operators.

[toc]

1 Overview

This cpp file essentially contains the implementation of the VTuple class.

With this class type named VTuple is implemented into
SECONDO. One element of an already existing type Tuple and one element of type
TupleDesrc can be summerized to a new VTuple.

2 Defines and includes

*/

#include "VTuple.h"
#include "GenericTC.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "VTHelpers.h"

namespace cstream {

/*
3 Implementation of the class VTuple

3.1 Constructors and Destructor

*/

VTuple::VTuple(Tuple* tuple, TupleDescr* tupledescr) :
_tuple(tuple), _tupledescr(tupledescr), _defined(true), refs(1) {

}

VTuple::VTuple(const VTuple& vt) : 
_tuple(vt._tuple), _tupledescr(vt._tupledescr), _defined(vt._defined), 
    refs(1)  {
}

VTuple::VTuple() : _defined(false), refs(1)  {

}

VTuple::~VTuple() {
    if (_tupledescr != NULL)
        _tupledescr->DeleteIfAllowed();
    if (_tuple != NULL)
        _tuple->DeleteIfAllowed();
}

/*
3.2 Implementation of the get functions

3.2.1 getTuple

*/

Tuple* VTuple::getTuple() const {
    return _tuple;
}

/*
3.2.2 getTupleDescr

*/

TupleDescr* VTuple::getTupleDescr() const {
    return _tupledescr;
}

/*
3.2.3 getTupleDescr

*/

bool VTuple::IsDefined() {
    return _defined;
}

bool VTuple::DeleteIfAllowed() {
    assert( refs > 0 );
    refs--;
    if( refs == 0 ){
    delete this;
    return true;
    } else {
        return false;
    }
}

void VTuple::IncReference() {
    _tuple->IncReference();
    refs++;
}


int VTuple::GetNumOfRefs() const {
    return refs;
}

/*
3.2.4 Secondo functions

*/

ListExpr VTuple::Property() {
    return gentc::GenProperty(
        "-> SIMPLE",
        VTuple::BasicType(),
        "(Tuple TupleDescr) = (t, td)",
        "");
}

Word VTuple::In(const ListExpr typeInfo, const ListExpr instance,  
        const int errorPos, ListExpr& errorInfo,  bool& correct) {

    LOG << " VTuple::In " << ENDL;

    Word res((void*)0);
    correct = false;

    if (listutils::isSymbolUndefined(instance)) {
        res.addr = new VTuple();
        correct = true;
        return res;
    }

    if(!nl->HasLength(instance, 2)) {
        cmsg.inFunError("expected two arguments");
        return res;
    }

    if(!Tuple::checkType(nl->First(instance))) {
        cmsg.inFunError("first argument is not a Tuple");
        return res;
    }

    // call of the in function of tuple. the first argument of the list has 
    // to be a tuple.
    Tuple* t = Tuple::In(nl->First(typeInfo), nl->First(instance), 
        errorPos, errorInfo, correct);

    if(!correct) {
        cmsg.inFunError("error accourt with the in function of Tuple");
        return res;
    }

    if(!TupleDescr::CheckType(nl->Second(instance))) {
        cmsg.inFunError("second argument is not a TupleDescr");
        return res;
    }

    correct = false;
    // call of the in function of tupledescr. the second argument of the
    // list has to be a tupledescr.
    Word tdw = TupleDescr::In(nl->Second(typeInfo), nl->Second(instance), 
        errorPos, errorInfo, correct);
    TupleDescr* td = static_cast<TupleDescr*>(tdw.addr);
    

    if(!correct) {
        cmsg.inFunError("error accourt with the in function of Tuple");
        return res;
    }

    // test if the tupledescr match to the tuple is missing

    res.addr = new VTuple(t, td);
    return res;
}

ListExpr VTuple::Out(ListExpr typeInfo, Word value) {

    LOG << " VTuple::Out " << ENDL;
    ListExpr l;

    VTuple* vt = (VTuple*)value.addr;

    if (!vt->IsDefined())
        return listutils::getUndefined();

    // Add the Tuple to the ListExpr
    l = (vt->getTuple())->Out(nl->Second(typeInfo));

    // Add the TupleDescr to the ListExpr
    l = nl->Append(l, TupleDescr::Out(vt->getTupleDescr()));

    return l;
}

Word VTuple::Create(const ListExpr typeInfo) {
    
    LOG << " VTuple::Create " << ENDL;

    Word w;
    w.addr = new VTuple(
        new Tuple(nl->TwoElemList(nl->TheEmptyList(), nl->TheEmptyList())), 
        new TupleDescr(""));
    return w;
}

void VTuple::Delete(const ListExpr typeInfo, Word& w) {

    LOG << " VTuple::Delete " << ENDL;

    VTuple *vt = (VTuple *)w.addr;
    vt->DeleteIfAllowed();
    w.addr = 0;
}

void VTuple::Close(const ListExpr typeInfo, Word& w) {

    LOG << " VTuple::Close " << ENDL;
    
    VTuple *vt = (VTuple *)w.addr;
    vt->DeleteIfAllowed();
    w.addr = 0;
}

bool VTuple::Save(SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo, Word& value) {

    LOG << " VTuple::Save " << ENDL;
    LOG << " not yet implemented " << ENDL;

    // cout << " VTuple::Save() " << endl;
    // VTuple* vt = (VTuple*)value.addr;
    // bool ok = true;

    // if (!vt->IsDefined()) {
    //     char def = 0;
    //     ok = valueRecord.Write(&def, sizeof(char), offset);
    //     offset += sizeof(char);
    //     return ok;
    // }
    // char def = 1;
    // ok = valueRecord.Write(&def, sizeof(char), offset);
    // offset += sizeof(char);

    // size_t length = td->_s.size();
    // ok = ok && valueRecord.Write(&length, sizeof(size_t), offset);
    // offset += sizeof(size_t);

    // if (length > 0) {
    //     ok = ok && valueRecord.Write(td->_s.c_str(), length, offset);
    //     offset += length;
    // }
        
    // return true;
    

    // Noch zu implementierten
    return true; // nur damit es keine Compilierungsfehler gibt.
}
    
bool VTuple::Open(SmiRecord& valueRecord, size_t& offset, 
        const ListExpr typeInfo, Word& value) {
    LOG << " VTuple::Open " << ENDL;
    LOG << " not yet implemented " << ENDL;
    // Noch zu implementierten
    return true; // nur damit es keine Compilierungsfehler gibt.
}

Word VTuple::Clone(const ListExpr typeInfo, const Word& w) {
    LOG << " VTuple::Clone " << ENDL;
    VTuple* vt = (VTuple*) w.addr;
    Word res;
    res.addr = new VTuple(vt->getTuple(), vt->getTupleDescr());
    return res;
}

void* VTuple::Cast(void* addr) {
    LOG << " VTuple::Cast " << ENDL;
    return (new (addr) VTuple);
}

const std::string VTuple::BasicType() { 
    return "vtuple";
}

bool VTuple::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, VTuple::BasicType());
}

int VTuple::SizeOf() {
    return sizeof(_tuple)+sizeof(_tupledescr);
}

const bool VTuple::CheckType(const ListExpr list) { 
    return listutils::isSymbol(list, BasicType()); 
} 

TypeConstructor VTupleTC(
    VTuple::BasicType(),
    VTuple::Property,
    VTuple::Out,
    VTuple::In,
    0,0,
    VTuple::Create,
    VTuple::Delete,
    VTuple::Open,
    VTuple::Save,
    VTuple::Close,
    VTuple::Clone,
    VTuple::Cast,
    VTuple::SizeOf,
    VTuple::TypeCheck
);

}
