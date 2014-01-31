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
public class DsplprecPoint extends DisplayGraph implements LabelAttribute{
  /** The internal datatype representation */
  Point2D.Double point;
  String label;
  private RectangularShape shp;

  /**
   * standard constructor.
   * @see <a href="Dsplpointsrc.html#Dsplpoint1">Source</a>
   */
  public DsplprecPoint () {
    super();
  }

  public boolean isPointType(int num){
    return true;
  }

  public int numberOfShapes(){
     return 1;
  }

  public String getLabel(double time){
     return label;
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
    if(point==null){
        return null;
    }
    Rectangle2D.Double r = getBounds();
    double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
    double pixy = Math.abs(ps/at.getScaleY());
    double pix = Math.abs(ps/at.getScaleX());
    if (Cat.getPointasRect())
      shp = new Rectangle2D.Double(r.getX()- pix/2, r.getY() - pixy/2, pix, pixy);
    else {
      shp = new Ellipse2D.Double(r.getX()- pix/2, r.getY() - pixy/2, pix, pixy);
    }
    return  shp;
  }


  protected void ScanValue (ListExpr v) {
    err=true;
    point = null;
    label = null;
    if(isUndefined(v)){
        err=false;
        return;
    }
    if(v.listLength()!=2){
      return;
    }
    if(v.first().atomType()!=ListExpr.INT_ATOM){
        return;
    }
    int scale = v.first().intValue();
    if(scale<=0){
       return;
    }
    v = v.second();
    if(v.listLength()!=2){
      return;
    }
    Double X = null;
    Double Y = null;
    try{
      X = Dsplprecise.getDouble(v.first(), false);
      Y = Dsplprecise.getDouble(v.second(), false);
    } catch(Exception e){
       e.printStackTrace();
       return;
    }
    if(X==null || Y==null){
        return;
    }
    double x = X.doubleValue()/scale;
    double y = Y.doubleValue()/scale;

    if(!ProjectionManager.project(x,y,aPoint)){
       return;
    }  
    x = aPoint.x;
    y = aPoint.y; 
    err = false;
    point = new Point2D.Double(x,y);
    label = "("+x+", " + y +")";
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
      qr.addEntry(new String(AttrName + ": error"));
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
    return  new Rectangle2D.Double(point.getX(), point.getY(), 0, 0);
  }

  public Point2D.Double getPoint(){
     return point;
  } 

}



