package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.TextArea;

public class PlainTrajDialog {
	
	private DialogBox plainTrajDialogBox = new DialogBox();
    private FlowPanel dialogContents = new FlowPanel(); 
    private TextArea textView = new TextArea();
	private Button closeButton = new Button("Close");
	
	public PlainTrajDialog(){
		plainTrajDialogBox.setText("Trajectory in text format");
		plainTrajDialogBox.setWidget(dialogContents);
		
		dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
		dialogContents.setSize("410px", "410px");
		textView.setSize("390px", "375px");
		
		
		// Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {          

			@Override
			public void onClick(ClickEvent event) {
				plainTrajDialogBox.hide();
				
			}
	        });
	    closeButton.setStyleName("right-floated-button");
	    
	    dialogContents.add(textView);
	    dialogContents.add(closeButton);		
	    
	}

	/**
	 * @return the dialogContents
	 */
	public FlowPanel getDialogContents() {
		return dialogContents;
	}

	/**
	 * @param dialogContents the dialogContents to set
	 */
	public void setTextViewInPlainTrajDialog(TextArea dialogContents) {
		this.textView.setText(dialogContents.getText());
	}

	/**
	 * @return the plainTrajDialogBox
	 */
	public DialogBox getPlainTrajDialogBox() {
		return plainTrajDialogBox;
	}

}
