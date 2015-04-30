package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * This class models a polyline element with an individual ID, the type
 * "Polyline", a name, a list of line elements for the path, a default color and
 * a list of attributes. It implements the interface DataType and has to be
 * serializable to be exchanged between client and server.
 * 
 * @author Irina Russkaya
 */
public class Polyline implements DataType, Serializable {

	private static final long serialVersionUID = -8408672105951171395L;
	private int id = 0;
	private String type = "Polyline";
	private String name = "Polyline";
	private ArrayList<Line> path = new ArrayList<Line>();
	private String color = "black";
	private ArrayList<String> attributeList = new ArrayList<String>();

	public Polyline() {
	}

	/**
	 * Adds a line element to the path of the polyline
	 * 
	 * @param line
	 *            The line element to be added to the path
	 * */
	public void addLineToPath(Line line) {
		this.path.add(line);
	}

	/**
	 * Returns the path of the polyline as a list of lines
	 * 
	 * @return The path of the polyline
	 * */
	public ArrayList<Line> getPath() {
		return path;
	}

	/**
	 * Returns the individual ID of the polyline
	 * 
	 * @return The ID of the polyline
	 * */
	@Override
	public int getId() {
		return id;
	}

	/**
	 * Sets the ID of the polyline to the given value
	 * 
	 * @param id
	 *            The new ID of the polyline
	 * */
	@Override
	public void setId(int id) {
		this.id = id;
	}

	/**
	 * Returns the type of the polyline
	 * 
	 * @return The type of the polyline
	 * */
	@Override
	public String getType() {
		return type;
	}

	/**
	 * Returns the name of the polyline
	 * 
	 * @return The name of the polyline
	 * */
	@Override
	public String getName() {
		return name;
	}

	/**
	 * Sets the name of the polyline to the given value
	 * 
	 * @param name
	 *            The new name of the polyline
	 * */
	@Override
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * Returns the color of the polyline
	 * 
	 * @return The color of the polyline
	 * */
	@Override
	public String getColor() {
		return color;
	}

	/**
	 * Sets the color of the polyline to the given value
	 * 
	 * @param color
	 *            The new color of the polyline
	 * */
	@Override
	public void setColor(String color) {
		this.color = color;
	}

	/**
	 * Returns the list of attributes of the polyline
	 * 
	 * @return The list of attributes of the polyline
	 * */
	@Override
	public ArrayList<String> getAttributeList() {
		return attributeList;
	}
}
