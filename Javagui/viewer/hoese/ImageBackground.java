package viewer.hoese;

import java.awt.Rectangle;
import java.awt.Graphics2D;
import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.LinkedList;
import java.util.Properties;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;

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
		useforbbox = false;
		listeners = new LinkedList<BackgroundListener>();
	}

	/**
	 * Constructs an ImageBackground by interaction with the user.
	 *
	 * @param parent
	 *            The parent component for the used dialog.
	 */
	public ImageBackground(JComponent parent) {
		name = "ImageBackground (no image)";
		license = "";
		useforbbox = false;
		listeners = new LinkedList<BackgroundListener>();
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
		useforbbox = false;
		listeners = new LinkedList<BackgroundListener>();
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
		useforbbox = false;
		listeners = new LinkedList<BackgroundListener>();
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
		if (listeners == null) {
			listeners = new LinkedList<BackgroundListener>();
		}
		if (bgi == null ) {
      bgi = new ImageBackgroundDialog(null);
    }
    bgi.readFrom(this);
		bgi.setVisible(true);
    readFrom(bgi);
	}


  /** reads the content of this background from an dialog **/
  private void readFrom(ImageBackgroundDialog bgi){
		BufferedImage bi = bgi.getImage();
		if (bi != null) {
			img = bi;
			bbox = bgi.getBBox();
			name = "ImageBackground";
			license = "";
			useforbbox = bgi.useForBoundingBox();
      backgroundColor = bgi.getBackgroundColor();
      backgroundColorChanged = true;
			BackgroundChangedEvent evt = new BackgroundChangedEvent() {
				public Object getSource() {
					return ImageBackground.this;
				}
			};
			informListeners(evt);
		}
    computeTransform();
  }




	/**
	 * Use the parameter as the new background image
	 * @param img The image to be used as new backgrund
	 */
	public void setImage(BufferedImage newimage) {
		img = newimage;
    computeTransform();
	}

  /** Returns the currently used image of this background **/
  public BufferedImage getImage(){
      return img;
  }


	/**
	 * Paint method
	 *
	 * @see viewer.hoese.Background#paint(java.awt.Graphics)
	 * @param g
	 *            Graphic to which the Background is drawn.
	 */
	@Override
	public void paint(JComponent parent,Graphics2D g, AffineTransform at, Rectangle2D clipRect) {
    if(parent!=null){
      if(backgroundColorChanged){
         backgroundColorChanged=false;
         parent.setBackground(backgroundColor);
      }
    }
		if (img != null) {
      if(at_img2wc==null){
         computeTransform();
      }
      AffineTransform at_img2screen = new AffineTransform(at_img2wc);
      at_img2screen.preConcatenate(at);
      //at_img2screen.concatenate(at);
			g.drawImage(img,at_img2screen, null);
		}
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
		if (listeners == null) {
			listeners = new LinkedList<BackgroundListener>();
		}
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
			img = bi;
		}
		// inform listeners
		BackgroundChangedEvent evt = new BackgroundChangedEvent() {
			public Object getSource() {
				return ImageBackground.this;
			}
		};
    computeTransform();
		informListeners(evt);
	}

  
	public void setConfiguration(BufferedImage img, double x, double y, double w, double h, boolean useForBBox, Color  BGColor) {
		if (listeners == null) {
			listeners = new LinkedList<BackgroundListener>();
		}
    this.img = img;
    setBBox(new Rectangle2D.Double(x,y,w,h));
    setUseForBoundingBox(useForBBox);
    setBackgroundColor(BGColor); 
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
				javax.imageio.ImageIO.write(img, "png", f);
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

  public String toString(){
     return getClass().getName()+"["+super.toString()+", img = "+
                                    img+ ", at_img2wc = " + at_img2wc+"]";
  }

  /** computes the affine transformation to map the image to the current bounding box **/
  private void computeTransform(){
     if(img==null || bbox == null){
       at_img2wc=null;
       return;
     }
     double img_w = img.getWidth();
     double img_h = img.getHeight();
     if(img_w<=0 || img_h<=0){
       at_img2wc=null;
       return;
     }
     double wc_x = bbox.getX();
     double wc_y = bbox.getY();
     double wc_w = bbox.getWidth();
     double wc_h = bbox.getHeight();

     if(wc_w<=0 || wc_h <=0){
       at_img2wc=null;
       return;
     }

     double scale_x = wc_w/img_w;
     double scale_y = wc_h/img_h * -1.0;
     at_img2wc = AffineTransform.getTranslateInstance(wc_x,  wc_y +  wc_h);
     at_img2wc.scale(scale_x, scale_y);
  }

  /** sets the color outside the image **/
  public void setBackgroundColor(Color c){
      backgroundColor = c;
      backgroundColorChanged = true;
  }

  /** returns the colors used outside the image **/
  public Color getBackgroundColor(){
     return backgroundColor;
  }


	/**
	 * The image that is used for the background.
	 */
	private BufferedImage img = null;

  /** Dialog for user interaction. **/
  private static ImageBackgroundDialog bgi = null;

  /** Transformation from image to bbox world coordinates*/
  private AffineTransform at_img2wc = null;

  /** Used backgroundcolor outside the image **/
  private Color backgroundColor = null;
  /** flag indicating a changed background color **/
  private boolean backgroundColorChanged = true; 


	/**
	 * String constant used as key for the background image within Property
	 */
	public static final String KEY_BGIMAGE = "backgroundimage";
}
