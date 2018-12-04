package mapviewer.list;

import java.awt.Graphics2D;
import java.util.List;

import org.jxmapviewer.JXMapViewer;

import mapviewer.features.MapPaintable;

/**
 * 
 * @author Markus Fuessel
 *
 */
public class CheckboxListItemResult extends CheckboxListItem implements MapPaintable {

   static int counter = 0;

   private List<MapPaintable> paintableObjects;

   /**
    * Constructor with label
    * 
    * @param label
    * @param paintableObjects
    */
   public CheckboxListItemResult(final String label, final List<MapPaintable> paintableObjects) {
      this.label = "R" + ++counter + ": " + label;
      this.paintableObjects = paintableObjects;

   }

   /**
    * Constructor
    * 
    * @param label
    * @param paintableObjects
    */
   public CheckboxListItemResult(final List<MapPaintable> paintableObjects) {
      this.label = "R" + ++counter;
      this.paintableObjects = paintableObjects;

   }

   /*
    * (non-Javadoc)
    * 
    * @see mapviewer.features.MapPaintable#draw(java.awt.Graphics2D,
    * org.jxmapviewer.JXMapViewer)
    */
   @Override
   public void draw(Graphics2D gIn, JXMapViewer map) {
      if (isSelected()) {
         for (MapPaintable pObject : paintableObjects) {
            pObject.draw(gIn, map);
         }
      }

   }

}
