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
import mmdb.data.attributes.util.SpatialObjects.Segment;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'line'.
 *
 * @author Alexander Castor
 */
public class AttributeLine extends MemoryAttribute {

	/**
	 * The list of segments the line consists of
	 */
	private List<Segment> segments = new ArrayList<Segment>();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) {
		ListExpr tmp = list;
		while (!tmp.isEmpty()) {
			ListExpr segmentList = tmp.first();
			Segment segment = new Segment();
			segment.fromList(segmentList);
			addSegment(segment);
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
		ListExpr segementList = new ListExpr();
		for (Segment segment : segments) {
			segementList = ListExpr.concat(segementList, segment.toList());
		}
		return segementList;
	}

	/**
	 * Getter for segments.
	 * 
	 * @return the segments
	 */
	public List<Segment> getSegments() {
		return segments;
	}

	/**
	 * Adds a segment to the list.
	 * 
	 * @param segment
	 *            the segment to be added
	 */
	public void addSegment(Segment segment) {
		segments.add(segment);
	}

}
