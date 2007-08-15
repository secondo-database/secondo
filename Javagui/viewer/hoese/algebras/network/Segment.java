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
   * Starting point of the segment.
   */
	private Point2D.Double m_xPoint1;
	
  /**
   * Endpoint of the segment.
   */
	private Point2D.Double m_xPoint2;
	
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

	/**
   * Length of the segment 
   * @return
	 */
  public double getLength() 
  {
		return Math.sqrt(Math.pow(getXLength(),2) + Math.pow(getYLength(),2));
	}

  /**
   * Returns the starting point of the segment.
   * 
   * @return Starting point
   */
	public Point2D.Double getPoint1() 
  {
		return m_xPoint1;
	}

  /**
   * Returns the Endpoint of the segment
   * 
   * @return Endpoint
   */
  public Point2D.Double getPoint2() 
  {
    return m_xPoint2;
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
    double dX = m_xPoint1.x + in_dDistanceOnSegment * getXLength() / getLength();
    double dY = m_xPoint1.y + in_dDistanceOnSegment * getYLength() / getLength();
    
    
    return new Point2D.Double(dX, dY);
  }
}
