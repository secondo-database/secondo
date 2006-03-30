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

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;
import tools.Reporter;


/**
 * The displayclass of the PointRectangle algebras rectangle datatype.
 */
public class Dsplrectangle extends DisplayGraph {
/** The internal datatype representation */
  Rectangle2D.Double rect;
  /**
   * Scans the numeric representation of a point datatype 
   * @param v the numeric value of the x- and y-coordinate
   * @see sj.lang.ListExpr
   * @see <a href="Dsplpointsrc.html#ScanValue">Source</a>
   */
  private void ScanValue (ListExpr v) {
    double koord[] = new double[2];
    if (v.listLength() != 4) {
      Reporter.writeError("Error: No correct rectangle expression: 4 elements needed");
      err = true;
      return;
    }
    if ((v.first().atomType() != ListExpr.INT_ATOM) || (v.second().atomType()
          != ListExpr.INT_ATOM) || (v.third().atomType() != ListExpr.INT_ATOM)
          || (v.fourth().atomType() != ListExpr.INT_ATOM)) {
      Reporter.writeError("Error: No correct rectangle : 4 INTs needed");
      err = true;
      return;
    }
    if (!err) {
      rect = new Rectangle2D.Double(v.first().intValue(),v.third().intValue(),v.second().intValue()-v.first().intValue(),
        v.fourth().intValue()-v.third().intValue());
    }
  }

  /**
   * Init. the Dsplrectangle instance.
   * @param type The symbol rectangle
   * @param value The 4 INTs  of a rectangle left,right,top,bottom.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrectanglesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(rectangle))"));
      return;
    } 
    else 
      qr.addEntry(this);
      RenderObject=rect;
  }

}



