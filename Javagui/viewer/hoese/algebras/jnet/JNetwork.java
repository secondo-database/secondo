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
 * A Network consisting of routes and junctions.
 *
 * The network supports finding points on a route
 * @author Simone Jandt
 *
 */
public class JNetwork
{

  /**
   * The id of this network
   */
  private String id;

  /**
   * All routes of this network
   */
  private JRoute[] routes;

  /**
   * All junctions of this network
   */
  private JJunction[] junctions;

  /**
   * Constructor
   *
   * @param in_xValue Network in secondo list-format
   * @throws Exception If an error in the list is found
   */
  public JNetwork(ListExpr in_xValue)
    throws Exception
  {
    Vector sectv = new Vector();
    Vector junctv = new Vector();
    Vector routev = new Vector();

    // As a jnetwork consists of identifier, routes, junctions and sections,
    // the length of the list should be four.
    // The type-identifier "jnetwork" is not passed to this function.
    if(in_xValue.listLength() != 4)
    {
      Reporter.writeError("Error in ListExpr");
    }

    // Split into the four parts
    ListExpr idList = in_xValue.first();
    ListExpr juncList = in_xValue.second();
    ListExpr sectList = in_xValue.third();
    ListExpr routeList = in_xValue.fourth();


    // Transfer Network-ID
    id = idList.stringValue();

    // Read Junctions from list
    while(!juncList.isEmpty()){
      ListExpr currJuncList = juncList.first();
      junctv.add(new JJunction(currJuncList));
      juncList = juncList.rest();
    }

    junctions = (JJunction[])junctv.toArray(new JJunction[0]);

    // Read sections from list
    while(!sectList.isEmpty()){

      ListExpr currSectList = sectList.first();
      sectv.add(new JSection(currSectList));
      sectList = sectList.rest();
    }

    JSection[] section = (JSection[])sectv.toArray(new JSection[0]);

    //Read Routes from List
    while(!routeList.isEmpty()){
      ListExpr currRouteList = routeList.first();
      routev.add(new JRoute(currRouteList, section));
      routeList = routeList.rest();
    }

    routes = (JRoute[])routev.toArray(new JRoute[0]);

    JNetManager.getInstance().addNetwork(this);

  }

  /**
   * Get the id of this network
   * @return An id
   */
  public String getId()
  {
    return id;
  }

  /**
  * Get the number of routes in the network
  *
  * @return Number of routes
  */
  public int getRouteCount() {
    return routes.length;
  }

  /**
   * Get the number of junctions in the network
   *
   * @return Number of Junctions
   */
  public int getJunctionCount()
  {
    return junctions.length;
  }

   /**
   * Get a route in the network.
   * @param in_iIndex between 0 and <code>getRouteCount()</code>
   * @return
   */
  public JRoute getRouteAt(int index)
  {
    return routes[index];
  }

  /**
  * Get route by id
  * @param rid route identifier
  * @return route with id rid
  */
  public JRoute getRouteById(int rid)
  {
    for (int i = 0; i < routes.length; i++)
    {
      JRoute actRoute = routes[i];
      if (actRoute.getId() == rid) return actRoute;
    }
    throw new RuntimeException("Route with id: " + rid + " not found.");
  }

  /**
  * Get junction
  * @param index arrayposition
  * @return junction at arrayposition
  */
  public JJunction getJunctionAt(int index)
  {
    return junctions[index];
  }
}
