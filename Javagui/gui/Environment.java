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

package gui;

import tools.Reporter;

/* This class provides some variables for globale use.
 *
 */
public class Environment{

/** forces more outputs when errors occur **/
public static boolean DEBUG_MODE = true;
/** enables the output of time measure at different places **/
public static boolean MEASURE_TIME = true;
/** enables the printing of the memery state before
  * and after some operations 
  **/
public static boolean MEASURE_MEMORY = false;
/** Flag indicating colorized outputs in the console **/
public static boolean FORMATTED_TEXT = false;
/** If the variable is true, lists of for objects and so on
  * are written in 
  * old style fashion, e.g. including model mapping 
  **/
public static boolean OLD_OBJECT_STYLE = false;
/** Shows all commands before sent them to the Secondo Server **/
public static boolean SHOW_COMMAND = false;
/** The maximum string length. 
  * Ensure to use the same value as in the Secondo kernel.
  **/
public static int MAX_STRING_LENGTH = 48;
/** Print out all stuff related to the Server communication **/
public static boolean TRACE_SERVER_COMMANDS = false;

/** Runs Javagui in Testmode, this means without any 
  * user interaction.
  **/
public static boolean TESTMODE = false;

/** Runs Javagui in extended testmode **/
public static boolean EXTENDED_TESTMODE = false;

/** interpretes scrips in ttystyle instead of 
  * simple one-line commands 
  **/
public static boolean TTY_STYLED_SCRIPT = true;


/** Returns the currently used memeory in bytes **/
public static long usedMemory(){
  return rt.totalMemory()-rt.freeMemory();
}

/** Formats md given in bytes to human readable format **/
public static String formatMemory(long md){
   String mem ="";
   if(Math.abs(md)>=1048576){
      mem = Double.toString(((int)(md/1048.576))/1000.0) + "MB"; 
    } else if(Math.abs(md)>1024){
      mem = Double.toString( ((int)(md/1.024))/1000.0 )+" kB";
    } else{
      mem = Long.toString(md)+" B";
    }
    return mem;
}

/** prints out the current memory state **/
public static void printMemoryUsage(){
    Reporter.writeInfo("total Memory :"+" "+formatMemory(rt.totalMemory()));
    Reporter.writeInfo("free Memory  :"+" "+formatMemory(rt.freeMemory()));
}


private static Runtime rt = Runtime.getRuntime();

}
