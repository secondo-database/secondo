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
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.TextArea;

/**
*  This class represents the raw data view containing the nested list result of queries to the secondo database.
*  
*  @author Kristina Steiger
*  
**/
public class RawDataView extends Composite{
	
	/**The main panel of the raw data view*/
	private FlowPanel contentPanel = new FlowPanel();
	
	/**The output textarea for the result text*/
	private TextArea rawDataOutput = new TextArea();
	
	/**The resultlist for the secondo query results*/
	private ArrayList<String> resultList = new ArrayList<String>();
	
	public RawDataView(){
		
		rawDataOutput.setEnabled(false);
		rawDataOutput.getElement().setClassName("rawdataoutput");
		
		contentPanel.getElement().setClassName("rawdataview");
		contentPanel.add(rawDataOutput);	
	}
	
    /**On resizing of the browser window the elements of the raw data view are readjusted with the commandpanel displayed
	 * 
	 * @param width The new width of the raw data view
	 * @param height The new height of the raw data view
	 * */
	public void resizeWithCP(int width, int height){
		
		if(width > 1000){
			//add 20 pixel for the padding + 1 for the border
			rawDataOutput.setWidth(width-91 + "px");
			contentPanel.setWidth(width-71 + "px");
		}
		else{
			rawDataOutput.setWidth(1000-91 + "px");
			contentPanel.setWidth(1000-71 + "px");
		}
		if(height > 650){
			//add 20 pixel for the padding + 1 for the border + + 50 header + 10 + 260 commandpanel 
			rawDataOutput.setHeight(height-341 + "px");		
			contentPanel.setHeight(height-321 + "px");
		}		
		else{
			rawDataOutput.setHeight(650-341 + "px");		
			contentPanel.setHeight(650-321 + "px");
		}
	}
	
	/**On resizing of the browser window the elements of the raw data view are readjusted with to fullscreen
	 * 
	 * @param width The new width of the raw data view
	 * @param height The new height of the raw data view
	 * */
    public void resizeToFullScreen(int width, int height){
    	
    	if(width > 1000){
			//add 20 pixel for the padding + 1 for the border
			rawDataOutput.setWidth(width-91 + "px");
			contentPanel.setWidth(width-71 + "px");
		}
    	else{
			rawDataOutput.setWidth(1000-91 + "px");
			contentPanel.setWidth(1000-71 + "px");
		}
    	if(height > 650){
    		//add 20 pixel for the padding + 1 for the border + + 50 header + 10 + 30 statusbar
    		rawDataOutput.setHeight(height-111+"px");		
    		contentPanel.setHeight(height-91+"px");
    	}
    	else{
			rawDataOutput.setHeight(650-111 + "px");		
			contentPanel.setHeight(650-91 + "px");
		}
	}
    
    /**Updates the view with all data from the result list*/
    public void updateRawDataView(){
    	
    	//reset text view
    	rawDataOutput.setText("");
    	String currentResult = "";
    			
    	//add data from the result list to the text view
    	for (String result : resultList){ 		
    		currentResult = rawDataOutput.getText();
 		    rawDataOutput.setText(currentResult + "\n\n********Raw Data Result********\n\n" + result);
    	}
    }
    
    /**Removes all data from the textarea and the resultlist*/
    public void resetData(){
    	
    	rawDataOutput.setText("");  	
    	resultList.clear();
    }

    /**Returns the textarea to display the raw data text
     * 
     * @return The textarea to display the raw data text
     * */
	public TextArea getRawDataOutput() {
		return rawDataOutput;
	}

	/**Returns the resultlist of all secondo query results
     * 
     * @return The resultlist of all secondo query results
     * */
	public ArrayList<String> getResultList() {
		return resultList;
	}

	/**Returns the main panel of the raw data view
     * 
     * @return The main panel of the raw data view
     * */
	public FlowPanel getContentPanel() {
		return contentPanel;
	}
}
