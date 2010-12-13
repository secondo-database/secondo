package tools.downloadmanager;

/** Class describing an state change of a download **/

public class DownloadEvent{

  /** Creates a new DownloadEvent from the arguments. **/
  public DownloadEvent( ActiveDownload source, DownloadState state, Exception ex){
     this.source = source;
     this.state = state;
     this.ex = ex;
  }

  /** Returns the source of this event, normally the assigned Download **/
  public ActiveDownload getSource(){
     return source;
  }

  /** Returns the new state of the download **/
  public DownloadState getState(){
     return state;
  }

  /** Returns an exeption occured during the download. If there is no
    * Exception, null is returned. 
    **/
  public Exception getException(){
    return ex;
  } 


  public String toString(){
    return "Source = " + source +"\n"+
           "State = " + state +"\n"+
           "Exception = " + ex;
  }

  private ActiveDownload source;
  private DownloadState state;
  private Exception ex;
}

