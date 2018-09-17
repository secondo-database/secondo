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

package mol.datatypes.spatial;

import java.util.ArrayList;
import java.util.List;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Spatial;
import mol.datatypes.spatial.util.Halfsegment;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.spatial.util.Segment;

/**
 * This class represents spatial objects of type 'Line'<br>
 * 
 * 
 * @author Markus Fuessel
 */
public class Line extends GeneralType implements Spatial {

   /**
    * List of the line halfsegments
    */
   private final List<Halfsegment> halfsegments;

   /**
    * The minimum bounding box of this 'Line' object
    */
   private Rectangle objectMBB;

   /**
    * Constructor for an empty 'Line' object
    * 
    * @param defined
    *           - specifies whether the 'Line' object is to be set as defined
    */
   public Line(final boolean defined) {
      halfsegments = new ArrayList<>();
      objectMBB = new Rectangle();

      setDefined(defined);
   }

   /**
    * Constructor for a 'Line' object.<br>
    * The points are connected in the order of their occurrence in the list.
    * 
    * @param points
    */
   public Line(final List<Point> points) {
      this.halfsegments = new ArrayList<>(points.size() * 2);
      objectMBB = new Rectangle();

      if (!points.isEmpty()) {
         for (int i = 0; i < points.size() - 1; i++) {
            add(points.get(i), points.get(i + 1));
         }

         setDefined(true);
      }
   }

   /**
    * Add the passed 'Segment' object to this 'Line' object<br>
    * Adds internally two halfsegments to this 'Line' object<br>
    * Updates the bounding box
    * 
    * @param segment
    * @return true if the adding was successful, false otherwise
    */
   public boolean add(final Segment segment) {

      if (!segment.isDefined()) {
         return false;
      }

      return add(segment.getLeftPoint(), segment.getRightPoint());
   }

   /**
    * Add the passed 'Point' objects to this 'Line' object<br>
    * Adds internally two halfsegments to this 'Line' object<br>
    * Updates the bounding box
    * 
    * @param p0
    *           - first endpoint
    * @param p1
    *           - second endpoint
    * 
    * @return true if the adding was successful, false otherwise
    */
   public boolean add(final Point p0, final Point p1) {
      if (!p0.isDefined() || !p1.isDefined()) {
         return false;
      }

      Halfsegment hsLeft = new Halfsegment(p0, p1, true);
      Halfsegment hsRight = new Halfsegment(p0, p1, false);

      if (!add(hsLeft)) {
         return false;
      }

      return add(hsRight);

   }

   /**
    * Add the passed halfsegment to this 'Line' object.<br>
    * Updates the bounding box
    * 
    * @param halfsegment
    * @return true if halfsegment was added, false otherwise
    */
   public boolean add(final Halfsegment halfsegment) {
      if (!halfsegment.isDefined()) {
         return false;
      }

      if (halfsegments.add(halfsegment)) {
         objectMBB = objectMBB.merge(halfsegment.getBoundingBox());

         return true;

      } else {

         return false;
      }
   }

   /**
    * Remove all segments from this 'Line' object and reset the bounding box
    */
   public void clear() {
      halfsegments.clear();
      objectMBB = new Rectangle();
   }

   /**
    * Verify if this object is empty
    * 
    * @return true if this object contains no halfsegments
    */
   @Override
   public boolean isEmpty() {
      return (!isDefined() || halfsegments.isEmpty());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Spatial#getBoundingBox()
    */
   @Override
   public Rectangle getBoundingBox() {
      return objectMBB;
   }

   /**
    * Get the total length of this 'Line' object
    * 
    * @param useSphericalGeometry
    *           - if true spherical geometry is used to calculate the length in
    *           metres, otherwise euclidean distance is used
    * 
    * @return euclidean or geographical length, depends on parameter
    *         useSphericalGeometry
    */
   public double length(final boolean useSphericalGeometry) {
      double length = 0.0d;

      if (isDefined()) {
         for (Halfsegment halfsegment : halfsegments) {
            if (halfsegment.isLeftDominating()) {
               length += halfsegment.length(useSphericalGeometry);
            }
         }
      }

      return length;
   }

   /**
    * Get the halfsegments
    * 
    * @return the halfsegments
    */
   public List<Halfsegment> getHalfsegments() {
      return new ArrayList<>(halfsegments);
   }

}
