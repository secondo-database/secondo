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

//2012, June Simone Jandt

package viewer.hoese.algebras;

import viewer.hoese.algebras.jnet.*;
import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import gui.SecondoObject;


/**
 * A Displayclass for graphical representation of JNetwork jnet.
 */
public class Dspljnet extends DisplayGraph {
	/** The internal datatype representation */
	JNetwork jnet;
  Rectangle2D.Double bounds=null;

  public Dspljnet(){
    super();
  }
/**
 * Init jnetwork instance.
 * @param value
 *            the nestedlist representation of the jnet
 * @param qr
 *            queryresult to display output.
 */

public void init(String name, int nameWidth, int indent,ListExpr type,
                 ListExpr value, QueryResult qr){
  AttrName = extendString(name,nameWidth, indent);
  jnet = new JNetwork(value);
  qr.addEntry(this);
}

public String toString(){
  return jnet.toString();
}

 public int numberOfShapes(){
    return jnet.numOfShapes();
 }

 public boolean isPointType(int no){
     return jnet.isPointType(no);
 }

 public boolean isLineType(int no){
    return jnet.isLineType(no);
 }


/**
 * Returns the bounding rectangle of the jnetwork by creating a union of the
 * bounds of all junctions and sections
 * @return The bound rectangle of the graph
 */

public Rectangle2D.Double getBounds(){
  if (bounds == null)
    bounds = jnet.getBounds();
  return bounds;
}


/**
* returns always junctions and sections.
**/
public Shape getRenderObject(int no, AffineTransform af){
  double pointSize = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
  boolean asRect = Cat.getPointasRect();
  return jnet.getRenderObject(no, af, pointSize, asRect);
}

}
