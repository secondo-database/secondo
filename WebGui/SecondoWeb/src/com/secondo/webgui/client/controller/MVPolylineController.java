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
import java.util.HashMap;
import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.Style;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.geometry.LineString;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;

import com.secondo.webgui.shared.model.Polyline;

/**
*  This class is a controller for the datatype polyline, to display polylines in the map view.
*  
*  @author Kristina Steiger
*  
**/
public class MVPolylineController {
	
	/**Map with IDs of polylines as keys and the layer for polylines as values*/
	private HashMap<Number, Vector> polylineMap = new HashMap<Number, Vector>();
	
	/**Temporary array for lines of a polyline*/
	private ArrayList<LineString> lineArray = new ArrayList<LineString>();
	
	/**Projection to transform geographical points to fit the map*/
	private Projection externalProjection = new Projection("EPSG:4326");
	private Projection internalProjection = new Projection("EPSG:900913");
	
	public MVPolylineController(){
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
	
    /**Creates a lineString representing a line and adds it to the lineArray
     * 
     * @param lat1 The Latitude of the point x1
     * @param lng1 The Longitude of the point y1
     * @param lat2 The Latitude of the point x2
     * @param lng2 The Longitude of the point y2
     * @param bounds The Bounds object
     * */
    public void addLine(double lat1, double lng1, double lat2, double lng2, Bounds bounds){
    	
    	if(isLatitude(lat1) && isLongitude(lng1)){              
       // create one point 
          Point point1 = new Point(lng1, lat1);
          point1.transform(externalProjection, internalProjection);
       
          if(isLatitude(lat2) && isLongitude(lng2)){
          // create another point
             Point point2 = new Point(lng2, lat2);
             point2.transform(externalProjection, internalProjection);

             Point [] points = {point1, point2};
             LineString lineString = new LineString(points);
             lineArray.add(lineString);
        
             bounds.extend(point1);
             bounds.extend(point2);
          }
       }
    }
    
    /**Creates a layer for a polyline and adds it to the polyline Map
     * 
     * @param key ID of the polyline to be added
     * */
    public void addPolyline(int key){
             
        	 //Create a style for the vectorlayer
             Style style = new Style();
             style.setStrokeColor("#0033ff");
             style.setStrokeWidth(2);
             
            // Create the vector layer
             Vector polylineLayer = new Vector("Polyline Overlay");
             polylineLayer.setIsBaseLayer(false);
             polylineLayer.setDisplayInLayerSwitcher(false);
             
             for(LineString lineString : lineArray){
            	 VectorFeature feature = new VectorFeature(lineString);
                 polylineLayer.addFeature(feature); 
                 feature.setStyle(style);
             }
             polylineMap.put(key, polylineLayer);
    }
    
    /**Shows all polyline layers in the array on the map
     * 
     * @param map The Map object
     * @param bounds The bounds object
     * */
    public void showPolylineOverlays(Map map, Bounds bounds){
    	
    	if(!polylineMap.isEmpty()){ 
    		
    		for(Vector lineLayer : polylineMap.values()){
    			map.addLayer(lineLayer);
    		}	
        
           map.zoomToExtent(bounds); 
           //zoom not closer than 10
           if(map.getZoom() > 10){ 
    			map.zoomTo(10);
    		}
    	}
    }
    
    /**Deletes all lines from the array*/
    public void deleteAllLines(){
    	lineArray.clear();
    }
    
    /**Deletes all polylines from the array*/
    public void deleteAllPolylines(){
    	polylineMap.clear();
    }
	
    /**Shows all polylines on the map*/
    public void showPolylines(){
   	 for(Vector polyline : polylineMap.values()){
   		 polyline.setIsVisible(true);
   	 }
    }
    
    /**Hides all polylines from the map*/
    public void hidePolylines(){
   	 for(Vector polyline : polylineMap.values()){
   		 polyline.setIsVisible(false);
   	 }
    }
    
    /**Shows the given polyline object on the map
     * 
     * @param polyline The polyline to be shown on the map
     * */
    public void showPolylineObject(Polyline polyline){
   	 polylineMap.get(polyline.getId()).setIsVisible(true);     
    }
    
    /**Hides the given polyline object from the map
     * 
     * @param polyline The polyline the be hidden from the map
     * */
    public void hidePolylineObject(Polyline polyline){
   	 polylineMap.get(polyline.getId()).setIsVisible(false); 
    }
    
	/** Changes the color of the given polyline id to the given color
	 * 
	 * @param polylineId The ID of the polyline
     * @param color The new color of the polyline
     * */
	public void changePolylineColor(int polylineId, String color) {

		VectorFeature[] features = polylineMap.get(polylineId).getFeatures();
		for(VectorFeature feature : features){
			feature.getStyle().setStrokeColor(color);
		}		
		polylineMap.get(polylineId).redraw();
	}

	/**Returns the hashmap with all polyline vector layers
     * 
     * @return The hashmap with all polyline vector layers
     * */
	public HashMap<Number, Vector> getPolylineMap() {
		return polylineMap;
	}
}
