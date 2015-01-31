package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.TextArea;

/**
 * This class represents a dialog box associated with the menu item
 * "plain trajectory" which contains a text area to display the secondo query
 * results in formatted text
 * 
 * @author Irina Russkaya
 * 
 */
public class PlainTrajDialog {

	private DialogBox plainTrajDialogBox = new DialogBox();
	/** The main panel of the dialog box */
	private FlowPanel dialogContents = new FlowPanel();
	/** The textarea to display formatted text */
	private TextArea textView = new TextArea();
	/** The close button for the dialog box */
	private Button closeButton = new Button("Close");
	/** The resultlist for the secondo query results */
	private ArrayList<ArrayList<String>> resultList = new ArrayList<ArrayList<String>>();
	private boolean dataLoaded = false;

	public PlainTrajDialog() {
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

	/** Updates the text view with the last result from the result list */
	public void updateTextView() {

		// reset text view
		textView.setText("");
		String currentResult = textView.getText();

		// add the last result from the list to the text output
		int lastindex = resultList.size() - 1;
		for (String data : resultList.get(lastindex)) {
			currentResult = textView.getText();
			textView.setText(currentResult + data);
		}

	}

	/** Deletes all data from the textview, the resultList and the resultlistBox */
	public void resetData() {

		// reset text view
		textView.setText("");
		resultList.clear();

	}

	/**
	 * @return the dialogContents
	 */
	public FlowPanel getDialogContents() {
		return dialogContents;
	}

	/**
	 * @param dialogContents
	 *            the dialogContents to set
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

	/**
	 * @return the resultList
	 */
	public ArrayList<ArrayList<String>> getResultList() {
		return resultList;
	}

	public boolean isDataLoaded() {
		return dataLoaded;
	}

	public void setDataLoaded(boolean dataLoaded) {
		this.dataLoaded = dataLoaded;
	}

	public TextArea getTextView() {
		return textView;
	}

}
