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

package project;

import tools.Reporter;

public class Cylindrical implements Projection{


   public boolean project(double lambda, double phi, java.awt.geom.Point2D.Double result){
     if(phi>90 || phi<90){
        result.x = lambda;
        result.y = phi;
        return false;
     }
     result.x = lambda-Lambda_0;
     if( phi<=(-90+secure_distance))
	      phi = -90+secure_distance;
     if( phi>=(90-secure_distance))
        phi = 90-secure_distance;

     double phi_2 = phi*PI/180;
     double y =   Math.tan(phi_2);
     result.y =  y*180/PI;
     return true;
   }

   public boolean showSettings(){
     Reporter.writeWarning("Cylindrical.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Cylindrical";
   }


   public boolean getOrig(double x, double y, java.awt.geom.Point2D.Double result){
      result.x = x + Lambda_0;
      result.y = (Math.atan((y * PI) / 180))*180/PI;
      return true;
   }    

   public boolean isReversible(){
      return true;
   }

   private double Lambda_0 = 0;
   private double secure_distance = 10;

}

