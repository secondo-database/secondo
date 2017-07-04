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

//paragraph [1] title: [{\Large \bf ]   [}]

April 2017 Michael Loris


[1] Standalone programm to test the Signature Quadratic From Distance

*/



#include "../JPEGImage.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <complex>
#include <limits>


double euclidDist(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
	double tmpRes = 0.0;
		
	tmpRes += pow((int)ist1.centroid.x - (int)ist2.centroid.x, 2);
	tmpRes += pow((int)ist1.centroid.y - (int)ist2.centroid.y, 2);
	tmpRes += pow((double)ist1.centroid.colorValue1 
    - (double)ist2.centroid.colorValue1, 2);
	tmpRes += pow((double)ist1.centroid.colorValue2 
    - (double)ist2.centroid.colorValue2, 2);
	tmpRes += pow((double)ist1.centroid.colorValue3 
    - (double)ist2.centroid.colorValue3, 2);
	tmpRes += pow((double)ist1.centroid.coarseness 
    - (double)ist2.centroid.coarseness, 2);
	tmpRes += pow((double)ist1.centroid.contrast 
    - (double)ist2.centroid.contrast, 2);
	return sqrt(tmpRes);	
}

double euclidDistSquared(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
	double tmpRes = 0.0;
		
	tmpRes += pow((int)ist1.centroid.x - (int)ist2.centroid.x, 2);
	tmpRes += pow((int)ist1.centroid.y - (int)ist2.centroid.y, 2);
	tmpRes += pow((double)ist1.centroid.colorValue1 
    - (double)ist2.centroid.colorValue1, 2);
	tmpRes += pow((double)ist1.centroid.colorValue2 
    - (double)ist2.centroid.colorValue2, 2);
	tmpRes += pow((double)ist1.centroid.colorValue3 
    - (double)ist2.centroid.colorValue3, 2);
	tmpRes += pow((double)ist1.centroid.coarseness 
    - (double)ist2.centroid.coarseness, 2);
	tmpRes += pow((double)ist1.centroid.contrast 
    - (double)ist2.centroid.contrast, 2);
	return tmpRes;	
}



double f_s(FeatureSignatureTuple fst1,
                    FeatureSignatureTuple fst2)
{
	double dist = euclidDist(fst1, fst2);
	return -dist;
}




//double f_s(FeatureSignatureTuple fst1,
//                    FeatureSignatureTuple fst2)
//{
    //const double sigma = 2.0;
//    const double alpha = 2.0;
//    double euDist = euclidDistSquared(fst1, fst2);
//    double expo = -alpha * euDist;
    //std::cout << "expo:" << expo << std::endl;
//    double res  = exp(expo);
    //std::cout << "res:" << res << std::endl;
//    return res;
//}


//double f_s2(FeatureSignatureTuple fst1,
//                    FeatureSignatureTuple fst2)
//{
//	const double alpha = 2.0;
//	double eucDist = euclidDist(fst1, fst2);		
	
//	if (eucDist == 0.0)
//		return 1.0;
		
//	double res = 1.0 /  (alpha * eucDist);
	//std::cout << "res:" << res << std::endl;	
//	return res;
//}


static double sqfd(std::vector<FeatureSignatureTuple> fst1,
                    std::vector<FeatureSignatureTuple> fst2)
{

    // scale weights
    double minW = 0.0;
    double maxW = 0.0;
    const double a = 0.0;
    const double b = 1.0; 
    
    for (int i = 0; i < fst1.size(); i++)
    {
        if (fst1.at(i).weight > maxW)
            maxW = fst1.at(i).weight;
        if (fst1.at(i).weight <= minW)
            minW = fst1.at(i).weight;
    }
    
    for (int i = 0; i < fst1.size(); i++)
    {
        fst1.at(i).weight 
        = (((b - a) * (fst1.at(i).weight - minW)) / (maxW - minW)) + a;
    }
    
    minW = 0.0;
    maxW = 0.0;
    
    for (int i = 0; i < fst2.size(); i++)
    {
        if (fst2.at(i).weight > maxW)
            maxW = fst2.at(i).weight;
        if (fst2.at(i).weight <= minW)
            minW = fst2.at(i).weight;
    }
    
     for (int i = 0; i < fst2.size(); i++)
    {
        fst2.at(i).weight 
        = (((b - a) * (fst2.at(i).weight - minW)) / (maxW - minW)) + a;
    }
   

    int width = fst1.size() + fst2.size();    

	// build up weight vector
    double* arr1 = new double[width];
    for (unsigned int i = 0; i < fst1.size(); i++)
    {
        arr1[i] = fst1.at(i).weight;
        std::cout << "arr1:" << arr1[i] << std::endl;
    }
    for (int i = fst1.size(); i < width; i++)
    {    
        arr1[i] = (-1.0) * fst2.at(i - fst1.size()).weight;
        std::cout << "arr 1:" << arr1[i] << std::endl;
    }     
        
    
    // init distance matrix
    double** mat = new double*[width]; 
       
    for (int i = 0; i < width; i++)
        mat[i] = new double[width];
    
    
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
			mat[y][x] = tmpDist;
           // std::cout << mat[y][x] << "|";            
        }
       // std::cout << std::endl;
    }
    
    // scale distance matrix
    double minD = 0.0;
    double maxD = 0.0;
    
    for (int y = 0; y < width; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (mat[y][x] > maxD)
                maxD = mat[y][x];
            if (mat[y][x] <= minD)
                minD = mat[y][x];
        }
    }
    /*
    std::cout << "scaled distances" << std::endl;  
    for (int y = 0; y < width; y++)
    {
        for (int x = 0; x < width; x++)
        {
            mat[y][x] = (((b-a) * (mat[y][x] - minD)) / (maxD - minD)) + a;
            std::cout << mat[y][x] << "|";
        }
        std::cout << std::endl;
    }
      */  
    

    // init temporary matrix (weight vector * mat)
    double* resMat = new double[width];
    for (int x = 0; x < width; x++)
    {
        resMat[x] = 0.0;
    }   
    
    
    // multiply weight vector with dist matrix
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < width; y++)
        {
			resMat[x] += arr1[y] * mat[y][x]; 
			std::cout << "resMat:" << resMat[x] << "|";
        }
        std::cout << std::endl;
    }


	// multiply temporary matrix with transposed weight vector
	// 
    double distance = 0.0;
    for (int x = 0; x < width; x++)
    {
        distance += resMat[x] * arr1[x];         
    }
    
    if (distance < 0)
		std::cout << "here's the problematic distance:" 
        << distance << std::endl;
    
    delete[] resMat;
    delete[] mat;
    delete[] arr1;
    
    // return square root
    std::cout << "distance:" << distance << std::endl;
    return sqrt(distance);
        
}

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout 
        << "usage: ./SQFD <signature.file> <another_signature.file>";
        return 0;
    }
    std::vector<FeatureSignatureTuple> ist1;
    std::vector<FeatureSignatureTuple> ist2;
    
    std::cout << argv[1] << " " << argv[2] << std::endl;
        

	double w;
    int x, y;    
    double r,g,b;
    double coa, con;
    
    std::ifstream infile(argv[1]);
    std::string std;
    int counter = 0;
    while (infile >> w >> x >> y >> r >> g >> b >> coa >> con)
    {        	
		ist1.push_back({w, x, y, r, g, b, coa, con});	
    }
    
    counter = 0;
    std::ifstream infile2(argv[2]);
    while (infile2 >> w >> x >> y >> r >> g >> b >> coa >> con)
    {	
		ist2.push_back({w, x, y, r, g, b, coa, con});	
    }
    
       
    std::cout << "sqf distance:" << sqfd(ist1, ist2) << std::endl;
    
    return 0;
}

