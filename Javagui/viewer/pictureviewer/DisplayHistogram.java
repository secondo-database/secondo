/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[TOC] [\tableofcontents]

[1] Display Histogram: Class Definitions

Dezember 2004 Christian Bohnbuck, Uwe Hartmann, Marion Langen and Holger
M[ue]nx during Prof. G[ue]ting's practical course
'Extensible Database Systems' at Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module is part of a SECONDO viewer ~PictureViewer~ which shows ~picture~ objects and their ~histograms~.


2 Includes and other preparations

*/

package viewer.pictureviewer;

import javax.swing.*;
import javax.swing.table.*;
import gui.*;
import java.awt.event.*;
import javax.swing.event.*;
import tools.*;
import java.io.*;
import java.awt.image.*;

import java.lang.*;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;

/*

3 Class DisplayHistogram

*/

public class DisplayHistogram
extends PictureViewable
{
/*

~histogramBorder~ defines the size of the border around the histogram
within the ~JPanel~.

*/
    static int histogramBorder = 5;

/*

~histogramTickLen~ defines the length of the ticks, which are drawn to
give an idea of the scale of the y-axis.

*/
    static int histogramTickLen = 5;

/*

~data~ is used to store the list representation of the histogram for
each color channel.

*/
    ListExpr data[] = { null, null, null, null };

    AffineTransform af2;

/*

3.1 Constructor ~DisplayHistogram~

This method initialize several dimension variables. Moreover, an AffineTransform 
object is created. Finally, all visible objects are removed.

*/

    public DisplayHistogram()
    {
	af2 = new AffineTransform();
	af2.scale(1, 1);
	removeall();
    }

/*

3.2 Function ~init~

The ~init()~ method removes all references to stored histograms from
the ~data~ array, which will enable callers of methods of this class
to build a fresh histogram from scratch.

*/

    public void init() 
    {
	for (int i = 0; i < 4; i++) data[i] = null;
    }


/*

3.3 Function ~set~

The method ~set()~ stores the ListExpr ~LE~, which contains the 
histogram data, in the array element of ~data~, which corresponds to 
the histogram's channel.

*/

    public void set( ListExpr LE )
    {
	this.setOpaque( false );

	if ( LE.listLength() != 3 )
	{
	    System.out.println( "DisplayHistogram:" +
				"3 Arguments expected: " +
				"1. Channel-Number, 2. Max Value, " +
				"3. List of numbers" );
	    return;
	}

	int channelNo = LE.first().intValue();

	if (channelNo < 0 || channelNo > 3) 
	{
	    System.out.println("Expected channel number between 0 and 3"
			       +" but received "+channelNo);
	    return;
	}

	data[channelNo] = LE;
    }


/*

3.4 Function ~ScanValue~

The function ~ScanValue~ creates a new graph representing a single histogram
channel, as provided in ~LE~, by adding line elements and corresponding colors 
to the vectors ~lines~ and ~lineColors~.

*/

    private void ScanValue( Vector lines, Vector lineColors, ListExpr LE )
    {
/*

Find the right color for the current channel.

*/
	int channelNo = LE.first().intValue();
	Color channelColor;

	if (channelNo == 0)
	    channelColor = Color.RED;
	else if (channelNo == 1)
	    channelColor = Color.GREEN;
	else if (channelNo == 2)
	    channelColor = Color.BLUE;
	else
	    channelColor = Color.WHITE;

/*

Check whether somebody has provided fishy histogram data.

*/	
	if ( LE.third().listLength() != 256 )
	{
	    System.out.println("DisplayHistogram: " + 
			       "No correct histogram expression: "+
			       "256 elements expected!" );
	    return;
	}
		
/*

Find out the current size of the ~JPanel~ and calculate factors ~fx~ and
~fy~, which will be used to scale the histogram data to the current 
~JPanel~ size.

*/
	double max = LE.second().realValue();
		
	Dimension dim = getSize(null);

	double panelWidth = dim.getWidth();
	double panelHeight = dim.getHeight();

	double dataWidth = panelWidth-2*histogramBorder-histogramTickLen;
        double dataHeight = panelHeight-2*histogramBorder-1;

	double fx = dataWidth/256;
	double fy = dataHeight/max;

/*

Draw the x-axis.

*/
	lines.add(
	    new Line2D.Double(
		new Point2D.Double(
		    histogramBorder+histogramTickLen, 
		    histogramBorder+dataHeight+1),
		new Point2D.Double(
		    histogramBorder+histogramTickLen+dataWidth+1, 
		    histogramBorder+dataHeight+1)));
	lineColors.add(Color.BLACK);

/*

Draw the y-axis.

*/	
	lines.add(
	    new Line2D.Double(
		new Point2D.Double(
		    histogramBorder+histogramTickLen, 
		    histogramBorder),
		new Point2D.Double(
		    histogramBorder+histogramTickLen,
		    histogramBorder+dataHeight+1)));
	lineColors.add(Color.BLACK);

/*

Draw the ticks, starting with the lowest one, which are lower than the
histogram's maximum value.

*/
	int ticks[] = { 1, 5, 10, 25, 50 };
	int i;
	for (i = 0; i < ticks.length; i++) 
	    if (ticks[i] <= max) {
		System.out.println(i);
		lines.add(
		    new Line2D.Double(
			new Point2D.Double(
			    histogramBorder,
			    histogramBorder+dataHeight-ticks[i]*fy),
			new Point2D.Double(
			    histogramBorder+histogramTickLen,
			    histogramBorder+dataHeight-ticks[i]*fy)));
		lineColors.add(Color.BLACK);
	    }

/*

Draw a line for each pair of histogram data, which is effectively
creating the well-known graphical representation of histogram data.

*/
	ListExpr p;
	for ( i = 0, p = LE.third(); 
	      i <= 254; 
	      i++, p = p.rest() )
	{
	    double y = p.first().realValue();
	    double ynext = p.rest().first().realValue();
	    
	    lines.add(
	    	new Line2D.Double(
		    new Point2D.Double(
			histogramBorder+histogramTickLen+1+i*fx,
			histogramBorder+dataHeight-y*fy),
		    new Point2D.Double(
			histogramBorder+histogramTickLen+1+(i+1)*fx,
			histogramBorder+dataHeight-ynext*fy)));
	    lineColors.add(channelColor);
	}
    }

/*

3.5 Function ~removeall~

This function removes all visible objects. 

*/
    
    public void removeall()
    {
	setOpaque( false );
	
	for (int i = 0; i < 4; i++) data[i] = null;

	repaint();
    }

/*
 
3.6 Function ~paint~

The function ~paint~ displays the ~histogram~.

*/

    public void paint ( Graphics g)
    {
/*

As the size of the ~JPanel~ may have changed since the last call to this
method, ~ScanValue()~ is called for each color channel to create a neat
histogram graph, which fits into the current ~JPanel~ size.

*/
	Vector lines = new Vector(4*262);
	Vector lineColors = new Vector(4*262);
	for (int i = 0; i < 4; i++) 
	    if (data[i] != null) 
		ScanValue( lines, lineColors, data[i] );

/*

If there is any data to be drawn, draw it. This is pretty straightforward.

*/
	if (  lines == null || lines.size() == 0 ) return;
	
    	Graphics2D g2 = (Graphics2D)g;
		
    	if ( lines.size() >= 1 )
    	{
	    for ( int i=0; i<lines.size(); i++ )
	    {
                Line2D.Double l = (Line2D.Double)lines.get(i);
		Color aktLineColor = (Color) lineColors.get(i);
		g2.setColor(aktLineColor);
                g2.draw(af2.createTransformedShape(l));
	    }
    	}
    }
}


