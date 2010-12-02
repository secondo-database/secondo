/**
 * 
 */
package viewer.hoese;

import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.Properties;

import javax.swing.JComponent;

import tools.Reporter;

/**
 * This class implements a Background showing a scalable image.
 * @author Christian Duentgen
 * 
 */
public class ImageBackground extends Background {

	private static final long serialVersionUID = 3198851131892244547L;

	public ImageBackground() {
		img = new ScalableImage();
		bbox = null;
		clipbbox = null;
	}

	/**
	 * Display a dialog for importing a background image from a file and
	 * locating it to the world (i.e. setting its bounds). Inform all listeners,
	 * if this was successful.
	 * @see viewer.hoese.Background#showConfigDialog(javax.swing.JComponent)
	 */
	@Override
	public void showConfigDialog(JComponent parent) {
		BufferedImage bi = new BackGroundImage(null).getImage();
		if (bi != null) {
			img.setImage(bi);
			bbox = new BackGroundImage(null).getBBox();
			if ((clipbbox == null) && (bbox != null)) {
				clipbbox = bbox;
			}
			img.setClipRect(clipbbox);
			BackgroundChangedEvent evt = new BackgroundChangedEvent() {
				public Object getSource() {
					return ImageBackground.this;
				}
			};
			informListeners(evt);
		}
	}

	/* (non-Javadoc)
	 * @see viewer.hoese.Background#paint(java.awt.Graphics)
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
