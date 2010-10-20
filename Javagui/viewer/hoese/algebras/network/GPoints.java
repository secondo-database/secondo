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

import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;

import sj.lang.ListExpr;
import tools.Reporter;
import java.util.Vector;

/**
 * A list of gpoint defined via a network.
 *
 * @author Simone Jandt
 *
 */
public class GPoints
{

  /**
   * The List of GPoints
   */
  Vector m_xGPoints;


  /**
   * Constructor.
   *
   * @param in_xList List in secondo-format
   * @throws NetworkNotAvailableException
   */
  public GPoints(ListExpr in_xValue)
    throws NetworkNotAvailableException
  {
    // TODO: Check format before reading out the values
    m_xGPoints = new Vector();

    // Read gpoints from list
    if(in_xValue.isAtom())
    {
      Reporter.writeError("GPoints: List must not be an atom.");
    }
    ListExpr in_xGPointList = in_xValue;
    while(!in_xGPointList.isEmpty())
    {
      GPoint gp = new GPoint(in_xGPointList.first());
      m_xGPoints.add(gp);

      in_xGPointList = in_xGPointList.rest();
    }

    // Check if the network is available. This will throw an exception
    // if it is not.
    NetworkManager.getInstance().getNetwork(getGPointAt(0).getNetworkId());
  }

  /**
   * Get-Method for the single gpoint of gpoints
   *
   * @param in_iIndex Index between 0 and <code>getGPointsCount()</code>
   * @return One GPoint
   */
  public GPoint getGPointAt(int in_iIndex)
  {
    GPoint xGPoint = (GPoint)m_xGPoints.get(in_iIndex);
    return xGPoint;
  }

  /**
   * Returns the number of gpoint this GPoints has.
   *
   * @return The number of intervals
   */
  public int getGPointsCount()
  {
    return m_xGPoints.size();
  }
}

