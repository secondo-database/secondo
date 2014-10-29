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

package mmdb.data.attributes.util;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.spatial.AttributePoints;
import sj.lang.ListExpr;

/**
 * This class is a container for spatial object types that are used in several
 * attribute types.
 *
 * @author Alexander Castor
 */
public class SpatialObjects {

	/**
	 * This class represents a segment which is a line in the plane. Although
	 * this class is (not yet) a secondo attribute type, it extends
	 * MemoryAttribute to be forced to implement fromList and toList.
	 *
	 * @author Alexander Castor
	 */
	public static class Segment extends MemoryAttribute {

		/**
		 * The point's x1 coordinate
		 */
		private float xValue1;

		/**
		 * The point's y1 coordinate
		 */
		private float yValue1;

		/**
		 * The point's x2 coordinate
		 */
		private float xValue2;

		/**
		 * The point's y2 coordinate
		 */
		private float yValue2;

		/**
		 * Converts a given nested list to a segment instance.
		 * 
		 * @param list
		 *            the list to be converted
		 */
		@Override
		public void fromList(ListExpr list) {
			setxValue1((float) list.first().realValue());
			setyValue1((float) list.second().realValue());
			setxValue2((float) list.third().realValue());
			setyValue2((float) list.fourth().realValue());
		}

		/**
		 * Converts the segment in nested list representation.
		 * 
		 * @return the converted list
		 */
		@Override
		public ListExpr toList() {
			return ListExpr.fourElemList(ListExpr.realAtom(getxValue1()),
					ListExpr.realAtom(getyValue1()), ListExpr.realAtom(getxValue2()),
					ListExpr.realAtom(getyValue2()));
		}

		/**
		 * Getter for xValue1.
		 * 
		 * @return the xValue1
		 */
		public float getxValue1() {
			return xValue1;
		}

		/**
		 * Setter for xValue1.
		 * 
		 * @param xValue1
		 *            the xValue1 to set
		 */
		public void setxValue1(float xValue1) {
			this.xValue1 = xValue1;
		}

		/**
		 * Getter for yValue1.
		 * 
		 * @return the yValue1
		 */
		public float getyValue1() {
			return yValue1;
		}

		/**
		 * Setter for yValue1.
		 * 
		 * @param yValue1
		 *            the yValue1 to set
		 */
		public void setyValue1(float yValue1) {
			this.yValue1 = yValue1;
		}

		/**
		 * Getter for xValue2.
		 * 
		 * @return the xValue2
		 */
		public float getxValue2() {
			return xValue2;
		}

		/**
		 * Setter for xValue2.
		 * 
		 * @param xValue2
		 *            the xValue2 to set
		 */
		public void setxValue2(float xValue2) {
			this.xValue2 = xValue2;
		}

		/**
		 * Getter for yValue2.
		 * 
		 * @return the yValue2
		 */
		public float getyValue2() {
			return yValue2;
		}

		/**
		 * Setter for yValue2.
		 * 
		 * @param yValue2
		 *            the yValue2 to set
		 */
		public void setyValue2(float yValue2) {
			this.yValue2 = yValue2;
		}

	}

	/**
	 * This class represents a face element consisting of an outer and hole
	 * cycles.
	 *
	 * @author Alexander Castor
	 */
	public static class Face {

		/**
		 * The inner cycle of this face.
		 */
		private AttributePoints outerCycle;

		/**
		 * The hole cycles of this face.
		 */
		private List<AttributePoints> holeCycles = new ArrayList<AttributePoints>();

		/**
		 * Converts a given nested list to a face instance.
		 * 
		 * @param list
		 *            the list to be converted
		 */
		public void fromList(ListExpr list) {
			ListExpr tmp = list;
			AttributePoints outerCycle = new AttributePoints();
			outerCycle.fromList(tmp.first());
			setOuterCycle(outerCycle);
			tmp = tmp.rest();
			while (!tmp.isEmpty()) {
				ListExpr holeCycleList = tmp.first();
				AttributePoints points = new AttributePoints();
				points.fromList(holeCycleList);
				addCycleToHoleCycles(points);
				tmp = tmp.rest();
			}
		}

		/**
		 * Converts the face in nested list representation.
		 * 
		 * @return the converted list
		 */
		public ListExpr toList() {
			ListExpr cycleList = new ListExpr();
			cycleList = ListExpr.concat(cycleList, getOuterCycle().toList());
			for (AttributePoints cycle : getHoleCycles()) {
				cycleList = ListExpr.concat(cycleList, cycle.toList());
			}
			return cycleList;
		}

		/**
		 * Getter for outerCycle.
		 * 
		 * @return the outerCycle
		 */
		public AttributePoints getOuterCycle() {
			return outerCycle;
		}

		/**
		 * Setter for outerCycle.
		 * 
		 * @param outerCycle
		 *            the outerCycle to be set
		 */
		public void setOuterCycle(AttributePoints outerCycle) {
			this.outerCycle = outerCycle;
		}

		/**
		 * Getter for holeCycles.
		 * 
		 * @return the holeCycles
		 */
		public List<AttributePoints> getHoleCycles() {
			return holeCycles;
		}

		/**
		 * Adds a new cycle to the hole cycles.
		 * 
		 * @param cycle
		 *            the cycle to be added
		 */
		public void addCycleToHoleCycles(AttributePoints cycle) {
			holeCycles.add(cycle);
		}

	}

}
