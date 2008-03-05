/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute iter and/or modify
iter under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that iter will be useful,
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

1 Implementation file "DistfunReg.cpp"[4]

January-February 2008, Mirko Dibbert
\\[3ex]
This file implements the "DistfunReg"[4] class.

*/
#include <sstream>
#include "DistfunReg.h"
#include "PictureFuns.h"

using namespace general_tree;

/*
Initialize static members:

*/
bool DistfunReg::initialized = false;
bool DistfunReg::distfunsShown = false;
map<string, string> DistfunReg::defaultNames;
DistfunInfo DistfunReg::defaultInfo;
map<string, DistfunInfo> DistfunReg::distfunInfos;

/*
Method ~addInfo~:

*/
void
DistfunReg::addInfo(DistfunInfo info)
{
    ostringstream osId;
    osId << info.data().algebraId() << "#"
         << info.data().typeId() << "#"
         << info.name() << "#"
         << info.data().id();
    distfunInfos [osId.str()] = info;

    if (info.isDefault())
        defaultNames[info.data().typeName()] = info.name();
}

/*
Method ~getDefaultDistfunName~:

*/
string
DistfunReg::defaultName(const string& typeName)
{
    if (!initialized)
        initialize();

    map<string, string>::iterator iter =
            defaultNames.find(typeName);
    if (iter == defaultNames.end())
        return DFUN_UNDEFINED;
    else
        return iter->second;
}

/*
Method ~getInfo~:

*/
DistfunInfo& DistfunReg::getInfo(
    const string& distfunName,
    const string& typeName,
    const string& dataName)
{
    if (!initialized)
        initialize();

    ostringstream osId;
    DistDataId id = DistDataReg::getDataId(typeName, dataName);

    if (!id.defined)
        return defaultInfo;

    if (distfunName == DFUN_DEFAULT)
    {
        osId << id.algebraId << "#" << id.typeId << "#"
            << defaultName(typeName) << "#" << id.distdataId;
    }
    else
    {
        osId << id.algebraId << "#" << id.typeId << "#"
            << distfunName << "#" << id.distdataId;
    }

    distfunIter iter2 = distfunInfos.find(osId.str());
    if (iter2 != distfunInfos.end())
        return iter2->second;
    else
        return defaultInfo;
}

/*
Method ~definedNames~:

*/
string
DistfunReg::definedNames(const string& typeName)
{
    if (!initialized)
        initialize();

    // search first info object for typeName
    distfunIter iter = distfunInfos.begin();
    while ((iter != distfunInfos.end()) &&
            (iter->second.data().typeName() != typeName))
    {
        ++iter;
    }

    // process all info objects for typeName
    ostringstream result;
    while ((iter != distfunInfos.end()) &&
            (iter->second.data().typeName() == typeName))
    {
        result << "\"" << iter->second.name() << "\""
               << "(accepted data type(s): ";
        string curName = iter->second.name();
        while ((iter != distfunInfos.end()) &&
                (iter->second.data().typeName() == typeName) &&
                (iter->second.name() == curName))
        {
            result << "\"" << iter->second.data().name() << "\"";
            ++iter;
            if ((iter != distfunInfos.end()) &&
                    (iter->second.data().typeName() == typeName) &&
                    (iter->second.name() == curName))
                result << ", ";
        }
        result << ")";
        if ((iter != distfunInfos.end()) &&
                (iter->second.data().typeName() == typeName))
        {
            result << endl << endl;
        }
    }
    return result.str();
}

/*
Method ~getInfos~:

*/
void
DistfunReg::getInfoList(list<DistfunInfo>& result)
{
    distfunIter iter = distfunInfos.begin();
    while(iter != distfunInfos.end())
    {
        result.push_back(iter->second);
        ++iter;
    }
}

/*
Method ~printDistfuns~:

*/
void
DistfunReg::printDistfuns()
{
    if (distfunsShown)
        return;
    else
        distfunsShown = true;

    if (!initialized)
        initialize();

    const string seperator = "\n" + string(70, '-') + "\n";
    cmsg.info() << endl << "Defined distance functions:"
                << seperator;
    list<DistfunInfo> distfunInfos;
    DistfunReg::getInfoList(distfunInfos);
    list<DistfunInfo>::iterator iter = distfunInfos.begin();
    while(iter != distfunInfos.end())
    {
        cmsg.info() << "name             : "
                    << iter->name() << endl
                    << "description      : "
                    << iter->descr() << endl
                    << "type constructor : "
                    << iter->data().typeName() << endl
                    << "accepted distdata: ";

        string curName = iter->name();
        string curType = iter->data().typeName();
        while ((iter->name() == curName) &&
                (iter->data().typeName() == curType))
        {
            cmsg.info() << iter->data().name();
            ++iter;

            if ((iter->name() == curName) &&
                    (iter->data().typeName() == curType))
                cmsg.info() << ", ";
        }
        cmsg.info() << seperator;
    }
    cmsg.info() << endl;
    cmsg.send();
}

/********************************************************************
Below, all avaliable distance functions are implemented:

********************************************************************/
/*
Method ~EuclideanInt~:

*/
void DistfunReg::EuclideanInt(
    const DistData* data1, const DistData* data2, double& result)
{
    int val1 = *static_cast<const int*>(data1->value());
    int val2 = *static_cast<const int*>(data2->value());
    result = abs(val1 - val2);
}

/*
Method ~EuclideanReal~:

*/
void DistfunReg::EuclideanReal(
    const DistData* data1, const DistData* data2, double& result)
{
    SEC_STD_REAL val1 =
        *static_cast<const SEC_STD_REAL*>(data1->value());
    SEC_STD_REAL val2 =
        *static_cast<const SEC_STD_REAL*>(data2->value());
    result = abs(val1 - val2);
}

/*
Method ~EditDistance~ :

*/
void DistfunReg::EditDistance(
    const DistData* data1, const DistData* data2, double& result)
{
    const char* str1 = static_cast<const char*>(data1->value());
    const char* str2 = static_cast<const char*>(data2->value());

    int len1 = static_cast<const DistData*>(data1)->size();
    int len2 = static_cast<const DistData*>(data2)->size();

    int d[len1 + 1][len2 + 1];
    int dist;

    // init row 1 with
    for (int i = 0; i <= len1; ++i)
        d[i][0] = i;

    // init col 1
    for (int j = 1; j <= len2; ++j)
        d[0][j] = j;

    // compute array getValues
    for (int i = 1; i <= len1; ++i)
    {
        for (int j = 1; j <= len2; ++j)
        {
        if (str1[i - 1] == str2[j - 1])
            dist = 0;
        else
            dist = 1;

        // d(i,j) = min{ d(i-1 , j ) + 1,
        //         d(i   , j-1) + 1,
        //         d(i-1 , j-1) + dist }
        d[i][j] = min(d[i - 1][j] + 1,
                    min((d[i][j - 1]) + 1,
                    d[i - 1][j - 1] + dist));
        }
    }
    result = (double) d[len1][len2];
}



/*
Method ~initialize~:

Registeres all defined distance functions. The parameters of the DistfunInfo constructor have the following meaning:

  1 name of the distance function

  2 description of the distance function

  3 pointer to the distance function

  4 id of the assigned distdata type

  5 flags

The following flags could be set:

  * "DFUN[_]IS[_]DEFAULT"[4]\\
    uses the resp. distance function as default distance function

  * "FUN[_]IS[_]METRIC"[4]\\
    indicates, that the distance function is a metric

If more than one distance function for a specific type constructer has been defined as default, the last registered one will be used as default.

Constants for the name and description for all distance functions should be defined in the "DistfunNames.h"[4] headerfile.

*/
void
DistfunReg::initialize()
{
    if (initialized)
        return;

    addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        EuclideanInt,
        DistDataReg::getDataId(INT, DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        EuclideanReal,
        DistDataReg::getDataId(REAL, DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_EDIT_DIST, DFUN_EDIT_DIST_DESCR,
        EditDistance,
        DistDataReg::getDataId(STRING, DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    picture_funs::PictureFuns::initDistfuns();

    initialized = true;
}
