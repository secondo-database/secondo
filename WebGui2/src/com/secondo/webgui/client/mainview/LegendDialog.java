package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;



/**
 * 
 * @author Irina Russkaya
 */
public class LegendDialog {
	
	private DialogBox legendDialogBox = new DialogBox();
    private FlowPanel dialogContents = new FlowPanel();
    private ScrollPanel scrollContent = new ScrollPanel();
	private HTML legendInfo;
	private Button closeButton = new Button("Close");
	
	
	public LegendDialog() {
		
		legendDialogBox.setText("Legend Info");
		
		// Create a table to layout the content
	    dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    legendDialogBox.setWidget(dialogContents);
	    legendInfo=new HTML("No info");
	    
	 // Add the text to the dialog
	    legendInfo.setSize("390px", "390px");
	    scrollContent.setSize("410px", "410px");
	    scrollContent.add(legendInfo);
	    dialogContents.add(scrollContent);
	    
	 // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {          

			@Override
			public void onClick(ClickEvent event) {
				legendDialogBox.hide();
				
			}
	        });
	    dialogContents.add(closeButton);
		
	}


	public DialogBox getHelpDialogBox() {
		return legendDialogBox;
	}


	public HTML getLegendInfo() {
		return legendInfo;
	}


	public void setLegendInfo(String legendInfo) {
		this.legendInfo.setHTML(legendInfo);
	}	
	
	public void resetLegendInfo(){
		this.legendInfo.setHTML("");
	}
	
	
	
	
}
