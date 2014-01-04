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
package de.fernunihagen.dna.hoese;


import  sj.lang.ListExpr;


/**
* A generic implementation of the DsplBase Interface. Useful as baseclass to avoid
* implementation of all the methods. If the datatype is unknown this class will be used.
* @author  Thomas Hoese
* @version 0.99 1.1.02
*/
public class DsplGeneric implements DsplBase {
protected String AttrName;
protected boolean selected;
private boolean visible = true;

/**
 * In relations it is neccessary to get the name of the attribute of this datatype instance in
 * a tuple.
 * @return attribute name
 * @see <a href="DsplGenericsrc.html#getAttrName">Source</a>
 */
public String getAttrName () {
  return  AttrName;
}
/**
 * If this datatype shouldn't be displayed in the default 2D-geographical viewer this
 * method returns the specialized frame, which can do this.
 * @return null MainWindow will be used
 * @see generic.SecondoFrame
 * @see generic.MainWindow
 * @see <a href="DsplGenericsrc.html#getFrame">Source</a>
 */

// TODO: SecondoFrame
public Object getFrame () {
  return  null;
}



public void init(String name,
               int nameWidth,
               int indent,
               ListExpr type,
               ListExpr value,
               QueryResult qr)
{
  String ts = "no display function defined";
  String N = extendString(name,nameWidth,indent);
  qr.addEntry(N + " : " + ts);
  return;
}





/**
 * Sets the visibility of an object
 * @param b true=show false=hide
 * @see <a href="DsplGenericsrc.html#setVisible">Source</a>
 */
public void setVisible (boolean b) {
  visible = b;
}

/**
 * Gets the visibility of an object
 * @return true if visible, false if not
 * @see <a href="DsplGenericsrc.html#getVisible">Source</a>
 */
public boolean getVisible () {
  return  visible;
}

/**
 * Sets the select status of an object, textual or graphical.
 * @param b true if selected, false if not.
 * @see <a href="DsplGenericsrc.html#setSelected">Source</a>
 */
public void setSelected (boolean b) {
  selected = b;
}

/**
 * Gets the select status of an object, textual or graphical
 * @return true if selected, false if not
 * @see <a href="DsplGenericsrc.html#getSelected">Source</a>
 */
public boolean getSelected () {
  return  selected;
}


/** Adds spaces to S so that S has a minimum length of MinWidth **/
public  static String extendStringRight(String S ,int MinWidth){
//  String R = new String(S);
// int NoSpaces=MinWidth-R.length();
// for(int i=0;i<NoSpaces;i++)
//    R = ' '+R;
// R=R.replaceAll("\n", " ");
// return R;
String R = new String(S);
while(R.length()<MinWidth){
   R += ' ';
}
R = R.replaceAll("\n"," ");
return R;
}

/** Adds spaces to S so that S has a minimum length of MinWidth **/
public  static String extendStringLeft(String S ,int MinWidth){
 String R = new String(S);
 int NoSpaces=MinWidth-R.length();
 for(int i=0;i<NoSpaces;i++)
    R = ' '+R;
 R=R.replaceAll("\n", " ");
 return R;
}

/** puts indent white spaces at the begin of S and extends S in that way
  * bay appending whitespaces that its minimum length is minWidth + indent
  **/
public static String extendString(String S, int minWidth, int indent){
  String K = extendStringRight(S, minWidth);
  String ind = "";
  for(int i=0;i<indent;i++){
    ind += " ";
  }
  return ind + K;
}




/**
  * Checks whether LE described an undefined value.
  */
public static boolean isUndefined(ListExpr LE){
  if(LE==null)
     return true;
  if(LE.atomType()==ListExpr.SYMBOL_ATOM){
     String v = LE.symbolValue().toLowerCase();
     return v.equals("undef") || v.equals("undefined") || v.equals("null");
  }
  if(LE.listLength()==1 && LE.first().atomType()==ListExpr.SYMBOL_ATOM){
     String v = LE.first().symbolValue().toLowerCase();
     return v.equals("undef") || v.equals("undefined") || v.equals("null");
  }
  return false;
}


}



