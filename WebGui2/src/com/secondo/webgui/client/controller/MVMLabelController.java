package com.secondo.webgui.client.controller;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import com.secondo.webgui.shared.model.Line;
import com.secondo.webgui.shared.model.MLabel;

/**
 * This class is a controller for the datatype moving label, to display moving
 * labels in the map view.
 * 
 * @author Irina Russkaya
 *
 */
public class MVMLabelController {

	/**
	 * The list with all loaded mlabels
	 */
	private ArrayList<MLabel> mlabelArray = new ArrayList<MLabel>();
	/**
	 * The temporary map; combines colors in hex with line(derived from matching
	 * to mpoint)
	 */
	private HashMap<Line, String> mapWithColorsForPolyline = new HashMap<Line, String>();

	private ArrayList<String> colorsForPolyline = new ArrayList<String>();
	/**
	 * The map with IDs of mlabels as keys and the map matching to each line
	 * segment a special color as values
	 */
	private HashMap<Number, HashMap<Line, String>> mapWithLineSegmentsAndAssosiatedColors = new HashMap<Number, HashMap<Line, String>>();

	/**
	 * The map with unique key of moving label and its legend(what color
	 * symbolizes what label)
	 */
	private HashMap<Number, HashMap<String, String>> mapWithMLandLegend = new HashMap<Number, HashMap<String, String>>();

	public MVMLabelController() {

	}

	/**
	 * Returns the list with all loaded mlabels
	 * 
	 * @return the mlabelArray the list with all loaded mlabels
	 */
	public ArrayList<MLabel> getMlabelArray() {
		return mlabelArray;
	}

	/**
	 * Sets a list with mlabels
	 * 
	 * @param mlabelArray
	 *            the mlabelArray to set
	 */
	public void setMlabelArray(ArrayList<MLabel> mlabelArray) {
		this.mlabelArray = mlabelArray;
	}

	/**
	 * Clears the temporary data: a map combining colors in hex with lines and a
	 * list with colors for the current polyline
	 */
	public void clearmapWithColors() {
		mapWithColorsForPolyline.clear();
		colorsForPolyline.clear();
	}

	/**
	 * Returns the map with IDs of mlabels as keys and the map matching to each
	 * line segment a special color
	 * 
	 * @return the mapWithLineSegmentsAndAssosiatedColors
	 */
	public HashMap<Number, HashMap<Line, String>> getMapWithLineSegmentsAndAssosiatedColors() {
		return mapWithLineSegmentsAndAssosiatedColors;
	}

	/**
	 * Returns the temporary map mapping color to line
	 * 
	 * @return the mapWithColorsForPolyline
	 */
	public HashMap<Line, String> getMapWithColorsForPolyline() {
		return mapWithColorsForPolyline;
	}

	/**
	 * Returns a list with colors used for the polyline
	 * 
	 * @return the colorsForPolyline
	 */
	public ArrayList<String> getColorsForPolyline() {
		return this.colorsForPolyline;
	}

	/**
	 * Generates a map containing label name and the color used to show this
	 * name
	 * 
	 * @param data
	 *            the current mlabel
	 * @return the map mapping the label name and the color used to show this
	 *         name
	 */
	public Map<String, String> generateLegendFromMLabel(MLabel data) {
		Map<String, String> labelToColor = new HashMap<String, String>();
		for (int i = 0; i < data.getLabel().size(); i++) {
			String label = data.getLabel().get(i);
			labelToColor.put(label,
					data.getColorFromMapWithLabelsAndColors(label));
		}
		return labelToColor;
	}

	/**
	 * Sets a current legend to the map with all legends and maps it to the
	 * specified mlabel
	 * 
	 * @param key
	 *            the key identifying a mlabel
	 * @param labelToColor
	 *            the map containing the information for legend
	 */
	public void setLegendToMapWithAllLegends(int key,
			Map<String, String> labelToColor) {
		mapWithMLandLegend.put(key, (HashMap<String, String>) labelToColor);
	}

	/**
	 * Adds the specified mlabel to the list of all mlabels in this session
	 * 
	 * @param data
	 *            the current mlabel
	 */
	public void addMLabel(MLabel data) {
		Map<String, String> labelToColor = generateLegendFromMLabel(data);
		setLegendToMapWithAllLegends(data.getId(), labelToColor);
		getMlabelArray().add(data);
	}

}
