/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2014, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 Jan Kristof Nidzwetzki

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 Helper functions for cassandra
 
 1 Includes and defines

*/

#ifndef __CASANDRA_HELPER__
#define __CASANDRA_HELPER__

#include <stdio.h>
#include <string>
#include <queue>

#include <cassert>
#include <cassandra.h>

using namespace std;

//namespace to avoid name conflicts
namespace cassandra {

/*
2.1 Helper Classes

*/
class CassandraHelper {

public:
  
/*
2.1.1 Return true if the string matches a known
      Consistence level, false otherwise

*/
    static bool checkConsistenceLevel(string consistenceLevel) {
        if ((consistenceLevel.compare("ANY") == 0)
                || (consistenceLevel.compare("ONE") == 0)
                || (consistenceLevel.compare("QUORUM") == 0)
                || (consistenceLevel.compare("ALL") == 0)) {

            return true;
        }

        return false;
    }
    
/*
2.1.1 Converts a string into a ~cql\_consistency\_enum~

*/    
    static CassConsistency convertConsistencyStringToEnum
      (string consistenceLevel) {
        
        if(consistenceLevel.compare("ANY") == 0) {
          return CASS_CONSISTENCY_ANY;
        }
        
        if(consistenceLevel.compare("ONE") == 0) {
          return CASS_CONSISTENCY_ONE;
        }
        
        if(consistenceLevel.compare("QUORUM") == 0) {
          return CASS_CONSISTENCY_QUORUM;
        }
        
        if(consistenceLevel.compare("ALL") == 0) {
          return CASS_CONSISTENCY_ALL;
        }
        
        return CASS_CONSISTENCY_ONE;
    }


/*
2.1.2 Get the error message from a future and print it 
 
1. Parameter is the future 
 
*/
   static void print_error(CassFuture* future) {
     CassString message = cass_future_error_message(future);
     fprintf(stderr, "Error: %.*s\n", (int)message.length, message.data);
   }

};
}
#endif
