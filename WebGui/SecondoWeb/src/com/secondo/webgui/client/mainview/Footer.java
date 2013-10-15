package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HTML;

public class Footer  extends Composite{
	
	private HTML text = new HTML("");
	
	public Footer(){
		
	}

	public HTML getText() {
		return text;
	}

	public void setText(HTML text) {
		this.text = text;
	}

}
