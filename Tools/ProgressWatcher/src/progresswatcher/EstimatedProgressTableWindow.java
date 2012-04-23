package progresswatcher;

import org.jfree.data.time.TimeSeriesDataItem;

/**
 * Create the table window for the EstimatedProgressTable
 *
 */
public class EstimatedProgressTableWindow extends AbstractTableWindow {

	protected WindowType getWindowType() {
		return WindowType.PROGRESS;
	}

	@Override
	protected String[] getTableHead() {
		return new String[]{"Time (ms)", "Progress", "Opt Progress", "Progress diff"};
	}
	
	protected float calculateOptValue(final long totalTime, final long timestamp,
			final TimeSeriesDataItem item, final TimeSeriesDataItem baseItem, 
			final TimeSeriesDataItem endItem) {
		
		return (float) timestamp / (float) totalTime * 100.0f;
	}
	
	@Override
	protected String getTitle() {
		return "SECONDO - Detailed progress view";
	}	

}