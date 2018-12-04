package mapviewer;

import java.awt.Rectangle;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.List;

import org.jxmapviewer.JXMapViewer;
import org.jxmapviewer.viewer.GeoPosition;

import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.Region;
import stlib.interfaces.spatial.PointIF;

public class SelectionAdapter extends MouseAdapter {

   private boolean dragging;
   private JXMapViewer viewer;
   private MapViewerSurface mvSurface;

   private Point2D startPos = new Point2D.Double();
   private Point2D endPos = new Point2D.Double();

   /**
    * @param viewer
    *           the jxmapviewer
    */
   public SelectionAdapter(JXMapViewer viewer, MapViewerSurface mvSurface) {
      this.viewer = viewer;
      this.mvSurface = mvSurface;
   }

   @Override
   public void mousePressed(MouseEvent e) {

      if (e.getButton() != MouseEvent.BUTTON3 || mvSurface.getFilterType() == FilterType.NO_FILTER) {
         return;
      }

      startPos.setLocation(e.getX(), e.getY());
      endPos.setLocation(e.getX(), e.getY());

      dragging = true;
   }

   @Override
   public void mouseDragged(MouseEvent e) {
      if (!dragging) {
         return;
      }

      endPos.setLocation(e.getX(), e.getY());

      viewer.repaint();
   }

   @Override
   public void mouseReleased(MouseEvent e) {
      if (!dragging) {
         return;
      }

      if (e.getButton() != MouseEvent.BUTTON3) {
         return;
      }

      if (mvSurface.getFilterType() != FilterType.NO_FILTER) {
         double minX = Math.min(startPos.getX(), endPos.getX());
         double minY = Math.min(startPos.getY(), endPos.getY());
         double maxX = Math.max(startPos.getX(), endPos.getX());
         double maxY = Math.max(startPos.getY(), endPos.getY());

         GeoPosition gp0 = viewer.convertPointToGeoPosition(new Point2D.Double(minX, minY));
         GeoPosition gp1 = viewer.convertPointToGeoPosition(new Point2D.Double(minX, maxY));
         GeoPosition gp2 = viewer.convertPointToGeoPosition(new Point2D.Double(maxX, maxY));
         GeoPosition gp3 = viewer.convertPointToGeoPosition(new Point2D.Double(maxX, minY));

         List<PointIF> boundaryPoints = new ArrayList<>();

         boundaryPoints.add(new Point(gp0.getLatitude(), gp0.getLongitude()));
         boundaryPoints.add(new Point(gp1.getLatitude(), gp1.getLongitude()));
         boundaryPoints.add(new Point(gp2.getLatitude(), gp2.getLongitude()));
         boundaryPoints.add(new Point(gp3.getLatitude(), gp3.getLongitude()));

         mvSurface.addFilter(new Region(boundaryPoints), mvSurface.getFilterType());
      }

      viewer.repaint();

      dragging = false;
   }

   /**
    * @return the selection rectangle
    */
   public Rectangle getRectangle() {
      if (dragging) {
         int x1 = (int) Math.min(startPos.getX(), endPos.getX());
         int y1 = (int) Math.min(startPos.getY(), endPos.getY());
         int x2 = (int) Math.max(startPos.getX(), endPos.getX());
         int y2 = (int) Math.max(startPos.getY(), endPos.getY());

         return new Rectangle(x1, y1, x2 - x1, y2 - y1);
      }

      return null;
   }

   @Override
   public void mouseMoved(MouseEvent e) {

      GeoPosition gp = viewer.convertPointToGeoPosition(e.getPoint());
      int zoom = viewer.getZoom();

      mvSurface.setTitle(String.format(mvSurface.getInitialTitle() + " ( %.3f, %.3f ) Zoom: %d", gp.getLatitude(),
            gp.getLongitude(), zoom));

   }

}
