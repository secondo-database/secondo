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
package mol.datatypes.unit.spatial.util;

import java.util.ArrayList;
import java.util.List;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.util.Cycle;
import mol.datatypes.spatial.util.Face;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;

/**
 * This class represents 'MovableFace' objects, which are generally used to
 * build 'UnitRegion' objects.<br>
 * This objects can have moving holes.
 * 
 * @author Markus Fuessel
 */
public class MovableFace {

   /**
    * Outer moving boundary of this 'MovableFace'
    */
   private final MovableCycle movingBoundary;

   /**
    * List with moving holes this 'MovableFace' may contain
    */
   private final List<MovableCycle> movingHoles;

   /**
    * Constructor for a simple 'MovableFace' object without holes.
    * 
    * @param movingBoundary
    *           - outer boundary of this 'MovableFace', a 'MovableCycle' object
    */
   public MovableFace(final MovableCycle movingBoundary) {
      this.movingBoundary = movingBoundary;

      movingHoles = new ArrayList<>();

   }

   /**
    * Constructor for a 'MovableFace', possible with holes.<br>
    * The holes have to be inside the outer boundary for the complete movement.<br>
    * The holes must not intersect.
    * 
    * @param movingBoundary
    *           - outer boundary of this 'MovableFace', a 'MovableCycle' object
    * 
    * @param movingHoles
    *           - List of movingHoles, 'MovableCycle' objects
    */
   public MovableFace(final MovableCycle movingBoundary, final List<MovableCycle> movingHoles) {
      this(movingBoundary);

      for (MovableCycle movingHole : movingHoles) {
         add(movingHole);
      }

   }

   /**
    * Add a moving hole to this 'MovableFace'
    * 
    * @param movingHole
    *           - the moving hole to add, a 'MovableCycle' object
    * 
    * @return true if adding was successful, false otherwise
    */
   public boolean add(final MovableCycle movingHole) {

      return movingHoles.add(movingHole);
   }

   /**
    * This method returns a 'Face' which is valid at the passed time instant within
    * the period of the movement of this 'MovableFace'
    * 
    * @param movementPeriod
    *           - the period in which the movement of this face is defined
    * @param instant
    *           - time instant
    * 
    * @return a defined 'Face' which is valid at the passed instant, otherwise the
    *         returned 'Face' is undefined
    */
   public Face getValue(final Period movementPeriod, final TimeInstant instant) {

      Face face = new Face(movingBoundary.getValue(movementPeriod, instant));

      if (face.isDefined()) {
         for (MovableCycle movingHole : movingHoles) {
            Cycle currentHole = movingHole.getValue(movementPeriod, instant);

            face.add(currentHole);
         }
      }

      return face;
   }

   /**
    * Get the initial 'Face' object at the beginning of the movement
    * 
    * @return the initial 'Face' object
    */
   public Face getInitial() {
      Face face = new Face(movingBoundary.getInitial());

      if (face.isDefined()) {
         for (MovableCycle movingHole : movingHoles) {
            face.add(movingHole.getInitial());
         }
      }
      return face;
   }

   /**
    * Get the final 'Face' object at the end of the movement
    * 
    * @return the final 'Face' object
    */
   public Face getFinal() {
      Face face = new Face(movingBoundary.getFinal());

      if (face.isDefined()) {
         for (MovableCycle movingHole : movingHoles) {
            face.add(movingHole.getFinal());
         }
      }
      return face;
   }

   /**
    * Get all 'MovableSegment' objects of boundary and holes
    * 
    * @return list of 'MovableSegment' objects of boundary and holes
    */
   public List<MovableSegment> getMovingSegments() {
      List<MovableSegment> movingsegments = new ArrayList<>();

      movingsegments.addAll(movingBoundary.getMovingSegments());

      for (MovableCycle mhole : movingHoles) {
         movingsegments.addAll(mhole.getMovingSegments());
      }

      return movingsegments;
   }

   /**
    * Get the projection bounding box of this 'MovableFace' object
    * 
    * @return a 'Rectangle' object
    */
   public Rectangle getProjectionBoundingBox() {

      return movingBoundary.getProjectionBoundingBox();
   }

   /**
    * Get the number of moving holes in this 'MovableFace' object
    * 
    * @return number of moving holes
    */
   public int getNoMovingHoles() {
      return movingHoles.size();
   }
}
