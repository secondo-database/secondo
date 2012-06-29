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

package  viewer.hoese.algebras.jnet;

import java.awt.geom.*;
import java.awt.*;
import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;



/**
 * JLine
 * Describes a single position in the jnet network.
 */
public class JLine{

   private String netId;
   private Rectangle2D.Double bounds = null;
   private Vector<JSection> parts = new Vector<JSection> ();


  public JLine(ListExpr value) throws JNetworkNotAvailableException {
    if (value.listLength() == 2){
      netId = value.first().stringValue();
      JNetwork jnet = JNetworkManager.getInstance().getNetwork(netId);
      ListExpr rintList = value.second();
      while (!rintList.isEmpty()){
        JRouteInterval actInt = new JRouteInterval(rintList.first());
        parts.add(jnet.getSection(actInt));
        rintList = rintList.rest();
      }
    } else {
      netId = "undefined";
    }
  }

  public String toString(){
    if (netId.compareTo("undefined") != 0)
      return "jline";
    else
      return "undefined";
  }

  public Rectangle2D.Double getBounds(){
    if (parts.size()>0){
      Rectangle2D.Double res = ((JSection)parts.get(0)).getBounds();
      for (int i = 1; i < parts.size(); i++){
        Rectangle2D.Double next = ((JSection)parts.get(i)).getBounds();
        if (next != null)
          res.add(next);
      }
      return res;
    }
    return null;
  }

  public int numOfShapes(){
    return (2*parts.size());
  }

  public boolean isPointType(int no){
    return isArrowType(no);
  }

  public boolean isLineType(int no){
    return (no < parts.size());
  }

  public Shape getRenderObject(int no, AffineTransform af, double pointSize,
                               boolean asRect){
    if (isLineType(no)){
      return ((JSection)parts.get(no)).getRenderObject(0,af,pointSize);
    }
    if (isArrowType(no)){
      return ((JSection)parts.get(no-parts.size())).getRenderObject(1, af, pointSize);
    }
    return null;
  }

  private boolean isArrowType(int no){
    return (parts.size() <= no && no < 2*parts.size());
  }

}



