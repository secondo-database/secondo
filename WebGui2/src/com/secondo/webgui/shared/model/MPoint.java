package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * This class models a moving point element with an individual ID and a name,
 * the type "MPoint", a list of line elements and a list of timeintervals, a
 * default color and an attributelist. It implements the interface DataType and
 * has to be serializable to be exchanged between client and server.
 * 
 * @author Irina Russkaya
 * 
 */
public class MPoint implements DataType, Serializable {

	private static final long serialVersionUID = -8592716841354310709L;
	private int id = 0;
	private String name = "MPoint";
	private String type = "MPoint";
	private ArrayList<Line> path = new ArrayList<Line>();
	private ArrayList<TimeInterval> time = new ArrayList<TimeInterval>();
	private String color = "red";
	private ArrayList<String> attributeList = new ArrayList<String>();
	private String attributeNameInRelation = "";

	public MPoint() {
	}

	/**
	 * Returns a list of line elements representing the path of the moving point
	 * 
	 * @return The list of line elements
	 * */
	public ArrayList<Line> getPath() {
		return path;
	}

	/**
	 * Sets the path to the given list of line elements
	 * 
	 * @param path
	 *            The new list of line elements
	 * */
	public void setPath(ArrayList<Line> path) {
		this.path = path;
	}

	/**
	 * Returns a list of timeinterval elements representing the time that is
	 * passed when the moving point moves along its path.
	 * 
	 * @return The list of timeinterval elements
	 * */
	public ArrayList<TimeInterval> getTime() {
		return time;
	}

	/**
	 * Sets the list of timeinterval elements to the given list
	 * 
	 * @param time
	 *            The new list of timeinterval elements
	 * */
	public void setTime(ArrayList<TimeInterval> time) {
		this.time = time;
	}

	/**
	 * Returns the individual ID of the moving point
	 * 
	 * @return The ID of the moving point
	 * */
	@Override
	public int getId() {
		return id;
	}

	/**
	 * Sets the ID of the moving point to the given value
	 * 
	 * @param id
	 *            The new ID of the moving point
	 * */
	@Override
	public void setId(int id) {
		this.id = id;
	}

	/**
	 * Returns the type of the moving point
	 * 
	 * @return The type of the moving point
	 * */
	@Override
	public String getType() {
		return type;
	}

	/**
	 * Returns the name of the moving point
	 * 
	 * @return The name of the moving point
	 * */
	@Override
	public String getName() {
		return name;
	}

	/**
	 * Sets the name of the moving point to the given value
	 * 
	 * @param name
	 *            The new name of the moving point
	 * */
	@Override
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * Returns the list of attributes of the moving point
	 * 
	 * @return The list of attributes of the moving point
	 * */
	@Override
	public ArrayList<String> getAttributeList() {
		return attributeList;
	}

	/**
	 * Returns the color of the moving point
	 * 
	 * @return The color of the moving point
	 * */
	@Override
	public String getColor() {
		return color;
	}

	/**
	 * Sets the color of the moving point to the given value
	 * 
	 * @param color
	 *            The new color of the moving point
	 * */
	@Override
	public void setColor(String color) {
		this.color = color;
	}

	/**
	 * Sets the attribute name of the type Mpoint in relation
	 * 
	 * @param The
	 *            attributeNameInRelation
	 */
	public void setAttributeNameInRelation(String attributeNameInRelation) {
		this.attributeNameInRelation = attributeNameInRelation;
	}

	/**
	 * Returns the attribute name of the type Mpoint in relation
	 * 
	 * @return The attribute name of the type Mpoint in relation
	 */
	public String getAttributeNameInRelation() {
		return attributeNameInRelation;
	}
}
