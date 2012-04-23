package progresswatcher;

import org.jfree.data.time.TimeSeriesDataItem;

/**
 * Create the table window for the EstimatedCardinalityTable
 *
 */
public class EstimatedCardinalityTableWindow extends AbstractTableWindow {

	protected WindowType getWindowType() {
		return WindowType.CARD;
	}

	@Override
	protected String[] getTableHead() {
		return new String[]{"Time (ms)", "Card", "Opt Card", "Diff"};
	}
	
	protected float calculateOptValue(long totalTime, final long timestamp, 
			final TimeSeriesDataItem item, final TimeSeriesDataItem baseItem, 
		 final TimeSeriesDataItem endItem) {
		return endItem.getValue().floatValue();
	}
	
	@Override
	protected String getTitle() {
		return "SECONDO - Detailed cardinality view";
	}	

}