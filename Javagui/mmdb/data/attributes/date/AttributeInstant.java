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
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Orderable;
import mmdb.data.features.Parsable;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'instant'.
 *
 * @author Alexander Castor
 */
public class AttributeInstant extends MemoryAttribute implements Orderable, Parsable {

	/**
	 * Format for date of type yyyy-MM-dd HH:mm:ss.SSS.
	 */
	public static final SimpleDateFormat FORMAT_1;

	/**
	 * Format for date of type yyyy-MM-dd-HH:mm:ss.
	 */
	public static final SimpleDateFormat FORMAT_2;

	/**
	 * Format for date of type yyyy-MM-dd-HH:mm.
	 */
	public static final SimpleDateFormat FORMAT_3;

	/**
	 * Format for date of type yyyy-MM-dd.
	 */
	public static final SimpleDateFormat FORMAT_4;

	/**
	 * The list of all formats.
	 */
	private static List<SimpleDateFormat> formats;

	static {
		FORMAT_1 = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss.SSS");
		FORMAT_2 = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss");
		FORMAT_3 = new SimpleDateFormat("yyyy-MM-dd-HH:mm");
		FORMAT_4 = new SimpleDateFormat("yyyy-MM-dd");
		FORMAT_1.setLenient(false);
		FORMAT_2.setLenient(false);
		FORMAT_3.setLenient(false);
		FORMAT_4.setLenient(false);
		formats = new ArrayList<SimpleDateFormat>();
		formats.add(FORMAT_1);
		formats.add(FORMAT_2);
		formats.add(FORMAT_3);
		formats.add(FORMAT_4);
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
		for (SimpleDateFormat dateFormat : formats) {
			synchronized (this) {
				try {
					setDate(dateFormat.parse(dateString));
					setFormat(dateFormat);
					return;
				} catch (ParseException e) {
					// nothing to do
				}
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
		AttributeInstant instantOther = (AttributeInstant) attribute;
		if (getDate().equals(instantOther.getDate())) {
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
		Date dateOther = ((AttributeInstant) attribute).getDate();
		return getDate().compareTo(dateOther);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Parseable#parse(java.lang.String)
	 */
	@Override
	public Parsable parse(String text) {
		AttributeInstant result = new AttributeInstant();
		synchronized (this) {
			for (SimpleDateFormat dateFormat : formats) {
				try {
					result.setDate(dateFormat.parse(text));
					result.setFormat(dateFormat);
					return result;
				} catch (ParseException e) {
					// nothing to do
				}
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
