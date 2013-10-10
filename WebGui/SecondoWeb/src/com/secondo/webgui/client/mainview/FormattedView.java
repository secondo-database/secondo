package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

import com.google.gwt.cell.client.AbstractCell;
import com.google.gwt.safehtml.shared.SafeHtmlBuilder;
import com.google.gwt.user.cellview.client.CellList;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.view.client.ProvidesKey;
import com.google.gwt.view.client.SelectionModel;
import com.google.gwt.view.client.SingleSelectionModel;

public class FormattedView extends Composite {
	
	private ScrollPanel formattedScrollPanel = new ScrollPanel();
	private VerticalPanel contentPanel = new VerticalPanel();
	private TextArea formattedOutput = new TextArea();
	private CellList<String> cellList = new CellList<String>(new dataCell());
	private ArrayList<String> formattedList =  new ArrayList<String>();
	
	/**
	   * A custom {@link Cell} used to render a {@link String}.
	   */
	   private static class dataCell extends AbstractCell<String> {
	      @Override
	      public void render(com.google.gwt.cell.client.Cell.Context context,
					String value, SafeHtmlBuilder sb) {
	         if (value != null) {
	            sb.appendEscaped(value);
	         }		
	      }
	   }

	
	public FormattedView(){
		
		// Push data into the CellList.
	      cellList.setRowCount(formattedList.size(), true);
	      cellList.setRowData(0, formattedList);

	      // Add a selection model
	      SelectionModel<String> selectionModel 
	      = new SingleSelectionModel<String>();
	      cellList.setSelectionModel(selectionModel);
	
		  formattedScrollPanel.setSize("890px", "460px");
		
		  formattedOutput.setEnabled(false);
		  formattedOutput.setSize("880px", "450px");
		  formattedOutput.getElement().setClassName("formattedoutput");
		
		  formattedScrollPanel.add(formattedOutput);
		  //formattedScrollPanel.add(cellList);
		
		  contentPanel.add(formattedScrollPanel);
		
	}

	public TextArea getFormattedOutput() {
		return formattedOutput;
	}

	public void setFormattedOutput(TextArea formattedOutput) {
		this.formattedOutput = formattedOutput;
	}

	public ScrollPanel getFormattedScrollPanel() {
		return formattedScrollPanel;
	}

	public void setFormattedScrollPanel(ScrollPanel formattedScrollPanel) {
		this.formattedScrollPanel = formattedScrollPanel;
	}

	public VerticalPanel getContentPanel() {
		return contentPanel;
	}

	public void setContentPanel(VerticalPanel contentPanel) {
		this.contentPanel = contentPanel;
	}

	public ArrayList<String> getFormattedList() {
		return formattedList;
	}

	public void setFormattedList(ArrayList<String> formattedList) {
		this.formattedList = formattedList;
	}

	public CellList<String> getCellList() {
		return cellList;
	}

	public void setCellList(CellList<String> cellList) {
		this.cellList = cellList;
	}
	

}
