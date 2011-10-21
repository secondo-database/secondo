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
import java.awt.geom.Rectangle2D;

import javax.swing.JOptionPane;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.jnet.JLine;
import viewer.hoese.algebras.jnet.JNetworkNotAvailableException;

/**
 * @author Simone Jandt
 *
 * Views JLines in the Hoese-Viewer
 *
 */
public class Dspljline extends DisplayGraph
{
  /**
   * The JLine to be displayed
   */
  JLine jline;

  /**
   * Constructor
   */
  public Dspljline()
  {
    super();
  }

  /**
   * Returns the shape to be displayed in the HoeseViewer.
   *
   * @return A shape
   */
  public Shape getRenderObject(int index,
                               AffineTransform in_xAf)
  {
    try
    {
      return jline.getRouteIntervalAt(index).getRenderObject();
    }
    catch (JNetworkNotAvailableException xEx)
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
    return jline.getRouteIntervalCount();
  }

  /**
   * Returns wether a shape displays a point or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a point
   */
  public boolean isPointType(int in_iIndex)
  {
    return false;
  }

  /**
   * Returns wether a shape displays a line or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a line
   */
  public boolean isLineType(int in_iIndex)
  {
    return true;
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
     for (int i = 0; i < jline.getRouteIntervalCount(); i++) {
       Shape xShape = jline.getRouteIntervalAt(i).getRenderObject();
       Rectangle2D xShapeBounds= xShape.getBounds2D();
       if(xBounds == null){
         xBounds = new Rectangle2D.Double(xShapeBounds.getX(),
                                          xShapeBounds.getY(),
                                          xShapeBounds.getWidth(),
                                          xShapeBounds.getHeight());
       } else
       {
         xBounds.add(xShapeBounds);
       }
     }
     return xBounds;
   }
   catch (JNetworkNotAvailableException xEx)
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

      jline = new JLine(in_xValue);
      inout_xQueryResult.addEntry(this);
    }
    catch(JNetworkNotAvailableException xNEx)
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
      Reporter.writeError("JLine: Error in ListExpr :parsing aborted");
      inout_xQueryResult.addEntry(AttrName + ": error");
      return;
    }
  }
}
