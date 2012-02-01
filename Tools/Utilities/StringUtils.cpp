
/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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


#include <string>
#include <sstream>
#include "StringUtils.h"

namespace stringutils {


/*
1 Implementation of StringTokenizer

1.1 Constructor

*/

      StringTokenizer::StringTokenizer(const std::string& s, 
                                       const std::string& _delims): 
           str(s), delims(_delims), pos(0){}

/*
~hasNextToken~

Checks whether at least one further token is available.

*/
       bool StringTokenizer::hasNextToken() const{
         return pos <= str.length();
       } 

/*
~getRest~

Returns the part of the string which is not processed yet. 

*/
     std::string StringTokenizer::getRest() const{
        if(pos>=str.length()){
           return "";
        }
        return str.substr(pos);
     }




/*
~nextToken~

Returns the next token for this tokenizer.

*/

       std::string StringTokenizer::nextToken(){
          if(!hasNextToken()){
              return "";
          }
          if(pos==str.length()){
              pos++;
              return "";
          }
          size_t nextPos = str.find_first_of(delims, pos);
          if(nextPos==std::string::npos){
             std::string res = str.substr(pos);
             pos = str.length()+1;
             return res;
          }
          std::string res = str.substr(pos,nextPos-(pos));
          pos = nextPos+1;
          return res;
       }
       

/*
~trim~

Remove white spaces at the begin and at the end of a string.

*/

void trim(std::string& str) {
    std::string whiteSpaces = " \r\n\t";
    std::string::size_type pos = str.find_last_not_of(whiteSpaces);
    if(pos != std::string::npos) {
      str.erase(pos + 1);
      pos = str.find_first_not_of(whiteSpaces);
      if(pos != std::string::npos){
         str.erase(0, pos);
      }    
    } else {
     str.erase(str.begin(), str.end());
    }    
}

/*
The following function is used to replace all occurences of a pattern within a
string by an other pattern.

*/

std::string replaceAll(const std::string& textStr,
                  const std::string& patternOldStr,
                  const std::string& patternNewStr)
{
  std::stringstream sstextReplaced;
  size_t lastpos = 0; 
  size_t pos = 0; 
  if( patternOldStr.length() == 0 )
  {
    return textStr;
  }
  do { 
    lastpos = pos; 
    pos = textStr.find(patternOldStr, pos);
    if (pos != std::string::npos)
    {    
      size_t len = pos - lastpos;
      sstextReplaced << textStr.substr(lastpos,len) << patternNewStr;
      pos += patternOldStr.length();
    }    
    else 
    {    
      sstextReplaced << textStr.substr(lastpos, textStr.length()-lastpos);
    }    
  } while ( (pos != std::string::npos) && (pos < textStr.length()) );
  return sstextReplaced.str();
}



std::string int2str(int a){
   return any2str<int>(a);
}


} // end of namespace stringutils
