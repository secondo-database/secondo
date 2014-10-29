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

package mmdb.data.attributes.date;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Orderable;
import mmdb.data.features.Parsable;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'date'.
 *
 * @author Alexander Castor
 */
public class AttributeDate extends MemoryAttribute implements Orderable, Parsable {

	/**
	 * Format for date of type d.M.yyyy.
	 */
	public static final SimpleDateFormat FORMAT_1;

	/**
	 * Format for date of type yyyy-M-d.
	 */
	public static final SimpleDateFormat FORMAT_2;

	static {
		FORMAT_1 = new SimpleDateFormat("d.M.yyyy");
		FORMAT_2 = new SimpleDateFormat("yyyy-M-d");
		FORMAT_1.setLenient(false);
		FORMAT_2.setLenient(false);
	}

	/**
	 * The date value.
	 */
	private Date date;

	/**
	 * The date's format.
	 */
	private SimpleDateFormat format;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) throws ConversionException {
		String dateString = list.stringValue();
		synchronized (this) {
			try {
				setDate(FORMAT_1.parse(dateString));
				setFormat(FORMAT_1);
				return;
			} catch (ParseException e) {
				// nothing to do
			}
			try {
				setDate(FORMAT_2.parse(dateString));
				setFormat(FORMAT_2);
				return;
			} catch (ParseException e) {
				// nothing to do
			}
		}
		throw new ConversionException("-> Could not parse date value from string");
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() {
		return ListExpr.stringAtom(format.format(getDate()));
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object attribute) {
		if (attribute == null) {
			return false;
		}
		AttributeDate dateOther = (AttributeDate) attribute;
		if (getDate().equals(dateOther.getDate())) {
			return true;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		return getDate().hashCode();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	@Override
	public int compareTo(Orderable attribute) {
		Date dateOther = ((AttributeDate) attribute).getDate();
		return getDate().compareTo(dateOther);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Parseable#parse(java.lang.String)
	 */
	@Override
	public Parsable parse(String text) {
		AttributeDate result = new AttributeDate();
		synchronized (this) {
			try {
				result.setDate(FORMAT_1.parse(text));
				result.setFormat(FORMAT_1);
				return result;
			} catch (ParseException e) {
				// do nothing
			}
			try {
				result.setDate(FORMAT_2.parse(text));
				result.setFormat(FORMAT_2);
				return result;
			} catch (ParseException e) {
				// do nothing
			}
		}
		return null;
	}

	/**
	 * Getter for date.
	 * 
	 * @return the date
	 */
	public Date getDate() {
		return date;
	}

	/**
	 * Setter for date.
	 * 
	 * @param date
	 *            the date to set
	 */
	public void setDate(Date date) {
		this.date = date;
	}

	/**
	 * Getter for format.
	 * 
	 * @return the format
	 */
	public SimpleDateFormat getFormat() {
		return format;
	}

	/**
	 * Setter for format.
	 * 
	 * @param format
	 *            the format to set
	 */
	public void setFormat(SimpleDateFormat format) {
		this.format = format;
	}

}
