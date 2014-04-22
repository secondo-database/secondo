
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
#include <algorithm>
#include <cctype>
#include "StringUtils.h"
#include "math.h"
#include <iostream>

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

/*
toUpper and toLower

*/

  void toLower(std::string& str){
      std::transform(str.begin(),str.end(), str.begin(), (int(*)(int)) tolower);
  }

  void toUpper(std::string& str){
      std::transform(str.begin(),str.end(), str.begin(), (int(*)(int)) toupper);
  }



std::string int2str(int a){
   return any2str<int>(a);
}

/*
3 endsWith

checks whether a2 is a suffix of a1

*/
bool endsWith(const std::string& a1, const std::string& a2){
    size_t len1 = a1.length();
    size_t len2 = a2.length();
    if(len2 > len1){
        return false;
    }
    return a1.substr(len1-len2)==a2;  
}


/*
4 startsWith

checks whether a2 is a prefix of a1

*/
bool startsWith(const std::string& a1, const std::string& a2){
    size_t len1 = a1.length();
    size_t len2 = a2.length();
    if(len2 > len1){
        return false;
    }
    return a1.substr(0,len2)==a2;  
}

/*
4.15 Computes the Levenshtein distance between two strings.

This distance is defined by the minimum count of operators in

  * add character

  * remove character

  * replace character

to get the __target__ from the __source__.

The complexity is source.length [*] target.length.

*/
int ld(const std::string& source, const std::string& target){
  int n = source.length();
  int m = target.length();
  if(n==0){
     return m;
  }
  if(m==0){
     return n;
  }
  n++; 
  m++; 
  int matrix[n][m];
  // initialize
  for(int i=0;i<n;i++){
    matrix[i][0] = i; 
  }
  for(int i=0;i<m;i++){
    matrix[0][i] = i; 
  }
  int cost;
  for(int i=1;i<n;i++){
     for(int j=1;j<m;j++){
        cost = source[i-1]==target[j-1]?0:1;
        matrix[i][j] = std::min(matrix[i-1][j]+1,
                           std::min(matrix[i][j-1]+1,
                           matrix[i-1][j-1]+cost));
     }
  }
  int res = matrix[n-1][m-1];
  return res;
}


/*
~isLetter~ checks whether an character is a letter

*/
bool isLetter(const char c){
   return ((c>='A') && (c<='Z')) || ((c>='a' && c<='z'));
}

/*
~isDigit~ checks whether a character is a digit

*/
bool isDigit(const char c) {
  return (c>='0') && (c<='9');
}



std::string double2str(const double v, int prec /*= 16*/, 
                       const bool afterComma /*=true*/)
{
    std::stringstream ss;
    if(afterComma){
      int f = 1 + (int) v;
      double l1 = log10(f);
      int l2 = (int) l1;
      if(l2<l1){
         l2++;
      }
      prec +=l2;
    }
    ss.precision(prec);
    ss << v;
    std::string res =  ss.str();
    return res;
}





} // end of namespace stringutils
