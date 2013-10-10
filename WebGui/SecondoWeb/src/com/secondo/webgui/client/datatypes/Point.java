package com.secondo.webgui.client.datatypes;

import java.io.Serializable;
import java.util.ArrayList;


public class Point implements DataType, Serializable{
	
	private static final long serialVersionUID = 1368755481902782244L;
	private String name = "Point";
	private double x = 0.0;
	private double y = 0.0;
	private ArrayList<String> textList = new ArrayList<String>();
	
	public Point(){}
	
	@Override
	public String getName() {
		return name;
	}

	public double getX() {
		return x;
	}

	public void setX(double x) {
		this.x = x;
	}

	public double getY() {
		return y;
	}

	public void setY(double y) {
		this.y = y;
	}
	
	public void addTextEntry(String text){
		textList.add(text);
	}

	public ArrayList<String> getTextList() {
		return textList;
	}

	public void setTextList(ArrayList<String> textList) {
		this.textList = textList;
	}
	

	/**
     * Determines whether or not two points are equal. Two instances of
     * Point are equal if the values of their x and y member fields, representing
     * their position in the coordinate space, are the same.
     * @param obj an object to be compared with this <code>Point</code>
     * @return <code>true</code> if the object to be compared is
     *         an instance of <code>Point</code> and has
     *         the same values; <code>false</code> otherwise.
     */
    public boolean equals(Object obj) {
        if (obj instanceof Point) {
            Point pt = (Point)obj;
            return (this.x == pt.x) && (y == pt.y);
        }
        return false;
    }


}
