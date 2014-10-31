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

package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;

import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.LonLat;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.MapOptions;
import org.gwtopenmaps.openlayers.client.MapWidget;
import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.RenderIntent;
import org.gwtopenmaps.openlayers.client.control.LayerSwitcher;
import org.gwtopenmaps.openlayers.client.control.MousePosition;
import org.gwtopenmaps.openlayers.client.control.OverviewMap;
import org.gwtopenmaps.openlayers.client.control.ScaleLine;
import org.gwtopenmaps.openlayers.client.control.SelectFeature;
import org.gwtopenmaps.openlayers.client.control.SelectFeatureOptions;
import org.gwtopenmaps.openlayers.client.event.FeatureHighlightedListener;
import org.gwtopenmaps.openlayers.client.event.FeatureUnhighlightedListener;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3MapType;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3Options;
import org.gwtopenmaps.openlayers.client.layer.OSM;
import org.gwtopenmaps.openlayers.client.layer.Vector;
import org.gwtopenmaps.openlayers.client.Style;

import com.google.gwt.core.client.Callback;
import com.google.gwt.geolocation.client.Geolocation;
import com.google.gwt.geolocation.client.Position;
import com.google.gwt.geolocation.client.PositionError;

import org.gwtopenmaps.openlayers.client.geometry.Point;

import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.secondo.webgui.client.controller.MVMLabelController;
import com.secondo.webgui.client.controller.MVMPointController;
import com.secondo.webgui.client.controller.MVPointController;
import com.secondo.webgui.client.controller.MVPolygonController;
import com.secondo.webgui.client.controller.MVPolylineController;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MLabel;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.Polygon;
import com.secondo.webgui.shared.model.Polyline;
import com.secondo.webgui.shared.model.TimeInterval;
import com.secondo.webgui.utils.config.ModesToShowSymTraj;

/**
 * This class represents all elements of the map view to display graphical
 * datatypes on a map with the gwt-openlayers library.
 * 
 * @author Kristina Steiger
 * 
 **/
public class MapView extends Composite implements View {

	/** The Main panel of the map view */
	private FlowPanel contentPanel = new FlowPanel();

	/** The Map element */
	private Map map;

	/** The MapWidget containing the map */
	private MapWidget mapWidget;

	/** The Bounds object containing all geographic bound points of all queries */
	private Bounds boundsAll = new Bounds();

	/**
	 * The Bounds object containing all geographic bound points of the last
	 * query
	 */
	private Bounds boundsLast = new Bounds();

	/** Controller for point objects */
	private MVPointController pointController = new MVPointController();

	/** Controller for polyline objects */
	private MVPolylineController polylineController = new MVPolylineController();

	/** Controller for polygon objects */
	private MVPolygonController polygonController = new MVPolygonController();

	/** Controller for moving point objects */
	private MVMPointController mpointController = new MVMPointController();

	/** Controller for moving label objects */
	private MVMLabelController mlabelController = new MVMLabelController();

	/** Width of the map view */
	private int width = Window.getClientWidth() - 293;

	/** Height of the map view */
	private int height = Window.getClientHeight() - 321;

	/** Resultlist for current secondo query result */
	private ArrayList<DataType> currentResultTypeList = new ArrayList<DataType>();

	/** Value is true if data has finished loading */
	private boolean dataLoaded = false;
	private boolean dataInitialized = false;

	private boolean zoomToAll = false;

	private int modeForSymTraj;

	private static final Projection DEFAULT_PROJECTION = new Projection(
			"EPSG:4326");

	private LegendDialog legend = new LegendDialog();
	
	private String attributeNameOfMPoint;
	
	

	public MapView() {

		contentPanel.getElement().setClassName("mapcontentpanel");

		// create a mapwidget and add it to the contentpanel
		MapOptions defaultMapOptions = new MapOptions();
		// TODO
		// mapWidget = new MapWidget(width + "px", height + "px",
		// defaultMapOptions);
		mapWidget = new MapWidget("500px", "500px", defaultMapOptions);
		mapWidget.getElement().setClassName("mapwidget");

		contentPanel.add(mapWidget);

		map = mapWidget.getMap();

		initOsmMap();
		initGoogleLayers();
	}

	/**
	 * Initializes the map with Open Street Map layers
	 * 
	 * @param lat
	 *            The latitude of the default center point
	 * @param lng
	 *            The longitude of the default center point
	 * */
	public void initOsmMap() {

		LonLat lonLat = new LonLat(51.3760448, 7.4947253);
		lonLat.transform("EPSG:4326", "EPSG:900913");

		map = mapWidget.getMap();

		OSM osm_1 = OSM.Mapnik("Open Street Map Mapnik "); // Label for menu
															// 'LayerSwitcher'
		osm_1.setIsBaseLayer(true);

		OSM osm_2 = OSM.CycleMap("Open Street Map CycleMap ");

		map.addLayer(osm_1);
		map.addLayer(osm_2);

		// Add some default controls to the map
		map.addControl(new MousePosition()); // shows the coordinates of the
												// mouseposition in the
												// lowerright corner
		map.addControl(new LayerSwitcher()); // + sign in the upperright corner
												// to display the layer switcher
		map.addControl(new OverviewMap()); // + sign in the lowerright to
											// display the overviewmap
		map.addControl(new ScaleLine()); // Display the scaleline

		map.setCenter(lonLat, 10);

		// initGeoLocation();

		// force the map to fall behind popups
		mapWidget.getElement().getFirstChildElement().getStyle().setZIndex(0);

	}

	/**
	 * Initializes the map centered on the location of the user; if location is
	 * not supported from the browser, centered on Hagen
	 * */
	private void initGeoLocation() {

		// Create a marker layer to the current location marker
		final Vector markerLayer = new Vector("Marker layer");
		map.addLayer(markerLayer);

		// Start GeoLocation stuff (note that the GeoLocation is just plain GWT
		// stuff)
		Geolocation geoLocation = Geolocation.getIfSupported();

		if (geoLocation == null) {
			Window.alert("No GeoLocation support available in this browser :-(");
		} else {
			final Geolocation.PositionOptions geoOptions = new Geolocation.PositionOptions();
			geoOptions.setHighAccuracyEnabled(true);

			geoLocation.watchPosition(new Callback<Position, PositionError>() {
				public void onFailure(final PositionError reason) {
					Window.alert("Something went wrong fetching the geolocation:\n"
							+ reason);

					LonLat lonLat = new LonLat(51.3760448, 7.4947253);
					lonLat.transform("EPSG:4326", "EPSG:900913");
					map.setCenter(lonLat, 18);

				}

				public void onSuccess(final Position result) {
					// put the received result in an openlayers LonLat
					// object
					final LonLat lonLat = new LonLat(result.getCoordinates()
							.getLongitude(), result.getCoordinates()
							.getLatitude());
					lonLat.transform("EPSG:4326", "EPSG:900913"); // transform
																	// lonlat to
																	// OSM
																	// coordinate
																	// system
					// Center the map on the received location
					map.setCenter(lonLat, 18);

					// lets create a vector point on the location
					Style pointStyle = new Style();
					pointStyle.setFillColor("red");
					pointStyle.setStrokeColor("green");
					pointStyle.setStrokeWidth(2);
					pointStyle.setFillOpacity(0.9);

					final Point point = new Point(result.getCoordinates()
							.getLongitude(), result.getCoordinates()
							.getLatitude());
					point.transform(DEFAULT_PROJECTION, new Projection(
							"EPSG:900913")); // transform point to OSM
												// coordinate system
					final VectorFeature pointFeature = new VectorFeature(point,
							pointStyle);
					markerLayer.destroyFeatures();
					markerLayer.addFeature(pointFeature);
				}
			}, geoOptions);
		}

	}

	/** Adds various layers of googlemaps to the map */
	public void initGoogleLayers() {

		// Create some Google Layers
		GoogleV3Options gHybridOptions = new GoogleV3Options();
		gHybridOptions.setIsBaseLayer(true);
		gHybridOptions.setType(GoogleV3MapType.G_HYBRID_MAP);
		GoogleV3 gHybrid = new GoogleV3("Google Hybrid", gHybridOptions);

		GoogleV3Options gNormalOptions = new GoogleV3Options();
		gNormalOptions.setIsBaseLayer(true);
		gNormalOptions.setType(GoogleV3MapType.G_NORMAL_MAP);
		GoogleV3 gNormal = new GoogleV3("Google Normal", gNormalOptions);

		GoogleV3Options gSatelliteOptions = new GoogleV3Options();
		gSatelliteOptions.setIsBaseLayer(true);
		gSatelliteOptions.setType(GoogleV3MapType.G_SATELLITE_MAP);
		GoogleV3 gSatellite = new GoogleV3("Google Satellite",
				gSatelliteOptions);

		GoogleV3Options gTerrainOptions = new GoogleV3Options();
		gTerrainOptions.setIsBaseLayer(true);
		gTerrainOptions.setType(GoogleV3MapType.G_TERRAIN_MAP);
		GoogleV3 gTerrain = new GoogleV3("Google Terrain", gTerrainOptions);

		// Add google layers to the map
		Map map = mapWidget.getMap();
		map.addLayer(gHybrid);
		map.addLayer(gNormal);
		map.addLayer(gSatellite);
		map.addLayer(gTerrain);
	}

	/**
	 * On resizing of the browser window the elements of the map view are
	 * readjusted with the commandpanel displayed
	 * 
	 * @param width
	 *            The new width of the map view
	 * @param height
	 *            The new height of the map view
	 * */
	@Override
	public void resizeWithCP(int width, int height) {

		// add 3 pixel for borders, 50px for sidebar, 20px for sidepadding, 220
		// for toolbox
		if (width > 1000) {
			this.width = width - 293;
			contentPanel.setWidth(width - 293 + "px");
			mapWidget.setWidth(width - 293 + "px");
		} else {
			this.width = 1000 - 293;
			contentPanel.setWidth(1000 - 293 + "px");
			mapWidget.setWidth(1000 - 293 + "px");
		}
		if (height > 650) {
			contentPanel.setHeight(height - 321 + "px");
			this.height = height - 321;
			mapWidget.setHeight(height - 321 + "px");
		} else {
			contentPanel.setHeight(650 - 321 + "px");
			this.height = 650 - 321;
			mapWidget.setHeight(650 - 321 + "px");
		}
		map.updateSize();
	}

	/**
	 * On resizing of the browser window the elements of the map view are
	 * readjusted with the textpanel displayed
	 * 
	 * @param width
	 *            The new width of the map view
	 * @param height
	 *            The new height of the map view
	 * */
	@Override
	public void resizeWithTextPanel(int width, int height) {

		if (width > 1000) {
			// add 3 pixel for borders, 50px for sidebar, 20px for sidepadding,
			// 220 for toolbox + 300 textpanel
			this.width = width - 593;
			contentPanel.setWidth(width - 593 + "px");
			mapWidget.setWidth(width - 593 + "px");
		} else {
			this.width = 1000 - 593;
			contentPanel.setWidth(1000 - 593 + "px");
			mapWidget.setWidth(1000 - 593 + "px");
		}
		if (height > 650) {
			this.height = height - 91;
			contentPanel.setHeight(height - 91 + "px");
			mapWidget.setHeight(height - 91 + "px");
		} else {
			contentPanel.setHeight(650 - 91 + "px");
			this.height = 650 - 91;
			mapWidget.setHeight(650 - 91 + "px");
		}
		map.updateSize();
	}

	/**
	 * On resizing of the browser window the elements of the map view are
	 * readjusted with the textpanel and commandpanel displayed
	 * 
	 * @param width
	 *            The new width of the map view
	 * @param height
	 *            The new height of the map view
	 * */
	@Override
	public void resizeWithTextAndCP(int width, int height) {

		if (width > 1000) {
			this.width = width - 593;
			contentPanel.setWidth(width - 593 + "px");
			mapWidget.setWidth(width - 593 + "px");
		} else {
			this.width = 1000 - 603;
			contentPanel.setWidth(1000 - 593 + "px");
			mapWidget.setWidth(1000 - 593 + "px");
		}
		if (height > 650) {
			this.height = height - 321;
			contentPanel.setHeight(height - 321 + "px");
			mapWidget.setHeight(height - 321 + "px");
		} else {
			contentPanel.setHeight(650 - 321 + "px");
			this.height = 650 - 321;
			mapWidget.setHeight(650 - 321 + "px");
		}
		map.updateSize();
	}

	/**
	 * On resizing of the browser window the elements of the map view are
	 * readjusted with to fullscreen
	 * 
	 * @param width
	 *            The new width of the map view
	 * @param height
	 *            The new height of the map view
	 * */
	@Override
	public void resizeToFullScreen(int width, int height) {

		if (width > 1000) {
			this.width = width - 293;
			contentPanel.setWidth(width - 293 + "px");
			mapWidget.setWidth(width - 293 + "px");
		} else {
			this.width = 1000 - 293;
			contentPanel.setWidth(1000 - 293 + "px");
			mapWidget.setWidth(1000 - 293 + "px");
		}
		if (height > 650) {
			this.height = height - 91;
			contentPanel.setHeight(height - 91 + "px");
			mapWidget.setHeight(height - 91 + "px");
		} else {
			contentPanel.setHeight(650 - 91 + "px");
			this.height = 650 - 91;
			mapWidget.setHeight(650 - 91 + "px");
		}
		map.updateSize();
	}

	/**
	 * Gets the current secondo result from the list and put it into the
	 * datatype arrays
	 */
	public void initializeOverlays() {

		if (!currentResultTypeList.isEmpty()) {

			dataLoaded = false;
			dataInitialized = false;
			boundsLast = new Bounds();

			// Sorting with mlabel at the end of list
			Collections.sort(currentResultTypeList, new Comparator<DataType>() {

				@Override
				public int compare(DataType arg0, DataType arg1) {
					if (arg0.getType().equalsIgnoreCase("Polyline")) {
						return 1;
					}
					if (arg1.getType().equalsIgnoreCase("Polyline")) {
						return -1;
					}
					if (arg0.getType().equalsIgnoreCase("MLabel")) {
						return 1;
					}
					if (arg1.getType().equalsIgnoreCase("MLabel")) {
						return -1;
					}

					else
						return arg0.getType().compareTo(arg1.getType());

				}

			});

			for (DataType data : currentResultTypeList) {
				System.out.println("Type of data from result list: "
						+ data.getType());

				if (data.getType().equals("Point")) {

					pointController.addPoint(
							((com.secondo.webgui.shared.model.Point) data)
									.getY(),
							((com.secondo.webgui.shared.model.Point) data)
									.getX(), data.getId(), boundsAll,
							boundsLast);
				}

				if (data.getType().equals("Polyline")) {
					
					polylineController.addPolyline((Polyline) data, boundsAll, boundsLast);
									}
				if (data.getType().equals("Polygon")) {

					polygonController.deleteAllPolygonPoints();

					for (com.secondo.webgui.shared.model.Point point : ((Polygon) data)
							.getPath()) {
						polygonController.addPolygonPoint(point.getY(),
								point.getX(), boundsAll, boundsLast);
					}
					polygonController.addPolygon(data.getId());
				}
				if (data.getType().equals("MPoint")) {

					mpointController.addMP((MPoint)data, boundsAll, boundsLast);				
					polylineController.addPolylineFromMP((MPoint) data, boundsAll, boundsLast);
				}
				// add label to mpoint if time intervals equals
				if (data.getType().equals("MLabel")) {
					mlabelController.addMLabel((MLabel)data);
					mpointController.transmitLabelsToMPandCalculateColorsForPolyline((MLabel) data, polylineController);
					attributeNameOfMPoint=((MLabel)data).getAttributeNameInRelation();
//					polylineController.setColorsAndLegend(colors.get(data.getId()), mlabelController
//							.getLegendToMLabel(data.getId()));
					
					
//					if (!mpointController.getMpointArray().isEmpty()) {
//						for (MPoint mpoint : mpointController.getMpointArray()) {
//
//							if (!mpointController
//									.containsLabelListToMpoint(mpoint.getId())) {
//								ArrayList<String> labelList = mlabelController
//										.matchMLtoMPonBaseOfTime((MLabel) data,
//												mpoint);
//
//								if (!labelList.isEmpty()) {
//									mpointController.addLabelListToMpoint(
//											labelList, mpoint.getId());
//								}
//								if (!mlabelController.getColorsForPolyline()
//										.isEmpty()) {
//									polylineController
//											.addColorsFromML(mlabelController
//													.getColorsForPolyline());
//									polylineController
//											.addLegendFromML(mlabelController
//													.getLegendToMLabel(data.getId()));
//
//								}
//							}
//
//						}
//
//					}

					

				}
			}
			dataInitialized = true;
		}
	}

	/** Draws all elements of the map with the new size of the mapwidget */
	@Override
	public void updateView() {

		// Delete all overlays
		this.resetMap();
		dataLoaded = false;

		if (!currentResultTypeList.isEmpty()) {
			System.out.println(currentResultTypeList.size());

			if (!pointController.getPointMap().isEmpty()) {
				if (zoomToAll == true) {
					pointController.showPointOverlays(map, boundsAll);
				} else {
					pointController.showPointOverlays(map, boundsLast);
				}
			}

			if (!polylineController.getPolylineMap().isEmpty()
					|| !polylineController.getColoredPolylineMap().isEmpty()) {
				boolean showWithDifColors = false;
				if (modeForSymTraj == 3) {
					showWithDifColors = true;
					legend.setLegendInfo(polylineController.getCompleteLegendForMap());
				}

				if (zoomToAll == true) {
					polylineController.showPolylineOverlays(map, boundsAll,
							showWithDifColors);
				} else {
					polylineController.showPolylineOverlays(map, boundsLast,
							showWithDifColors);
				}

			}

			if (!polygonController.getPolygonMap().isEmpty()) {
				if (zoomToAll == true) {
					polygonController.showPolygonOverlays(map, boundsAll);
				} else {
					polygonController.showPolygonOverlays(map, boundsLast);
				}
			}

			if (!mpointController.getMpointArray().isEmpty()) {
				mpointController.stopAllAnimations();
				if (zoomToAll == true) {
					mpointController.drawFirstMovingPoint(map, boundsAll);
				} else {
					mpointController.drawFirstMovingPoint(map, boundsLast);
				}
			}			
			
		}
		dataLoaded = true;
	}

	/** Removes all data from the map */
	@Override
	public void resetData() {
		pointController.deleteAllPoints();
		polylineController.deleteAllPolylines();
		polylineController.deleteAllValuesFromMPandML();
		polygonController.deleteAllPolygonPoints();
		polygonController.deleteAllPolygons();
		mpointController.deleteAllMPoints();
		mpointController.resetTimeBounds();
		mpointController.getMpointTimerList().clear();
		mlabelController.getMlabelArray().clear();
		legend.resetLegendInfo();
		this.resetBounds();
		this.resetMap();
	}

	/** Resets bounds by creating a new bounds object */
	public void resetBounds() {
		boundsAll = new Bounds();
		boundsLast = new Bounds();
	}

	/** Resets the map by deleting all overlays from the map */
	public void resetMap() {
		map.removeOverlayLayers();
	}

	/**
	 * Returns the content panel of the map view
	 * 
	 * @return The content panel of the map view
	 * */
	public FlowPanel getContentPanel() {
		return contentPanel;
	}

	/**
	 * Returns the current result list of all datatypes
	 * 
	 * @return The current result list of all datatypes
	 * */
	public ArrayList<DataType> getCurrentResultTypeList() {
		return currentResultTypeList;
	}

	/**
	 * Returns true if the data has finished loading
	 * 
	 * @return True if data is loaded
	 * */
	public boolean isDataLoaded() {
		return dataLoaded;
	}

	/**
	 * Sets the dataloaded attribute to the given value
	 * 
	 * @param dataLoaded
	 *            True if data is loaded, else false
	 * */
	public void setDataLoaded(boolean dataLoaded) {
		this.dataLoaded = dataLoaded;
	}

	/**
	 * Returns true if the data is initialized
	 * 
	 * @return True if data is initialized
	 * */
	public boolean isDataInitialized() {
		return dataInitialized;
	}

	/**
	 * Sets the datainitialized attribute to the given value
	 * 
	 * @param dataInitialized
	 *            True if data is initialized, else false
	 * */
	public void setDataInitialized(boolean dataInitialized) {
		this.dataInitialized = dataInitialized;
	}

	/**
	 * Returns true if the map should zoom to all queries
	 * 
	 * @return True if the map should zoom to all queries
	 * */
	public boolean isZoomToAll() {
		return zoomToAll;
	}

	/**
	 * Sets the zoomToAll attribute to the given value
	 * 
	 * @param zoomToAll
	 *            True if the map should zoom to all queries, else false
	 * */
	public void setZoomToAll(boolean zoomToAll) {
		this.zoomToAll = zoomToAll;
	}

	/**
	 * Returns the map element
	 * 
	 * @return The map element
	 * */
	public Map getMap() {
		return map;
	}

	/**
	 * Returns the point controller of the map view
	 * 
	 * @return The point controller of the map view
	 * */
	public MVPointController getPointController() {
		return pointController;
	}

	/**
	 * Returns the polyline controller of the map view
	 * 
	 * @return The polyline controller of the map view
	 * */
	public MVPolylineController getPolylineController() {
		return polylineController;
	}

	/**
	 * Returns the polygon controller of the map view
	 * 
	 * @return The polygon controller of the map view
	 * */
	public MVPolygonController getPolygonController() {
		return polygonController;
	}

	/**
	 * Returns the moving point controller of the map view
	 * 
	 * @return The moving point controller of the map view
	 * */
	public MVMPointController getMpointController() {
		return mpointController;
	}

	/**
	 * @param modeForSymTraj
	 *            the modeForSymTraj to set
	 */
	public void setModeForSymTraj(int modeForSymTraj) {
		this.modeForSymTraj = modeForSymTraj;
	}

	/**
	 * Resets all temporary objects in controllers
	 */
	public void clearControllers() {
		mpointController.getMpointArray().clear();
		mlabelController.getMlabelArray().clear();
		polylineController.getPolylineMap().clear();
		polylineController.getColoredPolylineMap().clear();
	}

	/**
	 * @return the legend
	 */
	public LegendDialog getLegend() {
		return legend;
	}
	
	/**
	 * @return the attributeNameOfMPoint
	 */
	public String getAttributeNameOfMPoint() {
		return attributeNameOfMPoint;
	}
}