/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

last change 2006-01-23

*/

package viewer.update;

import sj.lang.ListExpr;
import java.util.Vector;
import tools.Reporter;

/*
This class is the central instance for formatting attributes within the 
UpdateViewer. Here, the conversion from string into nested list and vice
versa is performed. To do this, we use format classes from the package 
viewer.update.format. To avoid freequently creating of formatting objects, we 
hold an instance for each type occured up to now. Whenever a unknow type is
asked, we look for a format class. If this class is found, we create an
object instance from it for formatting all incoming attributes of this type. 
If no formatting class is found, we use the standard format class converting
a nested list into its string representation. For allowing to add or update 
the formatting at runtime, we introduce a delete method removing all connections
between types and its textual representation.

*/
public class AttributeFormatter{

/** Checks wether this is a simple type. 
  * Other types are not allowed withing the current implementation
  * of the updateviewer.
  **/
public static boolean typeAllowed(ListExpr LE){
    if(LE==null) return false;
    return LE.isAtom(); // we allow all atomar types
}



public static LEFormatter getFormatter(String TypeName){
   TypeName = TypeName.trim();
   LEFormatter LEF = SearchFormatter(TypeName);
   if(LEF!=null)
       return LEF;
   // try to load a specified formatting class 
   try{
      Class FClass = Class.forName("viewer.update.format.Format"+TypeName);
      Object o = FClass.newInstance();
      if(!( o instanceof LEFormatter))
          throw new Exception("Found class does not implement the LEFormatter interface");
      FormatConnection FC = new FormatConnection(TypeName,(LEFormatter)o);
      FormatObjects.add(FC);
      return FC.formatter; 
   }catch(Exception e){
     // its a normal case that this try crashes. we do not anything here
     Reporter.debug(e);
   }
   FormatConnection FC = new FormatConnection(TypeName,StdFormat);
   FormatObjects.add(FC);
   return StdFormat;
}


/** Deletes all connections between type name and format objects.
  * It should be called whenever an implementation of a display class
  * has been changed or a new Display class implementation was added.
  **/
public static void cleanConnections(){
   FormatObjects.clear();
}


/** This formatter is used whenever no special class for the type is found
  **/
static LEFormatter StdFormat = new StandardFormatter();



/** Datastructure containing all known connections between types 
  * and formatters. In the future we have replace the vector by a 
  * datastructure better supporting a search for a given typename. 
  **/
private static Vector FormatObjects = new Vector();

/** Searches the FormatObject within the vector, Returns null, if not found.
  **/
private static  LEFormatter SearchFormatter(String TypeName){
    for(int i=0;i<FormatObjects.size();i++){
       FormatConnection fc = (FormatConnection) FormatObjects.get(i);
       if(fc.typename.equals(TypeName))
          return fc.formatter;
    }
    return null;
}

private static   class FormatConnection{
   public FormatConnection(String name, LEFormatter LEF){
      this.typename=name;
      this.formatter=LEF;
   }
   String typename;
   LEFormatter formatter;
}
 
} 
