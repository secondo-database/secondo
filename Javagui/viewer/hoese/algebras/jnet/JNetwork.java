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

package viewer.hoese.algebras.jnet;

import java.awt.geom.*;
import java.awt.*;
import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;



/**
 * JNetwork
 * For the graphical interface we limit us almost to the spatial informations
 * about the jnet.
 */
public class JNetwork{

   private Vector junctions = new Vector<JJunction>();
   private Vector sections = new Vector<JSection>();
   private String name = "";

  public JNetwork(ListExpr value)
  {
    if (value.listLength() == 5){
      name = value.first().stringValue();
      fillJunctions(value.second());
      fillSections(value.third());
    }
    else {
      name = "undefined";
    }
  }

  public int numOfShapes() {
    return junctions.size() + 2*sections.size();
  }

  public boolean isPointType(int no) {
    return (no < junctions.size());
  }

  public boolean isLineType(int no) {
    return (no >= junctions.size() && no < numOfShapes());
  }

  public String toString(){
    return name;
  }

  public Rectangle2D.Double getBounds(){
    if (sections.size()>0){
      Rectangle2D.Double res = ((JSection)sections.get(0)).getBounds();
      for (int i = 1; i < sections.size(); i++){
        Rectangle2D.Double next = ((JSection)sections.get(i)).getBounds();
        if (next != null)
          res.add(next);
      }
      return res;
    } else if (junctions.size() > 0){
      Rectangle2D.Double res = ((JJunction)junctions.get(0)).getBounds();
      for (int i = 1; i < junctions.size(); i++){
        Rectangle2D.Double next = ((JJunction)junctions.get(i)).getBounds();
        if (next != null)
          res.add(next);
      }
      return res;
    } else {
      return null;
    }
  }

  public Shape getRenderObject(int no, AffineTransform af, double pointSize,
                               boolean asRect){
    if (isPointType(no)){
      return ((JJunction)junctions.get(no)).getRenderObject(af, pointSize, asRect);
    }
    if (isLineType(no)){
      if (no < (junctions.size() + sections.size())) {
        return ((JSection)sections.get(no - junctions.size())).getRenderObject(0,af,pointSize);
      } else if (no < numOfShapes() && no > (junctions.size() + sections.size())){
        return ((JSection)sections.get(no-(junctions.size()+sections.size()))).getRenderObject(1, af, pointSize);
      }
    }
    return null;
  }

  private void fillJunctions(ListExpr juncList){
    ListExpr rest = juncList;
    while (!rest.isEmpty()){
      junctions.add(new JJunction(rest.first()));
      rest = rest.rest();
    }
  }

  private void fillSections(ListExpr sectList){
    ListExpr rest = sectList;
    while(!rest.isEmpty()){
      sections.add(new JSection(rest.first()));
      rest = rest.rest();
    }
  }

}



