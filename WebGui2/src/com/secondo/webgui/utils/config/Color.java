/**
 * 
 */
package com.secondo.webgui.utils.config;

/**
 * @author Irina Russkaya
 *
 */
public enum Color {
	Red("#FF0000"), 
	Seagreen("#2E8B57"), 
	Steelblue("#4682B4"), 
	Olive("#808000"), 
	Indigo("#4B0082"), 
	Black("#000000"), 
	Chocolate("#D2691E"), 
	Crimson("#DC143C"),
	Softgreen("#03c03c"),
	Red2("#c23b22"),
	Rosa("#ff77aa"),
	Yellow("#ffd526"),
	Violet("#800080");
	
	private String hex;
	
	private Color(String hex){
		this.hex=hex;
	}
	
	
	
	/**
	 * @return the hex
	 */
	public String getHex() {
		return hex;
	}



	/**
	 * cyclic return elements of enum 
	 * @return elements of enum
	 */
	public Color getNext() {
	return values()[(ordinal()+1) % values().length];
	}
	
	public static  String getHexValueForElementAt(int position){
		if(position+1<values().length){
		return values()[position].getHex();
		}
		else{
			return values()[(position+1)% values().length].getHex();
		}
	}

}
