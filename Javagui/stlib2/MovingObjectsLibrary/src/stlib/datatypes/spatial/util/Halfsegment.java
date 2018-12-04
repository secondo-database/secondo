//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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

package stlib.datatypes.spatial.util;

import stlib.datatypes.spatial.Point;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.HalfsegmentIF;

/**
 * Class to represent 'Halfsegment' objects
 * 
 * @author Markus Fuessel
 *
 */
public class Halfsegment extends Segment implements HalfsegmentIF {

   /**
    * Flag to indicate if the left point of this segment is the dominating point
    * (true) or the right point (false)
    */
   private final boolean leftDominating;

   /**
    * Constructor for a empty and undefined 'Halfsegment' object.<br>
    * Should not normally be used
    */
   public Halfsegment() {
      super();
      this.leftDominating = false;
   }

   /**
    * Constructor to create a 'Halfsegment' object by passing the endpoints and a
    * flag which indicate the dominating point of this segment<br>
    * The passed points will be assigned to the internal leftPoint and rightPoint
    * by the following rule:<br>
    * leftPoint = min(p0, p1)<br>
    * rightPoint = max(p0, p1)
    * 
    * @param p0
    *           - first endpoint
    * @param p1
    *           - second endpoint
    */
   public Halfsegment(final PointIF p0, final PointIF p1, final boolean leftDominating) {
      super(p0, p1);

      this.leftDominating = leftDominating;

      setDefined(p0.isDefined() && p1.isDefined());
   }

   /**
    * Constructor to create a 'Halfsegment' object by passing coordinats of the
    * endpoints and a flag which indicate the dominating point<br>
    * 
    * 
    * @param x0
    *           - x value of first endpoint
    * @param y0
    *           - y value of first endpoint
    * @param x1
    *           - x value of second endpoint
    * @param y1
    *           - x value of second endpoint
    * @param leftDominating
    *           - true, left point is dominating - false, right point is dominating
    */
   public Halfsegment(final double x0, final double y0, final double x1, final double y1,
                      final boolean leftDominating) {
      this(new Point(x0, y0), new Point(x1, y1), leftDominating);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.HalfsegmentIF#getDominatingPoint()
    */
   @Override
   public PointIF getDominatingPoint() {
      if (this.leftDominating) {
         return getLeftPoint();
      } else {
         return getRightPoint();
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.HalfsegmentIF#getNonDominatingPoint()
    */
   @Override
   public PointIF getNonDominatingPoint() {
      if (this.leftDominating) {
         return getRightPoint();
      } else {
         return getLeftPoint();
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.HalfsegmentIF#isLeftDominating()
    */
   @Override
   public boolean isLeftDominating() {
      return leftDominating;
   }

   /**
    * Compare this 'Halfsegment' object to other ones<br>
    * <br>
    * Ensures the total order of 'Halfsegment' objects<br>
    * The rule is specified in the paper of the ROSE Algebra paper:<br>
    * dominating points > left dominating point flages > directions (rotations)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(HalfsegmentIF otherHS) {

      PointIF thisDP = getDominatingPoint();
      PointIF otherDP = otherHS.getDominatingPoint();

      int compareDP = thisDP.compareTo(otherDP);

      if (compareDP < 0) {
         return -1;

      } else if (compareDP > 0) {
         return 1;

      } else {

         if (!this.isLeftDominating() && otherHS.isLeftDominating()) {
            return -1;

         } else if (this.isLeftDominating() && !otherHS.isLeftDominating()) {
            return 1;

         } else {

            PointIF thisNDP = getNonDominatingPoint();
            PointIF otherNDP = otherHS.getNonDominatingPoint();

            double thisGradient = (thisDP.getYValue() - thisNDP.getYValue())
                  / (thisDP.getXValue() - thisNDP.getXValue());

            double otherGradient = (otherDP.getYValue() - otherNDP.getYValue())
                  / (otherDP.getXValue() - otherNDP.getXValue());

            if (thisGradient < otherGradient) {
               if (this.isVertical()) {
                  return 1;
               } else {
                  return -1;
               }

            } else if (thisGradient > otherGradient) {
               if (otherHS.isVertical()) {
                  return -1;
               } else {
                  return 1;
               }

            } else {

               return thisNDP.compareTo(otherNDP);
            }
         }
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      final int prime = 31;
      int result = super.hashCode();
      result = prime * result + (leftDominating ? 1231 : 1237);
      return result;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }

      if (!super.equals(obj)) {
         return false;
      }

      if (!(obj instanceof HalfsegmentIF)) {
         return false;
      }

      HalfsegmentIF otherHS = (HalfsegmentIF) obj;

      return leftDominating == otherHS.isLeftDominating();
   }

}
