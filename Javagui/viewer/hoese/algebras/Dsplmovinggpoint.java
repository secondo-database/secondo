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

import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.hoese.*;
import viewer.hoese.algebras.network.MGPoint;
import viewer.hoese.algebras.network.NetworkNotAvailableException;


/**
 * @author Martin Scheppokat
 *
 * Views moving GPoints in the Hoese-Viewer
 *
 */
public class Dsplmovinggpoint extends DisplayTimeGraph
{
  Rectangle2D.Double m_xBounds;

  private MGPoint m_xMovingPoint;


  public int numberOfShapes()
  {
     return 1;
  }



  /**
   * Gets the shape of this instance at the ActualTime.
   *
   * @param in_xAt The actual transformation, used to calculate the correct size.
   * @return Rectangle or Circle Shape if ActualTime is defined otherwise null.
   */
  public Shape getRenderObject (int num,
                                AffineTransform in_xAt)
  {
    try
    {
      if(num != 0)
      {
        return null;
      }
      if(Intervals == null)
      {
        return null;
      }
      if(RefLayer == null)
      {
        return null;
      }

      double dTime = RefLayer.getActualTime();

      Point2D.Double xPoint = m_xMovingPoint.getPointAtTime(dTime);

      if(xPoint == null)
      {
        return null;
      }
      if(!ProjectionManager.project(xPoint.x,xPoint.y,xPoint)){
        return null;
      }


      double dPointSize = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
      double dPointSizeX = Math.abs(dPointSize/in_xAt.getScaleY());
      double dPointSizeY = Math.abs(dPointSize/in_xAt.getScaleX());
      Shape xShape;
      if (Cat.getPointasRect())
      {
        xShape = new Rectangle2D.Double(xPoint.getX()- dPointSizeX/2,
                                        xPoint.getY() - dPointSizeY/2,
                                        dPointSizeX,
                                        dPointSizeY);
      }
      else
      {
        xShape = new Ellipse2D.Double(xPoint.getX()- dPointSizeX/2,
                                      xPoint.getY() - dPointSizeY/2,
                                      dPointSizeX,
                                      dPointSizeY);
      }
      return  xShape;
    }
    catch(NetworkNotAvailableException xEx)
    {
      // If the network is availabe is checked in init. Their is no possibility
      // to output an error message here. But it will propably never happen.
      return null;

    }
  }



  /**
   * Returns wether a shape displays a point or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a point
   */
  public boolean isPointType(int num)
  {
     return true;
  }

  /**
   * Initializes this class.
   *
   * @param in_xType The type of the object
   * @param in_xValue Nestedlist representation of the object
   * @param inout_xQueryResult Result object. If everything is fine the object
   * will be added.
   */
  public void init (String name,
                    int nameWidth,
                    int indent,
                    ListExpr in_xType,
                    ListExpr in_xValue,
                    QueryResult inout_xQueryResult)
  {
    try
    {
      AttrName = extendString(name, nameWidth, indent);
      m_xMovingPoint = new MGPoint(in_xValue);
      Intervals = m_xMovingPoint.getIntervals();
      inout_xQueryResult.addEntry(this);
      m_xBounds = null;
      TimeBounds = null;
      if(Intervals==null)
      {
        // empty moving point
        return;
      }
      for (int j = 0; j < Intervals.size(); j++)
      {
        Interval xInterval = (Interval)Intervals.elementAt(j);
        if (TimeBounds == null)
        {
          TimeBounds = xInterval;
        }
        else
        {
          TimeBounds = TimeBounds.union(xInterval);
        }
      }
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
      Reporter.writeError("Error in ListExpr :parsing aborted");
      inout_xQueryResult.addEntry(new String("(" + AttrName + ": error)"));
      return;
    }
  }

  /**
   * Returns the bounds of all objects displayed
   *
   * @return The boundingbox of the drawn Shape
   */
  public Rectangle2D.Double getBounds ()
  {
    return m_xBounds;
  }
}
