package mapviewer;

import java.awt.Graphics2D;

import javax.swing.DefaultListModel;

import org.jxmapviewer.JXMapViewer;
import org.jxmapviewer.painter.Painter;

import mapviewer.features.MapPaintable;

/**
 * 
 * @author Markus Fuessel
 *
 */
public class ObjectListPainter<T extends MapPaintable> implements Painter<JXMapViewer> {

   private DefaultListModel<T> mapPaintableModel;

   public ObjectListPainter(DefaultListModel<T> mapPaintableModel) {

      this.mapPaintableModel = mapPaintableModel;

   }

   @Override
   public void paint(Graphics2D gIn, JXMapViewer map, int w, int h) {

      for (int i = 0; i < mapPaintableModel.size(); i++) {

         mapPaintableModel.get(i).draw(gIn, map);

      }
   }

}
