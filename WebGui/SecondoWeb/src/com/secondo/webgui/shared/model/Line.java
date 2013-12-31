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

/**
 * This class models a line element with 2 points and the type "Line". A line is always part of a polyline or a moving point. 
 * Therefore it does not implement the DataType interface but it has to be serializable to be exchanged between client and server.
 * 
 * @author Kristina Steiger
 * 
 */
public class Line implements Serializable{
	
	private static final long serialVersionUID = -7668694383184433149L;
	private String type = "Line";
	private Point pointA = new Point();
	private Point pointB = new Point();
	
	public Line(){}

	/**Returns the type of the datatype
	 * 
	 * @return The type of the datatype
	 * */
	public String getType() {
		return type;
	}

	/**Returns the first point of the line
	 * 
	 * @return The first point of the line
	 * */
	public Point getPointA() {
		return pointA;
	}
	
	/**Sets the first point of the line to the given point object
	 * 
	 * @param The point object
	 * */
	public void setPointA(Point pointA) {
		this.pointA = pointA;
	}

	/**Returns the second point of the line
	 * 
	 * @return The second point of the line
	 * */
	public Point getPointB() {
		return pointB;
	}

	/**Sets the second point of the line to the given point object
	 * 
	 * @param The point object
	 * */
	public void setPointB(Point pointB) {
		this.pointB = pointB;
	}
}
