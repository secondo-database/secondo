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
package stlib.datatypes.unit.spatial.util;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.spatial.util.Face;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.util.CycleIF;
import stlib.interfaces.spatial.util.FaceIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.spatial.util.MovableCycleIF;
import stlib.interfaces.unit.spatial.util.MovableFaceIF;
import stlib.interfaces.unit.spatial.util.MovableSegmentIF;

/**
 * This class represents 'MovableFace' objects, which are generally used to
 * build 'UnitRegion' objects.<br>
 * This objects can have moving holes.
 * 
 * @author Markus Fuessel
 */
public class MovableFace implements MovableFaceIF {

   /**
    * Outer moving boundary of this 'MovableFace'
    */
   private final MovableCycleIF movingBoundary;

   /**
    * List with moving holes this 'MovableFace' may contain
    */
   private final List<MovableCycleIF> movingHoles;

   /**
    * Constructor for a simple 'MovableFace' object without holes.
    * 
    * @param movingBoundary
    *           - outer boundary of this 'MovableFace', a 'MovableCycleIF' object
    */
   public MovableFace(final MovableCycleIF movingBoundary) {
      this.movingBoundary = movingBoundary;

      movingHoles = new ArrayList<>();

   }

   /**
    * Constructor for a 'MovableFace', possible with holes.<br>
    * The holes have to be inside the outer boundary for the complete movement.<br>
    * The holes must not intersect.
    * 
    * @param movingBoundary
    *           - outer boundary of this 'MovableFace', a 'MovableCycleIF' object
    * 
    * @param movingHoles
    *           - List of movingHoles, 'MovableCycleIF' objects
    */
   public MovableFace(final MovableCycleIF movingBoundary, final List<MovableCycleIF> movingHoles) {
      this(movingBoundary);

      for (MovableCycleIF movingHole : movingHoles) {
         add(movingHole);
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.spatial.util.MovableFaceIF#add(stlib.interfaces.unit.
    * spatial. util.MovableCycleIF)
    */
   @Override
   public boolean add(final MovableCycleIF movingHole) {

      return movingHoles.add(movingHole);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.spatial.util.MovableFaceIF#getValue(stlib.interfaces.
    * interval.PeriodIF, stlib.interfaces.time.TimeInstantIF)
    */
   @Override
   public FaceIF getValue(final PeriodIF movementPeriod, final TimeInstantIF instant) {

      FaceIF face = new Face(movingBoundary.getValue(movementPeriod, instant));

      if (face.isDefined()) {
         for (MovableCycleIF movingHole : movingHoles) {
            CycleIF currentHole = movingHole.getValue(movementPeriod, instant);

            face.add(currentHole);
         }
      }

      return face;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableFaceIF#getInitial()
    */
   @Override
   public FaceIF getInitial() {
      FaceIF face = new Face(movingBoundary.getInitial());

      if (face.isDefined()) {
         for (MovableCycleIF movingHole : movingHoles) {
            face.add(movingHole.getInitial());
         }
      }
      return face;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableFaceIF#getFinal()
    */
   @Override
   public FaceIF getFinal() {
      FaceIF face = new Face(movingBoundary.getFinal());

      if (face.isDefined()) {
         for (MovableCycleIF movingHole : movingHoles) {
            face.add(movingHole.getFinal());
         }
      }
      return face;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableFaceIF#getMovingSegments()
    */
   @Override
   public List<MovableSegmentIF> getMovingSegments() {
      List<MovableSegmentIF> movingsegments = new ArrayList<>();

      movingsegments.addAll(movingBoundary.getMovingSegments());

      for (MovableCycleIF mhole : movingHoles) {
         movingsegments.addAll(mhole.getMovingSegments());
      }

      return movingsegments;
   }

   /**
    * Get the projection bounding box of this 'MovableFace' object
    * 
    * @return a 'RectangleIF' object
    */
   public RectangleIF getProjectionBoundingBox() {

      return movingBoundary.getProjectionBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableFaceIF#getNoMovingHoles()
    */
   @Override
   public int getNoMovingHoles() {
      return movingHoles.size();
   }
}
