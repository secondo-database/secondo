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
   * -------------------
   * The member length is not used at the moment. The length is calculated 
   * via the length of the segments
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
   * Flag indication wether this route has to distinct sides
   */
	private boolean m_bIsDualRoute;
	
  /**
   * Flag indication that this route starts at the smaller end
   */
	private boolean m_bStartSmaller;
	
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

    // Read segments
		while (!xLineList.isEmpty()) {
			ListExpr xSegmentList = xLineList.first();
			if (xSegmentList.listLength() != 4) {
				throw new Exception("Error: No correct line expression: 4 elements needed");
			}

			double x1 = LEUtils.readNumeric(xSegmentList.first()).doubleValue();
			double y1 = LEUtils.readNumeric(xSegmentList.second()).doubleValue();
			double x2 = LEUtils.readNumeric(xSegmentList.third()).doubleValue();
			double y2 = LEUtils.readNumeric(xSegmentList.fourth()).doubleValue();

			Point2D.Double xPoint1 = new Point2D.Double();
			boolean bSuccess = ProjectionManager.project(x1 ,y1 ,xPoint1);

			Point2D.Double xPoint2 = new Point2D.Double();
			bSuccess &= ProjectionManager.project(x2 ,y2 ,xPoint2);

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
    
    maintainSegmentsOrdering();
	}
	

  /**
   * Constructor to copy only part of the route. Needed by 
   * <code>getPartOfRoute</code>
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
//      throw new RuntimeException("End > Start");
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
    
    for (int i = 0; i < in_xOtherRoute.getSegmentCount(); i++) 
    {
      Segment xCurrentSegment = in_xOtherRoute.getSegmentAt(i);
      
      // Get absolut start and end-position from current segment
      double dSegmentStart = dCurrentPosition;
      double dSegmentEnd = dSegmentStart + xCurrentSegment.getLength();
      
      
      // Segment lies before the choosen part
      if(dSegmentEnd < in_dStart)
      {
        // Calculate new position
        dCurrentPosition += xCurrentSegment.getLength();
        // Next segment
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

      xSegments.add(xCurrentSegment.subSegment(dSegmentRelativeStart,
                                               dSegmentRelativeEnd));
      
      // Calculate new position
      dCurrentPosition += xCurrentSegment.getLength();
    }
    m_xSegments = (Segment[])xSegments.toArray(new Segment[0]);
    maintainSegmentsOrdering();
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
		double dDistanceOnRoute = 0;
		while(xSegment != null) 
    {
			dDistanceOnRoute += xSegment.getLength();
			if(dDistanceOnRoute > in_dDistance)
      {
				break;
			}
      xSegment = xSegment.getNextSegment();
		}

		// Calculate offset for this segment
		double dDistanceOnSegment = in_dDistance - 
                                  dDistanceOnRoute + 
                                  xSegment.getLength();
		
		return xSegment.getPointOnSegment(dDistanceOnSegment);
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
    
	  GeneralPath xPath = new GeneralPath();
    
    for (int i = 0; i < m_xSegments.length; i++) 
    {
      Segment xCurrentSegment = m_xSegments[i];

      // Draw segment 
      xPath.moveTo((float)xCurrentSegment.getPoint1().x, 
                   (float)xCurrentSegment.getPoint1().y);
      xPath.lineTo((float)xCurrentSegment.getPoint2().x, 
                   (float)xCurrentSegment.getPoint2().y);
    }

    return xPath;
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
    int iSegmentCount = 0;
    while(xCurrentSegment != null) 
    {
      iSegmentCount++;
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
        Point2D.Double xCurrentEndPoint;
        if(xCurrentSegment.getStarts() == Segment.STARTS_SMALLER)
        {
          xCurrentEndPoint = xCurrentSegment.getPoint2();
        }
        else
        {
          xCurrentEndPoint = xCurrentSegment.getPoint1();
        }
        
        if(xCurrentEndPoint.x == xOtherSegment.getPoint1().x &&
            xCurrentEndPoint.y == xOtherSegment.getPoint1().y)
        {
          xCurrentSegment.setNextSegment(xOtherSegment);
          xOtherSegment.setPreviousSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_SMALLER);
        }
        if(xCurrentEndPoint.x == xOtherSegment.getPoint2().x &&
            xCurrentEndPoint.y == xOtherSegment.getPoint2().y)
        {
          xCurrentSegment.setNextSegment(xOtherSegment);
          xOtherSegment.setPreviousSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_BIGGER);
        }
      }
      // Update last segment and current segment
      m_xLastSegment = xCurrentSegment;
      xCurrentSegment = xCurrentSegment.getNextSegment();
    }
  

    // Follow from the other end of the start segment 
    // until we find the other end of the route
    xCurrentSegment = xStartSegment;
    iSegmentCount = 0;
    while(xCurrentSegment != null) 
    {
      iSegmentCount++;
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

        Point2D.Double xCurrentStartPoint;
        if(xCurrentSegment.getStarts() == Segment.STARTS_SMALLER)
        {
          xCurrentStartPoint = xCurrentSegment.getPoint1();
        }
        else
        {
          xCurrentStartPoint = xCurrentSegment.getPoint2();
        }
        
        if(xCurrentStartPoint.x == xOtherSegment.getPoint2().x &&
            xCurrentStartPoint.y == xOtherSegment.getPoint2().y)
        {
          xCurrentSegment.setPreviousSegment(xOtherSegment);
          xOtherSegment.setNextSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_SMALLER);
        }
        if(xCurrentStartPoint.x == xOtherSegment.getPoint1().x &&
            xCurrentStartPoint.y == xOtherSegment.getPoint1().y)
        {
          xCurrentSegment.setPreviousSegment(xOtherSegment);
          xOtherSegment.setNextSegment(xCurrentSegment);
          xOtherSegment.setStarts(Segment.STARTS_BIGGER);
        }
      }
      // Update last segment and current segment
      m_xFirstSegment = xCurrentSegment;
      xCurrentSegment = xCurrentSegment.getPreviousSegment();
    }
  
    // Maybe we mixed-up start and end of the route. This can be 
    // corrected in O(n) time
    // TODO: Change ordering!
    
    xCurrentSegment = m_xFirstSegment;
    while(xCurrentSegment != null)
    {
//      if(xCurrentSegment.getStarts() == Segment.STARTS_SMALLER)
//      {
//        System.out.println("Segment (" + 
//                           xCurrentSegment.getPoint1().x + ", " +
//                           xCurrentSegment.getPoint1().y + ") - (" +
//                           xCurrentSegment.getPoint2().x + ", " +
//                           xCurrentSegment.getPoint2().y + ")");
//      }
//      else
//      {
//        System.out.println("Segment (" + 
//                           xCurrentSegment.getPoint2().x + ", " +
//                           xCurrentSegment.getPoint2().y + ") - (" +
//                           xCurrentSegment.getPoint1().x + ", " +
//                           xCurrentSegment.getPoint1().y + ")");        
//      }
      
      xCurrentSegment = xCurrentSegment.getNextSegment();
    } 
  }  
}
