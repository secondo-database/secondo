/*
\newpage

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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Implementation file "GeneralTreeAlgebra.cpp"[4]

January-May 2008, Mirko Dibbert

This file implements the GeneralTreeAlgebra.

1.1 Includes and defines.

*/
#include "Algebra.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Base64.h"
#include "Symbols.h"

#include "GeneralTreeAlgebra.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace gta {

/********************************************************************
1.1. Type constructors

1.1.1 Type constructor "hpoint"[4]

********************************************************************/
static ListExpr HPointProp()
{
  return (
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> DATA"),
                nl->StringAtom(HPoint::BasicType()),
                nl->StringAtom("(<dim> (<c_1> ... <c_n>) "),
                nl->StringAtom("(2 (0.5 2.5))"))));
}

ListExpr OutHPoint(ListExpr type_Info, Word value)
{
    HPoint* point = static_cast<HPointAttr*>(value.addr)->hpoint();
    NList coord_list;

    for (unsigned i = 0; i < point->dim(); ++i)
        coord_list.append(NList(point->coord(i)));

    NList dim_list(static_cast<int>(point->dim()));
    NList result(dim_list, coord_list);
    delete point;
    return result.listExpr();
}

Word InHPoint(
        ListExpr type_Info, ListExpr value,
        int errorPos, ListExpr &_error_Info, bool &corect)
{
    NList valueList(value);

    if (valueList.length() != 2)
    {
        corect = false;
        ErrorReporter::ReportError(
          "Excepted a two elemental list, but got" +
            valueList.convertToString());
        return SetWord(Address(0));
    }

    NList dim_list = valueList.first();
    NList coords_list = valueList.second();

    unsigned dim;
    if (!dim_list.isInt())
    {
        corect = false;
        ErrorReporter::ReportError(
        "Excepted an int atom as first list element, but got " +
            dim_list.convertToString());
        return SetWord(Address(0));
    }
    else
        dim = dim_list.intval();

    if (coords_list.length() != dim)
    {
        corect = false;
        ostringstream error;
        error <<  "Excepted a list with " << dim
              << " real values as second list element, but got "
              << coords_list.convertToString();
        ErrorReporter::ReportError(error.str());
        return SetWord(Address(0));
    }

    GTA_SPATIAL_DOM coords[dim];
    int pos = 0;
    while (!coords_list.isEmpty())
    {
        NList coord = coords_list.first();
        coords_list.rest();

        if (!coord.isReal())
        {
            corect = false;
            ErrorReporter::ReportError(
            "Excepted an real atom as list element, but got " +
                coord.convertToString());
            return SetWord(Address(0));
        }
        else
            coords[pos] = nl->RealValue(coord.listExpr());

        pos++;
    }

    corect = true;
    return SetWord(new HPointAttr(dim, coords));
}

Word createHPoint(const ListExpr type_Info)
{ return SetWord(new HPointAttr(0)); }

void DeleteHPoint(const ListExpr type_Info, Word &w)
{
    HPointAttr* attr = static_cast<HPointAttr*>(w.addr);
    attr->deleteFLOB();
    delete attr;
    w.addr = 0;
}

bool OpenHPoint(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word &value)
{
    value = SetWord(Attribute::Open(valueRecord, offset, type_Info));
    return true;
}

bool SaveHPoint(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word &value)
{
    Attribute::Save(
            valueRecord, offset, type_Info,
            static_cast<Attribute*>(value.addr));
    return true;
}

void CloseHPoint(const ListExpr type_Info, Word &w)
{ delete(HPointAttr*)w.addr; }

Word CloneHPoint(const ListExpr type_Info, const Word &w)
{
    HPointAttr *src = static_cast<HPointAttr*>(w.addr);
    HPointAttr *cpy = new HPointAttr(*src);
    return SetWord(cpy);
}

void * CastHPoint(void *addr)
{ return new(addr) HPointAttr; }

int SizeOfHPoint()
{ return sizeof(HPointAttr); }

bool CheckHPoint(ListExpr typeName, ListExpr &error_Info)
{ return (nl->IsEqual(typeName, HPoint::BasicType())); }

TypeConstructor hpoint_tc(
        HPoint::BasicType(),     HPointProp,
        OutHPoint,    InHPoint,
        0, 0,
        createHPoint, DeleteHPoint,
        OpenHPoint,   SaveHPoint,
        CloseHPoint,  CloneHPoint,
        CastHPoint,   SizeOfHPoint,
        CheckHPoint);



/********************************************************************
1.1.1 Type constructor "hrect"[4]

********************************************************************/
static ListExpr HRectProp()
{
  return (
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> DATA"),
                nl->StringAtom(HRect::BasicType()),
                nl->StringAtom("(<dim> (<lb_1> ... <lb_n>) "
                                      "(<ub_1> ... <ub_n>))"),
                nl->StringAtom("(2 (0.0 0.0) (1.0 1.0))"))));
}

ListExpr OutHRect(ListExpr type_Info, Word value)
{
    HRect *rect = static_cast<HRectAttr*>(value.addr)->hrect();
    NList lb_list, ub_list;

    for (unsigned i = 0; i < rect->dim(); ++i)
    {
        lb_list.append(NList(rect->lb(i)));
        ub_list.append(NList(rect->ub(i)));
    }

    NList dim_list(static_cast<int>(rect->dim()));
    NList result(dim_list, lb_list, ub_list);
    delete rect;
    return result.listExpr();
}

Word InHRect(
        ListExpr type_Info, ListExpr value,
        int errorPos, ListExpr &_error_Info, bool &corect)
{
    NList valueList(value);

    if (valueList.length() != 3)
    {
        corect = false;
        ErrorReporter::ReportError(
          "Excepted a three elemental list, but got" +
            valueList.convertToString());
        return SetWord(Address(0));
    }

    NList dim_list = valueList.first();
    NList lb_list = valueList.second();
    NList ub_list = valueList.third();

    unsigned dim;
    if (!dim_list.isInt())
    {
        corect = false;
        ErrorReporter::ReportError(
        "Excepted an int atom as first list element, but got " +
            dim_list.convertToString());
        return SetWord(Address(0));
    }
    else
        dim = dim_list.intval();

    if (lb_list.length() != dim)
    {
        corect = false;
        ostringstream error;
        error <<  "Excepted a list with " << dim
              << " real values as second list element, but got "
              << lb_list.convertToString();
        ErrorReporter::ReportError(error.str());
        return SetWord(Address(0));
    }

    if (ub_list.length() != dim)
    {
        corect = false;
        ostringstream error;
        error <<  "Excepted a list with " << dim
              << " real values as third list element, but got "
              << ub_list.convertToString();
        ErrorReporter::ReportError(error.str());
        return SetWord(Address(0));
    }

    GTA_SPATIAL_DOM lbVector[dim], ubVector[dim];
    int pos = 0;
    while (!lb_list.isEmpty())
    {
        NList lb = lb_list.first();
        NList ub = ub_list.first();
        lb_list.rest();
        ub_list.rest();

        if (!lb.isReal())
        {
            corect = false;
            ErrorReporter::ReportError(
            "Excepted an real atom as list element, but got " +
                lb.convertToString());
            return SetWord(Address(0));
        }
        else
            lbVector[pos] = nl->RealValue(lb.listExpr());

        if (!ub.isReal())
        {
            corect = false;
            ErrorReporter::ReportError(
            "Excepted an real atom as list element, but got " +
                ub.convertToString());
            return SetWord(Address(0));
        }
        else
            ubVector[pos] = nl->RealValue(ub.listExpr());

        pos++;
    }

    corect = true;
    return SetWord(new HRectAttr(dim, lbVector, ubVector));
}

Word createHRect(const ListExpr type_Info)
{ return SetWord(new HRectAttr(0)); }

void DeleteHRect(const ListExpr type_Info, Word &w)
{
    HRectAttr *attr =
        static_cast<HRectAttr*>(w.addr);
    attr->deleteFLOB();
    delete attr;
    w.addr = 0;
}

bool OpenHRect(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word &value)
{
    value = SetWord(Attribute::Open(valueRecord, offset, type_Info));
    return true;
}

bool SaveHRect(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word &value)
{
    Attribute::Save(
            valueRecord, offset, type_Info,
            static_cast<Attribute*>(value.addr));
    return true;
}

void CloseHRect(const ListExpr type_Info, Word &w)
{ delete(HRectAttr*)w.addr; }

Word CloneHRect(const ListExpr type_Info, const Word &w)
{
    HRectAttr *src = static_cast<HRectAttr*>(w.addr);
    HRectAttr *cpy = new HRectAttr(*src);
    return SetWord(cpy);
}

void * CastHRect(void *addr)
{ return new(addr) HRectAttr; }

int SizeOfHRect()
{ return sizeof(HRectAttr); }

bool CheckHRect(ListExpr typeName, ListExpr &error_Info)
{ return (nl->IsEqual(typeName, HRect::BasicType())); }

TypeConstructor hrect_tc(
        HRect::BasicType(),     HRectProp,
        OutHRect,    InHRect,
        0, 0,
        createHRect, DeleteHRect,
        OpenHRect,   SaveHRect,
        CloseHRect,  CloneHRect,
        CastHRect,   SizeOfHRect,
        CheckHRect);



/********************************************************************
1.1 Type constructor "distdata"[4]

********************************************************************/
static ListExpr DistDataProp()
{
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(DistDataAttribute::BasicType()),
                             nl->StringAtom("no list, use operator"
                                            " getdistdata"),
                             nl->StringAtom("---"))));



}

ListExpr OutDistData(ListExpr type_Info, Word value)
{
    DistDataAttribute* ddAttr =
            static_cast<DistDataAttribute*>(value.addr);
    Base64 b64;
    string b64string;
    char * data = ddAttr->getData();
    b64.encode(data, ddAttr->size(), b64string);
    delete[] data;
    DistDataInfo info = DistDataReg::
            getInfo(ddAttr->distdataId());

    string name = info.name();
    string type = info.typeName();

    // remove lf from b64string
    b64string = b64string.substr(0, b64string.size() - 1);

    return nl->ThreeElemList(
               nl->StringAtom(type),
               nl->StringAtom(name),
               nl->TextAtom(b64string));
}

Word InDistData(
        ListExpr type_Info, ListExpr value,
        int errorPos, ListExpr &error_Info, bool &correct)
{
    NList valueList(value);

    if (valueList.length() != 3)
    {
        correct = false;
        ErrorReporter::ReportError(
          "Excepted a three elemental list, but got" +
            valueList.convertToString());
        return SetWord(Address(0));
    }

    NList nameNL = valueList.second();
    NList typeNL = valueList.first();
    NList dataNL = valueList.third();

    // get distdata name
    string name;
    if (!nameNL.isString())
    {
        correct = false;
        ErrorReporter::ReportError(
          "Excepted a string atom as second list element, but got " +
            nameNL.convertToString());
        return SetWord(Address(0));
    }
    else
        name = nameNL.str();

    // get type constructor name
    string type;
    if (!typeNL.isString())
    {
        correct = false;
        ErrorReporter::ReportError(
          "Excepted a string atom as first list element, but got " +
            typeNL.convertToString());
        return SetWord(Address(0));
    }
    else
        type = typeNL.str();

    // get data string
    string data;
    if (!dataNL.isText())
    {
        correct = false;
        ErrorReporter::ReportError(
          "Excepted a text atom as third list element, but got " +
            dataNL.convertToString());
        return SetWord(Address(0));
    }
    else
        data = dataNL.str();

    // deconde data string
    DistDataAttribute* ddAttr = new DistDataAttribute(0);
    Base64 b64;
    char bindata[b64.sizeDecoded(data.size())];
    int size = b64.decode(data, bindata);

    DistDataId id = DistDataReg::getId(type, name);
    ddAttr->set(true, bindata, size, id);

    correct = true;

    return SetWord(ddAttr);
}

Word createDistData(const ListExpr type_Info)
{ return SetWord(new DistDataAttribute(0)); }

void DeleteDistData(const ListExpr type_Info, Word& w)
{
    DistDataAttribute* ddAttr =
        static_cast<DistDataAttribute*>(w.addr);
    ddAttr->deleteFLOB();
    delete ddAttr;
    w.addr = 0;
}

bool OpenDistData(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word& value)
{
    value = SetWord(Attribute::Open(valueRecord, offset, type_Info));
    return true;
}

bool SaveDistData(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word& value)
{
    Attribute::Save(valueRecord, offset, type_Info,
                    static_cast<Attribute*>(value.addr));
    return true;
}

void CloseDistData(const ListExpr type_Info, Word& w)
{ delete(DistDataAttribute*)w.addr; }

Word CloneDistData(const ListExpr type_Info, const Word& w)
{
    DistDataAttribute* src = static_cast<DistDataAttribute*>(w.addr);
    DistDataAttribute* cpy = new DistDataAttribute(*src);
    return SetWord(cpy);
}

void* CastDistData(void *addr)
{ return new(addr) DistDataAttribute; }

int SizeOfDistData()
{ return sizeof(DistDataAttribute); }

bool CheckDistData(ListExpr typeName, ListExpr &error_Info)
{ return (nl->IsEqual(typeName, DistDataAttribute::BasicType())); }

TypeConstructor distdata_tc(
        DistDataAttribute::BasicType(),    DistDataProp,
        OutDistData,   InDistData,
        0, 0,
        createDistData, DeleteDistData,
        OpenDistData,   SaveDistData,
        CloseDistData,  CloneDistData,
        CastDistData,   SizeOfDistData,
        CheckDistData);



/********************************************************************
1.1. Operators

1.1.1 Value mappings

********************************************************************/
template <unsigned paramCnt> int
gethpoint_VM(
        Word* args, Word& result,
        int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    HPointAttr* hpointAttr =
            static_cast<HPointAttr*>(result.addr);

    Attribute* attr = static_cast<Attribute*>(args[0].addr);

    string typeName = static_cast<CcString*>(
            args[paramCnt].addr)->GetValue();

    string hpointName = static_cast<CcString*>(
            args[paramCnt+1].addr)->GetValue();

    HPointInfo info = HPointReg::getInfo(typeName, hpointName);
    HPoint *p = info.getHPoint(attr);
    hpointAttr->set(true, p);
    delete p;
    return 0;
} // gethpoint_VM



template <unsigned paramCnt> int
getbbox_VM(
        Word* args, Word& result,
        int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    HRectAttr* hrectAttr =
            static_cast<HRectAttr*>(result.addr);

    Attribute* attr = static_cast<Attribute*>(args[0].addr);

    string typeName = static_cast<CcString*>(
            args[paramCnt].addr)->GetValue();

    string hrectName = static_cast<CcString*>(
            args[paramCnt+1].addr)->GetValue();

    BBoxInfo info = BBoxReg::getInfo(typeName, hrectName);
    HRect *r = info.getBBox(attr);
    hrectAttr->set(true, r);
    delete r;
    return 0;
} // getbbox_VM



template <unsigned paramCnt> int
getdistdata_VM(
        Word* args, Word& result,
        int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    DistDataAttribute* ddAttr =
            static_cast<DistDataAttribute*>(result.addr);

    Attribute* attr = static_cast<Attribute*>(args[0].addr);

    string typeName = static_cast<CcString*>(
            args[paramCnt].addr)->GetValue();

    string dataName = static_cast<CcString*>(
            args[paramCnt+1].addr)->GetValue();

    DistDataId id = DistDataReg::getId(typeName, dataName);
    DistDataInfo info = DistDataReg::getInfo(id);
    DistData *ddata = info.getData(attr);
    ddAttr->set(true, ddata, id);
    delete ddata;

    return 0;
} // getdistdata_VM



template <unsigned paramCnt> int
gdistance_VM(Word* args, Word& result,
             int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    CcReal* resultValue = static_cast<CcReal*>(result.addr);

    Attribute* attr1 =
        static_cast<Attribute*>(args[0].addr);

    Attribute* attr2 =
        static_cast<Attribute*>(args[1].addr);

    string typeName =
        static_cast<CcString*>(args[paramCnt].addr)->GetValue();

    string distfunName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string dataName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    double dist;
    DistfunInfo df_info = DistfunReg::
            getInfo(distfunName, typeName, dataName);
    DistData *data1 = df_info.getData(attr1);
    DistData *data2 = df_info.getData(attr2);
    df_info.dist(data1, data2, dist);
    delete data1;
    delete data2;
    resultValue->Set(true, dist);

    return 0;
} // gdistance_VM



template <unsigned paramCnt> int
gdistanceDD_VM(Word* args, Word& result,
               int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    CcReal* resultValue = static_cast<CcReal*>(result.addr);

    DistDataAttribute* ddAttr1 =
        static_cast<DistDataAttribute*>(args[0].addr);
    DistDataAttribute* ddAttr2 =
        static_cast<DistDataAttribute*>(args[1].addr);
    string distfunName =
        static_cast<CcString*>(args[paramCnt].addr)->GetValue();

    // check, if both attributes are assigned to the same
    // attribute- and distdata-type
    if (ddAttr1->distdataId() != ddAttr2->distdataId())
    {
        DistDataId id1 = ddAttr1->distdataId();
        DistDataId id2 = ddAttr2->distdataId();
        DistDataInfo info1 = DistDataReg::getInfo(id1);
        DistDataInfo info2 = DistDataReg::getInfo(id2);
        const string seperator = "\n" + string(70, '-') + "\n";

        if ((id1.algebraId() != id2.algebraId()) ||
            (id1.typeId() != id2.typeId()))
        { // type constructors not equal
            cmsg.error() << seperator
            << "Operator gdistance got distdata attributes for \n"
            << "different attribute types: \""
            << info1.typeName() << "\", \"" << info2.typeName()
            << "\"!" << seperator << endl;
            cmsg.send();
        }
        else
        { // distance functions not equal
            cmsg.error() << seperator
            << "Operator gdistance got distdata attributes for \n"
            << "different data types \""
            << info1.name() << "\", \"" << info2.name()
            << "\"!" << seperator << endl;
            cmsg.send();
        }
        resultValue->Set(false, 0);
        return 0;
    }

    // get distance function
    DistfunInfo info = DistfunReg::getInfo(
            distfunName, ddAttr1->distdataId());

    // check, if selected distance function is defined
    if(!info.isDefined())
    {
        DistDataInfo info = DistDataReg::
                getInfo(ddAttr1->distdataId());
        string typeName = info.typeName();
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error()
        << seperator
        << "Operator gdistance: \n"
        << "Distance function \"" << distfunName << "\" "
        << "for type \"" << typeName << "\" is not defined!" << endl
        << "Defined distance functions: " << endl << endl
        << DistfunReg::definedNames(typeName)
        << seperator << endl;
        cmsg.send();

        resultValue->Set(false, 0);
        return 0;
    }

    double dist;
    char * data = ddAttr1->getData();
    DistData dd1(ddAttr1->size(), data);
    delete [] data;
    data = ddAttr2->getData();
    DistData dd2(ddAttr2->size(),data);
    delete[] data;
    info.dist(&dd1, &dd2, dist);
    resultValue->Set(true, dist);

    return 0;
} // gdistanceDD_VM


/********************************************************************
1.1.1 Type mappings

********************************************************************/
template<unsigned paramCnt>
ListExpr gethpoint_TM(ListExpr args)
{
    if (!HPointReg::isInitialized())
        HPointReg::initialize();

    NList args_NL(args);
    if(!args_NL.hasLength(paramCnt)){
      stringstream err;
      err << "Expected " << paramCnt << " arguments!";
      return listutils::typeError(err.str());
    }

    NList data_NL = args_NL.first();

//     CHECK_SYMBOL(data_NL, 1)
    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 1 must be a symbol or an atomar type!");
    }

    string typeName = data_NL.str();

    // select bbox type
    string funName;
    if (paramCnt == 2){
      if(!args_NL.second().isSymbol()){
        stringstream err;
        err << "Argument 2 must be the name of an existing "
            << "gethpoint function or \"" + HPOINT_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      funName = args_NL.second().str();
    }
    else
        funName = HPOINT_DEFAULT;

    // check, if selected get-hpoint function is defined
    if (funName == HPOINT_DEFAULT){
      funName = HPointReg::defaultName(typeName);
      if(funName == HPOINT_UNDEFINED){
        string errmsg;
        errmsg = "No default gethpoint function defined for type "
        "constructor \"" + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else if(!HPointReg::isDefined(typeName, funName)){
      string errmsg;
      errmsg = "gethpoint function  \"" + funName +
      "\" for type constructor \"" + typeName +
      "\" is not defined! Use \"" + HPOINT_DEFAULT +
      "\" or one of the following names: \n\n" +
      HPointReg::definedNames(typeName);
      return listutils::typeError(errmsg);
    }

    NList res1(Symbol::APPEND());
    NList res2(NList(typeName, true), NList(funName, true));
    NList res3(HPoint::BasicType());
    NList result(res1, res2, res3);
    return result.listExpr();
}



template<unsigned paramCnt>
ListExpr getbbox_TM(ListExpr args)
{
    if (!BBoxReg::isInitialized()){
      BBoxReg::initialize();
    }

    NList args_NL(args);
    if(!args_NL.hasLength(paramCnt)){
      stringstream err;
      err << "Expected " << paramCnt << " arguments!";
      return listutils::typeError(err.str());
    }

    NList data_NL = args_NL.first();

    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 1 must be a symbol or an atomar type!");
    }

    string typeName = data_NL.str();

    // select bbox type
    string funName;
    if (paramCnt == 2){
      if(!args_NL.second().isSymbol()){
        stringstream err;
        err << "Argument 2 must be the name of an existing "
            << "gethrect function or \"" + BBOX_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      funName = args_NL.second().str();
    } else {
      funName = BBOX_DEFAULT;
    }

    // check, if selected get-hrect function is defined
    if (funName == BBOX_DEFAULT){
      funName = BBoxReg::defaultName(typeName);
      if(funName == BBOX_UNDEFINED){
        string errmsg = "No default gethrect function defined for type "
                        "constructor \"" + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else if(!BBoxReg::isDefined(typeName, funName)){
        string errmsg = "gethrect function  \"" + funName +
                      "\" for type constructor \"" + typeName +
                      "\" is not defined! Use \"" + BBOX_DEFAULT +
                      "\" or one of the following names: \n\n" +
                      BBoxReg::definedNames(typeName);
        return listutils::typeError(errmsg);
    }

    NList res1(Symbol::APPEND());
    NList res2(NList(typeName, true), NList(funName, true));
    NList res3(HRect::BasicType());
    NList result(res1, res2, res3);
    return result.listExpr();
}



template<unsigned paramCnt>
ListExpr getdistdata_TM(ListExpr args)
{
    #ifdef PRINT_DISTFUN_INFOS
    if (!distfuninfos_shown)
    {
        distfuninfos_shown = true;
        string distfun_str = DistfunReg::printDistfuns();
        cmsg.info() << distfun_str;
        cmsg.info() << "(this info is only shown once on the first "
                    << "call of the getdistdata or gdistance "
                    << "operator)" << endl;
        cmsg.send();
    }
    #endif

    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);
    if(!args_NL.hasLength(paramCnt)){
      stringstream err;
      err << "Expected " << paramCnt << " arguments!";
      return listutils::typeError(err.str());
    }

    NList data_NL = args_NL.first();
    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 1 must be a symbol or an atomar type!");
    }

    string typeName = data_NL.str();

    // select distdata type
    string dataName;
    if (paramCnt == 2){
      if(!args_NL.second().isSymbol()){
        stringstream err;
        err << "Argument 2 must be the name of an existing "
        << "distdata type or \"" + DDATA_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      dataName = args_NL.second().str();
    }
    else
        dataName = DDATA_DEFAULT;

    // check, if selected distdata type is defined
    if (dataName == DDATA_DEFAULT){
      dataName = DistDataReg::defaultName(typeName);
      if(dataName == DDATA_UNDEFINED){
        string errmsg;
        errmsg = "No default distdata type defined for type "
        "constructor \"" + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else if(!DistDataReg::isDefined(typeName, dataName)){
      string errmsg;
      errmsg = "Distdata type \"" + dataName + "\" for "
      "type constructor \"" + typeName +
      "\" is not defined! Defined names: \n\n" +
      DistDataReg::definedNames(typeName);
      return listutils::typeError(errmsg);
    }

    NList res1(Symbol::APPEND());
    NList res2(NList(typeName, true), NList(dataName, true));
    NList res3(DistDataAttribute::BasicType());
    NList result(res1, res2, res3);
    return result.listExpr();
}


template<unsigned paramCnt>
ListExpr gdistance_TM(ListExpr args)
{
    #ifdef PRINT_DISTFUN_INFOS
    if (!distfuninfos_shown)
    {
        distfuninfos_shown = true;
        string distfun_str = DistfunReg::printDistfuns();
        cmsg.info() << distfun_str;
        cmsg.info() << "(this info is only shown once on the first "
                    << "call of the getdistdata or gdistance "
                    << "operator)" << endl;
        cmsg.send();
    }
    #endif

    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(paramCnt)){
      stringstream err;
      err << "Expected " << paramCnt << " arguments!";
      return listutils::typeError(err.str());
    }

    NList data1_NL = args_NL.first();
    NList data2_NL = args_NL.second();

    if(!data1_NL.isSymbol()){
      return listutils::typeError(
      "Argument 1 must be a symbol or an atomar type!");
    }
    if(!data2_NL.isSymbol()){
      return listutils::typeError(
      "Argument 2 must be a symbol or an atomar type!");
    }

    if(!(data1_NL == data2_NL)){
      return listutils::typeError(
                    "Expecting two data attributes of the same type!");
    }

    string typeName = data1_NL.str();

    // select distfun name
    string distfunName;
    if (paramCnt >= 3){
      if(!args_NL.third().isSymbol()){
        stringstream err;
        err << "Argument 3 must be the name of an existing "
        << "distance function or \"" + DFUN_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      distfunName = args_NL.third().str();
    }
    else
        distfunName = DFUN_DEFAULT;

    if(typeName == DistDataAttribute::BasicType())
    {   // further type checkings for distdata attributes are done in
        // the value mapping function, since they need the name of
        // the assigned type constructor, which is stored within the
        // attribute objects.
        NList res1(Symbol::APPEND());
        NList res2(distfunName, true); res2.enclose();
        NList res3(CcReal::BasicType());
        NList result(res1, res2, res3);
        return result.listExpr();
    }

    // select distdata type
    string dataName;
    if (paramCnt >= 4){
      if(!args_NL.fourth().isSymbol()){
        stringstream err;
        err << "Argument 4 must be the name of an existing "
        << "distdata type or \"" + DDATA_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      dataName = args_NL.fourth().str();
    }
    else
        dataName = DistDataReg::defaultName(typeName);

    // check, if selected distdata type is defined
    if (dataName == DDATA_DEFAULT){
      dataName = DistDataReg::defaultName(typeName);
      if(dataName == DDATA_UNDEFINED){
        string errmsg;
        errmsg = "No default distdata type defined for type "
        "constructor \"" + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else if(!DistDataReg::isDefined(typeName, dataName)){
      string errmsg;
      errmsg = "Distdata type \"" + dataName + "\" for "
      "type constructor \"" + typeName +
      "\" is not defined! Defined names: \n\n" +
      DistDataReg::definedNames(typeName);
      return listutils::typeError(errmsg);
    }

    // Returs a type error, if the specified distance function is not defined.
    string errmsg;
    if (distfunName == DFUN_DEFAULT){
      distfunName = DistfunReg::defaultName(typeName);
      if(distfunName == DFUN_UNDEFINED){
        errmsg = "No default distance function defined for type \""
        + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else {
      if (!DistfunReg::isDefined(distfunName, typeName, dataName)){
        errmsg = "Distance function \"" + distfunName +
          "\" not defined for type \"" +
          typeName + "\" and data type \"" +
          dataName + "\"! Defined names: \n\n" +
          DistfunReg::definedNames(typeName);
        return listutils::typeError(errmsg);
      }
    }

    NList res1(Symbol::APPEND());
    NList res2(NList(
        typeName, true),
        NList(distfunName, true),
        NList(dataName, true));
    NList res3(CcReal::BasicType());
    NList result(res1, res2, res3);
    return result.listExpr();
}



/********************************************************************
1.1 Selection functions

********************************************************************/
int gdistance_Select(ListExpr args)
{
  NList argsNL(args);
  NList arg1 = argsNL.first();

  if (arg1.isEqual(DistDataAttribute::BasicType()))
    return 1;
  else
    return 0;
}



/********************************************************************
1.1 Value mapping arrays

********************************************************************/
ValueMapping gdistance_Map[] = {
    gdistance_VM<2>,
    gdistanceDD_VM<2>
};

ValueMapping gdistance2_Map[] = {
    gdistance_VM<3>,
    gdistanceDD_VM<3>
};

ValueMapping gdistance3_Map[] = {
    gdistance_VM<4>,
    gdistanceDD_VM<4>
};


/********************************************************************
1.1.1 Operator Infos

********************************************************************/
struct gethpoint_Info : OperatorInfo
{
    gethpoint_Info()
    {
    name      = "gethpoint";
    signature = "DATA -> hpoint";
    syntax    = "gethpoint(_)";
    meaning   = "maps DATA to a hpoint, using the default "
                "get-hpoint function";
    example  =  "gethpoint(5)";
  }
};

struct gethpoint2_Info : OperatorInfo
{
  gethpoint2_Info()
  {
    name      = "gethpoint2";
    signature = "DATA x symbol -> hpoint";
    syntax    = "gethpoint2(_, _)";
    meaning   = "maps DATA to a hpoint, using the get-hpoint "
                "function specified in arg2";
    example   = "gethpoint2(5, native)";
  }
};

struct getbbox_Info : OperatorInfo
{
    getbbox_Info()
    {
    name      = "getbbox";
    signature = "DATA -> hrect";
    syntax    = "getbbox(_)";
    meaning   = "maps DATA to a hpoint, using the default "
                "get-hrect function";
    example  =  "getbbox(5)";
  }
};

struct getbbox2_Info : OperatorInfo
{
  getbbox2_Info()
  {
    name      = "getbbox2";
    signature = "DATA x symbol -> hrect";
    syntax    = "getbbox2(_, _)";
    meaning   = "maps DATA to a hpoint, using the get-hrect "
                "function specified in arg2";
    example   = "getbbox2(5, native)";
  }
};

struct getdistdata_Info : OperatorInfo
{
  getdistdata_Info()
  {
    name      = "getdistdata";
    signature = "DATA -> distdata";
    syntax    = "getdistdata(_)";
    meaning   = "maps DATA to a distdata attribute of the default "
                "type for type constructor of DATA";
    example   = "getdistdata(5)";
  }
};

struct getdistdata2_Info : OperatorInfo
{
  getdistdata2_Info()
  {
    name      = "getdistdata2";
    signature = "DATA x symbol -> distdata";
    syntax    = "getdistdata2(_, _)";
    meaning   = "maps DATA to a distdata attribute of the specified "
                "distdata-type (arg2).";
    example   = "getdistdata2(5, native)";
  }
};

struct gdistance_Info : OperatorInfo
{
  gdistance_Info()
  {
    name      = "gdistance";
    signature = "DATA x DATA -> real";
    syntax    = "gdistance(_, _)";
    meaning   = "computes the distance between arg1 and arg2, "
                "using the default distance function and default "
                "distdata-type (distdata attributes use the "
                "assigned distdata-type)";
    example   = "gdistance(5, 2)";
    remark    = "arg1 and arg2 must be of the same distdata-type";
  }
};

struct gdistance2_Info : OperatorInfo
{
  gdistance2_Info()
  {
    name      = "gdistance2";
    signature = "attribute x attribute x symbol -> real";
    syntax    = "gdistance2(_, _, _)";
    meaning   = "computes the distance between arg1 and arg2, "
                "using the specified distance function (arg3) and "
                "the default distdata-type (distdata attributes use "
                "the assigned datatype)";
    example   = "gdistance2(5, 2, euclid)";
    remark    = "arg1 and arg2 must be of the same distdata-type, ";
  }
};

struct gdistance3_Info : OperatorInfo
{
  gdistance3_Info()
  {
    name      = "gdistance3";
    signature = "attribute x attribute x symbol x symbol -> real";
    syntax    = "gdistance3(_, _, _, _)";
    meaning   = "computes the distance between arg1 and arg2, "
                "using the specified distance function (arg3) and "
                "the the specified distdata-type (arg4) ";
    example   = "gdistance3(5, 2, euclid, native)";
    remark    = "arg1 and arg2 must be of the same type, "
                "no distdata attributes allowed";
  }
};

/********************************************************************
1.1 Create and initialize the Algebra

********************************************************************/
class GeneralTreeAlgebra
    : public Algebra
{
  public:
    GeneralTreeAlgebra() : Algebra()
    {
        AddTypeConstructor(&hpoint_tc);
        AddTypeConstructor(&hrect_tc);
        AddTypeConstructor(&distdata_tc);

        hpoint_tc.AssociateKind( Kind::DATA() );
        hrect_tc.AssociateKind( Kind::DATA() );
        distdata_tc.AssociateKind( Kind::DATA() );

        AddOperator(
            gethpoint_Info(),
            gethpoint_VM<1>,
            gethpoint_TM<1>);

        AddOperator(
            gethpoint2_Info(),
            gethpoint_VM<2>,
            gethpoint_TM<2>);


        AddOperator(
            getbbox_Info(),
            getbbox_VM<1>,
            getbbox_TM<1>);

        AddOperator(
            getbbox2_Info(),
            getbbox_VM<2>,
            getbbox_TM<2>);

        AddOperator(
            getdistdata_Info(),
            getdistdata_VM<1>,
            getdistdata_TM<1>);

        AddOperator(
            getdistdata2_Info(),
            getdistdata_VM<2>,
            getdistdata_TM<2>);

        AddOperator(
            gdistance_Info(),
            gdistance_Map,
            gdistance_Select,
            gdistance_TM<2>);

        AddOperator(
            gdistance2_Info(),
            gdistance2_Map,
            gdistance_Select,
            gdistance_TM<3>);

       AddOperator(
            gdistance3_Info(),
            gdistance3_Map,
            gdistance_Select,
            gdistance_TM<4>);
    }

    ~GeneralTreeAlgebra() {};
}; // class GeneralTreeAlgebra

} // namespace gta


extern "C"
Algebra* InitializeGeneralTreeAlgebra(
        NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return (new gta::GeneralTreeAlgebra());
}
