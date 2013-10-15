package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DockLayoutPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.SplitLayoutPanel;

public class MainPanel extends Composite{

	private DockLayoutPanel dockPanel = new DockLayoutPanel(Unit.EM); //Parameter: int splitterSize
	
	public MainPanel(){


	    // Add text all around.
	    dockPanel.addNorth(new Label("This is the first north component."), 2);//Parameter: Widget widget, double size
	    dockPanel.addSouth(new Label("This is the first south component."), 2); //height
	    dockPanel.addEast(new Label("This is the east component."), 2); //width
	    dockPanel.addWest(new Label("This is the west component."), 2);
	    //splitPanel.addNorth(new Label("This is the second north component."), 50);
	    //splitPanel.addSouth(new Label("This is the second south component."), 50);

	    // Add scrollable text to the center.
	    String centerText = "This is text in the center";
	    
	    Label centerLabel = new Label(centerText);
	    ScrollPanel centerScrollable = new ScrollPanel(centerLabel);
	    dockPanel.add(centerScrollable); //with add only content is added to the center
	}
}
