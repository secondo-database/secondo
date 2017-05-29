package viewer.hoese.algebras.continuousupdate;

import gui.ViewerControl;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import java.util.Vector;

/**
 * Starts a thread to run a Query which provieds the online updates
 * 
 * @author secondo
 *
 */
public class AsyncRequestThread extends Thread {

	private ViewerControl vc;
	private OnlineResultsReceiver receiver;
	private String filter;
	private String remotePort;
  private Vector<QueryFinishedListener> qflistener;

	/**
	 * Constructor
	 * 
	 * @param receiver
	 *            Receiver which controls the handling of the received tuples
	 * @param filter
	 *            Filter command to be used in the query
	 * @param vc
	 *            ViewerControl running the query
	 */
	public AsyncRequestThread(OnlineResultsReceiver receiver, String filter,
			String remotePort, ViewerControl vc) {
		super();
    qflistener = new Vector<QueryFinishedListener>();
		this.vc = vc;
		this.receiver = receiver;
		this.filter = filter;
		this.remotePort = remotePort;
	}

  /**
   * Adds a new listener to be informed if the query is finished.
   **/
  public void addQueryFinishedListener(QueryFinishedListener qfl){
     qflistener.add(qfl);
  }


	/**
	 * Method run by the Thread-Framework
	 */
	@Override
	public void run() {
		System.out.println("Starting Receive-Thread");
		ListExpr resultList = ListExpr.theEmptyList();
		IntByReference errorCode = new IntByReference();
		IntByReference errorPos = new IntByReference();
		StringBuffer errorMessage = new StringBuffer();
		vc.execCommand("query receivenlstream(\"localhost\"," + remotePort
				+ ") " + filter + " sendmessages count");
		System.out.println(resultList.toString());
		System.out.println(errorCode.value);
		System.out.println(errorPos.value);
		System.out.println(errorMessage);
    int err = errorCode.value;
    for(int i=0;i<qflistener.size();i++){
       qflistener.get(i).queryFinished(err);
    }
	}

}
