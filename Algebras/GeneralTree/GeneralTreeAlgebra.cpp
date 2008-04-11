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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]
//paragraph [11] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [23]  table3columns:    [\begin{quote}\begin{tabular}{lll}] [\end{tabular}\end{quote}]
//[--------]      [\hline]
//[TOC] [\tableofcontents]

[11] General Tree Algebra

January-February 2008, Mirko Dibbert

[TOC]

1 Overview

This algebra provides the "distdata"[4] type constructor and the following operators:

  * "getdistdata(attr)"[4]

  * "getdistdata2(attr, data[_]name)"[4]

  * "gdistance(attr, attr)"[4]

  * "gdistance2(attr, attr, distfun[_]name)"[4]

  * "gdistance3(attr, attr, distfun[_]name, data[_]name)"[4]

The "distdata"[4] type constructor could be used to store precomputed data for faster computation of the respective distance functions. These attributes could e.g. be used in the "gdistance"[4] (general distance) and "gdistance2"[4] operators or in the "rangesearch"[4] and "nnsearch"[4] operators of the m-tree and x-tree algebra. This is useful, if the same attribute is needed multiple time for distance computeations, in particular if the computation is relatively complex, like the computation of picture histograms.

The "getdistdata"[4] and "getdistdata2"[4] operators could be used to create a new "distdata"[4] attribute for the respective data type. The first operator uses the default data type for the respective type constructor, whereas the second operator expects the name of a defined type as second parameter.

The currently registered distance functions and distdata types are curently only shown on the first call of the getdistdata2, gdistance2 or gdistance3 operator.
TODO: A "list distfuns"[4] command or an operator which shows the defined distance functions needs to be implemented, since the current way to do this is only a temporary solution)

Using "DFUN[_]DEFAULT"[4] resp. "DDATA[_]DEFAULT"[4] as name for the distance function or distdata type will select the default distance function / data type. Appending "[_]ncompr"[4] to the picture data type names (e.g. "hsv128[_]ncompr"[4]) uses uncompressed histograms, whereas the default data types store the histograms in a compressed manner.

The quardatic distance with the lab256 data type need the most time to be computed, but also returns the best results, since the lab-colorspace has been designed to approximate human vision (the fastest combination is euclid with hsv128 data).

The "gdistance"[4] operators expects two attributes of the same type as first and second parameter and the name of a defined distance function (only "gdistance2"[4] and "gdistance3"[4]) and distdata type (only "gdistance3"[4]) as third and fourth parameter.

The "gdistance3"[4] operator could not be used with "distdata"[4] attributes, since the data type is stored within these attributes.

1 Includes and defines

*/
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Base64.h"
#include "DistfunReg.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

namespace generalTree
{

/* comment out this define to disable printing a list of all defined
   distance functions, when calling the distdata2, gdistance2 or
   gdistance3 operator first time */
#define PRINT_DISTFUN_INFOS

/********************************************************************
1 Type constructors

1.1 Type constructor "distdata"[4]

********************************************************************/
static ListExpr
DistDataProp()
{
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist, "getdistdata(<attr>)");
    return
        nl->TwoElemList(
            nl->TwoElemList(
                nl->StringAtom("Creation"),
                nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                nl->TextAtom("let datarel = images feed extend "
                             "[data: getdistdata(.pic)] consume")));
}

ListExpr
OutDistData(ListExpr type_Info, Word value)
{
    DistDataAttribute* ddAttr =
            static_cast<DistDataAttribute*>(value.addr);
    Base64 b64;
    string b64string;
    b64.encode(ddAttr->value(), ddAttr->size(), b64string);
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

Word
InDistData(ListExpr type_Info, ListExpr value,
           int errorPos, ListExpr &error_Info, bool &correct)
{
    NList valueList(value);

    if (valueList.length() != 3)
    {
        correct = false;
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

    // get data strinc
    string data;
    if (!dataNL.isText())
    {
        correct = false;
        ErrorReporter::ReportError(
          "Excepted a string atom as third list element, but got " +
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

    DistDataId id = DistDataReg::getDataId(type, name);
    ddAttr->set(true, bindata, size, id);

    correct = true;

    return SetWord(ddAttr);
}

Word
createDistData(const ListExpr type_Info)
{ return SetWord(new DistDataAttribute(0)); }

void
DeleteDistData(const ListExpr type_Info, Word& w)
{
    DistDataAttribute* ddAttr =
        static_cast<DistDataAttribute*>(w.addr);
    ddAttr->deleteFLOB();
    delete ddAttr;
    w.addr = 0;
}

bool
OpenDistData(SmiRecord &valueRecord, size_t &offset,
             const ListExpr type_Info, Word& value)
{
    value = SetWord(Attribute::Open(valueRecord, offset, type_Info));
    return true;
}

bool
SaveDistData(SmiRecord &valueRecord, size_t &offset,
             const ListExpr type_Info, Word& value)
{
    Attribute::Save(valueRecord, offset, type_Info,
                    static_cast<Attribute*>(value.addr));
    return true;
}

void
CloseDistData(const ListExpr type_Info, Word& w)
{
    delete(DistDataAttribute*)w.addr;
}

Word
CloneDistData(const ListExpr type_Info, const Word& w)
{
    DistDataAttribute* src = static_cast<DistDataAttribute*>(w.addr);
    DistDataAttribute* cpy = new DistDataAttribute(*src);
    return SetWord(cpy);
}

void*
CastDistData(void *addr)
{
    return new(addr) DistDataAttribute;
}

int
SizeOfDistData()
{
    return sizeof(DistDataAttribute);
}

bool
CheckDistData(ListExpr typeName, ListExpr &error_Info)
{
    return (nl->IsEqual(typeName, "distdata"));
}

TypeConstructor
distDataTC("distdata",    DistDataProp,
           OutDistData,   InDistData,
           0, 0,
           createDistData, DeleteDistData,
           OpenDistData,   SaveDistData,
           CloseDistData,  CloneDistData,
           CastDistData,   SizeOfDistData,
           CheckDistData);

/********************************************************************
1 Operators

1.1 Value mappings

1.1.1 getdistdata[_]VM

Value mapping for operators "getdistdata"[4] and "getdistdata2"[4].

********************************************************************/
template <unsigned paramCnt> int
getdistdata_VM(Word* args, Word& result,
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

    DistDataId id = DistDataReg::getDataId(typeName, dataName);
    DistDataInfo info = DistDataReg::getInfo(id);
    ddAttr->set(true, info.getData(attr), id);

    return 0;
}

/********************************************************************
1.1.1 gdistance[_]VM

Value mapping for the "gdistance"[4] operators (no distdata attributes).

********************************************************************/
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

    DFUN_RESULT dist;
    DistfunReg::getInfo(distfunName, typeName, dataName).
                                            dist(attr1, attr2, dist);
    resultValue->Set(true, dist);

    return 0;
}

/********************************************************************
1.1.1 gdistanceDD[_]VM

Value mapping for the "gdistance"[4] operators (distdata attributes).

********************************************************************/
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

        if ((id1.algebraId != id2.algebraId) ||
            (id1.typeId != id2.typeId))
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

    DFUN_RESULT dist;
    DistData dd1(ddAttr1->size(), ddAttr1->value());
    DistData dd2(ddAttr2->size(), ddAttr2->value());
    info.dist(&dd1, &dd2, dist);
    resultValue->Set(true, dist);

    return 0;
}

/********************************************************************
1.1 Type mappings

1.1.1 getdistdata[_]TM

Type mapping for operator "getdistdata"[4].

********************************************************************/
ListExpr getdistdata_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    string errmsg;
    NList nl_args(args);

    errmsg = "Expecting one argument.";
    CHECK_COND(nl_args.length() == 1, errmsg);

    string typeName = nl_args.first().str();
    string dataName = DistDataReg::defaultName(typeName);

    // check if a default distdata type exists
    errmsg = "No default distdata type defined for attribute type \""
             + typeName + "\"!";
    CHECK_COND(dataName != DDATA_UNDEFINED, errmsg);

    NList res1(APPEND);
    NList res2(NList(typeName, true), NList(dataName, true));
    NList res3(DISTDATA);
    NList result(res1, res2, res3);
    return result.listExpr();
}

/********************************************************************
1.1.1 getdistdata2[_]TM

Type mapping for operator "getdistdata2"[4].

********************************************************************/
ListExpr getdistdata2_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    #ifdef PRINT_DISTFUN_INFOS
    DistfunReg::printDistfuns();
    #endif

    string errmsg;
    NList nl_args(args);

    errmsg = "Expecting two arguments.";
    CHECK_COND(nl_args.length() == 2, errmsg);

    string typeName = nl_args.first().str();

    // check if the distdata type given in second argument is defined
    errmsg = "Expecting the name of a registered distdata type as "
            "second argument, but got a list with structure '" +
            nl_args.second().convertToString() +
            "'!";
    CHECK_COND(nl_args.second().isSymbol(), errmsg);
    string dataName = nl_args.second().str();
    if (dataName == DDATA_DEFAULT)
    {
        errmsg =
            "No default distdata type defined for type \"" +
            typeName + "\"!";
        dataName = DistDataReg::defaultName(typeName);
        CHECK_COND(dataName != DDATA_UNDEFINED, errmsg);
    }
    else if(!DistDataReg::isDefined(typeName, dataName))
    {
        errmsg =
            "Distdata type \"" + dataName + "\" for type \"" +
            typeName + "\" is not defined! Defined names: \n\n" +
            DistDataReg::definedNames(typeName);
        CHECK_COND(false, errmsg);
    }

    NList res1(APPEND);
    NList res2(NList(typeName, true), NList(dataName, true));
    NList res3(DISTDATA);
    NList result(res1, res2, res3);
    return result.listExpr();
}

/********************************************************************
1.1.1 gdistance[_]TM

Type mapping for operator "gdistance"[4]

********************************************************************/
ListExpr gdistance_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    string errmsg;
    NList nl_args(args);

    errmsg = "Expecting two arguments.";
    CHECK_COND(nl_args.length() == 2, errmsg);

    NList arg1 = nl_args.first();
    NList arg2 = nl_args.second();

    errmsg = "Expecting two attributes of the same type!";
    CHECK_COND(arg1 == arg2, errmsg);

    string typeName = arg1.str();

    if(typeName == DISTDATA)
    {   // further type checkings for distdata attributes are done in
        // the value mapping function, since they need the name of
        // the assigned type constructor, which is stored within the
        // attribute objects.
        NList res1(APPEND);
        NList res2(DFUN_DEFAULT, true); res2.enclose();
        NList res3(REAL);
        NList result(res1, res2, res3);
        return result.listExpr();
    }
    else
    {
        string distfunName = DistfunReg::defaultName(typeName);
        string dataName = DistDataReg::defaultName(typeName);
        if (distfunName == DFUN_UNDEFINED)
        {
            errmsg =
                "No default distance function defined for type \""
                + typeName + "\"!" + " Defined names: \n\n" +
                distfunName;
            CHECK_COND(false, errmsg);
        }

        NList res1(APPEND);
        NList res2(NList(
            typeName, true),
            NList(distfunName, true),
            NList(dataName, true));
        NList res3(REAL);
        NList result(res1, res2, res3);
        return result.listExpr();
    }
}

/********************************************************************
1.1.1 gdistance2[_]TM

Type mapping for operator "gdistance"[4]

********************************************************************/
ListExpr gdistance2_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    #ifdef PRINT_DISTFUN_INFOS
    DistfunReg::printDistfuns();
    #endif

    string errmsg;
    NList nl_args(args);

    errmsg = "Expecting three arguments.";
    CHECK_COND(nl_args.length() == 3, errmsg);

    NList arg1 = nl_args.first();
    NList arg2 = nl_args.second();
    NList arg3 = nl_args.third();

    errmsg = "Expecting two attributes of the same type!";
    CHECK_COND(arg1 == arg2, errmsg);

    string typeName = arg1.str();

    errmsg =
        "Expecting the name of a registered distance function as "
        "third argument, but got a list with structure '" +
        arg3.convertToString() + "'!";
    CHECK_COND(arg3.isSymbol(), errmsg);
    string distfunName = arg3.str();
    string dataName = DistDataReg::defaultName(typeName);

    if (typeName == DISTDATA)
    {   // further type checkings for distdata attributes are done in
        // the value mapping function, since they need the name of
        // the assigned type constructor, which is stored within the
        // attribute objects.
        NList res1(APPEND);
        NList res2(distfunName, true); res2.enclose();
        NList res3(REAL);
        NList result(res1, res2, res3);
        return result.listExpr();
    }

    if (distfunName == DFUN_DEFAULT)
    {
        distfunName = DistfunReg::defaultName(typeName);
        errmsg =
            "No default distance function defined for type \""
            + typeName + "\"!";
        CHECK_COND(distfunName != DFUN_UNDEFINED, errmsg);
    }
    else
    { // search distfun
        if (!DistfunReg::isDefined(
                distfunName, typeName, dataName))
        {
            errmsg =
                "Distance function \"" + distfunName +
                "\" not defined for type \"" +
                typeName + "\" and data type \"" +
                dataName + "\"! " +
                "Defined names: \n\n" +
                DistfunReg::definedNames(typeName);
            CHECK_COND(false, errmsg);
        }
    }

        NList res1(APPEND);
        NList res2(NList(
            typeName, true),
            NList(distfunName, true),
            NList(dataName, true));
        NList res3(REAL);
        NList result(res1, res2, res3);
        return result.listExpr();
    return result.listExpr();
}

/********************************************************************
1.1.1 gdistance3[_]TM

Type mapping for operator "gdistance3"[4]

********************************************************************/
ListExpr gdistance3_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    #ifdef PRINT_DISTFUN_INFOS
    DistfunReg::printDistfuns();
    #endif

    string errmsg;
    NList nl_args(args);

    errmsg = "Expecting four arguments.";
    CHECK_COND(nl_args.length() == 4, errmsg);

    NList arg1 = nl_args.first();
    NList arg2 = nl_args.second();
    NList arg3 = nl_args.third();
    NList arg4 = nl_args.fourth();

    errmsg = "Expecting two attributes of the same type!";
    CHECK_COND(arg1 == arg2, errmsg);

    string typeName = arg1.str();

    errmsg =
        "Expecting the name of a registered distance function as "
        "third argument, but got a list with structure '" +
        arg3.convertToString() + "'!";
    CHECK_COND(arg3.isSymbol(), errmsg);
    string distfunName = arg3.str();

    errmsg =
        "Operator gdistance3 could not be used with distdata "
        "attributes! Use gdistance or gdistance2 instead.";
    CHECK_COND(typeName != DISTDATA, errmsg);

    // check if the distdata type given in second argument is defined
    errmsg = "Expecting the name of a registered distdata type as "
            "second argument, but got a list with structure '" +
            arg4.str() + "'!";
    CHECK_COND(arg4.isSymbol(), errmsg);
    string dataName = arg4.str();

    if (dataName == DDATA_DEFAULT)
    {
        errmsg =
            "No default distdata type defined for type \"" +
            typeName + "\"!";
        dataName = DistDataReg::defaultName(typeName);
        CHECK_COND(dataName != DDATA_UNDEFINED, errmsg);
    }
    else if(!DistDataReg::isDefined(typeName, dataName))
    {
        errmsg =
            "Distdata type \"" + dataName + "\" for type \"" +
            typeName + "\" is not defined! Defined names: \n\n" +
            DistDataReg::definedNames(typeName);
        CHECK_COND(false, errmsg);
    }

    if (distfunName == DFUN_DEFAULT)
    {
        distfunName = DistfunReg::defaultName(typeName);
        errmsg =
            "No default distance function defined for type \""
            + typeName + "\"!";
        CHECK_COND(distfunName != DFUN_UNDEFINED, errmsg);
    }
    else
    { // search distfun
        if (!DistfunReg::isDefined(
                distfunName, typeName, dataName))
        {
            errmsg =
                "Distance function \"" + distfunName +
                "\" not defined for type \"" +
                typeName + "\" and data type \"" +
                dataName + "\"! " +
                "Defined names: \n\n" +
                DistfunReg::definedNames(typeName);
            CHECK_COND(false, errmsg);
        }
    }

    NList res1(APPEND);
    NList res2(NList(
        typeName, true),
        NList(distfunName, true),
        NList(dataName, true));
    NList res3(REAL);
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

  if (arg1.isEqual("distdata"))
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

/********************************************************************
1.1 Operator infos

********************************************************************/
struct getdistdata_Info : OperatorInfo
{
  getdistdata_Info()
  {
    name      = "getdistdata";
    signature = "attribute -> distdata";
    syntax    = "getdistdata(_)";
    meaning   = "returns the distdata attribute for the given "
                "attribute, using the default distdata type";
    example   = "getdistdata(5)";
  }
};

struct getdistdata2_Info : OperatorInfo
{
  getdistdata2_Info()
  {
    name      = "getdistdata2";
    signature = "attribute x distdataName -> distdata";
    syntax    = "getdistdata(_, _)";
    meaning   = "returns the distdata attribute for the given "
                "attribute, using the specified distdata type";
    example   = "getdistdata(5, native)";
  }
};

struct gdistance_Info : OperatorInfo
{
  gdistance_Info()
  {
    name      = "gdistance";
    signature = "attribute x attribute -> real";
    syntax    = "gdistance(_, _)";
    meaning   = "computes the distance between arg1 and arg2, "
                "using the default distance function and default "
                "datatype (distdata attributes use the assigned "
                "datatype)";
    example   = "gdistance(5, 2)";
    remark    = "arg1 and arg2 must be of the same type";
  }
};

struct gdistance2_Info : OperatorInfo
{
  gdistance2_Info()
  {
    name      = "gdistance2";
    signature = "attribute x attribute x distfunName -> real";
    syntax    = "gdistance2(_, _, _)";
    meaning   = "computes the distance between arg1 and arg2, "
                "using the specified distance function and default "
                "datatype (distdata attributes use the assigned "
                "datatype)";
    example   = "gdistance2(5, 2, euclid)";
    remark    = "arg1 and arg2 must be of the same type";
  }
};

struct gdistance3_Info : OperatorInfo
{
  gdistance3_Info()
  {
    name      = "gdistance3";
    signature = "attribute x attribute x distfunName x "
                "distdataName -> real";
    syntax    = "gdistance3(_, _, _, _)";
    meaning   = "computes the distance between arg1 and arg2, "
                "using the specified distance function and datatype";
    example   = "gdistance3(5, 2, euclid, native)";
    remark    = "arg1 and arg2 must be of the same type, "
                "no distdata attributes";
  }
};

/********************************************************************
1 Create and initialize the Algebra

********************************************************************/

class GeneralTreeAlgebra : public Algebra
{

public:
    GeneralTreeAlgebra() : Algebra()
    {
        AddTypeConstructor(&distDataTC);
        distDataTC.AssociateKind("DATA");

        AddOperator(
            getdistdata_Info(),
            getdistdata_VM<1>,
            getdistdata_TM);

        AddOperator(
            getdistdata2_Info(),
            getdistdata_VM<2>,
            getdistdata2_TM);

        AddOperator(
            gdistance_Info(),
            gdistance_Map,
            gdistance_Select,
            gdistance_TM);

        AddOperator(
            gdistance2_Info(),
            gdistance2_Map,
            gdistance_Select,
            gdistance2_TM);

        AddOperator(
            gdistance3_Info(),
            gdistance_VM<4>,
            gdistance3_TM);
    }

    ~GeneralTreeAlgebra() {};
};

} // generalTree

generalTree::GeneralTreeAlgebra generalTreeAlgebra;

extern "C"
    Algebra* InitializeGeneralTreeAlgebra(
        NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return (&generalTreeAlgebra);
}
