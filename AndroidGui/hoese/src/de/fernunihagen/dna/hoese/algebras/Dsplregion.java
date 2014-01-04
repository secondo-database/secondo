
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
package de.fernunihagen.dna.hoese.algebras;


import  sj.lang.ListExpr;
import javamini.awt.geom.AffineTransform;
import javamini.awt.geom.Area;
import javamini.awt.geom.GeneralPath;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.ProjectionManager;
import de.fernunihagen.dna.hoese.QueryResult;
import javamini.awt.Shape;


import tools.Reporter;
import de.fernunihagen.dna.hoese.algebras.DisplayGraph;


/**
* The displayclass of the region datatype (Rose algebra).
*/
// extends DisplayGraph
public class Dsplregion extends DisplayGraph {
/** The internal datastructure of the region datatype */
Area areas;
boolean defined;

public int numberOfShapes(){
   return 1;
}

public Shape getRenderObject(int num, AffineTransform at){
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
   // first compute the number of all containing points
   ListExpr TMP  = LE;
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
   boolean empty = true;
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
	      Double x = LEUtils.readNumeric(P.first());
	      Double y = LEUtils.readNumeric(P.second());
	      if(x!=null && y!=null)
	        try{
          if(!ProjectionManager.project(x.doubleValue(),y.doubleValue(),aPoint)){
             Reporter.writeError("error in projection at ("+x+", "+y+")");
              err=true;
              return;
           }else{
	              float x1 = (float)aPoint.x;
	              float y1 = (float)aPoint.y;
              empty = false;
	              if(isFirstPoint){
	                  GP.moveTo(x1,y1);
		                isFirstPoint=false;
	              }else{
	                 GP.lineTo(x1,y1);
	              }
	           } 
        }catch(Exception e){
	           Reporter.writeError("Error in Projection at ("+x+","+y+")");
		          err=true;
		          return;
	        }
	      else{
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




