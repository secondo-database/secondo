package com.secondo.webgui.client.loginview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HorizontalPanel;

public class LoginHeader  extends Composite{
	
	private FlowPanel hPanel = new FlowPanel();
	
	public LoginHeader (){
		
	}
	
	public FlowPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(FlowPanel hPanel) {
		this.hPanel = hPanel;
	}
	

}
