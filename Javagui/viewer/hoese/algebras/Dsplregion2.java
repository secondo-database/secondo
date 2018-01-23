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
import  java.math.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;
import tools.Reporter;


/**
 * The displayclass of the region2 datatype (Rose algebra).
 */
public class Dsplregion2 extends DisplayGraph {

  private static final int MOVETO=0;
  private static final int LINETO=1;
  private static final int QUADTO=2;
  private static final int CLOSE=4;


  /** The internal datastructure of the region2 datatype */
  GeneralPath p;
  Area areas;
  boolean defined;

  public int numberOfShapes(){
     return 1;
  }

  public Shape getRenderObject(int num,AffineTransform at){
    if(num<1){
       return areas;
    } else{
       return null;
    }
  }

  private ListExpr  appendCurve(GeneralPath p, int conType, ListExpr coordList) throws Exception{
    ListExpr res = coordList;
    switch(conType){
       case MOVETO:
       case LINETO:
               double x = LEUtils.readNumeric(res.first());
               res = res.rest();
               double y = LEUtils.readNumeric(res.first());
               res = res.rest();
               if(ProjectionManager.project(x,y,aPoint)){
                  x =  aPoint.x;
                  y = aPoint.y;
               } 
               if(conType==MOVETO){
                  p.moveTo(x,y);
               } else {
                  p.lineTo(x,y);
               }
               return res;
        case  QUADTO: 
               double x1 = LEUtils.readNumeric(res.first());
               res = res.rest();
               double y1 = LEUtils.readNumeric(res.first());
               res = res.rest();
               double x2 = LEUtils.readNumeric(res.first());
               res = res.rest();
               double y2 = LEUtils.readNumeric(res.first());
               res = res.rest();
               if(ProjectionManager.project(x1,y1,aPoint)){
                  x1 =  aPoint.x;
                  y1 = aPoint.y;
               } 
               if(ProjectionManager.project(x2,y2,aPoint)){
                  x2 =  aPoint.x;
                  y2 = aPoint.y;
               } 
               p.quadTo(x1,y1,x2,y2);
               return res;
       case  CLOSE: p.closePath();
                    return res;
               

    }
    throw new Exception("invalid connection type");
  }


  /** convert the Listrepresentation LE to a Area */
  public void ScanValue(ListExpr LE){
     p= null;
     if(isUndefined(LE)){
        defined=false;
        err=false;
        return;
     }
     if(LE==null){
       defined=false;
       err=true;
       return;
     }
     if(LE.listLength()!=2){
       defined=false;
       err=true;
       return;
     }

     defined=true;
     areas = null;
     ListExpr conList = LE.first();
     ListExpr coordList = LE.second();
     p = new GeneralPath();

     while(!conList.isEmpty()){
        ListExpr con = conList.first();
        conList = conList.rest();
        if(con.atomType()!=ListExpr.INT_ATOM){
           defined=false;
           err=true;
           return;
        }
        int c = con.intValue();
        try{
          coordList = appendCurve(p,c,coordList);
        } catch(Exception e){
           defined=false;
           err=true;
           return;
        }
     }
     areas = new Area(p);
  }

  /**
   * Init. the Dsplregion2 instance.
   * @param type The symbol region2
   * @param value A list of polygons /holepolygons
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplregion2src.html#init">Source</a>
   */
   public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = extendString(name, nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry((AttrName)+" : Error(region2)");
      return;
    } else if(!defined){
       qr.addEntry((AttrName)+" : undefined");
       return;
    }
    qr.addEntry(this);
  }



}



