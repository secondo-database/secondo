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
created by Thomas Uchdorf. It contains an interface a parser class must
implement to be usable with the XmlFileReader implemented in the equally named
file.

*/

// [...]
#ifndef __XML_PARSER_INTERFACE_H__
#define __XML_PARSER_INTERFACE_H__

// --- Including header-files
#include "Element.h"

/*
This class defines an interface used by the XML reader ("XmlFileReader"[2]) to
send information about events that just occured.

*/
class XmlParserInterface
{
public:

   XmlParserInterface ();
   virtual ~XmlParserInterface ();

   virtual void pushedElementToStack (const Element &element) = 0;
   virtual void poppedElementFromStack (const Element &element) = 0;
   virtual bool isElementInteresting (const Element &element) const = 0; 
   virtual bool foundInterestingElement () const = 0;

   virtual void processedText(const std::string &text);
   virtual void processedEntityReference(const std::string &name);
};

#endif /* __XML_PARSER_INTERFACE_H__*/
