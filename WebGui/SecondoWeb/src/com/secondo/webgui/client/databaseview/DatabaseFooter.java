package com.secondo.webgui.client.databaseview;

import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;

public class DatabaseFooter  extends Composite{
	
	private HorizontalPanel hpanel = new HorizontalPanel();
	private Button logoutButton = new Button("<img src='resources/images/logout.gif' height='20px' width='20px'/>");
	
	public DatabaseFooter(){
		
		int windowWidth = Window.getClientWidth();
		
		hpanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
		hpanel.setHeight("100px");
		hpanel.setWidth(windowWidth-100 +"px");
		//hpanel.add(logoutButton);
		
	}

	public HorizontalPanel getHpanel() {
		return hpanel;
	}

	public void setHpanel(HorizontalPanel hpanel) {
		this.hpanel = hpanel;
	}

	public Button getLogoutButton() {
		return logoutButton;
	}

	public void setLogoutButton(Button logoutButton) {
		this.logoutButton = logoutButton;
	}


}
