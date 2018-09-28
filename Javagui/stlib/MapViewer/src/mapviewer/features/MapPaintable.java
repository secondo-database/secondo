package mapviewer.features;

import java.awt.Graphics2D;

import org.jxmapviewer.JXMapViewer;

/**
 * Implemented by classes which objects should be paintabel on a map
 * 
 * @author Markus Fuessel
 */
public interface MapPaintable {

   public void draw(Graphics2D gIn, JXMapViewer map);

}
