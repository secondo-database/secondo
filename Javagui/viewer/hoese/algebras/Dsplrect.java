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


/**
 * The displayclass for rectangles
 */
public class Dsplrect extends DisplayGraph {
/** The internal datatype representation */
  Rectangle2D.Double rect;
  /**
   * Scans the numeric representation of a rectangle
   */
  private void ScanValue (ListExpr v) {
    double koord[] = new double[2];
    //System.out.println(v.writeListExprToString());
    if (v.listLength() != 4) {
      System.err.println("Error: No correct rectangle expression: 4 elements needed");
      err = true;
      return;
    }
    Double X1 = LEUtils.readNumeric(v.first());
    Double X2 = LEUtils.readNumeric(v.second());
    Double Y1 = LEUtils.readNumeric(v.third());
    Double Y2 = LEUtils.readNumeric(v.fourth());
    if(X1==null || X2==null || Y1==null | Y2==null){
       System.err.println("Error: no correct rectangle expression (not a numeric)");
       err =true;
       return;
    }
    try{
       double tx1 = X1.doubleValue();
       double tx2 = X2.doubleValue();
       double ty1 = Y1.doubleValue();
       double ty2 = Y2.doubleValue();
       double x1 = ProjectionManager.getPrjX(tx1,ty1);
       double y1 = ProjectionManager.getPrjY(tx1,ty1);
       double x2 = ProjectionManager.getPrjX(tx2,ty2);
       double y2 = ProjectionManager.getPrjY(tx2,ty2);
       double x = Math.min(x1,x2);
       double w = Math.abs(x2-x1);
       double y = Math.min(y1,y2);
       double h = Math.abs(y2-y1);
       rect = new Rectangle2D.Double(x,y,w,h);
    }catch(Exception e){
       err = true;
    }
  }

  /**
   * Init. the Dsplrect instance.
   * @param type The symbol rect
   * @param value The 4 Numeric  of a rectangle x1 x2 y1 y2
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(rectangle))"));
      return;
    } 
    else 
      qr.addEntry(this);
      RenderObject=rect;
  }

}



