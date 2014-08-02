//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

package  viewer;

import sj.lang.*;
import gui.SecondoObject;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;

import javax.swing.JLabel;
import javax.swing.JLayeredPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JTextField;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import tools.Reporter;

/**
 * This class represents the viewer to display the data of optics operator.
 * 
 * @author Marius Haug
 *
 */
public class OpticsViewer extends SecondoViewer
{
	private static final long serialVersionUID = 1L;
	
	private OpticsBarChart chart = new OpticsBarChart(this);
	
	private JPanel pnlCtrl = new JPanel();
	
	private JScrollPane sclChart = new JScrollPane(chart);
	
	private JSlider sldZoom = new JSlider(JSlider.HORIZONTAL);
	
	private JTextField txtEps = new JTextField();
	private JLabel lblEps = new JLabel("Eps");

	public OpticsViewer()
	{
		super();
		create();
	}
	
	private void create()
	{
		this.setLayout(new GridBagLayout());
		lblEps.setPreferredSize(new Dimension(35, 20));
		txtEps.setPreferredSize(new Dimension(200, 20));
		txtEps.addFocusListener(new FocusListener() 
		{	
			@Override
			public void focusGained(FocusEvent e){}

			@Override
			public void focusLost(FocusEvent e)
			{
				if(e.getSource() == txtEps)
				{
					String value = txtEps.getText();
					
					if(value != null && !value.trim().equals(""))
					{
						try
						{
							double eps = Double.valueOf(value);
							chart.setEps(eps);
							sclChart.doLayout();
						}
						catch(NumberFormatException ex)
						{
							Reporter.showError("Not a valid number " + ex.getMessage());
						}
					}
				}
			}
		}); 
		
		sldZoom.setPreferredSize(new Dimension(200, 20));
		sldZoom.setMaximum(5);
		sldZoom.setMinimum(1);
		sldZoom.addChangeListener(new ChangeListener() 
		{
			@Override
			public void stateChanged(ChangeEvent e) 
			{
				if(e.getSource() == sldZoom)
				{
					chart.setZoom(sldZoom.getValue());
					sclChart.doLayout();
				}
			}
		});
		
		sldZoom.setValue(1);	
		pnlCtrl.setPreferredSize(new Dimension(200, 25));
		sclChart.getVerticalScrollBar().setUnitIncrement(16);
		sclChart.getHorizontalScrollBar().setUnitIncrement(16);
		
		pnlCtrl.add(lblEps,  new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
		    ,GridBagConstraints.WEST, GridBagConstraints.NONE
		    ,new Insets(0, 0, 0, 0), 0, 0));
		pnlCtrl.add(txtEps,  new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
		    ,GridBagConstraints.WEST, GridBagConstraints.NONE
		    ,new Insets(0, 0, 0, 0), 0, 0));
		pnlCtrl.add(sldZoom, new GridBagConstraints(2, 0, 1, 1, 1.0, 0.0
		    ,GridBagConstraints.WEST, GridBagConstraints.RELATIVE
		    ,new Insets(0, 0, 0, 0), 0, 0));
		
		this.add(pnlCtrl,  new GridBagConstraints(0, 0, 1, 1, 1.0, 0.0
		    ,GridBagConstraints.NORTH, GridBagConstraints.HORIZONTAL
		    ,new Insets(0, 0, 0, 0), 0, 0));
		this.add(sclChart, new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0
		    ,GridBagConstraints.CENTER, GridBagConstraints.BOTH
		    ,new Insets(0, 0, 0, 0), 0, 0));
	}
	
	@Override
	public String getName()
	{
		return "OpticsViewer 0.1";
	}

	@Override
	public boolean addObject(SecondoObject o)
	{
		boolean result = true;
		ListExpr LE = o.toListExpr();
		ListExpr type = LE.first();
		ListExpr value = LE.second();
                  
		// analyse type
		ListExpr maintype = type.first();

		if ( type.listLength() != 2 || !maintype.isAtom() 
		  || maintype.atomType() != ListExpr.SYMBOL_ATOM
		  || !( maintype.symbolValue().equals("rel") 
			  || maintype.symbolValue().equals("mrel") 
			  || maintype.symbolValue().equals("trel") ) )
		{
			Reporter.showError("Not a relation!");
			return false;
		}

		ListExpr tupletype = type.second();

		// analyse Tuple
		ListExpr TupleFirst = tupletype.first();
		
		if ( tupletype.listLength()!= 2 || !TupleFirst.isAtom()
		  || TupleFirst.atomType() != ListExpr.SYMBOL_ATOM
		  || !( TupleFirst.symbolValue().equals("tuple")
			   | TupleFirst.symbolValue().equals("mtuple") ) )
		{
			Reporter.showError("Not a tuple!");
			return false;
		}

		ListExpr TupleTypeValue = tupletype.second();

		ListExpr TupleSubType = TupleTypeValue.first();

		ArrayList<OpticsPoint> alPoints = new ArrayList<OpticsPoint>();

		if (result)
		{
			// analyse the values
			ListExpr TupleValue;
			ListExpr Elem;
			int i = 1;

			while (!value.isEmpty())
			{
				TupleValue = value.first();				
				Elem = TupleValue.third();

				try
				{
					alPoints.add(new OpticsPoint(i++, Elem.realValue()));

				}
				catch(Exception ex)
				{
					System.out.println("NumberFormatException-------- " 
							+ ex.getMessage());
					return false;
				}

	      value = value.rest();
			}
		}

		chart.add(alPoints);
		sclChart.doLayout();
		return true;
	}

	@Override
	public void removeObject(SecondoObject o) 
	{
    chart.clear();
	}

	@Override
	public void removeAll() 
	{
		chart.clear();
	}

	@Override
	public boolean canDisplay(SecondoObject o) 
	{
	  ListExpr LE = o.toListExpr();
		ListExpr type = LE.first();
		ListExpr value = LE.second();
                  
		// analyse type
		ListExpr maintype = type.first();

		if ( type.listLength() != 2 || !maintype.isAtom() 
		  || maintype.atomType() != ListExpr.SYMBOL_ATOM
		  || !( maintype.symbolValue().equals("rel") 
			  || maintype.symbolValue().equals("mrel") 
			  || maintype.symbolValue().equals("trel") ) )
		{
			return false;
		}

		ListExpr tupletype = type.second();

		// analyse Tuple
		ListExpr TupleFirst = tupletype.first();
		
		if ( tupletype.listLength()!= 2 || !TupleFirst.isAtom()
		  || TupleFirst.atomType() != ListExpr.SYMBOL_ATOM
		  || !( TupleFirst.symbolValue().equals("tuple")
			   | TupleFirst.symbolValue().equals("mtuple") ) )
		{
			return false;
		}
		
		ListExpr TupleTypeValue = tupletype.second();

		ListExpr TupleSubType = TupleTypeValue.first();

		ArrayList<OpticsPoint> alPoints = new ArrayList<OpticsPoint>();

		
		// analyse the values
		ListExpr TupleValue;
		ListExpr Elem;
		int i = 1;

	  if(!value.isEmpty())
	  {
	    TupleValue = value.first();				
	  	Elem = TupleValue.third();

	  	try
	  	{
	  		alPoints.add(new OpticsPoint(i++, Elem.realValue()));
	  	}
	  	catch(Exception ex)
	  	{
	  		return false;
	  	}
	  }
	  
		return true;
	}

	@Override
	public boolean isDisplayed(SecondoObject o) 
	{
		return true;
	}

	@Override
	public boolean selectObject(SecondoObject o) 
	{
		return addObject(o);
	}
	
	/**
	 * 
	 * @author Marius Haug
	 *
	 */
	class OpticsPoint implements Comparable<OpticsPoint>
	{
		private int order;
		
		private double reachDist;
		
		public OpticsPoint(int order, double reachDist)
		{
			this.order     = order;
			this.reachDist = reachDist;
		}
		
		public int getOrder()
		{
			return order;
		}
		
		public double getReachDist()
		{
			return reachDist;
		}

		@Override
		public int compareTo(OpticsPoint o)
		{
			if(this.order < o.order)
			{
				return -1;
			}
			else if(this.order > o.order)
			{
				return 1;
			}
			
			return 0;
		}
	}
		
	/**
	 * 
	 * @author Marius Haug
	 *
	 */
	class OpticsBarChart extends JLayeredPane
	{
		private static final long serialVersionUID = 1L;
		
		private static final int OFF_SCL_HEIGTH = 40;	
		private static final int OFF_SCL_WIDTH  = 80;	
		private static final int CS_HEIGHT      = 200;
		private static final int MARK_WIDTH     = 10;
		private static final int TXT_SPACE      = 45;
		private static final int MARK_STEP      = CS_HEIGHT/10;
		private static final int MARK_STEP_PNT  = 10;
		
		private int zoomHeight;
		private int zoomWidth;
		private int zoomOff;
		private int zoomMarkStep;
		private int zoomMarkStepPnt;
		private int zoom   = 1;
		private int points = 100;
		
		private double maxEps = 100;
		
		private boolean initialized = false;
		
		private Dimension dimSize = new Dimension(points, (int) maxEps);
		
		private ArrayList<OpticsPoint> alPoints;
		
		private Color curColor = Color.BLUE;
		
		private OpticsViewer parent;
		
		/**
		 * Constructor of the class.
		 * 
		 * @param parent
		 */
		public OpticsBarChart(OpticsViewer parent)
		{
			super();
			this.parent = parent;
			this.setLayout(new GridBagLayout());
		}
		
		/**
		 * Constructor of the class.
		 * 
		 * @param parent
		 * @param maxEps maximum displayed eps
		 */
		public OpticsBarChart(OpticsViewer parent, double maxEps)
		{
			this(parent);
			this.maxEps = maxEps * zoom;
		}
		
		/**
		 * This method sets the points.
		 * 
		 * @param alPoints
		 */
		public void add(ArrayList<OpticsPoint> alPoints)
		{
			Collections.sort(alPoints);
			this.alPoints = alPoints;
			points = alPoints.size();
			initialized = true;
			repaint();
		}
		/**
		 * This method deletes all points.
		 * 
		 */
		public void clear()
		{
			initialized = false;
			alPoints.clear();
			repaint();
		}
		
		/**
		 * This method sets the zoom factor.
		 * 
		 * @param zoom
		 */
		public void setZoom(int zoom)
		{
			this.zoom = zoom;
			repaint();
		}
		
		/**
		 * This method sets the max eps displayed in the coordinate system.
		 * 
		 * @param eps
		 */
		public void setEps(double eps)
		{
			maxEps = eps;
			repaint();
		}
		
		private void calcStrt()
		{
			zoomWidth       = points * zoom;
			zoomHeight      = CS_HEIGHT * zoom;
			zoomOff         = zoom/2;
			zoomMarkStep    = MARK_STEP * zoom;
			zoomMarkStepPnt = MARK_STEP_PNT * zoom;
			dimSize = new Dimension(OFF_SCL_WIDTH + OFF_SCL_WIDTH + zoomWidth
			                       ,OFF_SCL_HEIGTH + OFF_SCL_HEIGTH + zoomHeight);
		}
		
		public Dimension getPreferredSize()
		{
			return dimSize;
		}
		
		public void paint(Graphics g)
		{
			super.paint(g);
			calcStrt();
			paintBars(g);
			paintCoordinateSystem(g);
		}
		
		/**
		 * This method paints the coordinate system.
		 * 
		 * @param g
		 */
		private void paintCoordinateSystem(Graphics g)
		{
			if(initialized)
			{
				Graphics2D g2 = (Graphics2D) g;
				g2.setColor(Color.RED);
				g2.setStroke(new BasicStroke(2));
				
				//draw max eps line
				g2.drawLine(OFF_SCL_WIDTH + zoomOff
   						     ,OFF_SCL_HEIGTH - zoomOff
						       ,OFF_SCL_WIDTH + zoomOff + zoomWidth + 5
						       ,OFF_SCL_HEIGTH - zoomOff);
				
				g2.setColor(Color.BLACK);
				g2.setStroke(new BasicStroke(1));
				
				double eps = maxEps;
				for(int i = 0; i < 10; i++)
				{	
					g.drawString(Math.round(eps*100d)/100d + ""
					    ,OFF_SCL_WIDTH - MARK_WIDTH - TXT_SPACE
					    ,OFF_SCL_HEIGTH + (zoomMarkStep * i + 5));
					//draw mark lines
					g2.drawLine(OFF_SCL_WIDTH + zoomOff - MARK_WIDTH
							       ,OFF_SCL_HEIGTH + (zoomMarkStep * i)
							       ,OFF_SCL_WIDTH + zoomOff
							       ,OFF_SCL_HEIGTH + (zoomMarkStep * i));
							       
					eps = eps - (maxEps/10);					
				}
				
				int steps = points/MARK_STEP_PNT;
				for(int i = 1; i <= steps; i++)
				{
					//draw mark lines
					g2.drawLine(OFF_SCL_WIDTH +  (zoomMarkStepPnt * i)
							       ,OFF_SCL_HEIGTH + zoomOff + zoomHeight
							       ,OFF_SCL_WIDTH +  (zoomMarkStepPnt * i)
							       ,OFF_SCL_HEIGTH + zoomOff + zoomHeight + MARK_WIDTH);
					
				}				
				
				//draw X-coordinate
				g2.drawLine(OFF_SCL_WIDTH + zoomOff
						       ,OFF_SCL_HEIGTH - 5
						       ,OFF_SCL_WIDTH + zoomOff
						       ,OFF_SCL_HEIGTH + zoomOff + zoomHeight);
				
				//draw Y-coordinate
				g2.drawLine(OFF_SCL_WIDTH + zoomOff
						       ,OFF_SCL_HEIGTH + zoomOff + zoomHeight
						       ,OFF_SCL_WIDTH + zoomOff + zoomWidth + 5
						       ,OFF_SCL_HEIGTH + zoomOff + zoomHeight);
			}
		}
		
		/**
		 * This method paints the points into the coordinate system.
		 * 
		 * @param g
		 */
		private void paintBars(Graphics g)
		{
			if(initialized)
			{
				Graphics2D g2 = (Graphics2D) g;
				g2.setStroke(new BasicStroke(zoom));
				Iterator<OpticsPoint> it = alPoints.iterator();
				
				while(it.hasNext())
				{
					OpticsPoint p = it.next();
					
				  
					int p1X = (int) (OFF_SCL_WIDTH + (p.getOrder() * zoom));
					int p1Y = (int) OFF_SCL_HEIGTH + zoomHeight;
					int p2X = (int) (OFF_SCL_WIDTH + (p.getOrder() * zoom));
					int p2Y = (int) ((p.getReachDist() >= maxEps || p.getReachDist() < 0)
					  ? OFF_SCL_HEIGTH 
					  : OFF_SCL_HEIGTH + zoomHeight - (scale(p.getReachDist()) * zoom));
					
					if(p.getReachDist() < 0)
					{
					  curColor = getNextColor();
					}
					
					g2.setColor(curColor);
					g2.drawLine(p1X, p1Y, p2X, p2Y);
				}
			}
			
			curColor = Color.YELLOW;
		}
		
		private double scale(double d)
		{
			return d/(maxEps/CS_HEIGHT);
		}
		
		private Color getNextColor()
		{
		  if(curColor.equals(Color.BLUE))
		  {
		    return Color.GREEN;
		  }
		  else if(curColor.equals(Color.GREEN))
		  {
		    return Color.YELLOW;
		  }
		  else
		  {
		    return Color.BLUE;
		  }
		}
	}
}



















