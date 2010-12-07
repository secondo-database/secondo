package viewer.hoese;

import java.awt.geom.Rectangle2D;
import java.awt.Rectangle;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Properties;
import java.util.Set;
import java.util.StringTokenizer;

import javax.swing.JComponent;

import sj.lang.ListExpr;

/**
 * This class is to be extended by classes representing different kinds of
 * backgrounds in the HoeseViever, e.g. SimpleBackground, ImageBackground, or
 * TiledBackground.
 * 
 * Specializations of this class should override the inherited paint() method.
 * 
 * @author Christian Duentgen
 */
public abstract class Background extends javax.swing.JComponent {

	private static final long serialVersionUID = 9039714158763811782L;

	/**
	 * The constructor sets all member variables to null. SInce this is an
	 * abstract class, it should never be used anyway.
	 **/
	public Background() {
		name = null;
		license = null;
		bbox = null;
		clipbbox = null;
		viewport = null;
		listeners = null;
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
	 * Sets the Background's clipping boundary (world coordinates). 
	 * Only parts of the background lying within the clipping area are 
	 * displayed. This allows the Background to keep only its visible parts 
	 * in memory.
	 * 
	 * @param rect
	 *            A Rectangle2D defining the clipping area.
	 */
	public void setClipBBox(Rectangle2D.Double rect) {
		if( (bbox != null) && rect != null) {
			clipbbox.setRect(bbox.createIntersection(rect));
		} else {
			clipbbox = rect;
		}
	}
    
	/**
	 * Returns the Background's current clipping area (world coordinates).
	 * @return A Rectangle2D defining the Background's clipping area.
	 */
	public Rectangle2D.Double getClipBBox() {
		return clipbbox;
	}

	/**
	 * Set the the current viewport (visible area in screen coordinates).
	 * The viewport is set to the intersection of the background's current
	 * bounds and the parameter Rectangle. 
	 * @param vp Rectangle describing visible area in screen coordinates.
	 */
	public void setViewport(Rectangle vp) {
		viewport = vp;
	}
	
	/**
	 * Return the current viewport (visible area in screen coordinates)
	 */
	public Rectangle getViewport() {
		return viewport;
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
	 * Overriding methods should check the property KEY_BACKGROUNDTYPE on whether 
	 * it fits their classname! 
	 * 
	 * @param p
	 *            The Background settings to restore.
	 * @param backgrounddatapath
	 *            A path to a directory where additional data is stored in
	 *            files.
	 */
	public void setConfiguration(Properties p, String backgrounddatapath) {
		String n = p.getProperty(KEY_NAME);
		if (n != null) {
			name = n;
		}
		n = p.getProperty(KEY_LICENSE);
		if (n != null) {
			license = n;
		}
		Rectangle2D.Double b = createRectangle2DFromString(p
				.getProperty(KEY_BBOX));
		if (b != null) {
			bbox = b;
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
	 * Overriding methods should add KEY_BACKGROUNDTYPE with their classname 
	 * as the value. 
	 * 
	 * @param backgrounddatapath
	 *            Path, where to store data files.
	 * @return The Background's settings as a Properties object.
	 */
	public Properties getConfiguration(String backgrounddatapath) {
		Properties p = new Properties();
		p.setProperty(KEY_NAME, name);
		p.setProperty(KEY_LICENSE, license);
		String bboxstr = "" + bbox.getX() + " " + bbox.getY() + " "
				+ bbox.getWidth() + " " + bbox.getHeight();
		p.setProperty(KEY_BBOX, bboxstr);
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
	 * The boundary of the part of the background to be currently displayed.
	 */
	protected Rectangle2D.Double clipbbox;

	/**
	 * The current viewport (clipping area in screen coordinates)
	 */
	protected Rectangle viewport; 
	
	/**
	 * Structure to maintain the set of registered listeners
	 */
	private LinkedList<BackgroundListener> listeners;

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
	 * String constant used as key for the background class type within Property
	 */
	public static final String KEY_BACKGROUNDTYPE = "backgroundtype";

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
		// TODO Finish implementation!
		Properties p = getConfiguration(backgrounddatapath);
		Set<Object> keys = p.keySet();
		Iterator<Object> i = keys.iterator();
		if (!i.hasNext()) {
			return ListExpr.theEmptyList();
		}
		String keystr = (String) i.next();
		String valuestr = p.getProperty(keystr);
		ListExpr pair = ListExpr.twoElemList(ListExpr.stringAtom(keystr),
				ListExpr.textAtom(valuestr));
		ListExpr nl = ListExpr.oneElemList(pair);
		ListExpr last = nl;
		while (i.hasNext()) {
			keystr = (String) i.next();
			valuestr = p.getProperty(keystr);
			pair = ListExpr.twoElemList(ListExpr.stringAtom(keystr),
					ListExpr.textAtom(valuestr));
			last = ListExpr.append(last, pair);
		}
		return nl;
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
		while (!(l.isAtom() || l.isEmpty())) {
			ListExpr pair = l.first();
			l = l.rest();
			if ((pair.listLength() == 2)
					&& (pair.first().isAtom() && pair.first().atomType() == ListExpr.STRING_ATOM)
					&& (pair.second().isAtom() && pair.second().atomType() == ListExpr.TEXT_ATOM)) {
				String key = pair.first().toString();
				String value = pair.second().toString();
				p.setProperty(key, value);
			}
		}
		setConfiguration(p, backgrounddatapath);
	}
}
