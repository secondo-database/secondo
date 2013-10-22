package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.VerticalPanel;

public class SideBar extends Composite{
	
	private FlowPanel sidebar = new FlowPanel();
	private Button showTerminalButton = new Button("<img src='resources/images/showterminal.png' height='30px' width='30px'/>");
	private Button hideTerminalButton = new Button("<img src='resources/images/hideterminal.png' height='30px' width='30px'/>");
	private Button showTextButton = new Button("<img src='resources/images/text-icon2.png' height='30px' width='30px'/>");
	private Button hideTextButton = new Button("<img src='resources/images/hide-text-icon.png' height='30px' width='30px'/>");
	private Button openToolsButton = new Button("<img src='resources/images/tools-icon.png' height='30px' width='30px'/>");
	private Button closeToolsButton = new Button("<img src='resources/images/close-tools-icon.png' height='30px' width='30px'/>");
	private Button mapButton = new Button("<img src='resources/images/map-icon4.png' height='30px' width='30px'/>");
	private Button gmapButton = new Button("<img src='resources/images/googlemapicon.jpg' height='30px' width='30px'/>");
	private Button geometryButton = new Button("<img src='resources/images/geometry-icon4.jpg' height='30px' width='30px'/>");
	private Button rawdataButton = new Button("<img src='resources/images/binary-icon.png' height='30px' width='30px'/>");
	private Button closedatabaseButton = new Button("<img src='resources/images/close-database.png' height='30px' width='30px'/>");
	private Button logoutButton = new Button("<img src='resources/images/logout.gif' height='30px' width='30px'/>");
	
	public SideBar(){
		
		sidebar.add(hideTerminalButton);
		hideTerminalButton.setTitle("Hide Commandpanel");
		showTerminalButton.setTitle("Show Commandpanel");
		
		sidebar.add(showTextButton);
		showTextButton.setTitle("Show Text Panel");
		hideTextButton.setTitle("Hide Text Panel");
		
		/*sidebar.add(openToolsButton);
		openToolsButton.setTitle("Show Tools Panel");*/
		
		sidebar.add(mapButton);
		mapButton.setTitle("Map View");
		//sidebar.add(geometryButton);
		geometryButton.setTitle("Graphical View");

		sidebar.add(rawdataButton);
		rawdataButton.setTitle("Show Raw Data");
		
		sidebar.setWidth("50px");
		sidebar.getElement().setClassName("sidebar");
		
	}
	
	public void resizeHeight(int height){
		sidebar.setHeight(height-55 + "px");
	}

	public FlowPanel getSidebar() {
		return sidebar;
	}

	public void setSidebar(FlowPanel sidebar) {
		this.sidebar = sidebar;
	}

	public Button getShowTerminalButton() {
		return showTerminalButton;
	}

	public void setShowTerminalButton(Button showTerminalButton) {
		this.showTerminalButton = showTerminalButton;
	}

	public Button getHideTerminalButton() {
		return hideTerminalButton;
	}

	public void setHideTerminalButton(Button hideTerminalButton) {
		this.hideTerminalButton = hideTerminalButton;
	}
	

	public Button getOpenToolsButton() {
		return openToolsButton;
	}

	public void setOpenToolsButton(Button openToolsButton) {
		this.openToolsButton = openToolsButton;
	}

	public Button getCloseToolsButton() {
		return closeToolsButton;
	}

	public void setCloseToolsButton(Button closeToolsButton) {
		this.closeToolsButton = closeToolsButton;
	}

	public Button getMapButton() {
		return mapButton;
	}

	public void setMapButton(Button mapButton) {
		this.mapButton = mapButton;
	}

	public Button getGmapButton() {
		return gmapButton;
	}

	public void setGmapButton(Button gmapButton) {
		this.gmapButton = gmapButton;
	}

	public Button getGeometryButton() {
		return geometryButton;
	}

	public void setGeometryButton(Button geometryButton) {
		this.geometryButton = geometryButton;
	}

	public Button getShowTextButton() {
		return showTextButton;
	}

	public void setShowTextButton(Button showTextButton) {
		this.showTextButton = showTextButton;
	}

	public Button getHideTextButton() {
		return hideTextButton;
	}

	public void setHideTextButton(Button hideTextButton) {
		this.hideTextButton = hideTextButton;
	}

	public Button getRawdataButton() {
		return rawdataButton;
	}

	public void setRawdataButton(Button rawdataButton) {
		this.rawdataButton = rawdataButton;
	}

	
}


