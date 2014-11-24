package com.secondo.webgui.client.mainview;

import com.google.gwt.core.client.GWT;
import com.google.gwt.dom.client.Element;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.FileUpload;
import com.google.gwt.user.client.ui.FormPanel;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.TextBox;

public class FileUploadPanel extends FormPanel {

	private Button uploadButton = new Button("Upload file");
	private FileUpload fileUpload = new FileUpload();
	private Grid gridForUploadComponents;
	private TextBox fileUploadBox;
	private String nameOfUploadedFile;
	private Label lblSelectAgpx;

	/**
	
	 */
	public FileUploadPanel() {
		super();
		this.setAction(GWT.getModuleBaseURL() + "uploadService");
		this.setEncoding(FormPanel.ENCODING_MULTIPART);
		this.setMethod(FormPanel.METHOD_POST);

		gridForUploadComponents = new Grid(2, 3);
		gridForUploadComponents.ensureDebugId("gridForUploadComponents");

		lblSelectAgpx = new Label("Select a .gpx track: ");
		lblSelectAgpx.setStyleName("labelTextInOneLine");

		fileUploadBox = new TextBox();
		fileUploadBox.ensureDebugId("textBoxForUpload");
		fileUploadBox.setWidth("130px");
		fileUploadBox.addClickHandler(new ClickHandler() {

			public void onClick(ClickEvent event) {
				
				fileClick(fileUpload.getElement());
				// fileUploadBox.setText(fileUpload.getFilename());
				// fileUploadBox.setEnabled(false);

			}
		});

		fileUpload.setName("upload");
		fileUpload.ensureDebugId("fileUploadHidden");
		fileUpload.setVisible(false);
		fileUpload.setStyleName("fileUploadInOneLine");

		final FileUploadPanel form = this;
		uploadButton.setWidth("100px");
		uploadButton.addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				System.out.println("File selected " + fileUpload.getFilename());
				form.submit();

			}

		});

		Image help = getHelp();

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
				// secondoSyntaxDialog.getHelpDialogBox().center();
				// secondoSyntaxDialog.getHelpDialogBox().show();

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
