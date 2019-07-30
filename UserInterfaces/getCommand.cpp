/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
Faculty of Mathematics and  Computer Science, 
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


#include "getCommand.h"
#include <string>
#include <iostream>
#include <functional>
#include "StringUtils.h"

#ifdef READLINE
  #include <stdio.h>
  #include <readline/readline.h>
  #include <readline/history.h>
  #define HISTORY_FILE ".secondo_history"
  #define HISTORY_FILE_ENTRIES 200
#endif



using namespace std;

bool getCommand(istream& in, 
                const bool isPD, 
                string& cmdRet,
                function<void(const bool)> showPrompt,
                function<bool(const string&)> isInternalCommand,
                const bool isStdInput,
                const string& prompt){

  bool complete = false;
  bool first = true;
  string line = "";
  bool inPD = false;
  cmdRet = ""; // clear any parts


  while (!complete && !in.eof() && !in.fail())
  {
    line = "";
    showPrompt( first );
    #ifdef READLINE
      if(isStdInput){
         char* cline = readline(prompt.c_str());
         line = string(cline);
         free(cline);
      }
      else
    #endif
         getline( in, line );

    stringutils::trimRight(line); 


    if ( line.length() > 0  || inPD) {
      if ( !isStdInput )           // Echo input if not standard input
      {
        cout << " " << line << endl;
      }
      bool comment = false;
      if(!isPD){
         comment = line[0] == '#';
      } else {
        if(!inPD){
          if(line.length()>0){
             comment = line[0] == '#';
          } 
          if(!comment) {
            if(line.length()>1){
              if((line[0]=='/') && (line[1]=='/')){ // single line comment
                 comment = true;
              } else if((line[0]=='/') && (line[1]=='*')) { // big comment
                 comment = true;
                 inPD = true;
              }
            }
          }
        } else {
          comment = true;
          if(line.length()>1){
            if( (line[0]=='*') && (line[1]=='/')){
              inPD = false;
              line = line.substr(2);
              stringutils::trim(line);
              comment = line.empty();
            }
          }
        }
     }

      if ( !comment )        // Process if not comment line
      {
        if ( line[line.length()-1] == ';' )
        {
          complete = true;
          line.erase( line.length()-1 );
        }
        if ( first )               // Check for single line command
        {
          if ( !complete )
          {
            complete = isInternalCommand( line );
          }
          cmdRet = line + " ";
          first = false;
        }
        else
        {
          cmdRet = cmdRet + "\n" + line + " ";
        }
      }
    }
    else                           // Empty line ends command
    {
      complete = cmdRet.length() > 0;
      first = true;
    }
  }


  // remove spaces from the end of cmd
  size_t end = cmdRet.find_last_not_of(" \t");
  if(end==string::npos)
     end = 0;
  else
     end += 1;
  cmdRet = cmdRet.substr(0,end);
  #ifdef READLINE
     if(complete && (cmdRet.length()>0) && isStdInput){
        // get the last entry from the history if avaiable
        int noe = history_length;
        string last = "";
        if(noe){
           HIST_ENTRY* he = history_get(noe);
           if(he)
               last = string(he->line);
        }
        if(last!=cmdRet && (cmdRet.find_last_not_of(" \t\n")!=string::npos))
          add_history(cmdRet.c_str());
       }
  #endif
  return complete;
}


