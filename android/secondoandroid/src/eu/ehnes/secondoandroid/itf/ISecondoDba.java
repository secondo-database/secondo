/*
 * 
 */
package eu.ehnes.secondoandroid.itf;

// TODO: Auto-generated Javadoc
/**
 * The Interface ISecondoDba.
 */
public interface ISecondoDba {
	
	/**
	 * Initialize sync.
	 *
	 * @param configPath the config path
	 * @return true, if successful
	 */
	public boolean initializeSync(String configPath);
	
	/**
	 * Query sync.
	 *
	 * @param queryString the query string
	 * @return the object
	 */
	public Object querySync(String queryString);
	
	/**
	 * Error message sync.
	 *
	 * @return the string
	 */
	public String errorMessageSync();
	
	/**
	 * Shutdown sync.
	 */
	public void shutdownSync();
	
	/**
	 * Query a sync.
	 *
	 * @param queryString the query string
	 * @param function the function
	 */
	void queryASync(String queryString,ISecondoDbaCallback function);

	/**
	 * call initialize function asynchronous.
	 *
	 * @param configPath the config path
	 * @param callBackInterface the call back interface
	 */
	void initializeASync(String configPath,	ISecondoDbaCallback callBackInterface);
	
}
