package com.secondo.webgui.client.mainview;

import com.google.gwt.core.client.JsArrayNumber;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.DOM;
import com.google.gwt.user.client.Element;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;


public class GraphicalView extends Composite {
	
	/*Elements for graphical output*/
	private ScrollPanel scrollPanel = new ScrollPanel();
	private FlowPanel contentPanel = new FlowPanel();
	private Button refreshButton= new Button("Refresh");
	private Button resetButton= new Button("Reset Data");
	private HTML d3Heading = new HTML("<h2>D3 Example</h2>");
    private Element div = DOM.createDiv();
    private int width =0;
    private int height= 0;

	public GraphicalView() {

		scrollPanel.add(contentPanel);
		scrollPanel.getElement().setClassName("graphicalscrollpanel");

	    contentPanel.add(d3Heading);
	       	    
		// create DOM element & attach to parent
	    div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
		
	    //add a reset data button
	    resetButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  resetData();
	      		  addPointToDataset(51.3760448, 7.4947253); //Fernuni Hagen
	    		  showPointArray(div);
	          }
		 });
	    contentPanel.add(resetButton);

	    resetData();
	    //resizeView(width, height);
	    addPointToDataset(51.3760448, 7.4947253); //Fernuni Hagen
		showPointArray(div);

		// load data from somewhere (here randomly generated)
	   /* double[] data = new double[10];
		for (int i = 0; i < data.length; i++) {
			data[i] = Math.random() + .1;
		}*/

		// convert data into jsarray
		/*JsArrayNumber jsData = JavaScriptObject.createArray().cast();
		for (int i = 0; i < data.length; i++) {
			jsData.push(data[i]);
		}*/
	
	}
	
	/**on resizing of the browser window the elements of the graphical view are readjusted*/
	public void resize(int width, int height){
		  scrollPanel.setWidth(width-120 + "px");
		  scrollPanel.setHeight(height-380 + "px");
		  this.width = width-120;
		  this.height= height-380;
		  resizeView(width, height);
	}
	
	/**removes all data from the graphical view*/
	public void resetData(){
		deleteAllPoints(); 
		deleteAllLines();
		removeSVG();
	}
	
	/** Resizes the svg*/
	public native void resizeView(int width, int height)/*-{
		
		$wnd.resizeView(width, height);
			}-*/;
	
	public void showPointArray(Element div){
		showPointArrayJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the points to the size of the svg and draws them as circles*/
	public native void showPointArrayJS(Element div, int width, int height)/*-{
		
		$wnd.showPointArray(div, width, height);
			}-*/;
	
	public void showLineArray(Element div){
		showLineArrayJS(div, width, height);
	}
	
	/** Gets the data from the dataset, scales the lines to the size of the svg and draws them*/
	public native void showLineArrayJS(Element div, int width, int height)/*-{
		
		$wnd.showLineArray(div, width, height);
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
		$wnd.deleteAllLines();
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

	public ScrollPanel getScrollPanel() {
		return scrollPanel;
	}


	public void setScrollPanel(ScrollPanel scrollPanel) {
		this.scrollPanel = scrollPanel;
	}

	public Element getDiv() {
		return div;
	}

}
