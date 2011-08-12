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

package viewer.hoese.algebras.jnet;

import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

import sj.lang.ListExpr;
import viewer.hoese.LEUtils;
import viewer.hoese.ProjectionManager;

/**
 * A junction of a jnetwork
 *
 * @author Simone Jandt
 *
 */
public class JJunction
{

  /**
   * The point this junction lies at.
   */
  private Point2D.Double pos;

  /**
   * Constructor
   *
   * @param in_xList Representation in secondo list-format
   * @throws Exception
   */
  public JJunction(ListExpr list)
  throws Exception
  {
    // We are only interested in the exakt location that has already
    // been calculated by the server.

    ListExpr pointList = list.second();

    double koord[] = new double[2];
    if (pointList.listLength() != 2)
    {
      // TODO: Handle undefined point in networks
      pos = new Point2D.Double();
      boolean bSuccess = ProjectionManager.project(1, 1, pos);
      return;
    }

    // Create Point
    double dX = LEUtils.readNumeric(pointList.first()).doubleValue();
    double dY = LEUtils.readNumeric(pointList.second()).doubleValue();
    pos = new Point2D.Double();
    boolean bSuccess = ProjectionManager.project(dX, dY, pos);
    if (!bSuccess) {
      throw new Exception("Error: No success in projection.");
    }
  }

  /**
   * Returns a Point displayable by the viewer
   * @param af
   * @return
   */
  public Point2D.Double getRenderObject()
  {
    return pos;
  }

}
