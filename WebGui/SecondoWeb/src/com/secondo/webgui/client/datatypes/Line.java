package com.secondo.webgui.client.datatypes;

import java.io.Serializable;
import java.util.ArrayList;


public class Line implements DataType, SpatialType, Serializable{
	
	private static final long serialVersionUID = 488749984602413533L;
	private String name = "Line";
	private ArrayList<String> text = new ArrayList<String>();
	private ArrayList<Point> location = new ArrayList<Point>();
	private Point a = new Point();
	private Point b = new Point();
	private Rectangle bounds = new Rectangle();
	private String firstTuple = "";
	
	public Line(){}


	@Override
	public String getName() {
		return name;
	}


	@Override
	public ArrayList<Point> getLocation() {
		
		return location;
	}

	@Override
	public void setLocation(ArrayList<Object> location) {
		this.location.add((Point) location.get(0));
		this.location.add((Point) location.get(1));
	}

	@Override
	public void resetLocation() {
		location.clear();
		
	}

	@Override
	public Rectangle getBounds() {

		return bounds;
	}
	
	@Override
	public void setBounds(Rectangle bounds) {
		this.bounds = bounds;
		
	}



	public Point getA() {
		return a;
	}


	public void setA(Point a) {
		this.a = a;
	}


	public Point getB() {
		return b;
	}


	public void setB(Point b) {
		this.b = b;
	}


	@Override
	public ArrayList<String> getTextList() {
		// TODO Auto-generated method stub
		return null;
	}


	@Override
	public void setTextList(ArrayList<String> textlist) {
		// TODO Auto-generated method stub
		
	}


}
