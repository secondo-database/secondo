package de.fernunihagen.dna.secondocore.itf;

// TODO: Auto-generated Javadoc
/**
 * The Interface ISecondoDbaCallback.
 */
public interface ISecondoDbaCallback  {
	
	/**
	 * Query call back.
	 *
	 * @param result the result
	 */
	public void queryCallBack(Object result);
	
	/**
	 * Initialize call back.
	 *
	 * @param result the result
	 */
	public void initializeCallBack(boolean result);
	
}
