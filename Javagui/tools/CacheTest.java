

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
     if(imgs.size()==0){
        return;
     }

     Cache<CachedImage,ImgLoader> cache = new Cache<CachedImage, ImgLoader>(2*1024*1024 , new ImgLoader());

     long t1 = System.currentTimeMillis();
     // first test, simple cache all files
     System.out.println("######################  Test 1 simple ########################################");
     for(int i=0;i<imgs.size();i++){
        CachedImage img = cache.getElem(imgs.get(i));
        if(img==null){
           System.out.println("Problem with file " + imgs.get(i));
        } 
        System.out.println(cache);
     }
     long t2 = System.currentTimeMillis();
     System.out.println("Test 1 has taken " + (t2-t1 ) + " ms");
     t1 = t2;


     System.out.println("##################### Test 2 twice insertion ########################################");
     cache.clear();
     int max = 1;
     for(int i=0;i<imgs.size(); i++){
        CachedImage img = cache.getElem(imgs.get(i));
        System.out.println("First = " + cache);
        img = cache.getElem(imgs.get(i));
        if(img==null){
           System.out.println("Problem with file " + imgs.get(i));
        } 
        System.out.println("Second: "+ cache);
        max = Math.max(max,cache.size());
     }

     t2 = System.currentTimeMillis();
     System.out.println("Test 2 has taken " + (t2-t1 ) + " ms");
     t1 = t2;


     System.out.println("################### Test 3 random from 2*maximum reached size , 100.000 times#################################");
     cache.clear();
     java.util.Random R = new java.util.Random(1);
     for(int i=0;i<imgs.size(); i++){
        int num = R.nextInt(max + 1);
        CachedImage img = cache.getElem(imgs.get(num));
        System.out.println( cache);
        if(img==null){
           System.out.println("Problem with file " + imgs.get(i));
        } 
     }


     t2 = System.currentTimeMillis();
     System.out.println("Test 3 has taken " + (t2-t1 ) + " ms");
     t1 = t2;

      
     


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

