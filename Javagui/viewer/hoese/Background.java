package viewer.hoese;

import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Properties;
import java.util.StringTokenizer;

import javax.swing.JComponent;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * This class is to be extended by classes representing different kinds of
 * backgrounds in the HoeseViever, e.g. SimpleBackground, ImageBackground, or
 * TiledBackground.
 *
 * Specializations of this class should override the inherited paint() method.
 *
 * @author Christian Duentgen
 */
public abstract class Background {

	private static final long serialVersionUID = 9039714158763811782L;
  /**
   * The name of the Background
   */
  protected String name;

  /**
   * A license string to be displayed with the background
   */
  protected String license;

  /**
   * The boundary of the complete background.
   */
  protected Rectangle2D.Double bbox;


  /**
   * Structure to maintain the set of registered listeners
   */
  protected LinkedList<BackgroundListener> listeners;

  /**
   * Whether the Background's bounding box is considered in calculation of the
   * world's bounding box.
   */
  protected boolean useforbbox;

  /**
   * String constant used as key for license within Property
   */
  public static final String KEY_LICENSE = "license";

  /**
   * String constant used as key for name within Property
   */
  public static final String KEY_NAME = "name";

  /**
   * String constant used as key for bbox within Property
   */
  public static final String KEY_BBOX = "bbox";

  /**
   * String constant used as key for useforbbox within Property
   */
  public static final String KEY_USEFORBBOX = "useforbbox";

  /**
   * String constant used as key for the background class name within Property
   */
  public static final String KEY_BACKGROUNDCLASSNAME = "backgroundclassname";


  @Override
  public String toString(){
     return  getClass().getName()+"["+
                  "name = " + name + ", " +
                  "license = " + license + ", " +
                  "useforbbox = " + useforbbox + ", " +
                  "bbox = " + bbox +  "] ";
  }


  /** Paints the background to g using the clipRect as bounding box and at as
    * the transformation from world to screen coordinates.
   *
  **/
  public abstract void paint(JComponent parent, Graphics2D g, AffineTransform at, Rectangle2D clipRect);
  

  /** Returns a zoom value ensring that only a certain zoom level
    * should be used, e.g., in a tiled map to ensure a non resized tile view.
    **/
  public double getAntiScale( AffineTransform at, Rectangle2D clipRect){
     return 1.0;
  }

  /** Returns the zoom factor for single zoom in, zoom out.
    **/
  public double getZoomStep(){
     return 1.25;
  }

  /** Returns true, if preload is supported. **/
  public boolean supportsPreload(){
    return false;
  }

  /** Returns the number of files to download if preloading is started
    **/
  public int numberOfPreloadFiles(Rectangle2D.Double clipRect){
     return 0;
  }

  /** starts preloading of files.
    **/
  public void preload(Rectangle2D.Double clipRect, PreloadObserver observer){
  }

  /** Cancels a running preloading **/
  public void cancelPreload() {

  }



	/**
	 * The constructor sets all member variables to null. SInce this is an
	 * abstract class, it should never be used anyway.
	 **/
	public Background() {
		name = null;
		license = null;
		bbox = null;
		listeners = new LinkedList<BackgroundListener>();
	}


	/**
	 * Sets the Background's boundary (world coordinates)
	 * @param rect
	 *            A Rectangle2D defining the Background's boundary
	 */
	public void setBBox(Rectangle2D.Double rect) {
		bbox = (Rectangle2D.Double) rect.clone();
	}

	/**
	 * Returns the Background's boundary (world coordinates)
	 * @return The Background's boundary
	 */
	public Rectangle2D.Double getBBox() {
		return bbox;
	}



	/**
	 * Display a dialog to allow a user to set up parameters for the Background.
	 * @param parent
	 *            The parent of the dialog component used.
	 */
	public abstract void showConfigDialog(JComponent parent);

	/**
	 * Set up the Background with parameters given as a Properties object. Can
	 * be used to restore the Background settings, e.g. from a file. All
	 * registered Listeners are informed. Additional data may be retrieved from
	 * files rooted at the given path.
	 *
	 * @param p
	 *            The Background settings to restore.
	 * @param backgrounddatapath
	 *            A path to a directory where additional data is stored in
	 *            files.
	 */
	public void setConfiguration(Properties p, String backgrounddatapath) {
		String n = p.getProperty(KEY_NAME);
		if (n == null) {
			Reporter.writeWarning("Could not set Background property: "
					+ KEY_NAME + " not found).");
			name = "Unnamed " + this.getClass().getName();
		} else {
			name = n;
		}
		n = p.getProperty(KEY_LICENSE);
		if (n == null) {
			Reporter.writeWarning("Could not set Background property: "
					+ KEY_LICENSE + " not found).");
			n = "";
		} else {
			license = null;
		}
		n = p.getProperty(KEY_BBOX);
		if (n == null) {
			Reporter.writeWarning("Could not set Background property: "
					+ KEY_BBOX + " not found).");
			bbox = null;
		} else {
			Rectangle2D.Double b = createRectangle2DFromString(n);
			if (b != null) {
				bbox = b;
			}
		}
		n = p.getProperty(KEY_USEFORBBOX);
		useforbbox = (n != null) && (n == "yes");
	}

	/**
	 * Creates a {@link Rectangle2D.Double} from its String representation, e.g.
	 * to restore a rectangle from a value in a {@link Properties} object.
	 *
	 * @param s
	 *            The String to read from
	 * @return The Rectangle2D.Double represented by the String parameter
	 */
	public static Rectangle2D.Double createRectangle2DFromString(String s) {
		if (s == null) {
			return null;
		}
		StringTokenizer st = new StringTokenizer(s);
		double[] values = new double[4];
		int pos = 0;
		while (st.hasMoreTokens() && pos < 4) {
			Double d = Double.valueOf(st.nextToken());
			if (d == null) {
				return null;
			}
			values[pos] = d.doubleValue();
			pos++;
		}
		if (st.hasMoreTokens() || pos < 4) {
			return null;
		}
		return new Rectangle2D.Double(values[0], values[1], values[2],
				values[3]);
	}

	/**
	 * Return the Background's settings as a Properties object. The Properties
	 * can be used to save Background settings for later reference. Additional
	 * data may be stored to files rooted at the given path.
	 *
	 * @param backgrounddatapath
	 *            Path, where to store data files.
	 * @return The Background's settings as a Properties object.
	 */
	public Properties getConfiguration(String backgrounddatapath) {
		Properties p = new Properties();
		p.setProperty(KEY_BACKGROUNDCLASSNAME, this.getClass().getName());
		if (name != null) {
			p.setProperty(KEY_NAME, name);
		}
		if (license != null) {
			p.setProperty(KEY_LICENSE, license);
		}
		if (bbox != null) {
			String bboxstr = "" + bbox.getX() + " " + bbox.getY() + " "
				+ bbox.getWidth() + " " + bbox.getHeight();
			p.setProperty(KEY_BBOX, bboxstr);
		}
		if(useforbbox) {
			p.setProperty(KEY_USEFORBBOX, "yes");
		} else {
			p.setProperty(KEY_USEFORBBOX, "no");
		}
		return p;
	}

	/**
	 * Add an Object as a listener to the Background. Listeners will be informed
	 * when the Background's content changed, so that it might be painted again.
	 * @param l
	 *            The Object to enlist as a listener.
	 */
	public final void addBackgroundListener(BackgroundListener l) {
		if (!listeners.contains(l)) {
			listeners.add(l);
		}
	}

	/**
	 * Removes an Object from the Background's listener list. That object will
	 * no longer be informed about changes to the Background.
	 * @param l
	 *            The Object to remove from the listener list.
	 */
	public final void removeBackgroundListener(BackgroundListener l) {
		listeners.remove(l);
	}

	/**
	 * This is a call-back method to be called by external objects requesting
	 * the Background to update it's internal data. E.g. it could be called by a
	 * FileDownloadManager after a requestes map tile download has been
	 * finished.
	 *
	 */
	public void handleBackgroundDataChangedEvent(BackgroundChangedEvent evt) {
		// empty implementation
	}

	/**
	 * Returns the Background's name, e.g. to allow to choose between different
	 * available Backgrounds.
	 * @return The Background's name.
	 */
	public final String getName() {
		return name;
	}

	/**
	 * Returns a text with a license to which the Background is bound. Many maps
	 * or pictures come with a license stating to display a copyright or license
	 * message together with the map/ picture. The returned license should be
	 * displayed at an appropriate place.
	 * @return The Background's license.
	 */
	public final String getLicense() {
		return license;
	}

	/**
	 * Informs all enlisted Listeners about an update of the Background's state.
	 */
	protected final void informListeners(BackgroundChangedEvent evt) {
		ListIterator<BackgroundListener> i = listeners.listIterator(0);
		while (i.hasNext()) {
			i.next().handleBackgroundChangedEvent(evt);
		}
	}

	/**
	 * Check the flag that indicates whether the Background is to be considered
	 * when calculation the world's dimensions.
	 *
	 * @return true, iff the Background's bbox is considered in world's bbox.
	 */
	public final boolean useForBoundingBox() {
		return useforbbox;
	}


	/**
	 * Creates a nested list representation of the Background's configuration,
	 * that can be saved as part of a saved GUI session. The provided path
	 * indicates a directory where additional data files may be stored.
	 *
	 * @param backgrounddatapath
	 *            Path to the data directory
	 * @return The listExpr representing this object.
	 */
	public ListExpr toListExpr(String backgrounddatapath) {
		Properties p = getConfiguration(backgrounddatapath);
		return ListExpr.fromProperties(p);
	}

	/**
	 * Restores the Background settings from a nested list representation of
	 * Background's configuration, so that the Background can be restored when
	 * loading a saved GUI session. The provided path indicates the directory
	 * with additional data files used for restoration.
	 *
	 * @param l
	 *            The listExpr representing this object.
	 * @param backgrounddatapath
	 *            Path to the data directory
	 */
	public void readFromList(ListExpr l, String backgrounddatapath) {
		Properties p = new Properties();
		ListExpr.toProperties(l, p); // ignore boolean result;
		setConfiguration(p, backgrounddatapath);
	}

	/**
	 * Factory method, that restores and returns any Background object
	 * from an Properties object. If restoration fails, a SimpleBackground is
	 * returned.
	 *
	 * @param p
	 *            The Properties to restore from.
	 * @param backgrounddatapath
	 *            A directory with additional data.
	 * @return The restored Background object.
	 */
	public static final Background createFromProperties(Properties p,
			String backgrounddatapath) {
		String bgclassname = p.getProperty(KEY_BACKGROUNDCLASSNAME);
		if (bgclassname == null) {
			Reporter.writeError("Could not restore the Background (cannot determine Background class).");
			return new SimpleBackground();
		}
		try {
			Background inst = (Background) Class.forName(bgclassname)
					.newInstance();
			inst.setConfiguration(p, backgrounddatapath);
			return inst;
		} catch(Exception e) {
			Reporter.writeError("Could not restore the Background (failed to create the instance).");
		    Reporter.debug(e);
		    return new SimpleBackground();
		}
	}

	/**
	 * Factory method, that restores and returns any Background object
	 * from a ListExpr object. If restoration fails, a SimpleBackground is
	 * returned.
	 *
	 * @param l
	 *            The ListExpr to restore from.
	 * @param backgrounddatapath
	 *            A directory with additional data.
	 * @return The restored Background object.
	 */
	public static final Background createFromListExpr(ListExpr l,
			String backgrounddatapath) {
		Properties p = new Properties();
		while (!(l.isAtom() || l.isEmpty())) {
			ListExpr pair = l.first();
			l = l.rest();
			if ((pair.listLength() == 2)
					&& (pair.first().isAtom() && pair.first().atomType() == ListExpr.STRING_ATOM)
					&& (pair.second().isAtom() && pair.second().atomType() == ListExpr.TEXT_ATOM)) {
				String key = pair.first().stringValue();
				String value = pair.second().stringValue();
				p.setProperty(key, value);
			}
		}
		return createFromProperties(p, backgrounddatapath);
	}



}
