
package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import org.gwtopenmaps.openlayers.client.Bounds;
import org.gwtopenmaps.openlayers.client.LonLat;
import org.gwtopenmaps.openlayers.client.Map;
import org.gwtopenmaps.openlayers.client.MapOptions;
import org.gwtopenmaps.openlayers.client.MapWidget;
import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.control.DrawFeature;
import org.gwtopenmaps.openlayers.client.control.LayerSwitcher;
import org.gwtopenmaps.openlayers.client.control.MousePosition;
import org.gwtopenmaps.openlayers.client.control.OverviewMap;
import org.gwtopenmaps.openlayers.client.control.ScaleLine;
import org.gwtopenmaps.openlayers.client.feature.VectorFeature;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3MapType;
import org.gwtopenmaps.openlayers.client.layer.GoogleV3Options;
import org.gwtopenmaps.openlayers.client.layer.OSM;
import org.gwtopenmaps.openlayers.client.layer.Vector;
import org.gwtopenmaps.openlayers.client.Style;

import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.handler.RegularPolygonHandler;
import org.gwtopenmaps.openlayers.client.handler.RegularPolygonHandlerOptions;

import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.secondo.webgui.client.controller.MVMLabelController;
import com.secondo.webgui.client.controller.MVMPointController;
import com.secondo.webgui.client.controller.MVPolygonController;
import com.secondo.webgui.client.controller.MVPolylineController;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.MLabel;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.Polygon;
import com.secondo.webgui.shared.model.Polyline;

/**
 * This class represents all elements of the map view to display graphical
 * datatypes on a map with the gwt-openlayers library.
 * 
 * @author Irina Russkaya
 * 
 **/
public class MapView extends Composite {

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

	/** Controller for polyline objects */
	private MVPolylineController polylineController = new MVPolylineController();

	/** Controller for polygon objects */
	private MVPolygonController polygonController = new MVPolygonController();

	/** Controller for moving point objects */
	private MVMPointController mpointController = new MVMPointController();

	/** Controller for moving label objects */
	private MVMLabelController mlabelController = new MVMLabelController();

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

	private String attributeNameOfMLabel;
	private String attributeNameOfMPoint;

	/** Point with geo coordinates from user */
	private Point myLocation;

	private Vector drawLayer;
	private DrawFeature drawRegularPolygon;

	public MapView() {

		contentPanel.getElement().setClassName("mapcontentpanel");

		// create a mapwidget and add it to the contentpanel
		MapOptions defaultMapOptions = new MapOptions();

		mapWidget = new MapWidget(Window.getClientWidth() + "px",
				Window.getClientHeight() + "px", defaultMapOptions);

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
		LonLat lonLat = new LonLat(7.4947253, 51.3760448);
		lonLat.transform("EPSG:4326", "EPSG:900913");

		map = mapWidget.getMap();

		OSM osm_1 = OSM.Mapnik("Open Street Map Mapnik "); // Label for menu
															// 'LayerSwitcher'
		osm_1.setIsBaseLayer(true);

		OSM osm_2 = OSM.CycleMap("Open Street Map CycleMap ");
		osm_2.setIsBaseLayer(true);

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

		map.setCenter(lonLat, 15);		

		// force the map to fall behind popups
		mapWidget.getElement().getFirstChildElement().getStyle().setZIndex(0);

	}

	/**
	 * Initializes a draw layer to draw a rectangular for simple query with pass
	 * through
	 */
	public void initDrawLayer() {
		if (drawLayer == null) {
			// Create the Vector layer on which the user can draw new widgets
			drawLayer = new Vector("Draw layer");
			drawLayer.setIsBaseLayer(false);
			drawLayer.setDisplayInLayerSwitcher(false);
			map.addLayer(drawLayer);

			RegularPolygonHandlerOptions boxHandlerOptions = new RegularPolygonHandlerOptions();
			boxHandlerOptions.setIrregular(true);
			RegularPolygonHandler boxHandler = new RegularPolygonHandler();
			drawRegularPolygon = new DrawFeature(drawLayer, boxHandler);
			((RegularPolygonHandler) drawRegularPolygon.getHandler())
					.setOptions(boxHandlerOptions);

			map.addControl(drawRegularPolygon);
			drawRegularPolygon.activate();
		}
	}

	/**
	 * The map will be centered on the defined location (GPX coordinates should
	 * be provided)
	 * 
	 * @param lon
	 * @param lat
	 */
	public void centerOnMyLocation(double lon, double lat) {
		LonLat coordinatePair = new LonLat(lon, lat);
		map = mapWidget.getMap();
		coordinatePair.transform("EPSG:4326", "EPSG:900913");
		map.setCenter(coordinatePair, 18);

		Vector markerLayer = new Vector("Marker layer");
		markerLayer.setIsBaseLayer(false);
		map.addLayer(markerLayer);

		Style pointStyle = new Style();
		pointStyle.setFillColor("red");
		pointStyle.setStrokeColor("green");
		pointStyle.setStrokeWidth(2);
		pointStyle.setFillOpacity(0.9);

		myLocation = new Point(lon, lat);
		myLocation.transform(DEFAULT_PROJECTION, new Projection("EPSG:900913"));
		VectorFeature pointFeature = new VectorFeature(myLocation, pointStyle);
		markerLayer.destroyFeatures();
		markerLayer.addFeature(pointFeature);

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
	 * readjusted with to fullscreen
	 * 
	 * @param width
	 *            The new width of the map view
	 * @param height
	 *            The new height of the map view
	 * */
	public void resizeToFullScreen(int width, int height,
			int heightOfOptionsTabPanel) {

		if (width > 850) {

			contentPanel.setWidth(width + "px");
			mapWidget.setWidth(width + "px");
		} else {

			contentPanel.setWidth(850 + "px");
			mapWidget.setWidth(850 + "px");
		}
		if (height > heightOfOptionsTabPanel) {

			contentPanel.setHeight(height - 20 + "px");
			mapWidget.setHeight(height - 20 + "px");
		} else {
			contentPanel.setHeight(heightOfOptionsTabPanel + "px");
			mapWidget.setHeight(heightOfOptionsTabPanel + "px");
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

				if (data.getType().equals("Polyline")) {

					polylineController.addPolyline((Polyline) data, boundsAll,
							boundsLast);
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

					mpointController
							.addMP((MPoint) data, boundsAll, boundsLast);
					polylineController.addPolylineFromMP((MPoint) data,
							boundsAll, boundsLast);
					attributeNameOfMPoint = ((MPoint) data)
							.getAttributeNameInRelation();
				}
				// add label to mpoint if time intervals equals
				if (data.getType().equals("MLabel")) {
					mlabelController.addMLabel((MLabel) data);
					mpointController
							.transmitLabelsToMPandCalculateColorsForPolyline(
									(MLabel) data, polylineController);
					attributeNameOfMLabel = ((MLabel) data)
							.getAttributeNameInRelation();

				}
			}
			dataInitialized = true;
		}
	}

	/** Draws all elements of the map with the new size of the mapwidget */
	public void updateView() {

		// Delete all overlays
		this.resetMap();
		dataLoaded = false;

		if (!currentResultTypeList.isEmpty()) {
			System.out.println("Result list has "
					+ currentResultTypeList.size());

			if (!polylineController.getPolylineMap().isEmpty()
					|| !polylineController.getColoredPolylineMap().isEmpty()) {
				boolean showWithDifColors = false;
				if (modeForSymTraj == 3) {
					showWithDifColors = true;
					legend.setLegendInfo(polylineController
							.getCompleteLegendForMap());
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
	public void resetData() {
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

	/** Removes draw overlay from the map */
	public void removeDrawLayer() {
		if (drawLayer != null && drawRegularPolygon != null) {

			drawRegularPolygon.deactivate();
			drawRegularPolygon.disable();
			map.removeControl(drawRegularPolygon);
			drawRegularPolygon = null;

			drawLayer.destroyFeatures();
			map.removeLayer(drawLayer);
			drawLayer = null;

		}
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
	 * Sets the mode to display a symbolic trajectory
	 * 
	 * @param modeForSymTraj
	 *            The modeForSymTraj to set
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
	 * Returns a legend info
	 * 
	 * @return The legend info
	 */
	public LegendDialog getLegend() {
		return legend;
	}

	/**
	 * Returns an attribute name of mpoint in the loaded relation
	 * 
	 * @return the attributeNameOfMPoint
	 */
	public String getAttributeNameOfMLabel() {
		return attributeNameOfMLabel;
	}

	/**
	 * Returns the defined location point
	 * 
	 * @return The defined location point
	 */
	public Point getMyLocation() {
		return myLocation;
	}

	/**
	 * Returns the draw layer
	 * 
	 * @return
	 */
	public Vector getDrawLayer() {
		return drawLayer;
	}

	/**
	 * Returns the attribute name of mpoint
	 * 
	 * @return
	 */
	public String getAttributeNameOfMPoint() {
		return attributeNameOfMPoint;
	}

}