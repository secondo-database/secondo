package gui;

import sj.lang.ListExpr;

/** this class provied a named listExpr **/

public class SecondoObject{

private String Name="";
private ListExpr value=null;

/** returns the name of the object
  **/
public String getName(){
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
