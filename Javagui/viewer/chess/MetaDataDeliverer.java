package viewer.chess;
import java.util.HashMap;

/**
 * Classes that implement this interface provide functionality for all classes that need the meta data of a chessgame
 */
public interface MetaDataDeliverer
{
	/**
	 * returns a HashMap of the meta tags of a chessgame. The key of the tag is the key of the hashmap while the value of the hashmap is the value of the tag according to the key 
	 */
	public HashMap getMetaVals();
}
