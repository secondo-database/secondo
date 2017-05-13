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
#include <cmath>
#include <fstream>



static double l_2(int x1, int y1, int x2, int y2) // euclidean ground distance
{
    return sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
}


static double f_s(int x1, int y1, int x2, int y2)
{
    return 1.0 / (1.0 + l_2(x1, y1, x2, y2));
}


static double sqfd(std::vector<ImageSignatureTuple> i1, 
					std::vector<ImageSignatureTuple> i2)
{
    
    int width = i1.size() + i2.size();    
    double* arr1 = new double[width + 1];
    
    for (unsigned int i = 0; i < i1.size(); i++)
        arr1[i] = i1.at(i).weight;
    
    for (int i = i1.size(); i < width; i++)
    {    
        arr1[i] = (-1.0) * i2.at(i - i1.size()).weight;
    }     
    
    double** mat = new double*[width]; 
       
    for (int i = 0; i < width; i++)
        mat[i] = new double[width];
    
    std::vector<ImageSignatureTuple> ist;
    
    for (auto i : i1)
        ist.push_back(i);
    for (auto i: i2)
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

    double* resMat = new double[width];

    for (int x = 0; x < width; x++)
    {
        resMat[x] = 0.0;
    }   
    
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < width; y++)
        {
            resMat[x] += (arr1[y] * (double)mat[y][x]);
        }
    }

    double distance = 0.0;
    for (int x = 0; x < width; x++)
    {
        distance += (arr1[x] * resMat[x]);
    }
    
    delete[] resMat;
    delete[] mat;
    delete[] arr1;
    
    return sqrt(distance);
}

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "usage: ./SQFD <signature.file> <another_signature.file>";
        return 0;
    }
    std::vector<ImageSignatureTuple> ist1;
    std::vector<ImageSignatureTuple> ist2;
    
    std::cout << argv[1] << " " << argv[2] << std::endl;
        

    int x, y;
    double w;
    
    std::ifstream infile(argv[1]);
    std::string std;
    while (infile >> x >> y >> w)
    {        
        ist1.push_back({x, y, w});
    }
    
    
    std::ifstream infile2(argv[2]);
    while (infile2 >> x >> y >> w)
    {
        ist2.push_back({x, y, w});
    }
    
    
    
    std::cout << "sqfd:" << sqfd(ist1, ist2) << std::endl;
    
    return 0;
}

