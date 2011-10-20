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
   * All segments of this section
   */
  private JSegment[] segments;

  /**
   * First segment of the section
   */
  private JSegment firstSegment;

  /**
   * Last segment of the section
   */
  private JSegment lastSegment;

  /**
   * Flag indicates if the section starts at the smaller (true) end or not
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
   * @param in_xList Secondos representation of a section as a list.
   * @throws Exception If the list contains errors
   */
  public JSection(ListExpr list)
    throws Exception
  {
    Vector segmentsv = new Vector();

    // Read segments
    ListExpr lineList = list.second();
    starts = lineList.second().boolValue();
    ListExpr segmentsList = lineList.first();

    while (!segmentsList.isEmpty()) {
      ListExpr segmentList = segmentsList.first();
      if (segmentList.listLength() != 4) {
        throw
          new Exception("Error: No correct line expression: 4 elements needed");
      }

      double x1 = LEUtils.readNumeric(segmentList.first()).doubleValue();
      double y1 = LEUtils.readNumeric(segmentList.second()).doubleValue();
      double x2 = LEUtils.readNumeric(segmentList.third()).doubleValue();
      double y2 = LEUtils.readNumeric(segmentList.fourth()).doubleValue();

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
      segmentsv.add(new JSegment(xPoint1, xPoint2));
      segmentsList = segmentsList.rest();
    }
    segments = (JSegment[])segmentsv.toArray(new JSegment[0]);

    maintainSegmentsOrdering();
  }

  /**
   * Get a segment of the section
   *
   * @param in_iIndex between 0 and <code>getSegmentCount()</code>
   *
   * @return A segment
   */
  public JSegment getSegmentAt(int index)
  {
    return segments[index];
  }

  /**
   * Returns the number of segments this section has.
   */
  public int getSegmentCount()
  {
    return segments.length;
  }

  /**
   * Returns if this section starts at its smaller end.
   *
   * @return
   */
  private boolean startsSmaller()
  {
    return starts;
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

        if(currSegment.getEndPoint().x == otherSegment.getPoint1().x &&
           currSegment.getEndPoint().y == otherSegment.getPoint1().y)
        {
          currSegment.setNextSegment(otherSegment);
          otherSegment.setPreviousSegment(currSegment);
          otherSegment.setStarts(true);
          lastSegment = otherSegment;
          break;
        }
        if(currSegment.getEndPoint().x == otherSegment.getPoint2().x &&
           currSegment.getEndPoint().y == otherSegment.getPoint2().y)
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

        if(currSegment.getStartPoint().x == otherSegment.getPoint2().x &&
           currSegment.getStartPoint().y == otherSegment.getPoint2().y)
        {
          currSegment.setPreviousSegment(otherSegment);
          otherSegment.setNextSegment(currSegment);
          otherSegment.setStarts(true);
          firstSegment = otherSegment;
          break;
        }
        if( currSegment.getStartPoint().x == otherSegment.getPoint1().x &&
            currSegment.getStartPoint().y == otherSegment.getPoint1().y)
        {
          currSegment.setPreviousSegment(otherSegment);
          otherSegment.setNextSegment(currSegment);
          otherSegment.setStarts(false);
          firstSegment = otherSegment;
          break;
        }
      }
      // Update last segment and current segment
      currSegment = currSegment.getPreviousSegment();
    }

    // Maybe we mixed-up start and end of the route. This can be
    // corrected in O(n) time
    if(firstSegment.getStartPoint().x > lastSegment.getEndPoint().x ||
       (
        firstSegment.getStartPoint().x == lastSegment.getEndPoint().x &&
        firstSegment.getStartPoint().y >= lastSegment.getEndPoint().y
       )
      )
    {
      int j = 0;
      for (int i = 0; i < segments.length; i++)
      {
        // Exchange next and previous segment
        JSegment nextSegment = segments[i].getNextSegment();
        JSegment previousSegment = segments[i].getPreviousSegment();
        segments[i].setNextSegment(previousSegment);
        segments[i].setPreviousSegment(nextSegment);
        segments[i].setStarts(!segments[i].getStarts());
      }
      JSegment fSegment = firstSegment;
      firstSegment = lastSegment;
      lastSegment = fSegment;
    }
  }
}