/*

---- 
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, iFaculty of Mathematics and Computer Science, 
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


This class defines some commands for controlling a bash like terminal.

*/

#ifndef BASH_H
#define BASH_H

#include <iostream>
#include "StringUtils.h"


enum BashColor {
    Black, Red, Green, Yellow, Blue,  Violet, Cyan, White, Unknown };

class Bash{


public:

  static void setFGColor(BashColor c){
     std::cout << getCS() << colorStringFG(c) <<"m";
  }
  
  static void setBGColor(BashColor c){
     std::cout << "\033[" << colorStringBG(c) <<"m";
  }

  static void normalColors(){
     std::cout << "\033[0m";
  }

  static void setBold(){
     cout << "\033[1m";
  }

  static void setUnderline(){
     cout << "\033[4m";
  }

  static void positionCursor(int line, int col){
     std::cout << "\033[" << line << ";" << col <<"H";
  }

  static void cursorUp(int lines){
     std::cout << "\033[" << lines << "A";
  }

  static void cursorDown(int lines){
     std::cout << "\033[" << lines << "B";
  }

  static void cursorForward(int cols){
     std::cout << "\033[" << cols << "C";
  }
  

  static void cursorBackward(int cols){
     std::cout << "\033[" << cols << "D";
  }

  static void clearScreen(){
     std::cout << "\033[i2J";
  }

  static void clearRestOfLine(){
     std::cout << "\033[K";
  }

  static void saveCursor(){
     std::cout << "\033[s";
  }

  static void restoreCursor(){
     std::cout << "\033[u";
  }

  static string color2string(BashColor c){
      switch(c){
         case Black : return "Black";
         case Red   : return "Red";
         case Green : return "Green";
         case Yellow : return "Yellow";
         case Blue : return "Blue";
         case Violet : return "Violet";
         case Cyan   : return "Cyan";
         case White  : return "White";
         case Unknown : return "Unknown";
         default : return "Unknown";
 
       }
  }

  static BashColor string2color( string c){
      stringutils::toLower(c);
      if(c=="black") return Black;
      if(c=="blue") return Blue;
      if(c=="green")  return Green;
      if(c=="cyan")  return Cyan;
      if(c=="red")  return Red;
      if(c=="violet")  return Violet;
      if(c=="yellow")  return Yellow;
      if(c=="white")  return White;
      return Unknown;

  }


private:

  static std::string getCS(){
     string c = "\033[";
     return c;
  }
   
  static int colorStringFG(BashColor c){
     switch(c) {
         case Black : return 30;
         case Red   : return 31;
         case Green : return 32;
         case Yellow : return 33;
         case Blue : return 34;
         case Violet : return 35;
         case Cyan   : return 36;
         case White  : return 37;
         case Unknown : return 0;
         default : return 0;
     }
  }
  
  static int colorStringBG(BashColor c){
     int fg = colorStringFG(c);
     if(fg>0) fg = fg+10;
     return fg;
  }
};



#endif



