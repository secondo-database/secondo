package mapviewer.list;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Point2D;
import java.security.cert.CertPathChecker;
import java.util.ArrayList;
import java.util.List;

import org.jxmapviewer.JXMapViewer;
import org.jxmapviewer.viewer.GeoPosition;

import mapviewer.FilterType;
import mapviewer.features.MapPaintable;
import stlib.interfaces.spatial.RegionIF;
import stlib.interfaces.spatial.util.HalfsegmentIF;
import stlib.interfaces.spatial.util.SegmentIF;

/**
 * This class represents list items for region objects, which would be used as a
 * spatial filter
 * 
 * @author Markus Fuessel
 *
 */
public class CheckboxListItemFilter extends CheckboxListItem implements MapPaintable {

   static int counter = 0;

   private RegionIF regionFilter;
   private FilterType filterType;

   private List<SegmentIF> boundarySegments;

   private Color color = Color.BLUE;
   private boolean antiAlias = true;

   /**
    * Constructor
    * 
    * @param label
    * @param regionFilter
    */
   public CheckboxListItemFilter(final String label, final RegionIF regionFilter, final FilterType filterType) {
      this.label = label;
      this.regionFilter = regionFilter;
      this.filterType = filterType;

      setboundarySegments();
   }

   /**
    * Constructor
    * 
    * @param label
    * @param regionFilter
    */
   public CheckboxListItemFilter(final RegionIF regionFilter, final FilterType filterType) {
      this.label = ++counter + " " + filterType;
      this.regionFilter = regionFilter;
      this.filterType = filterType;

      setboundarySegments();
   }

   /**
    * Copy constructor
    * 
    * @param other
    */
   public CheckboxListItemFilter(final CheckboxListItemFilter other) {
      this(other.label, other.regionFilter, other.filterType);
      setSelected(other.isSelected());
   }

   /**
    * @return the regionFilter
    */
   public RegionIF getRegionFilter() {
      return regionFilter;
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

         drawRoute(g, map, boundarySegments);

         // do the drawing again
         g.setColor(color);
         g.setStroke(new BasicStroke(2));

         drawRoute(g, map, boundarySegments);

         g.dispose();
      }

   }

   /**
    * Set the trajectory line segments
    */
   private void setboundarySegments() {
      boundarySegments = new ArrayList<>();

      List<HalfsegmentIF> segments = regionFilter.getHalfsegments();

      for (HalfsegmentIF hs : segments) {
         if (hs.isLeftDominating()) {
            boundarySegments.add(hs);
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
