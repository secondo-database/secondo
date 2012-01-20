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

package viewer.hoese.algebras.network;

import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

import sj.lang.ListExpr;
import viewer.hoese.LEUtils;
import viewer.hoese.ProjectionManager;

/**
 * A junction of a network
 *
 * @author Martin Scheppokat
 *
 */
public class Junction
{

  /**
   * The point this junction lies at.
   */
  private Point2D.Double m_xPoint;

  /**
   * Constructor
   *
   * @param in_xList Representation in secondo list-format
   * @throws Exception
   */
  public Junction(ListExpr in_xList)
  throws Exception
  {
    // We are only interested in the exakt location that has already
    // been calculated by the server.
    ListExpr xPointList = in_xList.sixth();

    double koord[] = new double[2];
    if (xPointList.listLength() != 2)
    {
      // TODO: Handle undefined point in networks
      m_xPoint = new Point2D.Double();
      boolean bSuccess = ProjectionManager.project(1, 1, m_xPoint);
      return;
    }

    // Create Point
    double dX = LEUtils.readNumeric(xPointList.first()).doubleValue();
    double dY = LEUtils.readNumeric(xPointList.second()).doubleValue();
    m_xPoint = new Point2D.Double(dX,dY);
    /*boolean bSuccess = ProjectionManager.project(dX, dY, m_xPoint);
    if (!bSuccess) {
      System.err.println("Error in projection of a junction");
      throw new Exception("Error: No success in projection.");
    }
    */
  }

  /**
   * Returns a Point displayable by the viewer
   * @param af
   * @return
   */
  private Point2D.Double rendRes = new Point2D.Double(0,0);
  public Point2D.Double getRenderObject()
  {
    if(ProjectionManager.project(m_xPoint.x,m_xPoint.y,rendRes)){
      return rendRes;
    }
    return m_xPoint;
  }

}
