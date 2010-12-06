
package tools.downloadmanager;

import java.net.URL;
import java.net.URLConnection;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.File;



/** Class describing an download. After creation of the download, you have to call the
  * start mathod the start the download.
  **/
public class ActiveDownload extends PlannedDownload implements Runnable{


  /** Creates a new ActiveDownload form the arguments.
    * @param url: the url to be load
    * @param targetFile: the file where the content of the url should be stored
    * @param observer: an observer of this download
    **/
  public ActiveDownload(URL url, File targetFile, DownloadObserver observer){
     super(url,targetFile, observer);
     running = false;
  }

  /** creates an active download from a planned one **/
  public ActiveDownload(PlannedDownload pd){
     super(pd);
     running = false;
  }

  /** starts the download. **/
  public void start(){
     if(!running){
       new Thread(this).start();
     }
  }

  /** the main function of the thread **/
  public void run(){
    running = true;
    InputStream in = null;
    FileOutputStream out=null;
    try{
       URLConnection connection = url.openConnection();
       connection.setConnectTimeout(3000);
       connection.setReadTimeout(3000);
       in = connection.getInputStream();
       targetFile.getParentFile().mkdirs();
       out = new FileOutputStream(targetFile);
       byte[] buffer = new byte[256];
       int size = -1;
       do{
          size = in.read(buffer);
          if(size >=0){
              out.write(buffer,0,size); 
          }
       }while(!canceled && (size>=0));

       in.close();
       out.close();
       if(canceled){
         informListeners(new DownloadEvent(this, DownloadState.CANCEL, null));
          running=false;
         return;
       }
       // download complete
       informListeners(new DownloadEvent(this,DownloadState.DONE, null));
      
    } catch(Exception e){
       if(in!=null){
         try{
           in.close();
         }catch(Exception ein){}
       }
       if(out!=null){
         try{
           out.close();
         }catch(Exception eout){}
       }
       try{
         targetFile.delete();
       } catch(Exception e3){
       }
       informListeners(new DownloadEvent(this,DownloadState.BROKEN, e));
    }
    running=false;
  }

  public String toString(){
      return super.toString() + (running?"running":"waiting");
  }



  private boolean running;

}
