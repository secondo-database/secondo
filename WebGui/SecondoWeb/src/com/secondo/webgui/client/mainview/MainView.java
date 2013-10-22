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
	private HorizontalPanel view = new HorizontalPanel();
	private RawDataView rawDataView = new RawDataView();
	private TextView textView = new TextView();
	private CellListTextView cellListTextView = new CellListTextView();
	private TableView tableView = new TableView();
	private GraphicalView graphicalView = new GraphicalView();
	private MapView mapView = new MapView();
	private OSMView osmView = new OSMView();
	private ToolBar toolbar = new ToolBar();
	
	private boolean cpTurnedOn = true;
	private boolean textTurnedOn = false;
	private boolean mapTurnedOn = false;
	private boolean toolsTurnedOn = false;

	public MainView(){
			
		contentPanel.add(view);
		contentPanel.add(commandPanelWrapper);	
		contentPanel.add(statusBar.gethPanel());
		
		mainPanel.add(sidebar.getSidebar());
		mainPanel.add(contentPanel);	

		 //initialize the main view
		//showRawDataView();
		showGraphicalView();
		
		commandPanelWrapper.add(commandPanel.getCommandPanel());
		
		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
		
		this.resizeWithCP(windowWidth, windowHeight);
		
		
		//resize the application elements if the size of the window changes
		Window.addResizeHandler(new ResizeHandler()
		{
		  public void onResize(ResizeEvent event)
		  {
		    int windowWidth = event.getWidth();
		    int windowHeight = event.getHeight();
		    
		    if(cpTurnedOn && textTurnedOn){
			    resizeWithTextAndCP(windowWidth, windowHeight);
		    }
		    if(cpTurnedOn && !textTurnedOn){
		    	resizeWithCP(windowWidth, windowHeight);
		    }

		    if(!cpTurnedOn && textTurnedOn){
		    	resizeWithTextPanel(windowWidth, windowHeight);
		    }
		    if(!cpTurnedOn && !textTurnedOn){
		    	resizeToFullScreen(windowWidth, windowHeight);
		    }
		  }
		});

	}
	
	/** Resize all visible elements to the size of the browser window*/
	public void resizeWithCP(int width, int height){		
		
		header.resizeWidth(width);

		rawDataView.resize(width, height);
		textView.resizeWithCP(height);
		
		graphicalView.resizeWithCP(width, height);
		osmView.resizeWithCP(width, height);
		
		toolbar.resizeHeightWithCP(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resize(width);
		statusBar.resizeWidth(width);
		
	}
	
	public void resizeWithTextPanel(int width, int height){
		
		header.resizeWidth(width);

		rawDataView.resize(width, height);
		textView.resizeToFullScreen(height);
		
		graphicalView.resizeWithTextPanel(width, height);
		osmView.resizeWithTextPanel(width, height);
		
		toolbar.resizeHeightToFullScreen(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resize(width);
		statusBar.resizeWidth(width);
	}
	
	public void resizeWithTextAndCP(int width, int height){
		
		header.resizeWidth(width);

		rawDataView.resize(width, height);
		textView.resizeWithCP(height);
		
		graphicalView.resizeWithTextAndCP(width, height);
		osmView.resizeWithTextAndCP(width, height);
		
		toolbar.resizeHeightWithCP(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resize(width);
		statusBar.resizeWidth(width);
		
	}
	
	/**Resize the view elements to full screen*/
	public void resizeToFullScreen(int width, int height){
        
		header.resizeWidth(width);
		
		rawDataView.resizeToFullScreen(height);
		textView.resizeToFullScreen(height);
		
		graphicalView.resizeToFullScreen(width, height);
		osmView.resizeToFullScreen(width, height);
		
		toolbar.resizeHeightToFullScreen(height);
		sidebar.resizeHeight(height);
		commandPanel.resizeWidth(width);
		commandPanel.getMenubarCP().resize(width);
		statusBar.resizeWidth(width);
	}
	
	public void showCommandPanel(){
		
		cpTurnedOn = true;
		commandPanelWrapper.add(commandPanel.getCommandPanel());
		
		sidebar.getSidebar().remove(sidebar.getShowTerminalButton());
		sidebar.getSidebar().insert(sidebar.getHideTerminalButton(), 0); //Parameter: widget, beforeIndex
		
		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
		
		if(textTurnedOn){			
		   this.resizeWithTextAndCP(windowWidth, windowHeight);
		   
		}
		else{
		   this.resizeWithCP(windowWidth, windowHeight);
		}
	}
	
	public void hideCommandPanel(){
		
		cpTurnedOn = false;
		commandPanelWrapper.clear();
		
		sidebar.getSidebar().remove(sidebar.getHideTerminalButton());
		sidebar.getSidebar().insert(sidebar.getShowTerminalButton(), 0); //Parameter: widget, beforeIndex
		
		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
		
		if(textTurnedOn){			
			   this.resizeWithTextPanel(windowWidth, windowHeight);
			   
			}
		else{
			   this.resizeToFullScreen(windowWidth, windowHeight);
			}
		
	}

	
	public void showTextView(){
		
		textTurnedOn = true;
		view.clear();
		
		sidebar.getSidebar().remove(sidebar.getShowTextButton());
		sidebar.getSidebar().insert(sidebar.getHideTextButton(), 1); //Parameter: widget, beforeIndex
				
		view.add(textView.getTextOutput());
		if(mapTurnedOn){
			view.add(osmView.getContentPanel());
			view.add(toolbar.getFpanel());
		}
		else{
			view.add(graphicalView.getContentPanel());
			view.add(toolbar.getFpanel());
		}

		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
		
		if(cpTurnedOn){			
			   this.resizeWithTextAndCP(windowWidth, windowHeight);
			   
			}
		else{
			   this.resizeWithTextPanel(windowWidth, windowHeight);
			}

	}
	
	/**Remove the textpanel from the view*/
	public void hideTextView(){
		
		textTurnedOn = false;
		view.clear();
		
		sidebar.getSidebar().remove(sidebar.getHideTextButton());
		sidebar.getSidebar().insert(sidebar.getShowTextButton(), 1); //Parameter: widget, beforeIndex
		
		if(mapTurnedOn){
			view.add(osmView.getContentPanel());
			view.add(toolbar.getFpanel());
		}
		else{
			view.add(graphicalView.getContentPanel());
			view.add(toolbar.getFpanel());
		}
		
		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
				
		if(cpTurnedOn){			
			 this.resizeWithCP(windowWidth, windowHeight);
					   
			}
		else{
			this.resizeToFullScreen(windowWidth, windowHeight);
			}		
	}
	
	
	/**Show the graphical view*/
	public void showGraphicalView(){
		
		mapTurnedOn = false;
		
		sidebar.getSidebar().remove(sidebar.getGeometryButton());
		sidebar.getSidebar().insert(sidebar.getMapButton(), 2); //Parameter: widget, beforeIndex

		view.clear();
		
		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
		
		if(cpTurnedOn && textTurnedOn){
		    resizeWithTextAndCP(windowWidth, windowHeight);
		    view.add(textView.getTextOutput());
			view.add(graphicalView.getContentPanel());
			view.add(toolbar.getFpanel());
	    }
	    if(cpTurnedOn && !textTurnedOn){
	    	resizeWithCP(windowWidth, windowHeight);
			view.add(graphicalView.getContentPanel());
			view.add(toolbar.getFpanel());
	    }

	    if(!cpTurnedOn && textTurnedOn){
	    	resizeWithTextPanel(windowWidth, windowHeight);
	    	view.add(textView.getTextOutput());
			view.add(graphicalView.getContentPanel());
			view.add(toolbar.getFpanel());
	    }
	    if(!cpTurnedOn && !textTurnedOn){
	    	resizeToFullScreen(windowWidth, windowHeight);
			view.add(graphicalView.getContentPanel());
			view.add(toolbar.getFpanel());
	    }

	}	
	
	public void showMapView(){
		
		mapTurnedOn = true;
		
		sidebar.getSidebar().remove(sidebar.getMapButton());
		sidebar.getSidebar().insert(sidebar.getGeometryButton(), 2); //Parameter: widget, beforeIndex

		view.clear();
		
		//get the size of the browserwindow and set the elements to the right size
		int windowWidth = Window.getClientWidth();
		int windowHeight = Window.getClientHeight();
				
		if(cpTurnedOn && textTurnedOn){
			resizeWithTextAndCP(windowWidth, windowHeight);
			view.add(textView.getTextOutput());
			view.add(osmView.getContentPanel());
			view.add(toolbar.getFpanel());
			 }
		if(cpTurnedOn && !textTurnedOn){
			 resizeWithCP(windowWidth, windowHeight);
			 view.add(osmView.getContentPanel());
			 view.add(toolbar.getFpanel());
			  }

		if(!cpTurnedOn && textTurnedOn){
			 resizeWithTextPanel(windowWidth, windowHeight);
			 view.add(textView.getTextOutput());
			 view.add(osmView.getContentPanel());
			 view.add(toolbar.getFpanel());
			  }
		if(!cpTurnedOn && !textTurnedOn){
			  resizeToFullScreen(windowWidth, windowHeight);
			  view.add(osmView.getContentPanel());
			  view.add(toolbar.getFpanel());
			}		
	}
	
	public void showTools(){
		
	}
	
	public void hideTools(){
		
	}
	
	public void showRawDataView(){

		view.clear();
		view.add(rawDataView.getScrollPanel());
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

	public ToolBar getToolbar() {
		return toolbar;
	}

	public void setToolbar(ToolBar toolbar) {
		this.toolbar = toolbar;
	}

	public boolean isToolsTurnedOn() {
		return toolsTurnedOn;
	}

	public void setToolsTurnedOn(boolean toolsTurnedOn) {
		this.toolsTurnedOn = toolsTurnedOn;
	}

	public Footer getFooter() {
		return footer;
	}

	public void setFooter(Footer footer) {
		this.footer = footer;
	}

	public boolean isCpTurnedOn() {
		return cpTurnedOn;
	}

	public void setCpTurnedOn(boolean cpTurnedOn) {
		this.cpTurnedOn = cpTurnedOn;
	}

	public boolean isTextTurnedOn() {
		return textTurnedOn;
	}

	public void setTextTurnedOn(boolean textTurnedOn) {
		this.textTurnedOn = textTurnedOn;
	}

	public boolean isMapTurnedOn() {
		return mapTurnedOn;
	}

	public void setMapTurnedOn(boolean mapTurnedOn) {
		this.mapTurnedOn = mapTurnedOn;
	}
	

}
