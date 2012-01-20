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
package viewer.hoese.algebras;


import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;

import javax.swing.JOptionPane;

import viewer.hoese.CurrentState;
import viewer.hoese.algebras.network.GPoint;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.network.GPoints;
import viewer.hoese.algebras.network.NetworkNotAvailableException;

/**
 * @author Simone Jandt
 *
 * Views GPoints in the Hoese-Viewer
 *
 */
public class Dsplgpoints extends DisplayGraph
{
  /**
   * The GPoints to be displayed
   */
  GPoints m_GPoints;

  /**
   * Constructor
   */
  public Dsplgpoints()
  {
    super();
  }

  /**
   * Returns the shape to be displayed in the HoeseViewer.
   *
   * @return A shape
   */
  public Shape getRenderObject(int in_iIndex,
                               AffineTransform in_xAf)
  {
    try
    {
      Point2D.Double p = m_GPoints.getGPointAt(in_iIndex).getRenderObject();
      Point2D.Double xPoint = new Point2D.Double(0,0);
      if (!ProjectionManager.project(p.x,p.y,xPoint)){
        return null;
      }

      double dPointSize = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
      double dPointSizeX = Math.abs(dPointSize/in_xAf.getScaleY());
      double dPointSizeY = Math.abs(dPointSize/in_xAf.getScaleX());
      Shape xShape;
      if (Cat.getPointasRect())
      {
        xShape =
          new Rectangle2D.Double(xPoint.getX()- dPointSizeX/2,
                                 xPoint.getY() - dPointSizeY/2,
                                 dPointSizeX, dPointSizeY);
      }
      else
      {
        xShape =
          new Ellipse2D.Double(xPoint.getX()- dPointSizeX/2,
                               xPoint.getY() - dPointSizeY/2,
                               dPointSizeX, dPointSizeY);
      }
      return  xShape;
    }
    catch (NetworkNotAvailableException xEx)
    {
      // If the network is availabe is checked in init. Their is no possibility
      // to output an error message here. But it will propably never happen.
      return null;
    }
  }


  /**
   * Returns the number of shapes to be displayed.
   *
   * @return A number
   */
  public int numberOfShapes()
  {
    return m_GPoints.getGPointsCount();
  }

  /**
   * Returns wether a shape displays a point or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a point
   */
  public boolean isPointType(int in_iIndex)
  {
    return true;
  }

  /**
   * Returns wether a shape displays a line or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a line
   */
  public boolean isLineType(int in_iIndex)
  {
    return false;
  }

  /**
  * Returns the bounds of all objects displayed
  *
  * @return The boundingbox of the drawn Shape
  */
 public Rectangle2D.Double getBounds ()
 {
   try
   {
     Rectangle2D.Double xBounds = null;
     for (int i = 0; i < m_GPoints.getGPointsCount(); i++) {
       GPoint gp =  m_GPoints.getGPointAt(i);
       Rectangle2D.Double actRect =
        new Rectangle2D.Double(gp.getRenderObject().x,
                               gp.getRenderObject().y,
                               0,0);
       if(xBounds == null){
         xBounds = actRect;
       } else
       {
         xBounds.add(actRect);
       }
     }
     return xBounds;
   }
   catch (NetworkNotAvailableException xEx)
   {
     // If the network is availabe is checked in init. Their is no possibility
     // to output an error message here. But it will propably never happen.
     return null;
   }
 }


  /**
   * Initializes this class.
   *
   * @param in_xType The type of the object
   * @param in_xValue Nestedlist representation of the object
   * @param inout_xQueryResult Result object. If everything is fine the object
   * will be added.
   */
  public void init(String name,
                   int nameWidth, int indent,
                   ListExpr in_xType,
                   ListExpr in_xValue,
                   QueryResult inout_xQueryResult)
  {
    try
    {
      AttrName = extendString(name,nameWidth, indent);
      if(isUndefined(in_xValue))
      {
        inout_xQueryResult.addEntry(AttrName + ": undefined");
        return;
      }

      m_GPoints = new GPoints(in_xValue);
      inout_xQueryResult.addEntry(this);
    }
    catch(NetworkNotAvailableException xNEx)
    {
      err = true;
      Reporter.writeError(xNEx.getMessage());
      inout_xQueryResult.addEntry(new String("(" +
                                             AttrName + ": " +
                                             xNEx.getMessage() +
                                             ")"));
      return;
    }
    catch(Exception xEx)
    {
      xEx.printStackTrace();
      err = true;
      Reporter.writeError("GPoints: Error in ListExpr :parsing aborted");
      inout_xQueryResult.addEntry(AttrName + ": error");
      return;
    }
  }
}
