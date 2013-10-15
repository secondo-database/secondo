package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.VerticalPanel;

public class Commandpanel extends Composite {

	private VerticalPanel commandPanel = new VerticalPanel();
	private MenuBarCommandPanel menubarCP = new MenuBarCommandPanel();
	private TextArea textArea = new TextArea();

	public Commandpanel() {

		textArea.setVisibleLines(5);
		textArea.setHeight("200px");
		
		textArea.getElement().setClassName("commandpanel");
		
		commandPanel.add(menubarCP.gethPanel());
		commandPanel.add(textArea);
		commandPanel.getElement().setId("command");
		commandPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);

	}
	
	public void resize(int width){
		textArea.setWidth(width-70 + "px");
	}

	public VerticalPanel getCommandPanel() {
		return commandPanel;
	}

	public void setCommandPanel(VerticalPanel commandPanel) {
		this.commandPanel = commandPanel;
	}


	public TextArea getTextArea() {
		return textArea;
	}

	public void setTextArea(TextArea textArea) {
		this.textArea = textArea;
	}

	public MenuBarCommandPanel getMenubarCP() {
		return menubarCP;
	}

	public void setMenubarCP(MenuBarCommandPanel menubarCP) {
		this.menubarCP = menubarCP;
	}	

}
