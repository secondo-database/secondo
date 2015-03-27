package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;

public class PatternDialog {
	
	private DialogBox helpDialogBox = new DialogBox();
    private FlowPanel dialogContents = new FlowPanel();
    private ScrollPanel scrollContent = new ScrollPanel();
	private HTML patternInfo;
	private Button closeButton = new Button("Close");
	
	public PatternDialog(){
		
		helpDialogBox.setText("Info about Pattern Matching");

	    // Create a table to layout the content
	    dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    helpDialogBox.setWidget(dialogContents);
		
		patternInfo = new HTML("<h3>Information about Pattern Matching</h3>" +
				"In Pattern queries you can define: <p>"+
				"<b>unit pattern</b> which has one of the forms ( t l ), (_ l ), ( t _), or (), where <b>t</b> is a time interval, "+
				"<b>l</b> is a label, and <b>_</b> is a wildcard symbol.<p>" +
				"<b>sequence pattern</b> which has one of the forms *, +, [p], [p1 | p2], [p]+, [p]*, or [p]?, where p, p1, p2 are simple patterns <p>"+
				"<b>variable</b> to bind a specified pattern element <p>"+
				"<b>condition</b> which has a form <variable.attribute> with attributes: label(s), time, start, end, leftclosed, rightclosed"+
				"<h3>Sample Pattern</h3><p>" +
				"A (_\"SouthEast\") + B (_ \"SouthWest\")<br>" +				
				"// (B.end - A.start) < (20 * minute)  <p>" );
		
		// Add the text to the dialog
	    scrollContent.add(patternInfo);
	    dialogContents.add(scrollContent);

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            helpDialogBox.hide();}
	        });
	    closeButton.setStyleName("right-floated-button");
	    dialogContents.add(closeButton);
	}
	
	/**Returns the dialog box containing the optimizer info text
	 * 
	 * @return The dialog box containing the optimizer info text
	 * */
	public DialogBox getHelpDialogBox() {
		return helpDialogBox;
	}

}
