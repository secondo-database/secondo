/**
 * 
 */
package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ListBox;

/**
 * @author Irina Russkaya
 *
 */
public class GroupsToShowPanel extends HorizontalPanel {

	private ListBox numberOfTrajectoriesInGroup = new ListBox();
	private Label numberOfTrajectoriesToShowBeforeLabel = new Label(
			"Show in groups of: ");
	private Label numberOfTrajectoriesToShowAfterLabel = new Label("     tuples");
	private Button previousGroupButton = new Button(
			"<span></span> previous group");
	private Button nextGroupButton = new Button("next group <span></span>");

	public GroupsToShowPanel() {
		super();

		
		numberOfTrajectoriesToShowBeforeLabel
				.setStyleName("labelTextInOneLineWithItalic");
		
		numberOfTrajectoriesToShowAfterLabel
				.setStyleName("smallLabelTextInOneLineWithItalic");
		

		numberOfTrajectoriesInGroup.addItem(" ");
		numberOfTrajectoriesInGroup.addItem("3");
		numberOfTrajectoriesInGroup.addItem("5");
		numberOfTrajectoriesInGroup.addItem("10");
		numberOfTrajectoriesInGroup.addItem("15");

		previousGroupButton.setStyleName("previousButton");
		previousGroupButton.setEnabled(false);
		nextGroupButton.setStyleName("nextButton");
		nextGroupButton.setEnabled(true);
		nextGroupButton.getElement().setAttribute("padding-left", "5px");
		nextGroupButton.setWidth("114px");

		super.add(numberOfTrajectoriesToShowBeforeLabel);
		super.add(numberOfTrajectoriesInGroup);
		super.add(numberOfTrajectoriesToShowAfterLabel);
		super.getElement().setAttribute("padding-left", "10px");
	}

	/**
	 * @return the numberOfTrajectoriesInGroup
	 */
	public ListBox getNumberOfTrajectoriesInGroup() {
		return numberOfTrajectoriesInGroup;
	}

	/**
	 * @return the previousGroupButton
	 */
	public Button getPreviousGroupButton() {
		return previousGroupButton;
	}

	/**
	 * @return the nextGroupButton
	 */
	public Button getNextGroupButton() {
		return nextGroupButton;
	}
	
	public FlexTable getPreviousNextGroupPan(){
		FlexTable previousNextGroupPan = new FlexTable();
		previousNextGroupPan.setWidget(0, 0, previousGroupButton);
		previousNextGroupPan.setWidget(0, 1, nextGroupButton);
		return previousNextGroupPan;
	}

}
