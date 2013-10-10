package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.VerticalPanel;

public class StandardView extends Composite{
	
	private ScrollPanel scrollPanel = new ScrollPanel();
	private VerticalPanel contentPanel = new VerticalPanel();
	private TextArea secondoOutput = new TextArea();
	
	public StandardView(){
		
        scrollPanel.setSize("890px", "460px");
		
		secondoOutput.setEnabled(false);
		secondoOutput.setSize("880px", "450px");
		secondoOutput.getElement().setClassName("secondooutput");
		
		//Add Textfields to the scrollable Panels
		scrollPanel.add(secondoOutput);

		//not necessary
		contentPanel.add(scrollPanel);
		
	}

	public VerticalPanel getContentPanel() {
		return contentPanel;
	}

	public void setContentPanel(VerticalPanel contentPanel) {
		this.contentPanel = contentPanel;
	}

	public TextArea getSecondoOutput() {
		return secondoOutput;
	}

	public void setSecondoOutput(TextArea secondoOutput) {
		this.secondoOutput = secondoOutput;
	}

	public ScrollPanel getScrollPanel() {
		return scrollPanel;
	}

	public void setScrollPanel(ScrollPanel scrollPanel) {
		this.scrollPanel = scrollPanel;
	}


}
