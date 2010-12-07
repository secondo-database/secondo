package viewer.hoese;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.util.Properties;

import javax.swing.JColorChooser;
import javax.swing.JComponent;

import sj.lang.ListExpr;

/**
 * This Background class homogeneously fills the visible background (the total
 * clipping area) with a user-defined color.
 * 
 * @author Christian Duentgen
 * 
 */
public class SimpleBackground extends Background {

	private static final long serialVersionUID = 4578057018487847324L;

	/**
	 * The simple constructor sets the background color to the JComponent's
	 * default background color.
	 */
	public SimpleBackground() {
		name = "SimpleBackground";
		license = "";
		backgroundcolor = getBackground();
		useforbbox = false;
	}

	/**
	 * The simple constructor sets the background color to "white"
	 * 
	 * @param c
	 *            The color to be used as background color;
	 */
	public SimpleBackground(Color c) {
		name = "SimpleBackground";
		license = "";
		backgroundcolor = c;
		useforbbox = false;
	}

	/**
	 * Creates a SimpleBackground, restoring it from a property.
	 * 
	 * @param p
	 *            The properties to read from.
	 * @param datapath
	 *            The directory with additional data (not used).
	 */
	public SimpleBackground(Properties p, String datapath) {
		name = "SimpleBackground";
		license = "";
		backgroundcolor = getBackground();
		useforbbox = false;
		setConfiguration(p, datapath);
	}

	/**
	 * Creates a SimpleBackground from a nested list and a data path.
	 * 
	 * @param l
	 *            The nested list to read from.
	 * @param datapath
	 *            The directory with additional data (not used).
	 */
	public SimpleBackground(ListExpr l, String datapath) {
		name = "SimpleBackground";
		license = "";
		backgroundcolor = getBackground();
		useforbbox = false;
		readFromList(l, datapath);
	}

	/**
	 * Constructs a SimpleBackground by interaction with the user.
	 * 
	 * @param parent
	 *            The parent for the used dialog
	 */
	public SimpleBackground(JComponent parent) {
		name = "SimpleBackground";
		license = "";
		backgroundcolor = getBackground();
		useforbbox = false;
		showConfigDialog(parent);
	}

	/**
	 * Opens a Dialog for choosing the background color. Informs all listeners
	 * about the change.
	 * 
	 * @see viewer.hoese.Background#showConfigDialog(javax.swing.JComponent)
	 * @param parent
	 *            The parent of the dialog component used.
	 */
	@Override
	public void showConfigDialog(JComponent parent) {
		name = "SimpleBackground";
		license = "";
		useforbbox = false;
		backgroundcolor = JColorChooser.showDialog(this,
				"Choose the background color:", backgroundcolor);
		if (backgroundcolor == null) {
			backgroundcolor = getBackground();
		}
		BackgroundChangedEvent evt = new BackgroundChangedEvent() {
			public Object getSource() {
				return SimpleBackground.this;
			}
		};
		informListeners(evt);
	}

	/**
	 * Set up the Background with parameters given as a Properties object. Can
	 * be used to restore the Background settings, e.g. from a file. All
	 * registered Listeners are informed. The image data is retrieved from files
	 * rooted at the given path.
	 * @param p
	 *            The Background settings to restore.
	 * @param backgrounddatapath
	 *            A path to a directory where additional data is stored in
	 *            files.
	 * @see viewer.hoese.Background#setConfiguration(java.util.Properties)
	 */
	@Override
	public void setConfiguration(Properties p, String backgrounddatapath) {
		super.setConfiguration(p, backgrounddatapath);
		backgroundcolor = getBackground();
		String bcstring = p.getProperty(KEY_BGCOLOR, null);
		if(bcstring != null) {
			int rgb = Integer.parseInt(bcstring);
			backgroundcolor = new Color(rgb);
			BackgroundChangedEvent evt = new BackgroundChangedEvent() {
				public Object getSource() {
					return SimpleBackground.this;
				}
			};
			informListeners(evt);
		}
	}

	/**
	 * Return the Background's settings as a Properties object. The Properties
	 * can be used to save Background settings for later reference. Additional
	 * data may be stored to files rooted at the given path.
	 * @param BackgroundDataPath
	 *            Path where to store the current image
	 * @return The Background's settings as a Properties object.
	 * @see viewer.hoese.Background#getConfiguration(java.lang.Object)
	 */
	@Override
	public Properties getConfiguration(String backgrounddatapath) {
		Properties p = super.getConfiguration(backgrounddatapath);
		p.setProperty(KEY_BGCOLOR, "" + backgroundcolor.getRGB());
		return p;
	}

	/**
	 * Paints the background as a filled rectangle.
	 * @see viewer.hoese.Background#paint()
	 * @param g
	 *            The Graphics object to paint to.
	 */
	@Override
	public void paint(Graphics g) {
		Graphics2D g2d = (Graphics2D) g;
		g2d.setBackground(backgroundcolor);
		super.paint(g2d);
	}

	/**
	 * This is a call-back method to be called by external objects requesting
	 * the Background to update it's internal data. Since the SimpleBackground
	 * does not register with any other structure, the implementation stays
	 * empty.
	 */
	@Override
	public void handleBackgroundDataChangedEvent(BackgroundChangedEvent evt) {
	}

	/**
	 * Sets the Background's clipping boundary. Since this specialized
	 * Background should paint the entire visible background in a heterogeneous
	 * color, this simultaneously changes the bounds member attribute.
	 * 
	 * @see viewer.hoese.Background#getConfiguration(java.lang.Object)
	 * @param rect
	 */
	@Override
	public void setClipBBox(Rectangle2D.Double rect) {
		if ((bbox != null) && (rect != null)) {
			clipbbox = (Rectangle2D.Double) bbox.createIntersection(rect);
		} else {
			clipbbox = (Rectangle2D.Double) rect;
		}
		bbox = (Rectangle2D.Double) rect;
	}

	/**
	 * The background color to be used.
	 */
	private Color backgroundcolor;

	public static final String KEY_BGCOLOR = "backgroundcolor";
}
