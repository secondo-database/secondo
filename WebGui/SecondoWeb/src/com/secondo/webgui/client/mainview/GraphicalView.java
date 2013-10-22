package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

import com.google.gwt.core.client.JsArrayNumber;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.DOM;
import com.google.gwt.user.client.Element;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.secondo.webgui.client.datatypes.DataType;



public class GraphicalView extends Composite implements View{
	
	/*Elements for graphical output*/
	//private ScrollPanel scrollPanel = new ScrollPanel();
	private FlowPanel contentPanel = new FlowPanel();
	private Button refreshButton= new Button("Refresh");
	private Button resetButton= new Button("Reset Data");
	private HTML d3Heading = new HTML("<h2>D3 Example</h2>");
    private int width =0;
    private int height= 0;
    
    private ArrayList<DataType> currentResultTypeList = new ArrayList<DataType>();

	public GraphicalView() {

		//scrollPanel.add(contentPanel);
		//scrollPanel.getElement().setClassName("graphicalscrollpanel");

	   // contentPanel.add(d3Heading);
	       	    
		// create DOM element & attach to parent
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		contentPanel.getElement().setClassName("graphicalcontentpanel");
		
	    //add a reset data button
	    resetButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  resetData();
	      		  //addPointToDataset(51.3760448, 7.4947253); //Fernuni Hagen
	    		  //showPointArray(div);
	          }
		 });
	    //contentPanel.add(resetButton);

	    resetData();
	    //resizeView(width, height);
	    addPointToDataset(51.3760448, 7.4947253); //Fernuni Hagen
		showPointArray();
	
	}
	
	/**on resizing of the browser window the elements of the graphical view are readjusted*/
	@Override
	public void resizeWithCP(int width, int height){
		  contentPanel.setWidth(width-271 + "px");// add 40px for padding
		  contentPanel.setHeight(height-360 + "px");
		  this.width = width-271; 
		  this.height= height-360;
		  
		  /*removeSVG(); //funktioniert nicht
		  contentPanel.clear();
	    	if(hasPoints()){
	    		showPointArray();
	    	}
	    	  if(hasLines()){
	    	   showLineArray();
	    	 }*/
	}
	
	@Override
	public void resizeWithTextPanel(int width, int height){
		  contentPanel.setWidth(width-615 + "px"); // add 40px for padding
		  contentPanel.setHeight(height-130 +"px");
		  this.width = width-615; 
		  this.height= height-130;
		  /*removeSVG(); //funktioniert nicht
		  contentPanel.clear();
	    	if(hasPoints()){
	    		showPointArray();
	    	}
	    	  if(hasLines()){
	    	   showLineArray();
	    	 }*/
	}
	

	@Override
	public void resizeWithTextAndCP(int width, int height) {
		  contentPanel.setWidth(width-615 + "px"); // add 40px for padding
		  contentPanel.setHeight(height-360 + "px");
		  this.width = width-615; 
		  this.height= height-360;
		  /*removeSVG(); //funktioniert nicht
		  contentPanel.clear();
	    	if(hasPoints()){
	    		showPointArray();
	    	}
	    	  if(hasLines()){
	    	   showLineArray();
	    	 }*/
		
	}
	
	@Override
    public void resizeToFullScreen(int width, int height){
		
		contentPanel.setWidth(width-271 + "px");// add 40px for padding
    	contentPanel.setHeight(height-130 +"px");  //add 40px padding
    	
    	this.width = width-271; 
    	this.height= height-130;
    	
    	/*removeSVG();
    	contentPanel.clear();
    	if(hasPoints()){
    		showPointArray();
    	}
        if(hasLines()){
    	   showLineArray();
    	 }*/

	}
	
	/**removes all data from the graphical view*/
	public void resetData(){
		deleteAllPoints(); 
		deleteAllLines();
		deleteAllPathPoints();
		deleteAllPolygons();
		removeSVG();
		contentPanel.clear();
	}
	
	public Element initializeDiv(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		
		return div;
	}
	
	/** Checks if any points are in the array*/
	public native boolean hasPoints()/*-{
		
		return $wnd.hasPoints();
			}-*/;
	
	/** Checks if any lines are in the array*/
	public native boolean hasLines()/*-{
		
		return $wnd.hasLines();
			}-*/;
	
	/**Draws all points of the pointarray to the view*/
	public void showPointArray(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		showPointArrayJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the points to the size of the svg and draws them as circles*/
	public native void showPointArrayJS(Element div, int width, int height)/*-{
		
		$wnd.showPointArray(div, width, height);
			}-*/;
	
	/**Draws all lines of the linearray to the view*/
	public void showLineArray(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		showLineArrayJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the lines to the size of the svg and draws them*/
	public native void showLineArrayJS(Element div, int width, int height)/*-{
		
		$wnd.showLineArray(div, width, height);
			}-*/;
	
	public void addPolygon(){
		
		addPolygonJS(width, height);
	}
	
	/** Adds a path of points from the pointarray to the dataset of paths*/
	public native void addPolygonJS(int width, int height)/*-{
		
		$wnd.addD3Polygon(width, height);
			}-*/;
	
	/**Draws all points of the pointarray to the view*/
	public void createSVG(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		createSVGJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the polygon to the size of the svg and draws it*/
	public native void createSVGJS(Element div, int width, int height)/*-{
		
		$wnd.createSVG(div, width, height);
			}-*/;
	
	/**Draws all points of the pointarray to the view*/
	public void drawPolygon(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		drawPolygonJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the polygon to the size of the svg and draws it*/
	public native void drawPolygonJS(Element div, int width, int height)/*-{
		
		$wnd.drawD3Polygon(div, width, height);
			}-*/;
	
	/**Draws all paths of the patharray to the view*/
	public void showPaths(){
		Element div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		showPathsJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the paths to the size of the svg and draws them*/
	public native void showPathsJS(Element div, int width, int height)/*-{
		
		$wnd.showPaths(div, width, height);
			}-*/;

	/** Adds a point to the dataset to display on the graphical view*/
	public native void addPointToDataset(double x, double y)/*-{
		$wnd.addPointToDataset(x, y);
			}-*/;


	/** Deletes all data from the dataset*/
	public native void deleteAllPoints()/*-{
		$wnd.deleteAllPoints();
			}-*/;
	
	/** Adds a line to the dataset to display on the graphical view*/
	public native void addLineToDataset(double x1, double y1, double x2, double y2)/*-{
		$wnd.addLineToDataset(x1, y1, x2, y2);
			}-*/;
	
	/** Deletes all data from the dataset*/
	public native void deleteAllLines()/*-{
		$wnd.deleteAllD3Lines();
			}-*/;
	
	/** Adds a line to the dataset to display on the graphical view*/
	public native void addPointToPath(double x, double y)/*-{
		$wnd.addPointToPath(x, y);
			}-*/;
	
	/** Deletes all data from the dataset*/
	public native void deleteAllPathPoints()/*-{
		$wnd.deleteAllPathPoints();
			}-*/;
	
	/** Deletes all data from the dataset*/
	public native void deleteAllPolygons()/*-{
		$wnd.deleteAllD3Polygons();
			}-*/;
	
	/** Removes the points-svg from the view*/
	public native void removeSVG()/*-{
		$wnd.removeSVG();
			}-*/;
	

	/** Gets the data from the dataset, scales the points to the size of the svg and draws them as circles*/
	public native void testPointArray(Element div, int width, int height)/*-{

		$wnd.testPointArray(div, width, height);
			}-*/;
	
	
	/** call d3_test function from an external js-file*/
	private native void d3_shapes(Element div)/*-{
		$wnd.d3_shapes_test(div);
			}-*/;
	
	/** call d3_lines function from an external js-file*/
	private native void d3_lines(Element div)/*-{
		$wnd.d3_lines_test(div);
			}-*/;
	
	/** call d3_lines function from an external js-file*/
	private native void d3_path(Element div)/*-{
		$wnd.d3_path_test(div);
			}-*/;
	
	
	/** call d3 Barchart function with dom element & data from an external js-file*/
	private native void createBarchart(Element div, JsArrayNumber jsData)/*-{
		$wnd.d3_barchart(div, jsData);
	}-*/;

	public FlowPanel getContentPanel() {
		return contentPanel;
	}

	public void setContentPanel(FlowPanel contentPanel) {
		this.contentPanel = contentPanel;
	}

	public int getWidth() {
		return width;
	}

	public void setWidth(int width) {
		this.width = width;
	}

	public int getHeight() {
		return height;
	}

	public void setHeight(int height) {
		this.height = height;
	}

	public ArrayList<DataType> getCurrentResultTypeList() {
		return currentResultTypeList;
	}

	public void setCurrentResultTypeList(ArrayList<DataType> currentResultTypeList) {
		this.currentResultTypeList = currentResultTypeList;
	}


}
