package com.secondo.webgui.client.mainview;

import com.google.gwt.core.client.GWT;
import com.google.gwt.dom.client.Element;
import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.logical.shared.SelectionEvent;
import com.google.gwt.event.logical.shared.SelectionHandler;
import com.google.gwt.event.shared.EventHandler;
import com.google.gwt.user.client.Event;
import com.google.gwt.user.client.EventListener;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.FileUpload;
import com.google.gwt.user.client.ui.FormPanel;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.RootPanel;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.view.client.SelectionChangeEvent;
import com.sun.java.swing.plaf.windows.resources.windows;

public class FileUploadPanel extends FormPanel {

	private Button uploadButton = new Button("Upload file");
	private FileUpload fileUpload = new FileUpload();
	private Grid gridForUploadComponents;
	private TextBox fileUploadBox;
	private String nameOfUploadedFile;
	private Label lblSelectAgpx;
	private HelpDialogForCreateTraj helpDialog= new HelpDialogForCreateTraj();

	/**
	
	 */
	public FileUploadPanel() {
		super();
		this.setAction(GWT.getModuleBaseURL() + "uploadService");
		this.setEncoding(FormPanel.ENCODING_MULTIPART);
		this.setMethod(FormPanel.METHOD_POST);

		lblSelectAgpx = new Label("Select a .gpx track: ");
		lblSelectAgpx.setStyleName("labelTextInOneLine");

		fileUploadBox = new TextBox();
		fileUploadBox.ensureDebugId("textBoxForUpload");
		fileUploadBox.setWidth("130px");
		fileUploadBox.addClickHandler(new ClickHandler() {

			public void onClick(ClickEvent event) {
				
				fileClick(fileUpload.getElement());			
			}
		});

		fileUpload.setName("upload");
		fileUpload.ensureDebugId("fileUploadHidden");
		fileUpload.setVisible(false);
		fileUpload.setStyleName("fileUploadInOneLine");
		

		fileUpload.addChangeHandler(new ChangeHandler() {
			
			@Override
			public void onChange(ChangeEvent event) {				
				 String fileName = fileUpload.getFilename();
				 fileUploadBox.setText(fileName);
			}
		});
		
		final FileUploadPanel form = this;
		
		uploadButton.setWidth("100px");
		
		uploadButton.addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {					
				form.submit();
			}
		});

		Image help = getHelp();

		gridForUploadComponents = new Grid(2, 3);
		gridForUploadComponents.ensureDebugId("gridForUploadComponents");
		gridForUploadComponents.setCellSpacing(3);
		gridForUploadComponents.setWidget(0, 0, lblSelectAgpx);
		gridForUploadComponents.setWidget(0, 1, fileUploadBox);
		gridForUploadComponents.setWidget(0, 2, help);
		gridForUploadComponents.setWidget(1, 0, fileUpload);
		gridForUploadComponents.setWidget(1, 1, uploadButton);
		this.add(gridForUploadComponents);
		
		

	}

	/**
	 * @return
	 */
	private Image getHelp() {
		Image help = new Image("resources/images/question-icon.png");
		help.setWidth("18px");
		help.ensureDebugId("helpButton");
		help.setVisible(true);

		help.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				 helpDialog.getHelpDialogBox().center();
				 helpDialog.getHelpDialogBox().show();

			}
		});
		return help;
	}

	/**
	 * @return
	 */
	public String getFilenameFromFileUpload() {
		return fileUpload.getFilename();
	}

	private static native void fileClick(Element el) /*-{
//Element el=document.getElementById("gwt-debug-fileUploadHidden");
//window.alert("On input"+el.value);
//document.getElementById("gwt-debug-textBoxForUpload").value=el.value; 
//el.addEventListener('input',handleInput,false); 
el.click();
$doc.getElementById("gwt-debug-textBoxForUpload").value=el.value;
//window.alert("On input"+$doc.getElementById("gwt-debug-fileUploadHidden").value);

//el.onselect=function(){window.alert("On input"+el.value);};


//$doc.getElementById("gwt-debug-fileUploadHidden").onselect=function()
//{$window.alert("On input"+$doc.getElementById("gwt-debug-fileUploadHidden").value);
//};


//window.alert("On input"+$doc.getElementById("gwt-debug-fileUploadHidden").value);

// var x = el.form;
//   x.onselect=function()
//{$window.alert("On input"+el.value);}
											

}-*/;

	/**
	 * @return the nameOfUploadedFile
	 */
	public String getNameOfUploadedFile() {
		return nameOfUploadedFile;
	}

	/**
	 * @param nameOfUploadedFile the nameOfUploadedFile to set
	 */
	public void setNameOfUploadedFile(String nameOfUploadedFile) {
		this.nameOfUploadedFile = nameOfUploadedFile;
	}

}
