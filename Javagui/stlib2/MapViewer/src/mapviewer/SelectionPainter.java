package mapviewer;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;

import org.jxmapviewer.JXMapViewer;
import org.jxmapviewer.painter.Painter;
import org.jxmapviewer.viewer.GeoPosition;

public class SelectionPainter implements Painter<JXMapViewer> {
   private Color fillColor = new Color(128, 192, 255, 128);
   private Color frameColor = new Color(0, 0, 255, 128);

   private SelectionAdapter adapter;

   /**
    * @param adapter
    *           the selection adapter
    */
   public SelectionPainter(SelectionAdapter adapter) {
      this.adapter = adapter;
   }

   @Override
   public void paint(Graphics2D g, JXMapViewer viewer, int width, int height) {
      Rectangle rc = adapter.getRectangle();

      if (rc != null) {
         GeoPosition gp = viewer.convertPointToGeoPosition(rc.getLocation());

         g.setColor(frameColor);
         g.draw(rc);
         g.setColor(fillColor);
         g.fill(rc);
      }
   }
}