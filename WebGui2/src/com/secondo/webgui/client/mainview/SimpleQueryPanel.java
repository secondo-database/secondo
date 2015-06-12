package com.secondo.webgui.client.mainview;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyCodes;
import com.google.gwt.event.dom.client.KeyPressEvent;
import com.google.gwt.event.dom.client.KeyPressHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.VerticalPanel;

/**
 * @author Irina Russkaya
 *
 */
public class SimpleQueryPanel extends VerticalPanel {
	private DefaultTextBox textForQuery;	
	private Button defineButton;
	private Label resultInfoLabel;
	private Label infoAboutTupleNo;
	private VerticalPanel vertPanWithPreviousNextTuple = new VerticalPanel();
	private Button previousTuple=new Button("<span></span> previous tuple");
	private Button nextTuple=new Button("next tuple <span></span>");;

	public SimpleQueryPanel(String textForHelpInfoLabel, String typeOfQuery,
			String defaultText) {
		super();

		this.setSpacing(4);
		this.setWidth("100%");
		Label helpInfoLabel = new Label(textForHelpInfoLabel);
		helpInfoLabel.setStylePrimaryName("labelForPasses");
		this.add(helpInfoLabel);

		if (typeOfQuery.equals("passThrough")) {
			Label helpInfoLevel2 = new Label(
					"Define with your mouse a region on the map");
			helpInfoLevel2.setStylePrimaryName("labelForPasses");
			this.add(helpInfoLevel2);
		} else {
			textForQuery = new DefaultTextBox(defaultText);
			textForQuery.setWidth("90%");
			textForQuery.addKeyPressHandler(new KeyPressHandler() {
				@Override
				public void onKeyPress(KeyPressEvent event) {
					boolean enterPressed = KeyCodes.KEY_ENTER == event
							.getNativeEvent().getKeyCode();
					if (enterPressed) {
						defineButton.click();
					}
				}
			});
			
			this.add(textForQuery);
		}

		if (typeOfQuery.equals("passThrough") || typeOfQuery.equals("pass")) {
			
			defineButton = new Button("define");
			defineButton.setStyleName("floatRight");
			defineButton.addStyleName("gwt-Button");
			this.add(defineButton);
			
			resultInfoLabel=new Label();
			this.add(resultInfoLabel);
			
			previousTuple.setStyleName("previousButton");
			previousTuple.setEnabled(false);
			
			nextTuple.setStyleName("nextButton");
			nextTuple.setEnabled(true);			
			
			FlexTable previousNextPanel = new FlexTable();
			previousNextPanel.setWidget(0, 0, previousTuple);
			previousNextPanel.setWidget(0, 1, nextTuple);			

			infoAboutTupleNo = new Label();
			vertPanWithPreviousNextTuple.add(infoAboutTupleNo);
			vertPanWithPreviousNextTuple.add(previousNextPanel);
			vertPanWithPreviousNextTuple.setVisible(false);
			this.add(vertPanWithPreviousNextTuple);

		} else {
			defineButton = new Button("define");
			defineButton.setStyleName("floatRight");
			defineButton.addStyleName("gwt-Button");
			this.add(defineButton);			
			
			resultInfoLabel = new Label();
			this.add(resultInfoLabel);
		}		
	}

	/**
	 * Returns the user input used to build a query
	 * 
	 * @return The user input in a text box
	 */
	public DefaultTextBox getLabelTextForQuery() {
		return textForQuery;
	}

	

	/**
	 * Returns the label with result
	 * 
	 * @return The label with result
	 */
	public Label getResultInfoLabel() {
		return resultInfoLabel;
	}	

	/**
	 * Returns the button "count"
	 * 
	 * @return The button "count"
	 */
	public Button getDefineButton() {
		return defineButton;
	}

	public Button getNextTuple() {
		return nextTuple;
	}

	public Button getPreviousTuple() {
		return previousTuple;
	}

	/**
	 * @return the vertPanWithPreviousNextTuple
	 */
	public VerticalPanel getVertPanWithPreviousNextTuple() {
		return vertPanWithPreviousNextTuple;
	}

	/**
	 * @return the infoAboutTupleNo
	 */
	public Label getInfoAboutTupleNo() {
		return infoAboutTupleNo;
	}

	
}
