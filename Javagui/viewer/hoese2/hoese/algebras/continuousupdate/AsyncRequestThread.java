package viewer.hoese2.algebras.continuousupdate;

import gui.ViewerControl;
import sj.lang.IntByReference;
import sj.lang.ListExpr;

/**
 * Starts a thread to run a Query which provieds the online updates
 * @author secondo
 *
 */
public class AsyncRequestThread extends Thread{

	private ViewerControl vc;
	private OnlineResultsReceiver receiver;
	private String filter;
	
	/**
	 * Constructor
	 * @param receiver Receiver which controls the handling of the received tuples
	 * @param filter Filter command to be used in the query
	 * @param vc ViewerControl running the query
	 */
	public AsyncRequestThread(OnlineResultsReceiver receiver, String filter, ViewerControl vc){
		super();
		this.vc = vc;
		this.receiver = receiver;
		this.filter = filter;
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
		vc.execCommand("query receivenlstream(\"localhost\",9000) " + filter + " sendmessages count");
		System.out.println(resultList.toString());
		System.out.println(errorCode.value);
		System.out.println(errorPos.value);
		System.out.println(errorMessage);
		receiver.disable();
	}

	
}
