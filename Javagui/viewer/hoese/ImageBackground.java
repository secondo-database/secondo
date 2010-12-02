/**
 * 
 */
package viewer.hoese;

import java.awt.Graphics;
import java.util.Properties;

import javax.swing.JComponent;

/**
 * This class implements a Background showing a scalable image.
 * 
 * @author Christian Duentgen
 * 
 */
public class ImageBackground extends Background {

	/**
	 * 
	 */
	public ImageBackground() {
		img = new ScalableImage();
	}

	/* (non-Javadoc)
	 * @see viewer.hoese.Background#showConfigDialog(javax.swing.JComponent)
	 */
	@Override
	public void showConfigDialog(JComponent parent) {
		// TODO Auto-generated method stub
		BackGroundImage bgi = new BackGroundImage(null);
		// BufferedImage bi = bgi.getImage();
		// Rectangle2D buffer =

	}

	/* (non-Javadoc)
	 * @see viewer.hoese.Background#setConfiguration(java.util.Properties)
	 */
	@Override
	public void setConfiguration(Properties p) {
		super.setConfiguration(p);
		// TODO Auto-generated method stub
	}

	/* (non-Javadoc)
	 * @see viewer.hoese.Background#getConfiguration()
	 */
	@Override
	public Properties getConfiguration() {
		Properties p = super.getConfiguration();
		// TODO Auto-generated method stub
		// add property for the image (e.g. filename)
		return p;
	}

	/* (non-Javadoc)
	 * @see viewer.hoese.Background#paint(java.awt.Graphics)
	 */
	@Override
	public void paint(Graphics g) {
		img.paint(g);
		super.paint(g);
	}

	/* (non-Javadoc)
	 * @see viewer.hoese.Background#handleBackgroundDataChangedEvent()
	 */
	@Override
	public void handleBackgroundDataChangedEvent(BackgroundChangedEvent evt) {
		// TODO Auto-generated method stub

	}

	private ScalableImage img;
}
