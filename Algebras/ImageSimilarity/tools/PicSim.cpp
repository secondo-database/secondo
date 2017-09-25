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


[1] Standalone program to extract feature signatures from a jpeg image

*/


#include<iostream>
#include<stdio.h>
#include "jpeglib.h"
#include<cstring>
#include<cstdint>
#include "../JPEGImage.h"
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#define DIMENSIONS 7

using namespace std;


/*
1 Usage

This program expects parametes for the input file, the name of the
output file, the color space to be used (1 = HSV, 2 = RGB, 3 = Lab),
the size of the area that will be considered when calculating coarseness
and contrast, the number of samples of an image to be used (percentage)
and the number of clusters that the k-means algorithm shall produce. 


*/ 
 
int main(int argc, char* argv[])
{
    
    if (argc != 9)
    {
        std::cout << "usage: PicSim <infile> <outfile> colorSpace" 
		<<" coaRange conRange patchSize percentSamples noClusters" 
        << std::endl;
        return 0;
    }
    
    std::string fileName = argv[1];
    char* vFile = new char[fileName.size() + 1];
    copy(fileName.begin(), fileName.end(), vFile);
    vFile[fileName.size()] = '\0';
   
    std::string outfile = argv[2];
    const char* out_file = outfile.c_str();
    const char* out_file2 = "a_grayscale_outfile.jpg";
    const char* out_file3 = "a_coarseness_outfile.jpg";
    const char* out_file4 = "a_contrast_outfile.jpg";
    const char* out_file5 = "a_clustered_outfile.jpg";
 
    unsigned int colorSpace = atoi(argv[3]);
    unsigned int coaRange = atoi(argv[4]);
    unsigned int conRange = atoi(argv[5]);
    unsigned int patchSize = atoi(argv[6]);
    unsigned int percentSamples = atoi(argv[7]);
    unsigned int noClusters = atoi(argv[8]);
    
    std::cout << "colorSpace:" << colorSpace << std::endl;
    std::cout << "coaRange:" << coaRange << std::endl;
    std::cout << "conRange:" << conRange << std::endl;
    std::cout << "patchSize:" << patchSize << std::endl;
    std::cout << "percentSamples:" << percentSamples << std::endl;
    std::cout << "noClusters:" << noClusters << std::endl;
    
 
    JPEGImage ji;
    ji.importJPEGFile(vFile, colorSpace, coaRange, conRange, 
                        patchSize, percentSamples, noClusters);

    std::cout << "writing color image" << std::endl;
    ji.writeColorImage(out_file);
    
    std::cout << "writing grayscale image" << std::endl;
    ji.writeGrayscaleImage(out_file2);

    std::cout << "computing coarseness" << std::endl;
    ji.computeCoarsenessValues(coaRange);
 
    std::cout << "writing coarseness file" << std::endl;
    ji.writeCoarsenessImage(out_file3, 1.0);

    std::cout << "computing contrasts:" << std::endl;
    ji.computeContrastValues(conRange);


    std::cout << "writing contrast file" << std::endl;
    ji.writeContrastImage(out_file4, 1.0);

    unsigned int noDataPoints 
		= (unsigned int) static_cast<double>(ji.width * ji.height) 
		/ static_cast<double>(percentSamples);
    
    std::cout << "no data points:" << noDataPoints << std::endl;
    std::cout << "no pixels:" << ji.width * ji.height << std::endl;
    int dimensions = 7;
    //noDataPoints *= 100;    
    noDataPoints = ji.getNoDataPoints();
    ji.clusterFeatures(noClusters, dimensions, noDataPoints);
    std::cout << "clustering finished" << std::endl;    
    
    ji.writeClusterImage(out_file5, 500.0);

    std::cout << "done" << std::endl;
    
    for (auto sig : ji.signature)
		std::cout << sig.weight 
		<< " " << sig.centroid.x 
		<< " " << sig.centroid.y 
		<< " " << sig.centroid.colorValue1 
		<< " " << sig.centroid.colorValue2
		<< " " << sig.centroid.colorValue3
		<< " " << sig.centroid.coarseness 
		<< " " << sig.centroid.contrast
		<< std::endl;
		


   delete[] vFile;

    return 0;
}



