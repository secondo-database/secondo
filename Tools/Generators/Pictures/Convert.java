

import java.awt.image.BufferedImage;
import java.awt.Graphics2D;
import java.awt.Color;
import javax.imageio.ImageIO;
import java.io.*;
import java.util.Date;

/* class for resizing images
 *
 */

public class Convert {

static final int W = 0; // fixed witdh
static final int H = 1; // fixed height
static final int S = 2; // scale in percent
static final int M = 4; // maximum of width and height
static final boolean DEBUG = true; 

static PrintStream out = System.out;

static java.text.DateFormat dformat = new java.text.SimpleDateFormat("yyyy-MM-dd-HH:mm");

private static boolean convert(File source, File target, double scale,
                               int scaletype){
   try{
     BufferedImage img = ImageIO.read((source));
     if(img==null){
        return false;
     }
     File f = (target);
     // create the directory if needed:
     File p = f.getParentFile();
     if(p!=null && p.isDirectory()){
        p.mkdir();
     }
     if(f.exists()){
        return false;
     }
     int ow = img.getWidth();
     int oh = img.getHeight();
     if( (ow==0) ||  (oh==0)){
       return false;
     }
     double factor = 1.0;
     switch(scaletype){
        case W : { factor = scale / ow; break; }
        case H : { factor = scale / oh; break; }
        case S : { factor = scale / 100; break;}
        case M : { factor = scale / Math.max(ow,oh);break; }
     }

     // create the scaled version
     int h = (int) (oh*factor);
     int w = (int) (ow*factor);
     if(h<1) {
       h = 1;
     }
     if(w<1){
       w = 1;
     }
     BufferedImage sc_img = new BufferedImage(w,h,BufferedImage.TYPE_INT_RGB);
     Graphics2D g = sc_img.createGraphics();
     g.drawImage(img,0,0,w,h,Color.WHITE,null);
     ImageIO.write(sc_img,"jpg",f);
     g.dispose();
     return true;
   } catch(Exception e){
      if(DEBUG){
          e.printStackTrace();
      }
      return false;
   }
}


private static void showUsage(){
   System.err.println("java Convert <option> <scale> <source> <target> <rel>" );
   System.err.println("options are");
   System.err.println(" -W : scale determines the fixed width of the target files");
   System.err.println(" -H : scale determines the fixed height of the target files");
   System.err.println(" -M : the maximum size of height and width of the target is scale");
   System.err.println(" -S : scale is given as factor in percent \n");
   System.err.println(" <scale>: a number interpreted depending on <option>\n");
   System.err.println(" <source>: the directory of the source files ");
   System.err.println(" <target>: the target directory (can not exist)");
   System.err.println(" <rel>: name of the created Secondo relation");
  
   System.exit(1);
}

private static File computeTarget(File source, File sourceBase, File targetBase){
    String s = source.getAbsolutePath();
    String s2 = s.substring(sourceBase.getAbsolutePath().length(),s.length());

    return new File(targetBase.getAbsolutePath()+File.separator,s2);

}

private static void start(int mode, double scale, 
                          File sourceBase,
                          File source,
                          File targetBase,
                          File target){
    if(!source.isDirectory()){
        String name = source.getName().toLowerCase();
        if(name.endsWith("jpg") || name.endsWith("jpeg")){
          if(!convert(source,target,scale,mode)){
               System.err.println("error in converting picture " + source);
          } else {
             String date = dformat.format(new Date(source.lastModified()));
             //put the entry into the relation file
             out.println("   ( <text>" + source +"</text--->");
             out.println("      ( \"" + source.getName()+"\"");  // file
             out.println("        \"" + date.toString() + "\""); // date
             out.println("        \"unknown\"");                 // category  
             out.println("        TRUE ");                       // isPortrait
             out.println("        <file>" + target + "</file---> )");
             out.println("   )");
         }
        }
    } else { // source is a directory
       target.mkdir();
       if(!source.canRead()){
         System.err.println("not allowed to read " + source);
       } else {
          File[] content = source.listFiles();
          for(int i= 0; i < content.length;i++){
              start(mode,scale,sourceBase,content[i], 
                    targetBase,computeTarget(content[i],sourceBase,targetBase));

          } 
       }
    }   

}



public static void main(String[] args){
   if(args.length!=5){
      System.err.println("wrong number of arguments " + args.length);
      showUsage();
   }
   int mode = -1;
   if(args[0].toLowerCase().equals("-w")){
     mode = W;
   } else if(args[0].toLowerCase().equals("-h")){
     mode = H;
   } else if(args[0].toLowerCase().equals("-s")){
     mode = S;
   } else if(args[0].toLowerCase().equals("-m")){
     mode = M;
   } else {
      showUsage();
   }
   File source =  new File(args[2]);
   if(!source.exists()){
      System.err.println("Source directory does not exists");
      System.exit(2);
   }  
   File target = new File(args[3]);
   if(target.exists()){
      System.err.println("target directory already exists");
      System.exit(2);
   }
   double scale = 0.0; 
   try{
     scale = Double.parseDouble(args[1]);
   }catch(Exception e){
      System.err.println("scale is not a number");
      System.exit(2);
   }
   if(scale<=0){
      System.err.println("scale is invalid");
      System.exit(0); 
   }

   out.println("  (OBJECT "+ args[4]+ "()");
   out.println("     (rel ( tuple (");
   out.println("       (FilePath filepath)");
   out.println("       (Picture  picture)");
   out.println("     )))");
   out.println("     (");
   start(mode,scale,source,source,target,target);
   out.println(" ))");
}








}
