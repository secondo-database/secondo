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
import mol.datatypes.spatial.util.Face;
import mol.datatypes.spatial.util.Halfsegment;
import mol.datatypes.spatial.util.Rectangle;

/**
 * This class represents spatial objects of type 'Region'
 * 
 * @author Markus Fuessel
 */
public class Region extends GeneralType implements Spatial {

   /**
    * List of 'Face' objects
    */
   private final List<Face> faces;

   /**
    * The minimum bounding box of this 'Region' object
    */
   private Rectangle objectMBB;

   /**
    * Constructor for an empty 'Region' object.
    * 
    * @param defined
    *           - specifies whether the 'Region' object is to be set as defined
    */
   public Region(final boolean defined) {
      faces = new ArrayList<>();
      objectMBB = new Rectangle();

      setDefined(defined);
   }

   /**
    * Creates a simple 'Region' object with one 'Face'.<br>
    * 'Region' will be undefined if passed 'Face' is undefined.
    * 
    * @param face
    */
   public Region(final Face face) {
      this(false);

      setDefined(add(face));
   }

   /**
    * Creates a simple 'Region' object with one 'Face' without holes by passing a
    * list of points which define the outer boundary. The points of the passed list
    * are connected in the order of their occurrence in the list, whereby the last
    * point of the list is connected with the first.
    * 
    * @param boundaryPoints
    *           - List of points of the boundary
    */
   public Region(final List<Point> boundaryPoints) {
      this(new Face(boundaryPoints));
   }

   /**
    * Add a 'Face' object to this 'Region'<br>
    * Only a defined 'Face' will be added.
    * 
    * @param face
    *           - the 'Face' to add
    * 
    * @return true if adding was successful, false otherwise
    */
   public boolean add(final Face face) {
      if (face.isDefined() && faces.add(face)) {

         objectMBB = objectMBB.merge(face.getBoundingBox());

         return true;

      }

      return false;
   }

   /**
    * Get the number of components in this 'Region'. Number of faces.
    * 
    * @return number of faces in this 'Region'
    */
   public int getNoComponents() {
      return faces.size();
   }

   /**
    * Check if this 'Region' is empty
    * 
    * @return true if this 'Region' is empty, false otherwise
    */
   public boolean isEmpty() {
      return !isDefined() || faces.isEmpty();
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
    * Get the entire halfsegments of boundary and holes
    * 
    * @return the halfsegments
    */
   public List<Halfsegment> getHalfsegments() {
      List<Halfsegment> halfsegments = new ArrayList<>();

      for (Face face : faces) {
         halfsegments.addAll(face.getHalfsegments());
      }

      return halfsegments;
   }

}
