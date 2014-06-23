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
import java.util.ArrayList;

/**
 * This class models a polygon element with an individual ID, the type "Polygon", a name, a list of points for
 * the outline, a default color and a list of attributes.
 * It implements the interface DataType and has to be serializable to be exchanged between client and server.
 * 
 * @author Kristina Steiger
 */
public class Polygon implements DataType, Serializable{
	
	private static final long serialVersionUID = 3071379364546223507L;
	private int id = 0;
	private String type = "Polygon";	
	private String name = "Polygon";
	private ArrayList<Point> path = new ArrayList<Point>();
	private String color = "orange";
	private ArrayList<String> attributeList = new ArrayList<String>();
	
	public Polygon(){}
	
	/**Adds a point to the path list of the polygon
	 * 
	 * @param point The point to be added to the path
	 * */
	public void addPointToPath(Point point){
		this.path.add(point);		
	}

	/**Returns the path for the outline of the polygon
	 * 
	 * @return The path for the outline of the polygon
	 * */
	public ArrayList<Point> getPath() {
		return path;
	}

	/**Returns the individual ID of the polygon
	 * 
	 * @return The ID of the polygon
	 * */
	@Override
	public int getId() {
		return id;
	}

	/**Sets the ID of the polygon to the given value
	 * 
	 * @param id The new ID of the polygon
	 * */
	@Override
	public void setId(int id) {
		this.id = id;
	}

	/**Returns the type of the polygon
	 * 
	 * @return The type of the polygon
	 * */
	@Override
	public String getType() {
		return type;
	}
	
	/**Returns the name of the polygon
	 * 
	 * @return The name of the polygon
	 * */
	@Override
	public String getName() {
		return name;
	}

	/**Sets the name of the polygon to the given value
	 * 
	 * @param name The new name of the polygon
	 * */
	@Override
	public void setName(String name) {
		this.name = name;	
	}

	/**Returns the color of the polygon
	 * 
	 * @return The color of the polygon
	 * */
	@Override
	public String getColor() {
		return color;
	}

	/**Sets the color of the polygon to the given value
	 * 
	 * @param color The new color of the polygon
	 * */
	@Override
	public void setColor(String color) {
		this.color = color;
	}
	
	/**Returns the list of attributes of the polygon
	 * 
	 * @return The list of attributes of the polygon
	 * */
	@Override
	public ArrayList<String> getAttributeList() {
		return attributeList;
	}
}
