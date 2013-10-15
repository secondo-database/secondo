package com.secondo.webgui.client.databaseview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HTML;

public class DatabaseFooter  extends Composite{
	
	private HTML text = new HTML("");
	
	public DatabaseFooter(){
		
	}

	public HTML getText() {
		return text;
	}

	public void setText(HTML text) {
		this.text = text;
	}

}
