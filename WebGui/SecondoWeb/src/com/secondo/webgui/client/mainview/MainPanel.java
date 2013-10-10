package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.SplitLayoutPanel;

public class MainPanel extends Composite{

	private SplitLayoutPanel splitPanel = new SplitLayoutPanel(5); //Parameter: int splitterSize
	
	public MainPanel(){
	    
	    splitPanel.ensureDebugId("splitLayoutPanel");
	    splitPanel.getElement().getStyle()
	        .setProperty("border", "3px solid #e7e7e7");

	    // Add text all around.
	    splitPanel.addNorth(new Label("This is the first north component."), 50);//Parameter: Widget widget, double size
	    splitPanel.addSouth(new Label("This is the first south component."), 50); //height
	    splitPanel.addEast(new Label("This is the east component."), 100); //width
	    splitPanel.addWest(new Label("This is the west component."), 100);
	    //splitPanel.addNorth(new Label("This is the second north component."), 50);
	    //splitPanel.addSouth(new Label("This is the second south component."), 50);

	    // Add scrollable text to the center.
	    String centerText = "This is text in the center";
	    
	    Label centerLabel = new Label(centerText);
	    ScrollPanel centerScrollable = new ScrollPanel(centerLabel);
	    splitPanel.add(centerScrollable); //with add only content is added to the center
	}
}
