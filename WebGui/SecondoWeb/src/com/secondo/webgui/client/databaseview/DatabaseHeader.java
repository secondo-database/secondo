package com.secondo.webgui.client.databaseview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.Image;

public class DatabaseHeader extends Composite{
	
	private FlowPanel headerPanel = new FlowPanel();
	private Image logo = new Image("resources/images/secondo-logo.gif");
	private HTML secondoHeadline = new HTML("<h3>An Extensible Database System </h3>");
	
	public DatabaseHeader(){
		
		logo.setWidth("300px");
		headerPanel.add(logo);
		headerPanel.add(secondoHeadline);
		
	}

	public FlowPanel getHeaderPanel() {
		return headerPanel;
	}

	public void setHeaderPanel(FlowPanel headerPanel) {
		this.headerPanel = headerPanel;
	}
	
}
