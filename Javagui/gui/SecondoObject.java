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

import sj.lang.ListExpr;
import gui.idmanager.*;

/** this class provided a named listExpr **/

public class SecondoObject{

private String Name="";
private ListExpr value=null;
private ID myID= IDManager.getNextID();

public SecondoObject(ID aID){
   myID.equalize(aID);
}

public void setID(ID aID){
   myID.equalize(aID);
}


public ID getID(){
   return myID;
}

/** returns the name of the object
  **/
public String getName(){
  return Name;
}

/** return the name of this object */
public String toString(){
   return Name;
}


/** sets the objectname
  **/
public void setName(String Name){
   this.Name = Name;
}


/** stores the listexpr
 **/
public boolean fromList(ListExpr value){
  this.value = value;
  return true;
}

/** returns the value of this object as nested list
  **/
public ListExpr toListExpr(){
  return value;
}

}
