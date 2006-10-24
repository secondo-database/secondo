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


/**
 * The displayclass of the points datatype.
 */
public class Dsplpoints extends DisplayGraph {
  Point2D.Double[] points=null;
  Rectangle2D.Double bounds=null;
  private boolean defined;
  private double lastTime;
  private double lastSize = -1; // undefined

  /**
   * Scans the representation of the points datatype and 
   * an array containing the points. 
   * The representation is a list of points which are given just 
   * by the two coordinates describing the position. 
   * @param value: the list representing this points value
   */
  public void ScanValue (ListExpr value) {

    if(isUndefined(value)){
       err=false;
       defined = false;
       return;
    }
    defined = true;
    double koord[] = new double[2];
    Vector pointsV = new Vector(20, 20);
    while (!value.isEmpty()) {
      ListExpr v = value.first();
      if (v.listLength() != 2) {
        Reporter.writeError("Error: No correct points expression: 2 elements needed");
        err = true;
        return;
      }
      for (int koordindex = 0; koordindex < 2; koordindex++) {
        Double d = LEUtils.readNumeric(v.first());
        if (d == null) {
          err = true;
          return;
        }
        koord[koordindex] = d.doubleValue();
        v = v.rest();
      }
      if (!err) {
        if(!ProjectionManager.project(koord[0],koord[1],aPoint))
           Reporter.writeError("error in projection at coordinate ("+
                                koord[0]+", "+koord[1]+")");
        else{
          pointsV.add(new Point2D.Double(aPoint.x,aPoint.y));
        }
      }
      value = value.rest();
    }
    bounds = null;
    if(!err){ // copy the vector into the array to avoid cast 
              // during painting this object
       int size = pointsV.size();
       points = new Point2D.Double[size];
       for(int i=0;i<size;i++){
          Point2D.Double p = (Point2D.Double) pointsV.get(i);
          points[i] = p;
          if (bounds == null){
              bounds = new Rectangle2D.Double(p.getX(), p.getY(), 0, 0);
          } else {
              bounds = (Rectangle2D.Double)bounds.createUnion(new Rectangle2D.Double(p.getX(),
              p.getY(), 0, 0));
          }
       }
    }
  }

  /**
   * Init. the Dsplpoints instance.
   * @param type The symbol points
   * @param value The numeric value of a list of points.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplpointssrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(points))"));
      defined = false;
      return;
    }
    qr.addEntry(this);
  }

  /**
   * @return The boundingbox of all the points
   * @see <a href="Dsplpointssrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }
  
  public boolean isPointType(int num){
    return true;
  }

  public int numberOfShapes(){
     return points.length;
  }

  public Shape getRenderObject(int num, AffineTransform at){
     double ps;
    if(lastSize<0){
      lastTime = CurrentState.ActualTime;
      ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);     
    }else{
      double ct = CurrentState.ActualTime;
      if(lastTime==ct){
        ps = lastSize;
      } else {
        ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);     
        lastTime=ct; 
      }
    }
    lastSize = ps;

    double pixy = Math.abs(ps/at.getScaleY());
    double pix  = Math.abs(ps/at.getScaleX());
    boolean isRect = Cat.getPointasRect();
    Point2D.Double p = points[num];
    if (isRect){
       return (new Rectangle2D.Double(p.getX()- pix/2, p.getY() - pixy/2, pix, pixy));
    }  else {
       return (new Ellipse2D.Double(p.getX()- pix/2, p.getY() - pixy/2, pix, pixy));
    }
  }

  /**
   * Tests if a given position is contained in any of the points.
   * @param xpos The x-Position to test.
   * @param ypos The y-Position to test.
   * @param scalex The actual x-zoomfactor
   * @param scaley The actual y-zoomfactor
   * @return true if x-, ypos is contained in this points type
   * @see <a href="Dsplpointssrc.html#contains">Source</a>
   */
  public boolean contains (double xpos, double ypos, double scalex, double scaley) {
    if(!defined){
       return false;
    }
    double scale = Cat.getPointSize(renderAttribute,CurrentState.ActualTime)*0.7*scalex;  
    for(int i=0;i<points.length;i++){    
      if(points[i].distance(xpos, ypos) <= scale){
        return true;
      }
    }
    return  false;
  }

}



