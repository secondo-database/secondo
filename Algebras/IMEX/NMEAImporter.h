
/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen,
Faculty of Mathematics and Computer Science,
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

*/


#include <vector>
#include <iostream>
#include "NestedList.h"
#include "RelationAlgebra.h"


class NMEALineImporter;



/*
2 Class NMEAIMPORTER

*/
class NMEAImporter{
  public: 

     NMEAImporter();

     ~NMEAImporter();

/*
~setType~

Sets the LineImporter to be used. If the given string does not represent a
accepted Type of an included line importer, the result will be false and
the GGA importer is used instead.

*/
     bool setType(const std::string& type="GGA");


/*
~scanFile~

Starts to read a file. If the file cannot be opened or other
problems occurs, the function will return false and the errorMessage
is set to the recognized error.



*/
     bool scanFile(const std::string& fileName, std::string& errorMessage);

/*
~getKnownTypes~

Returns all sentences which can be evaluated using the currently installed
line importers as a single string (separated by white spaces).

*/
     std::string getKnownTypes() const;

/*
~getTupleType~

Returns the tuple type for the currently used line importer.

*/
     ListExpr getTupleType();
     
/*
~nextTuple~

Returns the Next Tuple corresponding to the currently selected sentence.
Donst forget to set the managed type and call scanFile first.

*/
     Tuple* nextTuple();

/*
~getTuple~

Returns the tuple represented by this arguments. If __line__ does not match
the current used importer, 0 is returned.

*/
    Tuple* getTuple(const std::string& line);



/*
~finishScan~

Closes the unfderlying file if open.

*/
     void finishScan();

  private:
     std::vector<NMEALineImporter*> importers;  // all available line importers
     int position;                        // current selected importer
     std::ifstream in;                         // current input stream
     
    
};



