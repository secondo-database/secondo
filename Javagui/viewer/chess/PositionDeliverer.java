package viewer.chess;

/**
 * This interface is used for classes providing a PositionData object to another class
 */
public interface PositionDeliverer
{
	/**
	 * returns the PositionData object held by the PositionDeliverer
	 */
	public PositionData getCurrentPosition();
}
