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

package mmdb.data.attributes.moving;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.unit.UnitAttribute;
import mmdb.data.attributes.util.TemporalObjects.Period;
import mmdb.data.features.Movable;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Superclass for all moving attributes.
 *
 * @author Alexander Castor
 */
public abstract class MovingAttribute<T extends UnitAttribute<?>> extends MemoryAttribute implements
		Movable {

	/**
	 * The list of units.
	 */
	private List<T> units = new ArrayList<T>();

	/**
	 * Injects a value object.
	 */
	protected abstract T getMovableObject();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) throws ConversionException {
		ListExpr tmp = list;
		while (!tmp.isEmpty()) {
			ListExpr unitList = tmp.first();
			T unit = getMovableObject();
			unit.fromList(unitList);
			addUnit(unit);
			tmp = tmp.rest();
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() throws ConversionException {
		ListExpr unitList = new ListExpr();
		for (T unit : getUnits()) {
			unitList = ListExpr.concat(unitList, unit.toList());
		}
		return unitList;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Movable#getPeriods()
	 */
	@Override
	public List<Period> getPeriods() {
		List<Period> periods = new ArrayList<Period>();
		for (T unit : getUnits()) {
			periods.add(unit.getPeriod());
		}
		return periods;
	}

	/**
	 * Getter for units.
	 * 
	 * @return the units
	 */
	public List<T> getUnits() {
		return units;
	}

	/**
	 * Adds a unit to the list.
	 * 
	 * @param unit
	 *            the unit to be added
	 */
	public void addUnit(T unit) {
		units.add(unit);
	}

}
