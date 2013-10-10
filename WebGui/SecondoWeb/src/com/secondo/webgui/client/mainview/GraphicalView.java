package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.List;
import com.google.gwt.cell.client.TextCell;
import com.google.gwt.core.client.JavaScriptObject;
import com.google.gwt.core.client.JsArrayNumber;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.cellview.client.CellList;
import com.google.gwt.user.cellview.client.HasKeyboardSelectionPolicy.KeyboardSelectionPolicy;
import com.google.gwt.user.cellview.client.SimplePager;
import com.google.gwt.user.client.DOM;
import com.google.gwt.user.client.Element;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.view.client.AsyncDataProvider;
import com.google.gwt.view.client.HasData;
import com.google.gwt.view.client.ListDataProvider;
import com.google.gwt.view.client.Range;
import com.google.gwt.view.client.SelectionChangeEvent;
import com.google.gwt.view.client.SelectionModel;
import com.google.gwt.view.client.SingleSelectionModel;


public class GraphicalView extends Composite {

	private HorizontalPanel hPanel = new HorizontalPanel();
	
	/*Elements for text output*/
	private ScrollPanel textScrollPanel = new ScrollPanel();
	private VerticalPanel textPanel = new VerticalPanel();
	private DecoratorPanel decPanel = new DecoratorPanel();
	//private TextArea formattedOutput = new TextArea();
    private CellList<String> textCellList = new CellList<String>(new TextCell());
    private final SelectionModel<String> selectionModel = new SingleSelectionModel<String>();
    private MyDataProvider dataProvider = new MyDataProvider(); 
    //private ListDataProvider<String> dataProvider = new ListDataProvider<String>();
    private SimplePager pager = new SimplePager();
	
	/*Elements for graphical output*/
	private ScrollPanel scrollPanel = new ScrollPanel();
	private FlowPanel contentPanel = new FlowPanel();
	private Button refreshButton= new Button("Refresh");
	private HTML d3Heading = new HTML("<h2>D3 Example</h2>");
	
	private static ArrayList<String> formattedList =  new ArrayList<String>();
    private Element div = DOM.createDiv();

	public GraphicalView() {

		 textCellList.setKeyboardSelectionPolicy(KeyboardSelectionPolicy.ENABLED);
		// textCellList.setHeight("440px");
		 
		// Add a selection model to handle user selection.
	     textCellList.setSelectionModel(selectionModel);
		 selectionModel.addSelectionChangeHandler(new SelectionChangeEvent.Handler() {
		      public void onSelectionChange(SelectionChangeEvent event) {
		        String selected = ((SingleSelectionModel<String>) selectionModel).getSelectedObject();
		        if (selected != null) {
		          //Window.alert("You selected: " + selected);
		        }
		      }
		    });
		    	   		 
	    // Add the cellList to the dataProvider.
	     dataProvider.addDataDisplay(textCellList);

		 // Create paging controls.
	     pager.setDisplay(textCellList);
	     //pager.setPageSize(10);
		
		// Push data into the CellList.
	    // textCellList.setRowCount(formattedList.size(), true);
	    // textCellList.setRowData(0, formattedList);
		
		decPanel.add(textScrollPanel);
		
		/*formattedOutput.setEnabled(false);
		formattedOutput.setSize("250px", "440px");
		formattedOutput.getElement().setClassName("graphicaltextoutput");*/		
		//textPanel.add(formattedOutput);
		
		textPanel.add(pager);
		textPanel.add(textCellList);
		
		textScrollPanel.setSize("250", "450px");
		textScrollPanel.add(textPanel);

		//panels for graphic
		scrollPanel.setSize("600px", "460px");
		scrollPanel.add(contentPanel);

	    contentPanel.add(d3Heading);
		// create DOM element & attach to parent
	    div = DOM.createDiv();
		contentPanel.getElement().appendChild(div);
	    contentPanel.add(refreshButton);
	    refreshButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  removeSVG();
	        	  showPointArray(div);
	        	//initializePointArray(div);
	          }
		 });
		
		//main panel
		hPanel.setSize("890px", "460px");
		hPanel.add(decPanel);
		hPanel.add(scrollPanel);
		
		//initialize graphical view
		deleteAllPoints(); 
		deleteAllLines();
		removeSVG();
		addPointToDataset(51.3760448, 7.4947253); //Fernuni Hagen
		showPointArray(div);
		//initializePointArray(div);
		

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
		
		//d3_shapes(div);
		
		//d3_lines(div);
		
		//d3_path(div);
		
		//d3_berlin(div);
		
		//createBarchart(div, jsData);			
	}

	
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
	
	/** call d3_lines function from an external js-file*/
	private native void d3_berlin(Element div)/*-{
		$wnd.d3_berlin_test(div);
			}-*/;
	
	
	/** call d3 Barchart function with dom element & data from an external js-file*/
	private native void createBarchart(Element div, JsArrayNumber jsData)/*-{
		$wnd.d3_barchart(div, jsData);
	}-*/;
	
	/** Gets the data from the dataset, scales the points to the size of the svg and draws them as circles*/
	public native void initializePointArray(Element div)/*-{
		
		$wnd.initializePointArray(div);
			}-*/;
	
	/** Gets the data from the dataset, scales the points to the size of the svg and draws them as circles*/
	public native void showPointArray(Element div)/*-{
		
		$wnd.showPointArray(div);
			}-*/;
	
	
	/** Gets the data from the dataset, scales the lines to the size of the svg and draws them*/
	public native void showLineArray(Element div)/*-{
		
		$wnd.showLineArray(div);
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
	

	public HorizontalPanel gethPanel() {
		return hPanel;
	}


	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}


	public ArrayList<String> getFormattedList() {
		return formattedList;
	}


	public void setFormattedList(ArrayList<String> formattedList) {
		this.formattedList = formattedList;
	}


	public ScrollPanel getScrollPanel() {
		return scrollPanel;
	}


	public void setScrollPanel(ScrollPanel scrollPanel) {
		this.scrollPanel = scrollPanel;
	}


	public VerticalPanel getTextPanel() {
		return textPanel;
	}


	public void setTextPanel(VerticalPanel textPanel) {
		this.textPanel = textPanel;
	}

	  public MyDataProvider getDataProvider() {
		return dataProvider;
	}


	public void setDataProvider(MyDataProvider dataProvider) {
		this.dataProvider = dataProvider;
	}

	public CellList<String> getTextCellList() {
		return textCellList;
	}


	public void setTextCellList(CellList<String> textCellList) {
		this.textCellList = textCellList;
	}

	public Element getDiv() {
		return div;
	}

	/**
	   * A custom {@link AsyncDataProvider}.
	   */
	  public static class MyDataProvider extends AsyncDataProvider<String> {
	    /**
	     * {@link #onRangeChanged(HasData)} is called when the table requests a new
	     * range of data. You can push data back to the displays using
	     * {@link #updateRowData(int, List)}.
	     */
	    @Override
	    protected void onRangeChanged(HasData<String> display) {
	      // Get the new range.
	      final Range range = display.getVisibleRange();

	      // Query the data asynchronously. If you are using a database, you can make an RPC call here.

	          if(!formattedList.isEmpty()){
	        	  
	          int start = range.getStart();
	          int length = range.getLength();
	          
	          List<String> newData = new ArrayList<String>();
	          for (int i = start; i < start + length; i++) {
	            newData.add(formattedList.get(i));
	          }

	          // Push the data to the displays. AsyncDataProvider will only update
	          // displays that are within range of the data.
	          updateRowData(start, newData);
	          }
	    }
	  }
}
