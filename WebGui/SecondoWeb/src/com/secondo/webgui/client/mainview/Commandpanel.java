package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.TextArea;

public class Commandpanel extends Composite {

	private HorizontalPanel hPanel = new HorizontalPanel();
	private TextArea textArea = new TextArea();

	public Commandpanel() {

		textArea.setVisibleLines(3);
		textArea.setWidth("900px");
		textArea.getElement().setClassName("commandpanel");
		hPanel.add(textArea);

	}

	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}

	public TextArea getTextArea() {
		return textArea;
	}

	public void setTextArea(TextArea textArea) {
		this.textArea = textArea;
	}	

}
