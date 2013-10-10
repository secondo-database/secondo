package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.user.client.ui.Widget;

public class MainView extends Composite{
	
	private VerticalPanel mainPanel = new VerticalPanel();

	public MainView(){
	
	}
	
	public void addView(Widget view){
		this.mainPanel.add(view);
	}
	
	public void removeView(Widget view){
		this.mainPanel.remove(view);
	}
	
}
