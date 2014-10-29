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

package mmdb.data.attributes.unit;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.util.TemporalObjects.Period;
import mmdb.data.features.Movable;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Superclass for all unit attributes.
 *
 * @author Alexander Castor
 */
public abstract class UnitAttribute<T extends MemoryAttribute> extends MemoryAttribute implements
		Movable {

	/**
	 * The unit's period.
	 */
	private Period period;

	/**
	 * The unit's value.
	 */
	private T value;

	/**
	 * Retrieves a uni object which will be used as value for this attribute.
	 */
	protected abstract T getUnitObject();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) throws ConversionException {
		Period period = new Period();
		period.fromList(list.first());
		setPeriod(period);
		T value = getUnitObject();
		value.fromList(list.second());
		setValue(value);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() throws ConversionException {
		return ListExpr.twoElemList(getPeriod().toList(), getValue().toList());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Movable#getPeriods()
	 */
	@Override
	public List<Period> getPeriods() {
		List<Period> periods = new ArrayList<Period>();
		periods.add(getPeriod());
		return periods;
	}

	/**
	 * Getter for period.
	 * 
	 * @return the period
	 */
	public Period getPeriod() {
		return period;
	}

	/**
	 * Setter for period.
	 * 
	 * @param period
	 *            the period to set
	 */
	public void setPeriod(Period period) {
		this.period = period;
	}

	/**
	 * Getter for value.
	 * 
	 * @return the value
	 */
	public T getValue() {
		return value;
	}

	/**
	 * Setter for value.
	 * 
	 * @param value
	 *            the value to set
	 */
	public void setValue(T value) {
		this.value = value;
	}

}
