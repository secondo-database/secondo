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

public class PMPLinear extends LinearMove{

  public PMPLinear(){
    x1=x2=y1=y2=0.0;
    interval = new RelInterval();
    defined = false;
  }

  public String getName(){
    return "pmpoint";
  }


  /** returns a Point2D.Double */
  public Object getObjectAt(Time T){
     if(!defined){
        if(Environment.DEBUG_MODE)
	   System.err.println("PMPLinear.getObjectAt called on an undefined instance");
        return null;
     }
     if(!interval.contains(T)){
        return null;
     }
     double p = interval.where(T);
     Object res = new Point2D.Double(x1+p*(x2-x1),y1+p*(y2-y1));
     return res;
  }

  protected boolean readStartEnd(ListExpr start, ListExpr end){
     if(start.listLength()!=2 || end.listLength()!=2){
       if(Environment.DEBUG_MODE)
          System.err.println("PMPLinear.readStartEnd wrong ListLength");
       return false;
     }
     Double X1 = LEUtils.readNumeric(start.first());
     Double Y1 = LEUtils.readNumeric(start.second());
     Double X2 = LEUtils.readNumeric(end.first());
     Double Y2 = LEUtils.readNumeric(end.second());
     if(X1==null || X2==null || Y1==null || Y2 == null){
       if(Environment.DEBUG_MODE)
         System.err.println("PMPLinear.readStartEnd : found a non-numeric");
       return false;
     }
     x1 = X1.doubleValue();
     y1 = Y1.doubleValue();
     x2 = X2.doubleValue();
     y2 = Y2.doubleValue();
     isstatic = (x1==x2) & (y1==y2);
     bounds = new BBox(x1,x2,y1,y2);
     defined = true;
     return true;
  }

  public BBox getBoundingBox(){
      if(!defined){
         if(Environment.DEBUG_MODE)
	    System.err.println("PMPLinear.getBoundingBox called with an undefined instance");
         return null;
      }
      if(bounds==null & Environment.DEBUG_MODE){
           System.err.println("PMPLinear.getBoundingBox called without a bounding box");
      }
      return bounds;
  }

  private double x1;
  private double x2;
  private double y1;
  private double y2;
  private BBox bounds;


}
