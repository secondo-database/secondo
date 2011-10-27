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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the OSM Algebra

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This implementation file contains the implementation of the class ~ShpFileReader~.

For more detailed information see ShpFileReader.h.

2 Defines and Includes

*/
// [...]
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "ShpFileReader.h"
#include "../Spatial/SpatialAlgebra.h"
#include "RegionTools.h"
#include "../FText/FTextAlgebra.h"
#include "ScalingEngine.h"
using namespace std;

// --- Constructors
// Default-Constructor
ShpFileReader::ShpFileReader ()
{
    // please, use the other constructor instead
    assert (false);
}

// Constructor
ShpFileReader::ShpFileReader (const ListExpr allowedType1, const FText* fname) {
    int allowedType = -1;
    if (listutils::isSymbol (allowedType1,Point::BasicType ())) {
        allowedType=1;
    //} else  if (listutils::isSymbol (allowedType1,Line::BasicType ())) {
    } else  if (listutils::isSymbol (allowedType1,SimpleLine::BasicType ())) {
        allowedType=3;
    } else  if (listutils::isSymbol (allowedType1,Region::BasicType ())) {
        allowedType=5;
    } else  if (listutils::isSymbol (allowedType1,Points::BasicType ())) {
        allowedType=8;
    }
    
    if (!fname->IsDefined ()) {
        defined = false;
    } else {
        defined = true;
        string name = fname->GetValue ();
        file.open (name.c_str (),ios::binary);
        if (!file.good ()) {
            defined = false;
            file.close ();
        } else {
            defined = readHeader (allowedType);
            if (!defined) {
                file.close ();
            } else {
                file.seekg (100,ios::beg); // to the first data set
            }
        }
    }
}

// Destructor
ShpFileReader::~ShpFileReader ()
{
    // empty
}

// --- Methods
void ShpFileReader::close () {
    if (defined) {
        file.close ();
        defined = false;
    }
}

Attribute* ShpFileReader::getNext () {
    if (!defined) {
        return 0;
    }
    switch (type) {
        case 1: return getNextPoint ();
        //case 3: return getNextLine ();
        case 3: return getNextSimpleLine ();
        case 5: return getNextPolygon ();
        case 8: return getNextMultiPoint ();
        default: return 0;
    }
}

Attribute* ShpFileReader::getNextSimpleLine () {
    uint32_t len = 0;
    uint32_t type = 0;
    uint32_t numParts = 0;
    uint32_t numPoints = 0;
    uint32_t part = 0;
    int numElems = 0;
    int jStart = 0;
    int jEnd = 0;
    int numEdges = -1;
    unsigned int i = 0;
    int j = 0;
    double x = 0.;
    double y = 0.;
    vector<int> parts;
    SimpleLine* line = NULL;
    Point startPoint;
    Point endPoint;
    Point p1;
    Point p2;
    HalfSegment hs;

    // Testing if the file is empty
    if (file.tellg ()==fileend) {
        return 0;
    }

    // Skipping recNo
    readBigInt32 ();
    // Fetching the length
    len = readBigInt32 ();
    // Yielding the type
    type = readLittleInt32 ();

    // --- Checking the type
    if (type == 0) {
        if (len != 2) {
            cerr << "Error in file detected" << endl;
            file.close ();
            defined = false;
            return 0;
        } else {
            return new SimpleLine (1);
        }
    }
    // a non-null line
    if (type != 3) {
        cerr << "Error in file detected" << endl;
        file.close ();
        defined = false;
        return 0;
    }

    // --- Ignoring box
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();

    // --- Getting the number of parts (lines) and the number of points
    numParts  = readLittleInt32 ();
    numPoints = readLittleInt32 ();

    // Collecting the parts (start of line indexes)
    for (i = 0; i < numParts && file.good () ; ++i) {
        part = readLittleInt32 ();
        parts.push_back (part);
    }

    // Testing whether an error occurred
    if (!file.good ()) {
        cerr << "error in reading file" << endl;
        file.close ();
        defined = false;
        return 0;
    }

    // Preparing the geometric structure
    line = new SimpleLine (numPoints);

    // --- Inserting the data
    line->StartBulkLoad ();
    for (i = 0; i < parts.size () && file.good (); ++i) {
        jStart = numElems;
        jEnd = (i == parts.size () - 1)? numPoints : parts[i + 1];
        for (j = jStart; j < jEnd && file.good () ; ++j) {
            x = readLittleDouble () * 
                ScalingEngine::getInstance ().getScaleFactorX ();
            y = readLittleDouble () *
                ScalingEngine::getInstance ().getScaleFactorY ();
            p2.Set (x,y);
            if (j > jStart) {
                if (!AlmostEqual (p1,p2)) {
                    hs.Set ( true, p1, p2 );
                    hs.attr.edgeno = ++numEdges;
                    (*line) += hs;
                    hs.SetLeftDomPoint ( !hs.IsLeftDomPoint () );
                    (*line) += hs;
                }
                // Storing the end point of the polyline
                //if (i == parts.size () - 1 && j == jEnd - 1)  {
                //    endPoint.Set(x, y);
                //}
                // Storing the end point of the polyline (considering circles)
                if (!AlmostEqual (startPoint, p2))  {
                    endPoint.Set(x, y);
                }
            } else  {
                // Storing the start point of the polyline
                if (i == 0 /*&& j == jStart*/)  {
                    startPoint.Set (x, y);
                }
            }
            p1 = p2;
            ++numElems;
        }
    }
    line->EndBulkLoad ();
    if (startPoint < endPoint)  {
        line->SetStartSmaller (true);
    } else if (startPoint > endPoint)  {
        line->SetStartSmaller (false);
    } else  {
        // street only consists of almost equal points
        assert (false);
    }
    // Checking if errors occurred    
    if (!file.good ()) {
        cerr << "Error in reading file" << endl;
        delete line;
        file.close ();
        defined = false;
        return 0;
    }

    return line;
}

Attribute* ShpFileReader::getNextLine () {
    uint32_t len = 0;
    uint32_t type = 0;
    uint32_t numParts = 0;
    uint32_t numPoints = 0;
    uint32_t part = 0;
    int numElems = 0;
    int jStart = 0;
    int jEnd = 0;
    int numEdges = -1;
    unsigned int i = 0;
    int j = 0;
    double x = 0.;
    double y = 0.;
    vector<int> parts;
    Line* line = NULL;
    Point p1;
    Point p2;
    HalfSegment hs;

    // Testing if the file is empty
    if (file.tellg ()==fileend) {
        return 0;
    }

    // Skipping recNo
    readBigInt32 ();
    // Fetching the length
    len = readBigInt32 ();
    // Yielding the type
    type = readLittleInt32 ();

    // --- Checking the type
    if (type == 0) {
        if (len != 2) {
            cerr << "Error in file detected" << endl;
            file.close ();
            defined = false;
            return 0;
        } else {
            return new Line (1);
        }
    }
    // a non-null line
    if (type != 3) {
        cerr << "Error in file detected" << endl;
        file.close ();
        defined = false;
        return 0;
    }

    // --- Ignoring box
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();

    // --- Getting the number of parts (lines) and the number of points
    numParts  = readLittleInt32 ();
    numPoints = readLittleInt32 ();

    // Collecting the parts (start of line indexes)
    for (i = 0; i < numParts && file.good () ; ++i) {
        part = readLittleInt32 ();
        parts.push_back (part);
    }

    // Testing whether an error occurred
    if (!file.good ()) {
        cerr << "error in reading file" << endl;
        file.close ();
        defined = false;
        return 0;
    }

    // Preparing the geometric structure
    line = new Line (numPoints);

    // --- Inserting the data
    line->StartBulkLoad ();
    for (i = 0; i < parts.size () && file.good (); ++i) {
        jStart = numElems;
        jEnd = (i == parts.size () - 1)? numPoints : parts[i + 1];
        for (j = jStart; j < jEnd && file.good () ; ++j) {
            x = readLittleDouble ();
            y = readLittleDouble ();
            p2.Set (x,y);
            if (j > jStart) {
                if (!AlmostEqual (p1,p2)) {
                    hs.Set ( true, p1, p2 );
                    hs.attr.edgeno = ++numEdges;
                    (*line) += hs;
                    hs.SetLeftDomPoint ( !hs.IsLeftDomPoint () );
                    (*line) += hs;
                }
            }
            p1 = p2;
            ++numElems;
        }
    }
    line->EndBulkLoad ();
    // Checking if errors occurred    
    if (!file.good ()) {
        cerr << "Error in reading file" << endl;
        delete line;
        file.close ();
        defined = false;
        return 0;
    }

    return line;
}

Attribute* ShpFileReader::getNextPoint () {
    uint32_t recLen = 0;
    uint32_t type = 0;
    double x = 0.;    
    double y = 0.;
    if (file.tellg () == fileend) {
        return 0;
    }
    // Skipping the record number
    readBigInt32 ();
    recLen = readBigInt32 ();
    type = readLittleInt32 ();
    if (type == 0) { // null shape
        if (recLen!=2 || !file.good ()) {
            cerr << "Error in shape file detected " << __LINE__ << endl;
            defined = false;
            file.close ();
            return 0;
        } else {
            return new Point (false,0,0);
        }
    }
    if (type != 1 || recLen != 10) {
        cerr << "Error in shape file detected " << __LINE__ << endl;
        defined = false;
        file.close ();
        return 0;
    }
    x = readLittleDouble () *
       ScalingEngine::getInstance ().getScaleFactorX ();
    y = readLittleDouble () *
       ScalingEngine::getInstance ().getScaleFactorY ();
    if (!file.good ()) {
        cerr << "Error in shape file detected " << __LINE__ << endl;
        defined = false;
        file.close ();
        return 0;
    }
    return new Point (true,x,y);
}

Attribute* ShpFileReader::getNextMultiPoint () {
    uint32_t recNo = 0;
    uint32_t len = 0;
    uint32_t type = 0;
    uint32_t numPoints = 0;
    uint32_t expectedLen = 0;
    Points* ps = NULL;
    Point p;

    if (file.tellg ()==fileend) {
        return 0;
    }
    recNo =  readBigInt32 ();
    len = readBigInt32 ();
    type = readLittleInt32 ();

    if (!file.good ()) {
        cerr << " problem in reading file " << __LINE__ << endl;
        cerr << "recNo = " << recNo << endl;
        cerr << "len = " << len << endl;
        cerr << "type = " << type << endl;
        defined = false;
        return 0;
    }

    if (type==0) {
        if (len!=2) {
            cerr << "Error in shape file detected " << __LINE__ << endl;
            defined = false;
            file.close ();
            return 0;
        } else {
            return new Points (1);
        }
    }
    if (type!=8) {
        cerr << "Error in shape file detected " << __LINE__ << endl;
        cout << "type = " << type << endl;
        cout << "file.good = " << file.good () << endl;
        defined = false;
        file.close ();
        return 0;
    }
    // ignore Bounding box
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();
    numPoints = readLittleInt32 ();

    expectedLen = (40 + numPoints*16) / 2;
    if (len != (expectedLen)) {
        cerr << "Error in file " << __LINE__ << endl;
        cerr << "len = " << len << endl;
        cerr << "numPoints " << numPoints << endl;
        cerr << " expected" << expectedLen << endl;
        file.close ();
        defined = false;
        return 0;
    }
    ps = new Points (numPoints);
    ps->StartBulkLoad ();
    for (unsigned int i=0;i<numPoints && file.good ();i++) {
        double x = readLittleDouble ();
        double y = readLittleDouble ();
        p.Set (x,y);
        (*ps) += p;
    }
    ps->EndBulkLoad ();
    if (!file.good ()) {
        cerr << "Error in file " << __LINE__ << endl;
        delete ps;
        return 0;
    } else {
        return ps;
    }
}

Attribute* ShpFileReader::getNextPolygon () {
    uint32_t clen = 0;
    uint32_t pos = 0;
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t numParts = 0;
    uint32_t numPoints = 0;
    uint32_t len = 0;
    uint32_t type = 0;
    unsigned int iPart = 0;
    unsigned int c = 0;
    double x = 0.;
    double y = 0.;
    vector<uint32_t> parts;
    vector<vector <Point> > cycles;
 
    if (file.tellg ()==fileend) { // end of file reached
        return 0;
    }

    readBigInt32 (); // ignore record number
    len = readBigInt32 ();
    type = readLittleInt32 ();
    if (type==0) {
        if (len!=2) {
            cerr << "Error in file detected" << __LINE__  <<  endl;
            file.close ();
            defined = false;
            return 0;
        } else { // NULL shape, return an empty region
            return new Region (0);
        }
    }
    if (type!=5) { // different shapes are not allowed
        cerr << "Error in file detected" << __LINE__ << endl;
        cerr << "Expected Type = 5, but got type: " << type << endl;
        file.close ();
        defined = false;
        return 0;
    }
    // ignore box
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();
    readLittleDouble ();
    numParts  = readLittleInt32 ();
    numPoints = readLittleInt32 ();

    // for debugging the file
    clen = (44 + 4*numParts + 16*numPoints)/2;
    if (clen!=len) {
        cerr << "File invalid: length given in header seems to be wrong"
            << endl;
        file.close ();
        defined = false;
        return 0;
    }

    // read the starts of the cycles
    for (iPart = 0; iPart < numParts; ++iPart) {
        parts.push_back (readLittleInt32 ());
    }

    // read the cycles
    for (iPart = 0;iPart < numParts; ++iPart) {
        vector<Point> cycle;
        start = pos;
        end = iPart< (numParts - 1) ? parts[iPart + 1]:numPoints;
        Point lastPoint (true,0.0,0.0);
        // read a single cycle
        for (c = start; c < end; ++c) {
            x = readLittleDouble ();
            y = readLittleDouble ();
            Point point (true,x,y);
            if (c==start) { // the first point
                cycle.push_back (point);
                lastPoint = point;
            } else if (!AlmostEqual (lastPoint,point)) {
                cycle.push_back (point);
                lastPoint = point;
            }
            ++pos;
        }
        cycles.push_back (cycle);
    }
    return buildRegion (cycles);
}

bool ShpFileReader::readHeader (unsigned int allowedType) {
    uint32_t code;
    uint32_t version;
    file.seekg (0,ios::end);
    streampos p = file.tellg ();
    fileend = p;
    if (p< 100) { // minimum size not reached
        return false;
    }
    file.seekg (0,ios::beg);
    file.read (reinterpret_cast<char*>(&code),4);
    file.seekg (28,ios::beg);
    file.read (reinterpret_cast<char*>(&version),4);
    file.read (reinterpret_cast<char*>(&type),4);
    if (WinUnix::isLittleEndian ()) {
        code = WinUnix::convertEndian (code);
    } else {
        version = WinUnix::convertEndian (version);
        type = WinUnix::convertEndian (type);
    }
    if (code!=9994) {
        return false;
    }
    if (version != 1000) {
        return false;
    }
    return type==allowedType;
}

uint32_t ShpFileReader::readBigInt32 () {
    uint32_t res;
    file.read (reinterpret_cast<char*>(&res),4);
    if (WinUnix::isLittleEndian ()) {
        res = WinUnix::convertEndian (res);
    }
    return res;
}

uint32_t ShpFileReader::readLittleInt32 () {
    uint32_t res;
    file.read (reinterpret_cast<char*>(&res),4);
    if (!WinUnix::isLittleEndian ()) {
        res = WinUnix::convertEndian (res);
    }
    return res;
}

double ShpFileReader::readLittleDouble () {
    uint64_t tmp;
    double res;
    file.read (reinterpret_cast<char*>(&tmp),8);
    if (!WinUnix::isLittleEndian ()) {
        tmp = WinUnix::convertEndian (tmp);
    }
    res = * (reinterpret_cast<double*>(&tmp));
    return res;
}

string ShpFileReader::getShpType(const string fname, bool& correct,
                                 string& errorMessage) {
  correct = false;
  if(fname.length()==0){
     errorMessage = "invalid filename";
     return "";
  }

  ifstream f(fname.c_str(),std::ios::binary);
  if(!f.good()){
     errorMessage = "problem in reading file";
     return "";
  }

  f.seekg(0,ios::end);
  streampos flen = f.tellg();
  if(flen < 100){
     errorMessage =  "not a valid shape file";
     f.close();
     return "";
  }


  f.seekg(0,ios::beg);
  uint32_t code = 0;
  f.read(reinterpret_cast<char*>(&code),4);
  if(WinUnix::isLittleEndian()){
     code = WinUnix::convertEndian(code);
  }
  if(code!=9994){
     errorMessage = "invalid file code  detected";
     f.close();
     return "";
  }

  uint32_t version;
  f.seekg(28,ios::beg);
  f.read(reinterpret_cast<char*>(&version),4);
  if(!WinUnix::isLittleEndian()){
      version = WinUnix::convertEndian(version);
  }
  if(version != 1000){
    errorMessage = "invalid version detected";
    f.close();
    return "";
  }
  uint32_t type;
  f.read(reinterpret_cast<char*>(&type),4);
  if(!WinUnix::isLittleEndian()){
    type = WinUnix::convertEndian(type);
  }
  f.close();

  switch(type){
    case 0 : { errorMessage = "null shape, no corresponding secondo type";
               return "";
             }
    case 1 : { correct = true;
               return Point::BasicType();
             }
    case 3 : { correct = true;
               return SimpleLine::BasicType();
             }
    case 5 : { correct = true;
               return Region::BasicType();
             }
    case 8 : { correct = true;
               return Points::BasicType();
             }
    case 11 : { errorMessage = "PointZ, no corresponding secondo type";
               return "";
             }
    case 13 : { errorMessage = ("PolyLineZ, no corresponding secondo type");
               return "";
             }
    case 15 : { errorMessage = ("PolygonZ, no corresponding secondo type");
               return "";
             }
    case 18 : { errorMessage=("MultiPointZ, no corresponding secondo type");
               return "";
             }
    case 21 : { errorMessage=("PointM, no corresponding secondo type");
               return "";
             }
    case 23 : { errorMessage =("PolyLineM, no corresponding secondo type");
               return "";
             }
    case 25 : {errorMessage=("PolygonM, no corresponding secondo type");
               return "";
             }
    case 28 : { errorMessage=("MultiPointM, no corresponding secondo type");
               return "";
             }
    case 31 : { errorMessage = ("MultiPatch, no corresponding secondo type");
               return "";
             }
    default : errorMessage = " not a valid shape type";
              return "";
  }

}
