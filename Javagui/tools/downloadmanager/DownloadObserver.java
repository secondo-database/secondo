package tools.downloadmanager;


/** Interface for observing the progress of an download **/




public abstract class  DownloadObserver implements Comparable{
  public DownloadObserver(){
      id = maxID++; 
  }

/** This method is called if the state of a download changed. **/
  public abstract void downloadStateChanged(DownloadEvent evt);


  public void fileExists(java.net.URL url){}

  public int compareTo(Object obj){
     DownloadObserver o = (DownloadObserver) obj;
     if(o.id==id) return 0;
     if(o.id<id) return 1;
     return -1;
  }

  public String toString(){
    return super.toString()+" id = " + id;
  }

  static int maxID=0;
  private int id;


}
