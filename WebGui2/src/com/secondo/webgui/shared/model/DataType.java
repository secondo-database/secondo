package com.secondo.webgui.shared.model;

import java.util.ArrayList;

/**
 * This interface must be implemented to create an new datatype.
 * 
 * @author Irina Russkaya
 * 
 **/
public interface DataType {

	/** Returns the id of the datatype */
	public int getId();

	/** Sets the id of the datatype to the given integer */
	public void setId(int id);

	/** Returns the type of the datatype */
	public String getType();

	/** Returns the name of the datatype */
	public String getName();

	/** Sets the name of the datatype to the given string */
	public void setName(String name);

	/** Returns the color of the datatype */
	public String getColor();

	/** Sets the color of the datatype to the given string */
	public void setColor(String color);

	/** Returns the list of attributes of the datatype */
	public ArrayList<String> getAttributeList();

}
