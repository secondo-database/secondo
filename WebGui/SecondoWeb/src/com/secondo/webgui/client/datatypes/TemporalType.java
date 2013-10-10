package com.secondo.webgui.client.datatypes;

import java.util.ArrayList;

public interface TemporalType {
	
	/** Returns the name of the datatype*/
	public String getName();
	
	/**Returns the spatial location of a geometric object*/
	public ArrayList<? extends Object> getLocation();
	
	/**Sets the spatial location of a geometric object*/
	public void setLocation(ArrayList<Object> location);
	
	/**Returns the surrounding Rectangle of a geometric object, which is necessary to scale the object*/
	public Rectangle getBounds();
	
	/**Sets the surrounding Rectangle of a geometric object*/
	public void setBounds(Rectangle bounds);

}
