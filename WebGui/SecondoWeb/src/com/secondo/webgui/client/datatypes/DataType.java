package com.secondo.webgui.client.datatypes;

import java.util.ArrayList;


public interface DataType {
	
	/** Returns the name of the datatype*/
	public String getName();

	/**Gets the textual representation of the datatype*/
	public ArrayList<String> getTextList();
	
	/**Sets the textual representation of the datatype*/
	public void setTextList(ArrayList<String> textlist);	


}
