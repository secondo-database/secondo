package progresswatcher;

import org.jfree.data.time.TimeSeriesDataItem;

/**
 * Create the table window for the EstimatedTimeTable
 *
 */
public class EstimatedTimeTableWindow extends AbstractTableWindow {

	protected WindowType getWindowType() {
		return WindowType.TIME;
	}

	@Override
	protected String[] getTableHead() {
		return new String[]{"Time (ms)", "Estimated Time", "Opt Time", "Diff"};
	}
	
	protected float calculateOptValue(long totalTime, final long timestamp, 
			final TimeSeriesDataItem item, final TimeSeriesDataItem baseItem, 
			final TimeSeriesDataItem endItem) {
		
		return totalTime;
	}
	
	@Override
	protected String getTitle() {
		return "SECONDO - Detailed time view";
	}	

}