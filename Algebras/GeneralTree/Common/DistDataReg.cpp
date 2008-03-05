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

1 Implementation file "DistDataReg.cpp"[4]

January-February 2008, Mirko Dibbert
\\[3ex]
This file implements the "DistDataReg"[4] class.

*/
#include <sstream>
#include "DistDataReg.h"
#include "PictureFuns.h"

using namespace general_tree;

/*
Initialize static members :

*/
bool DistDataReg::initialized = false;
map<string, string> DistDataReg::defaultNames;
DistDataInfo DistDataReg::defaultInfo;
map<string, int> DistDataReg::dataIds;
map<string, DistDataInfo> DistDataReg::distdataInfos;

/*
Method ~addInfo~:

*/
void
DistDataReg::addInfo(DistDataInfo info)
{
    ostringstream osTypeId;
    ostringstream osId;
    osTypeId << info.algebraId() << "#" << info.typeId();
    osId << osTypeId.str() << "#" << info.id();
    distdataInfos [osId.str()] = info;

    dataIds[info.name()] = info.id();

    if (info.isDefault())
        defaultNames[info.typeName()] = info.name();
}

/*
Method ~getDefaultDataName~:

*/
string
DistDataReg::defaultName(const string& typeName)
{
    if (!initialized)
        initialize();

    map<string, string>::iterator iter =
            defaultNames.find(typeName);
    if (iter == defaultNames.end())
        return DDATA_UNDEFINED;
    else
        return iter->second;
}

/*
Method ~getDataId~:

*/
DistDataId DistDataReg::getDataId(
        const string& typeName, const string& dataName)
{
    if (!initialized)
        initialize();

    int algebraId, typeId;
    si->GetTypeId(typeName, algebraId, typeId);

    map<string, int>::iterator iter;

    if (dataName == DDATA_DEFAULT)
        iter = dataIds.find(defaultName(typeName));
    else
        iter = dataIds.find(dataName);

    if (iter != dataIds.end())
        return DistDataId(algebraId, typeId, iter->second);
    else
        return DistDataId(false);
}

/*
Method ~getInfo~:

*/
DistDataInfo&
DistDataReg::getInfo(DistDataId id)
{
    if (!initialized)
        initialize();

    ostringstream osId;
    osId << id.algebraId << "#" << id.typeId << "#" << id.distdataId;

    distdataIter iter = distdataInfos.find(osId.str());
    if (iter != distdataInfos.end())
        return iter->second;
    else
        return defaultInfo;
}

/*
Method ~definedNames~:

*/
string
DistDataReg::definedNames(const string& typeName)
{
    if (!initialized)
        initialize();

    // search first info object for typeName
    distdataIter iter = distdataInfos.begin();
    while ((iter != distdataInfos.end()) &&
            (iter->second.typeName() != typeName))
    {
        ++iter;
    }

    // process all info objects for typeName
    ostringstream result;
    while ((iter != distdataInfos.end()) &&
            (iter->second.typeName() == typeName))
    {
        result << "\"" << iter->second.name() << "\"";

        if (iter->second.isDefault())
            result << " [default]";

        ++iter;
        if ((iter != distdataInfos.end()) &&
            (iter->second.typeName() == typeName))
            result << ", ";
    }

    return result.str();
}

/********************************************************************
Below, all avaliable getdata functions are implemented:

********************************************************************/
/*
Method ~getDataInt~:

*/

DistData* DistDataReg::getDataInt(const void* attr)
{
  int value = static_cast<const CcInt*>(attr)->GetValue();
  char buffer[sizeof(int)];
  memcpy(buffer, &value, sizeof(int));
  return new DistData(sizeof(int), buffer);
}

/*
Method ~getDataReal~:

*/
DistData* DistDataReg::getDataReal(const void* attr)
{
  SEC_STD_REAL value = static_cast<const CcReal*>(attr)->GetValue();
  char buffer[sizeof(SEC_STD_REAL)];
  memcpy(buffer, &value, sizeof(SEC_STD_REAL));
  return new DistData(sizeof(SEC_STD_REAL), buffer);
}

/*
Method ~getDataString~:

*/
DistData* DistDataReg::getDataString(const void* attr)
{
  string value = static_cast<const CcString*>(attr)-> GetValue();
  return new DistData(value);
}



/*
Method ~initialize~:

Registeres all defined distdata types. The parameters of the DistDataInfo constructor have the following meaning:

  1 name of the distdata type

  2 description of the distdata type

  3 unique id of the distdata type

  4 name of the assigned type constructor

  5 pointer to the respective getdata function

  6 flags

Currently only the following flag is defined (except of the "DDATA[_]IS[_]DEFINED"[4] flag, which is set automatically from the constructor):

  * "DDATA[_]IS[_]DEFAULT"[4]\\
    uses the resp. distdata type as default (e.g. for the getdistdata operator of the GeneralTreeAlgebra)

If more than one distdata type for a specific type constructer has been defined with the default flag, the last registered one will be used as default.

Constants for the name, description and type id for all distdata types should be defined in the "DistDataNames.h"[4] headerfile.

*/
void
DistDataReg::initialize()
{
    if (initialized)
        return;

    addInfo(DistDataInfo(
        DDATA_NATIVE, DDATA_NATIVE_DESCR, DDATA_NATIVE_ID,
        INT, getDataInt, DDATA_IS_DEFAULT));

    addInfo(DistDataInfo(
        DDATA_NATIVE, DDATA_NATIVE_DESCR, DDATA_NATIVE_ID,
        REAL, getDataReal, DDATA_IS_DEFAULT));

    addInfo(DistDataInfo(
        DDATA_NATIVE, DDATA_NATIVE_DESCR, DDATA_NATIVE_ID,
        STRING, getDataString, DDATA_IS_DEFAULT));

    picture_funs::PictureFuns::initDistData();

    initialized = true;
}
