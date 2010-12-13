
package viewer.hoese;

import java.awt.geom.Rectangle2D;
import java.awt.Shape;
import java.awt.Rectangle;
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
import tools.downloadmanager.DownloadState;
import tools.downloadmanager.ActiveDownload;
import tools.Cache;
import tools.Pair;


/** Implementation of Open Street Map Tiles as Background for 
  * the HoeseViewer.
  **/

public class OSMBackground extends Background {


  public OSMBackground(){
     name = "OSMBackground";
     double maxY = 85.0511;  

    
     bbox = new Rectangle2D.Double(-180,-maxY, 360,2*maxY); // able to display the "world"
     listeners = new LinkedList<BackgroundListener>();
     useforbbox = false;
     try{ 
        downloadManager = new DownloadManager(new File(PATH), MAXDOWNLOADS); 
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
  }

  // don't allow to change the bounding box from outside
  public void setBBox(Rectangle2D.Double rect){ }


  public void showConfigDialog(JComponent f){ 
     // TODO: settings for maxDownloads etc.

  }


  public void paint(JComponent parent, Graphics2D g, AffineTransform at, Rectangle2D clipRect){
     try{
       lastParent = parent;
       clipRect = clipRect.createIntersection(bbox);
       if(clipRect.isEmpty()){
          return;
       }

       LinkedList< Pair<URL, AffineTransform > > urls = computeURLs((Rectangle2D.Double)clipRect);
       if(urls!=null){
          paintURLs(g,urls, at);      
       }
    } catch(Exception e ){
        e.printStackTrace();
    }
  }


  private void paintURLs(Graphics2D g, LinkedList<Pair<URL, AffineTransform> > urls, AffineTransform at){
      //g.setTransform(new AffineTransform());A
      Iterator<Pair<URL, AffineTransform> > it = urls.iterator();
      while(it.hasNext()){
         paintURL(g, it.next(), at,false); 
      }
      it = urls.iterator();
      while(it.hasNext()){
         paintURL(g, it.next(), at, true); 
      }
  }

  private void paintURL(Graphics2D g, Pair<URL, AffineTransform> url, AffineTransform at, boolean frame){
      File f = downloadManager.getURL(url.first(), observer);

      if(f!=null){ // url already downloaded
         CachedImage cimg =  imageCache.getElem(f);
         if(cimg==null){
              System.err.println("could not load image from file " + f);
              f.delete();
         } else if(frame){
               paintImageFrame(g, cimg.getImage(), url.second(), at);
         } else {
               paintImage(g, cimg.getImage(), url.second(), at);
         }
      } 
  }


  private void paintImage(Graphics2D g, BufferedImage img, AffineTransform img2world, AffineTransform world2Screen){
      AffineTransform at = new AffineTransform(world2Screen);
      at.concatenate(img2world);
      g.drawImage(img,at,null);
  }



  private void paintImageFrame(Graphics2D g, BufferedImage img, AffineTransform img2world, AffineTransform world2Screen){
      AffineTransform at = new AffineTransform(world2Screen);
      at.concatenate(img2world);
      Rectangle2D.Double r = new Rectangle2D.Double(0,0,img.getWidth(), img.getHeight());
      Shape s = at.createTransformedShape(r);
      Rectangle r1 = s.getBounds();
      g.drawRect((int)r1.getX(), (int)r1.getY(), (int)r1.getWidth(), (int)r1.getHeight());
  }


  private void downloadStateChanged(DownloadEvent evt){
     DownloadState state = evt.getState();
     ActiveDownload ad = evt.getSource();
     if(lastParent!=null && state == DownloadState.DONE){
       lastParent.repaint();
     }

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


  private Pair<URL, AffineTransform> computeURL(int x, int y, int z){
     URL url;
     try{
       url = new URL("http://"+SERVER+"/"+z+"/"+x+"/"+y+".png");
     } catch(Exception e){
         e.printStackTrace();
         return null;
     }
     Rectangle2D.Double r = getBBoxForTile(x,y,z);


     double scale_x =  r.getWidth() / TILESIZE_X;
     double scale_y = -1.0 * r.getHeight() / TILESIZE_Y;

     AffineTransform at = AffineTransform.getTranslateInstance(r.getX(), r.getY() + r.getHeight()); 
     at.scale(scale_x, scale_y);
     return new Pair<URL, AffineTransform>(url,at);
  }


  private int computeZoomLevel(double width, double height){
     double z_x = Math.log((360*DIM_X) / (width*TILESIZE_X) ) / l2;   // computing log_2
     double z_y = Math.log((180*DIM_Y) / (height*TILESIZE_Y)) / l2;
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

  private int MAXDOWNLOADS = 22;

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

  JComponent lastParent = null;


  Rectangle2D lastClipRect = null;
  LinkedList<Pair<URL, AffineTransform>>  lastURLs;


}

