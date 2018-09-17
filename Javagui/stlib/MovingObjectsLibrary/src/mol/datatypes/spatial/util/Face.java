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

package mol.datatypes.spatial.util;

import java.util.ArrayList;
import java.util.List;

import mol.datatypes.features.Spatial;
import mol.datatypes.spatial.Point;

/**
 * This utility class represents spatial objects of type 'Face'.<br>
 * A 'Face' is a polygon which possibly containing some polygonal holes.
 * 
 * @author Markus Fuessel
 */
public class Face implements Spatial {

   /**
    * The defined flag, indicates if a data type object is defined
    */
   private boolean defined;

   /**
    * Outer boundary of this 'Face'
    */
   private final Cycle boundary;

   /**
    * List with holes this 'Face' may contain
    */
   private final List<Cycle> holes;

   /**
    * Constructor for a simple 'Face' object without holes.
    * 
    * @param boundary
    *           - outer boundary of this 'Face'
    */
   public Face(final Cycle boundary) {
      this.boundary = boundary;

      holes = new ArrayList<>();

      setDefined(this.boundary.isDefined());

   }

   /**
    * Creates a simple 'Face' object without holes by passing a list of points
    * which define the outer boundary. The points of the passed list are connected
    * in the order of their occurrence in the list, whereby the last point of the
    * list is connected with the first.
    * 
    * @param boundaryPoints
    *           - List of points of the boundary
    */
   public Face(final List<Point> boundaryPoints) {
      this(new Cycle(boundaryPoints));
   }

   /**
    * Add a hole to this 'Face'.<br>
    * Only defined 'Cycle' hole objects will be added.
    * 
    * @param hole
    *           - the hole, a 'Cycle', to add
    * 
    * @return true if adding was successful, false otherwise
    */
   public boolean add(final Cycle hole) {
      if (hole.isDefined()) {

         return holes.add(hole);
      }

      return false;
   }

   /**
    * Add a hole to this 'Face' by passing a list of points which define the hole
    * boundary. The points of the passed list are connected in the order of their
    * occurrence in the list, whereby the last point of the list is connected with
    * the first.
    * 
    * @param holePoints
    *           - the hole, List of points of the hole
    * 
    * @return true if adding was successful, false otherwise
    */
   public boolean add(final List<Point> holePoints) {

      return holes.add(new Cycle(holePoints));

   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Spatial#isEmpty()
    */
   @Override
   public boolean isEmpty() {
      return !isDefined();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Spatial#getBoundingBox()
    */
   @Override
   public Rectangle getBoundingBox() {

      return boundary.getBoundingBox();
   }

   /**
    * Get the boundary of this 'Face'
    * 
    * @return the boundary, a 'Cycle' object
    */
   public Cycle getBoundary() {
      return boundary;
   }

   /**
    * @return the holes
    */
   public List<Cycle> getHoles() {
      return holes;
   }

   /**
    * Get the number of holes in this 'Face' object
    * 
    * @return number of holes
    */
   public int getNoHoles() {
      return holes.size();
   }

   /**
    * Get the entire halfsegments of boundary and holes
    * 
    * @return the halfsegments
    */
   public List<Halfsegment> getHalfsegments() {
      List<Halfsegment> halfsegments = new ArrayList<>();

      halfsegments.addAll(boundary.getHalfsegments());

      for (Cycle hole : holes) {
         halfsegments.addAll(hole.getHalfsegments());
      }

      return halfsegments;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.IGeneralType#isDefined()
    */
   public boolean isDefined() {
      return defined;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.IGeneralType#setDefined(boolean)
    */
   public void setDefined(final boolean defined) {
      this.defined = defined;
   }

}
