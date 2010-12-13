
package viewer.hoese;

import java.awt.geom.Rectangle2D;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.util.LinkedList;
import java.util.Iterator;
import javax.swing.JComponent;
import java.io.File;
import java.net.URL;

import tools.downloadmanager.DownloadManager;
import tools.downloadmanager.DownloadObserver;
import tools.downloadmanager.DownloadEvent;
import tools.downloadmanager.DownloadObserver;
import tools.Cache;
import tools.Pair;


/** Implementation of Open Street Map Tiles as Background for 
  * the HoeseViewer.
  **/

public class OSMBackground extends Background {


  public OSMBackground(){
     name = "OSMBackground";
     bbox = new Rectangle2D.Double(-180,-90, 360,180); // able to display the "world"
     listeners = new LinkedList<BackgroundListener>();
     useforbbox = false;
     try{ 
        downloadManager = new DownloadManager(new File(PATH), MAXDOWNLOADS); 
     } catch (Exception e){
        System.err.println("Problem in initiating download manager");
        downloadManager = null;
     }
     
     observer = new DownloadObserver(){
        public void downloadStateChanged(DownloadEvent evt){
           OSMBackground.this.downloadStateChanged(evt);
        }
     };
     imageCache = new Cache<CachedImage, ImgLoader>(CACHESIZE, new ImgLoader());
  }

  // don't allow to change the bounding box from outside
  public void setBBox(Rectangle2D.Double rect){ }


  public void showConfigDialog(JComponent f){ 
     // TODO: settings for maxDownloads etc.

  }


  public void paint(JComponent parent, Graphics2D g, AffineTransform at, Rectangle2D clipRect){
     try{
        clipRect = clipRect.createIntersection(bbox);
        if(clipRect.isEmpty()){
          return;
        }
        //parent.setBackground(Color.RED);  // non available tiles will be painted in red
        LinkedList< Pair<URL, AffineTransform > > urls = computeURLs((Rectangle2D.Double)clipRect);
        if(urls!=null){
           paintURLs(g,urls, at);      
        }
    } catch(Exception e ){
        e.printStackTrace();
    }
  }


  private void paintURLs(Graphics2D g, LinkedList<Pair<URL, AffineTransform> > urls, AffineTransform at){
      Iterator<Pair<URL, AffineTransform> > it = urls.iterator();
      while(it.hasNext()){
         paintURL(g, it.next(), at); 
      }
  }

  private void paintURL(Graphics2D g, Pair<URL, AffineTransform> url, AffineTransform at){
      File f = downloadManager.getURL(url.first(), observer);
      if(f!=null){ // url already downloaded
         CachedImage cimg =  imageCache.getElem(f);
         if(cimg==null){
              System.err.println("could not load image from file " + f);
              f.delete();
         } else {
               paintImage(g, cimg.getImage(), url.second(), at);
         }
      }
  }


  private void paintImage(Graphics2D g, BufferedImage img, AffineTransform img2world, AffineTransform world2Screen){
      AffineTransform at = new AffineTransform(world2Screen);
      at.preConcatenate(img2world);
      g.drawImage(img,at,null);
  }

  private void downloadStateChanged(DownloadEvent evt){
     // TODO:  check for new state of download
     // inform listeners
  }



  private int getTileX(int zoom, double lon){
     return   (int)Math.floor( (lon + 180) / 360 * (1<<zoom) ) ;
  }

  private int getTileY(int zoom, double lat){
   return (int)Math.floor( (1 - Math.log(Math.tan(Math.toRadians(lat)) + 1 / Math.cos(Math.toRadians(lat))) / Math.PI) / 2 * (1<<zoom) ) ;
  }


  private LinkedList<Pair<URL, AffineTransform>> computeURLs(Rectangle2D.Double bbox){
     if(bbox==null){
       return null;
     }
     
     if(bbox.equals(lastClipRect)){
        return lastURLs;
     }

     System.out.println("enter compute URLS +++++++++++++++++++++++++++++++++++++++++++++++++++");

     int zoom = computeZoomLevel(bbox.getWidth(), bbox.getHeight());

     System.out.println("Zoom = " + zoom);



     System.out.println("bbox = " + bbox);
     System.out.println("bbClass = " + bbox.getClass().getName());

     System.out.println("wx1 = " + bbox.getX());
     System.out.println("wx2 = " + (bbox.getX()+bbox.getWidth()));
     System.out.println("wy1 = " + bbox.getY());
     System.out.println("wy2 = " + (bbox.getY()+bbox.getHeight()));
    
     int x1 = getTileX(zoom, bbox.getX());
     int x2 = getTileX(zoom, bbox.getX()+bbox.getWidth());
     int y1 = getTileY(zoom, bbox.getY() + bbox.getHeight()); 
     int y2 = getTileY(zoom, bbox.getY());


     // debug::start
      double lon = 51.17;
      double lat = 7.33;
      System.out.println("TileX = " + getTileX(zoom , lat));
      System.out.println("TileY = " + getTileY(zoom, lon));
     // debug::end
     
     System.out.println("computeURLS: (x1, y1 , x2, y2) = (" + x1+", " + y1+ ", " + x2 + ", " + y2+")");


     LinkedList<Pair<URL, AffineTransform>> res = new LinkedList<Pair<URL,AffineTransform>>();
     for(int x = x1; x <= x2; x++){
        for(int y = y1; y <= y2; y++){
            Pair<URL, AffineTransform> p = computeURL(x,y,zoom);
            if(p!=null){
              res.add(p);
            }
        }
     } 
     System.out.println("leave compute URLS +++++++++++++++++++++++++++++++++++++++++++++++++++");
     lastURLs = res;
     return res; 
  }


  private Pair<URL, AffineTransform> computeURL(int x, int y, int z){
     // TODO : implement it
     try{
       URL url = new URL("http://"+SERVER+"/"+z+"/"+x+"/"+y+".png");
       System.out.println("URL = " + url);
     } catch(Exception e){
         e.printStackTrace();
         return null;
     }
     //AffineTransform at = AffineTransform.getScaleInstance( TILESIZE_X * getTileSizeX(z) , TILESIZE_Y * getTileSizeY(z)); 
     //at.translate( getTileSizeX(z)*x -180 -x, getTileSizeY(z)*y - 90 - y);
     return null;
  }


  private int computeZoomLevel(double width, double height){
          


     double z_x = Math.log((360*DIM_X) / (width*TILESIZE_X) ) / l2;   // computing log_2
     double z_y = Math.log((180*DIM_Y) / (height*TILESIZE_Y)) / l2;
 
     System.out.println("z_x = " + z_x);
     System.out.println("z_y = " + z_y);


     double z = Math.max(z_x,z_y);
     return (int) z;

  }  



  Rectangle2D.Double getBBoxForTile(int x, int y, int zoom){
    double north = tile2lat(y, zoom);
    double south = tile2lat(y + 1, zoom);
    double west = tile2lon(x, zoom);
    double east = tile2lon(x + 1, zoom);
    return new Rectangle2D.Double(west, south, east-west, north-south);  
  }
 
  static double tile2lon(int x, int z) {
     return x / Math.pow(2.0, z) * 360.0 - 180;
  }
 
  static double tile2lat(int y, int z) {
    double n = Math.PI - (2.0 * Math.PI * y) / Math.pow(2.0, z);
    return Math.toDegrees(Math.atan(Math.sinh(n)));
  }



 
  /** Path to store downloaded files **/
  private String PATH = "osmDowloads"; 

  private int MAXDOWNLOADS = 5;

  private int CACHESIZE = 8*1024*1024;   // 8 MB cache

  private int DIM_X = 1024;
  private int DIM_Y = 768;


  private static final int MAX_ZOOM = 18;


  private static final int TILESIZE_X = 256;
  private static final int TILESIZE_Y = 256;


  private static final double l2 = Math.log(2);



  private static final String SERVER = "tile.openstreetmap.org";


  /** DownloadManager for downloading images from osm **/
  DownloadManager downloadManager;

  Cache<CachedImage, ImgLoader> imageCache;
  
  DownloadObserver observer; 


  Rectangle2D lastClipRect = null;
  LinkedList<Pair<URL, AffineTransform>>  lastURLs;


}

