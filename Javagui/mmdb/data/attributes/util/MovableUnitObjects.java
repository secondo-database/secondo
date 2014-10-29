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
import mmdb.data.attributes.spatial.AttributeLine;
import sj.lang.ListExpr;

/**
 * This class is a container for movable object types that are used in unit
 * types.
 *
 * @author Alexander Castor
 */
public class MovableUnitObjects {

	/**
	 * This class represents a movable region element consisting of a list of
	 * MovableFaces. Although this class is (not yet) a secondo attribute type,
	 * it extends MemoryAttribute to be forced to implement fromList and toList.
	 *
	 * @author Alexander Castor
	 */
	public static class MovableRegion extends MemoryAttribute {

		/**
		 * The unit's faces.
		 */
		private List<MovableFace> faces = new ArrayList<MovableFace>();

		/*
		 * (non-Javadoc)
		 * 
		 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
		 */
		@Override
		public void fromList(ListExpr list) {
			ListExpr tmp = list;
			while (!tmp.isEmpty()) {
				ListExpr faceList = tmp.first();
				MovableFace face = new MovableFace();
				face.fromList(faceList);
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
			for (MovableFace face : getFaces()) {
				facesList = ListExpr.concat(facesList, face.toList());
			}
			return facesList;
		}

		/**
		 * Getter for faces.
		 * 
		 * @return the faces
		 */
		public List<MovableFace> getFaces() {
			return faces;
		}

		/**
		 * Adds a new face to the faces cycles.
		 * 
		 * @param cycle
		 *            the cycle to be added
		 */
		public void addFace(MovableFace face) {
			faces.add(face);
		}

	}

	/**
	 * This class represents a movable face element consisting of an outer and
	 * hole cycles.
	 *
	 * @author Alexander Castor
	 */
	public static class MovableFace {

		/**
		 * The outer cycle of this face.
		 */
		private AttributeLine outerCycle;

		/**
		 * The hole cycles of this face.
		 */
		private List<AttributeLine> holeCycles = new ArrayList<AttributeLine>();

		/**
		 * Converts a given nested list to a movable face instance.
		 * 
		 * @param list
		 *            the list to be converted
		 */
		public void fromList(ListExpr list) {
			ListExpr tmp = list;
			AttributeLine outerCycle = new AttributeLine();
			outerCycle.fromList(tmp.first());
			setOuterCycle(outerCycle);
			tmp = tmp.rest();
			while (!tmp.isEmpty()) {
				ListExpr holeCycleList = tmp.first();
				AttributeLine holeCycle = new AttributeLine();
				holeCycle.fromList(holeCycleList);
				addCycleToHoleCycles(holeCycle);
				tmp = tmp.rest();
			}
		}

		/**
		 * Converts the movable face in nested list representation.
		 * 
		 * @return the converted list
		 */
		public ListExpr toList() {
			ListExpr cycleList = new ListExpr();
			cycleList = ListExpr.concat(cycleList, getOuterCycle().toList());
			for (AttributeLine holeCycle : getHoleCycles()) {
				cycleList = ListExpr.concat(cycleList, holeCycle.toList());
			}
			return cycleList;
		}

		/**
		 * Getter for outerCycle.
		 * 
		 * @return the outerCycle
		 */
		public AttributeLine getOuterCycle() {
			return outerCycle;
		}

		/**
		 * Setter for outerCycle.
		 * 
		 * @param outerCycle
		 *            the outerCycle to set
		 */
		public void setOuterCycle(AttributeLine outerCycle) {
			this.outerCycle = outerCycle;
		}

		/**
		 * Getter for holeCycles.
		 * 
		 * @return the holeCycles
		 */
		public List<AttributeLine> getHoleCycles() {
			return holeCycles;
		}

		/**
		 * Adds a new cycle to the hole cycles.
		 * 
		 * @param cycle
		 *            the cycle to be added
		 */
		public void addCycleToHoleCycles(AttributeLine cycle) {
			holeCycles.add(cycle);
		}

	}

	/**
	 * This class represents the spatial part of a moving real object. Although
	 * this class is (not yet) a secondo attribute type, it extends
	 * MemoryAttribute to be forced to implement fromList and toList.
	 *
	 * @author Alexander Castor
	 */
	public static class MovableReal extends MemoryAttribute {

		/**
		 * The movable's first real.
		 */
		private float firstReal;

		/**
		 * The movable's second real.
		 */
		private float secondReal;

		/**
		 * The movable's third real.
		 */
		private float thirdReal;

		/**
		 * The movable's boolean part.
		 */
		private boolean booleanPart;

		/**
		 * Converts a given nested list to a movable real instance.
		 * 
		 * @param list
		 *            the list to be converted
		 */
		@Override
		public void fromList(ListExpr list) {
			setFirstReal((float) list.first().realValue());
			setSecondReal((float) list.second().realValue());
			setThirdReal((float) list.third().realValue());
			setBooleanPart(list.fourth().boolValue());
		}

		/**
		 * Converts the segment in nested list representation.
		 * 
		 * @return the converted list
		 */
		@Override
		public ListExpr toList() {
			return ListExpr.fourElemList(ListExpr.realAtom(getFirstReal()),
					ListExpr.realAtom(getSecondReal()), ListExpr.realAtom(getThirdReal()),
					ListExpr.boolAtom(getBooleanPart()));
		}

		/**
		 * Getter for firstReal.
		 * 
		 * @return the firstReal
		 */
		public float getFirstReal() {
			return firstReal;
		}

		/**
		 * Setter for firstReal.
		 * 
		 * @param firstReal
		 *            the firstReal to set
		 */
		public void setFirstReal(float firstReal) {
			this.firstReal = firstReal;
		}

		/**
		 * Getter for secondReal.
		 * 
		 * @return the secondReal
		 */
		public float getSecondReal() {
			return secondReal;
		}

		/**
		 * Setter for secondReal.
		 * 
		 * @param secondReal
		 *            the secondReal to set
		 */
		public void setSecondReal(float secondReal) {
			this.secondReal = secondReal;
		}

		/**
		 * Getter for thirdReal.
		 * 
		 * @return the thirdReal
		 */
		public float getThirdReal() {
			return thirdReal;
		}

		/**
		 * Setter for thirdReal.
		 * 
		 * @param thirdReal
		 *            the thirdReal to set
		 */
		public void setThirdReal(float thirdReal) {
			this.thirdReal = thirdReal;
		}

		/**
		 * Getter for booleanPart.
		 * 
		 * @return the booleanPart
		 */
		public boolean getBooleanPart() {
			return booleanPart;
		}

		/**
		 * Setter for booleanPart.
		 * 
		 * @param booleanPart
		 *            the booleanPart to set
		 */
		public void setBooleanPart(boolean booleanPart) {
			this.booleanPart = booleanPart;
		}

	}

}
