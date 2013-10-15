package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.TextBox;

public class Header extends Composite{
	
	private HorizontalPanel hPanel = new HorizontalPanel();
	private String headertext = "<h2>Database Management</h2>";
	private HTML homeText = new HTML(headertext, true);
	private Image logo = new Image("resources/images/secondo-logo.gif");
	private Navigation navigation = new Navigation();
	
	public Header(){
		
		//int windowHeight = Window.getClientHeight();
		int windowWidth = Window.getClientWidth();
		
		logo.getElement().getStyle().setMarginLeft(30, Unit.PX);
		logo.setWidth("250px");
		hPanel.add(logo);
		//homeText.setHeight("45px");
		//homeText.addStyleName("headline");
		//hPanel.add(homeText);
		
		//hPanel.add(navigation.gethPanel());
		hPanel.getElement().setClassName("mainheader");
		
		hPanel.setWidth(windowWidth + "px");
		hPanel.setHeight("60px");
		//hPanel.setSize("100%", "60px");
	}
	
	public void resize(int width){
		hPanel.setWidth(width + "px");
	}

	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}

	public Navigation getNavigation() {
		return navigation;
	}

	public void setNavigation(Navigation navigation) {
		this.navigation = navigation;
	}
	
	

}
