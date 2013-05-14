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
  /** The internal datastructure of the region2 datatype */
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


  /** convert the Listrepresentation LE to a Area */
  public void ScanValue(ListExpr LE){
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
     defined=true;
     areas = null;

     ListExpr TMP  = LE;
     int outtype = 0;

     if ( TMP.first().isAtom() ) 
     {
       outtype = 1;
       TMP  = LE.second();
     }
     else 
     {
       outtype = 2;
     }

     int no = 0;
     while(!TMP.isEmpty()){
         ListExpr Face = TMP.first();
	 TMP = TMP.rest();
         while(!Face.isEmpty()){
	    no = no+Face.first().listLength()+2;
            Face = Face.rest();
	 }
     }

     GeneralPath GP = new GeneralPath(GeneralPath.WIND_EVEN_ODD,no);
     BigDecimal scaleBD = new BigDecimal(0);
     boolean empty = true;

     if ( outtype == 1 )
     {
       int scaleFactor = LE.first().intValue();
       scaleBD = new BigDecimal(10);
       scaleBD = scaleBD.pow(scaleFactor);
       LE = LE.second();
     }

     while(!LE.isEmpty()){
        ListExpr Face = LE.first();
        LE = LE.rest();
	while(!Face.isEmpty()){
           ListExpr Cycle = Face.first();
	   Face = Face.rest();
           boolean isFirstPoint=true;
	   while(!Cycle.isEmpty()){
	      ListExpr P = Cycle.first();
	      Cycle=Cycle.rest();

	      BigDecimal xBD = new BigDecimal(0);
	      BigDecimal yBD = new BigDecimal(0);

              if ( outtype == 1 )
              {
	      if (P.third().isEmpty())
	      {
	        xBD = new BigDecimal( P.first().intValue());
	        yBD = new BigDecimal( P.second().intValue());
	      }
	      else
	      {
	        String xprecpart = P.third().first().textValue();
	        String yprecpart = P.third().second().textValue();
	        try {
		  if (xprecpart.contains("/")) 
		  {
		    String[] xprec = xprecpart.split("/");
	            xBD = new BigDecimal( xprec[0]);  //numerator x
	            xBD = xBD.divide( new BigDecimal( xprec[1]), 1024, RoundingMode.HALF_UP);  //numerator x / denominator x
		  }
		  else
		    xBD = new BigDecimal( xprecpart );

	          if (yprecpart.contains("/")) 
		  {
		    String[] yprec = yprecpart.split("/");
	            yBD = new BigDecimal( yprec[0]);  //numerator y
	            yBD = yBD.divide( new BigDecimal( yprec[1]), 1024, RoundingMode.HALF_UP);  //numerator y / denominator y
		  }
		  else
		    yBD = new BigDecimal( yprecpart );

	          xBD = xBD.add( new BigDecimal( P.first().intValue()));  //
	          yBD = yBD.add( new BigDecimal( P.second().intValue())); //
	        }
	        catch(Exception e) {
	             Reporter.writeError("Error in Dividing!");
		     err=true;
		     return;
	        }
	      }
	      try {
	        xBD = xBD.divide(scaleBD, 1024, RoundingMode.HALF_UP);
	        yBD = yBD.divide(scaleBD, 1024, RoundingMode.HALF_UP);
	      }
	      catch(Exception e) {
	           Reporter.writeError("Error in Dividing!");
		   err=true;
		   return;
	      }
              }
              if ( outtype == 2 )
	      {
	        xBD = new BigDecimal( P.first().textValue());
	        yBD = new BigDecimal( P.second().textValue());
	      }

	      double x = xBD.doubleValue();
	      double y = yBD.doubleValue();
	      try{
                if(!ProjectionManager.project(x,y,aPoint))
                {
                  Reporter.writeError("error in projection at ("+x+", "+y+")");
                  err=true;
                  return;
                }
                else
                {
	          double x1 = aPoint.x;
	          double y1 = aPoint.y;
                  empty = false;
	          if (isFirstPoint) {
	            GP.moveTo((float)x1,(float)y1);
		    isFirstPoint=false;
	          } else {
	            GP.lineTo((float)x1,(float)y1);
	          }
	        } 
              }catch(Exception e){
	           Reporter.writeError("Error in Projection at ("+x+","+y+")");
		          err=true;
		          return;
	      } 
	   } 
           GP.closePath();
	}
     }

     if(!empty) 
        areas= new Area(GP);
     else
        areas= null;
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



