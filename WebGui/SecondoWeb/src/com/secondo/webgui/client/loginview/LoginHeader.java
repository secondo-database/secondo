package com.secondo.webgui.client.loginview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HorizontalPanel;

public class LoginHeader  extends Composite{
	
	private HorizontalPanel hPanel = new HorizontalPanel();
	
	public LoginHeader (){
		
		hPanel.setSize("900px", "80px");
		
	}
	
	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}
	

}
