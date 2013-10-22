package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextArea;

public class RawDataView extends Composite{
	
	private ScrollPanel scrollPanel = new ScrollPanel();
	private TextArea rawDataOutput = new TextArea();
	
	public RawDataView(){
		
		rawDataOutput.setEnabled(false);
		rawDataOutput.getElement().setClassName("rawdataoutput");
	
		//Add Textarea to the scrollable Panel
		scrollPanel.add(rawDataOutput);
		scrollPanel.getElement().setClassName("rawdatascrollpanel");	
	}
	
	public void resize(int width, int height){
		
		//remove 40 pixel in height and width for the padding of the dataoutput + 10 extra
		scrollPanel.setWidth(width-80 + "px");
		scrollPanel.setHeight(height-320 + "px");
		rawDataOutput.setWidth(width-120 + "px");
		rawDataOutput.setHeight(height-360 + "px");
	}
	
    public void resizeToFullScreen(int height){
    	scrollPanel.setHeight(height-90+"px");
		rawDataOutput.setHeight(height-130+"px");
	}


	public TextArea getRawDataOutput() {
		return rawDataOutput;
	}

	public void setRawDataOutput(TextArea secondoOutput) {
		this.rawDataOutput = secondoOutput;
	}

	public ScrollPanel getScrollPanel() {
		return scrollPanel;
	}

	public void setScrollPanel(ScrollPanel scrollPanel) {
		this.scrollPanel = scrollPanel;
	}


}
