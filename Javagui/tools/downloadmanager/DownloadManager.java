

package tools.downloadmanager;

import java.io.File;
import java.net.URL;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;

public class DownloadManager extends  DownloadObserver{


  /** Creates a new DownloadManager. 
   **/
  public DownloadManager(File tmpDirectory, int maxDownloads, boolean lastImportant) throws InvalidArgumentException{
   //  if(!tmpDirectory.isDirectory()){
   //      throw new InvalidArgumentException("tmpdirectory must be an directiry");
   //  }
     if(!tmpDirectory.exists()){
         tmpDirectory.mkdirs();
     }
     if(!tmpDirectory.canRead() || !tmpDirectory.canWrite()){
         throw new InvalidArgumentException("access to "+tmpDirectory.getName()+" not permitted");
     }
     // successful tmpdir can be used
     rootDir =tmpDirectory;
     this.maxDownloads = Math.max(1,maxDownloads);

     plannedDownloads = new HashMap<URL, PlannedDownload>();
     activeDownloads = new HashMap<URL, ActiveDownload>();
     plannedQueue = new LinkedList<URL>();
     this.lastImportant = lastImportant;
  }


  /** retrieves a file.
   *  If this url was already downloaded, the File containing the content of
   * the url is returned. Otherwise the return value is null. If the download state
   * changes (url does not exists, download complete etc.), the given observer is
   * informed about this change. To really get the file, just call getFile again.
   * Note: The file can become invalid if the clearCache or removeUrl function
   * is called.
   */
  public  File getURL(URL url, DownloadObserver ob){
    synchronized(syncObj){
			if (activeDownloads.containsKey(url)) {
				ActiveDownload ad = activeDownloads.get(url);
				ad.addObserver(ob);
				return null;
			}
			if (plannedDownloads.containsKey(url)) {
				PlannedDownload pd = plannedDownloads.get(url);
				pd.addObserver(ob);
				return null;
			}

			File f = computeFile(url);
			if (f.exists()) {
        if(ob!=null){
            ob.fileExists(url);
        }
				return f;
			}

		// insert observer is there is a download for that url
			if (insertObserver(url, ob)) {
				return null;
			}

		if ((plannedDownloads.size() > 0)
					|| (activeDownloads.size() >= maxDownloads)) {
				// all download slots are used. So, the download is just planned
				PlannedDownload d = new PlannedDownload(url, f, this);
				d.addObserver(ob);
				plannedDownloads.put(url, d);
				plannedQueue.offer(url);
				return null;
			}
			// the is a free download slot, create a new download for the url
			ActiveDownload dl = new ActiveDownload(url, f, this,
					connectTimeout, readTimeout);
			dl.addObserver(ob);
			activeDownloads.put(url, dl);
			dl.start();
    }
    return null;
  }


  /** Updates an downloaded url.
    * The file is actually updated if the file is not present,
    * force is set to true, the file content behind the urls does
    * not provide time information, or the content on the web server
    * is newer than the file.
    * If the file is up to date (or cannot be downloaded), the observer is
    * informed about this. To get the updated file, call the getFile method.
    * @param url: url to be updated
    * @param force: flag to force an update even is the local file seems to be up to date
    * @param ob: observer which should be informed about the progress of the download
    **/
  public void updateURL(URL url, boolean force, DownloadObserver ob){
    synchronized(syncObj){
			if(insertObserver(url,ob)){
				 return;
			}
			File f = computeFile(url);
			// file not present, just start the normal download
			if(!f.exists()){
				getURL(url,ob);
				return;
			}
			// file is present // do some more complicated stuff File tmpFile =
			// computeTmpFile(url);
				// idea start a new download into a temporarly file
				// assign another observer to that download
				// if the download is finished successful, move the new file to the 
				// file to be updated 
				// if the downloads breaks down, remove the temp file 
				// in each case inform ob about the new state.
				//TODO:...
   }
 }



  
  /** Removes a cached file, or cancels a download for a URL.
    * If the download is canceled, the observer is informed about that.
    **/
  public boolean removeURL(URL url){
    synchronized(syncObj){
			 PlannedDownload pd = plannedDownloads.get(url);
			 if(pd!=null){
					pd.cancel();
					plannedDownloads.remove(url);
					return true;
			 }
			 ActiveDownload ad = activeDownloads.get(url);
			 if(ad!=null){
					ad.cancel();
					activeDownloads.remove(url);
					return true; 
			 }
			 File f = computeFile(url);
			 if(f.exists()){
					f.delete();
					return true;
			 }
			 // url was not managed
     }
     return  false; 
  }


  /** cancels all active and inactive downloads informing the related observers **/
  public void cancelDownloads(){
    synchronized(syncObj){
			 Collection<PlannedDownload> cpd = plannedDownloads.values();
			 plannedDownloads.clear();     
			 Iterator<PlannedDownload> itpd = cpd.iterator();
			 while(itpd.hasNext()){
					PlannedDownload pd = itpd.next();
					itpd.remove();
					pd.cancel(); 
			 }

			 Collection<ActiveDownload> cad = activeDownloads.values();
			 activeDownloads.clear();
			 Iterator<ActiveDownload> itad = cad.iterator();
			 while(itad.hasNext()){
					ActiveDownload ad = itad.next();
					itad.remove();
					ad.cancel(); 
			 }
   }
  }

  /** cancels all downloads and removes all files located below the tmpdirectory **/
  public void clearChache() {
    synchronized(syncObj){
			 cancelDownloads();
			 deleteDirContent(rootDir);
   }
  }


  /** reaction to finished downloads **/
  public void downloadStateChanged(DownloadEvent evt){
   synchronized(syncObj){
      ActiveDownload ad = (ActiveDownload) evt.getSource();
      activeDownloads.remove(ad.getURL());
      while(plannedQueue.size()>0 && activeDownloads.size() < maxDownloads){
         URL url = lastImportant?plannedQueue.removeLast(): plannedQueue.poll();
         PlannedDownload pd = plannedDownloads.get(url);
         if(pd!=null){
            plannedDownloads.remove(url);
            activate(pd);
            return;
         }
      }
   }
  }

  /** Sets the timeout for connecting with the server in milliseconds. A value of zero
    * or less will describe an infinite timeout.
    **/
  public void setConnectTimeout(int connectTimeout){
      this.connectTimeout = Math.max(0, connectTimeout);
  }

  /** Sets the timeout for a single read action from  the server in milliseconds. A value of zero
    * or less will describe an infinite timeout.
    **/
  public void setReadTimeout(int readTimeout){
      this.readTimeout = Math.max(0, readTimeout);
  }

 /** returns the current value of the connection time out **/
  public int getConnectTimeout(){
    return connectTimeout;
  }

 /** returns the current value of timeout  for reading. **/
  public int getReadTimeout(){
    return readTimeout;
  }


  /** activates a planned download **/
  private void activate(PlannedDownload pd){
     ActiveDownload ad = new ActiveDownload(pd, connectTimeout, readTimeout);
     activeDownloads.put(ad.getURL(), ad);
     ad.start();
  }



  /* Compute the file from the rootDirectiry and the URL */
  private File computeFile(URL url, File rootDir){
    try{
      File p = new File(rootDir,url.getProtocol());
      File ph = new File(p,url.getHost());
      int port = url.getPort();
      if(port<0){
          port = url.getDefaultPort();
      }   
      File php = new File(ph,""+port);
      File phpf = new File(php,url.getFile());
      return phpf.getCanonicalFile();
     } catch(Exception e){ 
       e.printStackTrace();
       return new File("Error");
    }   
  }

  private File computeFile(URL url){
     return computeFile(url,rootDir);
  }

  private File computeTmpFile(URL url){
      return computeFile(url, new File(rootDir,"temp"+File.separator));
  }


  /** Inserts a new Observer into existing download for the specified url.
    * If there is no download for this url, the result will be false.
    * Otherwise true.
    **/
  private boolean insertObserver(URL url, DownloadObserver ob){
    PlannedDownload pd = plannedDownloads.get(url);
    if(pd!=null){
       pd.addObserver(ob);
       return true;
    }
    ActiveDownload ad = activeDownloads.get(url);
    if(ad!=null){
       ad.addObserver(ob);
       return true;
    }
    return false;
  }

  /** delete a directory and all contained stuff **/
  private boolean deleteDirectory(File root){
     if(deleteDirContent(root)){
           return root.delete();
     } else {
         return false;
     }
  }

  /** deletes the content of a directory **/
  private boolean deleteDirContent(File root){
    if(!root.isDirectory()){
       return false;
    }
    try{
      File[] content = root.listFiles();
      for(int i=0;i!=content.length;i++){
        if(    !content[i].getName().equals(".") 
            && !content[i].getName().equals("..")){
           if(content[i].isDirectory()){
              if(!deleteDirContent(content[i])){
                return false;
              }
           }
           content[i].delete();
        }
      }
    } catch(Exception e){
        e.printStackTrace();
        return false;
    }
    return true;
  }


  /** returns the number of pending downloads ***/
  public int numOfPendingDownloads(){
      return activeDownloads.size() + plannedDownloads.size();
  }

  private File rootDir;
  private int maxDownloads;
  private HashMap<URL, ActiveDownload> activeDownloads;
  private HashMap<URL, PlannedDownload> plannedDownloads;
  private LinkedList<URL> plannedQueue;
  private final Object syncObj = new Object();
  private int connectTimeout = 3000;
  private int readTimeout = 3000;
  private boolean lastImportant;  
}
