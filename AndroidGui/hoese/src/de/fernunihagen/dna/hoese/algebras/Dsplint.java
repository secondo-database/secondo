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

package de.fernunihagen.dna.hoese.algebras;


import de.fernunihagen.dna.hoese.DsplGeneric;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.RenderAttribute;
import  sj.lang.ListExpr;


/**
* A displayclass for the int-type, alphanumeric only
*/
public class Dsplint extends DsplGeneric implements  RenderAttribute, LabelAttribute {
 /** the value of this integer **/
 int value;
 /** the text to display **/
 String entry;
 /** flag for  defined state **/
 boolean defined;


 /** scans value and sets all internal variables **/
 private boolean scanValue(ListExpr value){
    if(isUndefined(value)){
        defined=false;
        this.value = 0;
        return true;
    } else {
      int at = value.atomType();
      if(at!=ListExpr.INT_ATOM){
        defined = false;
        this.value = 0;
        return false;
      }else{
         defined = true;
         this.value = value.intValue();
         return true;
      }
    }
 }

 /** scans the value, sets all internal variables and produces the
   * string of the value
   **/
 private String getString(ListExpr value){
  if(!scanValue(value)){
    return "<error>"; 
  } else if(defined){
    return ""+(this.value);
  } else{
    return "undefined"; 
  }
 }

/**
 * This method is used to analyse the type and value in NestedList format and build
 * up the intern datastructures for this type. An alphanumeric representation is 
 * neccessary for the displaying this type in the queryresultlist.
 */
public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
  entry = extendString(name,nameWidth, indent) +" : "+ getString(value);
  qr.addEntry(this);
  return;
}


public String toString(){
    return entry;
}

/** returns the maximum value **/
public double getMaxRenderValue(){
   return value;
}
/** returns the minimum value **/
public double getMinRenderValue(){
   return value;
}
/** returns the current value **/
public double getRenderValue(double time){
   return value;
}
/** returns true if this int is defined **/
public boolean mayBeDefined(){
    return defined;
}
/** returns tre if this integer is defined **/
public boolean isDefined(double time){
   return defined;
}
/** returns the label **/
public String getLabel(double time){
  return defined?""+value:"undefined";
}

}



