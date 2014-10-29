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

package mmdb.data.attributes.spatial;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.util.SpatialObjects.Face;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'region'.
 *
 * @author Alexander Castor
 */
public class AttributeRegion extends MemoryAttribute {

	/**
	 * The list of faces the region consists of
	 */
	private List<Face> faces = new ArrayList<Face>();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) {
		ListExpr tmp = list;
		while (!tmp.isEmpty()) {
			ListExpr facesList = tmp.first();
			Face face = new Face();
			face.fromList(facesList);
			addFace(face);
			tmp = tmp.rest();
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() {
		ListExpr facesList = new ListExpr();
		for (Face face : faces) {
			facesList = ListExpr.concat(facesList, face.toList());
		}
		return facesList;
	}

	/**
	 * Getter for faces.
	 * 
	 * @return the faces
	 */
	public List<Face> getFaces() {
		return faces;
	}

	/**
	 * Adds a face to the list.
	 * 
	 * @param face
	 *            the face to be added
	 */
	public void addFace(Face face) {
		faces.add(face);
	}

}
