/*
----
This file is part of SECONDO.

Copyright (C) 2007,
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


1 QueryExecutor for Distributed-SECONDO


1.1 Includes

*/

#ifndef __QEXECUTOR_UTILS__
#define __QEXECUTOR_UTILS__


/*
2 Helper class for the query executor

*/

class QEUtils {

public:
/*
2.1 Create a new UUID 

*/
static void createUUID(string &uuid) {
     char buffer[128];

     const char *filename = "/proc/sys/kernel/random/uuid";
     FILE *file = fopen(filename, "r");

     // Does the proc file exists?
     if( access(filename, R_OK ) == -1 ) {
       cerr << "Unable to get UUID from kernel" << endl;
       exit(-1);
     }
   
     if (file) {
        while (fscanf(file, "%s", buffer)!=EOF) {
           uuid.append(buffer);
        }
     }  

     fclose(file);
  }
  

  /*
  2.2 Replace placeholder like __NODEID__ in a given string

  */
  static void replacePlaceholder(string &query, string placeholder, 
    string value) {
    size_t startPos = 0;
    
    while((startPos = query.find(placeholder, startPos)) != std::string::npos) {
           query.replace(startPos, placeholder.length(), value);
           startPos += value.length();
    }
  }

  /*
  2.3 Does the given string contains a placeholder?

  */
  static bool containsPlaceholder(string searchString, string placeholder) {
    return searchString.find(placeholder) != std::string::npos;
  }
  
};

#endif