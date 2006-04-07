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
 * The displayclass of the line datatype (Rose algebra).
 */
public class Dsplline extends DisplayGraph {
  /** The bounding-box rectangle */
  Rectangle2D.Double bounds;
  /** The shape representing this line */
  GeneralPath GP;
  /** boolean flag indicating the defined state*/
  boolean defined;
  /** The textual representation of this line **/
  String entry;

  
  /** returns true because this type is a line **/
  public boolean isLineType(){
    return true;
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

  


 /**
   * Scans the representation of the line datatype and constucts the lines Vector.
   * @param v A list of segments
   * @see sj.lang.ListExpr
   * @see <a href="Dspllinesrc.html#ScanValue">Source</a>
  */
  public void ScanValue (ListExpr value) {
    if(isUndefined(value)){
       defined=false;
       GP=null;
       return;
    }
    defined=true;
    double koord[] = new double[4];
    double x1,y1,x2,y2;
    double lastX=0, lastY=0;
    boolean first = true;
    GP = new GeneralPath();
    while (!value.isEmpty()) {
      ListExpr v = value.first();
      if (v.listLength() != 4) {
        Reporter.writeError("Error: No correct line expression: 4 elements needed");
        GP=null;
        err = true;
        return;
      }
      for (int koordindex = 0; koordindex < 4; koordindex++) {
        Double d = LEUtils.readNumeric(v.first());
        if (d == null) { // error detected
          err = true;
          GP=null;
          return;
        }
        koord[koordindex] = d.doubleValue();
        v = v.rest();
      } 
			try{
				if(!ProjectionManager.project(koord[0],koord[1],aPoint)){
					 err = true; 
           Reporter.debug("error in project segment ("+koord[0]+","+koord[1]+")->"+koord[2]+","+koord[3]+")");
           GP=null;
           return;
				}else{ 
					 x1 = aPoint.x;
					 y1 = aPoint.y;
					 if(!ProjectionManager.project(koord[2],koord[3],aPoint)){
							err = true;
              GP=null;
              Reporter.debug("error in project segment ("+koord[0]+","+koord[1]+")->"+koord[2]+","+koord[3]+")");
              return;
					 }else{
							x2 = aPoint.x;
							y2 = aPoint.y;

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
					 }
			 }
			} catch(Exception e){
          Reporter.debug(e); 
			}
      value = value.rest();
    }
    if(first){ // empty line
       GP=null;
    }
  }


  public void init(ListExpr type, ListExpr value,QueryResult qr){
        init(type,0,value,0,qr);
  }


  /**
   * Init. the Dsplline instance.
   * @param type The symbol line
   * @param typwwidth the minium number of cahracters uded for the type
   * @param value A list of segments.
   * @param qr queryresult to display output.
   * @see QueryResult
   * @see sj.lang.ListExpr
   */
  public void init (ListExpr type,int typewidth, ListExpr value,int valueWidth, QueryResult qr) {
    AttrName = extendString(type.symbolValue(),typewidth);
    ScanValue(value);
    RenderObject = GP;
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      entry = AttrName + " : <error>"; 
      qr.addEntry(entry);
      bounds =null;
      RenderObject=null;
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
    err=false;
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
   * @see <a href="Dspllinesrc.html#getBounds">Source</a>
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
    if ((bounds.getWidth()*bounds.getHeight()!=0) && (!bounds.intersects(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex,
        10.0*scaley)))
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0*scalex, ypos -
        5.0*scaley, 10.0*scalex, 10.0*scaley);
    return stroke.createStrokedShape(GP).intersects(r);
  }

}



