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
 * This class represents a dialog containing the help information about creating
 * symbolic trajectory
 * 
 * @author Irina Russkaya
 *
 */
public class HelpDialog {
	private DialogBox helpDialogBox = new DialogBox();
	private FlowPanel dialogContents = new FlowPanel();
	private ScrollPanel scrollContent = new ScrollPanel();
	private HTML helpInfo;
	private Button closeButton = new Button("Close");

	public HelpDialog(String header, HTML helpInfo) {

		helpDialogBox.setText(header);

		// Create a table to layout the content
		dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
		helpDialogBox.setWidget(dialogContents);		

		// Add the text to the dialog
		scrollContent.add(helpInfo);
		dialogContents.add(scrollContent);

		// Add a close button at the bottom of the dialog
		closeButton.addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				helpDialogBox.hide();
			}
		});
		closeButton.setStyleName("right-floated-button");
		dialogContents.add(closeButton);
	}

	/**
	 * Returns the dialog box containing the optimizer info text
	 * 
	 * @return The dialog box containing the optimizer info text
	 * */
	public DialogBox getHelpDialogBox() {
		return helpDialogBox;
	}

}
