
package viewer.hoese;

import java.awt.geom.Rectangle2D;
import java.awt.Shape;
import java.awt.Paint;
import java.awt.Rectangle;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.geom.Point2D;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.util.LinkedList;
import java.util.Iterator;
import javax.swing.JComponent;
import java.io.File;
import java.net.URL;
import java.util.Properties;

import tools.downloadmanager.DownloadManager;
import tools.downloadmanager.DownloadObserver;
import tools.downloadmanager.DownloadEvent;
import tools.downloadmanager.DownloadState;
import tools.downloadmanager.ActiveDownload;
import tools.Cache;
import tools.Pair;


/** Implementation of Open Street Map Tiles as Background for 
  * the HoeseViewer.
  **/

public class OSMBackground extends Background {

  /** Creates a new OSMBackground initialized with default values. **/
  public OSMBackground(){

     Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
     DIM_X = (int) dim.getWidth();
     DIM_Y = (int) dim.getHeight();
     name = "OSMBackground";
     double maxY = 85.0511;  

    
     bbox = new Rectangle2D.Double(-180,-maxY, 360,2*maxY); // able to display the "world"
     listeners = new LinkedList<BackgroundListener>();
     useforbbox = false;
     try{ 
        downloadManager = new DownloadManager(new File(PATH), maxDownloads); 
     } catch (Exception e){
        System.err.println("Problem in initiating download manager");
        downloadManager = null;
     }
    
     downloadManager.setConnectTimeout(5000);
     downloadManager.setReadTimeout(5000);
     
 
     observer = new DownloadObserver(){
        public void downloadStateChanged(DownloadEvent evt){
           OSMBackground.this.downloadStateChanged(evt);
        }
     };
     imageCache = new Cache<CachedImage, ImgLoader>(CACHESIZE, new ImgLoader());

     settings = new OSMDialog(null);
  }

  /** Overrides the setBBox function from Background. Does nothing bacause the 
    ** bounding box is fixed by the server. 
    * @param rect Dummy only!
    **/
  public void setBBox(Rectangle2D.Double rect){ }


  /** Shows a dialog to configure this background. 
   * @param f Used as the dialog's parent  
   * **/
  public void showConfigDialog(JComponent f){ 
     settings.showDialog(); 
     Properties props = new Properties();
     settings.readSettings(props);
     setCheckedConfiguration(props);
  }


  /** Overrides the paint method from Background. Paints the tiles downloaded from
    * a specific server.
    * @param parent The parent component containing the background.
    * @param g The garphics context to paint on.
    * @param at Affine transformation from world to screen coordinates.
    * @param clipRect The part of the world to be drawn (world coordinates).
    */
  public void paint(JComponent parent, Graphics2D g, AffineTransform at, Rectangle2D clipRect){
     // set background color
     if(backgroundColorChanged && parent !=null){
       backgroundColorChanged = false;
       parent.setBackground(backgroundColor);
     }

     // compute the bounding box in geographic coordinates
     if(ProjectionManager.isReversible()){
        java.awt.geom.Point2D.Double p1 = new java.awt.geom.Point2D.Double();
        java.awt.geom.Point2D.Double p2 = new java.awt.geom.Point2D.Double();
        ProjectionManager.getOrig(clipRect.getX(), clipRect.getY(), p1);
        ProjectionManager.getOrig(clipRect.getX()+clipRect.getWidth(), clipRect.getY() + clipRect.getHeight(),  p2);
        clipRect.setRect(p1.getX(), p1.getY(), p2.getX()-p1.getX(), p2.getY()-p1.getY()); 
     }


     try{
       lastParent = parent;
       // only use the part inside the world
       clipRect = clipRect.createIntersection(bbox);
       if(clipRect.isEmpty()){
          return;
       }
       // compute the urls of all required tiles and 
       // define an affine transformation to move the tile to it's location within the world
       LinkedList< Pair<URL, AffineTransform > > urls = computeURLs((Rectangle2D.Double)clipRect);
       
       // paint the tiles, frames, and labels
       if(urls!=null){
          paintURLs(g,urls, at);      
       }
    } catch(Exception e ){
        e.printStackTrace();
    }
  }

  /** Paint the tiles, frames, and labels coming from urls depending on some
    * flag members. 
    * @param g Graphics context to paint on
    * @param urls Urls to be painted
    * @param at trrectansformation from world to screen coordinates
    */
  private void paintURLs(Graphics2D g, LinkedList<Pair<URL, AffineTransform> > urls, AffineTransform at){
      
      Iterator<Pair<URL, AffineTransform> > it = urls.iterator();
      if(showTiles){ 
        while(it.hasNext()){
           paintURL(g, it.next(), at,false); 
        }
      }
      Paint paint = g.getPaint();
      g.setPaint(color);

      if(showFrames){
         it = urls.iterator();
         while(it.hasNext()){
            paintURL(g, it.next(), at, true); 
         }
      }

      if(showNames){
         it = urls.iterator();
         while(it.hasNext()){
            paintName(g, it.next(), at); 
         }
      }
      g.setPaint(paint);
  }


  
  /** Paint a single tile or frame depending on the flag frame.
    * @param g graphics context to paint on
    * @param url source of the image and a transformation to map the image into the world
    * @param at mapping from world to screen coordinates
    * @param frame if set to true, only the frame of the image is painted, otherwise the image 
    */
  private void paintURL(Graphics2D g, Pair<URL, AffineTransform> url, AffineTransform at, boolean frame){
      // start download 
      File f = downloadManager.getURL(url.first(), observer);

      if(f!=null){ // url already downloaded
         CachedImage cimg =  imageCache.getElem(f);
         if(cimg==null){
              System.err.println("could not load image from file " + f);
              f.delete();
         } else {
           BufferedImage img = cimg.getImage();
           if(img != null){
             if(frame){
                paintImageFrame(g, cimg.getImage(), url.second(), at);
              } else {
                paintImage(g, cimg.getImage(), url.second(), at);
              }
            } else {
              System.err.println("could not extract image from file " + f);
            }
        } 
      }
  }


/**
 * Pain the name of a map tile image file to the screen
 * @param g Graphics context to draw on.
 * @param url Map tile, whose name is to be printed.
 * @param at Affine transformation from world to screen coordinates.
 */
  private void paintName(Graphics2D g, Pair<URL, AffineTransform> url, AffineTransform at){

    String labelText = url.first().getFile().toString();
    Rectangle2D.Double rImg = new Rectangle2D.Double(0,0, tileSizeX, tileSizeY);
    Rectangle2D rWorld = url.second().createTransformedShape(rImg).getBounds2D();   
    Point2D.Double p = new Point2D.Double(rWorld.getX() + rWorld.getWidth()/2, rWorld.getY() + rWorld.getHeight()/2);
    at.transform(p, p);
    float x = (float) p.getX();
    float y = (float) p.getY();
    Rectangle2D re = g.getFont().getStringBounds(labelText, g.getFontRenderContext());
     x -= re.getWidth()/2.0;
     y += re.getHeight()/2.0;
    g.drawString(labelText, x,y);
  }


  /** Paints a buffered image to a graphics context using the given  transformations.
    * @param g the graphics context to paint on
    * @param img the image to paint
    * @param img2world mapping from image coordinates to world coordinates
    * @param world2screnn mapping from world coordinates to screen coordinates
    **/ 
  private void paintImage(Graphics2D g, BufferedImage img, AffineTransform img2world, AffineTransform world2Screen){
     if(img!=null){
       AffineTransform at = new AffineTransform(world2Screen);
       at.concatenate(img2world);
       g.drawImage(img,at,null);
     }
  }


  /** paint the frame of an image to g.
    * @param g the graphics context to paint on
    * @param img the image whose frame is to be painted 
    * @param img2world mapping from image coordinates to world coordinates
    * @param world2screnn mapping from world coordinates to screen coordinates
    **/ 
  private void paintImageFrame(Graphics2D g, BufferedImage img, AffineTransform img2world, AffineTransform world2Screen){
     if(img!=null){
        AffineTransform at = new AffineTransform(world2Screen);
        at.concatenate(img2world);
        Rectangle2D.Double r = new Rectangle2D.Double(0,0,img.getWidth(), img.getHeight());
        Shape s = at.createTransformedShape(r);
        Rectangle r1 = s.getBounds();
        g.drawRect((int)r1.getX(), (int)r1.getY(), (int)r1.getWidth(), (int)r1.getHeight());
      }
  }

  /** Callback method. Called when a state of a pending download is changed.
    * Calls the  repaint method  of the parent component.
    * @param evt Event to handle.
    **/
  private void downloadStateChanged(DownloadEvent evt){
     DownloadState state = evt.getState();
     //ActiveDownload ad = evt.getSource();
     if(lastParent!=null && state == DownloadState.DONE){
       lastParent.repaint();
     }

  }
  
  /** returns the X index of a tile covering longitude  at specified zoom level.
    * @param zoom the used zoom level
    * @param lon longitude
    * @return the X-index of the tile covering the given longitude
    **/
  private int getTileX(int zoom, double lon){
     return   (int)Math.floor( (lon + 180) / 360 * (1<<zoom) ) ;
  }

  /** returns the Y index of a tile covering latitude  at specified zoom level.
    * @param zoom the used zoom level
    * @param lat latitude
    * @return the Y-index of the tile covering the given latitude
    **/
  private int getTileY(int zoom, double lat){
   return (int)Math.floor( (1 - Math.log(Math.tan(Math.toRadians(lat)) + 1 / Math.cos(Math.toRadians(lat))) / Math.PI) / 2 * (1<<zoom) ) ;
  }

  /** Computes the urls required to cover at least the given rectangle. 
    * @param bbox the rectangle to be covered in world coordinates
    * @return The list of all visibile map tiles and according translation/scale matrices    
    **/
  private LinkedList<Pair<URL, AffineTransform>> computeURLs(Rectangle2D.Double bbox){
     if(bbox==null){
       return null;
     }
     
     if(bbox.equals(lastClipRect)){
        return lastURLs;
     }

     int zoom = computeZoomLevel(bbox.getWidth(), bbox.getHeight());

     int x1 = getTileX(zoom, bbox.getX());
     int x2 = getTileX(zoom, bbox.getX()+bbox.getWidth());
     int y1 = getTileY(zoom, bbox.getY() + bbox.getHeight()); 
     int y2 = getTileY(zoom, bbox.getY());

     LinkedList<Pair<URL, AffineTransform>> res = new LinkedList<Pair<URL,AffineTransform>>();
     for(int x = x1; x <= x2; x++){
        for(int y = y1; y <= y2; y++){
            Pair<URL, AffineTransform> p = computeURL(x,y,zoom);
            if(p!=null){
              res.add(p);
            } else {
              System.err.println("Problem in computinmg URL from (x,y,z) = (" + x + ", " + y + ", " + zoom +")");
            }
        }
     } 
     lastURLs = res;
     return res; 
  }


  /** Computes a url from the x,y indexes at specified zoom level. Additionally,
    * a affine transformation is computed to map the image to be retrieved from the url 
    * to the proper location within the world.
    * @param x X index of a tile
    * @param y Y index of a tile
    * @param z zoom level 
    * @return A pair of image URL and affine transformation to shift it to its proper location and zoom it according to zoom level and visible screen
    **/
  private Pair<URL, AffineTransform> computeURL(int x, int y, int z){
     URL url;
     try{
       URL base = new URL(protocol, server, port, directory);
       url = new URL(base, ""+ z+"/"+x+"/"+y+ ".png");
     } catch(Exception e){
         e.printStackTrace();
         return null;
     }
     Rectangle2D.Double r = getBBoxForTile(x,y,z);


     // compute projected bounding box
     java.awt.geom.Point2D.Double p1 = new java.awt.geom.Point2D.Double();
     java.awt.geom.Point2D.Double p2 = new java.awt.geom.Point2D.Double();

     ProjectionManager.project(r.getX(), r.getY(), p1);
     ProjectionManager.project(r.getX()+r.getWidth(), r.getY() + r.getHeight(),  p2);

     r.setRect(p1.getX(), p1.getY(), p2.getX()-p1.getX(), p2.getY()-p1.getY()); 

     double scale_x =  r.getWidth() / tileSizeX;
     double scale_y = -1.0 * r.getHeight() / tileSizeY;

     AffineTransform at = AffineTransform.getTranslateInstance(r.getX(), r.getY() + r.getHeight()); 
     at.scale(scale_x, scale_y);
     return new Pair<URL, AffineTransform>(url,at);
  }


  /** compute a zoom level for a given size within the world.
    * @param witdh size in x dimension within the world
    * @param height size in y dimension within the world
    * @return The recommended zoom level
    **/
  private int computeZoomLevel(double width, double height){
     double z_x = Math.log((360*DIM_X) / (width*tileSizeX) ) / l2;   // computing log_2
     double z_y = Math.log((180*DIM_Y) / (height*tileSizeY)) / l2;
     double z = Math.max(z_x,z_y);
     int zoom = (int) z;
     if(zoom > maxZoomLevel){
        zoom = maxZoomLevel;
     }
     if(zoom<minZoomLevel){
        zoom = minZoomLevel;
     }
     return zoom;
  }  


  /** compute the bounding box covered by a specified tile in world coordinates.
    * @param x X index of a tile
    * @param y Y index of a tile
    * @param z zoom level 
    * @return The images's MBR in world coordinates  
    **/
  private Rectangle2D.Double getBBoxForTile(int x, int y, int zoom){
    double north = tile2lat(y, zoom);
    double south = tile2lat(y + 1, zoom);
    double west = tile2lon(x, zoom);
    double east = tile2lon(x + 1, zoom);
    return new Rectangle2D.Double(west, south, east-west, north-south);  
  }

  /** Export the background's properties
   * @param backgroundDataPath Directory for storing files
   * @return The backgound's properties
   */
  public Properties getConfiguration(String backgroundDataPath) {
    Properties res = super.getConfiguration(backgroundDataPath);
    settings.readSettings(res);
    return res;
  }

  /** Set the background properties
   * @param properties The background properties to restore
   * @param backgroundDataPath Directory with additional data files
   */
  @Override
  public void setConfiguration(Properties properties, String backgroundDataPath){
     super.setConfiguration(properties, backgroundDataPath);
     
     settings.reset(properties);
     Properties corrected = new Properties();
     settings.readSettings(corrected);
     setCheckedConfiguration(corrected);

      
  }

  /**
   * Set previously validated background settings
   * @param s The validated background properties to restore
   */
  private void setCheckedConfiguration(Properties s ){
     // now we can be sure that all the values are correct :-)

   try{
      protocol  = s.getProperty(KEY_PROTOCOL); 
      server    = s.getProperty(KEY_SERVER);
      port      = Integer.parseInt(s.getProperty(KEY_PORT)); 

      directory = s.getProperty(KEY_DIRECTORY);
      minZoomLevel = Integer.parseInt(s.getProperty(KEY_MINZOOMLEVEL)); 
      maxZoomLevel = Integer.parseInt(s.getProperty(KEY_MAXZOOMLEVEL)); 
      maxDownloads = Integer.parseInt(s.getProperty(KEY_MAXDOWNLOADS)); 
      tileSizeX = Integer.parseInt(s.getProperty(KEY_TILESIZEX));
      tileSizeY = Integer.parseInt(s.getProperty(KEY_TILESIZEY));
      name = s.getProperty(KEY_NAME);
      showFrames = s.getProperty(KEY_SHOWFRAMES).equals("TRUE");
      showNames = s.getProperty(KEY_SHOWNAMES).equals("TRUE"); 
      backgroundColor = new Color(Integer.parseInt(s.getProperty(KEY_BACKGROUNDCOLOR )));
      backgroundColorChanged = true;
      color = new Color(Integer.parseInt(s.getProperty(KEY_FOREGROUNDCOLOR )));
    } catch(Exception e){
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
  static final String KEY_MINZOOMLEVEL = "MINZOOMLEVEL";
  static final String KEY_MAXZOOMLEVEL = "MAXZOOMLEVEL";
  static final String KEY_MAXDOWNLOADS = "MAXDOWNLOADS";
  static final String KEY_LICENSEURL = "LICENSEURL";
  static final String KEY_TILESIZEX = "TILESIZEX";
  static final String KEY_TILESIZEY = "TILESIZEY";
  static final String KEY_NAME = "NAME";
  static final String KEY_SHOWFRAMES = "SHOWFRAMES";
  static final String KEY_SHOWNAMES = "SHOWNAMES";
  static final String KEY_BACKGROUNDCOLOR = "BACKGROUNDCOLOR";
  static final String KEY_FOREGROUNDCOLOR = "FOREGROUNDCOLOR";


  /** computes the western boundary of a specified tile in world coordinates.
   * @param x The map tile's X-index
   * @param z The zoom level
   * @return The longitude of the western boundary of a map tile
    **/
  private static double tile2lon(int x, int z) {
     return x / Math.pow(2.0, z) * 360.0 - 180;
  }
 
  /** computes the northern  boundary of a specified tile in world coordinates.
   * @param x The map tile's Y-index
   * @param z The zoom level
   * @return The latitude of the nortehrn boundary of a map tile
    **/
  private static double tile2lat(int y, int z) {
    double n = Math.PI - (2.0 * Math.PI * y) / Math.pow(2.0, z);
    return Math.toDegrees(Math.atan(Math.sinh(n)));
  }

  /** Path to store downloaded files **/
  private String PATH = "osmDowloads"; 

  /** maximum amount of parallel downloads **/
  private int maxDownloads = 2;

  /** size of the image cache in bytes **/
  private int CACHESIZE = 8*1024*1024;   // 8 MB cache

  /** screen x resolution **/
  private int DIM_X;

  /** screen y resolution **/
  private int DIM_Y;

  /** maximum zoom level to be used **/
  private int maxZoomLevel = 18;
  /** minimum zoom level to be used **/
  private int minZoomLevel  = 1;

 
  /** Size of a single tile in x dimension **/
  private int tileSizeX = 256;

  /** Size of a single tile in y dimension **/
  private  int tileSizeY = 256;


  /** logarithm of 2 to save computation time **/
  private static final double l2 = Math.log(2);


  /* host name of the tile server. */
  private  String protocol = "http";
  private  String server = "tile.openstreetmap.org";
  private int port;
  private  String directory = "/";


  /** DownloadManager for downloading images from osm **/
  private DownloadManager downloadManager;

  /** cache for images **/ 
  private Cache<CachedImage, ImgLoader> imageCache;
  
  /** observer of active downloads **/
  private DownloadObserver observer; 

  /** the parent component **/
  private JComponent lastParent = null;

  /** the last used bounding box **/
  private Rectangle2D lastClipRect = null;

  /** the urls according to the last used bounding box **/
  LinkedList<Pair<URL, AffineTransform>>  lastURLs;

  /** flag indicating whether the tiles are to be painted **/
  private boolean showTiles = true;

  /** flag indicating whether the tile frames are to be painted **/
  private boolean showFrames=false;

  /** flag indicating whether the tile names are to be painted **/
  private boolean showNames=false;

  /** color for frames and names **/
  private Color color = Color.RED;
  /** background color **/
  private Color backgroundColor = Color.YELLOW;
  /** flag indicating a change of the background color since last painting **/
  private boolean backgroundColorChanged = true;

  /** The configuration dialog **/
  private OSMDialog settings;

}

