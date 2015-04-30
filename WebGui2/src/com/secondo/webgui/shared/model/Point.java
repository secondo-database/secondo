package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * This class models a point element with an individual ID, the type "Point", a
 * name, a x and y value, a default color and a list of attributes. It
 * implements the interface DataType and has to be serializable to be exchanged
 * between client and server.
 * 
 * @author Irina Russkaya
 * 
 */
public class Point implements DataType, Serializable {

	private static final long serialVersionUID = 1368755481902782244L;
	private int id = 0;
	private String type = "Point";
	private String name = "Point";
	private double x = 0.0;
	private double y = 0.0;
	private String color = "blue";
	private ArrayList<String> attributeList = new ArrayList<String>();

	public Point() {
	}

	/**
	 * Returns the x value of the point
	 * 
	 * @return The x value of the point
	 * */
	public double getX() {
		return x;
	}

	/**
	 * Sets the x value of the point to the given double value
	 * 
	 * @param x
	 *            The double value for x
	 * */
	public void setX(double x) {
		this.x = x;
	}

	/**
	 * Returns the y value of the point
	 * 
	 * @return The y value of the point
	 * */
	public double getY() {
		return y;
	}

	/**
	 * Sets the y value of the point to the given double value
	 * 
	 * @param y
	 *            The double value for y
	 * */
	public void setY(double y) {
		this.y = y;
	}

	/**
	 * Returns the individual ID of the point
	 * 
	 * @return The ID of the point
	 * */
	@Override
	public int getId() {
		return id;
	}

	/**
	 * Sets the ID of the point to the given value
	 * 
	 * @param id
	 *            The new ID of the point
	 * */
	@Override
	public void setId(int id) {
		this.id = id;
	}

	/**
	 * Returns the type of the point
	 * 
	 * @return The type of the point
	 * */
	@Override
	public String getType() {
		return type;
	}

	/**
	 * Returns the name of the point
	 * 
	 * @return The name of the point
	 * */
	@Override
	public String getName() {
		return name;
	}

	/**
	 * Sets the name of the point to the given value
	 * 
	 * @param name
	 *            The new name of the point
	 * */
	@Override
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * Returns the color of the point
	 * 
	 * @return The color of the point
	 * */
	@Override
	public String getColor() {
		return color;
	}

	/**
	 * Sets the color of the point to the given value
	 * 
	 * @param color
	 *            The new color of the point
	 * */
	@Override
	public void setColor(String color) {
		this.color = color;
	}

	/**
	 * Returns the list of attributes of the point
	 * 
	 * @return The list of attributes of the point
	 * */
	@Override
	public ArrayList<String> getAttributeList() {
		return attributeList;
	}
}
