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
import java.util.List;

import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.Style;
import org.gwtopenmaps.openlayers.client.control.SelectFeature;
import org.gwtopenmaps.openlayers.client.event.VectorFeatureSelectedListener;
import org.gwtopenmaps.openlayers.client.event.VectorFeatureUnselectedListener;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.geometry.LineString;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;
import org.gwtopenmaps.openlayers.client.popup.FramedCloud;

import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.Polyline;

/**
 * This class is a controller for the datatype polyline, to display polylines in
 * the map view.
 * 
 * @author Irina Russkaya
 * 
 **/
public class MVPolylineController {

	/** Map with IDs of polylines as keys and the layer for polylines as values */
	private HashMap<Number, Vector> polylineMap = new HashMap<Number, Vector>();

	/** Map with IDs of polylines as keys and the layer for polylines as values */
	private HashMap<Number, Vector> coloredPolylineMap = new HashMap<Number, Vector>();

	/** Projection to transform geographical points to fit the map */
	private Projection externalProjection = new Projection("EPSG:4326");
	private Projection internalProjection = new Projection("EPSG:900913");
	/** id - mpoint id; value - list of line segments to this mpoint */
	private HashMap<Number, List<LineString>> listWithlineSegmentsFromMP = new HashMap<Number, List<LineString>>();
	/** key - id of mpoint; value - list of colors to this mpoint */
	private HashMap<Number, List<String>> listWithColorsToMP = new HashMap<Number, List<String>>();
	/** key - id of mp; value - map where key -label, value - color */
	private HashMap<Number, HashMap<String, String>> mpToLegendMap = new HashMap<Number, HashMap<String, String>>();

	private int count = 0;

	private String completeLegendForMap = new String(" ");

	public MVPolylineController() {
	}

	/**
	 * Checks if the given number is a geographic latitude
	 * 
	 * @param lat
	 *            The latitude value to be checked
	 * @return Returns true if the value is a geographic latitude
	 * */
	public boolean isLatitude(double lat) {
		// range lat => -90 +90
		if (lat < 90 && lat > -90) {
			return true;
		}
		return false;
	}

	/**
	 * Checks if the given number is a geographic longitude
	 * 
	 * @param lng
	 *            The longitude value to be checked
	 * @return Returns true if the value is a geographic longitude
	 * */
	public boolean isLongitude(double lng) {
		// range lng => -180 +180
		if (lng < 180 && lng > -180) {
			return true;
		}
		return false;
	}

	/**
	 * Creates a lineString representing a line and adds it to the lineArray
	 * 
	 * @param lat1
	 *            The Latitude of the point x1
	 * @param lng1
	 *            The Longitude of the point y1
	 * @param lat2
	 *            The Latitude of the point x2
	 * @param lng2
	 *            The Longitude of the point y2
	 * @param bounds
	 *            The Bounds object
	 * */
	public void addLineToArray(List<LineString> arrayWithLines, double lat1,
			double lng1, double lat2, double lng2, Bounds boundsAll,
			Bounds boundsLast) {

		if (isLatitude(lat1) && isLongitude(lng1)) {
			// create one point
			Point point1 = new Point(lng1, lat1);
			point1.transform(externalProjection, internalProjection);

			if (isLatitude(lat2) && isLongitude(lng2)) {
				// create another point
				Point point2 = new Point(lng2, lat2);
				point2.transform(externalProjection, internalProjection);

				Point[] points = { point1, point2 };
				LineString lineString = new LineString(points);

				arrayWithLines.add(lineString);

				boundsAll.extend(point1);
				boundsAll.extend(point2);
				boundsLast.extend(point1);
				boundsLast.extend(point2);
			}
		}
	}

	/**
	 * Creates a layer for a polyline and adds it to the polyline Map
	 * 
	 * @param key
	 *            ID of the polyline to be added
	 * */
	@SuppressWarnings("unchecked")
	public void addPolyline(Polyline data, Bounds boundsAll, Bounds boundsLast) {
		List<LineString> arrayWithLines;
		Integer key = 0;

		if (listWithlineSegmentsFromMP.isEmpty()) {
			arrayWithLines = new ArrayList<LineString>();

			for (Line line : ((Polyline) data).getPath()) {
				addLineToArray(arrayWithLines, line.getPointA().getY(), line
						.getPointA().getX(), line.getPointB().getY(), line
						.getPointB().getX(), boundsAll, boundsLast);
			}
		} else {
			System.out.println(" Size of lineArrayFromMP "
					+ listWithlineSegmentsFromMP.size());
			arrayWithLines = (List<LineString>) listWithlineSegmentsFromMP
					.values().toArray()[count];
			key = (Integer) listWithlineSegmentsFromMP.keySet().toArray()[count];
			count++;
		}

		System.out.println("Size of line segments " + arrayWithLines.size());
		if (!listWithColorsToMP.isEmpty()) {

			String legendForColoredPolyline = generateLegend(mpToLegendMap
					.get(key));
			Vector coloredPolylineLayer = createLayerForColoredPolyline(
					arrayWithLines, listWithColorsToMP.get(key), data.getId(),
					legendForColoredPolyline);
			coloredPolylineMap.put(data.getId(), coloredPolylineLayer);
		}
		Vector polylineLayer = createLayerForSimplePolyline(arrayWithLines);
		polylineMap.put(data.getId(), polylineLayer);
	}

	/**
	 * @param data
	 * @param boundsAll
	 * @param boundsLast
	 */
	public void addPolylineFromMP(MPoint data, Bounds boundsAll,
			Bounds boundsLast) {
		count = 0;
		List<LineString> arrayWithLines = new ArrayList<LineString>();

		// add just pointA of all lines, not pointB, because thats
		// the same as pointA from the next element
		for (Line line : data.getPath()) {
			addLineToArray(arrayWithLines, line.getPointA().getY(), line
					.getPointA().getX(), line.getPointB().getY(), line
					.getPointB().getX(), boundsAll, boundsLast);
		}
		addToLineArrayFromMP(data.getId(), arrayWithLines);
	}

	/**
	 * @param lines
	 * @param colorList
	 * @param key
	 * @param legendForColoredPolyline
	 * @return
	 */
	private Vector createLayerForColoredPolyline(List<LineString> lines,
			List<String> colorList, int key, String legendForColoredPolyline) {

		// Create the vector layer
		Vector coloredPolylineLayer = new Vector("Polyline Overlay with Colors");
		coloredPolylineLayer.setIsBaseLayer(false);
		coloredPolylineLayer.setDisplayInLayerSwitcher(false);

		if (colorList != null) {
			System.out.println("Size with colors " + colorList.size());

			for (int i = 0; i < lines.size(); i++) {

				VectorFeature coloredFeature = new VectorFeature(lines.get(i));
				Style styleForColoredPolyline = new Style();
				styleForColoredPolyline.setStrokeWidth(2);
				if (i < colorList.size()) {

					styleForColoredPolyline.setStrokeColor(colorList.get(i));
				} else {
					styleForColoredPolyline.setStrokeColor("#000000");
				}
				coloredPolylineLayer.addFeature(coloredFeature);
				coloredFeature.setStyle(styleForColoredPolyline);
				coloredFeature.setPopup(new FramedCloud(new Integer(key)
						.toString(), coloredFeature.getCenterLonLat(), null,
						legendForColoredPolyline, null, false));
			}
		}
		return coloredPolylineLayer;

	}

	/**
	 * @param lines
	 * @return
	 */
	private Vector createLayerForSimplePolyline(List<LineString> lines) {
		// Create a style for the vectorlayer
		Style style = new Style();
		style.setStrokeColor("#000000");
		style.setStrokeWidth(2);

		// Create the vector layer
		Vector polylineLayer = new Vector("Polyline Overlay");
		polylineLayer.setIsBaseLayer(false);
		polylineLayer.setDisplayInLayerSwitcher(false);

		for (int i = 0; i < lines.size(); i++) {

			VectorFeature feature = new VectorFeature(lines.get(i));
			polylineLayer.addFeature(feature);
			feature.setStyle(style);
		}
		return polylineLayer;
	}

	/**
	 * Shows all polyline layers in the array on the map
	 * 
	 * @param map
	 *            The Map object
	 * @param bounds
	 *            The bounds object
	 * */
	public void showPolylineOverlays(Map map, Bounds bounds,
			boolean showWithColors) {
		if (!showWithColors) {
			addLineLayersToMap(polylineMap, map, bounds);
		} else {
			addPopupToShowLegend(coloredPolylineMap, map, bounds);
		}
	}

	/**
	 * @param lineMap
	 * @param map
	 * @param bounds
	 */
	private void addPopupToShowLegend(HashMap<Number, Vector> lineMap,
			final Map map, Bounds bounds) {

		System.out.println("LineMap " + lineMap.size());
		if (!lineMap.isEmpty()) {
			Vector[] overlays = new Vector[lineMap.size()];
			int arrayIndex = 0;
			for (Vector lineLayer : lineMap.values()) {
				overlays[arrayIndex] = lineLayer;
				// Secondly add a VectorFeatureSelectedListener to the feature
				lineLayer
						.addVectorFeatureSelectedListener(new VectorFeatureSelectedListener() {
							public void onFeatureSelected(
									FeatureSelectedEvent eventObject) {
								// And attach the popup to the map
								map.addPopup(eventObject.getVectorFeature()
										.getPopup());
							}
						});

				// And add a VectorFeatureUnselectedListener which removes the
				// popup.
				lineLayer
						.addVectorFeatureUnselectedListener(new VectorFeatureUnselectedListener() {
							public void onFeatureUnselected(
									FeatureUnselectedEvent eventObject) {
								map.removePopup(eventObject.getVectorFeature()
										.getPopup());

							}
						});
				arrayIndex++;

			}
			map.addLayers(overlays);

			final SelectFeature selectFeature = new SelectFeature(overlays);
			selectFeature.setAutoActivate(true);
			map.addControl(selectFeature);
			map.zoomToExtent(bounds);
		}

	}

	/**
	 * @param map
	 * @param bounds
	 */
	private void addLineLayersToMap(HashMap<Number, Vector> lineMap, Map map,
			Bounds bounds) {
		System.out.println(lineMap.size());
		if (!lineMap.isEmpty()) {

			for (Vector lineLayer : lineMap.values()) {

				map.addLayer(lineLayer);
			}
			map.zoomToExtent(bounds);
		}
	}

	/** Deletes all polylines from the array */
	public void deleteAllPolylines() {
		polylineMap.clear();
	}

	/** Clears temporary data */
	public void deleteAllValuesFromMPandML() {
		listWithlineSegmentsFromMP.clear();
		listWithColorsToMP.clear();
		completeLegendForMap = "";
	}

	/** Shows all polylines on the map */
	public void showPolylines() {
		for (Vector polyline : polylineMap.values()) {
			polyline.setIsVisible(true);
		}
	}

	/** Hides all polylines from the map */
	public void hidePolylines() {
		for (Vector polyline : polylineMap.values()) {
			polyline.setIsVisible(false);
		}
	}

	/**
	 * Shows the given polyline object on the map
	 * 
	 * @param polyline
	 *            The polyline to be shown on the map
	 * */
	public void showPolylineObject(Polyline polyline) {
		polylineMap.get(polyline.getId()).setIsVisible(true);
	}

	/**
	 * Hides the given polyline object from the map
	 * 
	 * @param polyline
	 *            The polyline the be hidden from the map
	 * */
	public void hidePolylineObject(Polyline polyline) {
		polylineMap.get(polyline.getId()).setIsVisible(false);
	}

	/**
	 * Changes the color of the given polyline id to the given color
	 * 
	 * @param polylineId
	 *            The ID of the polyline
	 * @param color
	 *            The new color of the polyline
	 * */
	public void changePolylineColor(int polylineId, String color) {

		VectorFeature[] features = polylineMap.get(polylineId).getFeatures();
		for (VectorFeature feature : features) {
			feature.getStyle().setStrokeColor(color);
		}
		polylineMap.get(polylineId).redraw();
	}

	/**
	 * Returns the hashmap with all polyline vector layers
	 * 
	 * @return The hashmap with all polyline vector layers
	 * */
	public HashMap<Number, Vector> getPolylineMap() {
		return polylineMap;
	}

	/**Adds to the map an mpoint id and list with its line segments
	 * @param key
	 * @param arrayWithLines
	 */
	public void addToLineArrayFromMP(int key, List<LineString> arrayWithLines) {
		listWithlineSegmentsFromMP.put(key, arrayWithLines);
		System.out.println("Size of added lineArray " + arrayWithLines.size());
		System.out.println("Size of lineArrayFromMP "
				+ listWithlineSegmentsFromMP.size());

	}

	/**
	 * Returns a map with polylines IDs and associated layers
	 * 
	 * @return The Map with IDs of polylines as keys and the layer for polylines
	 *         as values
	 */
	public HashMap<Number, Vector> getColoredPolylineMap() {
		return coloredPolylineMap;
	}

	/**Generates legend to display
	 * @param mapWithLabelToColor
	 *            The map binding label name to color in hex
	 * @return The HTML to display as legend
	 */
	private String generateLegend(HashMap<String, String> mapWithLabelToColor) {
		String legend_firstPart = ""
				+ "<table cellspacing=\"0\" cellpadding=\"0\">" + "<tbody>"
				+ "<tr>";
		String legend_lastPart = "" + "</tr>" + "</tbody>" + "</table>";
		String html = "";
		if (mapWithLabelToColor != null) {
			for (String label : mapWithLabelToColor.keySet()) {

				String color = mapWithLabelToColor.get(label);
				System.out.println("label and color for legend " + label
						+ color);
				html = html
						+ "<tr>"
						+ "<td align=\"left\" style=\"vertical-align: top;\">"
						+ "<div style=\"background-color: "
						+ color
						+ "; width: 25px; height: 15px; margin: 5px;\">"
						+ "</div>"
						+ "</td>"
						+ "<td align=\"left\" style=\"vertical-align: top;\">"
						+ "<div style=\"margin: 5px; font-family: Verdana; color:blue;\">"
						+ label + "</div>" + "</td> </tr>";
			}
		}
		completeLegendForMap = completeLegendForMap
				+ "<h>Legend for the trajectory</h> " + html;
		return legend_firstPart + html + legend_lastPart;
	}

	/**Sets to the global map mpoint ID and asociated color list
	 * @param mapFromMP The map with colors  
	 */
	public void setListWithColorsToMP(
			HashMap<Number, ArrayList<String>> mapFromMP) {
		if (!mapFromMP.isEmpty()) {
			for (Number i : mapFromMP.keySet()) {
				listWithColorsToMP.put(i, mapFromMP.get(i));
			}
		}
	}

	/**Adds a new entry to the map with mpoint id and successive labels and colors
	 * @param The mapFromMP
	 */
	public void setMapWithMpToLegend(
			HashMap<Number, HashMap<String, String>> mapFromMP) {
		if (!mapFromMP.isEmpty()) {
			for (Number i : mapFromMP.keySet()) {
				mpToLegendMap.put(i, mapFromMP.get(i));
			}
		}
	}

	/**Returns the legend to display in the dialog 
	 * @return The completeLegendForMap
	 */
	public String getCompleteLegendForMap() {
		return completeLegendForMap;
	}

}
