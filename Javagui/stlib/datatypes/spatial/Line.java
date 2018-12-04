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

package stlib.datatypes.spatial;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.GeneralType;
import stlib.datatypes.spatial.util.Halfsegment;
import stlib.datatypes.spatial.util.Rectangle;
import stlib.interfaces.spatial.LineIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.HalfsegmentIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.spatial.util.SegmentIF;

/**
 * This class represents spatial objects of type 'Line'<br>
 * 
 * 
 * @author Markus Fuessel
 */
public class Line extends GeneralType implements LineIF {

   /**
    * List of the line halfsegments
    */
   private final List<HalfsegmentIF> halfsegments;

   /**
    * The minimum bounding box of this 'Line' object
    */
   private RectangleIF objectMBB;

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
   public Line(final List<PointIF> points) {
      this.halfsegments = new ArrayList<>(points.size() * 2);
      objectMBB = new Rectangle();

      if (!points.isEmpty()) {
         for (int i = 0; i < points.size() - 1; i++) {
            add(points.get(i), points.get(i + 1));
         }

         setDefined(true);
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.spatial.LineIF#add(stlib.interfaces.spatial.util.SegmentIF)
    */
   @Override
   public boolean add(final SegmentIF segment) {

      if (!segment.isDefined()) {
         return false;
      }

      return add(segment.getLeftPoint(), segment.getRightPoint());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.LineIF#add(stlib.interfaces.spatial.PointIF,
    * stlib.interfaces.spatial.PointIF)
    */
   @Override
   public boolean add(final PointIF p0, final PointIF p1) {
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

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.LineIF#add(stlib.interfaces.spatial.util.
    * HalfsegmentIF)
    */
   @Override
   public boolean add(final HalfsegmentIF halfsegment) {
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

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.LineIF#clear()
    */
   @Override
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
    * @see stlib.interfaces.features.Spatial#getBoundingBox()
    */
   @Override
   public RectangleIF getBoundingBox() {
      return objectMBB;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.LineIF#length(boolean)
    */
   @Override
   public double length(final boolean useSphericalGeometry) {
      double length = 0.0d;

      if (isDefined()) {
         for (HalfsegmentIF halfsegment : halfsegments) {
            if (halfsegment.isLeftDominating()) {
               length += halfsegment.length(useSphericalGeometry);
            }
         }
      }

      return length;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.LineIF#getHalfsegments()
    */
   @Override
   public List<HalfsegmentIF> getHalfsegments() {
      return new ArrayList<>(halfsegments);
   }

}
