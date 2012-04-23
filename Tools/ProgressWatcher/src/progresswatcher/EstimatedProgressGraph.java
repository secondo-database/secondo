//This file is part of SECONDO.

//Copyright (C) 2006, University in Hagen, Department of Computer Science, 
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

// Written 2012 by Jan Kristof Nidzwetzki 

package progresswatcher;

import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesDataItem;

/**
 * Draw the Estimated Progress Graph
 *
 */
public class EstimatedProgressGraph extends AbstractGraph {
	
	public EstimatedProgressGraph() {
    	maxDataRange = 100; // 100 %
	}
	
	protected double getOptimalMinValue(final TimeSeriesDataItem beginItem, 
			final TimeSeriesDataItem endItem) {
		
		return 0;
	}
	
	protected double getOptimalMaxValue(final TimeSeriesDataItem beginItem, 
			final TimeSeriesDataItem endItem) {
		
		return 100;
	}
	
	protected WindowType getWindowTypeForGraph() {
		return WindowType.PROGRESS;
	}
	
	protected void calculateDataSeries() {
        seriesOut = new TimeSeries("Query Progress");        
		seriesIn = new TimeSeries("Optimal Progress");
	}

	@Override
	protected boolean getLineVisibleInLegend() {
		return false;
	}
	
	@Override
	protected String getGraphTitle() {
		return "Estimated Progress";
	}

	@Override
	protected String getGraphXAxisLegend() {
		return "Time";
	}

	@Override
	protected String getGraphYAxisLegend() {
		return "Percent";
	}
}
