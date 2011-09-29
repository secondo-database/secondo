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
 * A Network consisting of routes and junctions.
 *
 * The network supports finding points on a route
 * @author Scheppokat
 *
 */
public class Network
{

  /**
   * The id of this network
   */
  private long m_lNetworkId;

  /**
   * The scalefactor of this network
   */
  private double m_scalefactor;

  /**
   * All routes of this network
   */
  private Route[] m_xRoutes;

  /**
   * All junctions of this network
   */
  private Junction[] m_xJunctions;

  /**
   * Constructor
   *
   * @param in_xValue Network in secondo list-format
   * @throws Exception If an error in the list is found
   */
  public Network(ListExpr in_xValue)
    throws Exception
  {

    Vector xRoutes = new Vector();
    Vector xJunctions = new Vector();

    // As a network consists of routes, junctions and sections, the length
    // of the list should be three. The type-identifier "Network" is not
    // passed to this function.
    if(in_xValue.listLength() != 4)
    {
      Reporter.writeError("Error in ListExpr");
    }

    // Split into the three parts
    ListExpr xIdList = in_xValue.first();
    ListExpr xScaleList = in_xValue.second();
    ListExpr xRouteList = in_xValue.third();
    ListExpr xJunctionList = in_xValue.fourth();

    // Transfer Network-ID
    m_lNetworkId = xIdList.intValue();

    //Transfer scalefactor
    m_scalefactor = xScaleList.realValue();

    // Read routes from list
    while(!xRouteList.isEmpty()){

      ListExpr xCurrentRouteList = xRouteList.first();
      xRoutes.add(new Route(xCurrentRouteList));
      xRouteList = xRouteList.rest();
    }

    // Read Junctions from list
    while(!xJunctionList.isEmpty()){

      ListExpr xCurrentJunctionList = xJunctionList.first();
      xJunctions.add(new Junction(xCurrentJunctionList));
      xJunctionList = xJunctionList.rest();
    }

    m_xRoutes = (Route[])xRoutes.toArray(new Route[0]);
    m_xJunctions = (Junction[])xJunctions.toArray(new Junction[0]);

    // Register this network with the NetworkManager
    NetworkManager.getInstance().addNetwork(this);

  }

  /**
   * Get the number of routes in teh network
   *
   * @return Number of routes
   */
  public int getRouteCount()
  {
    return m_xRoutes.length;
  }

  /**
   * Get a route in the network.
   * @param in_iIndex between 0 and <code>getRouteCount()</code>
   * @return
   */
  public Route getRouteAt(int in_iIndex)
  {
    return m_xRoutes[in_iIndex];
  }


  /**
   * Get the number of junctions in the network
   *
   * @return Number of Junctions
   */
  public int getJunctionCount()
  {
    return m_xJunctions.length;
  }

  /**
   * Get a junction of the network
   *
   * @param in_iIndex between 0 and <code>getJunctionCount</code>
   * @return A junction
   */
  public Junction getJunctionAt(int in_iIndex)
  {
    return m_xJunctions[in_iIndex];
  }

  /**
   * Searches for a route.
   *
   * @param in_iId of the route
   * @return A route.
   */
  public Route getRouteById(int in_iId)
  {
    for (int i = 0; i < m_xRoutes.length; i++)
    {
      Route xCurrentRoute = m_xRoutes[i];
      if(xCurrentRoute.getId() == in_iId)
      {
        return xCurrentRoute;
      }
    }
    // No route found
    throw new RuntimeException("Route with id " + in_iId + " not found.");
  }

  /**
   * Returns the id of this network
   * @return An id
   */
  public long getId()
  {
    return m_lNetworkId;
  }
}
