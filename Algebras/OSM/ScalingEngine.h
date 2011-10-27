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

[1] Header File of the ScalingEngine

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class ~ScalingEngine~.

2 Defines and includes

*/
// [...]
#ifndef __SCALING_ENGINE_H__
#define __SCALING_ENGINE_H__

// --- Including header-files

class ScalingEngine {

private:

    // --- Constructors
    // Default-Constructor
    ScalingEngine ();
    // Copy-Constructor
    ScalingEngine (const ScalingEngine &);
    // Destructor
    ~ScalingEngine ();

    // Assignment-operator
    ScalingEngine & operator= (const ScalingEngine &);

public:
    // --- Class-functions
    static ScalingEngine & getInstance ();

    //  --- Methods
    void setScaleFactorX (int scaleFactorX);
    void setScaleFactorY (int scaleFactorY);
    int getScaleFactorX () const;
    int getScaleFactorY () const;

private:

    // --- Members    
    int scaleFactorX;
    int scaleFactorY;
    static int DEFAULT_SCALE_FACTOR_X;
    static int DEFAULT_SCALE_FACTOR_Y;

};

#endif /* __SCALING_ENGINE_H__ */
