package javazoom.jl.decoder;

import java.io.InputStream;

/**
 * The <code>JavaLayerHooks</code> class allows developers to change
 * the way the JavaLayer library uses Resources. 
 */

public interface JavaLayerHook
{
	
	/**
	 * Retrieves the named resource. This allows resources to be
	 * obtained without specifying how they are retrieved. 
	 */
	public InputStream getResourceAsStream(String name);	
	
	
}
