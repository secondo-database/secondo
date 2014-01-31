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
 * The displayclass of the region datatype (Rose algebra).
 */
public class DsplprecRegion extends DisplayGraph {
  /** The internal datastructure of the region datatype */
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



  private GeneralPath getCycle(ListExpr cycle, int scale){
     if(cycle.atomType()!=ListExpr.NO_ATOM || cycle.isEmpty()){
         return null;
     }
     GeneralPath res = null;
     while(!cycle.isEmpty()){
         ListExpr point = cycle.first();
         cycle = cycle.rest();
         if(point.listLength()!=2){
            return null;
         }
         Double X = Dsplprecise.getDouble(point.first(), false);
         Double Y = Dsplprecise.getDouble(point.second(), false);
         if(X==null || Y==null){
            return null;
         }
         double x = X.doubleValue()/scale;
         double y = Y.doubleValue()/scale;
         if(!ProjectionManager.project(x,y,aPoint)){
           return null;
         }
         if(res==null){
            res = new GeneralPath();
            res.moveTo((float)x,(float)y);
         } else {
            res.lineTo((float)x,(float)y);
         }
     }
     res.closePath();
     return res;
  }

  private Area getArea(ListExpr face, int scale){

     if(face.atomType()!=ListExpr.NO_ATOM || face.isEmpty()){
         return null;
     }
     GeneralPath res = new GeneralPath();
     AffineTransform af = new AffineTransform();
     while(!face.isEmpty()){
         GeneralPath cycle = getCycle(face.first(), scale);
         if(cycle==null){
            return null;
         }
         res.append(cycle.getPathIterator(af),false);
         face = face.rest();
     }
     return new Area(res);

  }

  /** convert the Listrepresentation LE to a Area */
  public void ScanValue(ListExpr LE){
     defined = false;
     err = true;
     if(isUndefined(LE)){
        defined=false;
        err=false;
        return;
     }
     defined=true;
     areas = null;
     if(LE.listLength()!=2){
        return;
     }
     if(LE.first().atomType()!=ListExpr.INT_ATOM){
         return;
     }
     int scale = LE.first().intValue();
     if(scale<=0){
        return;
     }
     ListExpr faces = LE.second();
     while(!faces.isEmpty()){
        Area a = getArea(faces.first(), scale);
        if(a==null){
          return;
        }
        if(areas==null){
          areas = a;
        }  else {
          areas.add(a); 
        }
        faces = faces.rest();
     } 
     err = false;
     return;
  }

  /**
   * Init. the Dsplregion instance.
   * @param type The symbol region
   * @param value A list of polygons /holepolygons
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplregionsrc.html#init">Source</a>
   */
   public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = extendString(name, nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry((AttrName)+" : Error(region)");
      return;
    } else if(!defined){
       qr.addEntry((AttrName)+" : undefined");
       return;
    }
    qr.addEntry(this);
  }



}




