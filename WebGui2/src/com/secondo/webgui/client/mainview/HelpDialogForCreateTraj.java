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
public class HelpDialogForCreateTraj {
	private DialogBox helpDialogBox = new DialogBox();
	private FlowPanel dialogContents = new FlowPanel();
	private ScrollPanel scrollContent = new ScrollPanel();
	private HTML helpInfo;
	private Button closeButton = new Button("Close");

	public HelpDialogForCreateTraj() {

		helpDialogBox.setText("How to Create Symbolic Trajectory?");

		// Create a table to layout the content
		dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
		helpDialogBox.setWidget(dialogContents);

		helpInfo = new HTML(
				"<h3>To create a symbolic trajectory on the basis of your own GPX data</h3>"
						+ "<h4 style=\"color:#009DD8\">Select and upload your GPX track</h4><p>"
						+ "<h4 style=\"color:#009DD8\">Select an option for creating symbolic trajectory</h4>"
						+ "<b>speed mode</b> with peofiles: <5km/h - slow (walking tempo), <20km/h - slow (cycle tempo), <50km/h - moderate tempo, >50km/h - fast tempo<br>"
						+ "<b>direction</b> with values: NorthEast, NorthWest, SouthWest, SouthEast<br>"
						+ "<b>distance</b> to the defined location<br>"
						+ "<b>administrative districts</b> (only within Germany)<br>"
						+ "<h4 style=\"color:#009DD8\">Press on the button Create trajectory </h4>"
						+ "Change to the next tab Try Trajectory and experiment with the generated trajectory");

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
