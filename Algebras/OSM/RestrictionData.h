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

[1] Header File of the RestrictionData

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This header file essentially contains the definition of the class
~RestrictionData~.

2 Defines and includes

*/
#ifndef __RESTRICTION_DATA_H__
#define __RESTRICTION_DATA_H__

// --- Including header-files
#include <string>
#include <vector>

class RestrictionData {

    public:

        // --- Constructors
        // Constructor
        RestrictionData ();
        // Destructor
        ~RestrictionData ();

        // --- Methods
        void setId (const int & id);

        void setFrom (const int & from);
        
        void setVia (const int & via);

        void setTo (const int & to);

        void setRestriction (const std::string & restriction);

        void setType (const std::string & type);

        const int & getId () const;

        const int & getFrom () const;
        
        const int & getVia () const;

        const int & getTo () const;

        const std::string & getRestriction () const;

        const std::string & getType () const;

        const std::vector<std::string> & getValues ();

        void print () const;

    protected:

        // --- Members
        int m_id;

        int m_from;

        int m_via;

        int m_to;

        std::string m_restriction;
        
        std::string m_type;

        std::vector<std::string> m_values;

};

void printRestriction (const RestrictionData &restriction);

#endif /*__RESTRICTION_DATA_H__ */
