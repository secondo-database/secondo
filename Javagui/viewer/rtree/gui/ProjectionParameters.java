package viewer.rtree.gui;

/**
 * ProjectionParameters contains detail information on 
 * the current projection and scaling.
 * 
 * @author Oliver Feuer
 * @author Christian Oevermann
 * @since 08.03.2010
 * @version 1.1
 */
public class ProjectionParameters { 
	
	// offsets
	double offsetX;
	double offsetY;
	// scale factors 
	double scaleFactor;
	double timeScaleFactor;
	// padding
	int padding;
	int extraPaddingTop;
	int extraPaddingBottom;
	// projection dimensions
	int projectionDimX;
	int projectionDimY;
	
	// constructors
	
	/**
	 * Creates a new ProjectionParameters object with default values.
	 */
	public ProjectionParameters() 
	{
		this.offsetX = 0;
		this.offsetY = 0;
		this.scaleFactor = 1;
		this.timeScaleFactor = 1;
		this.padding = 5;
		this.extraPaddingTop = 0;
		this.extraPaddingBottom = 0;
		this.projectionDimX = 0;
		this.projectionDimY = 1;
	}
	
	
	// public members

	
	/**
	 * Gets the offset in x direction.
	 * @return Offset in x direction
	 */
	public double getOffsetX() 
	{
		return this.offsetX;
	}
	
	/**
	 * Sets the offset in x direction.
	 * @param offsetX Offset in x direction
	 */
	public void setOffsetX(double offsetX) 
	{
		this.offsetX = offsetX;
	}
	
	/**
	 * Gets the offset in y direction.
	 * @return Offset in y direction
	 */
	public double getOffsetY() 
	{
		return this.offsetY;
	}
	
	/**
	 * Sets the offset in y direction.
	 * @param offsetY Offset in y direction
	 */
	public void setOffsetY(double offsetY) 
	{
		this.offsetY = offsetY;
	}
	
	/**
	 * Gets the scale factor.
	 * @return Scale factor
	 */
	public double getScaleFactor() 
	{
		return this.scaleFactor;
	}
	
	/**
	 * Sets the scale factor.
	 * @param scaleFactor Scale factor
	 */
	public void setScaleFactor(double scaleFactor) 
	{
		this.scaleFactor = scaleFactor;
	}
	
	/**
	 * Gets the time scale factor.
	 * @return Time scale factor
	 */
	public double getTimeScaleFactor() 
	{
		return this.timeScaleFactor;
	}
	
	/**
	 * Sets the time scale factor.
	 * @param timeScaleFactor Time scale factor
	 */
	public void setTimeScaleFactor(double timeScaleFactor) 
	{
		this.timeScaleFactor = timeScaleFactor;
	}
	
	/**
	 * Gets the padding.
	 * @return Padding
	 */
	public int getPadding() 
	{
		return this.padding;
	}
	
	/**
	 * Sets the padding.
	 * @param padding Padding
	 */
	public void setPadding(int padding) 
	{
		this.padding = padding;
	}
	
	/**
	 * Gets the top extra padding e.g to offset a toolbar.
	 * @return Extra padding top
	 */
	public int getExtraPaddingTop() 
	{
		return this.extraPaddingTop;
	}
	
	/**
	 * Sets the top extra padding e.g to offset a toolbar.
	 * @param extraPaddingTop Extra padding top
	 */
	public void setExtraPaddingTop(int extraPaddingTop) 
	{
		this.extraPaddingTop = extraPaddingTop;
	}
	
	/**
	 * Gets the bottom extra padding e.g to offset a status bar.
	 * @return Extra padding bottom
	 */
	public int getExtraPaddingBottom() 
	{
		return this.extraPaddingBottom;
	}
	
	/**
	 * Sets the top extra padding e.g to offset a status bar.
	 * @param extraPaddingTop Extra padding bottom
	 */
	public void setExtraPaddingBottom(int extraPaddingBottom) 
	{
		this.extraPaddingBottom = extraPaddingBottom;
	}
	
	/**
	 * Gets the projection dimension in x direction.
	 * @return Projection dimension in x direction
	 */
	public int getProjectionDimX() 
	{
		return this.projectionDimX;
	}
	
	/**
	 * Sets the projection dimension in x direction.
	 * @param projectionDimX Projection dimension in x direction
	 */
	public void setProjectionDimX(int projectionDimX) 
	{
		this.projectionDimX = projectionDimX;
	}
	
	/**
	 * Gets the projection dimension in y direction.
	 * @return Projection dimension in y direction
	 */
	public int getProjectionDimY() 
	{
		return this.projectionDimY;
	}
	
	/**
	 * Sets the projection dimension in y direction.
	 * @param projectionDimX Projection dimension in y direction
	 */
	public void setProjectionDimY(int projectionDimY) 
	{
		this.projectionDimY = projectionDimY;
	}
}
