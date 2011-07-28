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

[1] Header File of the OSM Algebra

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class ~ShpFileReader~.

2 Defines and includes

*/
#ifndef __CONN_CODE_FINDER_H__
#define __CONN_CODE_FINDER_H__

// --- Including header-files
//...

class ConnCodeFinder {

public:

   // --- Constructors
   // Default-Constructor
   ConnCodeFinder ();
    // Destructor
    ~ConnCodeFinder ();

   // --- Methods
   // Returns the connectivity code for a junction between up to four sections
   //      |
   //      |-dir2
   // dir1 |
   //  |   |
   // -----x-----
   //      |   |
   //      | dir3
   // dir4-|
   //      |
   // dir1, dir2, dir3, dir4 represent the directions of the sections
   // that meet in the junction
   // - 0 is used when the section does not exist,
   // - 1 is used for positive direction (analogous to x- and y-axis in a 
   //   cartesian coordinate system)
   // - 2 is used for negative directions
   static int getConnectivityCode (int dir1, int dir2, int dir3, int dir4,
                                   bool ow1, bool ow2, bool ow3, bool ow4);

   static void printAllConnectivityCodes ();

};

#endif /* __CONN_CODE_FINDER_H__ */
