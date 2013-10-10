package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.TabPanel;
import com.google.gwt.user.client.ui.TextArea;

public class TabView extends Composite {

	private TabPanel tabPanel = new TabPanel();
	private StandardView standardView = new StandardView();
	private FormattedView formattedView = new FormattedView();
	private TableView tableView = new TableView();
	private GraphicalView graphicalView = new GraphicalView();
	private MapView mapView = new MapView();


	public TabView() {

		tabPanel.setSize("900px", "100%"); // width,height
		tabPanel.getElement().setClassName("tabpanel");

		// Add the first tab with the standardview in a scrollPanel to the tabPanel
		tabPanel.add(standardView.getScrollPanel(), "StandardView");

		// Add a tab with the formatted view in a scrollPanel to the tabPanel
		tabPanel.add(formattedView.getContentPanel(), "FormattedView");

		 
       // Add the tab with the table
		//tabPanel.add(tableView.getTable(), tabTitles[2]);

		// Add the graphical view tab 
		tabPanel.add(graphicalView.gethPanel(), "GraphicalView");
		
		// Add the map view tab 
		tabPanel.add(mapView.gethPanel(), "MapView");

		tabPanel.selectTab(0);
		tabPanel.ensureDebugId("tabPanel");

	}

	public TabPanel getTabPanel() {
		return tabPanel;
	}

	public void setTabPanel(TabPanel tabPanel) {
		this.tabPanel = tabPanel;
	}

	public TextArea getSecondoOutput() {
		return standardView.getSecondoOutput();
	}

	public void setSecondoOutput(TextArea secondoOutput) {
		standardView.setSecondoOutput(secondoOutput);
	}

	public GraphicalView getGraphicalView() {
		return graphicalView;
	}

	public void setGraphicalView(GraphicalView graphicalView) {
		this.graphicalView = graphicalView;
	}
	
	public StandardView getStandardView() {
		return standardView;
	}

	public void setStandardView(StandardView standardView) {
		this.standardView = standardView;
	}

	public TextArea getFormattedOutput() {
		return formattedView.getFormattedOutput();
	}

	public void setFormattedOutput(TextArea formattedOutput) {
		formattedView.setFormattedOutput(formattedOutput);
	}

	public FormattedView getFormattedView() {
		return formattedView;
	}

	public void setFormattedView(FormattedView formattedView) {
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
	

}
