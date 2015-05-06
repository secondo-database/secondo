
/*
---- 
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Faculty of Mathematics and Computer Science, 
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

1 This file provides some functions handling with strings.

The implentations can be found at StringUtils.cpp in directory Tools/Utilities.

*/


#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <sstream>
#include <errno.h>


namespace stringutils {

/*
1 StringTokenizer

This tokenizer splits a string at positions of delimiters. 
Empty tokens are returned but the delimiters are omitted.
 

*/

class StringTokenizer{
   public:
/*
1.1 Constructor

Constructs a tokenizer for the given string and the given delimiters.

*/

       StringTokenizer(const std::string& s, const std::string& _delims);

/*
1.2 Checks whether more tokens are available.

*/

       bool hasNextToken() const;

/*
1.3 getRest

Returns the non processed part of the string.

*/
      std::string getRest() const;

/*
1.4 getPos

Returns the current position in the source string

*/
    size_t getPos() const{
       return pos;
    }



/*
~nextToken~

Returns the next token.

*/
       std::string nextToken();
   private:
       std::string str;
       std::string delims;
       size_t pos;
};

/*
2 ~trim~

removes whitespaces at the begin and at the end of the string.

*/

void trim(std::string& str);

/*
The following function is used to replace all occurences of a pattern within a
string by an other pattern.

*/

std::string replaceAll(const std::string& textStr,
                  const std::string& patternOldStr,
                  const std::string& patternNewStr);




/*
3 endsWith

Checks whether the second argument is a suffix of the first one

*/
bool endsWith(const std::string& a1, const std::string& a2);

/*
4 startsWith

Checks whether the second argument is a prefix of the first one

*/
bool startsWith(const std::string& a1, const std::string& a2);



/*
5 toLower

*/
  void toLower(std::string& s);

/*
6 toUpper

*/
  void toUpper(std::string& s);



/*
7 isIdent

Checks whether the string represents an identifier

*/


bool isIdent(const std::string& s);








/*
Function converting a type supporting the shift (output) operator
to a string.

*/
template<typename t>
std::string any2str(const t& a){
   std::stringstream s;
   s << a;
   return s.str();
}


/*
Converts an int into a string

*/
std::string int2str(int a);

/*
Converts a double value into a string using the given precision.
If afterComma is set to true, the precision is interpreted as
digits after the comma. Otherwise prec indicates the number
of significant digits.

*/
std::string double2str(const double v, int prec = 16, 
                       const bool afterComma = true);




/*
~ld~ computes the levensthein distance for two strings.

*/

int ld(const std::string& source, const std::string& target);


/*
~isLetter~ checks whether an character is a letter

*/
bool isLetter(const char c);

/*
~isDigit~ checks whether a character is a digit

*/
bool isDigit(const char c);

/*
returns the value of a digit char as an int

*/
int getDigit(const char c);


template<class inttype>
inttype str2int(std::string& str, bool& correct){
  int sign = 1;
  inttype value = 0;
  replaceAll(str," ","");

  for(size_t  i=0;i<str.length();i++){
    char c = str[i];
    if(c=='-'){
      if(i>0){
        correct = false;
        return 0;
      }
      sign = -1; 
    } else if(c=='+'){
       if(i>0){
          correct=false;
          return 0;
       } 
    } else if(isDigit(c)){
       errno=0;
       value = value*10 + (c-'0'); 
       if(errno){
         correct = false;
         return 0;
       }
    } else {
       correct = false;
       return 0;
    }
  }
  errno = 0;
  value *= sign;
  if(errno){
     correct = false;
     return 0;
  }
  correct = true;
  return value;


}

} // end of namespace stringutils
#endif


