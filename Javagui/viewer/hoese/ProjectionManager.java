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

package viewer.hoese;

import project.*;


public class ProjectionManager {


   public static boolean project(double x, double y, java.awt.geom.Point2D.Double result){
      return P.project(x,y,result);
   }   


   public static boolean showSettings(){
      return P.showSettings();
   }

   public static String getName(){
      return P.getName();
   }

   public static Projection getVoidProjection(){
      return VP;
   }

   public static Projection getActualProjection(){
      return P;
   }

   public static void setProjection(Projection Prj){
     if(Prj!=null)
       P = Prj;
   }

   public static boolean isReversible(){
      return P.isReversible();
   }

   public static boolean getOrig(double px, double py, java.awt.geom.Point2D.Double result){
      return P.getOrig(px,py,result);
   } 

private static Projection P = new  VoidProjection();

private static Projection VP = new VoidProjection();

}
