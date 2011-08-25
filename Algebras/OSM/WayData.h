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

[1] Header File of the WayData

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~WayData~.

2 Defines and includes

*/
#ifndef __WAY_DATA_H__
#define __WAY_DATA_H__

// --- Including header-files
#include <string>
#include <vector>

class WayData {

    public:

        // --- Constructors
        // Constructor
        WayData ();
        // Destructor
        ~WayData ();

        // --- Methods
        const int & getId () const;
        void setId (const int &id);
        const std::vector<int> & getRefs () const;
        void addRef (const int &ref);
        void print () const;
        const std::string & getHighway () const;
        const std::string & getName () const;
        const int & getMaxSpeed () const;
        const int & getOneWay () const;
        const int & getLayer () const;
        const std::string & getBridge () const;
        const std::string & getTunnel () const;
        const std::string & getRef () const;
        void setHighway  (const std::string & highway);
        void setName  (const std::string & name);
        void setMaxSpeed  (const int & maxSpeed);
        void setOneWay  (const int & oneWay);
        void setLayer  (const int & layer);
        void setBridge  (const std::string & bridge);
        void setTunnel  (const std::string & tunnel);
        void setRef  (const std::string & ref);
        const std::vector<std::string> & getValues ();


    protected:

        // --- Members
        int m_id;
        std::vector<int> m_refs;
        std::string m_highway;
        std::string m_name;
        int m_maxSpeed;
        int m_oneWay;
        int m_layer;
        std::string m_bridge;
        std::string m_tunnel;
        std::string m_ref;
        std::vector<std::string> m_values;

};

void printWay (const WayData &way);

#endif /*__WAY_DATA_H__ */
