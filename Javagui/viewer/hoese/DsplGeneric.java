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


package  viewer.hoese;

import  sj.lang.ListExpr;
import viewer.HoeseViewer;


/**
 * A generic implementation of the DsplBase Interface. Useful as baseclass to avoid
 * implementation of all the methods. If the datatype is unknown this class will be used.
 * @author  Thomas Höse
 * @version 0.99 1.1.02
 */
public class DsplGeneric implements DsplBase,DsplSimple {
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

  public SecondoFrame getFrame () {
    return  null;
  }
  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is
   * neccessary for the displaying this type in the queryresultlist.
   * This class is used if datatype is unknown.
   * @param type A symbolatom with the datatype
   * @param value The value of this object
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="DsplGenericsrc.html#init">Source</a>
   */

  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    String ts;
    if (type.isAtom())
      ts = type.writeListExprToString();
    else
      ts = type.first().writeListExprToString();
    qr.addEntry(new String("Unknown Type: " + ts));
   // HoeseViewer.("ListExpr for unknown type " + ts + value.writeListExprToString());
    return;
  }


 public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
    if(!this.getClass().equals(theClass)) {
       init(type,value,qr);
       return;
    }

    String ts;
    if (type.isAtom())
      ts = type.writeListExprToString();
    else
     ts = type.first().writeListExprToString();
     String T = "Unknown Type";
     String N = extendString(ts,typewidth);
     qr.addEntry(N + " : " + T);
     return;

  }

 protected String extendString(String S ,int MinWidth){
   String R = new String(S);
   int NoSpaces=MinWidth-R.length();
   for(int i=0;i<NoSpaces;i++)
      R = ' '+R;
   R=R.replaceAll("\n", " ");
   return R;
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

  /**
    * Checks whether LE described an undefined value.
    */
  public static boolean isUndefined(ListExpr LE){
    if(LE==null)
       return true;
    if(LE.atomType()==LE.SYMBOL_ATOM){
       String v = LE.symbolValue();
       return v.equals("undef") || v.equals("undefined");
    }
    if(LE.listLength()==1 && LE.first().atomType()==LE.SYMBOL_ATOM){
       String v = LE.first().symbolValue();
       return v.equals("undef") || v.equals("undefined");
    }
    return false;
  }


  private static Class theClass = new DsplGeneric().getClass();

}



