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

import java.util.HashMap;
import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.Style;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;

/**
*  This class is a controller for the datatype point, to display points in the map view.
*  
*  @author Kristina Steiger
*  
**/
public class MVPointController {
	
	/**Map with IDs of points as keys and the layer for points as values*/
	private HashMap<Number, Vector> pointMap = new HashMap<Number, Vector>();
	
	/**Projection to transform geographical points to fit the map*/
	private Projection externalProjection = new Projection("EPSG:4326");
	private Projection internalProjection = new Projection("EPSG:900913");
	
	public MVPointController(){		
	}
	
	/**Checks if the given number is a geographic latitude
     * 
     * @param lat The latitude value to be checked
     * @return Returns true if the value is a geographic latitude
     * */
    public boolean isLatitude(double lat){
   	 
   	 //range lat => -90 +90
   	 if(lat < 90 && lat > -90){
   		 return true; 		 
   	 }	 
   	 return false;
    }
    
    /**Checks if the given number is a geographic longitude
     * 
     * @param lng The longitude value to be checked
     * @return Returns true if the value is a geographic longitude
     * */
    public boolean isLongitude(double lng){ 	 
   	 
   	 //range lng => -180 +180
   	 if(lng < 180 && lng > -180){
   		 return  true;
   	 } 	 
   	 return false;
    }  
	
    /**Creates a point element from the given values and adds it to the pointmap
     * 
     * @param lat The Latitude of the point
     * @param lng The Longitude of the point
     * @param key The ID of the point to be added
     * @param bounds The Bounds object
     * */
    public void addPoint(double lat, double lng, int key, Bounds boundsAll, Bounds boundsLast){
    	
    	if(isLatitude(lat) && isLongitude(lng)){
        	
        	Point point = new Point(lng, lat);
            point.transform(externalProjection, internalProjection);
            
            boundsAll.extend(point);
            boundsLast.extend(point);
            
            //Create a style for the vectorlayer
            Style style = new Style();
            style.setStrokeColor("#FF0000"); //red
            style.setStrokeWidth(2);;
            style.setPointRadius(6);
            style.setFillColor("#0000FF");//blue
            style.setFillOpacity(1);
            
            Vector pointLayer = new Vector("Marker Overlay");
            pointLayer.setIsBaseLayer(false);
            pointLayer.setDisplayInLayerSwitcher(false);
            
            VectorFeature feature = new VectorFeature(point);
            feature.setStyle(style);
            pointLayer.addFeature(feature); 

            pointMap.put(key, pointLayer);
    	}	
    }
    
    /**Shows all points of the point hashmap on the map and zooms to bounds
     * 
     * @param map The Map object
     * @param bounds The bounds object
     * */
    public void showPointOverlays(Map map, Bounds bounds){
    	
    	if(!pointMap.isEmpty()){
    		
    		for(Vector pointLayer: pointMap.values()){
    			map.addLayer(pointLayer);
    		}

			map.zoomToExtent(bounds); 
				if(map.getZoom() > 10){ //zoom not closer than 10
					map.zoomTo(10);
				}
    	}
    }
    
    /**Deletes all points from the pointmap*/
    public void deleteAllPoints(){
    	pointMap.clear();
    }
	
	/**Shows all points on the map*/
    public void showPoints(){
   	 for(Vector point : pointMap.values()){
   		 point.setIsVisible(true);
   	 }
    }
    
    /**Hides all points from the map*/
    public void hidePoints(){   	 
   	 for(Vector point : pointMap.values()){
   		 point.setIsVisible(false);
   	  }
    }

    /**Shows the given point on the map
     * 
     * @param point The point to be shown on the map
     * */
    public void showPointObject(com.secondo.webgui.shared.model.Point point){ 	
   	 pointMap.get(point.getId()).setIsVisible(true);  
    }
    
    /**Hides the given point from the map
     * 
     * @param point The point to be hidden from the map
     * */
    public void hidePointObject(com.secondo.webgui.shared.model.Point point){
   	 pointMap.get(point.getId()).setIsVisible(false); 
    }
    
    /**Changes the color of the given point id to the given color 
     * 
     * @param pointId The ID of the point
     * @param color The new color of the point
     * */
	public void changePointColor(int pointId, String color) {

		VectorFeature[] features = pointMap.get(pointId).getFeatures();
		for(VectorFeature feature : features){
			feature.getStyle().setFillColor(color);
		}		
		pointMap.get(pointId).redraw();
	}

	/**Returns the hashmap with all point vector layers
     * 
     * @return The hashmap with all point vector layers
     * */
	public HashMap<Number, Vector> getPointMap() {
		return pointMap;
	}
}
