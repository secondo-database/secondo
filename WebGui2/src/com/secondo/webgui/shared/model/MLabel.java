package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;

import com.secondo.webgui.utils.config.Color;

public class MLabel implements DataType, Serializable{
	
	private static final long serialVersionUID = -8592716841354310710L;
	private int id = 0;
	private String name = "MLabel";
	private String type = "MLabel";
	private ArrayList<TimeInterval> time = new ArrayList<TimeInterval>();
	private ArrayList<String> labelList = new ArrayList<String>(); 
	private ArrayList<Line> path = new ArrayList<Line>();
	private ArrayList<String> colorOfLines = new ArrayList<String>();
	private String color = "red";
	private ArrayList<String> attributeList = new ArrayList<String>();
	private String attributeNameInRelation="";
	
	public MLabel(){	
	}

	@Override
	public int getId() {		
		return id;
	}

	@Override
	public void setId(int id) {
		this.id =id;
		
	}

	@Override
	public String getType() {		
		return type;
	}

	@Override
	public String getName() {		
		return name;
	}

	
	@Override
	public void setName(String name) {
		this.name=name;
		
	}

	@Override
	public String getColor() {		
		return color;
	}

	@Override
	public void setColor(String color) {
		this.color=color;
		
	}

	@Override
	public ArrayList<String> getAttributeList() {		
		return attributeList;
	}
	
	public ArrayList<TimeInterval> getTime() {
		return time;
	}

	
	public void setTime(ArrayList<TimeInterval> time) {
		this.time = time;
	}
	
	public ArrayList<String> getLabel() {
		return labelList;
	}

	
	public void setLabel(ArrayList<String> label) {
		this.labelList = label;			
	}

	/**
	 * @return the path
	 */
	public ArrayList<Line> getPath() {
		return path;
	}

	/**
	 * @param path the path to set
	 */
	public void setPath(ArrayList<Line> path) {
		this.path = path;
	}
	
	public void generateColorsForLabel(int i){
		
			
			colorOfLines.add(Color.getHexValueForElementAt(i));
		
	}
	
	public void generateColorsForDuplicateLabel(int fistOccurrence){
		
		
		colorOfLines.add(colorOfLines.get(fistOccurrence));
	
}
	
	HashMap<String, String> mapWithLabelsAndColors = new HashMap<String, String>();
	public void generateColorsForLabel2(String label){
		if(!mapWithLabelsAndColors.containsKey(label)){
			int i = mapWithLabelsAndColors.size();
			mapWithLabelsAndColors.put(label, Color.getHexValueForElementAt(i));
			System.out.println("To label "+ label+ " generated color "+ mapWithLabelsAndColors.get(label));
		}
		
		
	
}

	public String getColorFromMapWithLabelsAndColors(String label) {
		if (mapWithLabelsAndColors.containsKey(label)) {
			return mapWithLabelsAndColors.get(label);
		} else {
			return "#000000";
		}
	}

	/**
	 * @return the attributeNameInRelation
	 */
	public String getAttributeNameInRelation() {
		return attributeNameInRelation;
	}

	/**
	 * @param attributeNameInRelation the attributeNameInRelation to set
	 */
	public void setAttributeNameInRelation(String attributeNameInRelation) {
		this.attributeNameInRelation = attributeNameInRelation;
	}
	
	

}
