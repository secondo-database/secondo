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
// [...]
#ifndef __SHP_FILE_READER_H__
#define __SHP_FILE_READER_H__

// --- Including header-files
#include "NestedList.h"
#include <fstream>
#include <sys/stat.h>
#include <iostream>

class FText;
class Attribute;

class ShpFileReader {

public:
    // --- Constructors
    // Default-Constructor
    ShpFileReader ();
    // Constructor
    ShpFileReader (const ListExpr, const FText*);
    // Destructor
    ~ShpFileReader ();

    //  --- Methods
    void close ();

    Attribute* getNext ();
    Attribute* getNextSimpleLine ();
    Attribute* getNextLine ();
    Attribute* getNextPoint ();
    Attribute* getNextMultiPoint ();
    Attribute* getNextPolygon ();

    bool readHeader (unsigned int);
    uint32_t readBigInt32 ();
    uint32_t readLittleInt32 ();
    double readLittleDouble ();

    static string getShpType(const string fname, bool& correct,
                             string& errorMessage);

private:

    // --- Attributes
    bool defined;
    ifstream file;
    uint32_t type;
    streampos fileend;

};

#endif /* __SHP_FILE_READER_H__ */

