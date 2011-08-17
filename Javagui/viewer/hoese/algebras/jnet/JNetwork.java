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
  private String netId;

  /**
   * All routes of this network
   */
  private JSection[] sects;

  /**
   * All junctions of this network
   */
  private JJunction[] juncts;

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

    // As a jnetwork consists of identifier, routes, junctions and sections,
    // the length of the list should be four.
    // The type-identifier "jnetwork" is not passed to this function.
    if(in_xValue.listLength() != 4)
    {
      Reporter.writeError("Error in ListExpr");
    }

    // Split into the three parts
    ListExpr idList = in_xValue.first();
    ListExpr juncList = in_xValue.second();
    ListExpr sectList = in_xValue.third();

    // Transfer Network-ID
    netId = idList.stringValue();

    // Read routes from list
    while(!sectList.isEmpty()){

      ListExpr currSectList = sectList.first();
      sectv.add(new JSection(currSectList));
      sectList = sectList.rest();
    }

    // Read Junctions from list
    while(!juncList.isEmpty()){

      ListExpr currJuncList = juncList.first();
      junctv.add(new JJunction(currJuncList));
      juncList = juncList.rest();
    }

    sects = (JSection[])sectv.toArray(new JSection[0]);
    juncts = (JJunction[])junctv.toArray(new JJunction[0]);

    JNetManager.getInstance().addNetwork(this);

  }

  /**
   * Get the number of sections in the network
   *
   * @return Number of routes
   */
  public int getSectionCount()
  {
    return sects.length;
  }

  /**
   * Get a section in the network.
   * @param in_iIndex between 0 and <code>getSectionCount()</code>
   * @return
   */
  public JSection getSectionAt(int index)
  {
    return sects[index];
  }


  /**
   * Get the number of junctions in the network
   *
   * @return Number of Junctions
   */
  public int getJunctionCount()
  {
    return juncts.length;
  }

  /**
   * Get a junction of the network
   *
   * @param in_iIndex between 0 and <code>getJunctionCount</code>
   * @return A junction
   */
  public JJunction getJunctionAt(int index)
  {
    return juncts[index];
  }

  /**
   * Searches for a section
   *
   * @param in_iId of the section
   * @return A route.
   */
  public JSection getSectionById(int id)
  {
    for (int i = 0; i < sects.length; i++)
    {
      JSection currSection = sects[i];
      if(currSection.getId() == id)
      {
        return currSection;
      }
    }
    // No route found
    throw new RuntimeException("Section with id " + id + " not found.");
  }

  /**
   * Searches the section a route id and a position
   *
   * @param rid routeid of the position
   * @param pos distance of the position from the start of the route
   * @param side direction
   * @return A section
   */
  public JSection getSection(int rid, double pos, String side)
  {
    return getSection(new JRouteLocation(rid, pos, side));
  }

  /**
   * Searches the section a route id and a position
   *
   * @param rloc JRouteLocation
   * @return A section
   */
  public JSection getSection(JRouteLocation rloc)
  {
    for (int i = 0; i < sects.length; i++)
    {
      JSection currSection = sects[i];
      if (currSection.contains(rloc))
        return currSection;
    }
    // No route found
    throw new RuntimeException("No Section containing route location found.");
  }

  /**
   * Searches the section a route id, a start position, a end position and a
   * side value
   *
   * @param rid routeid of the position
   * @param from distance of the start from the start of the route
   * @param to distance of the end from the start of the route
   * @param side
   * @return A section
   */

  public JSection[] getSections(int rid, double from, double to, String side)
  {
    return getSections(new JRouteInterval(rid, from, to, side));
  }

  /**
   * Searches the section a route id and a position
   *
   * @param rint JRouteInterval
   * @return A section
   */
  public JSection[] getSections(JRouteInterval rint)
  {
    Vector sectsv = new Vector();
    for (int i = 0; i < sects.length; i++)
    {
      JSection currSection = sects[i];
      if (currSection.intersects(rint))
        sectsv.add(currSection);
    }
    return (JSection[])sectsv.toArray(new JSection[0]);
  }

  /**
   * Returns the id of this network
   * @return An id
   */
  public String getId()
  {
    return netId;
  }
}
