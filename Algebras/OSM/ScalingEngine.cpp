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

This implementation file contains the implementation of the class ~ScalingEngine~.

For more detailed information see OsmAlgebra.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "ScalingEngine.h"

// --- Defining class-variables
int ScalingEngine::DEFAULT_SCALE_FACTOR_X = 1;//1000;
int ScalingEngine::DEFAULT_SCALE_FACTOR_Y = 1;//1000;

// --- Constructors
// Default-Constructor
ScalingEngine::ScalingEngine ()
  : scaleFactorX (DEFAULT_SCALE_FACTOR_X),
    scaleFactorY (DEFAULT_SCALE_FACTOR_Y)
{

}

// Copy-Constructor
ScalingEngine::ScalingEngine (const ScalingEngine &)
{
    // empty
}

// Destructor
ScalingEngine::~ScalingEngine ()
{
    // empty
}

ScalingEngine & ScalingEngine::operator= (const ScalingEngine &)
{
   // empty
   return (*this);
}

// --- Class-functions
ScalingEngine & ScalingEngine::getInstance ()
{
    static ScalingEngine instance;
    return instance;
}

// --- Methods
void ScalingEngine::setScaleFactorX (int scaleFactorX)
{
    this->scaleFactorX = scaleFactorX;
}

void ScalingEngine::setScaleFactorY (int scaleFactorY)
{
    this->scaleFactorY = scaleFactorY;
}

int ScalingEngine::getScaleFactorX () const
{
    return this->scaleFactorX;
}

int ScalingEngine::getScaleFactorY () const
{
    return this->scaleFactorY;
}
