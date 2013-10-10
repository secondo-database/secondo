package com.secondo.webgui.client.datatypes;

import java.io.Serializable;
import java.util.ArrayList;


public class Circle implements DataType, SpatialType, Serializable{
	
	private String name = "Circle";
	
	public Circle(){}


	@Override
	public String getName() {
		return name;
	}

	@Override
	public Rectangle getBounds() {
		// TODO Auto-generated method stub
		return null;
	}


	@Override
	public ArrayList<Object> getLocation() {
		// TODO Auto-generated method stub
		return null;
	}


	@Override
	public void setLocation(ArrayList<Object> location) {
		// TODO Auto-generated method stub
		
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
