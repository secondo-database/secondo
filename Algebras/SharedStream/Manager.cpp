
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

*/

#include "Manager.h"
#include "StringUtils.h"


namespace sharedstream {

    void Manager::addSource(std::string sourceName, std::string ipAdress,
                            std::string port, NestedList tupleDescription){
SourceEntry entry;
        entry.SourceName = sourceName;
        entry.ipAdress = ipAdress;
        entry.port = port;
        entry.tupleDescription = tupleDescription;
/*Add the new Source to the SourceList*/
        Manager::SourceEntries.push_back(entry);
 /*Add the new Source to the Webinterface*/
        //TODO: Kommunikation web<->Secondo klären

    }
/*The query from the webinterface is given as a string separated by
 * semicolons in the form:
 * "Name of the stream; indicator B for basic data types or S for spatial data
 * types; Name of the attribute; choosen relational operator; entered value
 * for the attribute; salutation; name; eMail-address"
 * For example:
 * „Strom1;B;Strasse;gleich;Kastanienallee;Frau,Mustermann;
 * rita.mustermann@mustermann.de“
 */

    /*
    void Manager::registeredQuery(std::string webquery) {
stringutils::StringTokenizer splitty = new StringTokenizer(webquery,";");

    }
     */

}//end namespace