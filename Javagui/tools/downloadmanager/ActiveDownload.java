
package tools.downloadmanager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;



/** Class describing an download. After creation of the download, you have to call the
  * start mathod the start the download.
  **/
public class ActiveDownload extends PlannedDownload implements Runnable{


  /** Creates a new ActiveDownload form the arguments.
    * @param url: the url to be load
    * @param targetFile: the file where the content of the url should be stored
    * @param observer: an observer of this download
    **/
  public ActiveDownload(URL url, File targetFile, DownloadObserver observer, int connectTimeout, int readTimeout){
     super(url,targetFile, observer);
     running = false;
     this.connectTimeout  = Math.max(0,connectTimeout);
     this.readTimeout  = Math.max(0,readTimeout);
  }

  /** creates an active download from a planned one **/
  public ActiveDownload(PlannedDownload pd, int connectTimeout, int readTimeout){
     super(pd);
     running = false;
     this.connectTimeout  = Math.max(0,connectTimeout);
     this.readTimeout  = Math.max(0,readTimeout);
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
       connection.setConnectTimeout(connectTimeout);
       connection.setReadTimeout(readTimeout);
       in = connection.getInputStream();
       targetFile.getParentFile().mkdirs();
			boolean first = true;
       byte[] buffer = new byte[256];
       int size = -1;
       do{
          size = in.read(buffer);
          if(size >=0){
					if (first) {
						first = false;
						out = new FileOutputStream(targetFile);
					}
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
  private int readTimeout;
  private int connectTimeout;

}
