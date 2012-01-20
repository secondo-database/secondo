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

import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.GeneralPath;
import java.awt.geom.Point2D;
import java.util.Vector;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.hoese.LEUtils;
import viewer.hoese.ProjectionManager;
import viewer.hoese.algebras.Dsplline;

/**
 * A route in a network
 *
 * @author Martin Scheppokat
 *
 */
public class Route
{

  /**
   * ID for this route
   */
  private int m_iId;

  /**
   * Length of the route
   */
  private double m_dLength;

  /**
   * All segments of this route
   */
  private Segment[] m_xSegments;

  /**
   * First segment of route
   */
  private Segment m_xFirstSegment;

  /**
   * Last segment of route
   */
  private Segment m_xLastSegment;

  /**
   * Flag indication wether this route has two distinct sides
   */
  private boolean m_bIsDualRoute;

  /**
   * Flag indication that this route starts at the smaller end
   */
  private boolean m_bStartSmaller;

  /**
   * Path to be returned by getRenderObject
   *
   */
  private GeneralPath m_xPath;

  /**
   * Constructor
   *
   * @param in_xList Secondos representation of a Route as a list.
   * @throws Exception If the list contains errors
   */
  public Route(ListExpr in_xList)
    throws Exception
  {

    Vector xSegments = new Vector();

    // Read values for the list
    m_iId =  in_xList.first().intValue();
    m_dLength = in_xList.second().realValue();
    ListExpr xLineList = in_xList.third();
    m_bIsDualRoute = in_xList.fourth().boolValue();
    m_bStartSmaller = in_xList.fifth().boolValue();

    System.out.println("Route " + m_iId);

    // Read segments


    if(xLineList.listLength()==2 && xLineList.second().atomType()==ListExpr.BOOL_ATOM){ // new style
       // startSmaller = xLineList.second().boolxLineList();
       // ignore startSmaller of the simple line because we have stored it also in the network
       xLineList = xLineList.first();
    }

    while (!xLineList.isEmpty()) {
      ListExpr xSegmentList = xLineList.first();
      if (xSegmentList.listLength() != 4) {
        throw new Exception("Error: No correct line expression: 4 elements needed");
      }

      double x1 = LEUtils.readNumeric(xSegmentList.first()).doubleValue();
      double y1 = LEUtils.readNumeric(xSegmentList.second()).doubleValue();
      double x2 = LEUtils.readNumeric(xSegmentList.third()).doubleValue();
      double y2 = LEUtils.readNumeric(xSegmentList.fourth()).doubleValue();

      Point2D.Double xPoint1 = new Point2D.Double(x1,y1);
      //boolean bSuccess = ProjectionManager.project(x1 ,y1 ,xPoint1);
      boolean bSuccess = true;

      Point2D.Double xPoint2 = new Point2D.Double(x2,y2);
      //bSuccess &= ProjectionManager.project(x2 ,y2 ,xPoint2);

      if(!bSuccess){
        throw new Exception("error in projection of segment (" +
            x1 + "," + y1 +
            ")->(" +
            x2 + "," + y2 + ")");
      }

      // Segment
      xSegments.add(new Segment(xPoint1,
                    xPoint2));

      // Look at next element in the list
      xLineList = xLineList.rest();
    }
    m_xSegments = (Segment[])xSegments.toArray(new Segment[0]);

    System.out.println(m_iId);
    maintainSegmentsOrdering();
    m_xPath = buildPath(m_xSegments);
  }


  /**
   * Constructor to copy only part of the route. Needed by
   * getPartOfRoute
   *
   * @param in_xOtherRoute
   * @param in_dStart
   * @param in_dEnd
   */
  public Route(Route in_xOtherRoute,
               double in_dStart,
               double in_dEnd)
  {
    if(in_dStart >= in_dEnd)
    {
      double dTemp = in_dStart;
      in_dStart = in_dEnd;
      in_dEnd = dTemp;
    }

    Vector xSegments = new Vector();

    m_iId =  in_xOtherRoute.getId();
    m_dLength = in_xOtherRoute.getLength();
    m_bIsDualRoute = in_xOtherRoute.isDualRoute();
    m_bStartSmaller = in_xOtherRoute.startsSmaller();

    double dCurrentPosition = 0;


    Segment xCurrentSegment = in_xOtherRoute.m_xFirstSegment;
    if (!m_bStartSmaller)
      xCurrentSegment = in_xOtherRoute.m_xLastSegment;
    while(xCurrentSegment != null)
    {
      // Get absolut start and end-position from current segment
      double dSegmentStart = dCurrentPosition;
      double dSegmentEnd = dSegmentStart + xCurrentSegment.getLength();


      // Segment lies before the choosen part
      if(dSegmentEnd < in_dStart)
      {
        // Calculate new position
        dCurrentPosition += xCurrentSegment.getLength();
        // Next segment
        if (m_bStartSmaller)
          xCurrentSegment = xCurrentSegment.getNextSegment();
        else
          xCurrentSegment = xCurrentSegment.getPreviousSegment();
        continue;
      }
      // Segment lies behind the choosen part
      if(dSegmentStart > in_dEnd)
      {
        // No more segments
        break;
      }

      // Calculate relative positions for the segment
      double dSegmentRelativeStart = Math.max(0,
                                              in_dStart - dCurrentPosition);
      double dSegmentRelativeEnd = Math.min(xCurrentSegment.getLength(),
                                            in_dEnd - dCurrentPosition);
      if (m_bStartSmaller)
        xSegments.add(xCurrentSegment.subSegment(dSegmentRelativeStart,
                                                  dSegmentRelativeEnd));
      else
        xSegments.add(xCurrentSegment.subSegment(xCurrentSegment.getLength()-
                                                  dSegmentRelativeStart,
                                                 xCurrentSegment.getLength()-
                                                  dSegmentRelativeEnd));
      // Calculate new position
      dCurrentPosition += xCurrentSegment.getLength();
      if (m_bStartSmaller)
        xCurrentSegment = xCurrentSegment.getNextSegment();
      else
        xCurrentSegment = xCurrentSegment.getPreviousSegment();
    }

    m_xSegments = (Segment[])xSegments.toArray(new Segment[0]);
    if(m_xSegments.length > 0)
    {
      maintainSegmentsOrdering();
    }
    m_xPath = buildPath(m_xSegments);
  }

  /**
   * Get a segment of the route
   * @param in_iIndex between 0 and <code>getSegmentCount()</code>
   *
   * @return A segment
   */
  private Segment getSegmentAt(int in_iIndex)
  {
    return m_xSegments[in_iIndex];
  }

  /**
   * Returns the number of segments this route has.
   */
  private int getSegmentCount()
  {
    return m_xSegments.length;
  }

  /**
   * Returns wether this route starts at its smaller end.
   *
   * @return
   */
  private boolean startsSmaller()
  {
    return m_bStartSmaller;
  }

  /**
   * Returns wether this route has two distinct sides.
   *
   * @return
   */
  private boolean isDualRoute()
  {
    return m_bIsDualRoute;
  }

  /**
   * Returns the absolute coordinates for a point somewhere on the route.
   *
   * @param in_dDistance From the start of the route
   *
   * @return A Point
   */
  public Point2D.Double getPointOnRoute(double in_dDistance)
  {

    // Look for segment
    Segment xSegment = m_xFirstSegment;
    if (!m_bStartSmaller) xSegment = m_xLastSegment;
    double dDistanceOnRoute = 0;
    while(xSegment != null)
    {
      dDistanceOnRoute += xSegment.getLength();
      if(dDistanceOnRoute >= (in_dDistance-0.0000001))
      {
        break;
      }
      if (m_bStartSmaller)
        xSegment = xSegment.getNextSegment();
      else
        xSegment = xSegment.getPreviousSegment();
    }

    // Calculate offset for this segment
    double dDistanceOnSegment = in_dDistance -
                                dDistanceOnRoute +
                                xSegment.getLength();


    Point2D.Double result;
    if (m_bStartSmaller)
      result = xSegment.getPointOnSegment(dDistanceOnSegment);
    else
      result = xSegment.getPointOnSegment(xSegment.getLength() -
                                        dDistanceOnSegment);
    return result;
  }

/**
 * Return the Id of the route.
 *
 * @return Id
 */
  public int getId()
  {
    return m_iId;
  }

  /**
   * Returns the length of the route
   *
   * @return Length
   */
  public double getLength()
  {
    return m_dLength;
  }

  /**
   * Returns a representation of the route to be displayed in the hoese-viewer.
   *
   * @return A Path following all segments
   */
  public Shape getRenderObject()
  {
    return m_xPath;
  }

  /**
   * Returns a part of the route. Needed by <code>RouteInterval</code>
   * @param in_dStart Start of the route
   * @param in_dEnd End of the route.
   * @return A possibly shorter route.
   */
  public Route getPartOfRoute(double in_dStart,
                              double in_dEnd)
  {
    return new Route(this,
                     in_dStart,
                     in_dEnd);
  }

  /**
   * Maintain a linear ordering of the segments
   *
   */
  private void maintainSegmentsOrdering()
  {
    // Start at any segment
    Segment xStartSegment = m_xSegments[0];

    // Follow from the end of this segment until
    // one end of the route is reached. We don't
    // know yet in which direction we are traveling.
    Segment xCurrentSegment = xStartSegment;
    xCurrentSegment.setStarts(Segment.STARTS_SMALLER);
    m_xLastSegment = xCurrentSegment;
    while(xCurrentSegment != null)
    {
      for (int i = 0; i < m_xSegments.length; i++)
      {
        Segment xOtherSegment = m_xSegments[i];
        // Don't compare a segment to itself and
        // don't build circles by using the start again
        if(xOtherSegment == xCurrentSegment ||
            xOtherSegment.getPreviousSegment() != null ||
            xOtherSegment.getNextSegment() != null)
        {
          continue;
        }

        if(xCurrentSegment.getLastPoint().x == xOtherSegment.getPoint1().x &&
           xCurrentSegment.getLastPoint().y == xOtherSegment.getPoint1().y)
        {
          xCurrentSegment.setNextSegment(xOtherSegment);
          xOtherSegment.setPreviousSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_SMALLER);
          m_xLastSegment = xOtherSegment;
          break;
        }
        if(xCurrentSegment.getLastPoint().x == xOtherSegment.getPoint2().x &&
           xCurrentSegment.getLastPoint().y == xOtherSegment.getPoint2().y)
        {
          xCurrentSegment.setNextSegment(xOtherSegment);
          xOtherSegment.setPreviousSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_BIGGER);
          m_xLastSegment = xOtherSegment;
          break;
        }
      }
      // Update last segment and current segment
      xCurrentSegment = xCurrentSegment.getNextSegment();
    }


    // Follow from the other end of the start segment
    // until we find the other end of the route
    xCurrentSegment = xStartSegment;
    m_xFirstSegment = xCurrentSegment;
    while(xCurrentSegment != null)
    {
      for (int i = 0; i < m_xSegments.length; i++)
      {
        Segment xOtherSegment = m_xSegments[i];
        // Don't compare a segment to itself and
        // don't build circles by using the start again
        if(xOtherSegment == xCurrentSegment ||
            xOtherSegment.getPreviousSegment() != null ||
            xOtherSegment.getNextSegment() != null)
        {
          continue;
        }

        if(xCurrentSegment.getFirstPoint().x == xOtherSegment.getPoint2().x &&
            xCurrentSegment.getFirstPoint().y == xOtherSegment.getPoint2().y)
        {
          xCurrentSegment.setPreviousSegment(xOtherSegment);
          xOtherSegment.setNextSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_SMALLER);
          m_xFirstSegment = xOtherSegment;
          break;
        }
        if(xCurrentSegment.getFirstPoint().x == xOtherSegment.getPoint1().x &&
           xCurrentSegment.getFirstPoint().y == xOtherSegment.getPoint1().y)
        {
          xCurrentSegment.setPreviousSegment(xOtherSegment);
          xOtherSegment.setNextSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_BIGGER);
          m_xFirstSegment = xOtherSegment;
          break;
        }
      }
      // Update last segment and current segment
      xCurrentSegment = xCurrentSegment.getPreviousSegment();
    }

    // Maybe we mixed-up start and end of the route. This can be
    // corrected in O(n) time
    if(m_xFirstSegment.getFirstPoint().x > m_xLastSegment.getLastPoint().x ||
       (
        m_xFirstSegment.getFirstPoint().x == m_xLastSegment.getLastPoint().x &&
        m_xFirstSegment.getFirstPoint().y >= m_xLastSegment.getLastPoint().y
       )
      )
    {
      int j = 0;
      for (int i = 0; i < m_xSegments.length; i++)
      {
        // Exchange next and previous segment
        Segment xNextSegment = m_xSegments[i].getNextSegment();
        Segment xPreviousSegment = m_xSegments[i].getPreviousSegment();
        m_xSegments[i].setNextSegment(xPreviousSegment);
        m_xSegments[i].setPreviousSegment(xNextSegment);
        if(m_xSegments[i].getStarts() == Segment.STARTS_SMALLER)
        {
          m_xSegments[i].setStarts(Segment.STARTS_BIGGER);
        }
        else
        {
          m_xSegments[i].setStarts(Segment.STARTS_SMALLER);
        }
      }
      Segment xFirstSegment = m_xFirstSegment;
      m_xFirstSegment = m_xLastSegment;
      m_xLastSegment = xFirstSegment;
    }
  }

  /**
   * Method building a path for the method getRenderObject.
   *
   * @param in_xSegments
   * @return
   */
  private static GeneralPath buildPath(Segment[] in_xSegments)
  {
    GeneralPath xPath = new GeneralPath();

    Point2D.Double p1 = new Point2D.Double(0,0);
    Point2D.Double p2 = new Point2D.Double(0,0);

    for (int i = 0; i < in_xSegments.length; i++)
    {
      Segment xCurrentSegment = in_xSegments[i];

      // Draw segment
      if(ProjectionManager.project(xCurrentSegment.getPoint1().x,
                                   xCurrentSegment.getPoint1().y, p1)){

        if(ProjectionManager.project(xCurrentSegment.getPoint2().x,
                                   xCurrentSegment.getPoint2().y, p2)){
          xPath.moveTo((float) p1.x, (float)p1.y);
          xPath.lineTo((float) p2.x, (float)p2.y);
        } else {
          System.err.println("problem in projection of p2");
        }
      } else {
         System.err.print("Problem in projection of p1");
      }
    }
    return xPath;
  }

}
