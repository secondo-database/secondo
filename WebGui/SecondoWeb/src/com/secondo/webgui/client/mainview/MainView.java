package com.secondo.webgui.client.mainview;

import com.google.gwt.event.logical.shared.ResizeEvent;
import com.google.gwt.event.logical.shared.ResizeHandler;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Widget;

public class MainView extends Composite{
	
	private HorizontalPanel mainPanel = new HorizontalPanel();
	private FlowPanel contentPanel = new FlowPanel();
	
	private Header header = new Header();
	private Footer footer = new Footer();
	private StatusBar statusBar = new StatusBar();
	
	private SideBar sidebar = new SideBar();
	private FlowPanel commandPanelWrapper = new FlowPanel();
	private Commandpanel commandPanel = new Commandpanel();
	
	// different views that can be displayed in the mainpanel
	private FlowPanel view = new FlowPanel();
	private RawDataView rawDataView = new RawDataView();
	private TextView textView = new TextView();
	private CellListTextView cellListTextView = new CellListTextView();
	private TableView tableView = new TableView();
	private GraphicalView graphicalView = new GraphicalView();
	private MapView mapView = new MapView();
	private OSMView osmView = new OSMView();
	
	// Panels for map and graphic views with a text sidebar
	private HorizontalPanel mapAndTextPanel = new HorizontalPanel();
	private HorizontalPanel graphicAndTextPanel = new HorizontalPanel();

	public MainView(){
			
		contentPanel.add(view);
		contentPanel.add(commandPanelWrapper);	
		contentPanel.add(statusBar.gethPanel());
		
		mainPanel.add(sidebar.getSidebar());
		mainPanel.add(contentPanel);	

		 //initialize the main view
		showRawDataView();
		
		commandPanelWrapper.add(commandPanel.getCommandPanel());
		
		this.resizeMainElements();
		
		
		//resize the application elements if the size of the window changes
		Window.addResizeHandler(new ResizeHandler()
		{
		  public void onResize(ResizeEvent event)
		  {
		    int windowWidth = event.getWidth();
		    int windowHeight = event.getHeight();
		    
		    header.resize(windowWidth);
		    commandPanel.resize(windowWidth);
		    commandPanel.getMenubarCP().resize(windowWidth);
		    statusBar.resize(windowWidth);
		    rawDataView.resize(windowWidth, windowHeight);
		    textView.resize(windowWidth, windowHeight);
		    graphicalView.resize(windowWidth, windowHeight);
		    mapView.resize(windowWidth, windowHeight);
		    osmView.resize(windowWidth, windowHeight);
		    sidebar.resize(windowHeight);

		  }
		});

	}
	
	
	public void resizeMainElements(){
		
		//get the size of the browserwindow and set the elements to the right size
		int windowHeight = Window.getClientHeight();
		int windowWidth = Window.getClientWidth();
		header.resize(windowWidth);
		commandPanel.resize(windowWidth);
		commandPanel.getMenubarCP().resize(windowWidth);
		statusBar.resize(windowWidth);
		rawDataView.resize(windowWidth, windowHeight);
		textView.resize(windowWidth, windowHeight);
		graphicalView.resize(windowWidth, windowHeight);
		mapView.resize(windowWidth, windowHeight);
		osmView.resize(windowWidth, windowHeight);
		sidebar.resize(windowHeight);
		
	}
	
	public void showCommandPanel(){
		commandPanelWrapper.add(commandPanel.getCommandPanel());
		this.resizeMainElements();
	}
	
	public void hideCommandPanel(){
		commandPanelWrapper.clear();
		int windowHeight = Window.getClientHeight();
		rawDataView.getScrollPanel().setHeight(windowHeight-120+"px");
		rawDataView.getRawDataOutput().setHeight(windowHeight-170+"px");
		
	}

	public void showRawDataView(){
		view.clear();
		view.add(rawDataView.getScrollPanel());
	}
	
	public void showTextView(){
		view.clear();
		view.add(textView.getFormattedScrollPanel());
	}
	
	public void showGraphicalView(){
		view.clear();
		view.add(graphicalView.getScrollPanel());
	}
	
	public void showGoogleMapView(){
		view.clear();
		view.add(mapView.getScrollPanel());
	}
	
	public void showOSMapView(){
		view.clear();
		view.add(osmView.getContentPanel());
	}

	public HorizontalPanel getMainPanel() {
		return mainPanel;
	}

	public void setMainPanel(HorizontalPanel mainPanel) {
		this.mainPanel = mainPanel;
	}

	public SideBar getSidebar() {
		return sidebar;
	}

	public void setSidebar(SideBar sidebar) {
		this.sidebar = sidebar;
	}

	public Commandpanel getCommandPanel() {
		return commandPanel;
	}

	public void setCommandPanel(Commandpanel commandPanel) {
		this.commandPanel = commandPanel;
	}

	public RawDataView getRawDataView() {
		return rawDataView;
	}

	public void setRawDataView(RawDataView rawDataView) {
		this.rawDataView = rawDataView;
	}

	public TextView getTextView() {
		return textView;
	}

	public void setTextView(TextView textView) {
		this.textView = textView;
	}
	

	public CellListTextView getCellListTextView() {
		return cellListTextView;
	}


	public void setCellListTextView(CellListTextView cellListTextView) {
		this.cellListTextView = cellListTextView;
	}


	public TableView getTableView() {
		return tableView;
	}

	public void setTableView(TableView tableView) {
		this.tableView = tableView;
	}

	public GraphicalView getGraphicalView() {
		return graphicalView;
	}

	public void setGraphicalView(GraphicalView graphicalView) {
		this.graphicalView = graphicalView;
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

	public HorizontalPanel getMapAndTextPanel() {
		return mapAndTextPanel;
	}


	public void setMapAndTextPanel(HorizontalPanel mapAndTextPanel) {
		this.mapAndTextPanel = mapAndTextPanel;
	}


	public HorizontalPanel getGraphicAndTextPanel() {
		return graphicAndTextPanel;
	}


	public void setGraphicAndTextPanel(HorizontalPanel graphicAndTextPanel) {
		this.graphicAndTextPanel = graphicAndTextPanel;
	}


	public Header getMainheader() {
		return header;
	}

	public void setMainheader(Header mainheader) {
		this.header = mainheader;
	}

	public StatusBar getStatusBar() {
		return statusBar;
	}

	public void setStatusBar(StatusBar statusBar) {
		this.statusBar = statusBar;
	}

	public Footer getFooter() {
		return footer;
	}

	public void setFooter(Footer footer) {
		this.footer = footer;
	}

}
