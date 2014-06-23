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

import java.util.ArrayList;

/**This interface must be implemented to create an new datatype.
 * 
 *  @author Kristina Steiger
 *  
 **/
public interface DataType {
	
	/**Returns the id of the datatype*/
	public int getId();
	
	/**Sets the id of the datatype to the given integer*/
	public void setId(int id);
	
	/** Returns the type of the datatype*/
	public String getType();
	
	/** Returns the name of the datatype*/
	public String getName();
	
	/** Sets the name of the datatype to the given string*/
	public void setName(String name);
	
	/** Returns the color of the datatype*/
	public String getColor();
	
	/** Sets the color of the datatype to the given string*/
	public void setColor(String color);
	
	/**Returns the list of attributes of the datatype*/
	public ArrayList<String> getAttributeList();

}
