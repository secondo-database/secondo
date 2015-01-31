package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.VerticalPanel;

public class SimpleQueryPanel extends VerticalPanel{
	DefaultTextBox labelTextForQuery; 
	Button queryButton;	
	Label resultInfoLabel;
	
	public SimpleQueryPanel(String textForHelpInfoLabel, String textForButton,
			String defaultText){
		super();
		
		this.setSpacing(4);
		Label helpInfoLabel = new Label(textForHelpInfoLabel);
		helpInfoLabel.setStylePrimaryName("labelForPasses");
		this.add(helpInfoLabel);
		labelTextForQuery = new DefaultTextBox(defaultText);
		labelTextForQuery.setWidth("90%");
		this.add(labelTextForQuery);
		queryButton = new Button(textForButton);
		queryButton.setStyleName("floatRight");
		this.add(queryButton);
		resultInfoLabel = new Label();
		this.add(resultInfoLabel);
		
	}

	public DefaultTextBox getLabelTextForQuery() {		
		return labelTextForQuery;
	}
	public Button getQueryButton() {
		return queryButton;
	}

	public Label getResultInfoLabel() {
		return resultInfoLabel;
	}
	
	
	
}
