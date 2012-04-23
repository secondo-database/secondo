package progresswatcher;

import javax.swing.JFrame;

/**
 * Some basic methods for windows classes
 * 
 */
public abstract class AbstractWindow {

	protected final JFrame mainframe = new JFrame();

	public JFrame getMainframe() {
		return mainframe;
	}

	/**
	 * Display this windows on screen
	 * 
	 */
	public void show() {
		if(! mainframe.isVisible()) {
			mainframe.pack();
			GuiHelper.locateOnScreen(mainframe);
			mainframe.setVisible(true);
		}
	}
	
	//===========================================
	// Abstract
	//===========================================
	protected abstract WindowType getWindowType();
	protected abstract String getTitle();
}
