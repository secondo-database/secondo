package com.secondo.webgui.utils.config;

/**The enum with modes for displaying a symbolic trajectory
 * @author Irina Russkaya
 *
 */
public enum ModesToShowSymTraj {

NoModes(0, ""),
SHOWwithColor(3, "color mode"), 
SHOWwithPopup(2, "popup mode"),
SHOWwithLabel(1, "label mode");
private int index;
private String value;

private ModesToShowSymTraj(int index, String value){
	this.index=index;
	this.value=value;
	
}

/**Returns the value of the enum item
 * 
 * @return the value
 */
public String getValue() {
	return value;
}

/**Returns the index of the enum item
 * 
 * @return the index
 */
public int getIndex() {
	return index;
}
}
