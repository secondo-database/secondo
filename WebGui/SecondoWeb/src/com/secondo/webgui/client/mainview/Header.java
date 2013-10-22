package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.TextBox;

public class Header extends Composite{
	
	private HorizontalPanel hPanel = new HorizontalPanel();
	//private String headertext = "<h2>Database Management</h2>";
	//private HTML homeText = new HTML(headertext, true);
	private Image logo = new Image("resources/images/secondo-logo.gif");
	//private Navigation navigation = new Navigation();
	
	private HorizontalPanel buttonPanel = new HorizontalPanel();
	private Button closedatabaseButton = new Button("<img src='resources/images/close-database.png' height='30px' width='30px'/>");
	private Button logoutButton = new Button("<img src='resources/images/logout.gif' height='30px' width='30px'/>");
	
	public Header(){

		int windowWidth = Window.getClientWidth();
		
		logo.getElement().getStyle().setMarginLeft(30, Unit.PX);
		logo.setWidth("250px");
		hPanel.add(logo);
		
		closedatabaseButton.getElement().setClassName("closedatabasebutton");
		closedatabaseButton.getElement().getStyle().setBackgroundColor("white");
		closedatabaseButton.setWidth("40px");
		closedatabaseButton.setTitle("Close Database");
		logoutButton.getElement().setClassName("logoutbutton");
		logoutButton.setWidth("40px");
		logoutButton.setTitle("Logout");
		
		buttonPanel.setWidth("100px");
		buttonPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
		buttonPanel.getElement().getStyle().setMarginRight(30, Unit.PX);
		buttonPanel.getElement().getStyle().setMarginLeft(windowWidth-410, Unit.PX);
		buttonPanel.add(closedatabaseButton);
		buttonPanel.add(logoutButton);

		hPanel.add(buttonPanel);

		hPanel.getElement().setClassName("mainheader");	
		hPanel.setWidth(windowWidth-20 + "px");
		hPanel.setHeight("50px");
	}
	
	public void resizeWidth(int width){
		hPanel.setWidth(width + "px");
		buttonPanel.getElement().getStyle().setMarginLeft(width-410, Unit.PX);
	}

	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
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
