package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.List;
import com.google.gwt.cell.client.TextCell;
import com.google.gwt.user.cellview.client.CellList;
import com.google.gwt.user.cellview.client.SimplePager;
import com.google.gwt.user.cellview.client.HasKeyboardSelectionPolicy.KeyboardSelectionPolicy;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.view.client.AsyncDataProvider;
import com.google.gwt.view.client.HasData;
import com.google.gwt.view.client.Range;
import com.google.gwt.view.client.SelectionChangeEvent;
import com.google.gwt.view.client.SelectionModel;
import com.google.gwt.view.client.SingleSelectionModel;

public class CellListTextView extends Composite{
	
	/*Elements for text output*/
	private ScrollPanel scrollPanel = new ScrollPanel();
	private VerticalPanel textPanel = new VerticalPanel();
    private CellList<String> textCellList = new CellList<String>(new TextCell());
    private final SelectionModel<String> selectionModel = new SingleSelectionModel<String>();
    private MyDataProvider dataProvider = new MyDataProvider(); 
    //private ListDataProvider<String> dataProvider = new ListDataProvider<String>();
    private SimplePager pager = new SimplePager();
    
    private static ArrayList<String> formattedList =  new ArrayList<String>();
    
    public CellListTextView(){
    	
    	textCellList.setKeyboardSelectionPolicy(KeyboardSelectionPolicy.ENABLED);
		 
		// Add a selection model to handle user selection. what to do when a listentry is selected.
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
		
		// Push data into the CellList. Only necessary without mydataprovider
	    // textCellList.setRowCount(formattedList.size(), true);
	    // textCellList.setRowData(0, formattedList);
		
		textPanel.add(pager);
		textPanel.add(textCellList);
		textPanel.getElement().setClassName("celllisttextpanel");
		
		//add a fix width
		scrollPanel.setWidth("250px");
		scrollPanel.add(textPanel);
		scrollPanel.getElement().setClassName("celllistscrollpanel");

    }
      
		public void resize(int width, int height) {
			scrollPanel.setHeight(height-330 + "px");
			textPanel.setHeight(height-350 + "px");
			
		}

		public void resizeToFullScreen(int width, int height) {
			scrollPanel.setHeight(height-100+"px");
			textPanel.setHeight(height-120+"px");
			
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
	  

	  public ArrayList<String> getFormattedList() {
			return formattedList;
		}


		public void setFormattedList(ArrayList<String> formattedList) {
			this.formattedList = formattedList;
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

		public ScrollPanel getScrollPanel() {
			return scrollPanel;
		}

		public void setScrollPanel(ScrollPanel scrollPanel) {
			this.scrollPanel = scrollPanel;
		}


}
