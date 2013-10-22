package com.secondo.webgui.client.mainview;

public interface View {
	
	public void resizeWithCP(int width, int height);
	
	public void resizeWithTextPanel(int width, int height);
	
	public void resizeWithTextAndCP(int width, int height);
	
	public void resizeToFullScreen(int width, int height);


}
