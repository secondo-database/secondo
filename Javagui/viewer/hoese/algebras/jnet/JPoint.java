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
 * JPoint
 * Describes a single position in the jnet network.
 */
public class JPoint{

   private String netId;
   private boolean defined;
   private RouteLocation rloc;
   private Point2D.Double pos;

  public JPoint(ListExpr value) throws JNetworkNotAvailableException {
    if (value.listLength() == 2){
      netId = value.first().stringValue();
      rloc = new RouteLocation(value.second());
      if (rloc != null){
        JNetwork jnet = JNetworkManager.getInstance().getNetwork(netId);
        pos = jnet.getPosition(rloc);
        if (pos != null){
          Point2D.Double rendPos = new  Point2D.Double(0.0,0.0);
          if (ProjectionManager.project(pos.x, pos.y, rendPos)){
            pos = rendPos;
          }
        }
      } else {
        pos = null;
      }
      if (pos == null )
        defined = false;
      else
        defined = true;
    } else {
      defined = false;
      netId = "undefined";
    }
  }

  public String toString(){
    if (defined)
      return rloc.toString();
    else
      return "undefined";
  }

  public Point2D.Double getPos(){
    if (defined)
      return pos;
    else
     return null;
  }

  public Rectangle2D.Double getBounds(){
    if (!defined)
      pos = null;
    return new Rectangle2D.Double(pos.x, pos.y,0,0);
  }

  public Shape getRenderObject(int no, AffineTransform af, double pointSize,
                               boolean asRect){
    if (defined)
    {
      double pointSizeX = Math.abs(pointSize/af.getScaleX());
      double pointSizeY = Math.abs(pointSize/af.getScaleY());
      Shape shape;
      if (asRect) {
        shape = new Rectangle2D.Double(pos.getX()- pointSizeX/2,
                                       pos.getY()- pointSizeY/2,
                                       pointSizeX,
                                       pointSizeY);
      } else {
        shape = new Ellipse2D.Double(pos.getX()- pointSizeX/2,
                                     pos.getY() - pointSizeY/2,
                                     pointSizeX,
                                     pointSizeY);
      }
      return  shape;
    }
    return null;

  }

}



