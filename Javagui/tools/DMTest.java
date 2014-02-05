
package tools;

import java.io.File;
import java.net.URL;
import tools.downloadmanager.*;
import java.util.Vector;
import java.io.BufferedReader;
import java.io.FileReader;


 class PrintObserver extends DownloadObserver{


       public void downloadStateChanged(DownloadEvent evt){
          System.out.println(evt);
          Exception e = evt.getException();
          if(e!=null){
             e.printStackTrace();
           }
       }

};     


/** class for testing the downloadmanager **/

public class DMTest{

  private static void showUsage(){
    System.out.println("java DMTest url1 url2 url3");
    System.out.println("\twill download all urls given as argument to ./DM_Downloads");
    System.out.println("java DMTest -f <file>");
    System.out.println("\tdownloads all urls from a given file. The file has to contain one url in each line.");
  }

  private static Vector<URL> readUrls(File f){
       if(!f.exists()){
         System.out.println("File does not exist");
         return null;
       }
       if(!f.isFile()){
          System.out.println("File must specify a regular file");
          return null;
       }
       if(!f.canRead()){
          System.out.println("nit allowed to read the file");
          return null;
       }
       BufferedReader in = null;
       Vector<URL> urls = new Vector<URL>();
       try{
           in = new BufferedReader(new FileReader(f));
           while(in.ready()){
             String line = in.readLine();
             try{
                 URL url = new URL(line);
                 urls.add(url);
             } catch(Exception e){
                 System.out.println("problem with " + line);
             }
           }
           in.close();
       } catch(Exception e){
           System.out.println("problem in analysing file");
           System.exit(0);
       }
       return urls;       
  }

  private static Vector<URL> readUrls(String[] urls){
    Vector<URL> res = new Vector<URL>();
    for(int i=0;i<urls.length;i++){
      try{
        URL url = new URL(urls[i]);
        res.add(url);
      } catch(Exception e){
        System.out.println("problem with url " + urls[i]);
      }
    }
    return res;
  }


  public static void main(String[] args){

     if(args.length==0){
       showUsage();
       System.exit(0);
     }

     Vector<URL> urls;
     if(args[0]=="-f"){
       if(args.length!=2){
         showUsage();
         System.exit(0);
       }
       File f = new File(args[1]);
       urls= readUrls(f);
       if(urls==null){
          System.exit(0);
       }
     } else {
        urls = readUrls(args);
     } 


     File f = new File(".");
     f = new File(f,"myDownloads"+File.separator);
     DownloadManager dm = null;
     try{
         dm = new DownloadManager(f,maxDownloads, false);
     } catch (Exception e){
         System.out.println("Cannot create downloadManager");
         System.out.println("File = " + f);
         System.out.println("MaxDownloads = " + maxDownloads);
         System.out.println(e);
         e.printStackTrace();
         System.exit(0);
     }
     DownloadObserver observer = new PrintObserver();
     for(int i=0;i<urls.size();i++){
          dm.getURL(urls.get(i),observer);
     }
  }

  static int maxDownloads = 5;


}


