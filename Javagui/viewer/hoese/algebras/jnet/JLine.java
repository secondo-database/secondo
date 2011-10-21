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

import java.util.Vector;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * A line defined via a network
 *
 * @author Simone Jandt
 */
public class JLine
{
  /**
   * The Network this Line belongs to
   */
  String netId;

  /**
   * The parts of this line. Each part is
   * one section on a route in the network.
   */
  Vector routesv;

  /**
   * Constructor. Builds a <code>JLine</code> from
   * a text in secondo list-format.
   *
   * @param in_xValue JLine in secondo's text-format
   * @throws JNetworkNotAvailableException
   */
  public JLine(ListExpr inlist)
    throws JNetworkNotAvailableException
  {
    // Initialize members
    routesv = new Vector();

    // Input correct?
    if(inlist.listLength() != 2)
    {
      Reporter.writeError("JLine: List length must be 2.");
    }

    // Split input into two parts
    ListExpr netIdList = inlist.first();
    ListExpr intervalList = inlist.second();

    // Read out network-id
    netId = netIdList.stringValue();
    JNetwork net = JNetManager.getInstance().getJNetwork(netId);

    // Read routes from list

    while(!intervalList.isEmpty()){

      ListExpr currList = intervalList.first();
      routesv.add(new JRouteInterval(currList, net));
      intervalList = intervalList.rest();
    }
  }

  /**
   * Get-Method for the parts of the route
   *
   * @param in_iIndex Index between 0 and <code>getRouteIntervalCount()</code>
   * @return One part of the route
   */
  public JRouteInterval getRouteIntervalAt(int index)
  {
    JRouteInterval actInterval = (JRouteInterval)routesv.get(index);
    return actInterval;
  }

  /**
   * Returns the number of intervals this JLine has.
   *
   * @return The number of intervals
   */
  public int getRouteIntervalCount()
  {
    return routesv.size();
  }
}
