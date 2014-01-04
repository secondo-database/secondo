package de.fernunihagen.dna.hoese;

import java.io.Serializable;

/*
   Category handles the attributes for painting like color and pointsize
*/
public class Category implements Serializable{
	int color = -6025450; // Default red
	int selectedColor = -16053337; // Blue
	int strokeWidth = 4;
	int alpha = 150;

	public static Category getDefaultCat() {
		return new Category();
	}

	public String getName() {
		return "";
	}

	public double getPointSize(RenderAttribute renderAttribute,
			double actualTime) {
		return 1;
	}

	public boolean getPointasRect() {
		return false;
	}

	public int getColor() {
		return color; 
	}
	public int setColor(int color) {
		return this.color = color; 
	}

	public int getSelectedColor() {
		return selectedColor; 
	}
	public int setSelectedColor(int color) {
		return this.selectedColor = color; 
	}

	public int getStrokeWidth() {
		return strokeWidth;
	}
	public void setStrokeWidth(int strokeWidth) {
		this.strokeWidth = strokeWidth;
	}
	
	public int getAlpha() {
		return alpha;
	}
	public void setAlpha(int alpha) {
		this.alpha = alpha;
	}
	
}
