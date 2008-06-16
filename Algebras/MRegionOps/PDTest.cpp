/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

*/

void PUnit::ComputeBoundingRect() {

    // Calculate the projection bounding rectangle in the (x, y)-plane
    // of the URegion:
    
    double minX = uRegion->BoundingBox().MinD(0);
    double maxX = uRegion->BoundingBox().MaxD(0);
    double minY = uRegion->BoundingBox().MinD(1);
    double maxY = uRegion->BoundingBox().MaxD(1);
    
    boundingRect = Rectangle<2>(true, minX, maxX, minY, maxY);
}
