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

package mmdb.data.attributes.range;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.util.TemporalObjects.Period;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'periods'.
 *
 * @author Alexander Castor
 */
public class AttributePeriods extends MemoryAttribute {

	/**
	 * The attribute's periods
	 */
	private List<Period> periods = new ArrayList<Period>();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) throws ConversionException {
		ListExpr tmp = list;
		while (!tmp.isEmpty()) {
			ListExpr periodList = tmp.first();
			Period period = new Period();
			period.fromList(periodList);
			addPeriod(period);
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
		ListExpr periodList = new ListExpr();
		for (Period period : getPeriods()) {
			periodList = ListExpr.concat(periodList, period.toList());
		}
		return periodList;
	}

	/**
	 * Getter for periods.
	 * 
	 * @return the periods
	 */
	public List<Period> getPeriods() {
		return periods;
	}

	/**
	 * Adds a period to the list.
	 * 
	 * @param period
	 *            the period to be added
	 */
	public void addPeriod(Period period) {
		periods.add(period);
	}

}
