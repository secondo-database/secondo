package tools.downloadmanager;

import java.net.URL;
import java.io.File;
import java.util.TreeSet;
import java.util.Iterator;


/** Class decsribing a planned download. **/

class PlannedDownload{

 /** Creates a new Download for the specified url. 
   * The content behind the url is stored into the 
   * given file. If the state of the download changed,
   * the observer ob is informed about this. 
   * Note that the state of a pure planned download never canges.
   * After activating the download (creating an ActiveDownload and starting
   * it), the state will change.
   **/
  public PlannedDownload(URL url, File targetFile, DownloadObserver ob){
     this.url =url;
     this.targetFile = targetFile;
     canceled = false;
     observers= new TreeSet<DownloadObserver>();
     if(ob!=null){
        observers.add(ob);
     }
  }

  /** Just a copy constructor **/
  public PlannedDownload(PlannedDownload src){
    this.url = src.url;
    this.targetFile = src.targetFile;
    this.canceled=src.canceled;
    this.observers = new TreeSet<DownloadObserver>();
    this.observers.addAll(src.observers);
  }

  /** adds a new Observer to this download **/

  public boolean addObserver(DownloadObserver observer){
    if(canceled){
       return false;
    }  else if(observer!=null){
       observers.add(observer);
       return  true;
    }
    return false;
  }

  /** Cancels this download. **/
  public void cancel(){
      canceled=true;
  }

  /** Return the url of this download **/
  public URL getURL(){
     return url;
  }

  /** returns the target file **/
  public File getTargetFile(){
     return targetFile;
  }

  /** checks for canceled state **/
  public boolean isCanceled(){
    return canceled;
  }


  /** Informs all resistered linsteners using the specified event. **/
  protected void informListeners(DownloadEvent evt){
    Iterator<DownloadObserver> it = observers.iterator();
    while(it.hasNext()){
       DownloadObserver ob = it.next();
       ob.downloadStateChanged(evt);
    }
  }

  public String toString(){
     String res =  "Download:"+url+"\n file = " + targetFile + "\n no Observers " + observers.size() + "\n[";
    res +=  observers +"]\n Canceled:" + canceled;
    return res;
  }


  protected URL url;
  protected File  targetFile;
  private TreeSet<DownloadObserver> observers;
  protected boolean canceled;
}

