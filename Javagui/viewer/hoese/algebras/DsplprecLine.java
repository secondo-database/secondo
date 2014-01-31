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


public class DsplprecLine extends DisplayGraph {
  /** The bounding-box rectangle */
  Rectangle2D.Double bounds;
  /** The shape representing this line */
  GeneralPath GP;
  /** boolean flag indicating the defined state*/
  boolean defined;
  /** The textual representation of this line **/
  String entry;

  
  /** returns true because this type is a line **/
  public boolean isLineType(int num){
    return true;
  }

  public int numberOfShapes(){
     return 1;
  }

  public Shape getRenderObject(int num, AffineTransform at){
    if(num!=0){
      return null;
    } else{
        return GP;
    }
  }


  /** returns the string for the textual representation of this **/
  public String toString(){
     if(err || !defined){
        return entry;
     }
     else{
        return entry + " ("+Cat.getName()+")";
     }
        
  }


  private boolean fillSegment(ListExpr s, int scale, double[] res){
     if(s.listLength()!=2){
       return false;
     }
     if(s.first().listLength()!=2 || s.second().listLength()!=2){
        return false;
     }
     Double x1 = Dsplprecise.getDouble(s.first().first(),false);
     Double y1 = Dsplprecise.getDouble(s.first().second(),false);
     Double x2 = Dsplprecise.getDouble(s.second().first(),false);
     Double y2 = Dsplprecise.getDouble(s.second().second(),false);
     if(x1==null || y1==null || x2==null || y2==null){
         return false;
     }
     res[0] = x1.doubleValue() / scale;
     res[1] = y1.doubleValue() / scale;
     if(!ProjectionManager.project(res[0],res[1],aPoint)){
       return false;
     }
     res[0] = aPoint.x;
     res[1] = aPoint.y;

     res[2] = x2.doubleValue() / scale;
     res[3] = y2.doubleValue() / scale;
     if(!ProjectionManager.project(res[2],res[3],aPoint)){
        return false;
     }
     res[2] = aPoint.x;
     res[3] = aPoint.y;

     return true;

  }


  public void ScanValue(ListExpr value){

    Point2D.Double last1 = new Point2D.Double();
    Point2D.Double last2 = new Point2D.Double();
    err = true;
    if(isUndefined(value)){
       defined=false;
       err = false;
       GP=null;
       return;
    }
    defined=true;

    if(value.listLength()!=2){
       return;
    } 
    if(value.first().atomType()!=ListExpr.INT_ATOM){
      return;
    }
    int scale = value.first().intValue();
    if(scale<=0){
       return;
    }
    value=value.second();
    if(value.atomType()!=ListExpr.NO_ATOM){
       return;
    }

    double koord[] = new double[4];
    double x1,y1,x2,y2;
    double lastX=0, lastY=0;
    boolean first = true;
    GP = new GeneralPath();

    while (!value.isEmpty()) {
      ListExpr v = value.first();
      if(!fillSegment(v,scale, koord)){
          GP=null;
          err=true;
          return;
      }
      x1 = koord[0];
      y1 = koord[1];
      x2 = koord[2];
      y2 = koord[3];
      last1.setLocation(x1,y1);
      last2.setLocation(x2,y2);

      if(first){
         GP.moveTo((float)x1,(float)y1);
         GP.lineTo((float)x2,(float)y2);
         first=false;
         lastX=x2;
         lastY=y2;
       }else{
         if((lastX==x1) && (lastY==y1)){
             GP.lineTo((float)x2,(float)y2);
             lastX=x2;
             lastY=y2;
          } else if((lastX==x2) && (lastY==y2)){
             GP.lineTo((float)x1,(float)y1);
             lastX=x1;
             lastY=y1;
          } else{ // not connected
              GP.moveTo((float)x1,(float)y1);
              GP.lineTo((float)x2,(float)y2);
              lastX=x2;
              lastY=y2;
          }
      }
      value = value.rest();
    }

    if(first){ // empty line
       GP=null;
    }
    err = false;
  }



  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      entry = AttrName + " : <error>"; 
      qr.addEntry(entry);
      bounds =null;
      GP=null;
      return;
    }
    else if(!defined){
      entry = AttrName+" : undefined";
      qr.addEntry(entry);
      return;
    }
    // normal case-> defined line
    entry = AttrName + " : line";
    defined=GP!=null;
    qr.addEntry(this);
    if(GP==null)
        bounds = null;
    else{
       bounds = new Rectangle2D.Double();
       bounds.setRect(GP.getBounds2D());
    }
  }


  /**
   * @return The boundingbox of this line-object
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }

  /**
   * Tests if a given position is near (10pxs) of this line, by iterating over all segments.
   * @param xpos The x-Position to test.
   * @param ypos The y-Position to test.
   * @param scalex The actual x-zoomfactor
   * @param scaley The actual y-zoomfactor
   * @return true if x-, ypos is contained in this points type
   */
  public boolean contains (double xpos, double ypos, double scalex, double scaley) {
    if (bounds==null) return false; // an empty line
    // bounding box test
    if ((bounds.getWidth()*bounds.getHeight()!=0) && (!bounds.intersects(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex,
        10.0*scaley)))
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0*scalex, ypos -
                                                  5.0*scaley, 10.0*scalex, 10.0*scaley);


    return GP.intersects(r);
  }

}



