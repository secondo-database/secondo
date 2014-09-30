

package tools;

import viewer.SecondoViewer;
import java.io.*;
import java.util.*;

public class CheckViewer{

   static Class SV;

   static public boolean isViewer(Class theclass){
        while(theclass!=null){
           if(theclass.equals(SV)){
               return true;
           } else {
               theclass = theclass.getSuperclass();
           }
        }
        return false;
   }


   public static String checkViewer(String name){
      try{
        name = name.replace('/','.');
        name = name.replace('\\','\\');
        Class vc = Class.forName(name);
        if(isViewer(vc)){
            return name;
        } else{
           return null;
        }
      } catch(Exception e){
         return null;
      } catch(NoClassDefFoundError e2){
          return null;
      }
   }

  
  private static void processFile(File f, Queue<File> files, Queue<Integer> depths, int depth){
     //System.err.println("process " + f);
     if(depth>1 && f.isDirectory()){
       return;
     }
     if(f.isDirectory()){
         File[] content = f.listFiles();
         for(int i=0;i<content.length;i++){
            if(content[i].isDirectory() || content[i].getPath().toLowerCase().endsWith(".java")){
                files.add(content[i]);
                depths.add(depth+1);
            }
         }
     } else {
       String name = f.getPath();
       if(name.endsWith(".java")){
          name = name.substring(0,name.length()-5);
          name = checkViewer(name);
          if(name!=null){
            if(name.startsWith("viewer.")){
              name = name.substring(7);
            }
            System.out.println(name);
          }
       }
     }
  }

  private static void processFiles(Queue<File> files, Queue<Integer> depths){
     while(!files.isEmpty()){
        processFile(files.poll(), files, depths, depths.poll().intValue());
     }
  }

  public static void main(String[] args){
     try{
        SV = Class.forName("viewer.SecondoViewer");
     } catch(Exception e){
        System.err.println("class SecondoViewer not found");
     }
     //tools.Environment.DEAD_SILENCE=true;
     File f = new File("viewer/");
     Queue<File> files = new LinkedList<File>();;
     Queue<Integer> depths = new LinkedList<Integer>();;
     files.add(f);
     depths.add(0);
     processFiles(files, depths);
  }


}
