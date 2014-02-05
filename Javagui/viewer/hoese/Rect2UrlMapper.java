

package viewer.hoese;

import java.util.LinkedList;
import java.net.URL;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;


import tools.Pair;

public interface Rect2UrlMapper{

  public LinkedList<Pair<URL, AffineTransform>> computeURLs(Rectangle2D.Double bboxi, AffineTransform world2screen);
  
  public double getAntiScale(AffineTransform at, Rectangle2D clipRect);

  public int numberOfPreloadFiles(Rectangle2D.Double clipRect);

  public LinkedList<URL> getPreloadURLs(Rectangle2D.Double clipRect);


}
