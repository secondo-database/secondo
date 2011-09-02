/**
 * 
 */
package viewer.hoese;

import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.net.URL;
import java.util.LinkedList;

import tools.Pair;

/**
 * @author duentgen
 * 
 */
public class StaticOSMMapper implements Rect2UrlMapper {

	public StaticOSMMapper(int tileSizeX, int tileSizeY, int DIM_X, int DIM_Y,
    int minZoomLevel, int maxZoomLevel, URL baseUrl, String prefix) {
		this.tileSizeX = tileSizeX;
		this.tileSizeY = tileSizeY;
		this.DIM_X = DIM_X;
		this.DIM_Y = DIM_Y;
		this.minZoomLevel = minZoomLevel;
		this.maxZoomLevel = maxZoomLevel;
		this.baseUrl = baseUrl;
		this.prefix = prefix;
	}

	/**
	 * Computes a url from the x,y indexes at specified zoom level.
	 * Additionally, a affine transformation is computed to map the image to be
	 * retrieved from the url to the proper location within the world.
	 * 
	 * @see viewer.hoese.Rect2UrlMapper#computeURLs(java.awt.geom.Rectangle2D.Double)
	 * @param x
	 *            X index of a tile
	 * @param y
	 *            Y index of a tile
	 * @param z
	 *            zoom level
	 * @return A pair of image URL and affine transformation to shift it to its
	 *         proper location and zoom it according to zoom level and visible
	 *         screen
	 **/
	protected Pair<URL, AffineTransform> computeURL(int x, int y, int z) {
		URL url;
		try {
			url = new URL(baseUrl, "" + z + "/" + x + "/" + y + ".png");
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
		Rectangle2D.Double r = getBBoxForTile(x, y, z);
		AffineTransform at = computeTransform(r);

		return new Pair<URL, AffineTransform>(url, at);
	}


  /** computes the affine transformation to map an rectangle (0,0,tileSizeX, tileSizeY)
    * to the given rectangle.
    **/
	protected AffineTransform computeTransform(Rectangle2D.Double r) {
		// compute projected bounding box
		java.awt.geom.Point2D.Double p1 = new java.awt.geom.Point2D.Double();
		java.awt.geom.Point2D.Double p2 = new java.awt.geom.Point2D.Double();

		ProjectionManager.projectWithoutScale(r.getX(), r.getY(), p1);
		ProjectionManager.projectWithoutScale(r.getX() + r.getWidth(), r.getY() + r.getHeight(), p2);

		r.setRect(p1.getX(), p1.getY(), p2.getX() - p1.getX(),
				p2.getY() - p1.getY());

		double scale_x = r.getWidth() / (double) tileSizeX;
		double scale_y = -1.0 * r.getHeight() / (double) tileSizeY;

		AffineTransform at = AffineTransform.getTranslateInstance(r.getX(),
				r.getY() + r.getHeight());
		at.scale(scale_x, scale_y);
		return at;

	}


	/**
	 * Computes the urls required to cover at least the given rectangle.
	 * 
	 * @see viewer.hoese.Rect2UrlMapper#computeURLs(java.awt.geom.Rectangle2D.Double)
	 * @param bbox
	 *            the rectangle to be covered in world coordinates
	 * @return The list of all visible map tiles and according translation/scale
	 *         matrices
	 **/
	public LinkedList<Pair<URL, AffineTransform>> computeURLs(
			Rectangle2D.Double bbox) {
		if (bbox == null) {
			return null;
		}

		if (bbox.equals(lastClipRect)) {
			return lastURLs;
		}

		int zoom = computeZoomLevel(bbox.getWidth(), bbox.getHeight());

		int x1 = getTileX(zoom, bbox.getX());
		int x2 = getTileX(zoom, bbox.getX() + bbox.getWidth());
		int y1 = getTileY(zoom, bbox.getY() + bbox.getHeight());
		int y2 = getTileY(zoom, bbox.getY());

		if (x1 < 0) {
			x1 = 0;
		}
		if (x2 > ((1 << zoom) - 1)) {
			x2 = (1 << zoom) - 1;
		}
		if (y1 < 0) {
			y1 = 0;
		}
		if (y2 > ((1 << zoom) - 1)) {
			y2 = (1 << zoom) - 1;
		}

		LinkedList<Pair<URL, AffineTransform>> res = new LinkedList<Pair<URL, AffineTransform>>();
		for (int x = x1; x <= x2; x++) {
			for (int y = y1; y <= y2; y++) {
				Pair<URL, AffineTransform> p = computeURL(x, y, zoom);
				if (p != null) {
					res.add(p);
				} else {
					System.err
							.println("Problem in computinmg URL from (x,y,z) = ("
									+ x + ", " + y + ", " + zoom + ")");
				}
			}
		}
		lastURLs = res;
		return res;
	}

	/**
	 * returns the X index of a tile covering longitude at specified zoom level.
	 * 
	 * @param zoom
	 *            the used zoom level
	 * @param lon
	 *            longitude
	 * @return the X-index of the tile covering the given longitude
	 **/
	private int getTileX(int zoom, double lon) {
		return (int) Math.floor((lon + 180) / 360 * (1 << zoom));
	}

	/**
	 * returns the Y index of a tile covering latitude at specified zoom level.
	 * 
	 * @param zoom
	 *            the used zoom level
	 * @param lat
	 *            latitude
	 * @return the Y-index of the tile covering the given latitude
	 **/
	private int getTileY(int zoom, double lat) {
		return (int) Math.floor((1 - Math.log(Math.tan(Math.toRadians(lat)) + 1
				/ Math.cos(Math.toRadians(lat)))
				/ Math.PI)
				/ 2 * (1 << zoom));
	}

	/**
	 * compute a zoom level for a given size within the world.
	 * 
	 * @param witdh
	 *            size in x dimension within the world
	 * @param height
	 *            size in y dimension within the world
	 * @return The recommended zoom level
	 **/
	protected int computeZoomLevel(double width, double height) {
		double z_x = Math.log((360 * DIM_X) / (width * tileSizeX)) / l2; // computing
																			// log_2
		double z_y = Math.log((180 * DIM_Y) / (height * tileSizeY)) / l2;
		double z = Math.max(z_x, z_y);
		int zoom = (int) z;
		if (zoom > maxZoomLevel) {
			zoom = maxZoomLevel;
		}
		if (zoom < minZoomLevel) {
			zoom = minZoomLevel;
		}
		return zoom;
	}

	/**
	 * compute the bounding box covered by a specified tile in world
	 * coordinates.
	 * 
	 * @param x
	 *            X index of a tile
	 * @param y
	 *            Y index of a tile
	 * @param z
	 *            zoom level
	 * @return The images's MBR in world coordinates
	 **/
	protected Rectangle2D.Double getBBoxForTile(int x, int y, int zoom) {
		double north = tile2lat(y, zoom);
		double south = tile2lat(y + 1, zoom);
		double west = tile2lon(x, zoom);
		double east = tile2lon(x + 1, zoom);
		return new Rectangle2D.Double(west, south, east - west, north - south);
	}

	/**
	 * computes the western boundary of a specified tile in world coordinates.
	 * 
	 * @param x
	 *            The map tile's X-index
	 * @param z
	 *            The zoom level
	 * @return The longitude of the western boundary of a map tile
	 **/
	private static double tile2lon(int x, int z) {
		return x / Math.pow(2.0, z) * 360.0 - 180;
	}

	/**
	 * computes the northern boundary of a specified tile in world coordinates.
	 * 
	 * @param x
	 *            The map tile's Y-index
	 * @param z
	 *            The zoom level
	 * @return The latitude of the nortehrn boundary of a map tile
	 **/
	private static double tile2lat(int y, int z) {
		double n = Math.PI - (2.0 * Math.PI * y) / Math.pow(2.0, z);
		return Math.toDegrees(Math.atan(Math.sinh(n)));
	}

	public String toString() {
		return "tileSizeX = " + tileSizeX + ", tileSizeY = " + tileSizeY
				+ ", DIM_X = " + DIM_X + ", DIM_Y = " + DIM_Y
				+ ", minZoomLevel = " + minZoomLevel + ", maxZoomLevel = "
				+ maxZoomLevel + ", baseUrl = " + baseUrl;
	}

	/** the last used bounding box **/
	private Rectangle2D lastClipRect = null;

	/** the urls according to the last used bounding box **/
	LinkedList<Pair<URL, AffineTransform>> lastURLs;

	int tileSizeX;
	int tileSizeY;
	int DIM_X;
	int DIM_Y;
	int minZoomLevel;
	int maxZoomLevel;
	URL baseUrl;
	String prefix;

	/** logarithm of 2 to save computation time **/
	private static final double l2 = Math.log(2);

}
