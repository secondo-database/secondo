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

package viewer.spacetimecube;

import gui.idmanager.ID;


/**
 * Class representing a simple 2D-Point.
 * @author Franz Fahrmeier
 *
 */
public class Point2DSTC {

	private double xCoord;
	private double yCoord;
	private ID id;
	
	public Point2DSTC(double x, double y, ID secondoID) {
		xCoord = x;
		yCoord = y;
		id = secondoID;
	}
	
	public double getX() { return xCoord; }
	public double getY() { return yCoord; }
	public ID getSecondoID() { return id; }	
	
}
