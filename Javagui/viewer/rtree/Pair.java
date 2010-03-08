package viewer.rtree;

/**
 * Pair represents a pair of two value
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 17.12.2009
 *
 * @param <F> Type of first element
 * @param <S> Type of second element
 */
public class Pair<F,S> {
	
	public F first;
	public S second;

	/**
	 * Creates a new Pair<F,S> object.
	 * @param first First element
	 * @param second Second element
	 */
	public Pair(F first, S second)
	{
		this.first = first;
		this.second = second;
	}
}
