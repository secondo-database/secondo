package javazoom.jl.decoder;

/**
 * Work in progress.
 */

public interface Control
{
	
	/**
	 * Starts playback of the media presented by this control.
	 */
	public void start();
	
	/**
	 * Stops playback of the media presented by this control.
	 */
	public void stop();
	
	public boolean isPlaying();
	
	public void pause();
		
	
	public boolean isRandomAccess();
	
	/**
	 * Retrieves the current position.
	 */
	public double	getPosition();
	
	/**
	 * 
	 */
	public void		setPosition(double d);
	
	
}
