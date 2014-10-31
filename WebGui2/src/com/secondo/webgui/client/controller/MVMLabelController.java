package com.secondo.webgui.client.controller;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MLabel;
import com.secondo.webgui.shared.model.MPoint;
import com.secondo.webgui.shared.model.TimeInterval;


/**
 * 
 * 
 * @author secuser
 *
 */
public class MVMLabelController {
	

	/**
	 * list with all mlabels
	 */
	private ArrayList<MLabel> mlabelArray = new ArrayList<MLabel>();
	/**
	 * Temporary map; combines colors in hex with line(derived from matching to mpoint)
	 */
	private HashMap<Line, String> mapWithColorsForPolyline = new HashMap<Line, String>();
	
	private ArrayList<String> colorsForPolyline= new ArrayList<String>();
	/**
	 * Map with IDs of mlabels as keys and the map matching to each line segment a special color as values
	 */
	private HashMap<Number,HashMap< Line, String>> mapWithLineSegmentsAndAssosiatedColors= new HashMap<Number,HashMap<Line, String>>(); 
	
	
	/**map with unique key of moving label and its legend(what color symbolizes what label)*/
	private HashMap<Number, HashMap<String, String>> mapWithMLandLegend = new HashMap<>();
	
	public MVMLabelController() {
		
	}
	
	


	/**
	 * @return the mlabelArray
	 */
	public ArrayList<MLabel> getMlabelArray() {
		return mlabelArray;
	}


	/**
	 * @param mlabelArray the mlabelArray to set
	 */
	public void setMlabelArray(ArrayList<MLabel> mlabelArray) {
		this.mlabelArray = mlabelArray;
	}




	
	
	
	
	public void clearmapWithColors(){
		mapWithColorsForPolyline.clear();
		colorsForPolyline.clear();
		if(colorsForPolyline.isEmpty()){
		System.out.println("colorsForPolyline is empty!");}
	}




	/**
	 * @return the mapWithLineSegmentsAndAssosiatedColors
	 */
	public HashMap<Number, HashMap<Line, String>> getMapWithLineSegmentsAndAssosiatedColors() {
		return mapWithLineSegmentsAndAssosiatedColors;
	}




	/**
	 * @return the mapWithColorsForPolyline
	 */
	public HashMap<Line, String> getMapWithColorsForPolyline() {
		return mapWithColorsForPolyline;
	}




	/**
	 * @return the colorsForPolyline
	 */
	public ArrayList<String> getColorsForPolyline() {
		System.out.println("colorsForPolyline in LabelController " +this.colorsForPolyline.size());
		return this.colorsForPolyline;
	}




	
	
	public void resetAllValuesFromMLabelController(){
		mlabelArray.clear();
		clearmapWithColors();
	}
	
	public Map<String, String> generateLegendFromMLabel(MLabel data){
		Map<String, String> labelToColor = new HashMap<String, String>();
		for(int i=0; i<data.getLabel().size();i++){
			String label=data.getLabel().get(i);
			labelToColor.put(label, data.getColorFromMapWithLabelsAndColors(label));
		}
		return labelToColor;
		
	}	
	
	
	@SuppressWarnings("unchecked")
	public void setLegendToMapWithAllLegends(int key, Map<String, String> labelToColor){
		mapWithMLandLegend.put(key, (HashMap)labelToColor);
	}
	
	public HashMap<String, String> getLegendToMLabel(int key){
		return mapWithMLandLegend.get(key);
	}
	
	
	public void addMLabel(MLabel data){
		
		Map<String, String> labelToColor=generateLegendFromMLabel(data);
		setLegendToMapWithAllLegends(data.getId(), labelToColor);
		getMlabelArray().add(data);		

	}

}
