package viewer.hoese;

import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.Properties;

import javax.swing.JComponent;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * This class implements a Background showing a scalable image.
 * @author Christian Duentgen
 * 
 */
public class ImageBackground extends Background {

	/**
	 * Generated VersionUID for serialization
	 */
	private static final long serialVersionUID = -9060443805324796816L;

	/**
	 * Constructs an undefined Background.
	 */
	public ImageBackground() {
		name = "ImageBackground (no image)";
		license = "";
		img = new ScalableImage();
		bbox = null;
		clipbbox = null;
		useforbbox = false;
	}

	/**
	 * Constructs an ImageBackground by interaction with the user.
	 * 
	 * @param parent
	 *            The parent component for the used dialog.
	 */
	public ImageBackground(JComponent parent) {
		name = "ImageBackground";
		license = "";
		img = new ScalableImage();
		bbox = null;
		clipbbox = null;
		useforbbox = false;
		showConfigDialog(parent);
	}

	/**
	 * Creates an ImageBackground from a nested list and a data path.
	 * 
	 * @param l
	 *            The nested list to read from.
	 * @param datapath
	 *            The directory with the background image file.
	 */
	public ImageBackground(ListExpr l, String datapath) {
		name = "ImageBackground";
		license = "";
		img = new ScalableImage();
		bbox = null;
		clipbbox = null;
		useforbbox = false;
		readFromList(l, datapath);
	}

	/**
	 * Constructs an ImageBackground from a suitable Properties object.
	 * 
	 * @param p
	 *            The Properties to restore from.
	 * @param datafilepath
	 *            Path of the directory holding the nackground image file.
	 */
	public ImageBackground(Properties p, String datafilepath) {
		name = "ImageBackground";
		license = "";
		img = new ScalableImage();
		bbox = null;
		clipbbox = null;
		useforbbox = false;
		setConfiguration(p, datafilepath);
	}

	/**
	 * Display a dialog for importing a background image from a file and
	 * locating it to the world (i.e. setting its bounds). Inform all listeners,
	 * if this was successful.
	 * 
	 * @see viewer.hoese.Background#showConfigDialog(javax.swing.JComponent)
	 * @param parent
	 *            This parameter is ignored.
	 */
	@Override
	public void showConfigDialog(JComponent parent) {
		BackGroundImage bgi = new BackGroundImage(null);
		bgi.setVisible(false);
		BufferedImage bi = bgi.getImage();
		if (bi != null) {
			img.setImage(bi);
			bbox = new BackGroundImage(null).getBBox();
			if ((clipbbox == null) && (bbox != null)) {
				clipbbox = bbox;
			}
			name = "ImageBackground";
			license = "";
			img.setClipRect(clipbbox);
			useforbbox = bgi.useForBoundingBox();
			BackgroundChangedEvent evt = new BackgroundChangedEvent() {
				public Object getSource() {
					return ImageBackground.this;
				}
			};
			informListeners(evt);
		}
	}

	/**
	 * This overrides the method inherited from JComponent. It sets the location
	 * and dimensions of the objects's graphical representation. It uses the
	 * field member img (class SacalableImage), which is capable of handling
	 * clipping and getting itself drawn. This methods deals with screen
	 * coordinates!
	 * 
	 * @param x
	 *            x-coordinate, relative to upper left border.
	 * @param y
	 *            y-coordinate, relative to upper left border.
	 * @param w
	 *            width (non-negative),
	 * @param h
	 *            height (non-negative),
	 */
	public void setBounds(int x, int y, int w, int h) {
		img.setBounds(x, y, w, h);
		super.setBounds(x, y, w, h);
	}

	/**
	 * Use the parameter as the new background image
	 * @param img The image to be used as new backgrund
	 */
	public void setImage(BufferedImage newimage) {
		img.setImage(newimage);
	}
	
	/**
	 * Paint method
	 * 
	 * @see viewer.hoese.Background#paint(java.awt.Graphics)
	 * @param g
	 *            Graphic to which the Background is drawn.
	 */
	@Override
	public void paint(Graphics g) {
		super.paint(g);
		img.paint(g);
	}

	/**
	 * This might be useful for automatically changing background images, but as
	 * these are not yet implemented, this method has an empty implementation.
	 * 
	 * @see viewer.hoese.Background#handleBackgroundDataChangedEvent()
	 */
	@Override
	public void handleBackgroundDataChangedEvent(BackgroundChangedEvent evt) {
		// empty implementation
	}

	/**
	 * Set up the Background with parameters given as a Properties object. Can
	 * be used to restore the Background settings, e.g. from a file. All
	 * registered Listeners are informed.
	 * 
	 * @param backgrounddatapath
	 *            Path, where data files are stored.
	 * @param p
	 *            The Background settings to restore.
	 */
	@Override
	public void setConfiguration(Properties p, String backgrounddatapath) {
		super.setConfiguration(p, backgrounddatapath);
		// TODO restore the image itself from a file stored as a property
		String filename = p.getProperty(KEY_BGIMAGE);
		if (filename == null) { // no image stored
			Reporter.showError("No stored background image available.");
			return;
		}
		// try to open the file
		if (!backgrounddatapath.endsWith(File.separator))
			backgrounddatapath += File.separator;
		File f = new File(backgrounddatapath + filename);
		BufferedImage bi = null;
		if (!f.exists()) {
			Reporter.showError("Background image \"" + backgrounddatapath
					+ filename + "\" not found.");
			bi = null;
		} else { // file exists: read the image
			try {
				bi = javax.imageio.ImageIO.read(f);
			} catch (Exception e) {
				Reporter.showError("Error while loading the background image.");
				bi = null;
			}
		}
		if (bi != null) { // assign the image, update bboxes
			img.setImage(bi);
			img.setClipRect(clipbbox);
		}
		// inform listeners
		BackgroundChangedEvent evt = new BackgroundChangedEvent() {
			public Object getSource() {
				return ImageBackground.this;
			}
		};
		informListeners(evt);
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
	@Override
	public Properties getConfiguration(String backgrounddatapath) {
		Properties p = super.getConfiguration(backgrounddatapath);
		// TODO store the image itself to a file and pass the filename as a
		// property
		String filename = null;
		try {// select a non-existent fileName for the image
			File f = new File(backgrounddatapath);
			if (!f.exists()) {
				Reporter.showError("Storing the background image failed:\n"
						+ " directory \"" + backgrounddatapath
						+ "\" doesn't exists");
			} else if (!f.isDirectory()) {
				Reporter.showError("Storing the background image failed:\n\""
						+ backgrounddatapath + "is not a directory");
			} else {
				if (!backgrounddatapath.endsWith(File.separator))
					backgrounddatapath += File.separator;
				filename = ".background";
				int bgnumber = -1;
				do {
					bgnumber++;
					f = new File(backgrounddatapath + filename + bgnumber
							+ ".png");
				} while (f.exists());
				javax.imageio.ImageIO.write(img.getImage(), "png", f);
				filename = filename + bgnumber + ".png";
			}
		} catch (Exception e) {
			Reporter.showError("Storing the background image failed!");
			Reporter.debug(e);
			filename = null;
		}
		if (filename != null) {
			p.setProperty(KEY_BGIMAGE, filename);
		}
		return p;
	}

	/**
	 * The image that is used for the background.
	 */
	private ScalableImage img;

	/**
	 * String constant used as key for the background image within Property
	 */
	public static final String KEY_BGIMAGE = "backgroundimage";
}
