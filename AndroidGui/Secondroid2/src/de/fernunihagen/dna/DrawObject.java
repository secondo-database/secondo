package de.fernunihagen.dna;

import de.fernunihagen.dna.hoese.Category;
import javamini.awt.geom.Rectangle2D;
import android.graphics.Path;

public class DrawObject {

	enum Type {
		POINT, LINE, FACE;
	}

	private Path path;
	private Type type;
	private Rectangle2D.Double bounds;
	private boolean isSelected = false;
	private Category category;

	public Path getPath() {
		return path;
	}

	public void setPath(Path path) {
		this.path = path;
	}

	public Type getObjectType() {
		return type;
	}

	public void setObjectType(Type objectType) {
		this.type = objectType;
	}

	public void setBounds(Rectangle2D.Double bounds) {
		this.bounds = bounds;
	}

	public Rectangle2D.Double getBounds() {
		return (this.bounds);
	}

	public boolean isSelected() {
		return isSelected;
	}

	public void setSelected(boolean isSelected) {
		this.isSelected = isSelected;
	}

	public Category getCategory() {
		return category;
	}

	public void setCategory(Category category) {
		this.category = category;
	}

}
