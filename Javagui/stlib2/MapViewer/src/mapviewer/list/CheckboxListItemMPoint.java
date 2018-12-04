package mapviewer.list;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.List;

import org.jxmapviewer.JXMapViewer;
import org.jxmapviewer.viewer.GeoPosition;

import mapviewer.features.MapPaintable;
import stlib.datatypes.spatial.Point;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.HalfsegmentIF;
import stlib.interfaces.spatial.util.SegmentIF;
import stlib.operations.projection.Trajectory;

/**
 * This class represents list items for moving points
 * 
 * @author Markus Fuessel
 */
public class CheckboxListItemMPoint extends CheckboxListItem implements MapPaintable {
   private MovingPointIF mpoint;

   private List<SegmentIF> trajectoryLine;

   private Color color = Color.RED;
   private boolean antiAlias = true;

   /**
    * Constructor
    * 
    * @param label
    * @param mpoint
    */
   public CheckboxListItemMPoint(final String label, final MovingPointIF mpoint) {
      this.label = label;
      this.mpoint = mpoint;

      setTrajectoryLine();
   }

   /**
    * Copy constructor
    * 
    * @param other
    */
   public CheckboxListItemMPoint(final CheckboxListItemMPoint other) {
      this(other.label, other.mpoint);
      setSelected(other.isSelected());
   }

   public MovingPointIF getMPoint() {
      return mpoint;
   }

   /**
    * Get the initial start point of the moving point
    * 
    * @return
    */
   public PointIF getStartPoint() {
      if (!mpoint.isDefined()) {
         return new Point();
      }

      return mpoint.getUnit(0).getInitial();

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * de.markus.maven.MapViewer.features.MapPaintable#draw(java.awt.Graphics2D,
    * org.jxmapviewer.JXMapViewer)
    */
   @Override
   public void draw(Graphics2D gIn, JXMapViewer map) {

      if (isSelected()) {

         Graphics2D g = (Graphics2D) gIn.create();

         // convert from viewport to world bitmap
         Rectangle rect = map.getViewportBounds();
         g.translate(-rect.x, -rect.y);

         if (antiAlias) {
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
         }

         // do the drawing
         g.setColor(Color.BLACK);
         g.setStroke(new BasicStroke(4));

         drawRoute(g, map, trajectoryLine);

         // do the drawing again
         g.setColor(color);
         g.setStroke(new BasicStroke(2));

         drawRoute(g, map, trajectoryLine);

         g.dispose();
      }
   }

   /**
    * Set the trajectory line segments
    */
   private void setTrajectoryLine() {
      trajectoryLine = new ArrayList<>();
      List<HalfsegmentIF> segments = Trajectory.execute(mpoint).getHalfsegments();

      for (HalfsegmentIF hs : segments) {
         if (hs.isLeftDominating()) {
            trajectoryLine.add(hs);
         }

      }
   }

   /**
    * @param g
    *           the graphics object
    * @param map
    *           the map
    */
   private static void drawRoute(Graphics2D g, JXMapViewer map, List<SegmentIF> segments) {

      for (SegmentIF seg : segments) {

         // convert geo-coordinate to world bitmap pixel
         GeoPosition gp1 = new GeoPosition(seg.getLeftPoint().getXValue(), seg.getLeftPoint().getYValue());
         GeoPosition gp2 = new GeoPosition(seg.getRightPoint().getXValue(), seg.getRightPoint().getYValue());

         Point2D pt1 = map.getTileFactory().geoToPixel(gp1, map.getZoom());
         Point2D pt2 = map.getTileFactory().geoToPixel(gp2, map.getZoom());

         g.drawLine((int) pt1.getX(), (int) pt1.getY(), (int) pt2.getX(), (int) pt2.getY());

      }
   }

}