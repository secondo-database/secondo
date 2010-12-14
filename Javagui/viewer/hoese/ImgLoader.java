package viewer.hoese;

import tools.ObjectLoader;
import java.io.File;


public class ImgLoader implements ObjectLoader<CachedImage>{

     public CachedImage loadFromFile(File f){ 
        try{
          return new CachedImage(javax.imageio.ImageIO.read(f));
        } catch(Exception e){ 
          //e.printStackTrace();
          System.err.println("problem in reading image from "+f);
          tools.Reporter.debug(e);
          return null;
        }   
     }   
 }
