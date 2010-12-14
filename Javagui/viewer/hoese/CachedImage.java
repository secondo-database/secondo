

package viewer.hoese;

import tools.Cacheable;
import java.awt.image.BufferedImage;

public class CachedImage implements Cacheable{
    public CachedImage(BufferedImage img){
       this.img = img;
       if(img==null){
          this.size =0; 
       } else {
          this.size=img.getWidth()*img.getHeight()*3;
       }   
    }   

    public int getUsedMem(){
       return size; 
    }  

    public BufferedImage getImage(){
      return img;
    } 

   private BufferedImage img;
   private int size;

 }

