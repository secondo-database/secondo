/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef SECONDO_IMPORTPOINTCLOUD_H
#define SECONDO_IMPORTPOINTCLOUD_H

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"  
#include "Symbols.h"
#include "Tools/Flob/DbArray.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include <AlgebraTypes.h>
#include <Operator.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <dirent.h>
#include <deque>
#include "PointCloud.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace routeplanningalgebra {
    /*
    ~importpointcloud~ operator

    Imports LAS point data into stream(pointcloud)
    {string, text} -> stream(pointcloud)

    Added and maintained by Gundula Swidersky, Dec 2017 
    */

    class ImportPointCloud {
    public:


        /*
        Type Mapping
        */
        static ListExpr importpointcloudTM(ListExpr args);

        /*
        Value Mapping
        */
        static int importpointcloudVM(Word *args,
                                      Word &result,
                                      int message,
                                      Word &local,
                                      Supplier s);
        /*
        Local Info Class
        */
        template<class T> 
        class importpointcloudLI {
            public:
            importpointcloudLI(T* arg) : pcCount(0) {
                pointCloudArray = 0;
                pointCloudArray2 = 0;
                lasFileDir = "";
                if(arg->IsDefined()) {
                    lasFileDir = arg->GetValue();
                }
                // Initialization of vars
                lasFileList.clear();                
                laspcFileName = "";
                maxPoints = 10000000; // limit of points read from one file
                maxNoPoints = 2400; // amount of points calc per PC for grid
                maxPointsPC = 2499; // max 2500 cell per stored PC
                pcCount = 0;
                pcCount2 = -1;
                noPCElems = 0;
                endOfPointClouds = false;
                noGridCells = 0;
                // Read las file dir (if provided) and read (first) las file
                // and build a PointCloud array that provides 
                // the pointclouds one after the other upon request.
                if (lasFileDir.empty()) { 
                    //cout << "Please enter a valid Dir or file name!"
                    //        << endl;
                    endOfPointClouds = true;
                } else {
                    readLasFileNames(lasFileDir);
                    if (lasFileList.size() <= 0) {
                        endOfPointClouds = true;
                    }
                    //cout << "Number of las files to be processed: " 
                    //        << lasFileList.size() << endl; 
                }
                // read first las file data
                while ((readLasFiles() != 0) && (!endOfPointClouds)) {
                    // if error try with next file
                }
            }

            ~importpointcloudLI() {
               deletePointCloudArray();
               if(pointCloudArray2) delete[] pointCloudArray2;
            }

            /*
            Read lasFiles names in provided directory and
            build array with filenames of lasFiles to be
            worked through later on demand.
            */
            void readLasFileNames(const string &lasDir) {
                cout << "Las files to be processed:" << endl;
                if (lasDir.substr((lasDir.length() -1), 1) != "/") {
                  // file name provided
                  lasFileList.push_front(lasDir);
                } else {
                    if (DIR *dir = opendir(lasDir.c_str())) {
                        while (struct dirent *entry = readdir(dir)) {
                            if (strcmp(entry->d_name, ".") == 0) { 
                                continue;
                            }

                            if (strcmp(entry->d_name, "..") == 0) {
                                continue;
                            }
                        
                            if ((entry->d_type & DT_DIR) == DT_DIR
                                && !((entry->d_type & DT_LNK) == DT_LNK)) {
                                continue;
                            }
                            if ((entry->d_type & DT_REG) == DT_REG) {
                                string lasFilNam = lasDir + entry->d_name;
                                cout << lasFilNam << endl;
                                lasFileList.push_front(lasFilNam);
                            }
                        }
                        closedir(dir);
                    }
                }
             }            
            
            /*
            Read lasFile and return PointCloud object(s).
            */
            int readLasFiles() {
                fError = false;
                pError = false;
                endOfPointClouds = false;
                pcCount = 0;
                noPCElems = 0;
                pcCount2 = -1;
                // file Name should be provided with path, 
                // e.g. "bin/lastest/xyz.las"
                // pick first las file name from list
                if (lasFileList.size() <= 0) {
                    endOfPointClouds = true;
                    return 0;
                }
                laspcFileName = lasFileList.back();
                lasFileList.pop_back();
                // test output las file name to be read
                cout << "   " << endl;
                cout << "Processing las file : " << laspcFileName  << endl;
                ifstream lasFile(laspcFileName.c_str(), ios::in|ios::binary);
                if (!lasFile) {
                    // handle error
                    fError = true;
                    cout << "Error opening lasfile." << endl;
                    return -2;
                } 
                // read Header data
                lasFile.read(reinterpret_cast<char*>(&fSignature), 
                             sizeof(&fSignature));
                sSignature = fSignature;
                if (sSignature != "LASF") {
                    // Error Handling
                    cout << 
                    "Error: 'LASF' Signature missing."
                    << "Not a valid lasfile???" << endl;
                    fError = true;
                    return -2;
                }
                cout << "Signature (LASF): " << sSignature << endl;
                lasFile.seekg(static_cast<ios::off_type>(24), ios::beg);
                lasFile.read(reinterpret_cast<char*>(&fVerMajor), 1);
                lasFile.read(reinterpret_cast<char*>(&fVerMinor), 1);
                unsigned int sVerMajor = (unsigned int) fVerMajor;
                unsigned int sVerMinor = (unsigned int) fVerMinor;
                cout << "Version: " << sVerMajor << "."
                        << sVerMinor << endl;
                lasFile.seekg(static_cast<ios::off_type>(70), ios::cur);
                lasFile.read(reinterpret_cast<char*>
                    (&fOffsetToPointData), 4);
                cout << "Offset to point data: "
                        << fOffsetToPointData << endl;
                lasFile.seekg(static_cast<ios::off_type>(5), ios::cur);
                lasFile.read(reinterpret_cast<char*>
                    (&fPointDataRecLength), 2);
                cout << "Point data record length: "
                        << fPointDataRecLength << endl;
                lasFile.read(reinterpret_cast<char*>
                    (&fNumPointRecs), 4);
                cout << "No of point records: "
                        << fNumPointRecs << endl;
                cellOffsetX = (fMaxX - fMinX) / noGridCellsSide;
                lasFile.seekg(static_cast<ios::off_type>(20), ios::cur);
                lasFile.read(reinterpret_cast<char*>(&fScaleX), 8);
                lasFile.read(reinterpret_cast<char*>(&fScaleY), 8);
                lasFile.read(reinterpret_cast<char*>(&fScaleZ), 8);
                lasFile.read(reinterpret_cast<char*>(&fOffsetX), 8);
                lasFile.read(reinterpret_cast<char*>(&fOffsetY), 8);
                lasFile.read(reinterpret_cast<char*>(&fOffsetZ), 8);
                lasFile.read(reinterpret_cast<char*>(&fMaxX), 8);
                lasFile.read(reinterpret_cast<char*>(&fMinX), 8);
                lasFile.read(reinterpret_cast<char*>(&fMaxY), 8);
                lasFile.read(reinterpret_cast<char*>(&fMinY), 8);
                lasFile.read(reinterpret_cast<char*>(&fMaxZ), 8);
                lasFile.read(reinterpret_cast<char*>(&fMinZ), 8);
                // min and max values are already absolute values.
                // So do not calculate scale and offset here
                noGridCellsSide = (long) (sqrt(fNumPointRecs / maxNoPoints) 
                                         + 1.0);
                cellOffsetX = (fMaxX - fMinX) / noGridCellsSide;
                cellOffsetY = (fMaxY - fMinY) / noGridCellsSide;
                noGridCells = noGridCellsSide * noGridCellsSide;
                noPCElems = noGridCells;
                // create Arrays to store point data
                pointCloudArray = new PointCloud*[noGridCells];
                pointCloudArray2 = new PointCloud*[noGridCells];                
                initPointCloudArray();
                double minXArray[noGridCells];
                double maxXArray[noGridCells];
                double minYArray[noGridCells];
                double maxYArray[noGridCells];
                // Output some status information during run
                cout << "noGridCellsSide: " << noGridCellsSide << endl;
                cout << "cellOffsetX: " << cellOffsetX << endl;
                cout << "cellOffsetY: " << cellOffsetY << endl;
                cout << "noGridCells: " << noGridCells << endl;
                maxX = fMaxX;
                minX = fMinX;
                maxY = fMaxY;
                minY = fMinY;
                maxZ = fMaxZ;
                minZ = fMinZ;
                cout << std::setprecision(14);
                cout << "Absolute Min Point (" << minX << ", " 
                    << minY << ", " << minZ << ")" << endl;
                cout << "Absolute Max Point (" << maxX << ", " 
                    << maxY << ", " << maxZ << ")" << endl;
                // read point data and build 2D Tree
                lasFile.seekg(static_cast<ios::off_type>
                          (fOffsetToPointData), ios::beg);
                maxRead = (maxPoints < fNumPointRecs)
                   ? maxPoints : fNumPointRecs; 
                long icount = 1;
                while ((icount <= maxRead)  && (!lasFile.eof())) {
                    lasFile.read(reinterpret_cast<char*>(&fPointX), 4);
                    lasFile.read(reinterpret_cast<char*>(&fPointY), 4);
                    lasFile.read(reinterpret_cast<char*>(&fPointZ), 4);
                    // lasFile.read(reinterpret_cast<char*>(&fIntensity), 2);
                    if (!lasFile.eof()) {
                        lasFile.seekg(static_cast<ios::off_type>
                            (fPointDataRecLength-12), ios::cur);
                    }
                    if (calcPointData(icount, minXArray, maxXArray, 
                                      minYArray, maxYArray) == -1) {
                        // unexpected error during calculation
                        // wenn return code eq to -2 just ignore the current
                        // point and continue reading next point
                        return 0;
                    }
                    icount++;
                }
                cout << "Number of points read: " << icount - 1 << endl;
                // Set Min Max values of PC 
                setMinMaxValues(minXArray, maxXArray, minYArray, maxYArray);
                if (lasFile) {
                    lasFile.close();
                }
                if ((pError) || ((icount - 1) < fNumPointRecs)) {
                    cout << "Error with point data in las file:  " 
                            << laspcFileName << endl;
                }
                if (fError) {
                    cout << "Error processing file: " << laspcFileName 
                            << endl;
                    return 0;
                }
                return 0;
            }
            
            /*
            Help method to read  and calculate point data
            */
            int calcPointData(long icount, double* minXArray, 
                              double* maxXArray, double* minYArray, 
                              double* maxYArray) {
                // calculate absolute value (scale and offset)
                pointX = (fPointX * fScaleX) + fOffsetX;
                pointY = (fPointY * fScaleY) + fOffsetY;
                pointZ = (fPointZ * fScaleZ) + fOffsetZ;
                cellNoX = (long) (1.0 + (pointX - minX) / cellOffsetX);
                if (cellNoX > noGridCellsSide) {
                    if (cellNoX > (noGridCellsSide + 1)) {
                        pError = true;
                        cout << "No " << icount 
                                << "   Error: point value not within min"
                                << " max area (" << pointX << ", " << pointY 
                                << ", " << pointZ << ")" 
                                << "   Will continue with reading next point..."
                                << endl;
                        return -2;                    
                    }
                    cellNoX = noGridCellsSide;
                }
                cellNoY = (long) (1.0 + (pointY - minY) / cellOffsetY);
                if (cellNoY > noGridCellsSide) {
                    if (cellNoY > (noGridCellsSide + 1)) {
                        pError = true;
                        cout << "No " << icount 
                                << "   Error: point value not within min"
                                << " max area (" << pointX << ", " << pointY 
                                << ", " << pointZ << ")" 
                                << "   Will continue with reading next point..."
                                << endl;
                        return -2;                    
                    }
                    cellNoY = noGridCellsSide;
                }                    
                cellNo = cellNoX + (noGridCellsSide * (cellNoY - 1));
                if ((cellNo < 1) || (cellNo > noGridCells)) { 
                    pError = true;
                        cout << "No " << icount 
                                << "   Error: point value not within min"
                                << " max area (" << pointX << ", " << pointY 
                                << ", " << pointZ << ")" 
                                << "   Will continue with reading next point..."
                                << endl;
                    return -2;
                }
                // addCpointnode
                (pointCloudArray[cellNo -1])->AppendCpointnode(
                       Cpointnode(pointX, pointY, pointZ, -1, -1));
                if ((pointCloudArray[cellNo -1])->GetNoCpointnodes() > 1) {
                    // calculate new min max val for the cell (actual PC)
                    if (pointX < minXArray[cellNo - 1]) {
                        minXArray[cellNo - 1] = pointX;
                    } else {
                        if (pointX > maxXArray[cellNo - 1]) {
                            maxXArray[cellNo - 1] = pointX;
                        }
                    }
                    if (pointY < minYArray[cellNo - 1]) {
                        minYArray[cellNo - 1] = pointY;
                    } else {
                        if (pointY > maxYArray[cellNo - 1]) {
                            maxYArray[cellNo - 1] = pointY;
                        }
                    }                            
                } else {
                    // initialize min max with current value
                    minXArray[cellNo - 1] = pointX;
                    maxXArray[cellNo - 1] = pointX;
                    minYArray[cellNo - 1] = pointY;
                    maxYArray[cellNo - 1] = pointY;
                }
                return 0;
            }
            
            /*
            initPointCloudArray 
            
            PointCloud area was broken into a grid due to the high amount
            pointdata read from file
            */
            void initPointCloudArray() {
                for (int i = 0; i < noPCElems; i++) {
                     pointCloudArray[i] = new PointCloud(0);
                }
            }

            void emptyPointCloudArray(){
                for (int i = 0; i < noPCElems; i++) {
                     if(pointCloudArray[i]) {
                        pointCloudArray[i]->DeleteIfAllowed();
                        pointCloudArray[i] = 0;
                     }
                }
            }
            void deletePointCloudArray(){
              if(pointCloudArray){
                 emptyPointCloudArray();
                 delete[] pointCloudArray;
                 pointCloudArray = 0;
              }
            }

            
           /*
           setMinMaxValues of PointClouds
           */
            void setMinMaxValues(double* minXArray, double* maxXArray, 
                                 double* minYArray, double* maxYArray) {
                int sizePC;
                for (int i = 0; i < noPCElems; i++) {
                    sizePC = pointCloudArray[i]->GetNoCpointnodes();
                    if (sizePC > 0) {
                        pointCloudArray[i]->setMinX(minXArray[i]);
                        pointCloudArray[i]->setMinY(minYArray[i]);
                        pointCloudArray[i]->setMaxX(maxXArray[i]);
                        pointCloudArray[i]->setMaxY(maxYArray[i]);
                    }
                }
            }
            
            /*
            doSplitPC
            Go over PC that are to big and need a split
            */
            void doSplitPC() {
                // Set pointer content pointCloudArray to 0
                // to avoid references to data that was already 
                // returned as result.

                if(pointCloudArray){
                  for (int di = 0; di < noPCElems; di++) {
                      pointCloudArray[di] = 0;
                  }
                  delete[] pointCloudArray;
                  pointCloudArray=0;                        
                }
                noPCElems = 0;
                bool xDim;
                // Check if split of PC necessary
                if (pcCount2 < 0) {
                    // cout << "No split necessary!" << endl;
                    return;
                }
                // New pointCloudArray with double size of 
                // pointCloudArray2
                noPCElems = (pcCount2 + 1) * 2;
                // cout << "No of PC elements after split: " << noPCElems 
                //         << endl;
                pointCloudArray = new PointCloud*[noPCElems];
                // Temp PC for split calculation
                PointCloud* pcTemp[2]; 
                /*
                pcTemp[0] = new PointCloud(0);
                pcTemp[1] = new PointCloud(0); 
                */
                int pcSize;
                double min2X, min2Y;
                double pcMin[2], pcMax[2];
                array<double, 4> pc1MinMax, pc2MinMax;
                double curX, curY, curZ;
                Cpointnode pcNode;
                Cpointnode* ppcNode;

                // Split PointClouds that are too big 
                for (int i = 0; i <= pcCount2; i++) {
                    // split actual PC object
                    pcTemp[0] = new PointCloud(0);
                    pcTemp[1] = new PointCloud(0);                    
                    // calculate new min max value with split of the longer side
                    pcMin[0] = pointCloudArray2[i]->getMinX();
                    pcMax[0] = pointCloudArray2[i]->getMaxX();
                    pcMin[1] = pointCloudArray2[i]->getMinY();
                    pcMax[1] = pointCloudArray2[i]->getMaxY();
                    if ( (pcMax[0] - pcMin[0]) > (pcMax[1] - pcMin[1]) ) { 
                        // split x dimension
                        xDim = true;
                        // cout << "Split x dim   ";
                        min2X = (pcMin[0] + (pcMax[0] - pcMin[0]) / 2.0);
                        min2Y = pcMin[1];
                    } else {
                        // split y dimension
                        xDim = false;
                        // cout << "Split y dim   ";
                        min2X = pcMin[0];
                        min2Y = (pcMin[1] + (pcMax[1] - pcMin[1]) / 2.0);
                    }
                    // Read actual PointCloud and divide into 2 PCs,
                    // No special procedure as every point needs to be read.
                    pcSize = pointCloudArray2[i]->GetNoCpointnodes();
                    for (int k = 0; k < pcSize; k++) {
                        pcNode = pointCloudArray2[i]->GetCpointnode(k);
                        ppcNode = &pcNode;
                        curX = ppcNode->getX();
                        curY = ppcNode->getY();
                        curZ = ppcNode->getZ();
                        // Determine to which PC append the node
                        // and calculate the new min max values.
                        if ( ((xDim) && (curX > min2X)) 
                            || ((!xDim) && (curY > min2Y)) ) {
                            // PC1
                            pcTemp[0]->AppendCpointnode(Cpointnode(
                                curX, curY, curZ, -1, -1));
                            if (pcTemp[0]->GetNoCpointnodes() > 1) {
                                pc1MinMax = calcNewMinMax(
                                    curX, curY, pc1MinMax);
                            } else {
                                // [0] minX, [1] minY, [2] maxX, [3] maxY
                                pc1MinMax[0] = curX;
                                pc1MinMax[1] = curY;
                                pc1MinMax[2] = curX;
                                pc1MinMax[3] = curY;
                            }
                        } else {
                            // PC2
                            pcTemp[1]->AppendCpointnode(Cpointnode(
                                curX, curY, curZ, -1, -1));
                            if (pcTemp[1]->GetNoCpointnodes() > 1) {
                                pc2MinMax = calcNewMinMax(
                                    curX, curY, pc2MinMax);
                            } else {
                                pc2MinMax[0] = curX;
                                pc2MinMax[1] = curY;
                                pc2MinMax[2] = curX;
                                pc2MinMax[3] = curY;
                            }
                        }
                    }
                    // set calculated MinMaxValues
                    pcTemp[0]->setMinX(pc1MinMax[0]);
                    pcTemp[0]->setMinY(pc1MinMax[1]);
                    pcTemp[0]->setMaxX(pc1MinMax[2]);
                    pcTemp[0]->setMaxY(pc1MinMax[3]);
                    pcTemp[1]->setMinX(pc2MinMax[0]);
                    pcTemp[1]->setMinY(pc2MinMax[1]);
                    pcTemp[1]->setMaxX(pc2MinMax[2]);
                    pcTemp[1]->setMaxY(pc2MinMax[3]);
                    // store splitted PC's in pointCloudArray
                    pointCloudArray[2 * i] = pcTemp[0];
                    pointCloudArray[(2 * i) + 1] = pcTemp[1];
                }
                // Delete not needed content pointCloudArray2
                for (int di = 0; di <= pcCount2; di++) {
                    pointCloudArray2[di]->DestroyPointCloud();
                    delete pointCloudArray2[di];
                    pointCloudArray2[di] = 0;
                }
                
                if(pointCloudArray2) delete[] pointCloudArray2;
                pointCloudArray2 = new PointCloud*[noPCElems];
                pcCount = 0;
                pcCount2 = -1;
                return;
            }
                                      
            /*
            calcNewMinMax
            */
            array<double, 4> calcNewMinMax(double curX, double curY, 
                                           array<double, 4> pcCurMinMax) {
                // [0] minX, [1] minY, [2] maxX, [3] maxY
                if (curX < pcCurMinMax[0]) {
                    pcCurMinMax[0] = curX;
                } else {
                    if (curX > pcCurMinMax[2]) {
                        pcCurMinMax[2] = curX;
                    }
                }
                if (curY < pcCurMinMax[1]) {
                    pcCurMinMax[1] = curY;
                } else {
                    if (curY > pcCurMinMax[3]) {
                        pcCurMinMax[3] = curY;
                    }
                }
                return pcCurMinMax;
            }
                                      
            /*
            locatePosIn2DTree
            
            Determines the location of a new node within the 2D Tree that is
            the internal data structure of points within a PointCloud object.
            */
            void locatePosIn2DTree(const double pointX, const double pointY, 
                       PointCloud* ptCloud, int idx, int chkDim, int lastElem) {
                bool searchLeft = true;
                // int lastElem = ptCloud->GetNoCpointnodes() - 1;
                Cpointnode tempnode = ptCloud->GetCpointnode(idx);
                Cpointnode* cptnode = &tempnode;
                if (chkDim == 1) {
                    // check x-Coordinate
                    if (cptnode->getX() > pointX) {
                        searchLeft = true;
                    } else {
                        searchLeft = false;
                    }
                } else {
                    // check y-Coordinate
                    if (cptnode->getY() > pointY) {
                        searchLeft = true;
                    } else {
                        searchLeft = false;
                    }
                }
                if (searchLeft) {
                    if (cptnode->getLeftSon() != -1) {
                        // continue search with left son
                        locatePosIn2DTree(pointX, pointY, ptCloud, 
                              (cptnode->getLeftSon()), (-1 * chkDim), lastElem);
                    } else {
                        // location of new element found
                        //cptnode->setLeftSon(lastElem);
                        //ptCloud->SetCpointnode(idx, cptnode);
                        ptCloud->changeLeftSon(idx,lastElem);
                    }
                } else {
                    if (cptnode->getRightSon() != -1) {
                        // continue search with right son
                        locatePosIn2DTree(pointX, pointY, ptCloud, 
                            (cptnode->getRightSon()), (-1 * chkDim), lastElem);
                    } else {
                        // location of new element found
                        //cptnode->setRightSon(lastElem);
                        //ptCloud->SetCpointnode(idx, cptnode);
                        ptCloud->changeRightSon(idx,lastElem);
                    }
                }
            }
            
            /*
            getNext
            
            Provide next PointCloud object for the stream.
            */
            PointCloud* getNext() {
                checkNext();
                if (endOfPointClouds) {
                    cout << "   " << endl;
                    cout << "end of pointclouds" << endl;
                    return 0;
                } else {
                    // build 2D Tree as internal datastructure for
                    // PC object to be returned
                    for (int i = 1; 
                         i < pointCloudArray[pcCount-1]->GetNoCpointnodes(); 
                         i++) {
                        Cpointnode tempnode = 
                            pointCloudArray[pcCount-1]->GetCpointnode(i);
                        Cpointnode* cptnode = &tempnode;
                        pointX = cptnode->getX();
                        pointY = cptnode->getY();
                        locatePosIn2DTree(pointX, pointY, 
                                      (pointCloudArray[pcCount -1]), 0, 1, i);
                    }
                    PointCloud* res = pointCloudArray[pcCount-1];
                    //pointCloudArray[pcCount-1] = 0; 
                    return (PointCloud*) res->Copy();
                }
            }
            
            /*
            checkNext
            */
            void checkNext() {
                if (!endOfPointClouds) {
                    pcCount++;
                    // Don't provide empty PC or PC that is too large
                    // Loop goes over an array that has changing size...
                    while ( (pcCount <= noPCElems) && (noPCElems != 0) &&
                           ((pointCloudArray[pcCount-1]->GetNoCpointnodes() 
                            == 0) || 
                           (pointCloudArray[pcCount-1]->GetNoCpointnodes()
                             > maxPointsPC)) )  {
                        // Move objects that need a split to pointCloudArray2
                        if (pointCloudArray[pcCount-1]->GetNoCpointnodes()
                             > maxPointsPC) {
                            pcCount2++;
                            pointCloudArray2[pcCount2] = 
                                pointCloudArray[pcCount-1];
                        }
                        pcCount++; 
                    }
                    if ((pcCount > noPCElems) && (noPCElems !=0) 
                        && (pcCount2 >= 0)) {
                        doSplitPC();
                        checkNext();
                    }
                    if (pcCount > noPCElems) {
                        for (int di = 0; di < noPCElems; di++) {
                            if(pointCloudArray[di]) delete pointCloudArray[di];
                            pointCloudArray[di] = 0;
                        }
                        delete[] pointCloudArray;
                        pointCloudArray = 0;
                        while ((readLasFiles() != 0) 
                               && (!endOfPointClouds)) {
                            // if error try with next file
                        } 
                        checkNext();
                    }
                }
            }
            
            private:
            // vars for lasfile treatment and interim PointClouds 
            string lasFileDir;
            string laspcFileName;
            deque<std::string> lasFileList;
            int pcCount, pcCount2;
            int noPCElems;
            bool endOfPointClouds;
            bool fError, pError;
            unsigned int maxPoints;
            unsigned int maxRead;
            char fSignature[4];
            string sSignature;
            unsigned char fVerMajor, fVerMinor;
            unsigned int fOffsetToPointData;
            unsigned short fPointDataRecLength;
            unsigned int fNumPointRecs;
            double fScaleX, fScaleY, fScaleZ;
            double fOffsetX, fOffsetY, fOffsetZ;
            double fMaxX, fMinX, fMaxY, fMinY;
            double fMaxZ, fMinZ;
            int fPointX, fPointY, fPointZ;
            double maxX, minX, maxY, minY;
            double maxZ, minZ;
            double pointX, pointY, pointZ;
            unsigned short fIntensity; 
            double cellOffsetX;
            double cellOffsetY;
            long noGridCellsSide;
            long noGridCells, cellNo, cellNoY, cellNoX;
            int maxNoPoints, maxPointsPC;
            PointCloud** pointCloudArray;
            PointCloud** pointCloudArray2;


        }; // end of class importpointcloudLI
         
    }; // end of class ImportPointCloud
    
} // end of namespace routeplanningalgebra

#endif //SECONDO_IMPORTPOINTCLOUD_H
