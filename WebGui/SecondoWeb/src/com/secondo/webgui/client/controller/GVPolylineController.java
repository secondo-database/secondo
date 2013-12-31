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
import com.secondo.webgui.shared.model.Polyline;

/**
*  This class is a controller for the datatype polyline, to display polylines in the graphical view.
*  It defines JSNI Functions that are defined in the JavaScript-File d3-geodata.js.
*  
*  @author Kristina Steiger
*  
**/
public class GVPolylineController {
	
	/**Contains all polylines from the last secondo queries*/
    private ArrayList<Polyline> polylineArray = new ArrayList<Polyline>();
	
	public GVPolylineController(){		
	}
	
	/**Draws all polylines from the array to the svg*/
	public void drawAllPolylines(){
		for(Polyline polyline : polylineArray){
			
			int indexPolyline = polylineArray.indexOf(polyline);
			
			showLineArray(polyline.getId(), indexPolyline, polyline.getColor());
		}
	}
	
	/**Shows the given line object in the graphical view
	 * 
	 * @param polyline The Polyline to be shown
	 * */
    public void showPolylineObject(Polyline polyline){
    	
    	int indexPolyline = polylineArray.indexOf(polyline);
    	
    	showLineArray(polyline.getId(), indexPolyline, polyline.getColor());
  
    }
    
    /**Returns the array with all polylines
	 * 
	 * @return The list of all polyline objects
	 * */
	public ArrayList<Polyline> getPolylineArray() {
		return polylineArray;
	}
    
/*########### JSNI JavaScript Functions #########
 *************************************************/

	/** Check if any lines are in the array*/
	public native boolean hasLines()/*-{		
		return $wnd.hasLines();
			}-*/;	
	
	/** Check if any polylines are in the array*/
	public native boolean hasPolylines()/*-{		
		return $wnd.hasPolylines();
			}-*/;
	
	/** Add a line to the dataset to display on the graphical view*/
	public native void addLineToDataset(double x1, double y1, double x2, double y2)/*-{
		$wnd.addLineToDataset(x1, y1, x2, y2);
			}-*/;
	
	/** Add all lines from the linearray to the dataset of polylines*/
	public native void addPolyline()/*-{	
		$wnd.addPolyline();
			}-*/;
	
	/**Show the polyline with the given id on the graphical view*/
	public native void showPolyline(int id)/*-{	
		$wnd.showPolyline(id);
			}-*/;
	
	/**Hide the polyline with the given id from the graphical view*/
	public native void hidePolyline(int id)/*-{	
		$wnd.hidePolyline(id);
			}-*/;
	
	/**Draw all lines for one polyline from the linearray to the svg*/
	public native void showLineArray(int id, int index, String color)/*-{		
		$wnd.showLineArray(id, index, color);
			}-*/;
	
	/**Delete all data from the dataset of lines*/
	public native void deleteAllLines()/*-{
		$wnd.deleteAllLines();
			}-*/;
	
	/**Delete all polylines from the dataset*/
	public native void deleteAllPolylines()/*-{
		$wnd.deleteAllPolylines();
			}-*/;
	
	/**Remove all lines from the view*/
	public native void removeLines()/*-{
		$wnd.removeLines();
			}-*/;
	
	/**Remove the polyline with the given id */
	public native void removePolyline(int id)/*-{
		$wnd.removePolyline(id);
			}-*/;

}
