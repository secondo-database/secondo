/**
 * 
 */
package viewer.hoese;

import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.net.URL;

import tools.Pair;

/**
 * @author duentgen
 * 
 */
public class StaticGoogleMapsMapper extends StaticOSMMapper implements
		Rect2UrlMapper {

	public StaticGoogleMapsMapper(int tileSizeX, int tileSizeY, int DIM_X,
			int DIM_Y, int minZoomLevel, int maxZoomLevel, URL baseUrl,
			String prefix) {
		super(tileSizeX, tileSizeY, DIM_X, DIM_Y, minZoomLevel, maxZoomLevel,
				baseUrl, prefix);
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
	 * @return A pair: image URL and affine transformation to shift the image to
	 *         its proper location and zoom it according to zoom level and
	 *         visible screen
	 **/
	protected Pair<URL, AffineTransform> computeURL(int x, int y, int z) {
		URL url;
		Rectangle2D.Double r = getBBoxForTile(x, y, z);
		double centerX = r.getX() + r.getWidth() / 2.0;
		double centerY = r.getY() + r.getHeight() / 2.0;
		try {
			url = new URL(baseUrl, prefix + "x=" + x + "&y=" + y + "&z=" + z);

		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}

		AffineTransform at = computeTransform(r);
		return new Pair<URL, AffineTransform>(url, at);
	}

	private String mapType = "roadmap";

}
