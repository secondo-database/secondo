package com.secondo.webgui.utils.config;

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

/**
 * @return the value
 */
public String getValue() {
	return value;
}

/**
 * @return the index
 */
public int getIndex() {
	return index;
}
}
