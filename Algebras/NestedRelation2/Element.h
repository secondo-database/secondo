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

The content of this file is based on the OSM algebra of SECONDO
created by Thomas Uchdorf.

It is used by the file reader and represents information about a structure
found in the XML file while reading.

*/

// [...]
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

// --- Including header-files
#include <string>
#include <vector>

class Element {

public:

   // --- Constructors
   // Default-Constructor
   Element ();
   // Constructor
   Element (std::string name, 
            std::vector<std::string> attributeNames,
            std::vector<std::string> attributeValues,
            std::string value,
            int level, const int line);
   // Destructor
   ~Element ();

   // --- Methods
   void setName (const std::string &name);
   void setValue (const std::string &value);
   void setAttributeNames (const std::vector<std::string>
   &attributeNames);
   void setAttributeValues (const std::vector<std::string>
   &attributeValues);
   void setLevel (const int &level);
   const std::string & getName () const;
   const std::string & getValue () const;
   const std::vector<std::string> & getAttributeNames () const;
   const std::vector<std::string> & getAttributeValues () const;
   const int & getLevel () const;
   const int & getLine () const;

   void print () const;

protected:

    // --- Members
    std::string m_name;
    std::vector<std::string> m_attributeNames;
    std::vector<std::string> m_attributeValues;
    std::string m_value;
    int m_level;
    int m_line;
};

void printElement (const Element & element);

#endif /*__ELEMENT_H__*/

