package tools.downloadmanager;


/** Interface for observing the progress of an download **/




public interface DownloadObserver{

/** This method is called if the state of a download changed. **/
  public void downloadStateChanged(DownloadEvent evt);
}
