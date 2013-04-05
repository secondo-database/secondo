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
import java.util.Vector;

/* This class provides some variables for globale use.
 *
 */
public class Environment{

/** If the variable is true, lists of for objects and so on
  * are written in 
  * old style fashion, e.g. including model mapping 
  **/
public static boolean OLD_OBJECT_STYLE = false;





/** interpretes scrips in ttystyle instead of 
  * simple one-line commands 
  **/
public static boolean TTY_STYLED_SCRIPT = true;

/** if set to true, commands are entered as in 
  * secondo tty, this means a ';' or an empty line
  * finish the command.
  **/
public static boolean TTY_STYLED_COMMAND = false;


/*
Name of the testOptimizer Script
*/
public static String testOptimizerConfigFile = "testOptimizer.cfg";



/** inserts a tab-extension **/
public static void insertExtension(String word){
   extensions.insert(word);
}

/** removes a tab-extension **/
public static void removeExtension(String word){
   extensions.deleteEntry(word);
}

/** returns all extension for the given prefix **/
public static Vector getExtensions(String prefix){
    return extensions.getExtensions(prefix);
}


private static Runtime rt = Runtime.getRuntime();
private static Trie extensions = new Trie();





}
