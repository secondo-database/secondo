//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.CheckBox;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratedStackPanel;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.Tree;
import com.google.gwt.user.client.ui.TreeItem;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.secondo.webgui.shared.model.DataType;

/**
*  This class represents the toolbox of the application which contains elements to edit the objects in the dataoutput views.
*  
*  @author Kristina Steiger
*  
**/
public class ToolBox extends Composite {

	/**The main panel of the toolbox*/
	private FlowPanel fpanel = new FlowPanel();
	
	/**The heading text of the toolbox*/
	private HTML heading = new HTML("<h2>ToolBox</h2>");
	
	/**The stackpanel of the toolbox to open one panel for editing at a time*/
    private DecoratedStackPanel stackPanel = new DecoratedStackPanel();
    
    //elements of the filter panel
    private Image filterIcon = new Image("resources/images/filters-icon.png");
    private CheckBox checkBoxPoints = new CheckBox("Points");
    private CheckBox checkBoxLines = new CheckBox("Lines");
    private CheckBox checkBoxPolygons = new CheckBox("Regions");
    
    //elements of the animation panel
    private VerticalPanel animationPanel = new VerticalPanel();
    private HorizontalPanel playpanel = new HorizontalPanel();
    private HorizontalPanel pausepanel = new HorizontalPanel();
	private TextBox timeCounter = new TextBox();
	private TimeSlider timeSlider = new TimeSlider(); 
	private Image animationIcon = new Image("resources/images/play-icon.png");
    private Image playIcon = new Image("resources/images/play-icon2.png");
    private Image forwardIcon = new Image("resources/images/speedup-icon.png");
    private Image rewindIcon = new Image("resources/images/speeddown-icon.png");
    private Image pauseIcon = new Image("resources/images/pause-icon.png");
	private Anchor playLink = new Anchor("Play");
	private Anchor forwardLink = new Anchor("Speed Up");
	private Anchor rewindLink = new Anchor("Speed Down");
	private Anchor pauseLink = new Anchor("Pause");
	
    //elements of the export panel
    private Image exportIcon = new Image("resources/images/export-icon.png");   
	private Anchor downloadTextLink = new Anchor("Export Text");
	private Anchor downloadRawDataLink = new Anchor("Export Raw Data");
    private Image downloadTextIcon = new Image("resources/images/text-icon-mini.png");  
    private Image downloadRawDataIcon = new Image("resources/images/page-white-icon.png");
    
    //elements of the reset panel
    private Image resetIcon = new Image("resources/images/red-cross-icon.png");
	private Anchor resetGraphicLink = new Anchor("Reset Graphic");
	private Anchor resetTextLink = new Anchor("Reset Text");
    private Image resetGraphicIcon = new Image("resources/images/graphic-icon.png");
    private Image resetTextIcon = new Image("resources/images/text-icon-mini.png"); 
    
    //elements of the object panel
    private FlowPanel objectPanel = new FlowPanel();
    private ScrollPanel objectScrollPanel = new ScrollPanel();
	private HorizontalPanel objectTools = new HorizontalPanel();
    private Tree objectTree = new Tree();  
    private Image objectsIcon = new Image("resources/images/cube-icon.jpg");
    private Button colorButton = new Button("<img src='resources/images/color-circle-icon.png' height='15px' width='15px'/>");
    private Button resetObjectlistButton = new Button("<img src='resources/images/red-cross-icon.png' height='15px' width='15px'/>");
    private ColorChooserDialog colorChooserDialog = new ColorChooserDialog();
    
    //Maps for checkboxes to make them controllable from mainview
    private Map<Number, CheckBox> queryBoxes = new HashMap<Number, CheckBox>();
    private Map<Number, CheckBox> objectBoxes = new HashMap<Number, CheckBox>();
    
    /** The list of all queryresults and the objects from the querys to display in the objectlist*/
    private ArrayList<ArrayList<DataType>> resultList = new ArrayList<ArrayList<DataType>>();

	public ToolBox() {
		
		fpanel.getElement().setClassName("toolbox");
		fpanel.setWidth("199px");
	    fpanel.add(heading);
	    
	    //Add items to objecttools
	    objectTools.setSize("50px", "20px");
	    objectTools.add(colorButton);
	    objectTools.add(resetObjectlistButton);
		
	    //Add items to objectpanel
		objectScrollPanel.setWidth("179px"); //- 20px for padding
		objectPanel.setWidth("179px");
		objectPanel.add(objectTools);
		objectPanel.add(objectScrollPanel);
		
		// Create a new stack panel
		stackPanel.getElement().setClassName("toolsstackpanel");
	    stackPanel.setWidth("199px");
	    stackPanel.ensureDebugId("cwStackPanel");
	    
	    //Set checkboxes on checked
	    checkBoxPoints.setValue(true);
	    checkBoxLines.setValue(true);
	    checkBoxPolygons.setValue(true);
	    
	   //Configure icons for object tools
	    colorButton.getElement().setClassName("colorpickerbutton");
	    colorButton.getElement().getStyle().setPadding(5, Unit.PX); 
	    colorButton.setTitle("Change color of a query");
	    resetObjectlistButton.getElement().setClassName("resetobjectlistbutton");
	    resetObjectlistButton.getElement().getStyle().setPadding(5, Unit.PX);   
	    resetObjectlistButton.setTitle("Delete all objects from the objectlist");
	    
	    //Configure icons for stackpanelheader
	    filterIcon.setSize("25px", "25px");
	    filterIcon.getElement().getStyle().setPadding(5, Unit.PX);
	    resetIcon.setSize("25px", "25px");
	    resetIcon.getElement().getStyle().setPadding(5, Unit.PX);
	    objectsIcon.setSize("25px", "25px");
	    objectsIcon.getElement().getStyle().setPadding(5, Unit.PX);
	    exportIcon.setSize("25px", "25px");
	    exportIcon.getElement().getStyle().setPadding(5, Unit.PX);
	    animationIcon.setSize("25px", "25px");
	    animationIcon.getElement().getStyle().setPadding(5, Unit.PX);
	    
	    //Configure icons for stackpanel items    
	    playIcon.setSize("16px", "16px");
	    playIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    playIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    forwardIcon.setSize("16px", "16px");
	    forwardIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    forwardIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    rewindIcon.setSize("16px", "16px");
	    rewindIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    rewindIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    pauseIcon.setSize("16px", "16px");
	    pauseIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    pauseIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    
	    timeCounter.setEnabled(false);
	    timeCounter.setSize("170px", "20px");
	    timeCounter.setText("Press Play to start animation");
	    timeCounter.getElement().getStyle().setBackgroundColor("white");
	    timeCounter.getElement().getStyle().setBorderWidth(0, Unit.PX);
	    
	    downloadTextIcon.setWidth("16px");
	    downloadTextIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    downloadTextIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    downloadTextIcon.setTitle("Download All Data from Text View to Textfile");
	    
	    downloadRawDataIcon.setWidth("16px");
	    downloadRawDataIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    downloadRawDataIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    downloadRawDataIcon.setTitle("Download All Data from Raw Data View to Textfile");
	    
	    resetGraphicIcon.setWidth("16px");
	    resetGraphicIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    resetGraphicIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    resetGraphicIcon.setTitle("Delete all graphical data");	    
	    resetTextIcon.setWidth("16px");
	    resetTextIcon.getElement().getStyle().setPaddingRight(5, Unit.PX);
	    resetTextIcon.getElement().getStyle().setPaddingLeft(5, Unit.PX);
	    resetTextIcon.setTitle("Delete all data from the text panel");	    
	    
	    // Add a list of filters to the stackpanel
	    String filtersHeader = getHeaderString("Filters", filterIcon);
	    stackPanel.add(createFiltersItem(), filtersHeader, true);
	    
	    // Add a list of animation functions to the stackpanel
	    String animationHeader = getHeaderString("Animation", animationIcon);
	    stackPanel.add(createAnimationItem(), animationHeader, true);   
	    
        // Add a list of export functions to the stackpanel
	    String exportHeader = getHeaderString("Export", exportIcon);
	    stackPanel.add(createExportItem(), exportHeader, true);
	    
	   // Add a list of reset links to the stackpanel
	    String resetHeader = getHeaderString("Reset Views", resetIcon);
	    stackPanel.add(createResetItem(), resetHeader, true); 
	    
	    // Add a list of objects to the stackpanel
	    String objectsHeader = getHeaderString("Objects", objectsIcon);
	    stackPanel.add(createObjectsItem(), objectsHeader, true);
	    
	    stackPanel.showStack(4);
	    
	    fpanel.add(stackPanel);
	    
	    /*Add an event handler on the resetobjectlist button to reset the objectlist*/
		this.resetObjectlistButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  resetData();
	          }
		 });
		
		// Open the colorpopup when the user clicks on the color circle
	    this.colorButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	         // Show the popup of contact info
	          int left = colorButton.getAbsoluteLeft() - 100;
	          int top = colorButton.getAbsoluteTop() - 100;
	          
	          //colorChooserDialog.getDialogBox().center();
	          colorChooserDialog.getDialogBox().setPopupPosition(left, top);
	          colorChooserDialog.getDialogBox().show();
	        }
	      });
	}
	
	/**On resizing of the browser window the elements of the toolbox are readjusted with the commandpanel displayed
	 * 
	 * @param height The new height of the main panel and the objectpanel of the toolbox
	 * */
	public void resizeHeightWithCP(int height){
		
		if(height > 650){
			//add 10 pixel for bottom padding
			fpanel.setHeight(height-331 + "px");
			//add 50px for heading + 20px for objecttools + 5x35 = 175px for stackpanelheader
		    objectScrollPanel.setHeight(height-616 + "px");
		    objectPanel.setHeight(height-596 + "px");
		}	
		else{
			fpanel.setHeight(650-331 + "px");
		    objectScrollPanel.setHeight(650-616 + "px");
		    objectPanel.setHeight(650-596 + "px");
		}
	}
	
	/**On resizing of the browser window the elements of the toolbox are readjusted to fullscreen
	 * 
	 * @param height The new height of the main panel and the objectpanel of the toolbox
	 * */
	public void resizeHeightToFullScreen(int height){
		
		if(height > 650){
			//add 10 pixel for bottom padding
			fpanel.setHeight(height-101 + "px"); 
			//add 50px for heading +20px for objecttools + 5x35 = 175px for stackpanelheader
		    objectScrollPanel.setHeight(height-386 + "px"); 
		    objectPanel.setHeight(height-366 + "px");
		}	
		else{
			fpanel.setHeight(650-101 + "px"); 
		    objectScrollPanel.setHeight(650-386 + "px"); 
		    objectPanel.setHeight(650-366 + "px");
		}
	}
	
	/**Deletes all data from elements*/
	public void resetData(){
		this.deleteObjectList();
		queryBoxes.clear();
		objectBoxes.clear();
		timeSlider.resetSlider();
		timeCounter.setText("Press Play to start animation");
		colorChooserDialog.getQueryBox().clear();
	}	
	
	/**Resets the animation panel to the default values*/
	public void resetAnimationPanel(){
		
		animationPanel.remove(0);
		animationPanel.insert(playpanel, 0);
		timeSlider.resetSlider();
		timeCounter.setText("Press Play to start animation");		
	}
	
	/**Creates the list of filters for the Filters item.
	   *
	   * @return the list of filters
	   */
	  private VerticalPanel createFiltersItem() {
	    VerticalPanel filtersPanel = new VerticalPanel();
	    filtersPanel.setSpacing(4);
	    filtersPanel.add(checkBoxPoints);
	    filtersPanel.add(checkBoxLines);
	    filtersPanel.add(checkBoxPolygons);
	    
	    return filtersPanel;
	  }
	
	/**Creates the list of animation functions for the animation item.
	   *
	   * @return a verticalpanel with a list of animation functions
	   */
	  private VerticalPanel createAnimationItem() {

	    animationPanel.setSpacing(4);
	    
        playpanel.add(playIcon);
        playpanel.add(playLink);       
	    animationPanel.add(playpanel);
	    
	    HorizontalPanel forwardpanel = new HorizontalPanel();
        forwardpanel.add(forwardIcon);
        forwardpanel.add(forwardLink);       
	    animationPanel.add(forwardpanel);
	    
	    HorizontalPanel rewindpanel = new HorizontalPanel();
        rewindpanel.add(rewindIcon);
        rewindpanel.add(rewindLink);       
	    animationPanel.add(rewindpanel);
	    
        pausepanel.add(pauseIcon);
        pausepanel.add(pauseLink);       
	    
	    animationPanel.add(timeCounter);
	    
	    animationPanel.add(timeSlider.getMainPanel());

	    return animationPanel;
	  }
	
	/**Creates the list of export functions for the export item.
	   *
	   * @return the list of export functions
	   */
	  private VerticalPanel createExportItem() {
	    VerticalPanel exportPanel = new VerticalPanel();
	    exportPanel.setSpacing(4);

	    HorizontalPanel exporttextpanel = new HorizontalPanel();
        exporttextpanel.add(downloadTextIcon);
        exporttextpanel.add(downloadTextLink);
        
        HorizontalPanel exportrawdatapanel = new HorizontalPanel();
        exportrawdatapanel.add(downloadRawDataIcon);
        exportrawdatapanel.add(downloadRawDataLink);
        
        exportPanel.add(exporttextpanel); 
        exportPanel.add(exportrawdatapanel); 

	    return exportPanel;
	  }
	
	/**Creates the list of resetButtons for the Reset item.
	   *
	   * @return the list of reset Buttons
	   */
	  private VerticalPanel createResetItem() {
	    VerticalPanel resetPanel = new VerticalPanel();
	    resetPanel.setSpacing(4);
	    
        HorizontalPanel resetgraphicpanel = new HorizontalPanel();
        resetgraphicpanel.add(resetGraphicIcon);
        resetgraphicpanel.add(resetGraphicLink);
        
        HorizontalPanel resettextpanel = new HorizontalPanel();
        resettextpanel.add(resetTextIcon);
        resettextpanel.add(resetTextLink);
	    
        resetPanel.add(resettextpanel);       
        resetPanel.add(resetgraphicpanel);
	    
	    return resetPanel;
	  }
	
	/**Creates a list of checkboxes for objects and puts them to the object tree
	   *
	   * @return the ScrollPanel with the objectlist
	   */
	  private FlowPanel createObjectsItem() {
	    
	    objectScrollPanel.clear();
	    objectTree.clear();
	    objectScrollPanel.add(objectTree);
	    colorChooserDialog.getQueryBox().clear();
	    
	    //get objects from resultlist and put them to the objectspanel
	    if(!resultList.isEmpty()){
	    
	    int index= 1;
	    	
	    for(ArrayList<DataType> result : resultList){
	    	
	    	String datatype = result.get(0).getType();
	    	CheckBox cbQuery = new CheckBox("query " + index + " : " + datatype);
	    	colorChooserDialog.getQueryBox().addItem("query " + index + " : " + datatype);
	    	TreeItem item = new TreeItem(cbQuery);
	    	queryBoxes.put(resultList.indexOf(result), cbQuery); //index of queryresult and query checkbox
		
		    for(DataType data : result){
		    	
		    	//add name for object checkbox
				  CheckBox cbObject = new CheckBox(data.getName());
			      TreeItem objectItem = new TreeItem(cbObject);
			      item.addItem(objectItem);
			      objectBoxes.put(data.getId(), cbObject); //id of object and object checkbox

			 //add attributes
		     if(!data.getAttributeList().isEmpty()){

		         final ArrayList<String> attributeList = data.getAttributeList();
		      
		         //add only text for attributes, no checkboxes
		         for(String attribute : attributeList){
		    	     objectItem.addTextItem(attribute);
		         }
		     }
		    }
		    objectTree.addItem(item);
		    index++;
	      }    
	    }
	    return objectPanel;
	  }
	

	  /**Gets a string representation of the header that includes an image and some text.
	   *
	   * @param text the header text
	   * @param image the Image to add next to the header
	   * @return the header as a string
	   */
	  private String getHeaderString(String text, Image image) {
	    // Add the image and text to a horizontal panel
	    HorizontalPanel hPanel = new HorizontalPanel();
	    hPanel.setSpacing(0);
	    hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
	    hPanel.add(image);
	    HTML headerText = new HTML(text);
	    headerText.setStyleName("cw-StackPanelHeader");
	    hPanel.add(headerText);

	    // Return the HTML string for the panel
	    return hPanel.getElement().getString();
	  }
	
	/**Updates the objectlist and puts the query result inside*/
	public void updateObjectList(){
		
		stackPanel.remove(4);
		
		 // Add a list of objects
	    String objectsHeader = getHeaderString("Objects", objectsIcon);
	    stackPanel.add(createObjectsItem(), objectsHeader, true);	    
	    stackPanel.showStack(4);
	}
	
	/**Deletes all objects from the objectlist*/
	public void deleteObjectList(){
		
		resultList.clear();		
		stackPanel.remove(4);
		
		 // Add the object header
	    String objectsHeader = getHeaderString("Objects", objectsIcon);
	    stackPanel.add(createObjectsItem(), objectsHeader, true);	    
	    stackPanel.showStack(4);		
	}
	
	/**Returns the main panel of the toolbox
	 * 
	 * @return The main panel of the toolbox
	 * */
	public FlowPanel getFpanel() {
		return fpanel;
	}

	/**Returns the reset link for the graphic in graphical and map view
	 * 
	 * @return The reset link for the graphic
	 * */
	public Anchor getResetGraphicLink() {
		return resetGraphicLink;
	}

	/**Returns the reset link for the text view
	 * 
	 * @return The reset link for the text view
	 * */
	public Anchor getResetTextLink() {
		return resetTextLink;
	}

	/**Returns the play link to start animations
	 * 
	 * @return The play link to start animations
	 * */
	public Anchor getPlayLink() {
		return playLink;
	}

	/**Returns the pause link to pause the animations
	 * 
	 * @return The pause link to pause the animations
	 * */
	public Anchor getPauseLink() {
		return pauseLink;
	}

	/**Returns the forward link speed up animations
	 * 
	 * @return The forward link speed up animations
	 * */
	public Anchor getForwardLink() {
		return forwardLink;
	}

	/**Returns the rewind link slow down animations
	 * 
	 * @return The rewind link slow down animations
	 * */
	public Anchor getRewindLink() {
		return rewindLink;
	}

	/**Returns the animation panel
	 * 
	 * @return The animation panel
	 * */
	public VerticalPanel getAnimationPanel() {
		return animationPanel;
	}

	/**Returns the play link panel
	 * 
	 * @return The play link panel
	 * */
	public HorizontalPanel getPlaypanel() {
		return playpanel;
	}

	/**Returns the pause link panel
	 * 
	 * @return The pause link panel
	 * */
	public HorizontalPanel getPausepanel() {
		return pausepanel;
	}

	/**Returns the download text link
	 * 
	 * @return The download text link
	 * */
	public Anchor getDownloadTextLink() {
		return downloadTextLink;
	}

	/**Returns the download raw data link
	 * 
	 * @return The download raw data link
	 * */
	public Anchor getDownloadRawDataLink() {
		return downloadRawDataLink;
	}

	/**Returns the checkbox for points
	 * 
	 * @return The checkbox for points
	 * */
	public CheckBox getCheckBoxPoints() {
		return checkBoxPoints;
	}

	/**Returns the checkbox for lines
	 * 
	 * @return The checkbox for lines
	 * */
	public CheckBox getCheckBoxLines() {
		return checkBoxLines;
	}

	/**Returns the checkbox for polygons
	 * 
	 * @return The checkbox for polygons
	 * */
	public CheckBox getCheckBoxPolygons() {
		return checkBoxPolygons;
	}

	/**Returns the resultlist for secondo query results
	 * 
	 * @return The resultlist for secondo query results
	 * */
	public ArrayList<ArrayList<DataType>> getResultList() {
		return resultList;
	}

	/**Returns a hashmap with all checkboxes for query results
	 * 
	 * @return A hashmap with all checkboxes for query results
	 * */
	public Map<Number, CheckBox> getQueryBoxes() {
		return queryBoxes;
	}

	/**Returns a hashmap with all checkboxes for object results
	 * 
	 * @return A hashmap with all checkboxes for object results
	 * */
	public Map<Number, CheckBox> getObjectBoxes() {
		return objectBoxes;
	}

	/**Returns the color chooser dialog box
	 * 
	 * @return The color chooser dialog box
	 * */
	public ColorChooserDialog getColorChooserDialog() {
		return colorChooserDialog;
	}

	/**Returns the textbox for the time counter
	 * 
	 * @return The textbox for the time counter
	 * */
	public TextBox getTimeCounter() {
		return timeCounter;
	}

	/**Returns the timeslider object
	 * 
	 * @return The timeslider object
	 * */
	public TimeSlider getTimeSlider() {
		return timeSlider;
	}
}
