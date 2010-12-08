

package tools;


import java.io.File;
import java.awt.image.BufferedImage;
import java.util.Vector;

public class CacheTest{



   private static Vector<File> getImgs(File root){ // returns image files of a directory
      File[] files = root.listFiles();
      Vector<File> res = new Vector<File>();
      for(int i=0;i<files.length;i++){
         File f = files[i];
         if(f.isFile() && f.canRead() ) {
           String name = f.getName();
           if(name.endsWith("jpg") || name.endsWith("gif") || name.endsWith("png")){
             res.add(f);
           }
         }
      }
      return res;
   }

   public static void main(String[] args){

     if(args.length!=1){
        System.out.println("missing argument");
        System.exit(1);
     } 
     File f = new File(args[0]);
     if(!f.exists() || !f.isDirectory()){
        System.out.println("argument must be an existing directory");
        System.exit(1);
     }

     Vector<File> imgs = getImgs(f);
     System.out.println("Start test with" + imgs.size() + "Files");

     Cache<CachedImage,ImgLoader> cache = new Cache<CachedImage, ImgLoader>(2*1024*1024 , new ImgLoader());

     // first test, simple cache all files
     for(int i=0;i<imgs.size();i++){
        CachedImage img = cache.getElem(imgs.get(i));
        if(img==null){
           System.out.println("Problem with file " + imgs.get(i));
        } 
        System.out.println(cache);
     }
     


   }


static  class ImgLoader implements ObjectLoader<CachedImage>{

     public CachedImage loadFromFile(File f){
        try{
          return new CachedImage(javax.imageio.ImageIO.read(f));
        } catch(Exception e){
          return null;
        }
     }
 }


 static class CachedImage implements Cacheable{
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

   private BufferedImage img;
   private int size;

 }



}

