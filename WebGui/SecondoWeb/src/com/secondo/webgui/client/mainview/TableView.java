package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.Comparator;
import com.google.gwt.core.client.GWT;
import com.google.gwt.user.cellview.client.CellTable;
import com.google.gwt.user.cellview.client.SimplePager;
import com.google.gwt.user.cellview.client.TextColumn;
import com.google.gwt.user.cellview.client.ColumnSortEvent.ListHandler;
import com.google.gwt.user.cellview.client.HasKeyboardSelectionPolicy.KeyboardSelectionPolicy;
import com.google.gwt.user.cellview.client.SimplePager.TextLocation;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.view.client.DefaultSelectionEventManager;
import com.google.gwt.view.client.MultiSelectionModel;
import com.google.gwt.view.client.ProvidesKey;
import com.google.gwt.view.client.SelectionModel;

public class TableView extends Composite {

	private ArrayList<TableItem> tableItems = new ArrayList<TableItem>();
	private CellTable<TableItem> table = new CellTable<TableItem>(KEY_PROVIDER);
	// Create a data provider. instead of arraylist
	   // private ListDataProvider<Contact> dataProvider = new ListDataProvider<Contact>();
	/**
	   * The key provider that allows us to identify Contacts even if a field
	   * changes. We identify contacts by their unique ID.
	   */
	private static final ProvidesKey<TableItem> KEY_PROVIDER = new ProvidesKey<TableItem>() {
	        @Override
	        public Object getKey(TableItem item) {
	          return item.id;
	        }
	      };
	private SimplePager pager;
	
	public TableView (){
		
		      //add some example data to the data grid
				tableItems.add(new TableItem("algebras", "StandardAlgebra FTextAlgebra RelationAlgebra ExtRelationAlgebra ExtRelation2Algebra OrderedRelationAlgebra BTreeAlgebra TupleIdentifierAlgebra StreamAlgebra FunctionAlgebra HashAlgebra PointRectangleAlgebra StreamExampleAlgebra PolygonAlgebra BinaryFileAlgebra DateAlgebra RTreeAlgebra TBTreeAlgebra RectangleAlgebra SpatialAlgebra TemporalAlgebra TemporalExtAlgebra TemporalLiftedAlgebra TemporalUnitAlgebra DateTimeAlgebra NetworkAlgebra TemporalNetAlgebra PlugJoinAlgebra UpdateRelationAlgebra ImExAlgebra MovingRegionAlgebra PlaneSweepAlgebra TopRelAlgebra TopOpsAlgebra ArrayAlgebra PictureAlgebra GraphAlgebra GSLAlgebra SimulationAlgebra HistogramAlgebra CollectionAlgebra ClusterAlgebra NestedRelationAlgebra BTree2Algebra RecordAlgebra SpatialJoinAlgebra TrieAlgebra MapMatchingAlgebra SuffixTreeAlgebra OsmAlgebra GroupbyAlgebra MMRTreeAlgebra HadoopParallelAlgebra SymbolicTrajectoryAlgebra"));
				tableItems.add(new TableItem("databases", "BERLINTEST OPT SYMTRAJ"));
		        tableItems.add(new TableItem("point", "9396.0 9871.0"));
		        
		     // Create a CellTable.  
		        table.setKeyboardSelectionPolicy(KeyboardSelectionPolicy.ENABLED);
		        table.setAutoHeaderRefreshDisabled(true); //no autorefresh of the header
		          	  
		        
		     // Attach a column sort handler to the ListDataProvider to sort the list.
		        ListHandler<TableItem> sortHandler =
		            new ListHandler<TableItem>(tableItems);
		        table.addColumnSortHandler(sortHandler);
		        
		        
		     // Create a Pager to control the table.
		        SimplePager.Resources pagerResources = GWT.create(SimplePager.Resources.class);
		        pager = new SimplePager(TextLocation.CENTER, pagerResources, false, 0, true);
		        pager.setDisplay(table);
		       
		        
		      // Add a text column to show the name.
		        TextColumn<TableItem> typeColumn = new TextColumn<TableItem>() {
		          @Override
		          public String getValue(TableItem item) {
		            return item.getType();
		          }
		        };
		        table.addColumn(typeColumn, "Type");
		        typeColumn.setSortable(true);
		        sortHandler.setComparator(typeColumn, new Comparator<TableItem>() {
		            @Override
		            public int compare(TableItem o1, TableItem o2) {
		              return o1.getType().compareTo(o2.getType());
		            }
		          });
		        
		     // We know that the data is sorted alphabetically by default.
		        table.getColumnSortList().push(typeColumn);

		        
		     // Add a text column to show the data.
		        TextColumn<TableItem> dataColumn = new TextColumn<TableItem>() {
		          @Override
		          public String getValue(TableItem object) {
		            return object.getData();
		          }
		        };
		        table.addColumn(dataColumn, "Data");
		        dataColumn.setSortable(true);

		        // Add a selection model to handle user selection.
		        /*final SingleSelectionModel<Contact> selectionModel = new SingleSelectionModel<Contact>();
		        table.setSelectionModel(selectionModel);
		        selectionModel.addSelectionChangeHandler(new SelectionChangeEvent.Handler() {
		          public void onSelectionChange(SelectionChangeEvent event) {
		            Contact selected = selectionModel.getSelectedObject();
		            if (selected != null) {
		              Window.alert("You selected: " + selected.getName());
		            }
		          }
		        });*/
		        
		     // Add a selection model so we can select cells.
		        final SelectionModel<TableItem> selectionModel =
		            new MultiSelectionModel<TableItem>(KEY_PROVIDER);
		        table.setSelectionModel(selectionModel, DefaultSelectionEventManager
		            .<TableItem> createCheckboxManager());
		        

		        // Set the total row count. This isn't strictly necessary, but it affects
		        // paging calculations, so its good habit to keep the row count up to date.
		        table.setRowCount(tableItems.size(), true);

		        // Push the data into the widget.
		        table.setRowData(0, tableItems);
		
	}

	public CellTable<TableItem> getTable() {
		return table;
	}

	public void setTable(CellTable<TableItem> table) {
		this.table = table;
	}
	
	
	
}
