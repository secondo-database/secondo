package com.secondo.webgui.client.datatypes;

import java.io.Serializable;
import java.util.ArrayList;


public class Polygon implements DataType, SpatialType, Serializable{
	
	private String name = "Polygon";
	private ArrayList<Point> path = new ArrayList<Point>();
	
	public Polygon(){}
	

	@Override
	public String getName() {
		return name;
	}

	@Override
	public ArrayList<Point> getLocation() {
		return path;
	}

	@Override
	public void setLocation(ArrayList<Object> location) {
		this.path.add((Point) location.get(0));
		this.path.add((Point) location.get(1));
		
	}
	
	public void addPointToPath(Point point){
		this.path.add(point);		
	}


	public ArrayList<Point> getPath() {
		return path;
	}

	public void setPath(ArrayList<Point> path) {
		this.path = path;
	}

	@Override
	public Rectangle getBounds() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void setBounds(Rectangle bounds) {
		// TODO Auto-generated method stub
		
	}


	@Override
	public void resetLocation() {
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
