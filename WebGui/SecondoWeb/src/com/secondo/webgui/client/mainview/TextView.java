package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextArea;


public class TextView extends Composite {
	
	private ScrollPanel formattedScrollPanel = new ScrollPanel();
	private TextArea formattedOutput = new TextArea();
	
	public TextView(){
		
		  formattedOutput.setEnabled(false);
		  formattedOutput.getElement().setClassName("formattedoutput");
		
		  formattedScrollPanel.add(formattedOutput);
		  formattedScrollPanel.getElement().setClassName("textscrollpanel");
		
	}
	
	public void resize(int width, int height){
		
		formattedScrollPanel.setWidth(width-70 + "px");
		formattedScrollPanel.setHeight(height-330 + "px");
		formattedOutput.setWidth(width-120 + "px");
		formattedOutput.setHeight(height-380 + "px");
	}

	public TextArea getFormattedOutput() {
		return formattedOutput;
	}

	public void setFormattedOutput(TextArea formattedOutput) {
		this.formattedOutput = formattedOutput;
	}

	public ScrollPanel getFormattedScrollPanel() {
		return formattedScrollPanel;
	}

	public void setFormattedScrollPanel(ScrollPanel formattedScrollPanel) {
		this.formattedScrollPanel = formattedScrollPanel;
	}

}
