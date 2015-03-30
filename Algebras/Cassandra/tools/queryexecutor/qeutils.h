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

1.2 Typedefs

*/
typedef bool(*myFunc)(string &);

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
  
  /*
  2.4 Replace shortcuts. The following shortcuts are 
      currently supported in DSECONDO:
  
      [read roads]              
                         -> ccollect('roads', ONE)
      [read roads consistency]  
                         -> ccollect('roads', consistency)
  
      [readparallel roads]              
                         -> ccollect('roads', ONE, __TOKENRANGE__)
      [readparallel roads consistency] 
                         -> ccollect('roads', consistency, __TOKENRANGE__)
  
      [write roads N]           
                         -> cspread['roads', 'ONE', __QUERYUUID__, N]
      [write roads N consistency] 
                         -> cspread['roads', consistency, __QUERYUUID__, N]
  
  
  */
  static bool replaceShortcuts(string &query) {
     
     map<string, myFunc> converterFunctions;
     
     converterFunctions["[read"] = &handleReadShortcut;
     converterFunctions["[readparallel"] = &handleReadparallelShortcut;
     converterFunctions["[write"] = &handleWriteShortcut;
     
     size_t pos = getPosOfFirstShortcut(query, converterFunctions);
     
     while(pos != std::string::npos) {
        
         for(map<string, myFunc>::iterator iter = converterFunctions.begin(); 
             iter != converterFunctions.end(); iter++) {
                
             string name = iter -> first;
                
             if(query.substr(pos, name.length()) == name) {
                myFunc handler = iter -> second;
                handler(query);
             }
         }
         
         pos = getPosOfFirstShortcut(query, converterFunctions);
     }
     
     return true;
  }
  
  /*
  2.5 Get position of the first shortcut
  
  */
  static size_t getPosOfFirstShortcut(string &query, 
     map<string, myFunc> &converterFunctions) {
     
     size_t result = std::string::npos;
     
     for(map<string, myFunc>::iterator iter = converterFunctions.begin(); 
         iter != converterFunctions.end(); iter++) {
         
         string name = iter -> first;
         size_t pos = query.find(name);
         
         if(result == std::string::npos || result > pos) {
            result = pos;
         }
     }
     
     return result;
  }
  
  /*
  2.5 Replace read shortcut
  
  */
  static bool handleReadShortcut(string &query) {
     
     size_t pos = query.find("[read ");
     
     if(pos == std::string::npos) {
        return false;
     }
     
     size_t fields = getShortcutFieldCount(query, pos);
     
     string relation = getShortcutField(query, pos, 1);
     string consistency = "ONE";
     
     if(fields == 2) {
        consistency = getShortcutField(query, pos, 2);
     }
     
     string shortcut = getFirstShortcut(query, pos);
     string replacement = "ccollect('" + relation + "', '" 
         + consistency + "')";
     
     replacePlaceholder(query, shortcut, replacement);
     
     return true;
  }
 
  
  /*
  2.6 Replace readparallel shortcut
  
  */ 
  static bool handleReadparallelShortcut(string &query) {
     
     size_t pos = query.find("[readparallel ");
     
     if(pos == std::string::npos) {
        return false;
     }
     
     size_t fields = getShortcutFieldCount(query, pos);
     
     string relation = getShortcutField(query, pos, 1);
     string consistency = "ONE";
     
     if(fields == 2) {
        consistency = getShortcutField(query, pos, 2);
     }
     
     string shortcut = getFirstShortcut(query, pos);
     string replacement = "ccollectrange('" + relation + "', '" 
         + consistency + "', __TOKENRANGE__)";
     
     replacePlaceholder(query, shortcut, replacement);
     
     return true;
  }
  
  
  /*
  2.7 Replace write shortcut
  
  */
  static bool handleWriteShortcut(string &query) {
     
     size_t pos = query.find("[write ");
     
     if(pos == std::string::npos) {
        return false;
     }
     
     size_t fields = getShortcutFieldCount(query, pos);
     
     string relation = getShortcutField(query, pos, 1);
     string partitionKey = getShortcutField(query, pos, 2);
     string consistency = "ONE";
     
     if(fields == 3) {
        consistency = getShortcutField(query, pos, 3);
     }
     
     string shortcut = getFirstShortcut(query, pos);
     string replacement = "cspread['" + relation + "', '" 
         + consistency + "', '__QUERYUUID__'" + ", " 
         + partitionKey + "]";
     
     replacePlaceholder(query, shortcut, replacement);
     
     return true;
  }
  
  /*
  2.8 get number of fields, pointed by pos
  
  */
  static size_t getShortcutFieldCount(string &query, size_t pos) {
     
     size_t fields = 0;
     
     for(size_t i = pos; i < query.length(); i++) {
        if(query[i] == ' ') {
           fields++;
        }
        
        if(query[i] == ']') {
           break;
        }
     }
     
     return fields;
  }
  
  /*
  2.9 Get the content of a field
  
  */
  static string getShortcutField(string &query, size_t pos, size_t field) {
     size_t currentField = 0;
     stringstream ss;
     
     for( ; pos < query.length(); pos++) {
        
        if(query[pos] == ' ') {
           currentField++;
           continue;
        }
        
        if(query[pos] == '[') {
           continue;
        }
        
        if(query[pos] == ']') {
           break;
        }
        
        if(currentField > field) {
           break;
        }
        
        if(currentField == field) {
           ss << query[pos];
        }
        
     }
     
     return ss.str();
  } 
  
  static string getFirstShortcut(string &query, size_t pos) {

     size_t end = query.find(']', pos);
     size_t length = end-pos+1;
     
     string shortcut = query.substr(pos, length);
     
     return shortcut;
  }
  
};

#endif