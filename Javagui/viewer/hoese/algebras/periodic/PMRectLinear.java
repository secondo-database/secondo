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

package viewer.hoese.algebras.periodic;

import viewer.hoese.*;
import sj.lang.ListExpr;
import java.awt.geom.*;

public class PMRectLinear extends LinearMove{

  public PMRectLinear(){
    maxX1=maxY1=minX1=minY2=maxX2=maxY2=minX2=minY2;
    isstatic=true;
    interval = new RelInterval();
    defined = false;
  }

  public String getName(){
    return "pmrect";
  }


  /** returns a Rectangle2D.Double */
  public Object getObjectAt(Time T){
     if(!defined){
        if(Environment.DEBUG_MODE)
	   System.err.println("PMRectLinear.getObjectAt called on an undefined instance");
        return null;
     }
     if(!interval.contains(T)){
        return null;
     }
     double p = interval.where(T);
     double minX = minX1 + p*(minX2-minX1);
     double minY = minY1 + p*(minY2-minY1);
     double maxX = maxX1 + p*(maxX2-maxX1);
     double maxY = maxY1 + p*(maxY2-maxY1);
     // special cases : rectangles without any extend
     if((maxX-minX)==0.0) return null;
     if((maxY-minY)==0.0) return null;
     Object res = new Rectangle2D.Double(minX,minY,maxX-minX,maxY-minY);
     return res;
  }

  protected boolean readStartEnd(ListExpr start, ListExpr end){
     if(start.listLength()!=4 || end.listLength()!=4){
       if(Environment.DEBUG_MODE)
          System.err.println("PMRectLinear.readStartEnd wrong ListLength");
       return false;
     }
     Double MinX1 = LEUtils.readNumeric(start.first());
     Double MinY1 = LEUtils.readNumeric(start.second());
     Double MaxX1 = LEUtils.readNumeric(start.third());
     Double MaxY1 = LEUtils.readNumeric(start.fourth());

     Double MinX2 = LEUtils.readNumeric(end.first());
     Double MinY2 = LEUtils.readNumeric(end.second());
     Double MaxX2 = LEUtils.readNumeric(end.third());
     Double MaxY2 = LEUtils.readNumeric(end.fourth());

     if(MinX1==null || MinY1==null || MaxX1==null || MaxY1==null ||
        MinX2==null || MinY2==null || MaxX2==null || MaxY2==null ){
        if(Environment.DEBUG_MODE)
           System.err.println("PmRectLinear.readStartEnd : found a non-numeric");
        return false;
     }
     minX1 = Math.min(MinX1.doubleValue(),MaxX1.doubleValue());
     maxX1 = Math.max(MinX1.doubleValue(),MaxX1.doubleValue());
     minY1 = Math.min(MinY1.doubleValue(),MaxY1.doubleValue());
     maxY1 = Math.max(MinY1.doubleValue(),MaxY1.doubleValue());
     minX2 = Math.min(MinX2.doubleValue(),MaxX2.doubleValue());
     maxX2 = Math.max(MinX2.doubleValue(),MaxX2.doubleValue());
     minY2 = Math.min(MinY2.doubleValue(),MaxY2.doubleValue());
     maxY2 = Math.max(MinY2.doubleValue(),MaxY2.doubleValue());
     isstatic = minX1==minX2 & minY1==minY1 & maxX1 == maxX2 & maxY1==maxY2;
     bounds = new BBox(Math.min(minX1,minX2),Math.max(maxX1,maxX2),
                       Math.min(minY1,minY2),Math.max(maxY1,maxY2));
     defined = true;
     return true;
  }

  public BBox getBoundingBox(){
      if(!defined){
         if(Environment.DEBUG_MODE)
	    System.err.println("PMRectLinear.getBoundingBox called with an undefined instance");
         return null;
      }
      if(bounds==null & Environment.DEBUG_MODE){
           System.err.println("PMRectLinear.getBoundingBox called without a bounding box");
      }
      return bounds;
  }

  private double minX1;
  private double minY1;
  private double maxX1;
  private double maxY1;
  private double minX2;
  private double minY2;
  private double maxX2;
  private double maxY2;

  private BBox bounds;

}
