//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.TextArea;

/**
*  This class represents the textpanel element of the application, which contains a textarea 
*  to display the secondo query results in formatted text
*  
*  @author Kristina Steiger
*  
**/
public class TextPanel extends Composite {
	
	/**The main panel of the textpanel*/
	private FlowPanel contentPanel = new FlowPanel();
	
	/**The textarea to display formatted text*/
	private TextArea textOutput = new TextArea();
	
	/**The resultlist for the secondo query results*/
	private ArrayList<ArrayList<String>> resultList = new ArrayList<ArrayList<String>>();
	
	/**A listbox to choose a result to display in the textarea*/
	private ListBox resultListBox = new ListBox(false);
	
	/**The defaulttext of the result list box*/
	private String defaultText = "Select Result...";
	
	private boolean dataLoaded = false;
	
	public TextPanel(){
		
		  textOutput.setEnabled(false);
		  textOutput.getElement().setClassName("textoutput");
		  textOutput.setWidth("290px");	//remove 10 for padding left and right
		  
		  resultListBox.setSize("300px", "25px");
		  resultListBox.addItem(defaultText);
		  
		  contentPanel.getElement().setClassName("textview");
		  contentPanel.setWidth("300px");
		  contentPanel.add(resultListBox);
		  contentPanel.add(textOutput);
		  
		  /*Adding a ClickHandler to the selection box to show the chosen query*/
	   	  resultListBox.addChangeHandler(new ChangeHandler() {
			      public void onChange(ChangeEvent event) {
		        	  
		        	  int selectedCommandIndex = resultListBox.getSelectedIndex();
		        	  
		        	  if(resultListBox.getItemText(selectedCommandIndex).equals("Select Result...")){		
		        		  //do nothing
						}
						else{	 					    	  
					    	//reset text view
					      	textOutput.setText("");
							String currentResult = textOutput.getText();
							
					    	for(String data: resultList.get(selectedCommandIndex -1)){  		
					    	   currentResult = textOutput.getText();
					 		   textOutput.setText(currentResult + data);
					 		}
						}       
		              }
		});
	}	

	/**On resizing of the browser window the elements of the text view are readjusted with the commandpanel displayed
	 * 
	 * @param height The new height of the main panel and the textarea of the text view
	 * */
	public void resizeWithCP(int height){
		
		if(height > 650){

			//10 + 50 header + 30 statusbar + 1 border + 230 commandpanel and menu
		   contentPanel.setHeight(height-321 + "px");
		   //add 10px padding + 30px resultlistbox in the textarea
		   textOutput.setHeight(height-361 + "px");
		}
		else{
			contentPanel.setHeight(650-321 + "px");
			textOutput.setHeight(650-361 + "px");
		}
	}
	
	/**On resizing of the browser window the elements of the text view are readjusted to fullscreen
	 * 
	 * @param height The new height of the main panel and the textarea of the text view
	 * */
    public void resizeToFullScreen(int height){

    	if(height > 650){

    		//10 + 50 header + 30 statusbar + 1 border
    		contentPanel.setHeight(height-91+"px");
    		//add 10px padding + 30px resultlistbox in the textarea
    		textOutput.setHeight(height-131 + "px");
    	}	
    	else{
    		contentPanel.setHeight(650-91+"px");
    		textOutput.setHeight(650-131 + "px");
    	}
	}
    
    /**Updates the text view with the last result from the result list*/
    public void updateTextView(){
    	
    	dataLoaded = false;
    	//reset text view
    	textOutput.setText("");
    	String currentResult = textOutput.getText();
    	
    	//add the last result from the list to the text output
    	int lastindex = resultList.size()-1;
    	for(String data: resultList.get(lastindex)){  				
 		   currentResult = textOutput.getText();
 		   textOutput.setText(currentResult + data);
 		}
    	
    	dataLoaded = true;
    }
    
    /**Deletes all data from the textview, the resultList and the resultlistBox*/
    public void resetData(){
    	
    	//reset text view
    	textOutput.setText("");   	
    	resultList.clear();  	
    	resultListBox.clear();
    	resultListBox.addItem(defaultText);   	
    }

    /**Returns the textarea to display formatted text
     * 
     * @return The textarea to display formatted text
     * */
	public TextArea getTextOutput() {
		return textOutput;
	}

	/**Returns the main panel of the text view
     * 
     * @return The main panel of the text view
     * */
	public FlowPanel getContentPanel() {
		return contentPanel;
	}

	/**Returns the result list with the secondo query results
     * 
     * @return The result list with the secondo query results
     * */
	public ArrayList<ArrayList<String>> getResultList() {
		return resultList;
	}
	
	/**Returns the result list box to choose a result
     * 
     * @return The result list box to choose a result
     * */
	public ListBox getResultListBox() {
		return resultListBox;
	}

	public boolean isDataLoaded() {
		return dataLoaded;
	}

	public void setDataLoaded(boolean dataLoaded) {
		this.dataLoaded = dataLoaded;
	}	
}
