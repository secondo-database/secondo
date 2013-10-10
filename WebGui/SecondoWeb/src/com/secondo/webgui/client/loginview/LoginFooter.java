package com.secondo.webgui.client.loginview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;

public class LoginFooter  extends Composite{
	
	private HorizontalPanel hPanel = new HorizontalPanel();
	private Label impressum = new Label("Impressum");
	
	public LoginFooter (){
		
		hPanel.setSize("900px", "80px");
	    hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_CENTER);
	    hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
	    hPanel.add(impressum);
	    hPanel.getElement().setId("hpanel");

	    
		
	}
	
	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}
	

}
