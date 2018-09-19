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

import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.util.CycleIF;
import mol.interfaces.spatial.util.FaceIF;
import mol.interfaces.spatial.util.HalfsegmentIF;
import mol.interfaces.spatial.util.RectangleIF;

/**
 * This utility class represents spatial objects of type 'Face'.<br>
 * A 'Face' is a polygon which possibly containing some polygonal holes.
 * 
 * @author Markus Fuessel
 */
public class Face implements FaceIF {

   /**
    * The defined flag, indicates if a data type object is defined
    */
   private boolean defined;

   /**
    * Outer boundary of this 'Face'
    */
   private final CycleIF boundary;

   /**
    * List with holes this 'Face' may contain
    */
   private final List<CycleIF> holes;

   /**
    * Constructor for a simple 'Face' object without holes.
    * 
    * @param boundary
    *           - outer boundary of this 'Face'
    */
   public Face(final CycleIF boundary) {
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
   public Face(final List<PointIF> boundaryPoints) {
      this(new Cycle(boundaryPoints));
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.spatial.util.FaceIF#add(mol.datatypes.spatial.util.CycleIF)
    */
   @Override
   public boolean add(final CycleIF hole) {
      if (hole.isDefined()) {

         return holes.add(hole);
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.util.FaceIF#add(java.util.List)
    */
   @Override
   public boolean add(final List<PointIF> holePoints) {

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
   public RectangleIF getBoundingBox() {

      return boundary.getBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.util.FaceIF#getBoundary()
    */
   @Override
   public CycleIF getBoundary() {
      return boundary;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.util.FaceIF#getHoles()
    */
   @Override
   public List<CycleIF> getHoles() {
      return holes;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.util.FaceIF#getNoHoles()
    */
   @Override
   public int getNoHoles() {
      return holes.size();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.util.FaceIF#getHalfsegments()
    */
   @Override
   public List<HalfsegmentIF> getHalfsegments() {
      List<HalfsegmentIF> halfsegments = new ArrayList<>();

      halfsegments.addAll(boundary.getHalfsegments());

      for (CycleIF hole : holes) {
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
