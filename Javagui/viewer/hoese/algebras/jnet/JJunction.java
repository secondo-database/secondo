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
 * Junction
 * Spatial position of a junction in the jnet.
 */

public class JJunction{

  private Point2D.Double pos;
  private Point2D.Double rendRes = new Point2D.Double(0.0,0.0);

  public JJunction(ListExpr value)
  {
    if (value.listLength() == 5){
      ListExpr posList = value.second();
      if (posList.listLength() == 2){
         double x = LEUtils.readNumeric(posList.first()).doubleValue();
         double y = LEUtils.readNumeric(posList.second()).doubleValue();
         pos = new Point2D.Double(x,y);
         setRenderPosition();
      }
    } else {
      pos = rendRes;
    }
  }

  public Shape getRenderObject(AffineTransform af, double pointSize,
                               boolean getRect){
    double pointSizeX = Math.abs(pointSize/af.getScaleX());
    double pointSizeY = Math.abs(pointSize/af.getScaleY());
    Shape shape;
    if (getRect) {
      shape = new Rectangle2D.Double(rendRes.getX()- pointSizeX/2,
                                     rendRes.getY()- pointSizeY/2,
                                     pointSizeX,
                                     pointSizeY);
    } else {
      shape = new Ellipse2D.Double(rendRes.getX()- pointSizeX/2,
                                   rendRes.getY() - pointSizeY/2,
                                   pointSizeX,
                                   pointSizeY);
    }
    return  shape;
  }

  public Rectangle2D.Double getBounds(){
    return new Rectangle2D.Double(rendRes.getX(), rendRes.getY(),0.0,0.0);
  }

  private void setRenderPosition(){
    if(!ProjectionManager.project(pos.x,pos.y,rendRes))
      rendRes = pos;
  }
}


