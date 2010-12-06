
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

import java.util.regex.*;


 class RecObserver extends DownloadObserver{

       RecObserver(URL url){
          this.url=url;
       }



      String extractHRefFromAnchor(String a){

         Pattern hrefPat1 = Pattern.compile("href\\s*=\\s*\"[^\"]*\"",Pattern.CASE_INSENSITIVE );  // with quotes

         Matcher m1 = hrefPat1.matcher(a);
         if(m1.find(0)){
           String g = m1.group();
           int index1 = g.indexOf("\"");
           int index2 = g.indexOf("\"",index1);
           g = g.substring(index1+1);
           g = g.substring(0,g.length()-1);
           return g;
         }
         
        Pattern hrefPat2 = Pattern.compile("href\\s*=\\s*[^\\s]*\\s",Pattern.CASE_INSENSITIVE); // without quotes
         Matcher m2 = hrefPat2.matcher(a);
         if(m2.find(0)){
             String g = m2.group();
             g = g.substring(g.indexOf("=")+1,g.length());
             g = g.trim();
             Pattern whiteSpacePat = Pattern.compile("\\s");
             Matcher wm = whiteSpacePat.matcher(g);
             if(wm.find()){
                g = g.substring(0,wm.start()-1);
             }
             return g;
         }
         return null;
      }


      Vector<URL> extractURLs(File f, URL url){
         Vector<URL> urls = new Vector<URL>();
         try{
            BufferedReader in = new BufferedReader(new FileReader(f));
            StringBuffer sb = new StringBuffer();
            while(in.ready()){
               sb.append(in.readLine());
            }
            in.close();  
            String fileContent = sb.toString();
            // extract links within the string
            Pattern p = Pattern.compile("<a .*>",Pattern.CASE_INSENSITIVE);   
            Matcher m = p.matcher(sb);
            int pos = 0;
            while(m.find(pos)){
                int start = m.start();
                String group = m.group();
                int index = group.indexOf(">");
                group = group.substring(0,index+1);
                String link = extractHRefFromAnchor(group);
                try{
                      urls.add(new URL(url,link));
                } catch(Exception e){
                   System.err.println("cannot create url from " + link);
                }
                pos = m.start() + group.length();
            }
          } catch(Exception e){
             System.err.println("problem in extracting links");
             e.printStackTrace();
          }
          return urls;
      }

      public void downloadStateChanged(DownloadEvent evt){
          ActiveDownload ad = (ActiveDownload) evt.getSource();

          URL url = ad.getURL();
          System.out.println("DownloadState changed " + url );
          if(evt.getState()==DownloadState.DONE){
             File f =  Mirror.dm.getURL(url,null);
             if(f==null){
               System.err.println("Problem file for url " + url + " not found after download");
               return;
             }
             String mime = URLConnection.getFileNameMap().getContentTypeFor(f.getAbsolutePath());
             if(mime.startsWith("text")){
                Vector<URL> urls = extractURLs(f,url);
                System.out.println("found URLS = " + urls);
                Mirror.urls.addAll(urls);
                Mirror.startDownload();
             } else{
                System.out.println("skip file "+ f +" because mime type is" + mime);
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

  public static void startDownload(){
    while(!urls.isEmpty()){
       URL first = urls.first();
       urls.remove(first); 
       if(basic.getHost().equals(first.getHost())){
          System.out.println("getURL " + first);
          dm.getURL(first, observer);
       } else {
          System.out.println("skip URL " + first);
       }
    }
  }


  public static void main(String[] args){
   
    if(args.length!=1){
      showUsage();
      System.exit(0);
    }

    try{
      dm = new DownloadManager(new File("./myDownloads/"), maxDownloads);
      Comparator cmp = new Comparator(){
          public int compare(Object o1, Object o2){
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


