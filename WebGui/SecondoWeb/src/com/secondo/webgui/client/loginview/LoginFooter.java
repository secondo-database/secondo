package com.secondo.webgui.client.loginview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;

public class LoginFooter  extends Composite{
	
	private Label impressum = new Label("Impressum");
	
	public LoginFooter (){

	}
	
	public Label getImpressum() {
		return impressum;
	}

	public void setImpressum(Label impressum) {
		this.impressum = impressum;
	}
	

}
