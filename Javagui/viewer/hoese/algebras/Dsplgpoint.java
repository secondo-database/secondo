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

import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

import javax.swing.JOptionPane;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.hoese.*;
import viewer.hoese.algebras.network.GPoint;
import viewer.hoese.algebras.network.NetworkNotAvailableException;

/**
 * @author Martin Scheppokat
 *
 * Views GPoints in the Hoese-Viewer
 *
 */
public class Dsplgpoint extends DisplayGraph
{
  GPoint m_xGPoint;

  /**
   *
   */
  public Dsplgpoint()
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
      Point2D.Double p = m_xGPoint.getRenderObject();
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
        xShape = new Rectangle2D.Double(xPoint.getX()- dPointSizeX/2, xPoint.getY() - dPointSizeY/2, dPointSizeX, dPointSizeY);
      }
      else
      {
        xShape = new Ellipse2D.Double(xPoint.getX()- dPointSizeX/2, xPoint.getY() - dPointSizeY/2, dPointSizeX, dPointSizeY);
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
    return 1;
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
     return new Rectangle2D.Double(m_xGPoint.getRenderObject().x,
                                   m_xGPoint.getRenderObject().y,
                                   0,
                                   0);
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
      if(isUndefined(in_xValue))
      {
        inout_xQueryResult.addEntry(new String("" + AttrName + ": undefined"));
        return;
      }


      AttrName = extendString(name, nameWidth, indent);
      m_xGPoint= new GPoint(in_xValue);
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
      Reporter.writeError("Error in ListExpr :parsing aborted");
      inout_xQueryResult.addEntry(new String("(" + AttrName + ": error)"));
      return;
    }
  }
}
