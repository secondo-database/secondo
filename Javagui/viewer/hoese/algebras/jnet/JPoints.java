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

//2012, July Simone Jandt

package  viewer.hoese.algebras.jnet;

import java.awt.geom.*;
import java.awt.*;
import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;



/**
 * JPoints
 * Describe a set of single positions in the jnet network.
 */
public class JPoints{

   private String netId;
   private Rectangle2D.Double bounds = null;
   private Vector<Point2D.Double> poss = new Vector<Point2D.Double>();


  public JPoints(ListExpr value) throws JNetworkNotAvailableException {
    if (value.listLength() == 2){
      netId = value.first().stringValue();
      JNetwork jnet = JNetworkManager.getInstance().getNetwork(netId);
      ListExpr rlocList = value.second();
      while (!rlocList.isEmpty()){
        RouteLocation actLoc = new RouteLocation(rlocList.first());
        Point2D.Double actPoint = jnet.getPosition(actLoc);
        Point2D.Double rendPos = new Point2D.Double(0.0,0.0);
        if (ProjectionManager.project(actPoint.x, actPoint.y, rendPos)){
          actPoint = rendPos;
        }
        poss.add(actPoint);
        rlocList = rlocList.rest();
      }
    } else {
      netId = "undefined";
    }
  }

  public String toString(){
    if (netId.compareTo("undefined") != 0)
      return "jpoints";
    else
      return "undefined";
  }

  public Rectangle2D.Double getBounds(){
    if (poss.size()>0){
      Point2D.Double actPoint = (Point2D.Double)poss.get(0);
      Rectangle2D.Double res = new Rectangle2D.Double(actPoint.getX(),
                                                      actPoint.getY(),
                                                      0.0, 0.0);
      for (int i = 1; i < poss.size(); i++){
        actPoint = (Point2D.Double)poss.get(i);
        Rectangle2D.Double next = new Rectangle2D.Double(actPoint.getX(),
                                                         actPoint.getY(),
                                                         0.0, 0.0);;
        res.add(next);
      }
      return res;
    }
    return null;
  }

  public int numOfShapes(){
    return (poss.size());
  }

  public boolean isPointType(int no){
    return true;
  }

  public boolean isLineType(int no){
    return false;
  }

  public Shape getRenderObject(int no, AffineTransform af, double pointSize,
                               boolean asRect){
    double pointSizeX = Math.abs(pointSize/af.getScaleX());
    double pointSizeY = Math.abs(pointSize/af.getScaleY());
    Shape shape;
    Point2D.Double actPos = (Point2D.Double) poss.get(no);
    if (asRect) {
      shape = new Rectangle2D.Double(actPos.getX()- pointSizeX/2,
                                     actPos.getY()- pointSizeY/2,
                                     pointSizeX,
                                     pointSizeY);
    } else {
      shape = new Ellipse2D.Double(actPos.getX()- pointSizeX/2,
                                   actPos.getY() - pointSizeY/2,
                                   pointSizeX,
                                   pointSizeY);
    }
    return  shape;
  }

}



