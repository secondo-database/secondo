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

/**
 * A linear part of a route in a network.
 *
 * @author Martin Scheppokat
 *
 */
public class Segment
{
  /**
   * Define for orientation of this segment
   */
  public static final int STARTS_SMALLER = 1;

  /**
   * Define for orientation of this segment
   */
  public static final int STARTS_BIGGER = 2;

  /**
   * Starting point of the segment.
   */
  private Point2D.Double m_xPoint1;

  /**
   * Endpoint of the segment.
   */
  private Point2D.Double m_xPoint2;

  /**
   * Next segment in a sequence
   * where connecting segments follow
   * one another
   */
  private Segment m_xNextSegment;

  /**
   * Previous segment
   */
  private Segment m_xPreviousSegment;

  /**
   * Direction of the segment i.e. if
   * it starts with point1 or point2
   */
  private int m_iStarts;
  /**
   * Constructor
   *
   * @param in_xPoint1 StartingPoint
   * @param in_xPoint2 EndPoint
   */
  public Segment(Point2D.Double in_xPoint1,
                 Point2D.Double in_xPoint2)
  {
    m_xPoint1 = in_xPoint1;
    m_xPoint2 = in_xPoint2;
  }

  public String toString(){
    return "(" + m_xPoint2 + " -> " + m_xPoint2 +")";
  }



  /**
   * Length of the segment
   * @return
   */
  public double getLength()
  {
    return Math.sqrt(Math.pow(getXLength(),2) + Math.pow(getYLength(),2));
  }

  /**
   * Returns the first point of the segment.
   *
   * @return Starting point
   */
  public Point2D.Double getPoint1()
  {
    return m_xPoint1;
  }

  /**
   * Returns the second of the segment
   *
   * @return Endpoint
   */
  public Point2D.Double getPoint2()
  {
    return m_xPoint2;
  }

  /**
   * Returns the starting point of the segment.
   *
   * @return Starting point
   */
  public Point2D.Double getFirstPoint()
  {
    switch(m_iStarts)
    {
    case STARTS_SMALLER:
      return m_xPoint1;
    case STARTS_BIGGER:
      return m_xPoint2;
    default:
      throw new RuntimeException("No such case in switch.");
    }
  }

  /**
   * Returns the Endpoint of the segment
   *
   * @return Endpoint
   */
  public Point2D.Double getLastPoint()
  {
    switch(m_iStarts)
    {
    case STARTS_SMALLER:
      return m_xPoint2;
    case STARTS_BIGGER:
      return m_xPoint1;
    default:
      throw new RuntimeException("No such case in switch.");
    }
  }




  /**
   * Returns the distance between start and end
   * point in horizontal direction
   *
   * @return A distance
   */
  public double getXLength()
  {
    return m_xPoint2.x - m_xPoint1.x;
  }

  /**
   * Returns the distance between start and end
   * point in vertical direction
   *
   * @return A distance
   */
  public double getYLength()
  {
    return m_xPoint2.y - m_xPoint1.y;
  }

  /**
   * Set's the next segment in a sequential ordering of segments.
   *
   * The ordering has to be maintained by the Route.
   *
   * @param in_xSegment The next segment
   */
  public void setNextSegment(Segment in_xSegment)
  {
    m_xNextSegment = in_xSegment;
  }

  /**
   * Returns the next segment in a sequential ordering of segments
   *
   * @return Next segment or null, iff this is the last segment
   */
  public Segment getNextSegment()
  {
    return m_xNextSegment;
  }

  /**
   * Set's the previous segment in a sequential ordering of segments.
   *
   * The ordering has to be maintained by the Route.
   *
   * @param in_xSegment The previous segment
   */
  public void setPreviousSegment(Segment in_xSegment)
  {
    m_xPreviousSegment = in_xSegment;
  }

  /**
   * Returns the previous segment in a sequential ordering of segments
   *
   * @return Previous segment or null, iff this is the first segment
   */
  public Segment getPreviousSegment()
  {
    return m_xPreviousSegment;
  }

  /**
   * Sets the orientation of the segment
   *
   * @param in_iStarts STARTS_SMALLER or STARTS_BIGGER
   */
  public void setStarts(int in_iStarts)
  {
    m_iStarts = in_iStarts;
  }

  /**
   * Returns the orientation of the segment
   *
   * @return STARTS_SMALLER or STARTS_BIGGER
   */
  public int getStarts()
  {
    return m_iStarts;
  }

  /**
   * Returns a Sub-Segment of this segment
   *
   * @param in_dStart Start
   * @param in_dEnd End
   * @return
   */
  public Segment subSegment(double in_dStart,
                           double in_dEnd)
  {
    return new Segment(getPointOnSegment(in_dStart),
                       getPointOnSegment(in_dEnd));
  }

  /**
   * Returns a Point on this segment in absolute coordinates.
   *
   * @param in_dDistanceOnSegment
   * @return
   */
  public Point2D.Double getPointOnSegment(double in_dDistanceOnSegment)
  {
    // Calculate position on this segment
    double dX = 0;
    double dY = 0;
    switch(m_iStarts)
    {
    case STARTS_SMALLER:
      dX = m_xPoint1.x + in_dDistanceOnSegment * getXLength() / getLength();
      dY = m_xPoint1.y + in_dDistanceOnSegment * getYLength() / getLength();
      break;
    case STARTS_BIGGER:
      dX = m_xPoint2.x - in_dDistanceOnSegment * getXLength() / getLength();
      dY = m_xPoint2.y - in_dDistanceOnSegment * getYLength() / getLength();
      break;
    }

    return new Point2D.Double(dX, dY);
  }
}