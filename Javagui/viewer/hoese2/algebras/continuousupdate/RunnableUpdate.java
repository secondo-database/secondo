package viewer.hoese2.algebras.continuousupdate;

import java.text.SimpleDateFormat;
import java.util.Date;

import viewer.HoeseViewer;
import viewer.hoese.DsplGraph;
import viewer.hoese.QueryResult;

/**
 * Runnale to be used to update the GUI
 * 
 * @author secondo
 *
 */
public class RunnableUpdate implements Runnable {

	private QueryResult currentQR;
	private QueryResult tmpQR;
	private HoeseViewer hoese;
	private Integer tupleLimit;

	/**
	 * Default Constrcutor cannot be used
	 */
	private RunnableUpdate() {
	}

	/**
	 * Constructor for a new Update
	 * 
	 * @param current
	 *            The current QueryResult
	 * @param tmp
	 *            The temporary QueryResult containing the new Tuples
	 * @param hoese
	 *            The Hoeseviewer
	 * @param The
	 *            maximum size of the ListModel after the update
	 */
	public RunnableUpdate(QueryResult current, QueryResult tmp,
			HoeseViewer hoese, Integer tupleLimit) {
		this.hoese = hoese;
		this.currentQR = current;
		this.tmpQR = tmp;
		this.tupleLimit = tupleLimit;
	}

	/**
	 * Primary method used by the Threadingframework
	 */
	@Override
	public void run() {

		int prevSize = currentQR.getGraphObjects().size();
		currentQR.addEntries(tmpQR.getModel());
		for (int i = prevSize; i < currentQR.getGraphObjects().size(); i++) {
			currentQR.getResultLayer().addGO(-1,
					(DsplGraph) currentQR.getGraphObjects().get(i));
		}

		currentQR.reduceModels(tupleLimit, hoese);
		currentQR.computeTimeBounds();

		hoese.updateViewParameter(false, false);

	}
}
