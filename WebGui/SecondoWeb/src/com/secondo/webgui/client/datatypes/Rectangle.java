package com.secondo.webgui.client.datatypes;

import java.io.Serializable;
import java.util.ArrayList;


public class Rectangle implements DataType, SpatialType, Serializable{
	
	private static final long serialVersionUID = 71527557983106157L;
	private String name = "Rectangle";
	private Point point = new Point();
	private ArrayList<String> text = new ArrayList<String>();
	private ArrayList<Point> location = new ArrayList<Point>(); //list of 4 points
	private int width = 0;
	private int height = 0;
	
	public Rectangle(){}

	public Point getPoint() {
		return point;
	}

	public void setPoint(Point point) {
		this.point = point;
	}

	public int getWidth() {
		return width;
	}

	public void setWidth(int width) {
		this.width = width;
	}

	public int getHeight() {
		return height;
	}

	public void setHeight(int height) {
		this.height = height;
	}

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
		
		this.resetLocation();
		
		if(location.size()==4){
		this.location.add((Point) location.get(0));
		this.location.add((Point) location.get(1));
		this.location.add((Point) location.get(2));
		this.location.add((Point) location.get(3));
		}
	}	

	@Override
	public void resetLocation() {
		this.location.clear();
		
	}

	@Override
	public Rectangle getBounds() {
		return this;
	}

	@Override
	public void setBounds(Rectangle bounds) {
		// TODO Auto-generated method stub
		
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
