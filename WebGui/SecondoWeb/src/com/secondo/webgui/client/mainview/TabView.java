package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.TabPanel;
import com.google.gwt.user.client.ui.TextArea;

public class TabView extends Composite {

	private TabPanel tabPanel = new TabPanel();
	private RawDataView standardView = new RawDataView();
	private TextView formattedView = new TextView();
	private TableView tableView = new TableView();
	private GraphicalView graphicalView = new GraphicalView();
	private MapView mapView = new MapView();
	private OSMView osmView = new OSMView();

	public TabView() {

		tabPanel.setSize("900px", "100%"); // width,height
		tabPanel.getElement().setClassName("tabpanel");

		// Add the first tab with the standardview in a scrollPanel to the tabPanel
		tabPanel.add(standardView.getScrollPanel(), "StandardView");

		// Add a tab with the formatted view in a scrollPanel to the tabPanel
		tabPanel.add(formattedView.getTextOutput(), "FormattedView");

		 
       // Add the tab with the table
		//tabPanel.add(tableView.getTable(), tabTitles[2]);

		// Add the graphical view tab 
		tabPanel.add(graphicalView.getContentPanel(), "GraphicalView");
		
		// Add the map view tab 
		tabPanel.add(mapView.getScrollPanel(), "MapView");
		
		// Add the osm view tab 
		tabPanel.add(osmView.getContentPanel(), "OSMView");

		tabPanel.selectTab(0);
		tabPanel.ensureDebugId("tabPanel");

	}

	public TabPanel getTabPanel() {
		return tabPanel;
	}

	public void setTabPanel(TabPanel tabPanel) {
		this.tabPanel = tabPanel;
	}

	public GraphicalView getGraphicalView() {
		return graphicalView;
	}

	public void setGraphicalView(GraphicalView graphicalView) {
		this.graphicalView = graphicalView;
	}
	
	public RawDataView getStandardView() {
		return standardView;
	}

	public void setStandardView(RawDataView standardView) {
		this.standardView = standardView;
	}

	public TextView getFormattedView() {
		return formattedView;
	}

	public void setFormattedView(TextView formattedView) {
		this.formattedView = formattedView;
	}

	public TableView getTableView() {
		return tableView;
	}

	public void setTableView(TableView tableView) {
		this.tableView = tableView;
	}

	public MapView getMapView() {
		return mapView;
	}

	public void setMapView(MapView mapView) {
		this.mapView = mapView;
	}

	public OSMView getOsmView() {
		return osmView;
	}

	public void setOsmView(OSMView osmView) {
		this.osmView = osmView;
	}

}
