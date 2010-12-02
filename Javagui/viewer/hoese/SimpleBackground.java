/**
 * 
 */
package viewer.hoese;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.util.Properties;

import javax.swing.JColorChooser;
import javax.swing.JComponent;

/**
 * This Background class homogeniously fills the visiable background (the total
 * clipping area) with a user-defined colour.
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
	}

	/**
	 * The simple constructor sets the background color to "white"
	 * 
	 * @param c
	 *            The color to be used as background color;
	 */
	public SimpleBackground(Color c) {
		setName("SimpleBackground");
		license = "";
		backgroundcolor = c;
	}

	/**
	 * Opens a Dialog for choosing the background color. Informs all listeners
	 * about the change.
	 * 
	 * @see viewer.hoese.Background#showConfigDialog(javax.swing.JComponent)
	 * 
	 * @param parent
	 *            The parent of the dialog component used.
	 */
	@Override
	public void showConfigDialog(JComponent parent) {
		backgroundcolor = JColorChooser.showDialog(this,
				"Choose the background color, nerd!", backgroundcolor);
		BackgroundChangedEvent evt = new BackgroundChangedEvent() {
			public Object getSource() {
				return SimpleBackground.this;
			}
		};
		informListeners(evt);
	}

	/**
	 * Set up the Background with the background color and bounds from the
	 * Argument. All registered Listeners are informed.
	 * 
	 * @see viewer.hoese.Background#setConfiguration(java.util.Properties)
	 * @param
	 */
	@Override
	public void setConfiguration(Properties p) {
		super.setConfiguration(p);
		String bcstring = p.getProperty(BGSTR);
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
	 * Saves the background color and bounds to a Properties Object.
	 * 
	 * @see viewer.hoese.Background#getConfiguration(java.lang.Object)
	 * @param The
	 *            Background's settings as a Properties object.
	 */
	@Override
	public Properties getConfiguration() {
		Properties p = super.getConfiguration();
		p.setProperty(BGSTR, "" + backgroundcolor.getRGB());
		return p;
	}

	/**
	 * Paints the background as a filled rectangle.
	 * 
	 * @see viewer.hoese.Background#paint()
	 * 
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
	 * Background should paint the entire visible background in a heterogenous
	 * color, this simultaneously changes the bounds member attribute.
	 * 
	 * @see viewer.hoese.Background#getConfiguration(java.lang.Object)
	 * @param rect
	 */
	@Override
	public void setClipBBox(Rectangle2D.Double rect) {
		clipbbox = rect;
		bbox = rect;
	}

	/**
	 * The background color to be used.
	 */
	private Color backgroundcolor;

	public static final String BGSTR = "backgroundcolor";
}
