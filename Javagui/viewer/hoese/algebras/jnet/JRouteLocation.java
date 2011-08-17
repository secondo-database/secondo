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

import sj.lang.ListExpr;

/**
 * A position in a network.
 *
 * @author Simone Jandt
 *
 */
public class JRouteLocation
{
  /**
  * RouteId of the JRouteLocation
  */
  private int rid;

  /**
  * Position JRouteLocation on the route
  */
  private double pos;


  /**
  * Side of the route the JRouteLocation is allocated
  */
  private String side;

  /**
   * Constructor
   *
   * @param r RouteIdentifier
   * @param p Distance of the JRouteLocation from the start of the route
   * @param dir Side of the route the JRouteLocation is allocated
   */
  public JRouteLocation(int r, double d, String dir)
  {
    rid = r;
    pos = d;
    side = dir;
  }

  /**
   * Constructor.
   *
   * @param inList List in secondo-format
   */
   public JRouteLocation(ListExpr inList)
  {
    // TODO: Check format before reading out the values

    rid =  inList.first().intValue();
    pos = inList.second().realValue();
    side = inList.third().first().stringValue();
  }


  /**
   * Returns the position of the JRouteLocation
   *
   * @return position
   */
  public double getPosition()
  {
    return pos;
  }


  /**
  * Return side of the JRouteLocation
  *
  * @return side
  */
  public String getSide()
  {
    return side;
  }

  /**
  * Return route identifier of the JRouteLocation
  *
  * @return rid
  */
  public int getRouteId()
  {
    return rid;
  }
}