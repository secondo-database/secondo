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
 * A part of a route in a network.
 *
 * @author Simone Jandt
 *
 */
public class JRouteInterval
{
  /**
  * RouteId of the JRouteInterval
  */
  private int rid;

  /**
  * StartPosition of the JRouteInterval on the route
  */
  private double from;

  /**
  * EndPosition of the JRouteInterval on the route
  */
  private double to;

  /**
  * Side of the route the JRouteInterval is allocated
  */
  private String side;

  /**
   * Constructor
   *
   * @param r RouteIdentifier
   * @param f StartPosition of JRouteInterval on the route
   * @param t EndPosition of JRouteInterval on the route
   * @param dir Side of the route the JRouteInterval is allocated
   */
  public JRouteInterval(int r, double f, double t, String dir)
  {
    rid = r;
    from = f;
    to = t;
    side = dir;
  }


 /**
   * Constructor.
   *
   * @param inList List in secondo-format
   */
   public JRouteInterval(ListExpr inList)
  {
    // TODO: Check format before reading out the values

    rid =  inList.first().intValue();
    from = inList.second().realValue();
    to = inList.third().realValue();
    side = inList.fourth().first().stringValue();
  }

  /**
   * Length of the JRouteInterval
   * @return
   */
  public double getLength()
  {
    return Math.abs(to - from);
  }

  /**
   * Returns the start position of the JRouteInterval
   *
   * @return start position
   */
  public double getStartPosition()
  {
    if (side == "Down") return to;
    else return from;
  }

  /**
   * Returns the end position of the JRouteInterval
   *
   * @return end position
   */
  public double getEndPosition()
  {
    if (side == "Down") return from;
    else return to;
  }

  /**
  * Return side of the JRouteInterval
  *
  * @return side
  */
  public String getSide()
  {
    return side;
  }

  /**
  * Return route identifier of the JRouteInterval
  *
  * @return rid
  */
  public int getRouteId()
  {
    return rid;
  }

  /**
  * Tests if the route Interval contains the route location.
  *
  * @param rloc
  * @return bool
  */
  public boolean contains(JRouteLocation rloc)
  {
    if (rid == rloc.getRouteId() &&
        from <= rloc.getPosition() && rloc.getPosition() <= to)
      return true;
    else
      return false;
  }

  /**
  * Tests if the route Interval intersects the route interval.
  *
  * @param rint
  * @return bool
  */
  public boolean intersects(JRouteInterval rint)
  {
    if (rid == rint.getRouteId() &&
        ((from <= rint.getStartPosition() && rint.getStartPosition() <= to) ||
         (from <= rint.getEndPosition() && rint.getEndPosition() <= to)))
      return true;
    else
      return false;
  }
}