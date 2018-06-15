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
            importpointcloudLI(T* arg)  {
                pointCloudArray = 0;
                std::string lasFileDir = "";
                if(arg->IsDefined()) {
                    lasFileDir = arg->GetValue();
                }
                // Initialization of vars
                lasFileList.clear();                
                maxNoPoints = 2400; // amount of points calc per PC for grid
                maxPointsPC = 2499; // max 2500 cell per stored PC
                noPCElems = 0;
                noGridCells = 0;
                // initialize the files to be read
                readLasFileNames(lasFileDir);
            }

            ~importpointcloudLI() {
               if(pointCloudArray) delete[] pointCloudArray;
               while(!resultList.empty()){
                  PointCloud* pc = resultList.front();
                  resultList.pop();
                  pc->DestroyPointCloud();
                  delete pc;
               }
            }

            /*
            Read lasFiles names in provided directory and
            build array with filenames of lasFiles to be
            worked through later on demand.
            */
            void readLasFileNames(const std::string &lasDir) {
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
                                std::string lasFilNam = lasDir + entry->d_name;
                                cout << lasFilNam << endl;
                                lasFileList.push_front(lasFilNam);
                            }
                        }
                        closedir(dir);
                    }
                }
             }            



             int readLasFile(const std::string& laspcFileName) {
                fError = false;
                pError = false;

                // test output las file name to be read
                cout  << endl;
                cout << "Processing las file : " << laspcFileName  << endl;
                std::ifstream lasFile(laspcFileName.c_str(), 
                                      std::ios::in|std::ios::binary);
                if (!lasFile) {
                    // handle error
                    fError = true;
                    cout << "Error opening lasfile." << endl;
                    return -2;
                } 


                // read Header data
                lasFile.read(reinterpret_cast<char*>(&fSignature), 
                             sizeof(fSignature));
                sSignature = std::string(fSignature,4);
                if (sSignature != "LASF") {
                    // Error Handling
                    cout << 
                    "Error: 'LASF' Signature missing."
                    << "Not a valid lasfile???" << endl;
                    fError = true;
                    return -2;
                }
                cout << "Signature (LASF): " << sSignature << endl;
                uint16_t fileSourceId;
                lasFile.read(reinterpret_cast<char*>(&fileSourceId), 2);
                cout << "fileSourceId is " << fileSourceId << endl;

                uint16_t globalEncoding;
                lasFile.read(reinterpret_cast<char*>(&globalEncoding),2);

                // ignore project ids
                lasFile.seekg(static_cast<std::ios::off_type>(24),
                              std::ios::beg);

                lasFile.read(reinterpret_cast<char*>(&fVerMajor), 1);
                lasFile.read(reinterpret_cast<char*>(&fVerMinor), 1);
                unsigned int sVerMajor = (unsigned int) fVerMajor;
                unsigned int sVerMinor = (unsigned int) fVerMinor;
                cout << "Version: " << sVerMajor << "."
                        << sVerMinor << endl;
                cout << "globalEncoding" << globalEncoding << endl;
                
                char systemIdentifier[32];
                char generatingSoftware[32];
                uint16_t dayOfYear;
                uint16_t year;
                uint16_t headerSize;
                lasFile.read(systemIdentifier,32);
                lasFile.read(generatingSoftware,32);
                lasFile.read(reinterpret_cast<char*>(&dayOfYear),2);
                lasFile.read(reinterpret_cast<char*>(&year),2);
                lasFile.read(reinterpret_cast<char*>(&headerSize),2);
                cout << "system identifier " << systemIdentifier << endl;
                cout << "generatingSoftware " << generatingSoftware << endl;
                cout << "&dayOfYear " << dayOfYear << endl;
                cout << "year " << year << endl;
                cout << "headersize " << headerSize << endl;

                lasFile.read(reinterpret_cast<char*>
                    (&fOffsetToPointData), 4);
                cout << "Offset to point data: "
                        << fOffsetToPointData << endl;
                lasFile.seekg(static_cast<std::ios::off_type>(5), 
                              std::ios::cur);
                lasFile.read(reinterpret_cast<char*>
                    (&fPointDataRecLength), 2);
                cout << "Point data record length: "
                        << fPointDataRecLength << endl;
                lasFile.read(reinterpret_cast<char*>
                    (&fNumPointRecs), 4);
                cout << "No of point records: "
                        << fNumPointRecs << endl;


                cellOffsetX = (fMaxX - fMinX) / noGridCellsSide;
                lasFile.seekg(static_cast<std::ios::off_type>(20), 
                              std::ios::cur);
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
                lasFile.seekg(static_cast<std::ios::off_type>
                          (fOffsetToPointData), std::ios::beg);

                maxRead =  fNumPointRecs; 
                long icount = 1;
                while ((icount <= maxRead)  && (!lasFile.eof())) {
                    lasFile.read(reinterpret_cast<char*>(&fPointX), 4);
                    lasFile.read(reinterpret_cast<char*>(&fPointY), 4);
                    lasFile.read(reinterpret_cast<char*>(&fPointZ), 4);
                    // lasFile.read(reinterpret_cast<char*>(&fIntensity), 2);
                    if (!lasFile.eof()) {
                        lasFile.seekg(static_cast<std::ios::off_type>
                            (fPointDataRecLength-12), std::ios::cur);
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
                (pointCloudArray[cellNo -1])->insert(pointX, pointY, pointZ);
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
                assert(!pointCloudArray);
                pointCloudArray = new PointCloud*[noPCElems];
                for (int i = 0; i < noPCElems; i++) {
                    pointCloudArray[i] = new PointCloud(0);
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
            calcNewMinMax
            */
            void calcNewMinMax(double curX, double curY, 
                                           double*  pcCurMinMax) {
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
            }
                                      
            
            /*
            getNext
            
            Provide next PointCloud object for the stream.
            */
            PointCloud* getNext() {
               while(true){
                  if(!resultList.empty()){
                     PointCloud* result = resultList.front();
                     resultList.pop();
                     return result;                  
                  } 
                  if(lasFileList.empty()){
                    return 0;
                  }
                  std::string fileName = lasFileList.front();
                  lasFileList.pop_front();
                  fillResultList(fileName);
               }
            }

            /*
              Reads in the next file and stores the contained
              point clouds into result list
            */
            void fillResultList(const std::string& fileName){
               if(readLasFile(fileName) == 0){ // no Error
                  movePCsToResultList();     
               } else {
                 assert(pointCloudArray==0);
               }
            } 

            /*
              moves non-empty points clouds from point cloud array into
              the resultList, splits point clouds if necessary
            */
            void movePCsToResultList() {
              assert(resultList.empty());
              for(int i=0;i<noPCElems; i++){
                 PointCloud* pc = pointCloudArray[i];
                 pointCloudArray[i] = 0;
                 movePCToResultList(pc);
              }
              delete[] pointCloudArray;
              pointCloudArray = 0;
            }

            /* inserts a non-empty point cloud into
               result list. if the number of points 
               exceeds a theshold, the point cloud is
               split until the number of points fits.
            */
            void movePCToResultList(PointCloud* pc){
               size_t numPoints = pc->GetNoCpointnodes();
               if(numPoints==0){ // empty point clouds are useless
                 pc->DestroyPointCloud();
                 delete pc;
                 return;
               }
               if(numPoints > maxPointsPC){ // too much points, split required
                  std::vector<PointCloud*> splitPCs;
                  splitPointCloud(pc,splitPCs);
                  for(size_t i=0;i<splitPCs.size();i++){
                    resultList.push(splitPCs[i]);
                  }
                  return;  
               }
               // normal case, points are there but not too much
               resultList.push(pc);
            }
 
            void splitPointCloud(PointCloud* pc, 
                                 std::vector<PointCloud*>& result){
               assert(result.empty());
               std::queue<PointCloud*> q;
               q.push(pc);
               while(!q.empty()){
                  PointCloud* currentPC = q.front();
                  q.pop();
                  size_t numPoints = currentPC->GetNoCpointnodes();
                  if(numPoints == 0){ // do not include empty pcs
                     currentPC->DestroyPointCloud();
                     delete currentPC;
                  } else if(numPoints > maxPointsPC) {
                     // split if too large
                     PointCloud* pc1=0;
                     PointCloud* pc2=0;
                     splitSinglePC(currentPC, pc1, pc2);
                     assert(pc1);
                     assert(pc2);
                     currentPC->DestroyPointCloud();
                     delete currentPC;
                     q.push(pc1);
                     q.push(pc2);
                  } else {
                    // insert if number of points is good
                    result.push_back(currentPC);
                  }
              }
            }

            void splitSinglePC(PointCloud* original, 
                               PointCloud*& part1,
                               PointCloud*& part2){
              part1 = new PointCloud(0);
              part2 = new PointCloud(0);                    
              // calculate new min max value with split of the longer side
              double pcMin[2];
              double pcMax[2];
              pcMin[0] = original->getMinX();
              pcMax[0] = original->getMaxX();
              pcMin[1] = original->getMinY();
              pcMax[1] = original->getMaxY();
              bool xDim;   // split dimension
              double min2X; // split value x-direction
              double min2Y; // split value y-direction
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
              size_t pcSize = original->GetNoCpointnodes();
              Cpointnode pcNode;
              double pc1MinMax[4];
              double pc2MinMax[4];
 
              for (size_t k = 0; k < pcSize; k++) {
                  pcNode = original->GetCpointnode(k);
                  double curX = pcNode.getX();
                  double curY = pcNode.getY();
                  double curZ = pcNode.getZ();
                  // Determine to which PC append the node
                  // and calculate the new min max values.
                  if ( ((xDim) && (curX > min2X)) 
                      || ((!xDim) && (curY > min2Y)) ) {
                      // PC1
                      part1->insert(curX,curY,curZ);
                      if (part1->GetNoCpointnodes() > 1) {
                          calcNewMinMax(curX, curY, pc1MinMax);
                      } else {
                          // [0] minX, [1] minY, [2] maxX, [3] maxY
                          pc1MinMax[0] = curX;
                          pc1MinMax[1] = curY;
                          pc1MinMax[2] = curX;
                          pc1MinMax[3] = curY;
                      }
                  } else {
                      // PC2
                      part2->insert(curX,curY,curZ);
                      if (part2->GetNoCpointnodes() > 1) {
                        calcNewMinMax(curX, curY, pc2MinMax);
                      } else {
                          pc2MinMax[0] = curX;
                          pc2MinMax[1] = curY;
                          pc2MinMax[2] = curX;
                          pc2MinMax[3] = curY;
                      }
                  }
              }
              // set calculated MinMaxValues
              part1->setMinX(pc1MinMax[0]);
              part1->setMinY(pc1MinMax[1]);
              part1->setMaxX(pc1MinMax[2]);
              part1->setMaxY(pc1MinMax[3]);
              part2->setMinX(pc2MinMax[0]);
              part2->setMinY(pc2MinMax[1]);
              part2->setMaxX(pc2MinMax[2]);
              part2->setMaxY(pc2MinMax[3]);
            }
                  

            
            
            private:
            // vars for lasfile treatment and interim PointClouds 
            std::deque<std::string> lasFileList;
            int noPCElems;
            bool fError, pError;
            unsigned int maxRead;
            char fSignature[4];
            std::string sSignature;
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
            size_t maxNoPoints, maxPointsPC;
            PointCloud** pointCloudArray;
            std::queue<PointCloud*> resultList; // for one file
        }; // end of class importpointcloudLI
         
    }; // end of class ImportPointCloud
    
} // end of namespace routeplanningalgebra

#endif //SECONDO_IMPORTPOINTCLOUD_H
