//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.Date;

/**
 * This class models an interval of time between two dates. A timeinterval is always part of a moving point. 
 * Therefore it does not implement the DataType interface but it has to be serializable to be exchanged between client and server.
 * 
 * @author Kristina Steiger
 * 
 */
public class TimeInterval implements Serializable{
	
	private static final long serialVersionUID = -2192791604380590026L;
	private String type = "Time";	
	private Date timeA = new Date();
	private Date timeB = new Date();
	
	public TimeInterval(){		
	}

	/**Returns the type of the timeinterval
	 * 
	 * @return The type of the timeinterval
	 * */
	public String getType() {
		return type;
	}
	
	/**Returns the first date of the time interval
	 * 
	 * @return The first date of the time interval
	 * */
	public Date getTimeA() {
		return timeA;
	}

	/**Sets the first date of the time interval to the given date
	 * 
	 * @param timeA The new first date of the time interval
	 * */
	public void setTimeA(Date timeA) {
		this.timeA = timeA;
	}

	/**Returns the second date of the time interval
	 * 
	 * @return The second date of the time interval
	 * */
	public Date getTimeB() {
		return timeB;
	}

	/**Sets the second date of the time interval to the given date
	 * 
	 * @param timeA The new second date of the time interval
	 * */
	public void setTimeB(Date timeB) {
		this.timeB = timeB;
	}
}
