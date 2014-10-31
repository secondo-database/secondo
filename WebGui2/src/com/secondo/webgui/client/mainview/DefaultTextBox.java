package com.secondo.webgui.client.mainview;

import com.google.gwt.event.dom.client.FocusEvent;
import com.google.gwt.event.dom.client.FocusHandler;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.Widget;


public class DefaultTextBox extends TextBox implements FocusHandler {
    private String defaultText;

    public DefaultTextBox(String defText) {
            defaultText = defText;
            setText(defaultText);  
            addFocusHandler(this);
          
    }

    public void setDefaultText(String defText) {
            defaultText = defText;
    }
    
    public void setDefaultTextAndDisable(String defText){
    	this.setText(defText);
    	this.setEnabled(false);
    }

    public String getDefaultText() {
            return defaultText;
    }
   

    public void onLostFocus(Widget sender) {
            this.setText(defaultText);
    }

	@Override
	public void onFocus(FocusEvent event) {		
		this.setText("");		
		
	}
	

} 
