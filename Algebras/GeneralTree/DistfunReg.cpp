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

1 Implementation file "DistfunReg.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include <sstream>
#include "DistfunReg.h"
#include "PictureFuns.h"
#include "GTA_SpatialAttr.h"
#include "Coord.h"
#include "Point.h"
#include "SpatialAlgebra.h"
#include "Algorithms.h"

using namespace gta;

/*
Initialize static members:

*/
bool DistfunReg::initialized = false;
bool DistfunReg::distfunsShown = false;
map<string, string> DistfunReg::defaultNames;
DistfunInfo DistfunReg::defaultInfo;
map<string, DistfunInfo> DistfunReg::distfunInfos;

/*
Default constructor for the ~DistfunInfo~ class:

*/
DistfunInfo::DistfunInfo()
: m_name("undef"), m_descr("undef"), m_distfun(0),
  m_distdataInfo(), m_flags(0)
{}

/*
Constructor for the ~DistfunInfo~ class:

*/
DistfunInfo::DistfunInfo(
        const string &name, const string &descr,
        const Distfun distfun, DistDataInfo distdataInfo,
        char flags)
: m_name(name), m_descr(descr), m_distfun(distfun),
  m_distdataInfo(distdataInfo), m_flags(DFUN_IS_DEFINED | flags)
{}

/*
Method ~DistfunReg::addInfo~:

*/
void DistfunReg::addInfo(DistfunInfo info)
{
    ostringstream osId;
    osId << info.data().algebraId() << "#"
         << info.data().typeId() << "#"
         << info.name() << "#"
         << info.data().distdataId();
    distfunInfos [osId.str()] = info;

    if (info.isDefault())
    {
        assert(info.data().isDefined());
        defaultNames[info.data().typeName()] = info.name();
        DistDataReg::defaultNames[info.data().typeName()] =
                                                  info.data().name();
    }
}

/*
Method ~DistfunReg::defaultName~:

*/
string DistfunReg::defaultName(const string &typeName)
{
    if(!initialized)
        initialize();

    map<string, string>::iterator iter =
            defaultNames.find(typeName);
    if (iter == defaultNames.end())
        return DFUN_UNDEFINED;
    else
        return iter->second;
}

/*
Method ~DistfunReg::getInfo~:

*/
DistfunInfo &DistfunReg::getInfo(
        const string &distfunName, DistDataId id)
{
    if(!initialized)
        initialize();

    ostringstream osId;
    if (!id.isDefined())
        return defaultInfo;

    if (distfunName == DFUN_DEFAULT)
    {
        string typeName = id.typeName();
        osId << id.algebraId() << "#" << id.typeId() << "#"
             << defaultName(typeName) << "#" << id.distdataId();
    }
    else
    {
        osId << id.algebraId() << "#" << id.typeId() << "#"
             << distfunName << "#" << id.distdataId();
    }

    distfunIter iter2 = distfunInfos.find(osId.str());
    if (iter2 != distfunInfos.end())
        return iter2->second;
    else
        return defaultInfo;
}

/*
Method ~DistfunReg::definedNames~:

*/
string DistfunReg::definedNames(const string &typeName)
{
    if(!initialized)
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
Method ~DistfunReg::getInfoList~:

*/
void DistfunReg::getInfoList(list<DistfunInfo>& result)
{
    if(!initialized)
        initialize();

    distfunIter iter = distfunInfos.begin();
    while(iter != distfunInfos.end())
    {
        result.push_back(iter->second);
        ++iter;
    }
}

/*
Method ~DistfunReg::printDistfuns~:

*/
string DistfunReg::printDistfuns()
{
    if(!initialized)
        initialize();

    ostringstream result;

    const string seperator = "\n" + string(70, '-') + "\n";
    result << endl << "Defined distance functions:"
           << seperator;
    list<DistfunInfo> distfunInfos;
    DistfunReg::getInfoList(distfunInfos);
    list<DistfunInfo>::iterator iter = distfunInfos.begin();

    while(iter != distfunInfos.end())
    {
        result << "name             : "
               << iter->name() << endl
               << "description      : "
               << iter->descr() << endl
               << "type constructor : "
               << iter->data().typeName() << endl
               << "accepted distdata: ";

        string curName = iter->name();
        string curType = iter->data().typeName();
        while ((iter != distfunInfos.end()) &&
               (iter->name() == curName) &&
               (iter->data().typeName() == curType))
        {
            result << iter->data().name();
            ++iter;

            if ((iter != distfunInfos.end()) &&
                (iter->name() == curName) &&
                (iter->data().typeName() == curType))
                result << ", ";
        }
        result << seperator;
    }
    result << endl;
    return result.str();
}

/********************************************************************
Implementation of distance functions:

********************************************************************/
/*
Method ~DistfunReg::EuclideanInt~:

*/
void DistfunReg::EuclideanInt(
        const DistData *data1, const DistData *data2,
        double &result)
{
    int val1 = *static_cast<const int*>(data1->value());
    int val2 = *static_cast<const int*>(data2->value());
    result = abs(val1 - val2);
}

/*
Method ~DistfunReg::EuclideanReal~:

*/
void DistfunReg::EuclideanReal(
        const DistData *data1, const DistData *data2,
        double &result)
{
    SEC_STD_REAL val1 =
        *static_cast<const SEC_STD_REAL*>(data1->value());
    SEC_STD_REAL val2 =
        *static_cast<const SEC_STD_REAL*>(data2->value());
    result = abs(val1 - val2);
}

void DistfunReg::euclidPoint(
         const DistData *data1, const DistData *data2,
         double &result) {

     // handle undefined values
     if(data1->size()==0 && data2->size()==0){
        result = 0;
        return ;
     }
     if(data1->size()==0 || data2->size()==0){
        result =  numeric_limits<double>::max();
        return;
     }

     Coord x1;
     Coord y1;
     memcpy(&x1, data1->value(), sizeof(Coord));
     memcpy(&y1, (char*) data1->value() + sizeof(Coord), sizeof(Coord));
     Coord x2;
     Coord y2;
     memcpy(&x2, data2->value(), sizeof(Coord));
     memcpy(&y2, (char*) data2->value() + sizeof(Coord), sizeof(Coord));
     result = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)); 
 }


/*
Method specialPoints.

*/
void DistfunReg::specialPoints(
        const DistData *data1, const DistData *data2,
        double &result)
{

  if(data1->size()==1 && data2->size() == 1){
     result = 0;
  }
  size_t n1 = data1->size();
  size_t n2 = data2->size();

  size_t o1 = 0; // offset in value1
  size_t o2 = 0;

  result = 0; 

  while(o1<n1 && o2<n2){
     Coord x1;
     Coord x2;
     memcpy(&x1, (char*)data1->value() + o1, sizeof(Coord));
     memcpy(&x2, (char*)data2->value() +o2, sizeof(Coord));
     if(AlmostEqual(x1,x2)){
        o1 += sizeof(Coord);
        o2 += sizeof(Coord);
        Coord y1;
        Coord y2;
        memcpy(&y1,(char*)data1->value() + o1, sizeof(Coord));
        memcpy(&y2,(char*)data2->value() + o2, sizeof(Coord));
        result += abs(y1-y2);
        o1 += sizeof(Coord);
        o2 += sizeof(Coord);
     } else if(x1<x2) {
        o1 += sizeof(Coord);
        Coord y1;
        memcpy(&y1,(char*)data1->value() + o1, sizeof(Coord));
        result += abs(y1);
        o1 += sizeof(Coord);
     } else {
        o2 += sizeof(Coord);
        Coord y2;
        memcpy(&y2,(char*)data2->value() + o2, sizeof(Coord));
        result += abs(y2);
        o1 += sizeof(Coord);
        o2 += sizeof(Coord);
     }
  }
  while(o1<n1){
        o1 += sizeof(Coord); // overread x
        Coord y1;
        memcpy(&y1,(char*)data1->value() + o1, sizeof(Coord));
        result += abs(y1);
        o1 += sizeof(Coord);
  } 
  while(o2<n2){
        o2 += sizeof(Coord);
        Coord y2;
        memcpy(&y2,(char*)data2->value() + o2, sizeof(Coord));
        result += abs(y2);
        o1 += sizeof(Coord);
        o2 += sizeof(Coord);
  }
}




/*
Method ~DistfunReg::EuclideanHPoint~:

*/
void DistfunReg::EuclideanHPoint(
        const DistData *data1, const DistData *data2,
        double &result)
{
    result = 0.0;
    const char *buf1 = static_cast<const char*>(data1->value());
    const char *buf2 = static_cast<const char*>(data2->value());

    // extract dimensions and check for equality
    unsigned dim1, dim2;
    memcpy(&dim1, buf1, sizeof(unsigned));
    memcpy(&dim2, buf2, sizeof(unsigned));
    if (dim1 != dim2)
    {
        cmsg.error()
            << "Tried to compute Euclidean distance between hpoints "
            << "of different dimensions!" << endl;
        cmsg.send();
        return;
    }

    // extract coordinate vectors
    const GTA_SPATIAL_DOM *coords1 =
            reinterpret_cast<const GTA_SPATIAL_DOM*>(
            buf1 + sizeof(unsigned));
    const GTA_SPATIAL_DOM *coords2 =
            reinterpret_cast<const GTA_SPATIAL_DOM*>(
            buf2 + sizeof(unsigned));

    // compute eucledean distance
    for (unsigned i = 0; i < dim1; ++i)
        result += pow(abs (coords1[i] - coords2[i]), 2);
    result = sqrt(result);
}


/*
Method ~DistfunReg::EditDistance~ :

*/
void DistfunReg::EditDistance(
        const DistData *data1, const DistData *data2,
        double &result)
{
    const char *str1 = static_cast<const char*>(data1->value());
    const char *str2 = static_cast<const char*>(data2->value());

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
Method ~DistfunReg::symTrajDistance1~ :

*/
template<class M>
void DistfunReg::symTrajDistance1(const DistData *data1, const DistData *data2,
                                  double &result) {
  if (data1->size() == 0 && data2->size() == 0) {
    result = 0;
  }
  if (data1->size() == 0 || data2->size() == 0){
    result = numeric_limits<double>::max();
  }
  M traj1, traj2;
  traj1.deserialize((const char*)data1->value());
  traj2.deserialize((const char*)data2->value());
  result = traj1.Distance(traj2);
}

/********************************************************************
Method ~DistfunReg::initialize~:

All avaliable distance functions must be registered in this method by calling the "addInfo"[4] method whith the respective "DistfunInfo" object. The parameter of the "DistfunInfo"[4] constructor have the following meaning:

  1 name of the distance function

  2 description of the distance function

  3 pointer to the distance function

  4 info obect of the assigned distdata type

  5 flags (optional)

The following flags could be set:

  * "DFUN[_]IS[_]DEFAULT"[4]\\
    uses the distance function as default for the resp. type constructor

  * "FUN[_]IS[_]METRIC"[4]\\
    indicates, that the distance function is a metric

If more than one distance function for a specific type constructer has been defined as default, the last registered one will be used as default.

Constants for the name and description for the distance functions are defined in the "DistfunSymbols.h"[4] headerfile.

*/
void DistfunReg::initialize()
{
    if (initialized)
        return;

    DistDataReg::initialize();

    addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        EuclideanInt,
        DistDataReg::getInfo(CcInt::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        EuclideanReal,
        DistDataReg::getInfo(CcReal::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
      DFUN_EUCLID, DFUN_EUCLID_DESCR,
      euclidPoint,
      DistDataReg::getInfo(Point::BasicType(), DDATA_NATIVE),
      DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_SPECIALPOINTS, DFUN_SPECIALPOINTS_DESCR,
        specialPoints,
        DistDataReg::getInfo(Points::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
        EuclideanHPoint,
        DistDataReg::getInfo("hpoint", DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_EDIT_DIST, DFUN_EDIT_DIST_DESCR,
        EditDistance,
        DistDataReg::getInfo(CcString::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR, 
        symTrajDistance1<stj::MLabel>,
        DistDataReg::getInfo(stj::MLabel::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR, 
        symTrajDistance1<stj::MLabels>,
        DistDataReg::getInfo(stj::MLabels::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR, 
        symTrajDistance1<stj::MPlace>,
        DistDataReg::getInfo(stj::MPlace::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR, 
        symTrajDistance1<stj::MPlaces>,
        DistDataReg::getInfo(stj::MPlaces::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    PictureFuns::initDistfuns();

    initialized = true;
}
