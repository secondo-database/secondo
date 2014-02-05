
package tools;

import java.io.File;
import java.net.URL;
import java.net.URLConnection;
import tools.downloadmanager.*;
import java.util.Vector;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.TreeSet;
import java.util.Comparator;
import java.util.Iterator;

import java.util.regex.*;

import javax.swing.text.html.*;


 class RecObserver extends DownloadObserver{

       RecObserver(URL url){
          this.url=url;
       }

      Vector<URL> extractURLs(File f , URL url){
        Vector<URL> res = new Vector<URL>();
        try{ 

          BufferedReader br = new BufferedReader(new FileReader(f));
          HTMLEditorKit editorKit = new HTMLEditorKit();
          HTMLDocument htmlDoc = new HTMLDocument();
          htmlDoc.putProperty("IgnoreCharsetDirective", Boolean.TRUE);

          editorKit.read(br, htmlDoc, 0);


          HTMLDocument.Iterator iter = htmlDoc.getIterator(HTML.Tag.A);


          while (iter.isValid()) {
            try{
               res.add(new URL(url, iter.getAttributes().getAttribute( HTML.Attribute.HREF).toString()  ));;
            } catch(Exception e){
                System.err.println("Error in constructing url 1:" + e);
            }
            iter.next();
          }
          
          iter = htmlDoc.getIterator(HTML.Tag.FRAME);

          while (iter.isValid()) {
            try{
               res.add(new URL(url, iter.getAttributes().getAttribute( HTML.Attribute.SRC).toString()  ));;
            } catch(Exception e){
                System.err.println("Error in constructing url 2:" + e);
            }
            iter.next();
          }


          iter = htmlDoc.getIterator(HTML.Tag.IMG);

          while (iter.isValid()) {
            try{
               res.add(new URL(url, iter.getAttributes().getAttribute( HTML.Attribute.SRC).toString()  ));;
            } catch(Exception e){
                System.err.println("Error in constructing url");
            }
            iter.next();
          }

       } catch(Exception e){
            System.err.println("Problem in extracting links");
       }
       return res;
 

      }


      public void downloadStateChanged(DownloadEvent evt){
          ActiveDownload ad = (ActiveDownload) evt.getSource();

          URL url = ad.getURL();
          if(evt.getState()==DownloadState.DONE){
             File f =  Mirror.dm.getURL(url,null);
             if(f==null){
               System.err.println("Problem file for url " + url + " not found after download");
               return;
             }
             String mime = URLConnection.getFileNameMap().getContentTypeFor(f.getAbsolutePath());
             if(mime.startsWith("text")){
                Vector<URL> urls = extractURLs(f,url);
                Mirror.urls.addAll(urls);
                Mirror.startDownload();
             } else{
             }
          } else {
             System.err.println("download of "+ url + " not successful " + evt.getState());
          }
       }

     private URL url;

};     


/** class for testing the downloadmanager **/

public class Mirror{
  static TreeSet<URL> urls;
  static DownloadManager dm;
  static RecObserver observer;
  static URL basic;


  private static void showUsage(){
     System.out.println("usage java Mirror <URL>");
  }

  static int maxDownloads = 5;

  public static synchronized void startDownload(){
    Iterator<URL> it = urls.iterator();
    while(it.hasNext()){
       URL n  = it.next();
       if(basic.getHost().equals(n.getHost())){
          System.out.println("getURL " + n);
          dm.getURL(n, observer);
       } else {
          System.out.println("skip URL " + n);
       }
    }
    urls.clear();
  }


  public static void main(String[] args){
   
    if(args.length!=1){
      showUsage();
      System.exit(0);
    }

    try{
      dm = new DownloadManager(new File("./myDownloads/"), maxDownloads, false);
      Comparator<URL> cmp = new Comparator<URL>(){
          public int compare(URL o1, URL o2){
             return o1.toString().compareTo(o2.toString());
          }
          public boolean equals(Object o1, Object o2){
             return o1.toString().equals(o2.toString());     
          }
      };
      urls = new TreeSet<URL>(cmp);
      basic = new URL(args[0]);
      urls.add(basic);
      observer = new RecObserver(basic);
      startDownload();
    } catch(Exception e){
      e.printStackTrace();
    }

  }




}


