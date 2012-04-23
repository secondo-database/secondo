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

import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.TickUnitSource;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesDataItem;

/**
 * Draw the Estimated Cardinality Graph
 *
 */
public class EstimatedCardinalityGraph extends AbstractGraph {
	
	protected void calculateDataSeries() {
        seriesOut = new TimeSeries("Estimated Cardinality");
		seriesIn = new TimeSeries("Optimal Cardinality");
	}

	@Override
	protected boolean getLineVisibleInLegend() {
		return false;
	}
	
	@Override
	protected String getGraphTitle() {
		return "Estimated Cardinality";
	}

	@Override
	protected String getGraphXAxisLegend() {
		return "Time";
	}

	@Override
	protected String getGraphYAxisLegend() {
		return "Estimated Cardinality";
	}
	
	@Override
	protected boolean enableAutoRange() {
		return true;
	}
	
	// Only show full ticks and now fractions
	protected void tickHook(org.jfree.chart.axis.ValueAxis rangeAxis) {
	  	TickUnitSource ticks = NumberAxis.createIntegerTickUnits();
		rangeAxis.setStandardTickUnits(ticks);
	}

	@Override
	protected double getOptimalMinValue(final TimeSeriesDataItem beginItem, 
			final TimeSeriesDataItem endItem) {
		return endItem.getValue().doubleValue();
	}

	@Override
	protected double getOptimalMaxValue(final TimeSeriesDataItem beginItem, 
			final TimeSeriesDataItem endItem) {
		return endItem.getValue().doubleValue();
	}

	@Override
	protected WindowType getWindowTypeForGraph() {
		return WindowType.CARD;
	}
}
