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

import java.util.Properties;

import tools.Reporter;

public class OSMMercator implements Projection{

  public boolean project(double lon, double lat, java.awt.geom.Point2D.Double result){
    
     if(lat>90 || lat<-90)
        return false;

     if(lat<=(-90+secure_distance))
	       lat = -90+secure_distance;
     if(lat>=(90-secure_distance))
        lat = 90-secure_distance;


     if(lon<-180 || lon >180){
         return false;
     }

  		double mx = lon * this.originShift / 180.0;
	  	double my = Math.log( Math.tan((90.0 + lat) * Math.PI / 360.0 )) / (Math.PI / 180.0);
  		my = my * this.originShift / 180.0;
	  	result.setLocation(mx,my);
      return true;
  }
   
  public boolean showSettings(){
     Reporter.writeWarning("Mercator.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "OSMMercator";
   }

   public boolean isReversible(){
      return true;
   }


  public boolean getOrig(double mx, double my, java.awt.geom.Point2D.Double result){

     double lon =  (mx / this.originShift) * 180.0;
   	 double lat =  (my / this.originShift) * 180.0;
		 lat = 180 / Math.PI * (2 * Math.atan( Math.exp( lat * Math.PI / 180.0)) - Math.PI / 2.0);
     result.setLocation(lon,lat);
     return true;
  }

	public Properties getProperties() {
		Properties p = new Properties();
		p.setProperty(KEY_ORIGINSHIFT, "" + originShift);
		p.setProperty(KEY_SECDISTANCE, "" + secure_distance);
		return p;
	}

	public boolean setProperties(Properties p) {
		try {
			String o = p.getProperty(KEY_ORIGINSHIFT);
			originShift = Double.parseDouble(o);
			o = p.getProperty(KEY_SECDISTANCE);
			secure_distance = Double.parseDouble(o);
		} catch (Exception e) { // set defaults
			originShift = Math.PI * 6378137.0;
			secure_distance = 1.0;
			return false;
		}
		return true;
	}

	private static String KEY_ORIGINSHIFT = "ORIGINSHIFT";
	private static String KEY_SECDISTANCE = "SECDISTANCE";

  double originShift =  Math.PI * 6378137.0;
  double secure_distance = 1.0;
}
