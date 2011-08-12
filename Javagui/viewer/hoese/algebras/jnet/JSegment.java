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

import java.awt.geom.Point2D;

/**
 * A linear part of a section in a network.
 *
 * @author Simone Jandt
 *
 */
public class JSegment
{

  /**
   * Starting point of the segment.
   */
  private Point2D.Double startPos;

  /**
   * Endpoint of the segment.
   */
  private Point2D.Double endPos;

  /**
   * Next segment in a sequence
   * where connecting segments follow
   * one another
   */
  private JSegment nextSegment;

  /**
   * Previous segment
   */
  private JSegment previousSegment;

  /**
   * Direction of the segment i.e. if
   * it starts with point1 or point2
   */
  private boolean starts;
  /**
   * Constructor
   *
   * @param point1 StartingPoint
   * @param point2 EndPoint
   */
  public JSegment(Point2D.Double point1,
                 Point2D.Double point2)
  {
    startPos = point1;
    endPos = point2;
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
    return startPos;
  }

  /**
   * Returns the second of the segment
   *
   * @return Endpoint
   */
  public Point2D.Double getPoint2()
  {
    return endPos;
  }

  /**
   * Returns the starting point of the segment.
   *
   * @return Starting point
   */
  public Point2D.Double getFirstPoint()
  {
    if (starts)
      return startPos;
    else
      return endPos;
  }

  /**
   * Returns the Endpoint of the segment
   *
   * @return Endpoint
   */
  public Point2D.Double getLastPoint()
  {
    if (starts) return endPos;
    else return startPos;
  }




  /**
   * Returns the distance between start and end
   * point in horizontal direction
   *
   * @return A distance
   */
  public double getXLength()
  {
    return endPos.x - startPos.x;
  }

  /**
   * Returns the distance between start and end
   * point in vertical direction
   *
   * @return A distance
   */
  public double getYLength()
  {
    return endPos.y - startPos.y;
  }

  /**
   * Set's the next segment in a sequential ordering of segments.
   *
   * The ordering has to be maintained by the Route.
   *
   * @param in_xSegment The next segment
   */
  public void setNextSegment(JSegment in_xSegment)
  {
    nextSegment = in_xSegment;
  }

  /**
   * Returns the next segment in a sequential ordering of segments
   *
   * @return Next segment or null, iff this is the last segment
   */
  public JSegment getNextSegment()
  {
    return nextSegment;
  }

  /**
   * Set's the previous segment in a sequential ordering of segments.
   *
   * The ordering has to be maintained by the Route.
   *
   * @param in_xSegment The previous segment
   */
  public void setPreviousSegment(JSegment in_xSegment)
  {
    previousSegment = in_xSegment;
  }

  /**
   * Returns the previous segment in a sequential ordering of segments
   *
   * @return Previous segment or null, iff this is the first segment
   */
  public JSegment getPreviousSegment()
  {
    return previousSegment;
  }

  /**
   * Sets the orientation of the segment
   *
   * @param in_iStarts STARTS_SMALLER or STARTS_BIGGER
   */
  public void setStarts(boolean in_iStarts)
  {
    starts = in_iStarts;
  }

  /**
   * Returns the orientation of the segment
   *
   * @return STARTS_SMALLER or STARTS_BIGGER
   */
  public boolean getStarts()
  {
    return starts;
  }

  /**
   * Returns a Sub-Segment of this segment
   *
   * @param in_dStart Start
   * @param in_dEnd End
   * @return
   */
  public JSegment subSegment(double start,
                           double end)
  {
    return new JSegment(getPointOnSegment(start),
                       getPointOnSegment(end));
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
    if(starts)
    {
      dX = startPos.x + in_dDistanceOnSegment * getXLength() / getLength();
      dY = startPos.y + in_dDistanceOnSegment * getYLength() / getLength();
    }
    else
    {
      dX = endPos.x - in_dDistanceOnSegment * getXLength() / getLength();
      dY = endPos.y - in_dDistanceOnSegment * getYLength() / getLength();
    }

    return new Point2D.Double(dX, dY);
  }
}