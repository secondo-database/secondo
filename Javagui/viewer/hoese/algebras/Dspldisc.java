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

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import java.text.DecimalFormat;


/**
 * The displayclass of the Rose algebras point datatype.
 */
public class Dspldisc extends DisplayGraph {
  /** The internal datatype representation */
  Ellipse2D.Double disc;

  public boolean isPointType(int num){
    return false;
  }

  public int numberOfShapes(){
     return 1;
  }


  /**
   * Creates the internal Object used to draw this point
   * @param at The actual Transformation, used to calculate the correct size.
   * @return Rectangle or Circle Shape
   * @see <a href="Dsplpointsrc.html#getRenderObject">Source</a>
   */
  public Shape getRenderObject (int num,AffineTransform at) {
    if(num!=0){
       return null;
    }
    return disc;
  }


  protected void ScanValue (ListExpr v) {
    disc = null;
    if(isUndefined(v)){
        err=false;
        return;
    }
    if(v.listLength()!=3){
       err = true;
       return;
    }
    Double X = LEUtils.readNumeric(v.first());
    Double Y = LEUtils.readNumeric(v.second());
    Double R = LEUtils.readNumeric(v.third());
    if(X==null || Y==null || R==null){
      err = true;
      return;
    } 
    err=false;
    double x = X.doubleValue();
    double y = Y.doubleValue();
    double r = R.doubleValue();
    double x1 = x-r;
    double y1 = y-r;
    double x2 = x+r;
    double y2 = y+r;
    if(!ProjectionManager.project(x1,y1,aPoint)){
       err = true;
       return;
    }
    x1 = aPoint.getX();
    y1 = aPoint.getY();
    if(!ProjectionManager.project(x2,y2,aPoint)){
       err = true;
       return;
    }
    x2 = aPoint.getX();
    y2 = aPoint.getY();
    disc = new Ellipse2D.Double(x1,y1, (x2-x1), (y2-y1));
    err = false;
  }


  public void init (String name, int nameWidth, int indent,
                    ListExpr type, 
                    ListExpr value,
                    QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    if(isUndefined(value)){
       qr.addEntry(new String("" + AttrName + ": undefined"));
       return;
    }
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(disc))"));
      return;
    }
    else
      qr.addEntry(this);
  }
  /**
   * @return A rectangle with height=0 and width=0
   * @see <a href="Dsplpointsrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
    return (disc==null)?null:new Rectangle2D.Double(disc.x,disc.y,disc.width,disc.height);
  }

}



