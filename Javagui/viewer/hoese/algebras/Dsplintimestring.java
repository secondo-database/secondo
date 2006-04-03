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


package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;
import  java.util.*;
import  javax.swing.*;
import tools.Reporter;


/**
 * A displayclass for the intimestring-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplintimestring extends Dsplinstant {
  String Wert;

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplintimestringsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    if(!defined){
      return new JPanel();
    }
    JPanel jp = super.getTimeRenderer(PixelTime);
    JLabel jl = (JLabel)jp.getComponent(0);
    jl.setText(jl.getText() + "  " + Wert);
    return  jp;
  }

  /**
   * Scans the representation of a intimestring datatype 
   * and produces a human readable string from it.
   */
  public  String getString (ListExpr v) {
    if(isUndefined(v)){
      defined=false;
      err=false;
      return "undefined";
    }
    if (v.listLength() != 2) {
      Reporter.writeError("Error: No correct intimebool expression: 2 elements needed");
      err = true;
      defined=false;
      return "<error>";
    }
    err=false;
    String v2 = super.getString(v.first());
    if (err){
      defined=false;
      return "<error>";
    } 
    err = true;
    if (v.second().atomType() != ListExpr.STRING_ATOM){
      defined=false;
      err=true;
      return "<error>";
    }
    Wert = v.second().stringValue();
    err = false;
    defined=true;
    return "v2 -"+Wert;
  }

  /**
   * Init. the Dsplintimestring instance.
   * @param type The symbol intimestring
   * @param value The value of an instant and an string.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimestringsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    String v = getString(value);
    entry = AttrName + ":"+v;
    if(err){
       qr.addEntry(entry);
       return;
    }
    qr.addEntry(this); 
  }

public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getString(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     entry=(T + " : " + V);
     if(!err){
       qr.addEntry(this);
     } else{
       qr.addEntry(entry);
     }
     return;
  }



  /** The text representation of this object 
   * @see <a href="Dsplintimestringsrc.html#toString">Source</a>
   */
  public String toString () {
     return entry;
  }
}



