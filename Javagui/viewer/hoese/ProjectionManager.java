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

   public static double getPrjX(double lambda, double phi) throws InvalidInputException{
      return P.getPrjX(lambda,phi);
   }

   public static double getPrjY(double lambda, double phi) throws InvalidInputException{
      return P.getPrjY(lambda,phi);
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


// private static  Projection P = new Mercator(); // later the equals-projection
// private static  Projection P = new Mollweide();
private static Projection P = new  VoidProjection();

private static Projection VP = new VoidProjection();

}
