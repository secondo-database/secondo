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
import org.gwtopenmaps.openlayers.client.geometry.LinearRing;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;
import com.secondo.webgui.shared.model.Polygon;

/**
 * This class is a controller for the datatype polygon, to display polygons in
 * the map view.
 * 
 * @author Kristina Steiger
 * 
 **/
public class MVPolygonController {
	
	/**Map with IDs of polygons as keys and the layer for polygons as values*/
	private HashMap<Number, Vector> polygonMap = new HashMap<Number, Vector>();

	/**Temporary array for points of the path of an polygon*/
	private ArrayList<Point> polygonPointArray = new ArrayList<Point>();
	
	/**Projection to transform geographical points to fit the map*/
	private Projection externalProjection = new Projection("EPSG:4326");
	private Projection internalProjection = new Projection("EPSG:900913");

	public MVPolygonController() {
	}

	/**Checks if the given number is a geographic latitude
     * 
     * @param lat The latitude value to be checked
     * @return Returns true if the value is a geographic latitude
     * */
	public boolean isLatitude(double lat) {

		// range lat => -90 +90
		if (lat < 90 && lat > -90) {
			return true;
		}
		return false;
	}

	/**Checks if the given number is a geographic longitude
     * 
     * @param lng The longitude value to be checked
     * @return Returns true if the value is a geographic longitude
     * */
	public boolean isLongitude(double lng) {

		// range lng => -180 +180
		if (lng < 180 && lng > -180) {
			return true;
		}
		return false;
	}

	/** Creates a point from the given values and adds it to the pointarray 
	 * 
	 * @param lat The Latitude of the point
     * @param lng The Longitude of the point
     * @param bounds The Bounds object*/
	public void addPolygonPoint(double lat, double lng, Bounds boundsAll, Bounds boundsLast) {

		if (isLatitude(lat) && isLongitude(lng)) {
			Point point = new Point(lng, lat);
			point.transform(externalProjection, internalProjection);

			polygonPointArray.add(point);

			boundsAll.extend(point);
			boundsLast.extend(point);
		}
	}

	/** Deletes all points from the pointarray */
	public void deleteAllPolygonPoints() {
		polygonPointArray.clear();
	}

	/**Creates a linear ring representing a polygon and adds it to the polygonarray 
	 * 
	 * @param key The ID of the polygon to be added
	 * */
	public void addPolygon(Integer key) {

		LinearRing polygon = new LinearRing(polygonPointArray.toArray(new Point[polygonPointArray.size()]));

		// Create a style for the vectorlayer
		Style style = new Style();
		style.setStrokeColor("#0033ff"); //blue
		style.setStrokeWidth(2);
		style.setFillColor("orange");
		style.setFillOpacity(0.3);

		// Create the layer
		Vector polygonLayer = new Vector("Polygon Layer");
		polygonLayer.setIsBaseLayer(false);
		polygonLayer.setDisplayInLayerSwitcher(false);

		VectorFeature feature = new VectorFeature(polygon);
		feature.setStyle(style);
		polygonLayer.addFeature(feature);

		polygonMap.put(key, polygonLayer);
	}

	/** Draws all polygons of the polygonarray to the map 
	 * 
	 * @param map The Map object
     * @param bounds The bounds object
     * */
	public void showPolygonOverlays(Map map, Bounds bounds) {

		if (!polygonMap.isEmpty()) {

			// add all polygons to the map
			for (Vector polygon : polygonMap.values()) {
				map.addLayer(polygon);
			}

			map.zoomToExtent(bounds);
			if (map.getZoom() > 10) {
				map.zoomTo(10);
			}
		}
	}

	/** Deletes all polygons from the polygon array */
	public void deleteAllPolygons() {
		polygonMap.clear();
	}

	/** Shows all polygons on the map */
	public void showPolygons() {
		for (Vector polygon : polygonMap.values()) {
			polygon.setIsVisible(true);
		}
	}

	/** Hides all polygons from the map */
	public void hidePolygons() {
		for (Vector polygon : polygonMap.values()) {
			polygon.setIsVisible(false);
		}
	}

	/** Shows the given polygon on the map 
	 * 
	 * @param polygon The polygon to be shown on the map
	 * */
	public void showPolygonObject(Polygon polygon) {

		polygonMap.get(polygon.getId()).setIsVisible(true);
	}

	/** Hides the given polygon from the map 
	 * 
	 * @param polygon The polygon the be hidden from the map
	 * */
	public void hidePolygonObject(Polygon polygon) {

		polygonMap.get(polygon.getId()).setIsVisible(false);
	}

	/** Changes the color of the given polygon id to the given color 
	 * 
	 * @param polygonId The ID of the polygon
     * @param color The new color of the polygon
     * */
	public void changePolygonColor(int polygonId, String color) {

		VectorFeature[] features = polygonMap.get(polygonId).getFeatures();
		for(VectorFeature feature : features){
			feature.getStyle().setFillColor(color);
		}		
		polygonMap.get(polygonId).redraw();
	}

	/**Returns the hashmap with all polygon vector layers
     * 
     * @return The hashmap with all polygon vector layers
     * */
	public HashMap<Number, Vector> getPolygonMap() {
		return polygonMap;
	}

}
