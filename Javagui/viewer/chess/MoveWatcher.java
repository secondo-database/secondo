package viewer.chess;
/**
 * Interface which provides functionality to give an implementing class a GameData object
 */
public interface MoveWatcher
{
	/**
	 * set the GameData object in the implementing class
	 */
	public void setCurrentGame(GameData g);
}
