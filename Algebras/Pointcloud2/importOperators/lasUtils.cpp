/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



0 A bunch of utility-functions adapted from Pointcloud-Algebra.

*/
#include "lasUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Pointcloud/lasreader/lasreader.h"

extern NestedList *nl;
namespace pointcloud2 {
template<class T>
ListExpr getAttr(std::string name) {

    name[0] = toupper(name[0]);

    return nl->TwoElemList(nl->SymbolAtom(name), listutils::basicSymbol<T>());
}

ListExpr lasFormatAttrList(int pointType) {

    ListExpr pc2Format = nl->OneElemList(getAttr<CcInt>("intensity"));
    // we support point formats 0,...,5
    if (pointType < 0 || pointType > 5) {
        return pc2Format;
    }
    // append all members of point format 0
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("returnNumber"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("numberOfReturns"));
    pc2Format = nl->Append(pc2Format, getAttr<CcBool>("scanDirectionFlag"));
    pc2Format = nl->Append(pc2Format, getAttr<CcBool>("edgeOfFlightLine"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("classification"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("scan_angle_rank"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("user_data"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("point_source_id"));

    if (pointType == 0) {
        return pc2Format;
    }
    if (pointType == 2) {
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("red"));
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("green"));
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("blue"));
        return pc2Format;
    }

    // append attributes of pointtype 1
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("gps_time"));

    if (pointType == 1) {
        return pc2Format;
    }

    if (pointType == 4) {
        pc2Format = nl->Append(pc2Format,
                getAttr<CcInt>("wavePacketDescriptorIndex"));
        pc2Format = nl->Append(pc2Format,
                getAttr<CcInt>("byteOffsetToWaveformData"));
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("waveFormPacketSize"));
        pc2Format = nl->Append(pc2Format,
                getAttr<CcReal>("returnPointWaveformLocation"));
        pc2Format = nl->Append(pc2Format, getAttr<CcReal>("x_t"));
        pc2Format = nl->Append(pc2Format, getAttr<CcReal>("y_t"));
        pc2Format = nl->Append(pc2Format, getAttr<CcReal>("z_t"));
        return pc2Format;
    }

    //append member of point type 3
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("red"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("green"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("blue"));
    if (pointType == 3) {
        return pc2Format;
    }

    assert(pointType == 5);
    pc2Format = nl->Append(pc2Format,
            getAttr<CcInt>("wavePacketDescriptorIndex"));
    pc2Format = nl->Append(pc2Format,
            getAttr<CcInt>("byteOffsetToWaveformData"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("waveFormPacketSize"));
    pc2Format = nl->Append(pc2Format,
            getAttr<CcReal>("returnPointWaveformLocation"));
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("x_t"));
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("y_t"));
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("z_t"));

    return pc2Format;
}

void extendAttrList(ListExpr pc2Format, int pointType) {

    // we support point formats 0,...,5
    if (pointType < 0 || pointType > 5) {
        return;
    }
    // append all members of point format 0
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("returnNumber"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("numberOfReturns"));
    pc2Format = nl->Append(pc2Format, getAttr<CcBool>("scanDirectionFlag"));
    pc2Format = nl->Append(pc2Format, getAttr<CcBool>("edgeOfFlightLine"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("classification"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("scan_angle_rank"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("user_data"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("point_source_id"));

    if (pointType == 0) {
        return;
    }
    if (pointType == 2) {
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("red"));
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("green"));
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("blue"));
        return;
    }

    // append attributes of pointtype 1
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("gps_time"));

    if (pointType == 1) {
        return;
    }

    if (pointType == 4) {
        pc2Format = nl->Append(pc2Format,
                getAttr<CcInt>("wavePacketDescriptorIndex"));
        pc2Format = nl->Append(pc2Format,
                getAttr<CcInt>("byteOffsetToWaveformData"));
        pc2Format = nl->Append(pc2Format, getAttr<CcInt>("waveFormPacketSize"));
        pc2Format = nl->Append(pc2Format,
                getAttr<CcReal>("returnPointWaveformLocation"));
        pc2Format = nl->Append(pc2Format, getAttr<CcReal>("x_t"));
        pc2Format = nl->Append(pc2Format, getAttr<CcReal>("y_t"));
        pc2Format = nl->Append(pc2Format, getAttr<CcReal>("z_t"));
        return;
    }

    //append member of point type 3
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("red"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("green"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("blue"));
    if (pointType == 3) {
        return;
    }

    assert(pointType == 5);
    pc2Format = nl->Append(pc2Format,
            getAttr<CcInt>("wavePacketDescriptorIndex"));
    pc2Format = nl->Append(pc2Format,
            getAttr<CcInt>("byteOffsetToWaveformData"));
    pc2Format = nl->Append(pc2Format, getAttr<CcInt>("waveFormPacketSize"));
    pc2Format = nl->Append(pc2Format,
            getAttr<CcReal>("returnPointWaveformLocation"));
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("x_t"));
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("y_t"));
    pc2Format = nl->Append(pc2Format, getAttr<CcReal>("z_t"));
}

CcInt* getAttr(int i) {
    return new CcInt(true, i);
}
CcInt* getAttr(uint8_t i) {
    return new CcInt(true, i);
}
CcInt* getAttr(uint16_t i) {
    return new CcInt(true, i);
}
CcInt* getAttr(uint32_t i) {
    return new CcInt(true, i);
}
CcInt* getAttr(uint64_t i) {
    return new CcInt(true, i);
}
CcInt* getAttr(char c) {
    return new CcInt(true, c);
}
CcBool* getAttr(bool b) {
    return new CcBool(true, b);
}
CcReal* getAttr(double d) {
    return new CcReal(true, d);
}

template<typename T>
void extendTuple(Tuple* t, T v, size_t& offset) {
    t->PutAttribute(offset, getAttr(v));
    offset++;
}

void fillTuple0(Tuple* t, lasPoint0* p, size_t& offset) {
    extendTuple(t, p->returnNumber, offset);
    extendTuple(t, p->numberOfReturns, offset);
    extendTuple(t, p->scanDirectionFlag, offset);
    extendTuple(t, p->edgeOfFlightLine, offset);
    extendTuple(t, p->classification, offset);
    extendTuple(t, p->scan_angle_rank, offset);
    extendTuple(t, p->user_data, offset);
    extendTuple(t, p->point_source_id, offset);
}

void fillTuple2(Tuple* t, lasPoint2* p, size_t& offset) {
    fillTuple0(t, p, offset);
    extendTuple(t, p->red, offset);
    extendTuple(t, p->green, offset);
    extendTuple(t, p->blue, offset);
}

void fillTuple1(Tuple* t, lasPoint1* p, size_t& offset) {
    fillTuple0(t, p, offset);
    extendTuple(t, p->gps_time, offset);
}

void fillTuple4(Tuple* t, lasPoint4* p, size_t& offset) {
    fillTuple1(t, p, offset);
    extendTuple(t, p->wavePacketDescriptorIndex, offset);
    extendTuple(t, p->byteOffsetToWaveformData, offset);
    extendTuple(t, p->waveFormPacketSize, offset);
    extendTuple(t, p->returnPointWaveformLocation, offset);
    extendTuple(t, p->x_t, offset);
    extendTuple(t, p->y_t, offset);
    extendTuple(t, p->z_t, offset);
}

void fillTuple3(Tuple* t, lasPoint3* p, size_t& offset) {
    fillTuple1(t, p, offset);
    extendTuple(t, p->red, offset);
    extendTuple(t, p->green, offset);
    extendTuple(t, p->blue, offset);
}

void fillTuple5(Tuple* t, lasPoint5* p, size_t& offset) {
    fillTuple3(t, p, offset);
    extendTuple(t, p->wavePacketDescriptorIndex, offset);
    extendTuple(t, p->byteOffsetToWaveformData, offset);
    extendTuple(t, p->waveFormPacketSize, offset);
    extendTuple(t, p->returnPointWaveformLocation, offset);
    extendTuple(t, p->x_t, offset);
    extendTuple(t, p->y_t, offset);
    extendTuple(t, p->z_t, offset);
}

}
