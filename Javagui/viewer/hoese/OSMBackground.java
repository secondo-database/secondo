
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
     bbox = new Rectangle2D.Double(-180,-90, 360,180); // able to display the world
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
     // basically there is no user interaction required for this viewer
     // maybe choosing of the directory of temporarly files
  }


  public void paint(JComponent parent, Graphics2D g, AffineTransform at, Rectangle2D clipRect){

     clipRect = clipRect.createIntersection(bbox);
     if(clipRect.isEmpty()){
        return;
     }
     parent.setBackground(Color.RED);  // non available tiles will be painted in red
     if(!clipRect.intersects(bbox)){
         return;   
     }
     LinkedList< Pair<URL, AffineTransform > > urls = computeURLs(clipRect);
     paintURLs(g,urls, at);      
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



  private LinkedList<Pair<URL, AffineTransform>> computeURLs(Rectangle2D bbox){
     System.out.println("enter compute URLS +++++++++++++++++++++++++++++++++++++++++++++++++++");

     int zoom = computeZoomLevel(bbox.getWidth(), bbox.getHeight());
     double noTiles = Math.pow(2,zoom);
     System.out.println("Zoom = " + zoom);
     System.out.println("no Tiles = " + noTiles);
     
     int x1 = (int)( ((bbox.getX()+180)*noTiles) / 360.0);
     int x2 = ((int)( ((bbox.getX()+bbox.getWidth()+180)*noTiles) / 360.0));

     
     int y1 = (int)( ((bbox.getY()+90)*noTiles) / 180.0);
     int y2 = ((int)( ((bbox.getY()+bbox.getHeight()+90)*noTiles) / 180.0)) ;


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
     return res; 
  }


  private Pair<URL, AffineTransform> computeURL(int x, int y, int z){
     // TODO : implement it
     try{
       URL url = new URL("http://"+SERVER+"/"+z+"/"+x+"/"+y+".png");
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
     double z_y = Math.log((180/DIM_Y) / (height*TILESIZE_Y)) / l2;
 
     System.out.println("z_x = " + z_x);
     System.out.println("z_y = " + z_y);


     double z = Math.max(z_x,z_y);
     int zoom = (int)(Math.ceil(z + .05));
     if(zoom>MAX_ZOOM){
        zoom = MAX_ZOOM;
     }
     if(zoom>0){
        zoom--;
     }
     return zoom;
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

  


}

