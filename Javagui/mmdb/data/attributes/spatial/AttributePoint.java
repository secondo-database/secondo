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

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Matchable;
import mmdb.data.features.Parsable;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'point'.
 *
 * @author Alexander Castor
 */
public class AttributePoint extends MemoryAttribute implements Matchable, Parsable {

	/**
	 * The point's x coordinate
	 */
	private float xValue;

	/**
	 * The point's y coordinate
	 */
	private float yValue;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) {
		setXValue((float) list.first().realValue());
		setYValue((float) list.second().realValue());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() {
		return ListExpr.twoElemList(ListExpr.realAtom(getXValue()), ListExpr.realAtom(getYValue()));
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (obj == null) {
			return false;
		}
		AttributePoint other = (AttributePoint) obj;
		if (Float.floatToIntBits(getXValue()) != Float.floatToIntBits(other.getXValue()))
			return false;
		if (Float.floatToIntBits(getYValue()) != Float.floatToIntBits(other.getYValue()))
			return false;
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + Float.floatToIntBits(getXValue());
		result = prime * result + Float.floatToIntBits(getYValue());
		return result;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Parseable#parse(java.lang.String)
	 */
	@Override
	public Parsable parse(String text) {
		if (text == null) {
			return null;
		}
		String[] tokens = text.split(" ");
		if (tokens == null || tokens.length != 2) {
			return null;
		}
		float xValue;
		float yValue;
		try {
			xValue = Float.parseFloat(tokens[0]);
			yValue = Float.parseFloat(tokens[1]);
		} catch (NumberFormatException e) {
			return null;
		}
		AttributePoint result = new AttributePoint();
		result.setXValue(xValue);
		result.setYValue(yValue);
		return result;
	}

	/**
	 * Getter for xValue.
	 * 
	 * @return the xValue
	 */
	public float getXValue() {
		return xValue;
	}

	/**
	 * Setter for xValue.
	 * 
	 * @param xValue
	 *            the xValue to set
	 */
	public void setXValue(float xValue) {
		this.xValue = xValue;
	}

	/**
	 * Getter for yValue.
	 * 
	 * @return the yValue
	 */
	public float getYValue() {
		return yValue;
	}

	/**
	 * Setter for yValue.
	 * 
	 * @param yValue
	 *            the yValue to set
	 */
	public void setYValue(float yValue) {
		this.yValue = yValue;
	}

}
