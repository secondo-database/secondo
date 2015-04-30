
package com.secondo.webgui.client.mainview;

/**
 * This interface must be implemented to create a view that is resizable on all
 * screens
 * 
 * @author Irina Russkaya
 * 
 **/
public interface View {

	public void resizeWithCP(int width, int height);

	public void resizeWithTextPanel(int width, int height);

	public void resizeWithTextAndCP(int width, int height);

	public void resizeToFullScreen(int width, int height);

	public void updateView();

	public void resetData();
}
