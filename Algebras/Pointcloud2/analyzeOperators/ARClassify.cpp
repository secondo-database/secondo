/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



0 The ARClassify Class

This Class provides the static function classify
for "AnalyzeRaster" Operator.

*/
#include "ARClassify.h"

#include <math.h>
#include <bitset>

#include "../utility/DbScan.h"
#include "../utility/DbScanPoint.h"

using namespace pointcloud2;

std::unordered_map<int, ObjProp>
ARClassify::createObjectMap(
        const std::vector<SmiFileId>& rasters, 
        const std::vector<Rect3>& rastersBbox,
        size_t pointInMem) {
    std::unordered_map<int, ObjProp> object;
    size_t NoObjID = 0;
    std::vector<int> neighbor = {
            -1,
            (int)pointInMem,
            1,
            -(int)pointInMem
        };
    for (size_t i = 0; i < rasters.size(); i++) {
        // For SmiRecordFile init
        const bool fixed = true;
        const bool temp = false;
        const SmiSize size = sizeof(RasterPointDB);
        SmiRecordFile* current = new SmiRecordFile(fixed, size, temp);
        bool opened = current->Open(rasters[i]);
        if (opened) {
            //DEBUG
            //std::cout<<"Classify: SmiRecordFile "<<std::to_string(rasters[i])
            //         <<" opened"<<endl;
            SmiRecordFileIterator it;
            current->SelectAll(it);
            SmiRecord record;
            const Rectangle<3>& fileBbox = rastersBbox[i];
            const int rasterOffsetX = floor(
                    fileBbox.MinD(0) / Pointcloud2::CELL_SIZE_IN_M);
            const int rasterOffsetY = floor(
                    fileBbox.MinD(1) / Pointcloud2::CELL_SIZE_IN_M);
            size_t arrayCount = 0;
            std::vector<RasterPoint> rasterMem(std::pow(pointInMem,2));
            while (it.Next(record)) {
                RasterPointDB cellDB;
                const size_t read = record.Read(&cellDB,sizeof(RasterPointDB));
                assert(read == sizeof(RasterPointDB));
                const RasterPoint cell = op_analyzeRaster::secondoToStruct(
                        cellDB);
                rasterMem[arrayCount] = cell;
                arrayCount++;
            }
            delete current;
            arrayCount = 0;
            for (auto cell : rasterMem){
                if (cell.objectID <= 0) {
                    if (cell.objectID==-1)NoObjID++;
                } else {
                 if (cell.objectID > 0) {
                    const int x = ((arrayCount % pointInMem) + 0.) +
                            rasterOffsetX;
                    bool edge = false;
                    size_t edgeCount = 0;
                    double altDiff = 0.;
                    for (int i = 0; i < 4; i++){
                      size_t arrayCountN = arrayCount + neighbor[i];
                      if (arrayCountN < pow(pointInMem, 2) &&
                          rasterMem[arrayCountN].objectID != cell.objectID){
                        edge = true;
                        altDiff = (altDiff * edgeCount + cell.maxAlt
                            - rasterMem[arrayCountN].maxAlt) / (edgeCount + 1);
                        ++edgeCount;
                      }
                    }
                    auto search = object.find(cell.objectID);
                    if (search == object.end()) {
                        //no entry for this objID yet
                        const int y = ((arrayCount / pointInMem) + 0.) +
                                rasterOffsetY;
                        double min[] = {(arrayCount % pointInMem) + 0.,
                            (arrayCount / pointInMem) + 0.};
                        double max[] = {(arrayCount % pointInMem) + 0.,
                            (arrayCount / pointInMem) + 0.};
                        Rect bboxObj;
                        bboxObj.Set(true, min, max);
                        ObjProp newObject = {
                                    cell.count,
                                    1,
                                    (edge) ? 1 : 0,
                                    altDiff,
                                    x,
                                    y,
                                    cell.minAlt,
                                    cell.maxAlt,
                                    cell.averageAlt,
                                    bboxObj,
                                    0,
                                    0};
                        object.insert( { cell.objectID, newObject });
                    } else {
                        // an entry for this object exists
                        ObjProp& knownObj = search->second;
                        if (knownObj.minX >= x) {
                            const int y = ((arrayCount / pointInMem) + 0.)
                                + rasterOffsetY;
                            if ((knownObj.minX == x) && (knownObj.minY < y)) {
                            } else {
                                knownObj.minX = x;
                                knownObj.minY = y;
                            }
                        }
                        if (knownObj.maxAlt < cell.maxAlt)
                            knownObj.maxAlt = cell.maxAlt;

                        if (knownObj.minAlt > cell.minAlt)
                            knownObj.minAlt = cell.minAlt;

                        if (edge)
                          ++knownObj.edgeCellCount;

                        if (edge){
                          knownObj.edgeAltDiff =
                              (knownObj.edgeAltDiff *
                                  (knownObj.edgeCellCount - 1) + altDiff) /
                                  knownObj.edgeCellCount;
                        }

                        recalcBbox(knownObj.bbox, arrayCount, pointInMem);

                        const double altsumKO = knownObj.averageAlt
                                * knownObj.pointCount;
                        const double altsumC = cell.averageAlt * cell.count;
                        knownObj.averageAlt = (altsumKO + altsumC)
                                / (knownObj.pointCount + cell.count);
                        knownObj.pointCount += cell.count;
                        knownObj.cellCount++;
                    }
                 } //end if objectID > 0
                } //end if objectID != -1
                arrayCount++;
            }
        }
    } //end for
    if (RASTER_CLASSIFY_DEBUG) {
        //for (auto i : object) std::cout<<i.second.toString()<<endl;
        std::cout<<"Classify: Amount of different objects in map: "
                 <<std::to_string(object.size())<<endl;
        std::cout<<"Classify: Cells with no ObjectID: "
                 <<std::to_string(NoObjID)<<endl;
    }
    return object;
}

void ARClassify::findDuplicates(std::unordered_map<int, ObjProp> object,
                std::shared_ptr<std::unordered_map<int,int>> duple) {

    struct IDandY {
            int objectID;
            double minY;
    };

    std::unordered_map<int, std::vector<IDandY>> objectIDbyXvalue;
    size_t sameButNotEq = 0;

    for (auto i : object) {
        IDandY entry;
        entry.objectID = i.first;
        entry.minY = i.second.minY;
        auto search = objectIDbyXvalue.find(i.second.minX);
        if (search == objectIDbyXvalue.end()) {
            std::vector<IDandY> vec;
            vec.emplace_back(entry);
            objectIDbyXvalue.insert({i.second.minX,vec});
        } else {
            search->second.emplace_back(entry);
        }
    }
    for (auto j : objectIDbyXvalue) {
        std::vector<IDandY> & vec = j.second;
        if (vec.size() > 1) {
           for (size_t k = 0; k < vec.size(); k++) {
               for (size_t l = k + 1; l < vec.size(); l++) {
                   if (vec[k].minY == vec[l].minY) {
                      if (object.at(vec[k].objectID).
                              equals(object.at(vec[l].objectID))) {
                        duple.get()->insert({vec[l].objectID,vec[k].objectID});
                        object.erase(vec[l].objectID);
                        vec.erase(vec.begin()+l);
                      } else {
                          sameButNotEq++;
//                      std::cout<<"Classify: DuplicateFinder: ("
//                              <<std::to_string(j.first) <<"X/"
//                              <<std::to_string(vec[k].minY)<<"Y) notEq: "
//                                   <<object.at(vec[k].objectID).toString()
//                                   <<" and "
//                                   <<object.at(vec[l].objectID).toString()
//                                   <<endl;
                      }
                   }
               }
           }
        }
    }

    //DEBUG
    if (RASTER_CLASSIFY_DEBUG) {
        std::cout<<"Classify: DuplicateFinder: Number of found duplicates:"
             <<std::to_string(duple.get()->size())<<endl;
        std::cout<<"Classify: DuplicateFinder: same coords but different"
             <<" values: "<<std::to_string(sameButNotEq)<<endl;
//        try {
//            for (auto i : *duple.get()) {
//                std::cout<<"IDs: "<<std::to_string(i.first)<<"||";
//                std::cout<<std::to_string(i.second)<<" (";
//                try {
//                    std::cout<<std::to_string(object.at(i.second).minX)<<"X, "
//                       <<std::to_string(object.at(i.second).minY)<<"Y)"<<endl;
//                } catch(exception &e) {
//                    std::cout<<"Exception in: object.at("
//                           <<std::to_string(i.second)<<endl;
//                }
//            }
//        } catch(exception &e) {
//            std::cout<<"Exception in: for (auto i : *duple.get())"<<endl;
//        }
    }
}

void ARClassify::testForSameValueObjects(
        const std::unordered_map<int, ObjProp>& objects) {
    std::cout << "Classify: DuplicateFinder: Start Process" << endl;
    std::unordered_map<int, std::vector<int> > duples;
    for (auto i : objects) {
        for (auto j : objects) {
            if ((i.second.equals(j.second)) && (i.first != j.first)) {
                auto search = duples.find(i.first);
                if (search == duples.end()) {
                    std::vector<int> vec;
                    vec.emplace_back(j.first);
                    duples.insert( { i.first, vec });
                } else {
                    auto vecsearch = std::find(search->second.begin(),
                            search->second.end(), j.first);
                    if (vecsearch == search->second.end()) {
                        search->second.emplace_back(j.first);
                    }
                }
                search = duples.find(j.first);
                if (search == duples.end()) {
                    std::vector<int> vec;
                    vec.emplace_back(i.first);
                    duples.insert( { j.first, vec });
                } else {
                    auto vecsearch = std::find(search->second.begin(),
                            search->second.end(), i.first);
                    if (vecsearch == search->second.end()) {
                        search->second.emplace_back(i.first);
                    }
                }
//                //DEBUG
//                std::cout<<"Classify: DuplicateFinder: equalValues ID"
//                  <<std::to_string(j.first)
//                  <<" ("<<std::to_string(j.second.minX)<<"X/"
//                  <<std::to_string(j.second.minY)<<"Y) "
//                   <<j.second.toString()<<" and ID"
//                   <<std::to_string(i.first)
//                 <<" ("<<std::to_string(i.second.minX)<<"X/"
//                 <<std::to_string(i.second.minY)<<"Y) "
//                  <<i.second.toString()<<endl;
            }
        }
    }
    std::cout<< "Classify: DuplicateFinder: following Object IDs have"
            " same values:"<< endl;
    for (auto i : duples) {
        std::cout << std::to_string(i.first) << "||";
        for (auto j : i.second) {
            std::cout << std::to_string(j) << "||";
            duples.erase(j);
        }
        std::cout << endl;
    }
}

void ARClassify::normalize(const std::unordered_map<int, ObjProp>& objects,
                            double adjust[fvd]) {
    const int objectNumber = objects.size();
    // 2.2.1 calculate standard deviation
    double sd[fvd] { 0, 0, 0, 0, 0, 0, 0};
    double mean[fvd] { 0, 0, 0, 0, 0, 0, 0};
    double sum[fvd] { 0, 0, 0, 0, 0, 0, 0};
    for (auto i : objects) {
        ObjProp& obj = i.second;
        double areaPerDiameter = (double)obj.cellCount /
            std::sqrt(std::pow(obj.bbox.MaxD(0) - obj.bbox.MinD(0) + 1.0, 2) +
                std::pow(obj.bbox.MaxD(1) - obj.bbox.MinD(1) + 1.0, 2));
        sum[0] += (double) (obj.pointCount);
        sum[1] += (double) (obj.cellCount);
        sum[2] += obj.maxAlt - obj.minAlt;
        sum[3] += obj.averageAlt;
        sum[4] += obj.edgeAltDiff;
        sum[5] += (double) obj.edgeCellCount / (obj.bbox.Perimeter() + 4);
        sum[6] += areaPerDiameter;
     }
    for (size_t i = 0; i < fvd; i++) {
        mean[i] = sum[i] / objectNumber;
    }
    for (auto i : objects) {
        ObjProp& obj = i.second;
        sd[0] += pow(obj.pointCount - mean[0], 2);
        sd[1] += pow(obj.cellCount - mean[1], 2);
        sd[2] += pow(obj.maxAlt - obj.minAlt - mean[2], 2);
        sd[3] += pow(obj.averageAlt - mean[3], 2);
        sd[4] += pow(obj.edgeAltDiff - mean[4], 2);
        sd[5] += pow((double) obj.edgeCellCount /
            (obj.bbox.Perimeter() + 4) - mean[5], 2);
        sd[6] += pow((double)obj.cellCount /
            std::sqrt(std::pow(obj.bbox.MaxD(0) - obj.bbox.MinD(0) + 1.0, 2)+
                std::pow(obj.bbox.MaxD(1) - obj.bbox.MinD(1) + 1.0, 2))
                - mean[4], 2);
    }
    for (size_t i = 0; i < fvd; i++) {
        sd[i] = sqrt(sd[i] / objectNumber);
        //DEBUG
        if (RASTER_CLASSIFY_DEBUG) {
            std::cout << "Classify: Standard Deviation: dim("
                    << std::to_string(i) << ") " << std::to_string(sd[i])
                    << endl;
        }
    }
    //2.2.2 calculate factor for adjustment
    for (size_t i = 0; i < fvd; i++) {
        adjust[i] = 1;
        if (sd[i] > 10) {
            while (sd[i] > 10) {
                sd[i] *= 0.1;
                adjust[i] *= 0.1;
            }
        } else {
            if (sd[i] < 1) {
                while (sd[i] < 1) {
                    sd[i] *= 10;
                    adjust[i] *= 10;
                }
            }
        }
        while (sd[i] > Pointcloud2::RASTER_CLASSIFY_NORMALIZE) {
            sd[i] *= 0.9;
            adjust[i] *= 0.9;
        }
        //DEBUG
        if (RASTER_CLASSIFY_DEBUG) {
            std::cout << "Classify: adjustment factor: dim("
                    << std::to_string(i) << ") " << std::to_string(adjust[i])
                    << endl;
        }
    } //end for
}

void ARClassify::createDbScanVector(
             std::shared_ptr<std::vector<DbScanPoint<fvdMax> > > dbScanObjects,
             std::unordered_map<int, ObjProp>& objects,
             const double adjust[fvd],
             size_t& ScanObjectIndex) {

    dbScanObjects.get()->resize(objects.size() + 1);
    std::bitset<fvd> switchFeatures(Pointcloud2::SWITCH_FEATURES);
    const size_t fvdS = switchFeatures.count();
    std::cout<<"Chosen "<<fvdS<<" features for classify."<<endl;
    if (switchFeatures.test(0)){
      std::cout<<"- pointCount"<<endl;
    }
    if (switchFeatures.test(1)){
      std::cout<<"- cellCount"<<endl;
    }
    if (switchFeatures.test(2)){
      std::cout<<"- MaxAlt - MinAlt"<<endl;
    }
    if (switchFeatures.test(3)){
      std::cout<<"- averageAlt"<<endl;
    }
    if (switchFeatures.test(4)){
      std::cout<<"- altDiff at edge"<<endl;
    }
    if (switchFeatures.test(5)){
      std::cout<<"- edgeCellCount / perimeter bbox"<<endl;
    }
    if (switchFeatures.test(6)){
      std::cout<<"- cellCount / diameter bbox"<<endl;
    }
    if (fvdS > fvdMax){
      std::cout<<"The last "<<fvdS - fvdMax;
      std::cout<<" feature(s) are ignored "
          "because exceeding maximum for dbscan"<<endl;
    }
    for (auto i : objects) {
        DbScanPoint<fvdMax>& scanObject = dbScanObjects.get()->at(
                ++ScanObjectIndex);
        ObjProp& o = i.second;
        o.scanIndex = ScanObjectIndex;
        objects[i.first] = o;
        double coords[fvdMax] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        size_t coordsSet = 0;
        if (switchFeatures.test(0)){
          coords[0] = (double) (o.pointCount) * adjust[0];
          ++coordsSet;
        }
        if (switchFeatures.test(1)){
          coords[coordsSet] = (double) (o.cellCount) * adjust[1];
          ++coordsSet;
        }
        if (switchFeatures.test(2)){
          coords[coordsSet] = (o.maxAlt - o.minAlt) * adjust[2];
          ++coordsSet;
        }
        if (switchFeatures.test(3)){
          coords[coordsSet] = o.averageAlt * adjust[3];
          ++coordsSet;
        }
        if (switchFeatures.test(4)){
          coords[coordsSet] = (double) (o.edgeAltDiff) * adjust[4];
          ++coordsSet;
        }
        if (switchFeatures.test(5)){
          coords[coordsSet] = ((double) o.edgeCellCount /
              (o.bbox.Perimeter() + 4)) * adjust[5];
          ++coordsSet;
        }
        if (coordsSet < fvdMax && switchFeatures.test(6)){
          coords[coordsSet] = (double)o.cellCount /
              std::sqrt(std::pow(o.bbox.MaxD(0) - o.bbox.MinD(0) + 1.0, 2) +
                  std::pow(o.bbox.MaxD(1) - o.bbox.MinD(1) + 1.0, 2)) *
                  adjust[6];
        }
        scanObject.setCoords(coords);
    }
}

std::shared_ptr<std::unordered_map<int,size_t>>
ARClassify::classify (std::vector<SmiFileId> rasters,
                            std::vector<Rect3> rastersBbox,
                            const size_t pointInMem,
                            std::shared_ptr<std::unordered_map<int,int>> duple){

    if (RASTER_CLASSIFY_DEBUG) {
            std::cout<<"Classify: Start!"<<endl;
    }

    // defines the number of properties for classification process
    // properties: o.pointCount, o.cellCount, o.maxAlt-o.minAlt, o.averageAlt
    //const size_t fvd = 7; // (fvd =feature vector dimensions)

    // 1. Creating a list of all calculated objects and their properties.

    std::unordered_map<int, ObjProp> objects = createObjectMap(
                                            rasters, rastersBbox, pointInMem);


    // 2. Find duplicate entries in object list (with different ObjectIDs)

//    if (rasters.size() > 1) {//only necessary when more then one raster
//        findDuplicates(objects,duple);
//    } else {
//        if (RASTER_CLASSIFY_DEBUG) {
//            std::cout<<"Classify:no overlapping rasters -> "
//                "no search for duplicates"<<endl;
//        }
//    }
//    if (RASTER_CLASSIFY_FINDDUPLE_EXTREME){
//        testForSameValueObjects(objects);
//    }


    // 3. Check if there are enough objects to proceed.

    std::shared_ptr<std::unordered_map<int,size_t>> result =
                        std::make_shared<std::unordered_map<int,size_t>>();
    if (objects.size() <= Pointcloud2::RASTER_CLASSIFY_MINPTS+1) {
        std::cout<<"Classify: There is no classification process, because the"
                "number of objects is smaller or equal to the parameter for"
                " minimal clustersize (RASTER_CLASSIFY_MINPTS + 1)"<<endl;

        for (auto i : objects) {
            result.get()->insert({
                    i.first,
                    0
            });
        }
        return result;
    }

    //4. Creating a Vector of dbScanPoints, with one DbScanPoint for each
    //Object. The properties that are wanted as part of the classification
    //process, are getting the coords of the dbscanpoints (= feature vector).

    // 4.1  normalize values
    double adjust[fvd] {1.0,1.0,1.0,1.0,1.0,1.0,1.0};
    if ((Pointcloud2::RASTER_CLASSIFY_NORMALIZE > 0)
            && (objects.size() > 1)) {
        normalize(objects, adjust);
    }


    // 4.2  creating vector
    std::shared_ptr<std::vector<DbScanPoint<fvdMax> > > dbScanObjects =
            std::make_shared<std::vector<DbScanPoint<fvdMax> > >();
    size_t ScanObjectIndex = 0;

    createDbScanVector(dbScanObjects, objects, adjust, ScanObjectIndex);


    // 5. Run DbScan
    double eps = Pointcloud2::RASTER_CLASSIFY_EPSILON;
    size_t minpts = Pointcloud2::RASTER_CLASSIFY_MINPTS;
    size_t maxClusterCount = 0;
    double maxClusterCountEps;
    size_t maxNoise = 0;


    // 5.1 Find best epsilon value
    std::string scanresult[Pointcloud2::RASTER_CLASSIFY_SCANSERIES];
    for (size_t i = 0; i < Pointcloud2::RASTER_CLASSIFY_SCANSERIES; i++) {
        //DEBUG
        std::cout<<"Classify: RASTER_CLASSIFY_EPSILON: "
                <<std::to_string(eps)<<endl;
        std::cout<<"Classify: RASTER_CLASSIFY_MINPTS: "
                <<std::to_string(minpts)<<endl;
        for (size_t j = 1; j <= ScanObjectIndex; j++)
            dbScanObjects.get()->at(j).initialize(true);
        DbScan<fvdMax> dbScan(
                    eps,
                    minpts,
                    dbScanObjects,
                    true);
        dbScan.run();
        if ((dbScan.getClusterCount() > maxClusterCount)
                || ((dbScan.getClusterCount() == maxClusterCount)
                        && (dbScan.getNoiseCount() > maxNoise))) {
            maxClusterCount = dbScan.getClusterCount();
            maxClusterCountEps = eps;
            maxNoise = dbScan.getNoiseCount();
        }
        scanresult[i] = "Scan" + std::to_string(i)
                + " eps: " + std::to_string(eps)
                + " Cluster: " + std::to_string(dbScan.getClusterCount())
                + " Noise: " + std::to_string(dbScan.getNoiseCount());
        eps += Pointcloud2::RASTER_CLASSIFY_SCANSERIES_ADD;
    }

    // 5.2 Final DbScan if necessary
    if ((Pointcloud2::RASTER_CLASSIFY_SCANSERIES > 1)
            && (maxClusterCount > 0)
            && (maxClusterCountEps !=
                    (eps - Pointcloud2::RASTER_CLASSIFY_SCANSERIES_ADD))){
        //DEBUG
        std::cout<<"Classify: Epsilon "
                <<std::to_string(maxClusterCountEps)<<" with MaxClusterCount: "
                <<std::to_string(maxClusterCount)<<endl;
        for (size_t j = 1; j <= ScanObjectIndex; j++)
            dbScanObjects.get()->at(j).initialize(true);
        DbScan<fvdMax> dbScan(
                    maxClusterCountEps,
                    minpts,
                    dbScanObjects,
                    true);
        dbScan.run();
    }

    // 5.3 Scanresults to screen
    for (size_t i = 0; i<Pointcloud2::RASTER_CLASSIFY_SCANSERIES;i++)
        std::cout<<scanresult[i]<<endl;

    // 6. write back the now found classIDs

    for (auto i : objects) {
        result.get()->insert({
                i.first,
                dbScanObjects.get()->at(i.second.scanIndex)._clusterId
        });
//        //DEBUG
//        std::cout<<"Classify: ObjectID: "
//                 <<std::to_string(i.first)
//                 <<"  ClassID: "
//                 <<std::to_string(dbScanObjects.get()->
//                         at(i.second.scanIndex)._clusterId)
//                 <<" ScanIndex: "<<std::to_string(i.second.scanIndex)<<endl;
    }

    return result;

}

void
ARClassify::recalcBbox(Rect& bboxObj, size_t nF, size_t pointInMem){

  double minX = bboxObj.MinD(0);
  double maxX = bboxObj.MaxD(0);
  double minY = bboxObj.MinD(1);
  double maxY = bboxObj.MaxD(1);

  if (minX > (nF % pointInMem)){
    minX = (nF % pointInMem) + 0.;
  }
  if (maxX < (nF % pointInMem)){
    maxX = (nF % pointInMem) + 0.;
  }
  if (minY > (nF / pointInMem)){
    minY = (nF / pointInMem) + 0.;
  }
  if (maxY < (nF / pointInMem)){
    maxY = (nF / pointInMem) + 0.;
  }
  double min[] = {minX, minY};
  double max[] = {maxX, maxY};
  bboxObj.Set(true, min, max);
}




