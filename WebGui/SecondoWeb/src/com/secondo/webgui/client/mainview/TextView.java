package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextArea;


public class TextView extends Composite {
	
	private TextArea textOutput = new TextArea();
	private ArrayList<String> currentTextList = new ArrayList<String>();
	
	public TextView(){
		
		  textOutput.setEnabled(false);
		  textOutput.getElement().setClassName("textoutput");
		  textOutput.setWidth("300px");		
	}
	

	/**Resize the height of the textview with commandpanel*/
	public void resizeWithCP(int height){
		
		textOutput.setHeight(height-362 + "px"); //add 40px padding
	}
	
	/**Resize the height of the textview to fullscreen*/
    public void resizeToFullScreen(int height){

		textOutput.setHeight(height-132+"px");
	}
    
    /**Update the text view with the current data from the last secondo call*/
    public void updateCurrentResult(){
    	//reset text view
		textOutput.setText("");

		String currentResult = "";
		
		//add data from formatted result list to view
		for (String data : currentTextList){	
			
			currentResult = textOutput.getText();
			textOutput.setText(currentResult + data);
		}
    }

	public TextArea getTextOutput() {
		return textOutput;
	}

	public void setTextOutput(TextArea textOutput) {
		this.textOutput = textOutput;
	}


	public ArrayList<String> getCurrentTextList() {
		return currentTextList;
	}


	public void setCurrentTextList(ArrayList<String> currentTextList) {
		this.currentTextList = currentTextList;
	}

}
