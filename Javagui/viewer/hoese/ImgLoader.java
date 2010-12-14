package viewer.hoese;

import tools.ObjectLoader;
import java.io.File;


public class ImgLoader implements ObjectLoader<CachedImage>{

     public CachedImage loadFromFile(File f){ 
        try{
          return new CachedImage(javax.imageio.ImageIO.read(f));
        } catch(Exception e){ 
          return null;
        }   
     }   
 }
