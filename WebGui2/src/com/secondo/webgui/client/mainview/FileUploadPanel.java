package com.secondo.webgui.client.mainview;

import com.google.gwt.core.client.GWT;
import com.google.gwt.dom.client.Element;
import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.FileUpload;
import com.google.gwt.user.client.ui.FormPanel;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.TextBox;

/**
 * This class represents a customized form panel to support a gpx file upload
 * 
 * @author Irina Russkaya
 *
 */
public class FileUploadPanel extends FormPanel {

	private Button uploadButton = new Button("Upload file");
	private FileUpload fileUpload = new FileUpload();
	private Grid gridForUploadComponents;
	private TextBox fileUploadBox;
	private String nameOfUploadedFile;
	private Label lblSelectAgpx;
	private HelpDialog helpDialog;

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
				// fileUploadBox.setText(fileUpload.getFilename());
				// fileUploadBox.setEnabled(false);

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
	 * Returns the help functionality to support creating trajectory process
	 * 
	 * @return The image in the form of question mark with the help info
	 */
	private Image getHelp() {
		Image help = new Image("resources/images/question-icon.png");
		help.setWidth("18px");
		help.ensureDebugId("helpButton");
		help.setVisible(true);
		HTML helpInfo = new HTML(
				"<h3>Three simple steps to create a symbolic trajectory on the basis of your own GPX data:</h3>"
				+"<ol type=\"1\">"
						+ "<li><h4 style=\"color:#009DD8\">Select and upload your GPX track</h4></li><p>"
						+ "<li><h4 style=\"color:#009DD8\">Select an option for creating symbolic trajectory</h4>"
						+ "<b>speed mode</b> with peofiles: <5km/h - slow (walking tempo), <20km/h - slow (cycle tempo), <50km/h - moderate tempo, >50km/h - fast tempo<br>"
						+ "<b>direction</b> with values: NorthEast, NorthWest, SouthWest, SouthEast<br>"
						+ "<b>distance</b> to the defined location<br>"
						+ "<b>administrative districts</b> (only within Germany)<br></li>"
						+ "<li><h4 style=\"color:#009DD8\">Press on the button Create trajectory </h4></li>"
						+ "Change to the next tab Try Trajectory and experiment with the generated trajectory"
						+	"</ol>");
		helpDialog= new HelpDialog("How to Create Symbolic Trajectory?", helpInfo);		

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
	 * Returns the file name from the file upload
	 * 
	 * @return The file name
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
	 * Returns the uploaded file name
	 * 
	 * @return the nameOfUploadedFile
	 */
	public String getNameOfUploadedFile() {
		return nameOfUploadedFile;
	}

	/**
	 * Sets the uploaded file name
	 * 
	 * @param nameOfUploadedFile
	 *            The nameOfUploadedFile to set
	 */
	public void setNameOfUploadedFile(String nameOfUploadedFile) {
		this.nameOfUploadedFile = nameOfUploadedFile;
	}

}
