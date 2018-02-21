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
#include "Algebras/Spatial/Coord.h"
#include "Algebras/Spatial/Point.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/SymbolicTrajectory/Algorithms.h"
#include "PictureFuns.h"

#ifndef NO_MP3
//---------------cru----------------
#include "Algebras/MP3b/FVector.h"
//----------------------------------
#endif

#ifndef NO_IMAGESIMILARITY
#include "../ImageSimilarity/JPEGImage.h"
#include "../ImageSimilarity/ImageSimilarityAlgebra.h"
#include <vector>
#include <math.h>
#include <iostream>
#include "ImageSimFuns.h"
#include "ImageSimFuns2.h"
#include <chrono>
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



/*
Method ~DistfunReg::sqfdImageSignature~:

*/
#ifndef NO_IMAGESIMILARITY
void DistfunReg::sqfdFeatureSignature(
    const DistData* data1, 
        const DistData* data2,
    double &result)
{   
    //std::cout << "entered distance fun" << std::endl;
     
  if(data1->size() == 0 && data2->size() == 0)
    {
        result = 0;
        return ;
    }
    
    if (data1->size() == 0 || data2->size() == 0) 
    {
        result = numeric_limits<double>::max();
        return;
    }
    
    std::vector<FeatureSignatureTuple> fst1;
    std::vector<FeatureSignatureTuple> fst2;      
    
    //unsigned int d = 0;
    size_t o1 = 0;
    
    //std::cout << "parameter 1" << std::endl;
    
    while (o1 < data1->size())
    {
        FeatureSignatureTuple fst;
        memcpy(&fst,(char*)data1->value() + o1, sizeof(FeatureSignatureTuple));
        o1 += sizeof(FeatureSignatureTuple); 
        fst1.push_back(fst);
    }
    
    
    size_t o2 = 0;
    while (o2 < data2->size())
    {
        FeatureSignatureTuple fst;
        memcpy(&fst,(char*)data2->value() + o2, sizeof(FeatureSignatureTuple));
        o2 += sizeof(FeatureSignatureTuple); 
        fst2.push_back(fst);
    }
    
    /*
    while (o1 < data1->size())
    {
        double weight;
        int x, y;
        double r, g, b;
        double coa, con;
        memcpy(&weight,(char*)data1->value() + o1, sizeof(double));
        o1 += sizeof(double);
        memcpy(&x,(char*)data1->value() + o1, sizeof(int));
        o1 += sizeof(int);
        memcpy(&y,(char*)data1->value() + o1, sizeof(int));
        o1 += sizeof(int);
        memcpy(&r,(double*)data1->value() + o1, sizeof(double));
        o1 += sizeof(double);
        memcpy(&g,(char*)data1->value() + o1, sizeof(double));
        o1 += sizeof(double);
        memcpy(&b,(char*)data1->value() + o1, sizeof(double));
        o1 += sizeof(double);
        memcpy(&coa,(double*)data1->value() + o1, sizeof(double));
        o1 += sizeof(double);
        memcpy(&con,(char*)data1->value() + o1, sizeof(double));
        o1 += sizeof(double);
        
        FeatureSignatureTuple fst = {weight, x, y, r, g, b, coa, con};
        
        
        std::cout << fst.weight << "   " << fst.centroid.x 
        << " " << fst.centroid.y << " " 
        << fst.centroid.colorValue1 
        << " " 
        << fst.centroid.colorValue2 
        << " " << fst.centroid.colorValue3 
        << " " << fst.centroid.coarseness << " " 
        << fst.centroid.contrast << std::endl;
        
        
        fst1.push_back(fst);
    //d += o1;
    }
    */
    
   // std::cout << "data1 size: " << data1->size() << " o1:" << o1 << std::endl;
    
    
   //  std::cout << "parameter 2" << std::endl;
   /*
    size_t o2 = 0;
    while (o2 < data2->size())
    {
        double weight;
        int x, y;
        double r, g, b;
        double coa, con;
        memcpy(&weight,(char*)data2->value() + o2, sizeof(double));
        o2 += sizeof(double);
        memcpy(&x,(char*)data2->value() + o2, sizeof(int));
        o2 += sizeof(int);
        memcpy(&y,(char*)data2->value() + o2, sizeof(int));
        o2 += sizeof(int);
        memcpy(&r,(char*)data2->value() + o2, sizeof(double));
        o2 += sizeof(double);
        memcpy(&g,(char*)data2->value() + o2, sizeof(double));
        o2 += sizeof(double);
        memcpy(&b,(char*)data2->value() + o2, sizeof(double));
        o2 += sizeof(double);
        memcpy(&coa,(char*)data2->value() + o2, sizeof(double));
        o2 += sizeof(double);
        memcpy(&con,(char*)data2->value() + o2, sizeof(double));
        o2 += sizeof(double);
        
        FeatureSignatureTuple fst = {weight, x, y, r, g, b, coa, con};
        
        std::cout << fst.weight << "   " << fst.centroid.x 
        << " " << fst.centroid.y << " " << fst.centroid.colorValue1 << " " 
        << fst.centroid.colorValue2 << " " << fst.centroid.colorValue3 
        << " " << fst.centroid.coarseness << " " 
        << fst.centroid.contrast << std::endl;
        
        
        fst2.push_back(fst);
    //d += o2;
    }
    */
    //std::cout << "data2 size: " << data2->size() << " o2:" << o2 << std::endl;
    
    
    int width = fst1.size() + fst2.size();    

  // build up weight vector
    long* arr1 = new long[width];
    for (unsigned int i = 0; i < fst1.size(); i++)
    {
        //arr1[i] = fst1.at(i).weight;
        arr1[i] = round(fst1.at(i).weight * 100000);
    }
    
    for (int i = fst1.size(); i < width; i++)
    {    
      //arr1[i] = (-1.0) * fst2.at(i - fst1.size()).weight;
      //arr1[i] = -floor(fst2.at(i - fst1.size()).weight / scale + 0.5) * scale;
        arr1[i] = -round(fst2.at(i - fst1.size()).weight * 100000);
    }     
        
    
    // init distance matrix
    long** mat = new long*[width]; 
       
    for (int i = 0; i < width; i++)
        mat[i] = new long[width];
    
    
    // fill vector with features to be compared
    std::vector<FeatureSignatureTuple> ist;
    
    for (auto i : fst1)
        ist.push_back(i);
        
    for (auto i: fst2)
        ist.push_back(i);
        
        
    // calculate distance matrix
    //std::cout << "distance matrix:" << std::endl;
    for (int y = 0; y < width; y++)
    {
        for (int x = 0; x < width; x++)
        {   
      double tmpDist =  f_s(ist.at(y), ist.at(x));        
      //mat[y][x] = tmpDist;
            //mat[y][x] = floor(tmpDist / scale + 0.5) * scale;
            mat[y][x] = round(tmpDist * 100000);
            //std::cout << mat[y][x] << "|";            
        }
        //std::cout << std::endl;
    }

    // init temporary matrix (weight vector * mat)
    long* resMat = new long[width];
    for (int x = 0; x < width; x++)
    {
        resMat[x] = 0.0;
    }   
    
    
    // multiply weight vector with dist matrix
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < width; y++)
        {
            //double tmpProduct = std::abs(arr1[y]) * std::abs(mat[y][x]);
            //if (arr1[y] < 0.0 || mat[y][x] < 0.0)
            //    tmpProduct = -tmpProduct;
      //resMat[x] += tmpProduct; 
            double tmp = std::abs(arr1[y]) * std::abs(mat[y][x]); 
      if (!(arr1[y] > 0 && mat[y][x] > 0))
      {
        tmp = -tmp;
      }
      resMat[x] += tmp; //arr1[y] * mat[y][x];
      //std::cout << "resMat:" << resMat[x] << "|";                       
        }
        //std::cout << std::endl;
    }


  // multiply temporary matrix with transposed weight vector
  // 
    long distance = 0;
    for (int x = 0; x < width; x++)
    {
        //double tmpProduct = std::abs(resMat[x]) * std::abs(arr1[x]);
        //if (resMat[x] < 0.0 || arr1[x] < 0.0)
        //    tmpProduct = -tmpProduct;
        distance += resMat[x] * arr1[x];
        //distance += (long)round(tmpProduct);         
    }
    
    
    delete[] resMat;
    for (int i = 0; i < width; i++)
    delete [] mat[i];
    
    delete[] mat;
    delete[] arr1;
    
    // return square root
    
    result =  sqrt(distance/100000);
    
    //std::cout << "result:" << result << std::endl;
    
    return;

}    


/*
Earth Mover's distance function for FeatureSignatures

*/


void DistfunReg::emdFeatureSignature(
    const DistData* data1, 
    const DistData* data2,
    double &result)
{
   if(data1->size() == 0 && data2->size() == 0)
     {
        result = 0;
        return ;
     }
     
     if(data1->size() == 0 || data2->size() == 0)
     {
        result = numeric_limits<double>::max(); 
        return;
     }
    
    std::vector<FeatureSignatureTuple> fst1;
    std::vector<FeatureSignatureTuple> fst2;
    
    size_t o1 = 0;    
    while (o1 < data1->size())
    {
        FeatureSignatureTuple fst;
        memcpy(&fst,(char*)data1->value() + o1, sizeof(FeatureSignatureTuple));
        o1 += sizeof(FeatureSignatureTuple); 
        fst1.push_back(fst);
    }
    
    size_t o2 = 0;
    while (o2 < data2->size())
    {
        FeatureSignatureTuple fst;
        memcpy(&fst,(char*)data2->value() + o2, sizeof(FeatureSignatureTuple));
        o2 += sizeof(FeatureSignatureTuple); 
        fst2.push_back(fst);
    }
    
  
    TransportProblem tp;
     
    tp.initialVAM(fst1, fst2);

   
    const int maxIterations = 50; // how often does the algorithm cycle
    int actualIterations = 0; // just a counter
    tp.basicsError = false;
    
   for (int i = 0; i < maxIterations; i++)
    {
        actualIterations++;
        
        if (tp.currentDistance == 0)
        {
            break;
        }
        
        
        try
        {
            tp.calcShadowCosts();
            tp.visitedShadowCosts = true;
        
      
            if (tp.enteringCell.val > -1)
            {
                break; // solution is already optimal
            }
        
        
            tp.visitedSteppingStones = true;
            tp.findSteppingStones();
            
            
            
            if (!tp.basicsError)
            {
                tp.visitedUpdateSolution = true;
        tp.updateSolution();
      }
      else
      {
        break;
      }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << '\n';
        }
    
     if (tp.currentDistance > tp.newDistance)
    {
      tp.currentDistance = tp.newDistance;      
    }
    else
    {
      break; // end here currentDistance is the smallest distance
    }    
  }
  
    
    result = tp.currentDistance;
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
        sqfdFeatureSignature,
        DistDataReg::getInfo(
        FeatureSignaturealg::
        FeatureSignature::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));


  addInfo(DistfunInfo(
       DFUN_EMD, DFUN_EMD_DESCR,
       emdFeatureSignature,
        DistDataReg::getInfo(
        FeatureSignaturealg::
        FeatureSignature::BasicType(), DDATA_NATIVE),
        DFUN_IS_METRIC | DFUN_IS_DEFAULT));
#endif


    PictureFuns::initDistfuns();

    initialized = true;
}
