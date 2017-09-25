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

January 2017 Michael Loris


[1] Declarations for the JPEGImage class

*/

#include "JPEGImage.h"
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <vector>
#include <random>

// includes for clustering of the Baylor machine learning library
#include "fast_kmeans/dataset.h"
#include "fast_kmeans/general_functions.h"
#include "fast_kmeans/hamerly_kmeans.h"
#include "fast_kmeans/annulus_kmeans.h"
#include "fast_kmeans/drake_kmeans.h"
#include "fast_kmeans/naive_kmeans.h"
#include "fast_kmeans/elkan_kmeans.h"
#include "fast_kmeans/compare_kmeans.h"
#include "fast_kmeans/sort_kmeans.h"
//#include "heap_kmeans.h"
//#include "naive_kernel_kmeans.h"
//#include "elkan_kernel_kmeans.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <string>
#include <map>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>


#define DIMENSIONS 7


/*
1.5 Conversion functions, taken from the Picture algebra

*/

namespace conversion {
// taken from picture algebra
Lab::Lab (unsigned char r_, unsigned char g_, unsigned char b_)
{
    double R, G, B;
    double rd = (double) r_ / 255;
    double gd = (double) g_ / 255;
    double bd = (double) b_ / 255;

    if (rd > 0.04045)
        R = std::pow((rd + 0.055) / 1.055, 2.2);
    else
        R = rd / 12.92;

    if (gd > 0.04045)
        G = std::pow ((gd + 0.055) / 1.055, 2.2);
    else
        G = gd / 12.92;

    if (bd > 0.04045)
        B = std::pow ((bd + 0.055) / 1.055, 2.2);
    else
        B = bd / 12.92;

    // compute X,Y,Z coordinates of r,g,b
    double X = 0.4124 * R + 0.3576 * G + 0.1805 * B;
    double Y = 0.2127 * R + 0.7152 * G + 0.0722 * B;
    double Z = 0.0193 * R + 0.1192 * G + 0.9500 * B;

    /* used chromacity coordinates of whitepoint D65:
    x = 0.312713, y = 0.329016

    the respective XYZ coordinates are
    Y = 1,
    X = Y * x / y       = 0.9504492183, and
    Z = Y * (1-x-y) / y = 1.0889166480
    */

    double eps = 0.008856; // = 216 / 24389
    double x = X / 0.95045;
    double y = Y;
    double z = Z / 1.08892;
    long double fx, fy, fz;

    if (x > eps)
        fx = std::pow (x, 0.333333);
    else
        fx = 7.787 * x + 0.137931;

    if (y > eps)
        fy = std::pow (y, 0.333333);
    else
        fy = 7.787 * y + 0.137931;

    if (z > eps)
        fz = std::pow (z, 0.333333);
    else
        fz = 7.787 * z + 0.137931;

    // compute Lab coordinates
    double Lab_Ld = ((116  * fy) - 16);
    double Lab_ad = (500 * (fx - fy));
    double Lab_bd = (200 * (fy - fz));

    L = (signed char) Lab_Ld;
    a = (signed char) Lab_ad;
    b = (signed char) Lab_bd;
}


// taken from picture algebra
HSV::HSV (unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char rgbMin = std::min (std::min (r, g), b);
    unsigned char rgbMax = std::max (std::max (r, g), b);
    unsigned char delta = rgbMax - rgbMin;

    // compute h
    if (delta == 0)
    {
        h = 0;
    }
    else
    {
        if (rgbMax == r)
        {
            h = 60 * (g - b) / delta;
        }
        else if (rgbMax == g)
        {
            h = 120 * (g - b) / delta;
        }
        else // rgbMax == b
        {
            h = 240 * (g - b) / delta;
        }
    }

    if (h < 0)
        h += 360;

    // compute s
    if (rgbMax == 0)
        s = 0;
    else
        s = 255 * delta / rgbMax;

    // compute v
    v = rgbMax;
}

} // end namespace


/*
1.6 Functions required from jpeglib

*/


// required for jpeg import, taken from jpeg6 library 
struct my_error_mgr
{
    struct jpeg_error_mgr pub;    /* "public" fields */
    jmp_buf setjmp_buffer;    /* for return to caller */
};

// required for jpeg import, taken from jpeg6 library 
typedef struct my_error_mgr* my_error_ptr;

// required for jpeg import,taken from jpeg6 library 
void my_error_exit (j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}



/*
1.7 Functions to scale values of features

*/


/*
Scale all values of all dimensions, not just contrast

*/
template<typename A>
double scale(A val, A min, A max, A a, A b)
{
    //double a = -10000.0;
    //double b = 10000.0;
    return (((b - a) * (val - min)) / ( max - min )) + a;
}

void JPEGImage::scalePCTDimensions()
{
    int minX = std::numeric_limits<int>::min();
    int maxX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::min();
    int maxY = std::numeric_limits<int>::max();
    
    double minC1 = std::numeric_limits<double>::min();
    double maxC1 = std::numeric_limits<double>::max();
    double minC2 = std::numeric_limits<double>::min();
    double maxC2 = std::numeric_limits<double>::max();
    double minC3 = std::numeric_limits<double>::min();
    double maxC3 = std::numeric_limits<double>::max();
    double minCoa = std::numeric_limits<double>::min();
    double maxCoa = std::numeric_limits<double>::max();
    double minCon = std::numeric_limits<double>::min();
    double maxCon = std::numeric_limits<double>::max();
    
    for (auto tuple : this->signature)
    {
        if (tuple.centroid.x <= maxX)
            maxX = tuple.centroid.x;
            
        if (tuple.centroid.x >= minX)
            minX = tuple.centroid.x;
            
        if (tuple.centroid.y <= maxY)
            maxY = tuple.centroid.y;
            
        if (tuple.centroid.y >= minY)
            minY = tuple.centroid.y;
        
        
        if (tuple.centroid.colorValue1 <= maxC1)
            maxC1 = tuple.centroid.colorValue1;
            
        if (tuple.centroid.colorValue1 >= minC1)
            minC1 = tuple.centroid.colorValue1;
            
        if (tuple.centroid.colorValue2 <= maxC2)
            maxC2 = tuple.centroid.colorValue2;
            
        if (tuple.centroid.colorValue2 >= minC2)
            minC2 = tuple.centroid.colorValue2;
            
        if (tuple.centroid.colorValue3 <= maxC3)
            maxC3 = tuple.centroid.colorValue3;
            
        if (tuple.centroid.colorValue3 >= minC3)
            minC3 = tuple.centroid.colorValue3;
            
        if (tuple.centroid.coarseness <= maxCoa)
            maxCoa = tuple.centroid.coarseness;
            
        if (tuple.centroid.coarseness >= minCoa)
            minCoa = tuple.centroid.coarseness;
            
        if (tuple.centroid.contrast <= maxCon)
            maxCon = tuple.centroid.contrast;
            
        if (tuple.centroid.contrast >= minCon)
            minCon = tuple.centroid.contrast;
    }
    

    for (auto tuple : this->signature)
    {
        int i1 = scale(tuple.centroid.x, minX, maxX, 0, 10000);
        int i2  = scale(tuple.centroid.y, minY, maxY, 0, 10000);
        
        double d1 
        = scale(tuple.centroid.colorValue1, minC1, maxC1, 0.0, 10000.0);
        double d2 
        = scale(tuple.centroid.colorValue2, minC2, maxC2, 0.0, 10000.0);
        double d3 
        = scale(tuple.centroid.colorValue3, minC3, maxC3, 0.0, 10000.0);
        double d4 
    = scale(tuple.centroid.coarseness, minCoa, maxCoa, 0.0, 10000.0);
        double d5 
        = scale(tuple.centroid.contrast, minCon, maxCon, 0.0, 10000.0);
        
        //std::cout << minX << " " << maxX << " " << minC1 << " " 
        //<< maxC1 << " " << minC2 << " " << maxC2 << " " 
        //<< minC1 << " " << maxC2 
        //<< " " << minCoa << " " << maxCoa << " " 
        //<< minCon << " " << maxCoa << std::endl; 
        
        std::cout << i1 << " " << i2 << " " 
        << d1 << " " << d2 << " " << d3 << " " 
        << d4 << " " << d5 << std::endl;
        
        tuple.centroid.colorValue1 
        = scale(tuple.centroid.x, minX, maxX, 0, 10000);
        tuple.centroid.colorValue2 
        = scale(tuple.centroid.y, minY, maxY, 0, 10000);
        
        tuple.centroid.colorValue1 
        = scale(tuple.centroid.colorValue1, minC1, maxC1, 0.0, 10000.0);
        tuple.centroid.colorValue2 
        = scale(tuple.centroid.colorValue2, minC2, maxC2, 0.0, 10000.0);
        tuple.centroid.colorValue3 
        = scale(tuple.centroid.colorValue3, minC3, maxC3, 0.0, 10000.0);
        tuple.centroid.coarseness 
    = scale(tuple.centroid.coarseness, minCoa, maxCoa, 0.0, 10000.0);
        tuple.centroid.contrast
         = scale(tuple.centroid.contrast, minCon, maxCon, 0.0, 10000.0);
    }
}



/*
 2.0 Destructor for JPEGImage class

*/ 
 
JPEGImage::~JPEGImage() 
{
    delete[] colorValues1;
    delete[] colorValues2;
    delete[] colorValues3;
    delete[] coa;
    delete[] con;
    delete[] weights; // of clusters
    delete[] pixels; 
    
    for (int i = 0; i < this->height; i++)
    {
        for (int j = 0; j < this->width; j++)
        {
            delete [] this->pixMat5[i][j];            
        }
        delete [] this->pixMat5[i];
    }
    delete [] this->pixMat5;
        
    
    for (int i = 0; i < this->height; i++)
    {
        delete [] this->assignments[i];
    }
    delete [] this->assignments;
        
    
    delete [] samplesX;
    delete [] samplesY;    
    //delete [] coarsenesses;
    //delete [] contrasts;
    
 
    
    for (int i = 0; i < this->height; i++)  
    {        
        for (int j = 0; j < this->width; j++)
        {
            delete [] this->pixMat4[i][j];
        }
        delete [] this->pixMat4[i];
    }
    delete [] this->pixMat4;
   
   for (unsigned int i = 0; i < this->clusters->size(); i++)
    {
        this->clusters->at(i).clear();        
    }
    this->clusters->clear();    
    delete this->clusters;
     
};

/*
  3.0 Function to import an jpg file
  
*/


void JPEGImage::importJPEGFile(const std::string _fileName,
    const int colorSpace,
    const int coaRange, 
    const int conRange, 
    const int patchSize, 
    const int percentSamples,
    const int noClusters) 
{
    
    this->fileName = _fileName;

    auto t1 = std::chrono::high_resolution_clock::now();
            
    const char* fileName = _fileName.c_str();
    
    std::cout << "filename:" << fileName << std::endl;
    
    struct jpeg_decompress_struct cinfo; // taken from jpeg library
    struct my_error_mgr jerr; // taken from jpeg library
    FILE * infile; // taken from jpeg library
    int row_stride; // taken from jpeg library

    if ((infile = fopen(fileName, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", fileName);
        return;
    }

    cinfo.err = jpeg_std_error(&jerr.pub); // taken from jpeg library
    jerr.pub.error_exit = my_error_exit; // taken from jpeg library

    if (setjmp(jerr.setjmp_buffer)) { // taken from jpeg library
        jpeg_destroy_decompress(&cinfo); // taken from jpeg library
        fclose(infile); // taken from jpeg library
        return; // taken from jpeg library
    }

    jpeg_create_decompress(&cinfo); // taken from jpeg library
    jpeg_stdio_src(&cinfo, infile); // taken from jpeg library
    (void) jpeg_read_header(&cinfo, TRUE); // taken from jpeg library
    (void) jpeg_start_decompress(&cinfo); // taken from jpeg library

    row_stride = cinfo.output_width * cinfo.output_components;
    this->width = cinfo.output_width;
    this->height = cinfo.output_height;

    if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        this->isGrayscale = true;
    }

    this->pixels = new unsigned char [(cinfo.output_width * 
        cinfo.output_height * 
        cinfo.output_components)]; 

    this->patchSize = patchSize;
    this->noDataPoints = static_cast<double>(percentSamples) / 100.0 
        * (this->width * this->height);
    
    
    this->noSamples = static_cast<unsigned int>(this->noDataPoints 
        / (double)(this->patchSize * this->patchSize));
    
    this->colorSpace = colorSpace;
    
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> randX(0, this->width);
    
    this->samplesX = new int[this->noSamples];  
       
    for (int i = 0; i < this->noSamples; i++)
    {
        int rx = randX(gen);
        this->samplesX[i] = rx;
    }
    
    this->samplesY = new int[this->noSamples];  
    
    std::uniform_int_distribution<> randY(0, this->height);
    for (int i = 0; i < this->noSamples; i++)
    {
        int ry = randY(gen);
        this->samplesY[i] = ry;
    }

    this->pixMat5 = new double**[this->height];  
    for (int i = 0; i < this->height; i++)
    {
        this->pixMat5[i] = new double*[this->width 
            * cinfo.output_components]; 
        for (int j = 0; j < this->width; j++)
        {
            this->pixMat5[i][j] = new double[9];
        }
    }
    
    // this array is used to write images
    // first a white 'canvas is drawn'
    // the the sampled pixels will be written
    this->pixMat4 = new unsigned char**[this->height];
    for (int i = 0; i < this->height; i++)  
    {
        this->pixMat4[i] = new unsigned char*[this->width]; 
        for (int j = 0; j < this->width; j++)
        {
            this->pixMat4[i][j] = new unsigned char[3];
        }
    } 
    

    JSAMPROW output_data;
    unsigned int cnt = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        output_data = (this->pixels + (cnt * row_stride));
        (void) jpeg_read_scanlines(&cinfo, &output_data, 1);
        unsigned int c = 0;
        for (int i = 0; i < this->width; i++)
        {
            if (colorSpace == 1) // HSV
            {
                conversion::HSV hsv = {output_data[c], output_data[c+1], 
                            output_data[c+2]};
                this->pixMat5[cnt][i][0] = static_cast<double>(hsv.h);
                this->pixMat5[cnt][i][1] = static_cast<double>(hsv.s);
                this->pixMat5[cnt][i][2] = static_cast<double>(hsv.v);
            } 
            else if (colorSpace == 2) // RGB 
            {
                this->pixMat5[cnt][i][0] 
                    = static_cast<double>(output_data[c]);
                this->pixMat5[cnt][i][1] 
                    = static_cast<double>(output_data[c+1]);
                this->pixMat5[cnt][i][2] 
                    = static_cast<double>(output_data[c+2]);
            } 
            else // Lab
            {
                conversion::Lab lab = {output_data[c], output_data[c+1], 
                            output_data[c+2]};
                this->pixMat5[cnt][i][0] = lab.L;
                this->pixMat5[cnt][i][1] = lab.a;
                this->pixMat5[cnt][i][2] = lab.b;
            }
            
            // always grab a grayscale image, 
            //as it's needed for the texture features
            // The average method
            //this->pixMat5[cnt][i][6] = (double) (output_data[c] + 
            //    (double)output_data[c+1] + (double)output_data[c+2]) / 3.0;
            
            // emphasis on green 
            //http://docs.opencv.org/3.1.0/de/d25/imgproc_color_conversions.html
            this->pixMat5[cnt][i][6] = 
            static_cast<double>(output_data[c]) * 0.299 
            + static_cast<double>(output_data[c+1]) * 0.587
            + static_cast<double>(output_data[c+2]) * 0.115 
            ;
            
            c += 3;
        }
         cnt++;
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);    
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "import() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";    
}



/*
  3.0 Function to cluster the extracted vectors using the Baylor ML 
  library.
  
*/

// this method uses the Baylor ML code to cluster the 
// selected features using the Hamerly algorithm 
void JPEGImage::clusterFeatures(unsigned int k, unsigned int dimensions, 
    unsigned int noDataPoints) 
{
    auto t1 = std::chrono::high_resolution_clock::now();

    
    // 1. create dataset
    int xcNdx = 0;
    Dataset* ds = new Dataset(this->noDataPoints, DIMENSIONS);
    
    unsigned int cnt = 0;
    
    int countr = 0;
    for (int z = 0; z < this->noSamples; z++)
    {
        if (this->samplesX[z] >= 0 && 
            this->samplesX[z] < this->width &&
            this->samplesY[z] >= 0 &&
            this->samplesY[z] < this->height
            )
        {
            int tmpX = this->samplesX[z];
            int tmpY = this->samplesY[z];
        
            for (int k = 0; k < this->patchSize; k++) 
            {
                for (int n = 0; n < this->patchSize; n++)
                {
                    countr++;
                    if (((tmpY + n) < this->height) && 
                        ((tmpX + k) < this->width))
                    {
                        ds->data[cnt] = (double)tmpX+k;
                        ds->data[cnt+1] = (double)tmpY+n;
                        ds->data[cnt+2] 
                            = this->pixMat5[tmpY+n][tmpX+k][0];
                        ds->data[cnt+3] 
                            = this->pixMat5[tmpY+n][tmpX+k][1];
                        ds->data[cnt+4] 
                            = this->pixMat5[tmpY+n][tmpX+k][2];
                        ds->data[cnt+5] 
                            = this->pixMat5[tmpY+n][tmpX+k][3];
                        ds->data[cnt+6] 
                            = this->pixMat5[tmpY+n][tmpX+k][4];
                        cnt += 7;
                    }
                }
            }
        }
    }

    // 2. init k-means, either random or k++ -> k++ as default :   
    Dataset* c;
    
    //if (k < 100) // there's a bug in kmeans++ for smaller values of k
        c = init_centers(*ds, k);
    //else
    //    c = init_centers_kmeanspp_v2(*ds, k); 

    // cluster number for each point
    unsigned short* assignment = new unsigned short[ds->n]; 
    for (int i = 0; i < ds->n; ++i)
        assignment[i] = 0;

    assign(*ds, *c, assignment);
    
    delete c;
    // 3. setting up parameters
    Kmeans* algorithm = new HamerlyKmeans();

    int numThreads = 0;
    int maxIterations = 20;
    std::vector<int>* numItersHistory = new std::vector<int>;
    std::string command = "annulus";

    // 4. run algorithm
    unsigned short *workingAssignment = new unsigned short[ds->n];
    std::copy(assignment, assignment + ds->n, workingAssignment);
    algorithm->initialize(ds, k, workingAssignment, numThreads);

    // 4 a. running algorithm
    int iterations = algorithm->run(maxIterations);

    // 5. iterations
    while (numItersHistory->size() <= (size_t)xcNdx)
    {
        numItersHistory->push_back(iterations);
    }

    if (iterations != numItersHistory->back()) {
        std::cerr << "ERROR: iterations = " 
        << iterations << " but last iterations was " 
        << numItersHistory->back() << std::endl;
    }

    // 6. return workingAssignment    
    this->assignments = new unsigned short*[this->height]; 
    for (int i = 0; i < this->height; i++)
    {
        this->assignments[i] = new unsigned short[this->width];
    }
    
  
    int cnt2 = 0;
    int l = 0;
    for (int z = 0; z < this->noSamples; z++)
    {
        int tmpX = this->samplesX[z];
        int tmpY = this->samplesY[z];
                        
        for (int kk = 0; kk < this->patchSize; kk++) 
        {
            for (int ll = 0; ll < this->patchSize; ll++) 
            {
                cnt2++;
                if (((tmpY + ll) < this->height) 
                && ((tmpX + kk) < this->width))
                {    
                    this->assignments[tmpY+ll][tmpX+kk] 
                        = workingAssignment[l];
                    l++;
                }
            }
        }
    }
    
    // 7. assigning clusters
    this->clusters = new std::vector<std::vector<Feature>>(k); 
    
    for (unsigned int n = 0; n < k; n++)
    {
        for (int z = 0; z < this->noSamples; z++)
        {
            int tmpX = this->samplesX[z];
            int tmpY = this->samplesY[z];
                        
            for (int kk = 0; kk < this->patchSize; kk++) 
            {
                for (int l = 0; l < this->patchSize; l++) 
                {
                    if (((tmpY + l) < this->height) 
                        && ((tmpX + kk) < this->width))
                    {
                        if (this->assignments[tmpY + l][tmpX + kk] == n)
                        {   
                            Feature f = {tmpY + l, tmpX + kk, 
                            this->pixMat5[tmpY + l][tmpX + kk][0],
                            this->pixMat5[tmpY + l][tmpX + kk][1], 
                            this->pixMat5[tmpY + l][tmpX + kk][2], 
                            this->pixMat5[tmpY + l][tmpX + kk][3], 
                            this->pixMat5[tmpY + l][tmpX + kk][4]};
                            this->clusters->at(n).push_back(f);
                        }    
                    }
                }
            }
        }
    }
    
    // "9. assigning centroids"
    this->centersX = new int[k];  
    this->centersY = new int[k];
    this->colorValues1 = new double[k];
    this->colorValues2 = new double[k];
    this->colorValues3 = new double[k];
    this->coa = new double[k]{}; 
    this->con = new double[k]{};
    
    this->weights = new double[k]{};

    int kk = 0;
    for (unsigned int l = 0; l < this->clusters->size(); l++)  
    {
       double tmpX = 0.0;
       double tmpY = 0.0;
       double tmpColVal1 = 0.0;
       double tmpColVal2 = 0.0;
       double tmpColVal3 = 0.0;
       double tmpCoa = 0.0;
       double tmpCon = 0.0;
       
       for (unsigned int i = 0; i < this->clusters->at(l).size(); i++)
       {
            tmpX += this->clusters->at(l).at(i).x;
            tmpY += this->clusters->at(l).at(i).y;
            tmpColVal1 += this->clusters->at(l).at(i).colorValue1;
            tmpColVal2 += this->clusters->at(l).at(i).colorValue2;
            tmpColVal3 += this->clusters->at(l).at(i).colorValue3;
            tmpCoa += this->clusters->at(l).at(i).coarseness;
            tmpCon += this->clusters->at(l).at(i).contrast;
       }
       
       this->centersX[l]= round(tmpX / this->clusters->at(l).size());  
       this->centersY[l]= round(tmpY / this->clusters->at(l).size());
       this->colorValues1[l] 
       = round(tmpColVal1 / this->clusters->at(l).size());  
       this->colorValues2[l] 
       = round(tmpColVal2 / this->clusters->at(l).size());
       this->colorValues3[l] 
       = round(tmpColVal3 / this->clusters->at(l).size());
       this->coa[l] = round(tmpCoa / this->clusters->at(l).size());
       this->con[l] = round(tmpCon / this->clusters->at(l).size());
       
       this->weights[l] = (double)this->clusters->at(l).size() 
                        / this->noDataPoints;
       
        if (this->weights[l] > 0.0) 
        {
            
            Feature tmpCentroid = {this->centersX[l], this->centersY[l],
            this->colorValues1[l], 
            this->colorValues2[l], this->colorValues3[l],
            this->coa[l], this->con[l]};   
            this->signature.push_back({this->weights[l], tmpCentroid});
            kk++;
            
        }
    }
    
   
    // scale all CT features to a range from -10000 to 10000
    //this->scalePCTDimensions();
    
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "clustering() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";            
    
    // "10. done, cleaning up
    delete numItersHistory;
    delete [] workingAssignment;
    delete []assignment;
    delete ds;
    delete algorithm;
 
}

 
/*
  4.0 Function to write a color image from the features extracted
  
*/

void JPEGImage::writeColorImage(const char* fileName)
{

    auto t1 = std::chrono::high_resolution_clock::now();
        
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(fileName, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", fileName);
        exit(1);
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = this->width;
    cinfo.image_height = this->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = this->width * cinfo.input_components;

    unsigned char* tmp_pixels 
        = new unsigned char[(this->height * row_stride) + 1];
    unsigned int c = 0;


    this->pixMat4 = new unsigned char**[this->height];
    for (int i = 0; i < this->height; i++)  
    {
        this->pixMat4[i] = new unsigned char*[this->width]; 
        for (int j = 0; j < this->width; j++)
        {
            this->pixMat4[i][j] = new unsigned char[3];
            this->pixMat4[i][j][0] = 255;
            this->pixMat4[i][j][1] = 255;
            this->pixMat4[i][j][2] = 255;
        }
    }
    
    for (int z = 0; z < this->noSamples; z++)
    {
        int tmpX = this->samplesX[z];
        int tmpY = this->samplesY[z];
                
        for (int k = 0; k < this->patchSize; k++) 
        {
            for (int n = 0; n < this->patchSize; n++) 
            {
                if (((tmpY + k) < this->height)
                && ((tmpX + n) < this->width))
                {
                    this->pixMat4[tmpY+k][tmpX+n][0] 
                    = static_cast<unsigned char>(
                    this->pixMat5[tmpY+k][tmpX+n][0]);
                    this->pixMat4[tmpY+k][tmpX+n][1] 
                    = static_cast<unsigned char>(
                    this->pixMat5[tmpY+k][tmpX+n][1]);
                    this->pixMat4[tmpY+k][tmpX+n][2] 
                    = static_cast<unsigned char>(
                    this->pixMat5[tmpY+k][tmpX+n][2]);
                }
            }
        }
    }
  
    // writing to tmp_pixels now to preserve order of features
    for (int i = 0; i < this->height; i++)
    {
        for (int j = 0; j < this->width; j++)
        {
            tmp_pixels[c] = this->pixMat4[i][j][0];
              tmp_pixels[c+1] = this->pixMat4[i][j][1];
              tmp_pixels[c+2] = this->pixMat4[i][j][2];
              c += 3;
        }    
    }

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &tmp_pixels[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
 
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "writeColor() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";        
}


/*
  5.0 Function to write a gray scale image from the features extracted
  
*/

void JPEGImage::writeGrayscaleImage(const char* fileName) 
{
    auto t1 = std::chrono::high_resolution_clock::now();
    
    bool grayscale = true;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    if ((outfile = fopen(fileName, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", fileName);
        exit(1);
    }
    
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = this->width;
    cinfo.image_height = this->height;
    
    if (grayscale)
    {
        cinfo.input_components = 1;
        cinfo.in_color_space = JCS_GRAYSCALE;
    }
    else
    {
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
    }
    
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = this->width * cinfo.input_components;
    unsigned char* tmp_pixels 
        = new unsigned char[(this->height * row_stride) + 1];

    int c = 0;
    if (grayscale)
    {
        for (int i = 0; i < this->height; i++)
        {
            for (int j = 0; j < this->width; j++)
            {
                tmp_pixels[c] 
                    = static_cast<unsigned char>(pixMat5[i][j][6]); 
                c++;
            }
        }
    }
    else
    {
        unsigned int c = 0;
        for (int i = 0; i < this->height; i++)
        {
            for (int j = 0; j < this->width; j++)
            {
                tmp_pixels[c] 
                = static_cast<unsigned char>(this->pixMat5[i][j][0]);
                tmp_pixels[c+1] 
                = static_cast<unsigned char>(this->pixMat5[i][j][1]);
                tmp_pixels[c+2] 
                = static_cast<unsigned char>(this->pixMat5[i][j][2]);
                c += 3;
            }
        }
    }
    
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &tmp_pixels[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "writeGrayscaleImage() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";        
}


/*
  6.0 Function to write an image from the contrast features extracted
  
*/


void JPEGImage::writeContrastImage(const char* filename, 
    double normalization)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = this->width;
    cinfo.image_height = this->height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = this->width * cinfo.input_components;
    unsigned char* tmp_pixels
        = new unsigned char[(this->height * row_stride)+1];
    
    this->pixMat4 = new unsigned char**[this->height];
    for (int i = 0; i < this->height; i++)  
    {
        this->pixMat4[i] = new unsigned char*[this->width]; 
        for (int j = 0; j < this->width; j++)
        {
            this->pixMat4[i][j] = new unsigned char[3];
            this->pixMat4[i][j][0] = 255;
            this->pixMat4[i][j][1] = 255;
            this->pixMat4[i][j][2] = 255;
        }
    }
    

    
    for (int z = 0; z < this->noSamples; z++)
    {
        int tmpX = this->samplesX[z];
        int tmpY = this->samplesY[z];
                
        for (int k = 0; k < this->patchSize; k++) 
        {
            for (int n = 0; n < this->patchSize; n++) 
            {
                if (((tmpY + k) < this->height)
                && ((tmpX + n) < this->width))
                {
                    this->pixMat4[tmpY+k][tmpX+n][0] 
                    = static_cast<unsigned char>(
                    (this->pixMat5[tmpY+k][tmpX+n][4]) * normalization);
                    // todo: fix parameter
                   // * normalization << std::endl;  
                }
            }
        }                
    }
 
    int c = 0;
    for (int i = 0; i < this->height; i++)
    {
        for (int j = 0; j < this->width; j++)
        {
            tmp_pixels[c] = this->pixMat4[i][j][0];
              c ++;
        }    
    }

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &tmp_pixels[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);    
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
   
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "writeConstrastImage() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";    
}



/*
  7.0 Function to write an image from the coarseness features extracted
  
*/

void JPEGImage::writeCoarsenessImage(const char* filename, 
    double normalization)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = this->width;
    cinfo.image_height = this->height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = this->width * cinfo.input_components;
    unsigned char* tmp_pixels 
        = new unsigned char[(this->height * row_stride) + 1];
 
    for (int i = 0; i < this->height; i++)  
    {
        for (int j = 0; j < this->width; j++)
        {
            this->pixMat4[i][j][0] = 255;
        }
    }
        
    for (int z = 0; z < this->noSamples; z++)
    {
        if (this->samplesX[z] >= 0 && 
            this->samplesX[z] < this->width &&
            this->samplesY[z] >= 0 &&
            this->samplesY[z] < this->height
            )
        {
                int tmpX = this->samplesX[z];
                int tmpY = this->samplesY[z];
                            
                for (int k = 0; k < this->patchSize; k++) 
                {
                    for (int n = 0; n < this->patchSize; n++) 
                    {
                        if (((tmpY + k) < this->height) 
                        && ((tmpX + n) < this->width))
                        {    
                            this->pixMat4[tmpY+k][tmpX+n][0] 
                            = static_cast<unsigned char>(
                            (this->pixMat5[tmpY+k][tmpX+n][3] 
                                * normalization));
                        }
                    }
                }
        }           
    }        
    
    
    
    int c = 0;
    for (int i = 0; i < this->height; i++)
    {
        for (int j = 0; j < this->width; j++)
        {
            tmp_pixels[c] = this->pixMat4[i][j][0];
              c ++;
        }    
    }

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &tmp_pixels[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    
 
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "writeCoarsenessImage() took "
<< std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";    
}

/*
  7.0 Function to draw a circle. Needed to write a cluster image.
  Clusters are shown as circles -> following the example
  from Beecks' papers
   
*/  

void JPEGImage::drawCircle(int x0, int y0, int radius)
{    
    for(int y = -radius; y<=radius; y++) 
    {
        for(int x = -radius; x<=radius; x++) 
        {
            if ((x*x) + (y*y) <= (radius*radius))
            {
                double xSum = static_cast<double>(x) 
                    + static_cast<double>(x0);
                double ySum = static_cast<double>(y) 
                    + static_cast<double>(y0);
                if (xSum < 0) continue;
                if (ySum < 0) continue;
                if (xSum >= this->width) continue;
                if (ySum >= this->height) continue;
        
                this->pixMat4[(int)ySum][(int)xSum][0] 
                = static_cast<unsigned char>(this->pixMat5[y0][x0][0]);
                this->pixMat4[(int)ySum][(int)xSum][1] 
                = static_cast<unsigned char>(this->pixMat5[y0][x0][1]);
                this->pixMat4[(int)ySum][(int)xSum][2] 
                = static_cast<unsigned char>(this->pixMat5[y0][x0][2]);
            }
        }
    }
}


/*
  7.0 Function to write an image from the clustered features
   
*/

void JPEGImage::writeClusterImage(const char* fileName, 
    double normalization)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo); 

    if ((outfile = fopen(fileName, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", fileName);
        exit(1);
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = this->width;
    cinfo.image_height = this->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = this->width * cinfo.input_components;
    unsigned char* tmp_pixels 
        = new unsigned char[(this->height * row_stride) + 1];

    
    for (int i = 0; i < this->height; i++)  
    {
        for (int j = 0; j < this->width; j++)
        {
            this->pixMat4[i][j][0] = 255;
            this->pixMat4[i][j][1] = 255;
            this->pixMat4[i][j][2] = 255;
        }
    }        
    
    
    for (unsigned int k = 0; k < this->clusters->size(); k++)  
    {    
        
        unsigned int x = this->centersX[k];
        unsigned int y = this->centersY[k];        
        int r = (int)(this->weights[k] * normalization);        
        drawCircle(y, x, r);
    }
    
    
    
    int c = 0;
    for (int i = 0; i < this->height; i++)
    {
        for (int j = 0; j < this->width; j++)
        {
              tmp_pixels[c] = this->pixMat4[i][j][0];
              tmp_pixels[c+1] = this->pixMat4[i][j][1];
              tmp_pixels[c+2] = this->pixMat4[i][j][2];
              c += 3;
        }
    }  
                
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &tmp_pixels[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "writeClusterImage() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";    
}


/*
  8.0 Helper functions for coarsness
   
  
*/

double JPEGImage::ak(int x, int y, unsigned int k)
{
    double tmpSum = 0.0;
    //for (int i = -pow(2,k-1);  i < pow(2, k-1) - 1; i++) 
    for (int i = -(1 << (k - 1));  i < (1 << (k - 1)) - 1; i++) 
    {
        //for (int j = -pow(2, k-1); j < pow(2, k-1) - 1; j++)
        for (int j = -(1 << (k - 1)); j < (1 << (k - 1)) - 1; j++)
        {
            if (((x+i) < this->width) && 
            ((y+j) < this->height) &&
            ((x+i) > 0) &&
            ((y+j) > 0))
            {
                tmpSum += this->pixMat5[y+j][x+i][6];
            }
        }
    }
    
    return (1.0/(double)pow(2, 2*k))  * tmpSum;
    //return (1.0 / ((1 << (2 * k)) * tmpSum));
}


double JPEGImage::ekh(int x, int y, unsigned int k)
{
    double res 
    //= std::abs(ak(x + pow(2, k-1), y, k) - ak(x - pow(2, k-1), y, k));
    = std::abs(ak(x + (1 << (k - 1)), y, k) - ak(x - (1 << (k - 1)), y, k));
    return res;
}


double JPEGImage::ekv(int x, int y, unsigned int k)
{
    double res 
    //= std::abs(ak(x, y + pow(2, k-1), k) - ak(x, y - pow(2, k-1), k));
    = std::abs(ak(x, y + (1 << (k - 1)), k) - ak(x, y - (1 << (k - 1)), k));
    return res;
}



double JPEGImage::localCoarseness(int x, int y, const int range)  
{
    double maxK = 0.0;
    double maxE = 0.0;
    
    for (int k = 1; k <= range; k++)
    {
        double tmpE = std::max(ekv(x, y, k), ekh(x, y, k));
        if (!std::isnan(tmpE))
        {
            if (tmpE > maxE)
            {
                maxE = tmpE;
                maxK = static_cast<double>(k);
                //std::cout << "printing k:" << k << std::endl;
            }
        }
    }

    if (std::isnan(maxE))
    {
        std::cout << "ugh, maxK is not a number:" << maxK << std::endl;
        return 0.0;
    }
    else
    {
        //std::cout << "maxK:" << maxK << std::endl;
        return maxK;
    }
}

void JPEGImage::computeCoarsenessValues(const int range)
{
    const double normalize = 1.0;  // this has to tried out
                                    
    
    auto t1 = std::chrono::high_resolution_clock::now();
  
    int cnt = 0;
    for (int z = 0; z < this->noSamples; z++)
    {
        if (this->samplesX[z] >= 0 && 
            this->samplesX[z] < this->width &&
            this->samplesY[z] >= 0 &&
            this->samplesY[z] < this->height
            )
        {
                int tmpX = this->samplesX[z];
                int tmpY = this->samplesY[z];
                            
                for (int k = 0; k < this->patchSize; k++)
                {
                    for (int n = 0; n < this->patchSize; n++) 
                    {
                        if (((tmpY + k) < this->height) 
                            && ((tmpX + n) < this->width))
                        {    
                            double coa = localCoarseness((tmpX + k),
                                (tmpY + n), range);
                                
                            //std::cout << " coa:" << coa << " norm" 
                            //<< normalize << " sum: " 
                            //<< (coa * normalize);
                            this->pixMat5[tmpY+k][tmpX+n][3] 
                                = (coa * normalize);
                            cnt++;
                        }
                    }
                }
        }           
    }
    
    
    //double min_norm = 0;
    //double max_norm = 255;
    //double res = (maxK - min)  * ((max_norm - min_norm) / (max - min));
                
        
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "coarseness() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";            
    
}


/*
  9.0 Helper functions for extracting contrast
   
  
*/

double JPEGImage::my(int x, int y, const int range)
{

    //int firstHalfRange = (-1) * (range / 2); 
    //int secondHalfRange = range - (-1) * firstHalfRange;
    int firstHalfRange = -(range / 2); 
    //int secondHalfRange = range - (-1) * firstHalfRange;
    int secondHalfRange = range + firstHalfRange;
  
    
    double tmpSum = 0.0;
    for (int j = firstHalfRange; j <= secondHalfRange; j++) 
    {
        for (int i = firstHalfRange; i <= secondHalfRange; i++)
        {
            if (((x+i) <= this->width - 1) && 
                ((y+j) <= this->height - 1) &&
                ((x+i) > 0) &&
                ((y+j) > 0))
            {
                tmpSum += this->pixMat5[y+j][x+i][6];
            }
        }
    }
    //return (1.0 / 9.0) * tmpSum;
    double tmpDev = static_cast<double>(pow(range,2));
    return (1.0 / tmpDev) * tmpSum;
}


double JPEGImage::sigma(int x, int y, const int range)
{    
    //int firstHalfRange = (-1) * (range / 2); 
    int firstHalfRange = -(range / 2); 
    //int secondHalfRange = range - (-1) * firstHalfRange;
    int secondHalfRange = range + firstHalfRange;
  
    double tmpSum = 0.0;
    
    for (int j = firstHalfRange; j <= secondHalfRange; j++)
    {
        for (int i = firstHalfRange; i <= secondHalfRange; i++)
        {
            if (((x+i) < this->width) && 
                ((y+j) < this->height) &&
                ((x+i) > 0) &&
                ((y+j) > 0))
            {
                tmpSum += pow(this->pixMat5[y+j][x+i][6] 
                    - my(x, y, range), 2);
            }
        }
    }
    double tmpDev = static_cast<double>(pow(range,2));
    return sqrt((1.0/tmpDev) * tmpSum);
    //return sqrt((1.0/9.0) * tmpSum);
}


double JPEGImage::eta(int x, int y, const int range)
{
    //int firstHalfRange = (-1) * (range / 2); 
    //int secondHalfRange = range / 2;
    //int firstHalfRange = (-1) * (range / 2); 
    //int secondHalfRange = range - (-1) * firstHalfRange;
    int firstHalfRange = -(range / 2); 
    //int secondHalfRange = range - (-1) * firstHalfRange;
    int secondHalfRange = range + firstHalfRange;
  
  
    
    double tmpSum = 0.0;
    for (int j = firstHalfRange; j <= secondHalfRange; j++)
    {
        for (int i = firstHalfRange; i <= secondHalfRange; i++)
        {
            if (((x+i) < this->width) && 
                ((y+j) < this->height) &&
                ((x+i) > 0) &&
                ((y+j) > 0))
                {                    
                    tmpSum += pow(this->pixMat5[y+j][x+i][6]
                    - my(x, y, range), 4);    
                }
        }
    }
    double tmpDev = static_cast<double>(pow(range,2));
    return (1.0/tmpDev) * tmpSum;
    //return (1.0/9.0) * tmpSum;
}


double JPEGImage::localContrast(int x, int y, const int range)
{
    double sig = sigma(x, y, range);
    double powEta = eta(x, y, range);
    double powSigma = pow(sig, 4);
    double denominator = powEta / powSigma;
    double res = sig / (double)pow(denominator, 0.25);

    if (!std::isnan(res))
        return res;
    else
        return 0.0;
}


void JPEGImage::computeContrastValues(const int range)
{
    
    
    auto t1 = std::chrono::high_resolution_clock::now();
    
    for (int z = 0; z < this->noSamples; z++)
    {
        if (this->samplesX[z] >= 0 && 
            this->samplesX[z] < this->width &&
            this->samplesY[z] >= 0 &&
            this->samplesY[z] < this->height
            )
        {
            int tmpX = this->samplesX[z];
            int tmpY = this->samplesY[z];
                        
            for (int k = 0; k < this->patchSize; k++) 
            {
                for (int n = 0; n < this->patchSize; n++) 
                {
                    if (((tmpY + k) < this->height) 
                        && ((tmpX + n) < this->width))
                    {            
                        double con = localContrast((tmpX+k), 
                            (tmpY+n), range);
                        this->pixMat5[tmpY+k][tmpX+n][4] 
                            = (con);
                    }
                }
            }
        }        
    }
             
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "contrasts() took "
    << 
    std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
    << " milliseconds\n";                
}









