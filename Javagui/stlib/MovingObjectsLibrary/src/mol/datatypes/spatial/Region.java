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
import mol.datatypes.spatial.util.Face;
import mol.datatypes.spatial.util.Rectangle;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.RegionIF;
import mol.interfaces.spatial.util.FaceIF;
import mol.interfaces.spatial.util.HalfsegmentIF;
import mol.interfaces.spatial.util.RectangleIF;

/**
 * This class represents spatial objects of type 'Region'
 * 
 * @author Markus Fuessel
 */
public class Region extends GeneralType implements RegionIF {

   /**
    * List of 'FaceIF' objects
    */
   private final List<FaceIF> faces;

   /**
    * The minimum bounding box of this 'Region' object
    */
   private RectangleIF objectMBB;

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
    * Creates a simple 'Region' object with one 'FaceIF'.<br>
    * 'Region' will be undefined if passed 'FaceIF' is undefined.
    * 
    * @param face
    */
   public Region(final FaceIF face) {
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
   public Region(final List<PointIF> boundaryPoints) {
      this(new Face(boundaryPoints));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.RegionIF#add(mol.datatypes.spatial.util.FaceIF)
    */
   @Override
   public boolean add(final FaceIF face) {
      if (face.isDefined() && faces.add(face)) {

         objectMBB = objectMBB.merge(face.getBoundingBox());

         return true;

      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.RegionIF#getNoComponents()
    */
   @Override
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
   public RectangleIF getBoundingBox() {
      return objectMBB;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.RegionIF#getHalfsegments()
    */
   @Override
   public List<HalfsegmentIF> getHalfsegments() {
      List<HalfsegmentIF> halfsegments = new ArrayList<>();

      for (FaceIF face : faces) {
         halfsegments.addAll(face.getHalfsegments());
      }

      return halfsegments;
   }

}
