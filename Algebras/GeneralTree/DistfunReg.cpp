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
#include "GTA_SpatialAttr.h"
#include "Coord.h"
#include "Point.h"
#include "SpatialAlgebra.h"
#include "Algorithms.h"
#include "PictureFuns.h"
#ifndef NO_MP3
//---------------cru----------------
#include "FVector.h"
//----------------------------------
#endif


#ifndef NO_IMAGESIMILARITY

#include "../ImageSimilarity/JPEGImage.h"
#include "../ImageSimilarity/ImageSimilarityAlgebra.h"
#include <vector>
#include <math.h>
#include <iostream>



static double l_2(int x1, int y1, int x2, int y2) // euclidean ground distance
{
    return sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)));
}


static double f_s(int x1, int y1, int x2, int y2)
{
  return 1.0 / (1.0 + l_2(x1, y1, x2, y2));
}


struct Flow
{
    double fromWeight;
    double distance;
    double toWeight;
};



static double euclid(double p, double q)
{
    double res = std::pow((p - q), 2);
    return std::sqrt(res);
}


#endif


using namespace gta;
using namespace std;

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
     result = std::sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
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
        result += std::pow(abs (coords1[i] - coords2[i]), 2);
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

#ifndef NO_MP3
//------------cru--------------------------
/*
Method ~DistfunReg::euclidFVector~:

*/
void DistfunReg::euclidFVector(
    const DistData *data1, const DistData *data2,
    double &result) {
  // handle undefined values
  if (data1->size()==0 && data2->size()==0){
    result = 0;
    return;
  }
  if (data1->size()==0 || data2->size()==0) {
    result = numeric_limits<double>::max();
    return;
  }
  // calculate distance
  size_t dim = data1->size()/sizeof(double);

  double x,y;
    result=0;
  for (unsigned int i=0; i<dim; i++){
    memcpy(&x, (char*)data1->value()+i*sizeof(double), sizeof(double));
    memcpy(&y, (char*)data2->value()+i*sizeof(double), sizeof(double));
    result+=(x-y)*(x-y);
  }
  result= sqrt(result);
}
//------------------------------------------
#endif

#ifndef NO_IMAGESIMILARITY

/*
Method ~DistfunReg::sqfdImageSignature~:

*/

void DistfunReg::sqfdImageSignature(
    const DistData* data1, const DistData* data2,
    double &result)
{    
  if(data1->size() == 0 && data2->size() == 0)
    {
       result = 0;
       return ;
    }
          
  std::vector<ImageSignatureTuple> istVector1;
    std::vector<ImageSignatureTuple> istVector2;    
    size_t offset;
    
    
    
    // init ist vectors 
    offset = 0;
    for (unsigned int i = 0; i < data1->size(); i++)
    {
    ImageSignatureTuple ist = {};
    memcpy(&ist.weight, (char*)data1->value() + offset, 
        sizeof(double));
        offset += sizeof(double);
    memcpy(&ist.centroidXpos, (char*)data1->value() + offset, 
        sizeof(int));
        offset += sizeof(int);
    memcpy(&ist.centroidYpos, (char*)data1->value() + offset, 
        sizeof(int));
        offset += sizeof(int);
    
        istVector1.push_back(ist);
  }
     
    offset = 0;
    for (unsigned int i = 0; i < data2->size(); i++)
    {
    ImageSignatureTuple ist = {};
    memcpy(&ist.weight, (char*)data2->value(), sizeof(double));
        offset += sizeof(double);
    memcpy(&ist.centroidXpos, (char*)data2->value(), sizeof(int));
        offset += sizeof(int);
    memcpy(&ist.centroidYpos, (char*)data2->value(), sizeof(int));
        offset += sizeof(int);
    
        istVector2.push_back(ist);
  }
     
    int width = data1->size() + data2->size();
  
  // init arr1
  double* arr1 = new double[width + 1];
  
  // fill arr1
  for (unsigned int i = 0; i < istVector1.size(); i++)
    arr1[i] = istVector1.at(i).weight;
  
  for (int i = istVector1.size(); i < width; i++)
  {  
    arr1[i] = (-1.0) * istVector2.at(i - istVector1.size()).weight;
  }   
    
  // set up matrix
  double** mat = new double*[width]; 
     
  for (int i = 0; i < width; i++)
    mat[i] = new double[width];
  
  std::vector<ImageSignatureTuple> ist; 
  
  for (auto i : istVector1)
    ist.push_back(i);
  for (auto i: istVector2)
    ist.push_back(i);
      
  int i;
  int j;
  for (int y = 0; y < width; y++)
  {
    for (int x = 0; x < width; x++)
    {
      i = y;
      j = x;      
      mat[y][x] = f_s(ist.at(i).centroidXpos,
              ist.at(i).centroidYpos,  
              ist.at(j).centroidXpos,
              ist.at(j).centroidYpos);      
    }    
  }
  
  // 1. multiply array arr1 with A_f_s
  // init result matrix / array
  double* resMat = new double[width];
  for (int x = 0; x < width; x++)
  {
    resMat[x] = 0.0;
  }   
  
  // multiply arr1 with mat
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < width; y++)
    {    
      resMat[x] += (arr1[y] * (double)mat[y][x]);
    }  
  }      
  
  // 2. multiply mat with arr`T
  double distance = 0.0;
  for (int x = 0; x < width; x++)
  {
    distance += (arr1[x] * resMat[x]);
  }
  
  delete[] resMat;
  delete arr1;          
    for (int i = 0; i < width; i++)
    delete mat[i];    
    delete [] mat;
    
  result = sqrt(distance);
  return;
}    


/*
Earth Mover's distance function for ImageSignatures

*/


void DistfunReg::emdImageSignature(
    const DistData* data1, 
    const DistData* data2,
    double &result)
{
   if(data1->size() == 0 && data2->size() == 0){
        result = 0;
        return ;
     }
     
     if(data1->size() == 0 || data2->size() == 0){
        result = numeric_limits<double>::max(); 
        return;
     }
          
    std::vector<ImageSignatureTuple> ist1;
    std::vector<ImageSignatureTuple> ist2;
    
    size_t offset;
    
    // init ist vectors 
    offset = 0;
    for (unsigned int i = 0; i < data1->size(); i++)
    {
    ImageSignatureTuple ist = {};
    memcpy(&ist, data1->value(), sizeof(ist));
    ist1.push_back(ist);
    offset += sizeof(ist);
  }
     
    offset = 0;
    for (unsigned int i = 0; i < data2->size(); i++) 
    {
    ImageSignatureTuple ist = {};
    memcpy(&ist, data2->value(), sizeof(ist));
    ist2.push_back(ist);
    offset += sizeof(ist);
  }
    
      
    std::vector<ImageSignatureTuple> p;
    std::vector<ImageSignatureTuple> q;

  if (ist1.size() > ist2.size())
  {
    p = ist1;
    q = ist2;
  }
  else
  {
    p = ist2;
    q = ist1;
  }
    
    double** distMat = new double*[q.size()];
    for (unsigned int i = 0; i < q.size(); i++)
    distMat[i] = new double[p.size()];
   
    // fill distance matrix
    for (unsigned int i = 0; i < q.size(); i++) // rows
    {
    for (unsigned int j = 0; j < p.size(); j++) // columns
    {
      distMat[i][j] = euclid(q.at(i).weight, p.at(j).weight);           
    }  
    }

    // side preliminaries
    // 1. indices start with 1
    // 2. no negative signs
    double cost =  0.0;
    unsigned int i = 0;
    unsigned int j = 0;
  
  
    // start in top left corner
    // supply and demand are equal -> no costs, move one down
    // supply < demand -> move one down,
    // supply > demand -> move one right, adjust supply(demand - supply)    
  while (i < q.size() && j < p.size())
  {
    if (p.at(j).weight > q.at(i).weight) // supply higher
      {      
      p.at(j).weight -= q.at(i).weight;
        cost += distMat[i][j] * q.at(i).weight;
      q.at(i).weight = 0;
          i++;
      }
      else if (p.at(j).weight < q.at(i).weight) // demand higher
      {    
      q.at(i).weight -= p.at(j).weight;
        cost += distMat[i][j] * q.at(j).weight;
      p.at(j).weight = 0;
          j++;
      }
      else
      {    
        distMat[i][j] *= q.at(i).weight; 
      p.at(j).weight = 0;
      q.at(i).weight = 0;
          j++;
      }
  }
      
    for (unsigned int i = 0; i < q.size(); i++)
    delete [] distMat[i];
    
    delete [] distMat;
    
  result = cost;
    return;
}

#endif



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

//     addInfo(DistfunInfo(
//         DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR,
//         symTrajDistance1<stj::MLabel>,
//         DistDataReg::getInfo(stj::MLabel::BasicType(), DDATA_NATIVE),
//         DFUN_IS_METRIC | DFUN_IS_DEFAULT));

    addInfo(DistfunInfo(
        DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR,
        symTrajDistance1<stj::MLabels>,
        DistDataReg::getInfo(stj::MLabels::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));

//     addInfo(DistfunInfo(
//         DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR,
//         symTrajDistance1<stj::MPlace>,
//         DistDataReg::getInfo(stj::MPlace::BasicType(), DDATA_NATIVE),
//         DFUN_IS_METRIC | DFUN_IS_DEFAULT));
//
//     addInfo(DistfunInfo(
//         DFUN_SYMTRAJ_DIST1, DFUN_SYMTRAJ_DIST1_DESCR,
//         symTrajDistance1<stj::MPlaces>,
//         DistDataReg::getInfo(stj::MPlaces::BasicType(), DDATA_NATIVE),
//         DFUN_IS_METRIC | DFUN_IS_DEFAULT));

#ifndef NO_MP3
//----------------------cru--------------------------
    addInfo(DistfunInfo(
        DFUN_EUCLID, DFUN_EUCLID_DESCR,
      euclidFVector,
      DistDataReg::getInfo(FVector::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));
//---------------------------------------------------
#endif


#ifndef NO_IMAGESIMILARITY
  addInfo(DistfunInfo(
        DFUN_SQFD, DFUN_SQFD_DESCR,
        sqfdImageSignature,
        DistDataReg::getInfo(
        ImageSignaturealg::ImageSignature::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));


  addInfo(DistfunInfo(
       DFUN_EMD, DFUN_EMD_DESCR,
       emdImageSignature,
        DistDataReg::getInfo(
        ImageSignaturealg::ImageSignature::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));
#endif


    PictureFuns::initDistfuns();

    initialized = true;
}
