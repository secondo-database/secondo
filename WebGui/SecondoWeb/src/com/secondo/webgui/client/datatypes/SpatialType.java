package com.secondo.webgui.client.datatypes;

import java.util.ArrayList;

public interface SpatialType{


	/** Returns the name of the datatype*/
	public String getName();
	
	/**Gets the textual representation of the datatype*/
	public ArrayList<String> getTextList();
	
	/**Sets the textual representation of the datatype*/
	public void setTextList(ArrayList<String> textlist);
	
	/**Returns the spatial location of a geometric object*/
	public ArrayList<? extends Object> getLocation();
	
	/**Sets the spatial location of a geometric object*/
	public void setLocation(ArrayList<Object> location);
	
	/**Resets the location value to the default value*/
	public void resetLocation();
	
	/**Returns the surrounding Rectangle of a geometric object, which is necessary to scale the object*/
	public Rectangle getBounds();
	
	/**Sets the surrounding Rectangle of a geometric object*/
	public void setBounds(Rectangle bounds);
	
	
}
