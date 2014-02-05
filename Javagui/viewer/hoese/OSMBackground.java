package viewer.hoese;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Paint;
import java.awt.Shape;
import java.awt.Toolkit;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.net.URL;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Properties;

import javax.swing.JComponent;

import tools.Cache;
import tools.Pair;
import tools.downloadmanager.ActiveDownload;
import tools.downloadmanager.DownloadEvent;
import tools.downloadmanager.DownloadManager;
import tools.downloadmanager.DownloadObserver;
import tools.downloadmanager.DownloadState;

/**
 * Implementation of Open Street Map Tiles as Background for the HoeseViewer.
 **/

public class OSMBackground extends Background {

	/** Creates a new OSMBackground initialized with default values. **/
	public OSMBackground() {

		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		DIM_X = (int) dim.getWidth();
		DIM_Y = (int) dim.getHeight();
		name = "OSMBackground";
		double maxY = 85.0511;

		bbox = new Rectangle2D.Double(-180, -maxY, 360, 2 * maxY); // able to
																	// display
																	// the
																	// "world"
		listeners = new LinkedList<BackgroundListener>();
		useforbbox = true;
		try {
			downloadManager = new DownloadManager(new File(PATH), maxDownloads, true);
		} catch (Exception e) {
			System.err.println("Problem in initiating download manager");
			downloadManager = null;
		}

		downloadManager.setConnectTimeout(5000);
		downloadManager.setReadTimeout(5000);

		observer = new DownloadObserver() {
			public void downloadStateChanged(DownloadEvent evt) {
				OSMBackground.this.downloadStateChanged(evt);
			}
		};
		imageCache = new Cache<CachedImage, ImgLoader>(CACHESIZE,
				new ImgLoader());

		settings = new OSMDialog(null);
	}

	/**
	 * Overrides the setBBox function from Background. Does nothing bacause the
	 * bounding box is fixed by the server.
	 * 
	 * @param rect
	 *            Dummy only!
	 **/
	public void setBBox(Rectangle2D.Double rect) {
	}

	/**
	 * Shows a dialog to configure this background.
	 * 
	 * @param f
	 *            Used as the dialog's parent
	 * **/
	public void showConfigDialog(JComponent f) {
		settings.showDialog();
		Properties props = new Properties();
		settings.storeSettingsToProperties(props);
		setCheckedConfiguration(props);
	}


  /** This background supports preloading **/
  @Override
  public boolean supportsPreload(){
     return true;
  }

  @Override
  public int numberOfPreloadFiles(Rectangle2D.Double clipRect){
     return mapper.numberOfPreloadFiles(clipRect); 
  }


  @Override
  public void preload(Rectangle2D.Double clipRect, PreloadObserver observer){
     preloadObserver = observer;
     LinkedList<URL> files = mapper.getPreloadURLs(clipRect);
     try{
        preloadManager = new DownloadManager(new File(PATH), maxDownloads, false);
        DownloadObserver obs = new DownloadObserver(){
           public void downloadStateChanged(DownloadEvent evt){
              if(preloadObserver==null){
                 return;
              } 
              if(evt.getState() == DownloadState.DONE){
                  preloadObserver.step(true);
              } else {
                 preloadObserver.step(false);
             }
             int pending = preloadManager.numOfPendingDownloads();
             if(pending==0){
                preloadObserver.finish(true);
             }
           }
         @Override
         public void fileExists(URL url){
              if(preloadObserver==null){
                 return;
              } 
              preloadObserver.step(true);
         }
             
       };
        
       Iterator<URL> it = files.iterator();
       while(it.hasNext()){
          preloadManager.getURL(it.next(),obs);
       }     
       int pending = preloadManager.numOfPendingDownloads();
       if(pending==0){
          preloadObserver.finish(true);
       }
    } catch(Exception e){}
 
  }

  @Override
  public void cancelPreload(){
     if(preloadManager!=null){
        preloadManager.cancelDownloads();
        preloadManager=null;
     }
  } 



  

  public double getAntiScale(AffineTransform at, Rectangle2D clipRect){
     if(!fixedZoomLevels) return 1.0;
     return mapper.getAntiScale(at,clipRect);
  }


  
  public double getZoomStep(){
     return 2.0;
  }


	/**
	 * Overrides the paint method from Background. Paints the tiles downloaded
	 * from a specific server.
	 * 
	 * @param parent
	 *            The parent component containing the background.
	 * @param g
	 *            The garphics context to paint on.
	 * @param at
	 *            Affine transformation from world to screen coordinates.
	 * @param clipRect
	 *            The part of the world to be drawn (world coordinates).
	 */
	public void paint(JComponent parent, Graphics2D g, AffineTransform at,
			Rectangle2D clipRect) {

		// set background color
		if (backgroundColorChanged && parent != null) {
			backgroundColorChanged = false;
			parent.setBackground(backgroundColor);
		}

		// check MBR
		if ((clipRect.getWidth() <= 0) || (clipRect.getHeight() <= 0)) {
			return;
		}

		// compute the bounding box in geographic coordinates
		if (ProjectionManager.isReversible()) {
			java.awt.geom.Point2D.Double p1 = new java.awt.geom.Point2D.Double();
			java.awt.geom.Point2D.Double p2 = new java.awt.geom.Point2D.Double();

			ProjectionManager.getOrigWithoutScale(clipRect.getX(), clipRect.getY(), p1);
			ProjectionManager.getOrigWithoutScale(clipRect.getX() + clipRect.getWidth(),
					clipRect.getY() + clipRect.getHeight(), p2);
			clipRect.setRect(p1.getX(), p1.getY(), p2.getX() - p1.getX(),
					p2.getY() - p1.getY());
		}

		try {
			lastParent = parent;
			// only use the part inside the world
			clipRect = clipRect.createIntersection(bbox);
			if (clipRect.isEmpty()) {
				return;
			}

			// compute the urls of all required tiles and
			// define an affine transformation to move the tile to it's location
			// within the world
			LinkedList<Pair<URL, AffineTransform>> urls = mapper.computeURLs((Rectangle2D.Double) clipRect, at);
			// paint the tiles, frames, and labels
			if (urls != null) {
				paintURLs(g, urls, at);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Paint the tiles, frames, and labels coming from urls depending on some
	 * flag members.
	 * 
	 * @param g
	 *            Graphics context to paint on
	 * @param urls
	 *            Urls to be painted
	 * @param at
	 *            trrectansformation from world to screen coordinates
	 */
	private void paintURLs(Graphics2D g,
			LinkedList<Pair<URL, AffineTransform>> urls, AffineTransform at) {

		Iterator<Pair<URL, AffineTransform>> it = urls.iterator();
		if (showTiles) {
			while (it.hasNext()) {
				paintURL(g, it.next(), at);
			}
		}
		Paint paint = g.getPaint();
		g.setPaint(color);

		if (showFrames) {
			it = urls.iterator();
			while (it.hasNext()) {
				paintTileFrame(g, it.next().second(), at);
			}
		}

		if (showNames) {
			it = urls.iterator();
			while (it.hasNext()) {
				paintName(g, it.next(), at);
			}
		}
		g.setPaint(paint);
	}

	/**
	 * Paint a single tile or frame depending on the flag frame.
	 * 
	 * @param g
	 *            graphics context to paint on
	 * @param url
	 *            source of the image and a transformation to map the image into
	 *            the world
	 * @param at
	 *            mapping from world to screen coordinates
	 * @param frame
	 *            if set to true, only the frame of the image is painted,
	 *            otherwise the image
	 */
	private void paintURL(Graphics2D g, Pair<URL, AffineTransform> url,
			AffineTransform at) {
		// start download

		if (url.first() == null) {
			return;
		}


		File f = downloadManager.getURL(url.first(), observer);

		if (f != null) { // url already downloaded
			CachedImage cimg = imageCache.getElem(f);
			if (cimg == null) {
				System.err.println("could not load image from file " + f);
				f.delete();
			} else {
				BufferedImage img = cimg.getImage();
				if (img != null) {
						paintImage(g, cimg.getImage(), url.second(), at);
				} else {
					System.err
							.println("could not extract image from file " + f);
				}
			}
		}
	}

	/**
	 * Pain the name of a map tile image file to the screen
	 * 
	 * @param g
	 *            Graphics context to draw on.
	 * @param url
	 *            Map tile, whose name is to be printed.
	 * @param at
	 *            Affine transformation from world to screen coordinates.
	 */
	private void paintName(Graphics2D g, Pair<URL, AffineTransform> url,
			AffineTransform at) {

		String labelText = (url.first() == null) ? "null" : url.first()
				.getFile().toString();
		Rectangle2D.Double rImg = new Rectangle2D.Double(0, 0, tileSizeX,
				tileSizeY);
		Rectangle2D rWorld = url.second().createTransformedShape(rImg)
				.getBounds2D();
		Point2D.Double p = new Point2D.Double(rWorld.getX() + rWorld.getWidth()
				/ 2, rWorld.getY() + rWorld.getHeight() / 2);
		at.transform(p, p);
		float x = (float) p.getX();
		float y = (float) p.getY();
		Rectangle2D re = g.getFont().getStringBounds(labelText,
				g.getFontRenderContext());
		x -= re.getWidth() / 2.0;
		y += re.getHeight() / 2.0;
		g.drawString(labelText, x, y);
	}

	/**
	 * Paints a buffered image to a graphics context using the given
	 * transformations.
	 * 
	 * @param g
	 *            the graphics context to paint on
	 * @param img
	 *            the image to paint
	 * @param img2world
	 *            mapping from image coordinates to world coordinates
	 * @param world2screnn
	 *            mapping from world coordinates to screen coordinates
	 **/
	private void paintImage(Graphics2D g, BufferedImage img,
			AffineTransform img2world, AffineTransform world2Screen) {
		if (img != null) {
			AffineTransform at = new AffineTransform(world2Screen);
			at.concatenate(img2world);

      if(Auxiliary.almostEqual(at.getScaleX(),1.0) && Auxiliary.almostEqual(at.getScaleY(),1.0)){
         double[] flat = new double[6];
         Auxiliary.setScale(at,1.0,1.0);
      }
			g.drawImage(img, at, null);
		}
	}

	/**
	 * paint the frame of an image to g.
	 * 
	 * @param g
	 *            the graphics context to paint on
	 * @param img
	 *            the image whose frame is to be painted
	 * @param img2world
	 *            mapping from image coordinates to world coordinates
	 * @param world2screnn
	 *            mapping from world coordinates to screen coordinates
	 **/
	private void paintTileFrame(Graphics2D g,
			AffineTransform img2world, AffineTransform world2Screen) {

		Rectangle2D.Double rImg = new Rectangle2D.Double(0, 0, tileSizeX,
				tileSizeY);
		Rectangle2D rWorld = img2world.createTransformedShape(rImg)
				.getBounds2D();

		Shape rScreen = world2Screen.createTransformedShape(rWorld);
		g.draw(rScreen);

	}

	/**
	 * Callback method. Called when a state of a pending download is changed.
	 * Calls the repaint method of the parent component.
	 * 
	 * @param evt
	 *            Event to handle.
	 **/
	private void downloadStateChanged(DownloadEvent evt) {
		DownloadState state = evt.getState();
		ActiveDownload ad = evt.getSource();
		if (lastParent != null && state == DownloadState.DONE) {
			lastParent.repaint();
		} else if (state == DownloadState.BROKEN) {
			if (evt.getException() != null) {
				evt.getException().printStackTrace();
			}
		}
	}

	/**
	 * Export the background's properties
	 * 
	 * @param backgroundDataPath
	 *            Directory for storing files
	 * @return The backgound's properties
	 */
	public Properties getConfiguration(String backgroundDataPath) {
		Properties res = super.getConfiguration(backgroundDataPath);
		settings.storeSettingsToProperties(res);
		return res;
	}

	/**
	 * Set the background properties
	 * 
	 * @param properties
	 *            The background properties to restore
	 * @param backgroundDataPath
	 *            Directory with additional data files
	 */
	@Override
	public void setConfiguration(Properties properties,
			String backgroundDataPath) {
		super.setConfiguration(properties, backgroundDataPath);

		settings.restoreSettingsFromProperties(properties);
		Properties corrected = new Properties();
		settings.storeSettingsToProperties(corrected);
		setCheckedConfiguration(corrected);

	}

	/**
	 * Set previously validated background settings
	 * 
	 * @param s
	 *            The validated background properties to restore
	 */
	private void setCheckedConfiguration(Properties s) {
		// now we can be sure that all the values are correct :-)

		try {
			protocol = s.getProperty(KEY_PROTOCOL);
			server = s.getProperty(KEY_SERVER);
			port = Integer.parseInt(s.getProperty(KEY_PORT));
			directory = s.getProperty(KEY_DIRECTORY);
			prefix = s.getProperty(KEY_PREFIX);

			minZoomLevel = Integer.parseInt(s.getProperty(KEY_MINZOOMLEVEL));
			maxZoomLevel = Integer.parseInt(s.getProperty(KEY_MAXZOOMLEVEL));
			maxDownloads = Integer.parseInt(s.getProperty(KEY_MAXDOWNLOADS));
			tileSizeX = Integer.parseInt(s.getProperty(KEY_TILESIZEX));
			tileSizeY = Integer.parseInt(s.getProperty(KEY_TILESIZEY));
			name = s.getProperty(KEY_NAME);
			showFrames = s.getProperty(KEY_SHOWFRAMES).equals("TRUE");
			showNames = s.getProperty(KEY_SHOWNAMES).equals("TRUE");
			fixedZoomLevels = s.getProperty(KEY_FIXEDZOOM).equals("TRUE");
			useforbbox = s.getProperty(Background.KEY_USEFORBBOX)
					.equals("TRUE");
			backgroundColor = new Color(Integer.parseInt(s
					.getProperty(KEY_BACKGROUNDCOLOR)));
			backgroundColorChanged = true;
			color = new Color(Integer.parseInt(s
					.getProperty(KEY_FOREGROUNDCOLOR)));

			URL baseUrl = new URL(protocol, server, port, directory);
			Class c = Class.forName(s.getProperty(KEY_MAPPERCLASS));
			mapper = (Rect2UrlMapper) c.getConstructor(
					new Class[] { int.class, int.class, int.class, int.class,
							int.class, int.class, URL.class, String.class })
					.newInstance(
					tileSizeX, tileSizeY, DIM_X, DIM_Y, minZoomLevel,
					maxZoomLevel, baseUrl, prefix);

		} catch (Exception e) {
			System.err.println("The impossible has occured.");
			e.printStackTrace();
		}
	}

	/* Constants used as keys within Properties representing background settings */
	static final String KEY_SELECTION = "SELECTION";
	static final String KEY_PROTOCOL = "PROTOCOL";
	static final String KEY_SERVER = "SERVER";
	static final String KEY_PORT = "PORT";
	static final String KEY_DIRECTORY = "DIRECTORY";
	static final String KEY_PREFIX = "PREFIX";
	static final String KEY_MINZOOMLEVEL = "MINZOOMLEVEL";
	static final String KEY_MAXZOOMLEVEL = "MAXZOOMLEVEL";
	static final String KEY_MAXDOWNLOADS = "MAXDOWNLOADS";
	static final String KEY_LICENSEURL = "LICENSEURL";
	static final String KEY_TILESIZEX = "TILESIZEX";
	static final String KEY_TILESIZEY = "TILESIZEY";
	static final String KEY_NAME = "NAME";
	static final String KEY_SHOWFRAMES = "SHOWFRAMES";
	static final String KEY_SHOWNAMES = "SHOWNAMES";
	static final String KEY_FIXEDZOOM = "FIXEDZOOM";
	static final String KEY_BACKGROUNDCOLOR = "BACKGROUNDCOLOR";
	static final String KEY_FOREGROUNDCOLOR = "FOREGROUNDCOLOR";
	static final String KEY_MAPPERCLASS = "MAPPERCLASS";

	/** Path to store downloaded files **/
	private String PATH = "osmDowloads";

	/** maximum amount of parallel downloads **/
	private int maxDownloads = 2;

	/** size of the image cache in bytes **/
	private int CACHESIZE = 8 * 1024 * 1024; // 8 MB cache

	/** screen x resolution **/
	private int DIM_X;

	/** screen y resolution **/
	private int DIM_Y;

	/** maximum zoom level to be used **/
	private int maxZoomLevel = 18;
	/** minimum zoom level to be used **/
	private int minZoomLevel = 1;

	/** Size of a single tile in x dimension **/
	private int tileSizeX = 256;

	/** Size of a single tile in y dimension **/
	private int tileSizeY = 256;

	/* host name of the tile server. */
	private String protocol = "http";
	private String server = "tile.openstreetmap.org";
	private int port;
	private String directory = "/";
	private String prefix = "";

	/** DownloadManager for downloading images from osm **/
	private DownloadManager downloadManager;

  private DownloadManager preloadManager; 
  private PreloadObserver preloadObserver;

	/** cache for images **/
	private Cache<CachedImage, ImgLoader> imageCache;

	/** observer of active downloads **/
	private DownloadObserver observer;

	/** the parent component **/
	private JComponent lastParent = null;

	/** flag indicating whether the tiles are to be painted **/
	private boolean showTiles = true;

	/** flag indicating whether the tile frames are to be painted **/
	private boolean showFrames = false;

	/** flag indicating whether the tile names are to be painted **/
	private boolean showNames = false;

	/** color for frames and names **/
	private Color color = Color.RED;
	/** background color **/
	private Color backgroundColor = Color.YELLOW;
	/** flag indicating a change of the background color since last painting **/
	private boolean backgroundColorChanged = true;

	/** The configuration dialog **/
	private OSMDialog settings;

	/** Mapper for mapping world coordinates to map tile URLs **/
	private Rect2UrlMapper mapper;

  private boolean fixedZoomLevels = true;

}
