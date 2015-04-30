package com.secondo.webgui.client.mainview;

import com.google.gwt.event.dom.client.FocusEvent;
import com.google.gwt.event.dom.client.FocusHandler;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.Widget;

/**
 * This class represents the default text box, i.e. text box with default text
 * 
 * @author Irina Russkaya
 */
public class DefaultTextBox extends TextBox implements FocusHandler {
	private String defaultText;

	public DefaultTextBox(String defText) {
		defaultText = defText;
		setText(defaultText);
		addFocusHandler(this);
	}

	/**
	 * Sets the default text in the text box
	 * 
	 * @param defText
	 */
	public void setDefaultText(String defText) {
		defaultText = defText;
	}

	/**
	 * Sets the default text in the text box and disables it
	 * 
	 * @param defText
	 */
	public void setDefaultTextAndDisable(String defText) {
		this.setText(defText);
		this.setEnabled(false);
	}

	/**
	 * Returns the default text
	 * 
	 * @return The default text
	 */
	public String getDefaultText() {
		return defaultText;
	}

	/**
	 * Sets the default text in the text box if focus lost
	 * 
	 * @param sender
	 */
	public void onLostFocus(Widget sender) {
		this.setText(defaultText);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * com.google.gwt.event.dom.client.FocusHandler#onFocus(com.google.gwt.event
	 * .dom.client.FocusEvent)
	 */
	@Override
	public void onFocus(FocusEvent event) {
		this.setText("");
	}
}
