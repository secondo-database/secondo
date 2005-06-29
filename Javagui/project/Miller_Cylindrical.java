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



public class Miller_Cylindrical implements Projection{


   public boolean project(double lambda, double phi, java.awt.geom.Point2D.Double result){
     if(phi>90 || phi<-90)
        return false;
     result.x = lambda - Lambda_0;
     double phi_2 = phi*PI/180;
     double y =  5/4* Math.log(Math.tan(PI/4 + 2*(phi_2/5)))/LOG_E;
     result.y = y*180/PI;
     return true;
   }

   public boolean showSettings(){
     System.out.println("Miller_Cylindrical.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Miller_Cylindrical";
   }

   public boolean isReversible(){
     return true;
   }
  
   public boolean getOrig(double x, double y, java.awt.geom.Point2D.Double result){
    try{
      result.y = 180*(2.5* Math.atan(Math.exp((y*4*PI)/(180*5))) - PI*5/8)/PI;
      result.x = x + Lambda_0;
      return true;
    } catch(Exception e){
       return false;
    }
   }

   private double Lambda_0 = 0;
   private double secure_distance = 1;

}
