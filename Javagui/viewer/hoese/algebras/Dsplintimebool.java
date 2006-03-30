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
import  java.util.*;
import  javax.swing.*;
import viewer.hoese.*;
import tools.Reporter;


/**
 * A displayclass for the intimebool-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplintimebool extends Dsplinstant {
  boolean Wert;

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplintimeboolsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    JPanel jp = super.getTimeRenderer(PixelTime);
    JLabel jl = (JLabel)jp.getComponent(0);
    jl.setText(jl.getText() + "  " + Wert);
    return  jp;
  }

  /**
   * Scans the representation of a intimeint datatype 
   * @param v An instant and an boolean value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimeboolsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if (v.listLength() != 2) {
      Reporter.writeError("Error: No correct intimebool expression: 2 elements needed");
      err = true;
      return;
    }
    super.ScanValue(v.first());
    if (err)
      return; 
    else 
      err = true;
    if (v.second().atomType() != ListExpr.BOOL_ATOM)
      return;
    Wert = v.second().boolValue();
    err = false;
  }

  /**
   * Init. the Dsplintimebool instance.
   * @param type The symbol intimebool
   * @param value The value of an instant and a bool.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimeboolsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": TA(InstantBool))"));
      return;
    } 
    else 
      qr.addEntry(this);
  }

  /** The text representation of this object 
   * @see <a href="Dsplintimeboolsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ":" + LEUtils.convertTimeToString(TimeBounds.getStart())
        + " " + Wert + ": TA(InstantBool) ";
  }
}



