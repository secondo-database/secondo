package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.VerticalPanel;

public class SideBar extends Composite{
	
	private FlowPanel sidebar = new FlowPanel();
	private Button showTerminalButton = new Button("<img src='resources/images/showterminal.png' height='30px' width='30px'/>");
	private Button hideTerminalButton = new Button("<img src='resources/images/hideterminal.png' height='30px' width='30px'/>");
	private Button mapButton = new Button("<img src='resources/images/map-icon4.png' height='30px' width='30px'/>");
	private Button gmapButton = new Button("<img src='resources/images/googlemapicon.jpg' height='30px' width='30px'/>");
	private Button geometryButton = new Button("<img src='resources/images/geometry-icon4.jpg' height='30px' width='30px'/>");
	private Button textButton = new Button("<img src='resources/images/text-icon2.png' height='30px' width='30px'/>");
	private Button rawdataButton = new Button("<img src='resources/images/binary-icon.png' height='30px' width='30px'/>");
	private Button closedatabaseButton = new Button("<img src='resources/images/close-database.png' height='30px' width='30px'/>");
	private Button logoutButton = new Button("<img src='resources/images/logout.gif' height='30px' width='30px'/>");
	
	public SideBar(){
		
		sidebar.add(showTerminalButton);
		showTerminalButton.setTitle("Show Commandpanel");
		sidebar.add(hideTerminalButton);
		hideTerminalButton.setTitle("Hide Commandpanel");
		sidebar.add(mapButton);
		mapButton.setTitle("Map View");
		/*sidebar.add(gmapButton);
		mapButton.setTitle("Googlemap View");*/
		sidebar.add(geometryButton);
		geometryButton.setTitle("Graphical View");
		sidebar.add(textButton);
		textButton.setTitle("Text only View");
		sidebar.add(rawdataButton);
		rawdataButton.setTitle("Raw Data View");
		sidebar.add(closedatabaseButton);
		closedatabaseButton.setTitle("Close Database");
		sidebar.add(logoutButton);
		logoutButton.setTitle("Logout");
		
		showTerminalButton.setEnabled(false);
		
		sidebar.setWidth("50px");
		sidebar.getElement().setClassName("sidebar");
		
	}
	
	public void resize(int height){
		sidebar.setHeight(height-60 + "px");
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

	public Button getTextButton() {
		return textButton;
	}

	public void setTextButton(Button textButton) {
		this.textButton = textButton;
	}

	public Button getRawdataButton() {
		return rawdataButton;
	}

	public void setRawdataButton(Button rawdataButton) {
		this.rawdataButton = rawdataButton;
	}

	public Button getClosedatabaseButton() {
		return closedatabaseButton;
	}

	public void setClosedatabaseButton(Button closedatabaseButton) {
		this.closedatabaseButton = closedatabaseButton;
	}

	public Button getLogoutButton() {
		return logoutButton;
	}

	public void setLogoutButton(Button logoutButton) {
		this.logoutButton = logoutButton;
	}

}


