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

import java.awt.Color;
import java.awt.geom.Rectangle2D;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Observable;
import java.util.Observer;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.apache.batik.dom.GenericDOMImplementation;
import org.apache.batik.svggen.SVGGraphics2D;
import org.apache.log4j.Logger;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartUtilities;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.DatasetRenderingOrder;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.StandardXYItemRenderer;
import org.jfree.chart.renderer.xy.XYAreaRenderer;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.title.LegendTitle;
import org.jfree.chart.title.TextTitle;
import org.jfree.data.general.SeriesChangeEvent;
import org.jfree.data.general.SeriesChangeListener;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.time.TimeSeriesDataItem;
import org.jfree.ui.HorizontalAlignment;
import org.jfree.ui.RectangleInsets;
import org.w3c.dom.DOMImplementation;
import org.w3c.dom.Document;

/**
 * This abstract class provides some methods
 * used by EstimatedQueryProgressGraph, EstimatedProgressGraph
 * and EstimatedTimeGraph
 *
 */
public abstract  class AbstractGraph implements Runnable{
    
    protected JFreeChart chart; 
    protected String filename = null; 
    
    protected TimeSeries seriesOut = null;
    protected TimeSeries seriesIn = null;
    protected double maxDataRange = 0.0;
    
	protected TextTitle lastAddedTextTitle = null;

    private static final Logger logger = Logger.getLogger(AbstractGraph.class);
	protected TimeSeriesCollection dataset2;
	protected TimeSeriesCollection dataset1;
	protected final StandardXYItemRenderer line_renderer;
	protected final XYItemRenderer area_renderer;
   
	/**
	 * Do some basic things for our chart
	 * Like adding the last query to the title
	 */
    public AbstractGraph() {
    	
    	// The query is a observable string, so we can add  
    	// a event listener and got notified if the query changes
		AppCtx.getInstance().getLastQuery().addObserver(new Observer() {
			
			public void update(final Observable o, final Object arg) {
				if(o instanceof ObservableString) {
					
					final ObservableString observableString 
						= (ObservableString) o;
					
					if(lastAddedTextTitle != null) {
						chart.removeSubtitle(lastAddedTextTitle);
					}
			
					lastAddedTextTitle = new TextTitle("for query: " 
							+ observableString.getData());
				
					chart.addSubtitle(lastAddedTextTitle);
				}
			}
		});	
		
        dataset1 = new TimeSeriesCollection();
        dataset2 = new TimeSeriesCollection();
		line_renderer = getLineRenderer();
		area_renderer = getAreaRenderer();
	}
    
    /**
     * Build our chart
     * 
     */
    protected void createChart() {

        if(seriesOut != null) {
            dataset1.addSeries(seriesOut);            
        }
    
        if(seriesIn != null) {
            dataset2.addSeries(seriesIn);
        }
    
        chart = ChartFactory.createTimeSeriesChart( 
                getGraphTitle(), getGraphXAxisLegend(), 
                getGraphYAxisLegend(), 
                dataset1, true, true, false); 
    
        formartChart(); 
    
        final XYPlot plot = (XYPlot) chart.getPlot();
    
        plot.setRenderer(area_renderer);
        plot.setDataset(1, dataset2);
        plot.setRenderer(1, line_renderer);
        formatAxis();
        formatPlot();
    }
    
    /**
     * Exports image as PNG
     * 
     */
    public void exportImage(final String filename) {
        try {
            logger.info("Writing Image to file: " + filename);
            
            ChartUtilities.saveChartAsPNG(new File(filename), chart, 
                    getExportWidth(), getExportHeight());
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    /**
	 * Exports a Chart to a SVG file.
	 * Using the batik framework
	 * 
	 */
	public void exportChartAsSVG(final String filename) {
		
        final DOMImplementation domImpl =
            GenericDOMImplementation.getDOMImplementation();
        final Document document = domImpl.createDocument(null, "svg", null);

        final SVGGraphics2D svgGenerator = new SVGGraphics2D(document);

        final XYPlot plot = (XYPlot) chart.getPlot();
        plot.setForegroundAlpha(1.0f);
        chart.draw(svgGenerator, 
        		new Rectangle2D.Double(0, 0, getExportWidth(), getExportHeight()));

        try {
			final OutputStream outputStream = 
				new FileOutputStream(new File(filename));
			
			final Writer out = new OutputStreamWriter(outputStream, "UTF-8");
			
			svgGenerator.stream(out, false /* don't use css */);						
			outputStream.flush();
			outputStream.close();
		} catch (Exception e) {
            e.printStackTrace();
		} finally {
	        plot.setForegroundAlpha(getAreaAlpha());
		}
	}
    
    public JFreeChart getChart() {
		run();    	
    	return chart;
    }

    protected int getExportHeight() {
        return 350;
    }

    protected int getExportWidth() {
        return 600;
    }

    protected void formartChart() {
        chart.setBackgroundPaint(Color.white);
    }

    /**
     * 
     * Build a area renderer for the chart
     */
    protected XYItemRenderer getAreaRenderer() {
        final XYItemRenderer area_renderer = new XYAreaRenderer();
        final Color color = getAreaRendererDefaultColor();
        area_renderer.setSeriesPaint(0, color);
        area_renderer.setBaseSeriesVisibleInLegend(getAreaVisibleInLegend());
        return area_renderer;
    }

    protected boolean getAreaVisibleInLegend() {
        return true;
    }

    /**
     * Build a line renderer for the chart
     */ 
    protected StandardXYItemRenderer getLineRenderer() {
        final StandardXYItemRenderer line_renderer   
            = new StandardXYItemRenderer(StandardXYItemRenderer.LINES);
        line_renderer.setSeriesPaint(0, Color.RED);
        line_renderer.setBaseSeriesVisibleInLegend(getLineVisibleInLegend());
        return line_renderer;
    }

    protected Color getAreaRendererDefaultColor() {
    	return Color.GREEN;
    }

    protected boolean getLineVisibleInLegend() {
        return true;
    }

    // Format the axis of the chart
    protected void formatAxis() {
        final XYPlot plot = (XYPlot) chart.getPlot();
        
        final DateAxis domainAxis = (DateAxis) plot.getDomainAxis();
        domainAxis.setAutoRange(true);
        domainAxis.setLowerMargin(0.0);
        domainAxis.setUpperMargin(0.0);
        domainAxis.setTickLabelsVisible(true);
        
        domainAxis.setAxisLinePaint(Color.BLACK);
        domainAxis.setLabelPaint(Color.BLACK);
        domainAxis.setTickLabelPaint(Color.BLACK);
        domainAxis.setTickMarkPaint(Color.BLACK);
        domainAxis.setPositiveArrowVisible(true);
        
        final ValueAxis rangeAxis = plot.getRangeAxis();
        rangeAxis.setAxisLinePaint(Color.BLACK);
        rangeAxis.setLabelPaint(Color.BLACK);
        rangeAxis.setTickLabelPaint(Color.BLACK);
        rangeAxis.setTickMarkPaint(Color.BLACK);
        rangeAxis.setPositiveArrowVisible(true);
        
        if(! enableAutoRange() ) {
        	rangeAxis.setAutoRange(false);
        	rangeAxis.setLowerBound(0);
        	rangeAxis.setUpperBound(maxDataRange * 1.01);
        } else {
        	rangeAxis.setAutoRange(true);
        	tickHook(rangeAxis);
        }
    }
    
    /**
     * A hook method, so subclasses can do special formating
     * on the range axis
     */
    protected void tickHook(final ValueAxis rangeAxis) {

    }
    
    protected boolean enableAutoRange() {
    	return false;
    }

    /**
     * Format our Plot
     */
    protected void formatPlot() {
        final XYPlot plot = (XYPlot) chart.getPlot();
        plot.setBackgroundPaint(Color.white);
        
        plot.setDomainGridlinesVisible(true);  
        plot.setRangeGridlinesVisible(true);  
        plot.setRangeGridlinePaint(Color.BLACK);  
        plot.setDomainGridlinePaint(Color.BLACK);   
        plot.setForegroundAlpha(getAreaAlpha());
        
        plot.setDatasetRenderingOrder(DatasetRenderingOrder.FORWARD);
        
        final LegendTitle legend = chart.getLegend();
        legend.setHorizontalAlignment(HorizontalAlignment.RIGHT);
        legend.setMargin(new RectangleInsets(0, 0, 5, 10));
        legend.setBorder(0, 0, 0, 0);
    }

    protected float getAreaAlpha() {
        return 0.80f;
    }

    //===========================================
    // Public
    //===========================================
    public void run() {
        calculateDataSeries();
        calculateOptimalDataSeries();
        createChart();        
    }

    /**
     * Calculate the optimal progression of the data series
     * and draw the optimal progression as a red line into the chart
     */
    protected void calculateOptimalDataSeries() {
    	final AbstractProgressWindow window = 
			 AppCtx.getInstance().getWindow(getWindowTypeForGraph());
		
		// Add Change Listener to progression checkbox
		window.getQueryCheckbox().addChangeListener(new ChangeListener() {
			
			public void stateChanged(ChangeEvent e) {
				
				// Add optimal progression
				if(window.getQueryCheckbox().isSelected()) {
					updateOptimalProgress();
				} else {
					// Otherwise remove progression
					line_renderer.setBaseSeriesVisibleInLegend(false);
					seriesIn.clear();
				} 
			}
		});
		
       // Add Change Listener to add optimal query lines
       seriesOut.addChangeListener(new SeriesChangeListener() {
			
			public void seriesChanged(SeriesChangeEvent arg0) {
				updateOptimalProgress();
			}

		});
	}
    
    /**
     * Draw a optimal query progress line
     * Start: Time0, 
     * End: Last Time,
     * Update on data change, because the last 
     * time entry maybe has changed
     */
	protected void updateOptimalProgress() {
		
		final AbstractProgressWindow window = 
			AppCtx.getInstance().getWindow(getWindowTypeForGraph());
		
		int items = seriesOut.getItemCount();
	
		if(items > 0) {
			// Add optimal Query Progress
			if (window.getQueryCheckbox().isSelected()) {
				
				final TimeSeriesDataItem beginItem 
					= seriesOut.getDataItem(0);
				
				final TimeSeriesDataItem endItem 
					= seriesOut.getDataItem(items - 1);
				
				seriesIn.clear();	
				line_renderer.setBaseSeriesVisibleInLegend(true);
	            
				seriesIn.addOrUpdate(seriesOut.getDataItem(0).getPeriod(), 
	            		getOptimalMinValue(beginItem, endItem));
	            
	            seriesIn.addOrUpdate(endItem.getPeriod(), 
	            		getOptimalMaxValue(beginItem, endItem));
			} 
		}
	}

	public void clearCache() {
        chart = null;
    }
    
    public String getFilename() {
        return filename;
    }

    public void setFilename(final String filename) {
        this.filename = filename;
    }
    
    protected void handleNewTimeValue(final double value) {
        maxDataRange = Math.max(value, maxDataRange);
    }
    
    public TimeSeries getSeriesOut() {
		return seriesOut;
	}

	public TimeSeries getSeriesIn() {
		return seriesIn;
	}

	//=====================================================
    // Abstract Methods
    //=====================================================
    protected abstract String getGraphYAxisLegend();
    protected abstract String getGraphXAxisLegend();
    protected abstract String getGraphTitle();
    protected abstract void calculateDataSeries();
    
	protected abstract double getOptimalMinValue(
			final TimeSeriesDataItem beginItem, 
			final TimeSeriesDataItem endItem);
	
	protected abstract double getOptimalMaxValue(
			final TimeSeriesDataItem beginItem, 
			final TimeSeriesDataItem endItem);
	
	protected abstract WindowType getWindowTypeForGraph();
}
