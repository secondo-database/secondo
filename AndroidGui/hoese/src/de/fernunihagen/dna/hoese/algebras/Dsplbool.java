package de.fernunihagen.dna.hoese.algebras;


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

import de.fernunihagen.dna.hoese.DsplGeneric;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.RenderAttribute;
import  sj.lang.ListExpr;



/**
* A displayclass for the bool-type, alphanumeric only
*/
public class Dsplbool extends DsplGeneric 
                    implements RenderAttribute, LabelAttribute { 
/** the value of this boolean if defined **/
boolean value;
/** the defined flag of this boolean **/
boolean defined;
/** the text to display */
String entry;
/** the label **/
String label;

/** analyses the arguments, sets all internal variables except entry
  * and returns the string for displaying this value textual
  **/
private String getString(ListExpr value){
  if(isUndefined(value)){
      defined=false;
      this.value=false;
      label = "undefined"; 
      return label;
  }
  if(value.atomType()!=ListExpr.BOOL_ATOM){
     defined=false;
     this.value = false;
     label = "<error>";
     return label;
  }
  defined = true;
  this.value = value.boolValue();
  label ="" + value;
  return label;
}



public void init (String name, int nameWidth, int indent, ListExpr type,
                  ListExpr value, QueryResult qr)
{
   String T = name;
   String V = getString(value);
   T=extendString(T,nameWidth, indent);
   entry=(T + " : " + V);
   qr.addEntry(this);
   return;
}

public String toString(){
    return entry;
}

/** returns the maximum value **/
public double getMaxRenderValue(){
   return value?1:0;
}
/** returns the minimum value **/
public double getMinRenderValue(){
   return value?1:0;
}
/** returns the current value **/
public double getRenderValue(double time){
   return value?1:0;
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
  return label;
}



}



