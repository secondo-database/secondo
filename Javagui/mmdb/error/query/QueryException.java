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

package mmdb.error.query;

import mmdb.error.MMDBException;

/**
 * This exception indicates an error when executing a query.
 *
 * @author Alexander Castor
 */
public class QueryException extends MMDBException {

	private static final long serialVersionUID = 5668160028159260767L;

	/**
	 * Unexpected error which triggers a printStackTrace().
	 */
	protected Throwable unexpectedError;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public QueryException(String message) {
		super(message);
	}

	/**
	 * Creates a new exception with an unexpected error causing the exception.
	 * 
	 * @param unexpectedError
	 *            the unexpected error causing the exception
	 */
	public QueryException(Throwable unexpectedError) {
		super(null);
		this.unexpectedError = unexpectedError;
	}

	/**
	 * Getter for unexpectedError.
	 * 
	 * @return the unexpectedError
	 */
	public Throwable getUnexpectedError() {
		return unexpectedError;
	}

}
