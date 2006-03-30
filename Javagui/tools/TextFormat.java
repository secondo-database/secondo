//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package tools;

/** This class wrappes text into constants which are 
  * interpreted by a bash console  as formatting commands.
  * This will run un linux systems as well as the bash provided
  * by the msys environment. In a DOS shell the commands are
  * not interpreted.
  *
  * An example of using this class is the following:
  * printing a red colored message on green background:
  * System.out.println(TextFormat.RED+TextFormat.BG_GREEN+"The Message"+TextFormat.NORMAL);
  **/
class TextFormat{
 public static final String NORMAL    ="\033[m";
 public static final String NORMAL1   ="\033[0m";
 public static final String BOLD      ="\033[1m";
 public static final String UNDERLINE ="\033[4m";
 public static final String BLACK     ="\033[30m";
 public static final String RED       ="\033[31m";
 public static final String GREEN     ="\033[32m";
 public static final String YELLOW    ="\033[33m";
 public static final String BLUE      ="\033[34m";
 public static final String VIOLET    ="\033[35m";
 public static final String CYAN      ="\033[36m";
 public static final String BG_BLACK  ="\033[40m";
 public static final String BG_RED    ="\033[41m";
 public static final String BG_GREEN  ="\033[42m";
 public static final String BG_YELLOW ="\033[43m";
 public static final String BG_VIOLET ="\033[44m";
 public static final String BG_CYAN   ="\033[45m";
 public static final String BELL      ="\007";


 public static void printWarning(String message){
    if(gui.Environment.FORMATTED_TEXT)
        System.err.println(RED+"Warning: "+CYAN+message+NORMAL);
    else
        System.err.println("Warning: "+message);
 }

 public static void printError(String message){
    if(gui.Environment.FORMATTED_TEXT)
        System.err.println(RED+"Error: "+message+NORMAL);
    else
        System.err.println("Error: "+message);
 }
 
 public static void printInfo(String message){
    if(gui.Environment.FORMATTED_TEXT)
        System.err.println(GREEN+"Info: "+message+NORMAL);
    else
        System.err.println("Info: "+message);
 }

}
