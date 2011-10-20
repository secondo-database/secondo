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

import viewer.hoese.algebras.jnet.JSegment;
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
public class JRoute
{

  /**
   * ID for this route
   */
  private int id;

  /**
  * Length of the route
  */
  private double lenth;

  /**
   * Route curve.
   */
  private JSegment[] segments;

  /**
  * First segment
  */
  private JSegment firstSegment;

  /**
  * Last segement
  */
  private JSegment lastSegment;

  /**
  * Flag indicating if route curve starts at smaller end
  */
  private boolean startsSmaller;

  /**
  * Path to be returned by getRenderObject
  */
  private GeneralPath path;

  /**
   * Constructor
   *
   * @param in_xList Secondos representation of a Route as a list.
   * @throws Exception If the list contains errors
   */
  public JRoute(ListExpr list, JSection[] sections)
    throws Exception
  {
    Vector rintsv = new Vector();
    //Read id
    id =  list.first().intValue();

    // Read length
    lenth = list.fourth().realValue();

    ListExpr rintsList = list.third();

    Vector segv = new Vector();

    ListExpr actRint = rintsList.first();
    rintsList = rintsList.rest();
    int secNo = actRint.first().intValue();
    JSection actSection = sections[secNo-1];
    JSegment actSegment = actSection.getSegmentAt(0);
    firstSegment = actSegment;
    if( 0 == actSection.getSegmentCount()-1)
      lastSegment = actSegment;
    else
      segv.add(actSegment);
    for (int i = 1 ; i < actSection.getSegmentCount(); i++)
    {
      actSegment = actSection.getSegmentAt(i);
      if( i == actSection.getSegmentCount()-1)
      {
        lastSegment = actSegment;
      }
      else
      {
        segv.add(actSegment);
      }
    }
    while (!rintsList.isEmpty())
    {
      actRint = rintsList.first();
      rintsList = rintsList.rest();
      secNo = actRint.first().intValue();
      actSection = sections[secNo-1];
      actSegment = actSection.getSegmentAt(0);
      lastSegment.setNextSegment(actSegment);
      actSegment.setPreviousSegment(lastSegment);
      segv.add(lastSegment);
      if( 0 == actSection.getSegmentCount()-1)
        lastSegment = actSegment;
      else
        segv.add(actSegment);
      for (int i = 1; i <  actSection.getSegmentCount(); i++)
      {
        actSegment = actSection.getSegmentAt(i);
        if( i == actSection.getSegmentCount()-1)
        {
          lastSegment = actSegment;
        }
        else
        {
          segv.add(actSegment);
        }
      }
    }
    segv.add(lastSegment);

    segments = (JSegment[]) segv.toArray(new JSegment[0]);

    if ((lastSegment.getEndPoint().x < firstSegment.getStartPoint().x)||
        (lastSegment.getEndPoint().x == firstSegment.getStartPoint().x &&
         lastSegment.getEndPoint().y < firstSegment.getStartPoint().y))
      startsSmaller = false;
    else
      startsSmaller = true;

    path = buildPath(segments);
  }

  /**
  * Constructor for route parts
  *
  * @param route complete route
  * @param from start position of route part
  * @param to end position of route part
  */
  public JRoute(JRoute route, double from, double to)
  {
    if (from > to)
    {
      double aux = from;
      from = to;
      to = aux;
    }

    Vector segv = new Vector();

    id =  route.getId();
    lenth = Math.abs(to - from);

    double actRoutePos = 0.0;

    JSegment actSegment = route.getFirstSegment();
    if (!route.getStartSmaller()) actSegment = route.getLastSegment();

    while (actSegment != null)
    {
      double segmentStart = actRoutePos;
      double segmentEnd = actRoutePos + actSegment.getLength();

      if (segmentEnd < from)
      {
        //Segment lies before part of searched interval
        actRoutePos += actSegment.getLength();
        if (route.getStartSmaller())
          actSegment = actSegment.getNextSegment();
        else
          actSegment = actSegment.getPreviousSegment();
        continue;
      }

      if (segmentStart > to) break; //segment lies behind searched interval

      // segment belongs to searched interval calculate relative positions
      // on segment.

      double relativeStartSegment = Math.max(0, from - actRoutePos);
      double relativeEndSegment = Math.min(actSegment.getLength(),
                                           to - actRoutePos);

      if (route.getStartSmaller())
        segv.add(actSegment.subSegment(relativeStartSegment,
                                       relativeEndSegment));
      else
        segv.add(actSegment.subSegment(actSegment.getLength() -
                                          relativeStartSegment,
                                       actSegment.getLength() -
                                           relativeEndSegment));

      // continue with next segment
      actRoutePos += actSegment.getLength();
      if (route.getStartSmaller())
        actSegment = actSegment.getNextSegment();
      else
        actSegment = actSegment.getPreviousSegment();
    }

    segments = (JSegment[]) segv.toArray(new JSegment[0]);

    firstSegment = segments[0];
    lastSegment = segments[segments.length - 1];

    if (firstSegment.getStartPoint().x > lastSegment.getEndPoint().x ||
        (firstSegment.getStartPoint().x == lastSegment.getEndPoint().x &&
          firstSegment.getStartPoint().y > lastSegment.getStartPoint().y))
      startsSmaller = false;
    else
      startsSmaller = true;

    path = buildPath(segments);
  }

  /**
   * Returns the number of segments of this route.
   * @return number of sections
   */
  private int getSegmentsCount()
  {
    return segments.length;
  }

  /**
  * Get segment of route
  * @param index position in segments array
  * @return segment at position
  */
  private JSegment getSegmentAt(int index)
  {
    return segments[index];
  }

  /**
   * Return the Id of the route.
   * @return Id of route
   */
  public int getId()
  {
    return id;
  }

  /**
   * get number of segments of the route
   * @return number of segments of the route.
   */
  private int getSegmentCount()
  {
    return segments.length;
  }

  /**
   * return true if the route starts at the smaller end point of the route
   * @return
   */
   private boolean getStartSmaller()
   {
     return startsSmaller;
   }

   /**
   * return first segment
   * @return
   */
   private JSegment getFirstSegment()
   {
     return firstSegment;
   }

   /**
   * return last segment
   * @return
   */
   private JSegment getLastSegment()
   {
     return lastSegment;
   }

   /**
   * compute coordinates for a single route position
   * @param pos distance from start of route
   * @return spatial point of position
   */
   public Point2D.Double getPointOnRoute(double pos)
   {

     JSegment actSegment = firstSegment;
     if(!startsSmaller) actSegment = lastSegment;

     double actRoutePosition = 0;
     while (actSegment != null)
     {
       actRoutePosition += actSegment.getLength();
       if (actRoutePosition >= pos) break;
       if (startsSmaller)
         actSegment = actSegment.getNextSegment();
       else
         actSegment = actSegment.getPreviousSegment();
     }

     double distOnSegment = pos - actRoutePosition + actSegment.getLength();
     if (startsSmaller)
       return actSegment.getPointOnSegment(distOnSegment);
     else
       return actSegment.getPointOnSegment(actSegment.getLength() -
                                           distOnSegment);
   }

   /**
   * computes a part of a route given by start and end of the interval
   * @param from
   * @param to
   * @return part of route as new route
   */
   public JRoute getPartOfRoute(double from, double to)
   {
     return new JRoute(this, from, to);
   }

   /**
   * computes a part of a route given by a route interval
   * @param route interval
   * @return part of route as new route
   */
   public JRoute getPartOfRoute(JRouteInterval rint)
   {
     return new JRoute(this, rint.getStartPosition(), rint.getEndPosition());
   }

   /**
   * length of route
   * @return length of route
   */
   public double getLength()
   {
     return lenth;
   }

   /**
   * route representation to be displayed in hoese-viewer
   * @return segmentspath
   */
   public Shape getRenderObject()
   {
     return path;
   }

  /**
   * Method building a path for the method getRenderObject.
   *
   * @param segments
   * @return
   */
  private static GeneralPath buildPath(JSegment[] insegments)
  {
    GeneralPath xPath = new GeneralPath();

    for (int i = 0; i < insegments.length; i++)
    {
      JSegment actSegment = insegments[i];

      // Draw segment
      xPath.moveTo((float)actSegment.getPoint1().x,
                   (float)actSegment.getPoint1().y);
      xPath.lineTo((float)actSegment.getPoint2().x,
                   (float)actSegment.getPoint2().y);
    }

    return xPath;
  }
}