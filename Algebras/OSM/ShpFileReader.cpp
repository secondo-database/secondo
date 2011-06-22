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

For more detailed information see OsmAlgebra.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "ShpFileReader.h"
#include "../Spatial/SpatialAlgebra.h"
#include "RegionTools.h"
#include "../FText/FTextAlgebra.h"
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
    } else  if (listutils::isSymbol (allowedType1,Line::BasicType ())) {
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
        case 3: return getNextLine ();
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
    if (file.tellg () == fileend) {
        return 0;
    }
    // uint32_t recNo =
    readBigInt32 ();
    uint32_t recLen = readBigInt32 ();
    uint32_t type = readLittleInt32 ();
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
    double x = readLittleDouble ();
    double y = readLittleDouble ();
    if (!file.good ()) {
        cerr << "Error in shape file detected " << __LINE__ << endl;
        defined = false;
        file.close ();
        return 0;
    }
    return new Point (true,x,y);
}

Attribute* ShpFileReader::getNextMultiPoint () {
    if (file.tellg ()==fileend) {
        return 0;
    }
    uint32_t recNo = 0;
    recNo =  readBigInt32 ();
    uint32_t len = 0;
    len = readBigInt32 ();
    uint32_t type = 0;
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
    uint32_t numPoints = readLittleInt32 ();

    uint32_t expectedLen = (40 + numPoints*16) / 2;
    if (len != (expectedLen)) {
        cerr << "Error in file " << __LINE__ << endl;
        cerr << "len = " << len << endl;
        cerr << "numPoints " << numPoints << endl;
        cerr << " expected" << expectedLen << endl;
        file.close ();
        defined = false;
        return 0;
    }
    Points* ps = new Points (numPoints);
    Point p;
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
    if (file.tellg ()==fileend) { // end of file reached
        return 0;
    }

    readBigInt32 (); // ignore record number
    uint32_t len = readBigInt32 ();
    uint32_t type = readLittleInt32 ();
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
    uint32_t numParts  = readLittleInt32 ();
    uint32_t numPoints = readLittleInt32 ();
    // for debugging the file
    uint32_t clen = (44 + 4*numParts + 16*numPoints)/2;
    if (clen!=len) {
        cerr << "File invalid: length given in header seems to be wrong"
            << endl;
        file.close ();
        defined = false;
        return 0;
    }
    // read the starts of the cycles
    vector<uint32_t> parts;
    for (unsigned int i=0;i<numParts;i++) {
        uint32_t p = readLittleInt32 ();
        parts.push_back (p);
    }

    // read the cycles
    vector<vector <Point> > cycles;
    uint32_t pos = 0;
    for (unsigned int p=0;p<parts.size (); p++) {
        vector<Point> cycle;
        uint32_t start = pos;
        uint32_t end = p< (parts.size ()-1) ? parts[p+1]:numPoints;
        Point lastPoint (true,0.0,0.0);
        // read a single cycle
        for (unsigned int c=start; c< end; c++) {
            double x = readLittleDouble ();
            double y = readLittleDouble ();
            Point point (true,x,y);
            if (c==start) { // the first point
                cycle.push_back (point);
                lastPoint = point;
            } else if (!AlmostEqual (lastPoint,point)) {
                cycle.push_back (point);
                lastPoint = point;
            }
            pos++;
        }
        cycles.push_back (cycle);
    }
    return buildRegion (cycles);
}

bool ShpFileReader::readHeader (unsigned int allowedType) {
    file.seekg (0,ios::end);
    streampos p = file.tellg ();
    fileend = p;
    if (p< 100) { // minimum size not reached
        return false;
    }
    file.seekg (0,ios::beg);
    uint32_t code;
    uint32_t version;
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
    file.read (reinterpret_cast<char*>(&tmp),8);
    if (!WinUnix::isLittleEndian ()) {
        tmp = WinUnix::convertEndian (tmp);
    }
    double res = * (reinterpret_cast<double*>(&tmp));
    return res;
}


