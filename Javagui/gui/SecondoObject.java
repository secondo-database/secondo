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
