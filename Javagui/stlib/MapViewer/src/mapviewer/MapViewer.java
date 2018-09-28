package mapviewer;

import javax.swing.SwingUtilities;

/**
 * Main Class to start the MapViewer
 * 
 * @author Markus Fuessel
 */
public class MapViewer {

   public static void main(String[] args) {
      SwingUtilities.invokeLater(new Runnable() {

         public void run() {
            start();

         }
      });

   }

   static void start() {
      MapViewerSurface mvSurface = new MapViewerSurface("MapViewer by Markus Füssel");

      mvSurface.pack();
      mvSurface.setVisible(true);
   }
}
