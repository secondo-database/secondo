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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Image Similarity Algebra

March 2017 Michael Loris

The Image Similarity Algebra for SECONDO provides operations to store 
pictures in JPEG format into m-trees. It consists a ~Signature~ object 
representing a the signature of a single JPEG image. The object is a 
subtype of the Attribute class. The Image Similarity Algebra consists of 
multiple files, including external files for k-means clustering.

*/

#include "JPEGImage.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <complex>
#include <limits>
#include "SQFDCalculator.h"




static double euclidDist(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
    double tmpRes = 0.0;
        
    tmpRes += pow((int)ist1.centroid.x - (int)ist2.centroid.x, 2);
    tmpRes += pow((int)ist1.centroid.y - (int)ist2.centroid.y, 2);
    tmpRes += pow(ist1.centroid.colorValue1 
    - ist2.centroid.colorValue1, 2);
    tmpRes += pow(ist1.centroid.colorValue2 
    - ist2.centroid.colorValue2, 2);
    tmpRes += pow(ist1.centroid.colorValue3 
    - ist2.centroid.colorValue3, 2);
    tmpRes += pow((double)ist1.centroid.coarseness 
    - (double)ist2.centroid.coarseness, 2);
    tmpRes += pow((double)ist1.centroid.contrast 
    - (double)ist2.centroid.contrast, 2);
    return sqrt(tmpRes);    
}


// alterlative similiarity functions


//double f_gaussian(FeatureSignatureTuple fst1,
//                    FeatureSignatureTuple fst2)
//{
//    const double alpha = 2.0;
//    double euDist = euclidDistSquared(fst1, fst2);
//    double expo = -alpha * euDist;
//    std::cout << "expo:" << expo << std::endl;
//    double res  = exp((double)expo);
//    std::cout << "res:" << res << std::endl;
//    return res;
//}

static double f_s(FeatureSignatureTuple fst1,
                    FeatureSignatureTuple fst2)
{
    const double alpha = 2.0;
    double eucDist = euclidDist(fst1, fst2);        
    double res = 1.0 /  (alpha + eucDist);
    return res;
}


double SQFDCalculator::calcSQFD(std::vector<FeatureSignatureTuple> fst1,
                    std::vector<FeatureSignatureTuple> fst2)
{
        
    //const double scale = 0.01;

    int width = fst1.size() + fst2.size();    

    // build up weight vector
    long* arr1 = new long[width];
    for (unsigned int i = 0; i < fst1.size(); i++)
    {
        //arr1[i] = floor(fst1.at(i).weight / scale + 0.5) * scale;
        arr1[i] = round(fst1.at(i).weight * 100000);
        //std::cout << "arr1:" << arr1[i] << std::endl;
    }
    for (int i = fst1.size(); i < width; i++)
    {    
        arr1[i] = -round(fst2.at(i - fst1.size()).weight * 100000);
        //std::cout << "arr1:" << arr1[i] << std::endl;
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
    //for (int i = 0; i < width; i++)
    //{
    //        std::cout << arr1[i] << "|";
    //}
            
    //std::cout << std::endl;
    
    for (int y = 0; y < width; y++)
    {
        //std::cout << y << ":";
        for (int x = 0; x < width; x++)
        {   
            double tmpDist =  f_s(ist.at(y), ist.at(x));        
            //mat[y][x] = floor(tmpDist / scale + 0.5) * scale;
            mat[y][x] = round(tmpDist * 100000);
          //  std::cout << mat[y][x] << "|";            
        }
        //std::cout << std::endl;        
    }
    
        //std::cout << std::endl;
         //   std::cout << std::endl;
    
    // init temporary matrix (weight vector * mat)
    long* resMat = new long[width];
    for (int x = 0; x < width; x++)
    {
        resMat[x] = 0.0;
    }   
    
    
    // multiply weight vector with dist matrix
    //std::cout << "tmpVector, resMat" << std::endl;
    
    for (int x = 0; x < width; x++)
    {
        
        for (int y = 0; y < width; y++)
        {
            double tmp = abs(arr1[y]) * abs(mat[y][x]); 
            if (!(arr1[y] > 0 && mat[y][x] > 0))
            {
                tmp = -tmp;
            }
            resMat[x] += tmp; //arr1[y] * mat[y][x]; 
        }
        //std::cout << std::endl;
    }

    std::cout << std::endl;
    
    // multiply temporary matrix with transposed weight vector
    // 
    //double distance = 0.0;
    long distance = 0;
    for (int x = 0; x < width; x++)
    {
        distance += resMat[x] * arr1[x];
      //  std::cout << "dist:" << distance << " resMat:" 
      //<< resMat[x] << " * " << arr1[x] << std::endl;         
    }
    
    
    delete[] resMat;
    for (int i = 0; i < width; i++)
        delete [] mat[i];
    
    delete[] mat;
    
    delete[] arr1;
    
    // return square root
    //std::cout << "distance:" << distance << std::endl;
    return sqrt(distance/100000);
        
}
