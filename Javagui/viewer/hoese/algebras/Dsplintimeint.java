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
 * A displayclass for the intimeint-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplintimeint extends Dsplinstant {
  int Wert;

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplintimeintsrc.html#getTimeRenderer">Source</a>
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
   * Scans the representation of a intimeint datatype 
   * and produces a string representation from it
   */
  public String getString(ListExpr v) {
    if(isUndefined(v)){
        defined=false;
        err=false;
        return "undefined";
    }

    if (v.listLength() != 2) {
      Reporter.writeError("Error: No correct intimeint expression: 2 elements needed");
      err = true;
      defined=false;
      return "<error>";
    }
    String v2 = super.getString(v.first());
    if (err){
      defined=false;
      return "<error>";
    } 
    err = true;
    if (v.second().atomType() != ListExpr.INT_ATOM){
      err=true;
      defined=false;
      return "<error>";
    }
    Wert = v.second().intValue();
    err = false;
    defined=true;
    return v2+" - " + Wert;
  }

  /**
   * Init. the Dsplintimeint instance.
   * @param type The symbol intimeint
   * @param value The value of an instant and an int.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimeintsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    String v = getString(value);
     entry = AttrName+":"+v;
    if(err)
       qr.addEntry(entry);
    else
       qr.addEntry(this);
  }

public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth,
QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getString(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     entry=(T + " : " + V);
     qr.addEntry(this);
     return;
  }


  /** The text representation of this object 
   * @see <a href="Dsplintimeintsrc.html#toString">Source</a>
   */
  public String toString () {
     return entry;
  }
}



