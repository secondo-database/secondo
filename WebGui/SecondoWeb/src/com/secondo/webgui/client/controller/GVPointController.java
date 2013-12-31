//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package com.secondo.webgui.client.controller;

import java.util.ArrayList;

import com.secondo.webgui.shared.model.Point;

/**
*  This class is a controller for the datatype point, to display points in the graphical view.
*  It defines JSNI Functions that are defined in the JavaScript-File d3-geodata.js.
*  
*  @author Kristina Steiger
*  
**/
public class GVPointController {
	
	/**Contains all points from the last secondo queries*/
    private ArrayList<Point> pointArray = new ArrayList<Point>();
	
	public GVPointController(){		
	}
	
	/**Draws all points from the pointArray to the svg*/
	public void drawAllPoints(){
		for(Point point : pointArray){
			
			int indexPoint = pointArray.indexOf(point);
			
			drawPoint(point.getId(), indexPoint, point.getName(), point.getColor());
		}
	}
	
	/**Shows the given point object in the view
	 *  
	 * @param point The point to be shown
	 * */
    public void showPointObject(Point point){
    	
    	int indexPoint = pointArray.indexOf(point);
    	
    	drawPoint(point.getId(), indexPoint, point.getName(), point.getColor());
  
    }
	
	/**Returns the array with all points
	 * 
	 * @return The list of all point objects
	 * */
	public ArrayList<Point> getPointArray() {
		return pointArray;
	}
	
	
/*########### JSNI JavaScript Functions #########
*************************************************/

	/** Check if any points are in the array*/
	public native boolean hasPoints()/*-{
		
		return $wnd.hasPoints();
			}-*/;
	
	/** Add a point to the dataset of points to display on the graphical view*/
	public native void addPointToDataset(double x, double y, String text, int id, String color)/*-{
		$wnd.addPointToDataset(x, y, text, id, color);
			}-*/;
	
	/**Show the point with the given id on the graphical view*/
	public native void showPoint(int id, String color)/*-{	
		$wnd.showPoint(id, color);
			}-*/;
	
	/**Hide the point with the given id from the graphical view*/
	public native void hidePoint(int id)/*-{	
		$wnd.hidePoint(id);
			}-*/;
	
	/**Draw the point with the given id on the graphical view*/
	public native void drawPoint(int id, int index, String name, String color)/*-{	
		$wnd.drawPoint(id, index, name, color);
			}-*/;
	

	/** Get the data from the dataset, scale the points to the size of the svg and draw them as circles*/
	public native void showPointArray()/*-{	
		$wnd.showPointArray();
			}-*/;
	
	/**Remove the point with the given id from the graphical view*/
	public native void removePoint(int id)/*-{	
		$wnd.removePoint(id);
			}-*/;

	/** Delete all data from the dataset of points*/
	public native void deleteAllPoints()/*-{
		$wnd.deleteAllPoints();
			}-*/;
	
	/** Remove all circles from the view*/
	public native void removeCircles()/*-{
		$wnd.removeCircles();
			}-*/;
}
