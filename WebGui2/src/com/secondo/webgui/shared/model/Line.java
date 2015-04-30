package com.secondo.webgui.shared.model;

import java.io.Serializable;

/**
 * This class models a line element with 2 points and the type "Line". A line is
 * always part of a polyline or a moving point. Therefore it does not implement
 * the DataType interface but it has to be serializable to be exchanged between
 * client and server.
 * 
 * @author Irina Russkaya
 * 
 */
public class Line implements Serializable {

	private static final long serialVersionUID = -7668694383184433149L;
	private String type = "Line";
	private Point pointA = new Point();
	private Point pointB = new Point();

	public Line() {
	}

	/**
	 * Returns the type of the datatype
	 * 
	 * @return The type of the datatype
	 * */
	public String getType() {
		return type;
	}

	/**
	 * Returns the first point of the line
	 * 
	 * @return The first point of the line
	 * */
	public Point getPointA() {
		return pointA;
	}

	/**
	 * Sets the first point of the line to the given point object
	 * 
	 * @param The
	 *            point object
	 * */
	public void setPointA(Point pointA) {
		this.pointA = pointA;
	}

	/**
	 * Returns the second point of the line
	 * 
	 * @return The second point of the line
	 * */
	public Point getPointB() {
		return pointB;
	}

	/**
	 * Sets the second point of the line to the given point object
	 * 
	 * @param The
	 *            point object
	 * */
	public void setPointB(Point pointB) {
		this.pointB = pointB;
	}
}
