/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Definition and Implementation of the class ~Statistic~

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

This file contains the definition and implementation of the class
~Statistic~ which provides several attributes to count objects, 
events and time to analyze the algorithm and offers a method to print 
the result.

1 Defines and Includes

*/


#ifndef STATISTIC_H_
#define STATISTIC_H_

#endif /*STATISTIC_H_*/

using namespace std;
namespace temporalalgebra {

namespace mregionops {

/*
1 Class Statistic

*/

class Statistic {
    
public:
    
/*
1.1 Constructor

*/   
    
    Statistic() {

        Reset();
    }
    
/*
1.1 Reset

Reset all counters to zero.

*/
    
    void Reset() {
        
        noUnitsIn = 0;
        noUnitsResult = 0;

        noPFaceTotal = 0;
        noPFaceReducedByPBR = 0;

        noIntSegsTotal = 0;
        noBorderIntSegs = 0;

        noRelevantPFaces = 0;
        noMSegsOverall = 0;
        noMSegsValidOverall = 0;
        noMSegsSkippedOverall = 0;
        noMSegCriticalOverall = 0;
        decisionsByPlumblineOverall = 0;
        decisionsByEntirelyInOutOverall = 0;
        decisionsByAdjacencyOverall = 0;
        decisionsByDegenerationOverall = 0;
        
        NoTestRegionsCreated = 0;
        NoTestRegionsCacheHits = 0;
        
        durationCreatePFacesOverall = 0.0;
        durationComputeIntSegsOverall = 0.0;
        durationCollectRelevantPFacesOverall = 0.0;
        durationConstructResultUnitsOverall = 0.0;
        durationProcessNormalPFace = 0.0;
        durationProcessCriticalPFace = 0.0;
        durationEndBulkloadOfResultUnit = 0.0;
        durationConvertResultUnitToURegionEmb = 0.0; 
    }
    
/*
1.1 Print

Print the current state of all counters.

*/
    
    void Print() const {

        cout << "_______________________________________________________"
                << endl;
        cout << endl;
        cout << "Overall Statistic:" << endl;
        cout << "_______________________________________________________"
                << endl;
        cout << endl;

        cout << "Units of RefinementPartition (Input): " << noUnitsIn << endl;
        cout << "Units of Result (Output): " << noUnitsResult << endl;
        cout << endl;

        cout << "Step 1: Creation of PFaces" << endl;
        cout << endl;
        cout << "Time: " << durationCreatePFacesOverall << " seconds." << endl;
        cout << "PFaces created: " << noPFaceTotal << endl;
        cout << "_____________________________________________________" << endl;
        cout << endl;

        cout << "Step 2: Creation of IntersectionSegments" << endl;
        cout << endl;
        cout << "Time: " << durationComputeIntSegsOverall << " seconds."
                << endl;
        cout << "IntersectionSegments created: " << noIntSegsTotal
                - noBorderIntSegs << endl;
        cout << "PFaces involved: " << noPFaceReducedByPBR << endl;
        cout << "_____________________________________________________" << endl;
        cout << endl;

        cout << "Step 3a: Collect relevant PFaces" << endl;
        cout << endl;
        cout << "Time: " << durationCollectRelevantPFacesOverall << " seconds."
                << endl;
        cout << "PFaces relevant for the result: " << noRelevantPFaces << endl;
        cout << "Border Segments added: " << noBorderIntSegs
                << endl;
        cout << "_____________________________________________________" << endl;
        cout << endl;

        cout << "Step 3b: Construction of ResultUnits" << endl;
        cout << endl;
        cout << "Time total: " << durationConstructResultUnitsOverall
                << " seconds." << endl;
        cout << endl;

        cout << "  Part 1: Create and decide new MSegments" << endl;
        cout << "  Time: " << durationProcessNormalPFace
                + durationProcessCriticalPFace << " seconds." << endl;
        cout << "  MSegments created total: " << noMSegsOverall << endl;
        cout << "  MSegments part of result: " << noMSegsValidOverall << endl;
        cout << "  MSegments skipped: " << noMSegsSkippedOverall << endl;
        cout << "  MSegments critical: " << noMSegCriticalOverall << endl;
        cout << "  Decisions by plumbline: " << decisionsByPlumblineOverall
                << endl;
        cout << "  Decisions by PFace-relevance: "
                << decisionsByEntirelyInOutOverall << endl;
        cout << "  Decisions by adjacency: " << decisionsByAdjacencyOverall
                << endl;
        cout << "  Decisions by degeneration: "
                << decisionsByDegenerationOverall << endl;
        cout << "  Testregions created total: " << NoTestRegionsCreated << endl;
        cout << "  Testregions cachehits: " << NoTestRegionsCacheHits << endl;
        cout << endl;

        cout << "  Part 2: Build cycles of ResultUnits" << endl;
        cout << "  Time: " << durationEndBulkloadOfResultUnit << " seconds."
                << endl;
        cout << endl;

        cout << "  Part 3: Convert ResultUnits to URegionEmb" << endl;
        cout << "  Time: " << durationConvertResultUnitToURegionEmb
                << " seconds." << endl;

        cout << "_____________________________________________________" << endl;
        cout << endl;
    }

/*
1.1 Attributes

*/    
       
    unsigned int noUnitsIn;
    unsigned int noUnitsResult;
    
    unsigned int noPFaceTotal;
    unsigned int noPFaceReducedByPBR;
    
    unsigned int noIntSegsTotal;
    unsigned int noBorderIntSegs;
    
    unsigned int noRelevantPFaces;
    unsigned int noMSegsOverall;
    unsigned int noMSegsValidOverall;
    unsigned int noMSegsSkippedOverall;
    unsigned int noMSegCriticalOverall;
    unsigned int decisionsByPlumblineOverall;
    unsigned int decisionsByEntirelyInOutOverall;
    unsigned int decisionsByAdjacencyOverall;
    unsigned int decisionsByDegenerationOverall;
    
    unsigned int NoTestRegionsCreated;
    unsigned int NoTestRegionsCacheHits;
    
    double durationCreatePFacesOverall;
    double durationComputeIntSegsOverall;
    double durationCollectRelevantPFacesOverall;
    double durationConstructResultUnitsOverall;
    double durationConvertResultUnitToURegionEmb;
    double durationEndBulkloadOfResultUnit;
    double durationProcessNormalPFace;
    double durationProcessCriticalPFace;
    
};

}

}
