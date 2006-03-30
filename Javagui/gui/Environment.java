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


public static boolean DEBUG_MODE = true;
public static boolean MEASURE_TIME = true;
public static boolean MEASURE_MEMORY = false;
public static boolean FORMATTED_TEXT = false;
public static boolean OLD_OBJECT_STYLE = false;
public static boolean SHOW_COMMAND = false;
public static int MAX_STRING_LENGTH = 48;
public static boolean TRACE_SERVER_COMMANDS = false;


public static long usedMemory(){
  return rt.totalMemory()-rt.freeMemory();
}

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

public static void printMemoryUsage(){
    Reporter.writeInfo("total Memory :"+" "+formatMemory(rt.totalMemory()));
    Reporter.writeInfo("free Memory  :"+" "+formatMemory(rt.freeMemory()));
}


private static Runtime rt = Runtime.getRuntime();

}
