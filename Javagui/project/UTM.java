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



public class UTM implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
     double bw = lambda;
     double lw = phi;
     // zone number??
     long lzn =(int)((lw+180)/6) + 1;
     System.out.println("lzn = "+lzn);
		 long bd = (int)(1+(bw+80)/8);
     double br = bw*PI/180; 
     double tan1 = Math.tan(br);
     double tan2 = tan1*tan1;
     double tan4 = tan2*tan2;
     double cos1 = Math.cos(br);
     double cos2 = cos1*cos1;
     double cos4 = cos2*cos2;
     double cos3 = cos2*cos1;
     double cos5 = cos4*cos1;
     double etasq = ex2*cos2;
     // Querkruemmungshalbmesser nd
     double nd = c/Math.sqrt(1+etasq);
     double g = e0*bw + e2*Math.sin(2*br) + e4*Math.sin(4*br) + e6*Math.sin(6*br);     
     long lh = (lzn - 30)*6 - 3;
     double dl = (lw - lh)*PI/180;
     double dl2 = dl*dl;
     double dl4 = dl2*dl2;
     double dl3 = dl2*dl;
     double dl5 = dl4*dl;
     double x,y;
     if(bw<0){
        x = 10e6 + 0.9996*(g + nd*cos2*tan1*dl2/2 + nd*cos4*tan1*(5-tan2+9*etasq)*dl4/24);
     }else{
        x = 0.9996*(g + nd*cos2*tan1*dl2/2 + nd*cos4*tan1*(5-tan2+9*etasq)*dl4/24) ;
     }
     return x; 
	 }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     double bw = lambda;
     double lw = phi;
     double br = bw*PI/180; 
     long lzn =(int)((lw+180)/6) + 1;
     long lh = (lzn - 30)*6 - 3;
     double cos1 = Math.cos(br);
     double cos2 = cos1*cos1;
     double cos4 = cos2*cos2;
     double cos3 = cos2*cos1;
     double cos5 = cos4*cos1;
     double etasq = ex2*cos2;
     double nd = c/Math.sqrt(1+etasq);
     double dl = (lw - lh)*PI/180;
     double dl2 = dl*dl;
     double dl4 = dl2*dl2;
     double dl3 = dl2*dl;
     double dl5 = dl4*dl;
     double tan1 = Math.tan(br);
     double tan2 = tan1*tan1;
     double tan4 = tan2*tan2;
     double y = 0.9996*(    nd*cos1*dl         + nd*cos3*(1-tan2+etasq)*dl3/6 +
                        nd*cos5*(5-18*tan2+tan4)*dl5/120) + 500000;
     return y;

    }

   public boolean showSettings(){
     return true;
   }

   public String getName(){
     return "UTM";
   }

   public boolean isReversible(){
      return false;
   }

   public double getOrigX(double prjx, double prjy){
      return prjx; 
   }

   public double getOrigY(double prjx, double prjy) throws InvalidInputException{
      return prjy;
   }

   // constants for the wgs84 date
   // big half axe
   protected static double   a = 6378137.000;
   // oblateness
   protected static double   f = 3.35281068e-3; 

   // derived constants
   
   private static double c = a/(1-f); // Polkrümmungshalbmesser in german

  // square of the excentricity 
  protected static double  ex2 = (2*f-f*f)/((1-f)*(1-f));
  protected static double  ex4 = ex2*ex2;
  protected static double  ex6 = ex4*ex2;
  protected static double  ex8 = ex4*ex4;




 // coefficients for computation of the length of the 
 // meridian arc 
 protected static  double e0 = c*(PI/180)*(1 - 3*ex2/4 + 
                               45*ex4/64  - 175*ex6/256  + 11025*ex8/16384);
 protected static double e2 = c*(  - 3*ex2/8 + 15*ex4/32  - 525*ex6/1024 +  2205*ex8/4096);
 protected static double e4 = c*(15*ex4/256 - 105*ex6/1024 +  2205*ex8/16384);
 protected static double e6 = c*( -  35*ex6/3072 +   315*ex8/12288);


}
