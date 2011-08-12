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

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.hoese.CurrentState;
import viewer.hoese.QueryResult;
import viewer.hoese.algebras.jnet.JNetwork;

/**
 * @author Simone Jandt
 *
 * Views Networks in the Hoese-Viewer
 *
 */
public class Dspljnetwork extends DisplayGraph
{
  /**
   * The network to be displayed.
   */
	JNetwork jnet;

	/**
	 * Constructor
	 */
	public Dspljnetwork()
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
		if(in_iIndex < jnet.getSectionCount())
    {
			return jnet.getSectionAt(in_iIndex).getRenderObject();
		}
    else
    {
			int iAdjustedIndex = in_iIndex - jnet.getSectionCount();

      Point2D.Double xPoint = jnet.getJunctionAt(iAdjustedIndex).getRenderObject();

      double dPointSize = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
      double dPointSizeX = Math.abs(dPointSize/in_xAf.getScaleY());
      double dPointSizeY = Math.abs(dPointSize/in_xAf.getScaleX());
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
	}



  /**
   * Returns the number of shapes to be displayed.
   *
   * @return A number
   */
	public int numberOfShapes()
  {
		return jnet.getSectionCount() + jnet.getJunctionCount();
	}

  /**
   * Returns wether a shape displays a point or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a point
   */
	public boolean isPointType(int in_iIndex)
  {
    boolean bPointType = in_iIndex >= jnet.getSectionCount();
    return bPointType;
	}

  /**
   * Returns wether a shape displays a line or not.
   *
   * @param in_iIndex Index of shape
   * @return true if the shape represents a line
   */
	public boolean isLineType(int in_iIndex)
  {
	  boolean bLineType = in_iIndex < jnet.getSectionCount();
    return bLineType;
	}

  /**
    * Returns the bounds of all objects displayed
    *
    * @return The boundingbox of the drawn Shape
    */
 public Rectangle2D.Double getBounds ()
 {
   Rectangle2D.Double xBounds = null;
   for (int i = 0; i < jnet.getSectionCount(); i++) {
     Shape xShape = jnet.getSectionAt(i).getRenderObject();
     Rectangle2D xShapeBounds= xShape.getBounds2D();
     if(xBounds == null){
       xBounds = new Rectangle2D.Double(xShapeBounds.getX(),
                                        xShapeBounds.getY(),
                                        xShapeBounds.getWidth(),
                                        xShapeBounds.getHeight());
     }
     else
     {
       xBounds.add(xShapeBounds);
     }
   }

   for (int i = 0; i < jnet.getJunctionCount(); i++) {
     Point2D.Double xShapeBounds = jnet.getJunctionAt(i).getRenderObject();
     if(xBounds == null){
       xBounds = new Rectangle2D.Double(xShapeBounds.getX(),
                                        xShapeBounds.getY(),
                                        0,
                                        0);
     }
     else
     {
       xBounds.add(xShapeBounds);
     }
   }

   return xBounds;
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
			AttrName = extendString(name, nameWidth, indent);
			jnet = new JNetwork(in_xValue);
			inout_xQueryResult.addEntry(this);
		}
		catch(Exception xEx)
		{
			xEx.printStackTrace();
			err = true;
				Reporter.writeError("Error in ListExpr :parsing aborted");
			inout_xQueryResult.addEntry(new String("(" + AttrName + ": GA(graph))"));
			return;
		}
	}
}
