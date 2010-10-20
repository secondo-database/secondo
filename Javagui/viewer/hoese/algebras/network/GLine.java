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

import java.util.Vector;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * A line defined via a network
 *
 * @author Martin Scheppokat
 */
public class GLine
{
  /**
   * The Network this Line belongs to
   */
  int m_iNetworkId;

  /**
   * The parts of this line. Each part is
   * one section on a route in the network.
   */
  Vector m_xRouteIntervals;

  /**
   * Construktor. Builds a <code>GLine</code> from
   * a text in secondo list-format.
   *
   * @param in_xValue GLine in secondo's text-format
   * @throws NetworkNotAvailableException
   */
  public GLine(ListExpr in_xValue)
    throws NetworkNotAvailableException
  {
    // Initialize members
    m_xRouteIntervals = new Vector();

    // Input correct?
    if(in_xValue.listLength() != 2)
    {
      Reporter.writeError("GLine: List length must be 2.");
    }

    // Split input into two parts
    ListExpr xNetworkIdList = in_xValue.first();
    ListExpr xRouteIntervalList = in_xValue.second();

    // Read out network-id
    if(!xNetworkIdList.isAtom() ||
       xNetworkIdList.atomType() != ListExpr.INT_ATOM)
    {
      Reporter.writeError("GLine: First part of the list must be an int atom.");
    }
    m_iNetworkId = xNetworkIdList.intValue();

    // Read routes from list
    if(xRouteIntervalList.isAtom())
     {
    Reporter.writeError("GLine: Second part of the list must not be an atom.");
     }
    while(!xRouteIntervalList.isEmpty()){

      ListExpr xCurrentRouteList = xRouteIntervalList.first();
      m_xRouteIntervals.add(new RouteInterval(m_iNetworkId ,
                                              xCurrentRouteList));
      xRouteIntervalList = xRouteIntervalList.rest();
    }

    // Check if the network is available. This will throw an exception
    // if it is not.
    NetworkManager.getInstance().getNetwork(m_iNetworkId);
  }

  /**
   * Get-Method for the parts of the route
   *
   * @param in_iIndex Index between 0 and <code>getRouteIntervalCount()</code>
   * @return One part of the route
   */
  public RouteInterval getRouteIntervalAt(int in_iIndex)
  {
    RouteInterval xInterval = (RouteInterval)m_xRouteIntervals.get(in_iIndex);
    return xInterval;
  }

  /**
   * Returns the number of intervals this GLine has.
   *
   * @return The number of intervals
   */
  public int getRouteIntervalCount()
  {
    return m_xRouteIntervals.size();
  }
}
