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
public class StaticGoogleMapsMapper2 extends StaticOSMMapper implements
		Rect2UrlMapper {

	public StaticGoogleMapsMapper2(int tileSizeX, int tileSizeY, int DIM_X,
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
      String file="staticmap?";
      file+=  "center="+centerY+","+centerX +
              "&zoom="+(z)+
              "&size="+tileSizeX+"x"+(tileSizeY+40)+
              "&format=png" + 
              "&maptype="+ mapType +  // roadmap, satellite, hybrid, terrain
              "&mobile=false"+
              "&sensor=false";
			url = new URL(baseUrl, file);

		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}

		AffineTransform at = computeTransform(r);
		return new Pair<URL, AffineTransform>(url, at);
	}


  


	private String mapType = "roadmap";

}
