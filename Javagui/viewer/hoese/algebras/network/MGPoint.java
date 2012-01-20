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

import java.awt.geom.Point2D;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.swing.JOptionPane;

import sj.lang.ListExpr;
import viewer.hoese.*;
import viewer.hoese.algebras.IntervalSearch;

/**
 * Represent a moving point defined via a network.
 *
 * @author Martin Scheppokat
 *
 */
public class MGPoint
{

  /**
   * The time-sliced units this moving point consists of.
   */
  private Map m_xUGPoints;

  /**
   * Constructor.
   *
   * @param in_xList List in secondo-format
   * @throws NetworkNotAvailableException
   */
  public MGPoint(ListExpr in_xList)
    throws NetworkNotAvailableException
  {
    m_xUGPoints = new HashMap();
    int iNetworkId = 0;

    // Read out all units
    ListExpr xRestList = in_xList;
    while (!xRestList.isEmpty())
    {
      // Next part of the list
      ListExpr xGPointList = xRestList.first();

      // Split into two parts
      ListExpr xIntervalList = xGPointList.first();
      ListExpr xPointList = xGPointList.second();

      // Read time interval
      Interval xInterval = LEUtils.readInterval(xIntervalList);

      // Read unit
      iNetworkId = xPointList.first().intValue();
      int iRouteId = xPointList.second().intValue();
      int iSideId = xPointList.third().intValue();
      double dPos1 = xPointList.fourth().realValue();
      double dPos2 = xPointList.fifth().realValue();

      // Create new point and add it to the list
      UGPoint xUGPoint = new UGPoint(xInterval,
                                  iNetworkId,
                                  iRouteId,
                                  iSideId,
                                  dPos1,
                                  dPos2);
      m_xUGPoints.put(xInterval, xUGPoint);

      xRestList = xRestList.rest();

    }
    // Check if the network is available. This will throw an exception
    // if it is not.
    NetworkManager.getInstance().getNetwork(iNetworkId);
  }

  /**
   * Returns the position of the point at a specific time
   *
   * @param in_dTime
   * @return
   * @throws NetworkNotAvailableException
   */
  public Point2D.Double getPointAtTime(double in_dTime)
    throws NetworkNotAvailableException
  {
    // Search for a interval the time fits into
    Vector xIntervals = getIntervals();
    int iIntervalIndex = IntervalSearch.getTimeIndex( in_dTime,
                                                        xIntervals);
    if(iIntervalIndex<0)
    {
      return null;
    }

    // Get Unit
    Interval xInterval = (Interval)xIntervals.get(iIntervalIndex);
    UGPoint xUGPoint = (UGPoint)m_xUGPoints.get(xInterval);

    // Calculate the position between start and end
    double t1 = xInterval.getStart();
    double t2 = xInterval.getEnd();
    double dTimeDelta = (in_dTime-t1)/(t2-t1);
    double dDistanceDelta = xUGPoint.getPos2() - xUGPoint.getPos1();
    double dDistanceOnRoute = xUGPoint.getPos1() + dDistanceDelta * dTimeDelta;

    // Get Network and find the points absolute position.
    long lNetworkId = xUGPoint.getNetworkId();
    Network xNetwork = NetworkManager.getInstance().getNetwork(lNetworkId);
    int iRouteId = xUGPoint.getRouteId();
    Route xRoute = xNetwork.getRouteById(iRouteId);
    Point2D.Double xPoint = xRoute.getPointOnRoute(dDistanceOnRoute);
    return xPoint;
    /*
    Point2D.Double xPointP = new Point2D.Double(0,0);
    if (ProjectionManager.project(xPoint.x, xPoint.y, xPointP)){
      return xPointP;
    } else {
      return xPoint;
    }
    */
  }

  /**
   * Get all time-slices
   */
  public Vector getIntervals()
  {
    return new Vector(m_xUGPoints.keySet());
  }
}
