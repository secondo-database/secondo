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
 * A section in a jnetwork
 *
 * @author Simone Jandt
 *
 */
public class JSection
{

  /**
   * ID for this route
   */
  private int id;

  /**
   * Length of the section
   * The member length is not used at the moment.
   */
  private double length;

  /**
  * List of route intervals covered by this section.
  *
  */
  private JRouteInterval[] rints;

  /**
   * All segments of this section
   */
  private JSegment[] segments;

  /**
   * First segment of route
   */
  private JSegment firstSegment;

  /**
   * Last segment of route
   */
  private JSegment lastSegment;

  /**
   * Flag indication wether this section has two distinct sides
   */
  private boolean dual;

  /**
  * Flag indication wether the section starts at the smaller or bigger point
  */
  private boolean starts;

  /**
   * Path to be returned by getRenderObject
   *
   */
  private GeneralPath path;

  /**
   * Constructor
   *
   * @param in_xList Secondos representation of a Route as a list.
   * @throws Exception If the list contains errors
   */
  public JSection(ListExpr list)
    throws Exception
  {
    Vector segmentsv = new Vector();

    // Read values for the list
    id =  list.first().intValue();
    length = list.tenth().realValue();
    ListExpr rintsList = list.fifth();
    ListExpr lineList = list.second();
    ListExpr dualList = list.twelfth().first();
    if (dualList.toString() == "Both") dual = true;
    else dual = false;
    starts = lineList.second().boolValue();

    System.out.println("Section " + id);

    // Read RouteIntervals

    Vector rintsv = new Vector();
    ListExpr restList = rintsList;

    while (!restList.isEmpty())
    {
      ListExpr rintList = restList.first().second();
      rintsv.add(new JRouteInterval(rintList));
      restList = restList.rest();
    }

    rints = (JRouteInterval[])rintsv.toArray(new JRouteInterval[0]);

    // Read segments

    ListExpr secLineList = lineList.first();

    while (!secLineList.isEmpty()) {
      ListExpr segmentList = secLineList.first();
      if (segmentList.listLength() != 4) {
        throw new Exception("Error: No correct line expression: 4 elements needed");
      }

      double x1 = LEUtils.readNumeric(segmentList.first()).doubleValue();
      double y1 = LEUtils.readNumeric(segmentList.second()).doubleValue();
      double x2 = LEUtils.readNumeric(segmentList.third()).doubleValue();
      double y2 = LEUtils.readNumeric(segmentList.fourth()).doubleValue();

      System.out.println("(" + x1 + ", " + y1 + ") -> (" + x2 + ", " + y2 + ")");

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
      segmentsv.add(new JSegment(xPoint1, xPoint2));

      // Look at next element in the list
      secLineList = secLineList.rest();
    }
    segments = (JSegment[])segmentsv.toArray(new JSegment[0]);

    System.out.println(id);
    maintainSegmentsOrdering();
    path = buildPath(segments);
  }



  /**
   * Constructor to copy only part of the section. Needed by
   * getPartOfRoute
   *
   * @param in_xOtherRoute
   * @param in_dStart
   * @param in_dEnd
   */
  public JSection(JSection otherSect,
               double from,
               double to)
  {
    if(from >= to)
    {
      double dTemp = from;
      from = to;
      to = dTemp;
    }

    Vector segmentsv = new Vector();

    id =  otherSect.getId();
    length = otherSect.getLength();
    dual = otherSect.isDualSection();
    starts = otherSect.startsSmaller();

    double currPos = 0;

    JSegment currSegment = otherSect.firstSegment;
    while(currSegment != null)
    {
      // Get absolut start and end-position from current segment
      double dSegmentStart = currPos;
      double dSegmentEnd = dSegmentStart + currSegment.getLength();


      // Segment lies before the choosen part
      if(dSegmentEnd < from)
      {
        // Calculate new position
        currPos += currSegment.getLength();
        // Next segment
        currSegment = currSegment.getNextSegment();
        continue;
      }
      // Segment lies behind the choosen part
      if(dSegmentStart > to)
      {
        // No more segments
        break;
      }

      // Calculate relative positions for the segment
      double relativeStart = Math.max(0, from - currPos);
      double relativeEnd = Math.min(currSegment.getLength(), to - currPos);

      segmentsv.add(currSegment.subSegment(relativeStart,relativeEnd));

      // Calculate new position
      currPos += currSegment.getLength();
      currSegment = currSegment.getNextSegment();
    }

    segments = (JSegment[])segmentsv.toArray(new JSegment[0]);
    if(segments.length > 0)
    {
      maintainSegmentsOrdering();
    }
    path = buildPath(segments);
  }

  /**
   * Get a segment of the section
   * @param in_iIndex between 0 and <code>getSegmentCount()</code>
   *
   * @return A segment
   */
  private JSegment getSegmentAt(int index)
  {
    return segments[index];
  }

  /**
   * Returns the number of segments this route has.
   */
  private int getSegmentCount()
  {
    return segments.length;
  }

  /**
   * Returns wether this route has two distinct sides.
   *
   * @return
   */
  private boolean isDualSection()
  {
    return dual;
  }

  /**
   * Returns wether this section starts at its smaller end.
   *
   * @return
   */
  private boolean startsSmaller()
  {
    return starts;
  }

  /**
   * Returns the absolute coordinates for a point somewhere on the route.
   *
   * @param pos distance from the start of the route
   *
   * @return A Point
   */
  public Point2D.Double getPointOnRoute(double pos)
  {

    // Look for segment
    JSegment segm = firstSegment;
    double dDistanceOnRoute = 0;
    while(segm != null)
    {
      dDistanceOnRoute += segm.getLength();
      if(dDistanceOnRoute >= pos)
      {
        break;
      }
      segm = segm.getNextSegment();
    }

    // Calculate offset for this segment
    double dDistanceOnSegment = pos -
                                dDistanceOnRoute +
                                segm.getLength();

    return segm.getPointOnSegment(dDistanceOnSegment);
  }

/**
 * Return the Id of the section.
 *
 * @return Id
 */
  public int getId()
  {
    return id;
  }

  /**
   * Returns the length of the section
   *
   * @return Length
   */
  public double getLength()
  {
    return length;
  }

  /**
   * Returns a representation of the section to be displayed in the hoese-viewer.
   *
   * @return A Path following all segments
   */
  public Shape getRenderObject()
  {
    return path;
  }

  /**
   * Returns a part of the route. Needed by <code>RouteInterval</code>
   * @param from of the route
   * @param to End of the route.
   * @return A possibly shorter route.
   */
  public JSection getPartOfRoute(double from,
                              double to)
  {
    return new JSection(this,
                     from,
                     to);
  }

  /**
   * Maintain a linear ordering of the segments
   *
   */
  private void maintainSegmentsOrdering()
  {
    // Start at any segment
    JSegment stSegm  = segments[0];

    // Follow from the end of this segment until
    // one end of the route is reached. We don't
    // know yet in which direction we are traveling.
    JSegment currSegment = stSegm;
    currSegment.setStarts(true);
    lastSegment = currSegment;

    while(currSegment != null)
    {
      for (int i = 0; i < segments.length; i++)
      {
        JSegment otherSegment = segments[i];
        // Don't compare a segment to itself and
        // don't build circles by using the start again
        if( otherSegment == currSegment ||
            otherSegment.getPreviousSegment() != null ||
            otherSegment.getNextSegment() != null)
        {
          continue;
        }

        if(currSegment.getLastPoint().x == otherSegment.getPoint1().x &&
           currSegment.getLastPoint().y == otherSegment.getPoint1().y)
        {
          currSegment.setNextSegment(otherSegment);
          otherSegment.setPreviousSegment(currSegment);
          otherSegment.setStarts(true);
          lastSegment = otherSegment;
          break;
        }
        if(currSegment.getLastPoint().x == otherSegment.getPoint2().x &&
           currSegment.getLastPoint().y == otherSegment.getPoint2().y)
        {
          currSegment.setNextSegment(otherSegment);
          otherSegment.setPreviousSegment(currSegment);
          otherSegment.setStarts(false);
          lastSegment = otherSegment;
          break;
        }
      }
      // Update last segment and current segment
      currSegment = currSegment.getNextSegment();
    }


    // Follow from the other end of the start segment
    // until we find the other end of the route
    currSegment = stSegm;
    firstSegment = currSegment;
    while(currSegment != null)
    {
      for (int i = 0; i < segments.length; i++)
      {
        JSegment otherSegment = segments[i];
        // Don't compare a segment to itself and
        // don't build circles by using the start again
        if( otherSegment == currSegment ||
            otherSegment.getPreviousSegment() != null ||
            otherSegment.getNextSegment() != null)
        {
          continue;
        }

        if(currSegment.getFirstPoint().x == otherSegment.getPoint2().x &&
           currSegment.getFirstPoint().y == otherSegment.getPoint2().y)
        {
          currSegment.setPreviousSegment(otherSegment);
          otherSegment.setNextSegment(currSegment);
          otherSegment.setStarts(false);
          firstSegment = otherSegment;
          break;
        }
        if( currSegment.getFirstPoint().x == otherSegment.getPoint1().x &&
            currSegment.getFirstPoint().y == otherSegment.getPoint1().y)
        {
          currSegment.setPreviousSegment(otherSegment);
          otherSegment.setNextSegment(currSegment);
          otherSegment.setStarts(true);
          firstSegment = otherSegment;
          break;
        }
      }
      // Update last segment and current segment
      currSegment = currSegment.getPreviousSegment();
    }

    // Maybe we mixed-up start and end of the route. This can be
    // corrected in O(n) time
    if(firstSegment.getFirstPoint().x > lastSegment.getLastPoint().x ||
       (
        firstSegment.getFirstPoint().x == lastSegment.getLastPoint().x &&
        firstSegment.getFirstPoint().y >= lastSegment.getLastPoint().y
       )
      )
    {
      int j = 0;
      for (int i = 0; i < segments.length; i++)
      {
        // Exchange next and previous segment
        JSegment nextSegment = segments[i].getNextSegment();
        JSegment previousSegment = segments[i].getPreviousSegment();
        segments[i].setStarts(!segments[i].getStarts());
        segments[i].setNextSegment(previousSegment);
        segments[i].setPreviousSegment(nextSegment);
      }
      JSegment fSegment = firstSegment;
      firstSegment = lastSegment;
      lastSegment = fSegment;
    }
  }

  /**
   * Method building a path for the method getRenderObject.
   *
   * @param in_xSegments
   * @return
   */
  private static GeneralPath buildPath(JSegment[] inSegments)
  {
    GeneralPath xPath = new GeneralPath();

    for (int i = 0; i < inSegments.length; i++)
    {
      JSegment currSegment = inSegments[i];

      // Draw segment
      xPath.moveTo((float)currSegment.getPoint1().x,
                   (float)currSegment.getPoint1().y);
      xPath.lineTo((float)currSegment.getPoint2().x,
                   (float)currSegment.getPoint2().y);
    }

    return xPath;
  }

  /**
  * Returns the route interval of the section covering the given position
  *
  * @param r RouteLocation
  * @return JRouteInterval
  */
  public JRouteInterval getRouteInterval(JRouteLocation rloc)
  throws Exception
  {
    for (int i = 0; i < rints.length; i++)
    {
      JRouteInterval currRint = rints[i];
      if (currRint.contains(rloc))
        return currRint;
    }
    throw new Exception("No JRouteInterval for rloc found");
  }

  /**
  * Returns spatial position of routelocation
  *
  * @param pos distance of position from start of route
  * @param startPos distance of section start from start of route
  * @return spatial position
  */
  public Point2D.Double getPointOnSection(double pos, double startPos)
  {
    // Look for segment
    JSegment segm = firstSegment;
    double dDistanceOnRoute = startPos;
    while(segm != null)
    {
      dDistanceOnRoute += segm.getLength();
      if(dDistanceOnRoute >= pos)
      {
        break;
      }
      segm = segm.getNextSegment();
    }

    // Calculate offset for this segment
    double dDistanceOnSegment = pos -
                                dDistanceOnRoute +
                                segm.getLength();

    return segm.getPointOnSegment(dDistanceOnSegment);
  }

  /**
  * Tests the section if their rints cover the given position data of the
  * network.
  *
  * @param r RouteLocation
  * @return bool
  */
  public boolean contains(JRouteLocation rloc)
  {
    for (int i = 0; i < rints.length; i++)
    {
      JRouteInterval currRint = rints[i];
      if (currRint.contains(rloc)) return true;
    }
    return false;
  }

/**
  * Tests the section if it intersects the given position interval of
  * the network.
  *
  * @param r JRouteLocation
  * @return bool
  */
  public boolean intersects(JRouteInterval rint)
  {
    for (int i = 0; i < rints.length; i++)
    {
      JRouteInterval currRint = rints[i];
      if (currRint.intersects(rint)) return true;
    }
    return false;
  }
}
