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
import com.secondo.webgui.shared.model.Polygon;

/**
*  This class is a controller for the datatype polygon, to display polygons in the graphical view.
*  It defines JSNI Functions that are defined in the JavaScript-File d3-geodata.js.
*  
*  @author Kristina Steiger
*  
**/
public class GVPolygonController {
	
	/**Contains all polygons from the last secondo queries*/
    private ArrayList<Polygon> polygonArray = new ArrayList<Polygon>();
	
	public GVPolygonController(){
	}
	

	/**Redraws all polygons from the array to the svg*/
	public void redrawAllPolygons(){
		
		deleteAllPolygons();
		
		for(Polygon polygon : polygonArray){			
			
			deleteAllPolygonPoints();
     	   
			for (Point point: polygon.getPath()){	
				addPointToPolygonPath(point.getX(), point.getY());
			}	
			addPolygon(polygon.getName(), polygon.getId(), polygon.getColor());	
		}
		showPolygonArray();
	}
	
	/**Returns the array with all polygons
	 * 
	 * @return The list of all polygon objects
	 * */
	public ArrayList<Polygon> getPolygonArray() {
		return polygonArray;
	}
	
/*########### JSNI JavaScript Functions #########
*************************************************/

	/** Check if any polygons are in the array*/
	public native boolean hasPolygons()/*-{
		
		return $wnd.hasPolygons();
			}-*/;
	
	/** Add a point to the dataset of polygon path points*/
	public native void addPointToPolygonPath(double x, double y)/*-{
		$wnd.addPointToPolygonPath(x, y);
			}-*/;
	
	/** Add a path of points from the pointarray to the dataset of polygons*/
	public native void addPolygon(String name, int id, String color)/*-{		
		$wnd.addPolygon(name, id, color);
			}-*/;
	
	/**Show the polygon with the given id in the graphical view*/
	public native void showPolygon(int id, String color)/*-{	
		$wnd.showPolygon(id, color);
			}-*/;
	
	/**Hide the polygon with the given id from the graphical view*/
	public native void hidePolygon(int id)/*-{	
		$wnd.hidePolygon(id);
			}-*/;
	
	/**Change the color of the polygon with the given id*/
	public native void changePolygonColor(int id, String color)/*-{	
		$wnd.changePolygonColor(id, color);
			}-*/;
	
	/** Get the data from the dataset, scale the polygon to the size of the svg and draw it*/
	public native void drawPolygon(int id, int index, String name, String color)/*-{		
		$wnd.drawPolygon(id, index, name, color);
			}-*/;
	
	/** Get the data from the polygon dataset, scale the paths to the size of the svg and draw them*/
	public native void showPolygonArray()/*-{		
		$wnd.showPolygonArray();
			}-*/;
	
	/** Delete all data from the dataset of polygon points */
	public native void deleteAllPolygonPoints()/*-{
		$wnd.deleteAllPolygonPoints();
			}-*/;		
	
	/** Delete all polygons from the dataset*/
	public native void deleteAllPolygons()/*-{
		$wnd.deleteAllPolygons();
			}-*/;
	
	/** Remove all polygons from the view*/
	public native void removePolygons()/*-{
		$wnd.removePolygons();
			}-*/;

}
