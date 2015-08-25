package viewer.hoese.algebras.continuousupdate;

import gui.MainWindow;

import java.awt.Color;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.swing.JButton;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import sj.lang.MessageListener;
import viewer.HoeseViewer;
import viewer.hoese.HoeseOnlineDialog;
import viewer.hoese.QueryResult;

/**
 * Enables the online receiving of Tuples from the Secondo Server.
 *
 */
public class OnlineResultsReceiver implements MessageListener {

	private boolean enabled;
	private QueryResult qr;
	private Thread cmdThread;
	private HoeseViewer hoese;
	private JButton button;
	private ListExpr cache;
	private ListExpr lastCachedItem;
	private String remotePort;
	private Integer tupleLimit;
	private Integer updateRate;
	private long lastUpdate;

	/**
	 * Constructor
	 * 
	 * @param hoese
	 *            HoeseViewer to which this Receiver belongs
	 * @param filter
	 *            Filter-Command to be used in the Query
	 * @param modelLimit
	 *            Maximum Size of the ListModel (lower size, higher performance)
	 * @param button
	 *            The Button displaying the current state
	 * @param qr
	 *            The QueryResult which sould be used
	 */
	public OnlineResultsReceiver(HoeseViewer hoese, String filter,
			Integer tupleLimit, Integer updateRate, String remotePort,
			JButton button, final QueryResult qr) {
		this.qr = qr;
		this.hoese = hoese;
		this.button = button;
		this.tupleLimit = tupleLimit;
		this.updateRate = updateRate;
		this.remotePort = remotePort;

		// Initialize the cache
		resetCache();

		// Enable the component
		enable();

		// Register this instance to receive incoming tuples
		hoese.getViewerControl().addMessageListener(this);
		cmdThread = new AsyncRequestThread(this, filter, remotePort,
				hoese.getViewerControl());
		cmdThread.start();
	}

	/**
	 * Resets the Cache and the lastCachedItem to null
	 */
	private void resetCache() {
		cache = null;
		lastCachedItem = null;
		this.lastUpdate = System.currentTimeMillis();
	}

	// Add the received tuple to the cache
	private void addToCache(ListExpr item) {

		if (lastCachedItem == null) {
			cache = ListExpr.oneElemList(item);
			lastCachedItem = cache;
		} else {
			lastCachedItem = ListExpr.append(lastCachedItem, item);
		}
	}

	/**
	 * Process the received ListExpr
	 */
	@Override
	public void processMessage(ListExpr message) {
		if (enabled) {

			addToCache(message);

			// Update the GUI if cache-timeout was reached
			if ((System.currentTimeMillis() - lastUpdate) > updateRate) {
				updateGUI();
			}
		}
	}

	/**
	 * Update the GUI from cache
	 */
	private void updateGUI() {
		long start = System.currentTimeMillis();
		qr.addTuples(hoese, cache, tupleLimit);
		Date d = new Date();
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		resetCache();
	}

	/**
	 * Returns the current state of this instance Would return false, if the
	 * connection to Secondo was lost Would return false, if the instance was
	 * disabled
	 * 
	 * @return
	 */
	public boolean isEnabled() {
		return enabled;
	}

	/**
	 * Disable this instance
	 */
	public void disable() {

		// Change visual status
		button.setBackground(new JButton().getBackground());
		MainWindow.enableCommandPanel();
		enabled = false;

		// Reconnect
		((ESInterface) MainWindow.getUpdateInterface()).terminate();

		cmdThread.interrupt();
		try {
			cmdThread.join();
		} catch (InterruptedException e) {
		}

		while (!((ESInterface) MainWindow.getUpdateInterface()).isConnected()) {
			((ESInterface) MainWindow.getUpdateInterface()).connect();
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
			}
		}

		// Open the previous database
		ListExpr l = new ListExpr();
		IntByReference i1 = new IntByReference();
		IntByReference i2 = new IntByReference();
		StringBuffer sb = new StringBuffer();

		((ESInterface) MainWindow.getUpdateInterface()).secondo(
				"open database " + HoeseOnlineDialog.lastValues.database, l,
				i1, i2, sb);
	}

	/**
	 * Enable this instance
	 */
	public void enable() {
		button.setBackground(Color.green);
		MainWindow.disableCommandPanel();
		enabled = true;
	}
}
