package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
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
		
		logo.getElement().getStyle().setMarginLeft(20, Unit.PX);
		logo.setWidth("250px");
		//logo.setHeight("45px");
		hPanel.add(logo);
		//homeText.setHeight("45px");
		//homeText.addStyleName("headline");
		//hPanel.add(homeText);
		hPanel.add(navigation.gethPanel());
		
		hPanel.setSize("900px", "60px");
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
