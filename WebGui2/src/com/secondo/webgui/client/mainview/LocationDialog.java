/**
 * 
 */
package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;

/**
 * @author Irina Russkaya
 *
 */
public class LocationDialog {
	private DialogBox locationDialogBox = new DialogBox();
	private FlowPanel locationDialogContents = new FlowPanel();
	private Button closeButton = new Button("Close");
	private Button getCoordinateButton = new Button("Get GPS coordinates");
	private String headline = "Define your location:";	
	private String streetLabel = "Street: ";
    private String buildingLabel = "Building nr.: ";
    private String zipLabel = "ZIP: ";
    private String cityLabel = "City: ";
    private TextBox street = new TextBox();
    private TextBox building = new TextBox();
    private TextBox zip = new TextBox();  
    private TextBox city = new TextBox();
    private FlexTable layout = new FlexTable();
    private DecoratorPanel decPanel = new DecoratorPanel();
    private Label labelForResult = new Label();
	
	public LocationDialog(){
		
		locationDialogBox.setText("My location");

	    // Create a table to layout the content
	    locationDialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    locationDialogBox.setWidget(locationDialogContents);
	    
	    layout.setCellSpacing(6);
        FlexCellFormatter cellFormatter = layout.getFlexCellFormatter();

        // Add a title to the form
        layout.setHTML(0, 0, this.headline);
        cellFormatter.setColSpan(0, 0, 2);
        cellFormatter.setHorizontalAlignment(
            0, 0, HasHorizontalAlignment.ALIGN_CENTER);

        // Add username and password fields
        street.setWidth("150px");
        street.setEnabled(true);
        building.setWidth("150px");
        building.setEnabled(true);
        zip.setWidth("150px");
        zip.setEnabled(true);
        city.setWidth("150px");
        city.setEnabled(true);
        layout.setHTML(1, 0, this.streetLabel);
        layout.setWidget(1, 1, street);
        layout.setHTML(2, 0, buildingLabel);
        layout.setWidget(2, 1, building);
        layout.setHTML(3, 0, zipLabel);
        layout.setWidget(3, 1, zip); 
        layout.setHTML(4, 0, cityLabel);
        layout.setWidget(4, 1, city);
        
     // Add a label with result to a form        
        layout.setWidget(5, 0, labelForResult);
        cellFormatter.setColSpan(5, 0, 2);
        cellFormatter.setHorizontalAlignment(
            5, 0, HasHorizontalAlignment.ALIGN_CENTER);
       
        // Wrap the content in a DecoratorPanel
        decPanel.setWidget(layout);
        
        locationDialogContents.add(decPanel);     

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  locationDialogBox.hide();}
	        });
	    closeButton.setStyleName("right-floated-button");
	    closeButton.getElement().setAttribute("margin-top", "3px");
	    
	    getCoordinateButton.setStyleName("right-floated-button");
	    getCoordinateButton.getElement().setAttribute("margin-top", "3px");
	    
	    locationDialogContents.add(getCoordinateButton);
	    locationDialogContents.add(closeButton);		
	}

	/**
	 * @return the locationDialogBox
	 */
	public DialogBox getLocationDialogBox() {
		return locationDialogBox;
	}

	/**
	 * @return the getCoordinateButton
	 */
	public Button getGetCoordinateButton() {
		return getCoordinateButton;
	}

	/**
	 * @return the street
	 */
	public TextBox getStreet() {
		return street;
	}

	/**
	 * @return the building
	 */
	public TextBox getBuilding() {
		return building;
	}

	/**
	 * @return the zip
	 */
	public TextBox getZip() {
		return zip;
	}

	/**
	 * @return the city
	 */
	public TextBox getCity() {
		return city;
	}

	/**
	 * @return the labelForResult
	 */
	public Label getLabelForResult() {
		return labelForResult;
	}

	/**
	 * @param labelForResult the labelForResult to set
	 */
	public void setLabelForResult(String result) {
		this.labelForResult.setText(result);
	}

}
