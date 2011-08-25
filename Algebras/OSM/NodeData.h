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

[1] Header File of the NodeData

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~NodeData~.

2 Defines and includes

*/
#ifndef __NODE_DATA_H__
#define __NODE_DATA_H__

// --- Including header-files
#include <string>
#include <vector>

class NodeData {

public:

    // --- Constructors
    // Constructor
    NodeData ();
    // Destructor
    ~NodeData ();

    // --- Methods
    void setId (const int & id);

    void setLon (const double & lon);

    void setLat (const double & lat);

    void setAmenity (const std::string & amenity);

    void setName (const std::string & name);

    const int & getId () const;

    const double & getLon () const;

    const double & getLat () const;
    
    const std::string & getAmenity () const;

    const std::string & getName () const;

    const std::vector<std::string> & getValues ();

    void print () const;

protected:

    // --- Members
    int m_id;

    double m_lon;

    double m_lat;
    
    std::string m_amenity;
    
    std::string m_name;

    std::vector<std::string> m_values;

};

void printNode (const NodeData &node);

#endif /*__NODE_DATA_H__ */
