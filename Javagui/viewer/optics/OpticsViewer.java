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

import gui.MainWindow;
import gui.SecondoObject;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.util.ArrayList;
import java.util.Collections;

import javax.swing.DefaultListCellRenderer;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JLayeredPane;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JSplitPane;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * This class represents the viewer to display the data of optics operator.
 * 
 * @author Marius Haug
 *
 */
public class OpticsViewer extends SecondoViewer
                          implements KeyListener
                                    ,ChangeListener
                                    ,ListSelectionListener
                                    ,MouseWheelListener
                                    ,MouseListener
                                    ,MouseMotionListener
                                    ,ActionListener
{
	private static final long serialVersionUID = 1L;
	
	private OpticsBarChart chart = new OpticsBarChart(this);
	
	private JPanel pnlCtrl = new JPanel();
	
	private JList lstPoints = new JList(new OpticsPoint[] {});
	
	private OpticsPointCellRenderer renderer = new OpticsPointCellRenderer();
	
	private JScrollPane sclChart  = new JScrollPane(chart);
	private JScrollPane sclPoints = new JScrollPane(lstPoints);
	
	private JSplitPane splView = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
	
	private JSlider sldZoom = new JSlider(JSlider.HORIZONTAL);
	
	private JTextField txtEps = new JTextField();
	
	private JLabel lblEps = new JLabel("Eps");
	
	private JButton btnToHeose = new JButton("Add to Hoese");
	
	private SecondoObject secObj = null;
	
	private MainWindow parent = null;

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
		txtEps.addKeyListener(this);
		txtEps.setFocusTraversalKeysEnabled(false);
		txtEps.setToolTipText("Confirm the new eps with TAB or ENTER");
		
		sldZoom.setPreferredSize(new Dimension(200, 20));
		sldZoom.setMaximum(5);
		sldZoom.setMinimum(1);
		sldZoom.addChangeListener(this);
		sldZoom.setValue(1);	
		sldZoom.addMouseWheelListener(this);
		
		btnToHeose.setPreferredSize(new Dimension(140, 20));
		btnToHeose.addActionListener(this);
		
		sclChart.getVerticalScrollBar().setUnitIncrement(16);
		sclChart.getHorizontalScrollBar().setUnitIncrement(16);
		
		lstPoints.setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		lstPoints.setCellRenderer(renderer);
		lstPoints.addListSelectionListener(this);
		
		sclPoints.getVerticalScrollBar().setUnitIncrement(16);
		sclPoints.getHorizontalScrollBar().setUnitIncrement(16);
		
		chart.addMouseListener(this);
		chart.addMouseMotionListener(this);
		
		splView.add(sclPoints, JSplitPane.LEFT);
		splView.add(sclChart, JSplitPane.RIGHT);
		
		pnlCtrl.setPreferredSize(new Dimension(600, 25));
		
		pnlCtrl.add(lblEps,     new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
		  ,GridBagConstraints.WEST, GridBagConstraints.NONE
		  ,new Insets(0, 0, 0, 0), 0, 0));
		pnlCtrl.add(txtEps,     new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
		  ,GridBagConstraints.WEST, GridBagConstraints.NONE
		  ,new Insets(0, 0, 0, 0), 0, 0));
		pnlCtrl.add(sldZoom,    new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0
		  ,GridBagConstraints.WEST, GridBagConstraints.NONE
		  ,new Insets(0, 0, 0, 0), 0, 0));
		pnlCtrl.add(btnToHeose, new GridBagConstraints(3, 0, 1, 1, 1.0, 0.0
		  ,GridBagConstraints.WEST, GridBagConstraints.RELATIVE
		  ,new Insets(0, 0, 0, 0), 0, 0));
		
		this.add(pnlCtrl, new GridBagConstraints(0, 0, 3, 1, 1.0, 0.0
		  ,GridBagConstraints.NORTH, GridBagConstraints.HORIZONTAL
		  ,new Insets(0, 0, 0, 0), 0, 0));
		this.add(splView, new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0
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
		secObj = o;
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
			ListExpr coreDist;
			ListExpr Elem;
			int i = 1;
			
			while (!value.isEmpty())
			{
				TupleValue = value.first();				
				coreDist = TupleValue.second();
	    	Elem = TupleValue.third();

	    	try
	    	{
	    		alPoints.add(new OpticsPoint(i++, Elem.realValue()
	    		                            ,coreDist.realValue()));
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
		lstPoints.setListData(alPoints.toArray());
		lstPoints.setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		lstPoints.revalidate();
		sclChart.doLayout();
		
		if(this.getParent().getParent().getParent().getParent().getParent() 
		  instanceof MainWindow )
		{
		  parent = ((MainWindow)this.getParent().getParent().getParent()
		  .getParent().getParent());
		}
		
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
		ListExpr coreDist;
		ListExpr Elem;
		int i = 1;

	  if(!value.isEmpty())
	  {
	    TupleValue = value.first();
	  	coreDist = TupleValue.second();
	  	Elem = TupleValue.third();

	  	try
	  	{
	  		alPoints.add(new OpticsPoint(i++, Elem.realValue(), coreDist.realValue()));
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
	
	@Override
	public void keyPressed(KeyEvent e) {}

	@Override
	public void keyReleased(KeyEvent e)
	{
		if( e.getSource() == txtEps
		 && ( e.getKeyCode() == KeyEvent.VK_TAB 
		   || e.getKeyCode() == KeyEvent.VK_ENTER ) )
		{
			String value = txtEps.getText();
			
			if(value != null && !value.trim().equals(""))
			{
				try
				{
//					test();
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

	@Override
	public void keyTyped(KeyEvent e) {}
	
	@Override
	public void valueChanged(ListSelectionEvent e) 
	{
		if(e.getSource() == lstPoints)
		{
			chart.setSelected(((OpticsPoint) 
			 ((JList) e.getSource()).getSelectedValue()).order);
		}
	}

	@Override
	public void stateChanged(ChangeEvent e) 
	{
		if(e.getSource() == sldZoom)
		{
			chart.setZoom(sldZoom.getValue());
			sclChart.doLayout();
		}
	}

	@Override
	public void mouseClicked(MouseEvent e) 
	{
		if(e.getSource() == chart)
		{
			int idx = chart.getSelected(e.getX(), e.getY());
			if(idx > -1)
			{
				lstPoints.setSelectedIndex(idx);
				lstPoints.ensureIndexIsVisible(idx);
			}
		}
	}

	@Override
	public void mouseWheelMoved(MouseWheelEvent e)
	{
		if(e.getSource() == sldZoom)
		{
			sldZoom.setValue(sldZoom.getValue() + e.getWheelRotation());
		}
	}
	
	@Override
	public void mouseEntered(MouseEvent e) {}

	@Override
	public void mouseExited(MouseEvent e) {}

	@Override
	public void mousePressed(MouseEvent e) {}

	@Override
	public void mouseReleased(MouseEvent e)
	{
		if(e.getSource() == chart)
		{
			chart.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
		}
	}
	
	@Override
	public void mouseDragged(MouseEvent e) 
	{
		if(e.getSource() == chart)
		{
			if(chart.grabbed)
			{
				chart.setCursor(new Cursor(Cursor.MOVE_CURSOR));
				chart.setEpsLine(e.getY());
			}
		}
	}

	@Override
	public void mouseMoved(MouseEvent e) 
	{
		if(e.getSource() == chart)
		{
			if(chart.epsLineFocused(e.getX(), e.getY()))
			{
				chart.setCursor(new Cursor(Cursor.HAND_CURSOR));
			}
			else
			{
				chart.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
			}
		}
	}
	
	@Override
	public void actionPerformed(ActionEvent e) 
	{
		if(e.getSource() == btnToHeose)
		{
			actionPerformed_btnToHoese(e); 
		}
	}
	
	private void actionPerformed_btnToHoese(ActionEvent e)
	{
	  if(parent != null)
	  {
		  ListExpr LE = secObj.toListExpr();
		  ListExpr type = LE.first();
		  ListExpr value = LE.second();
		
		  ListExpr choice = null;
		  ListExpr points = null;
		
		  int i = 0;
		  ListExpr last = null;
		
		  do
		  {
			  ListExpr tmp = value.first();
			
			  if(chart.isSelected(i))
			  {
				  if(points == null)
				  {
					  points = ListExpr.oneElemList(tmp);
					  choice = ListExpr.twoElemList(type, points);
					  last = points;
				  }
				  else
				  {
					  last = ListExpr.append(last, tmp);
				  }
			  }
			
			  value = value.rest();
			  i++;
		  } while(!value.isEmpty());
		
		  SecondoObject obj = new SecondoObject("Optics choice", choice);
		
		  parent.displayAt("Hoese-Viewer", obj);
		}
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
		private double coreDist;
		
		public OpticsPoint(int order, double reachDist, double coreDist)
		{
			this.order     = order;
			this.reachDist = reachDist;
			this.coreDist  = coreDist;
		}
		
		public int getOrder()
		{
			return order;
		}
		
		public double getReachDist()
		{
			return reachDist;
		}

		public double getCoreDist()
		{
			return coreDist;
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
	class OpticsPointCellRenderer extends DefaultListCellRenderer
	{
		private static final long serialVersionUID = 1L;

		@Override
		public Component getListCellRendererComponent(JList list, Object value
		  ,int index, boolean isSelected, boolean cellHasFocus)
		{
			JLabel lbl = (JLabel) super.getListCellRendererComponent(list, value
			  ,index, isSelected, cellHasFocus);
			
			OpticsPoint point = (OpticsPoint) value;
			
			lbl.setText("<html>Reach distance " + point.reachDist  
					       + "<br/>Core distance " + point.coreDist
					       + "<br/>Order " + point.order + "</html>");
			
			return lbl;
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
		private int selIdx = -1;
		private int points = 100;
		private int OFF_EPS_LINE = 0;
		private int epsLineOff = 0;
		
		private double maxEps = 100;
		
		private boolean initialized = false;
		private boolean grabbed = false;
		
		private Dimension dimSize = new Dimension(points, (int) maxEps);
		
		private ArrayList<OpticsPoint> alPoints;
		
		private Color curColor = Color.BLUE;
		
		private Rectangle epsLine = new Rectangle();
		private Rectangle epsLineCalc = new Rectangle();
		
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
		 * Sets the selected bar in the chart.
		 * 
		 * @param selPoint
		 */
		public void setSelected(int selPoint)
		{
			this.selIdx = selPoint-1;
			
			Point p = sclChart.getViewport().getViewPosition();
			Dimension d = sclChart.getViewport().getSize();
			int p1X = (int) (OFF_SCL_WIDTH + (zoom* selPoint));
			
			
			
			if(p.x + d.width < p1X + OFF_SCL_WIDTH)
			{
				p.x = p1X  +OFF_SCL_WIDTH - d.width;
				
				if(p.x + d.width > (OFF_SCL_WIDTH + (points * zoom)))
				{
					p.x = (OFF_SCL_WIDTH + (points * zoom));
				}
			}
			else if(p.x > p1X  -OFF_SCL_WIDTH)
			{
				p.x = p1X - OFF_SCL_WIDTH;

				if(p.x < 0)
				{
					p.x = 0;
				}
			}
			
			sclChart.getViewport().setViewPosition(p);
			
			repaint();
		}
		
		/**
		 * Returns the index of the bar in the chart.
		 * 
		 * @param x
		 * @param y
		 * @return
		 */
		public int getSelected(int x, int y)
		{
			int i = ((x - OFF_SCL_WIDTH)/zoom) -1;
			
			if(i > -1)
			{
				OpticsPoint p = alPoints.get(i);
				
				int p1Y = (int) OFF_SCL_HEIGTH + zoomHeight;
				int p2Y = (int) ((p.getReachDist() >= maxEps || p.getReachDist() < 0)
				  ? OFF_SCL_HEIGTH 
				  : OFF_SCL_HEIGTH + zoomHeight - (scale(p.getReachDist()) * zoom));
				
				if(p1Y >= y && p2Y <= y)
				{
					return i;
				}
			}
			
			return -1;
		}
		
		public boolean epsLineFocused(int x, int y)
		{
			if(epsLine.contains(x, y))
			{
				if(!grabbed)
				{
					grabbed = true;
					repaint();
				}
			}
			else
			{
				if(grabbed)
				{
					grabbed = false;
					repaint();
				}
			}
			
			return grabbed;
		}
		
		public void setEpsLine(int x)
		{
			OFF_EPS_LINE = (x - OFF_SCL_HEIGTH)/zoom;
			calcStrt();
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
			epsLineOff      = OFF_EPS_LINE * zoom;
			
			if(epsLineOff < 0)
			{
				epsLineOff = 0;
			}
			else if(epsLineOff > zoomOff + zoomHeight)
			{
				epsLineOff = zoomOff + zoomHeight;
			}
			
			dimSize = new Dimension(OFF_SCL_WIDTH + OFF_SCL_WIDTH + zoomWidth
			                       ,OFF_SCL_HEIGTH + OFF_SCL_HEIGTH + zoomHeight);
			
			epsLine.setBounds(OFF_SCL_WIDTH + zoomOff - 5
			           	     ,OFF_SCL_HEIGTH - 5 + epsLineOff
				                ,zoomOff + zoomWidth + 5 + 10
				                ,zoomOff + 10);
			
			epsLineCalc.setBounds(OFF_SCL_WIDTH + zoomOff
				                    ,OFF_SCL_HEIGTH + epsLineOff
				                    ,zoomOff + zoomWidth
				                    ,zoomOff + 1);
		}
		
		@Override
		public Dimension getPreferredSize()
		{
			return dimSize;
		}
		
		@Override
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
				
				g2.setColor(Color.BLACK);
				g2.setStroke(new BasicStroke(2));
				
				//draw max eps line
				g2.drawLine(OFF_SCL_WIDTH + zoomOff
   						      ,OFF_SCL_HEIGTH  + epsLineOff
						         ,OFF_SCL_WIDTH + zoomOff + zoomWidth + 5
						         ,OFF_SCL_HEIGTH  + epsLineOff);
				
				if(grabbed)
				{
					g2.setColor(Color.RED);
					g2.setStroke(new BasicStroke(2));
					g2.drawRect(OFF_SCL_WIDTH + zoomOff - 2
	   						      ,OFF_SCL_HEIGTH - 2 + epsLineOff
							         ,zoomOff + zoomWidth + 5 + 2
							         ,zoomOff + 2);
				}
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
				
				for(int i = 0; i < points; i++)
				{
					OpticsPoint p = alPoints.get(i);//it.next();
					
					int p1X = (int) (OFF_SCL_WIDTH + (p.getOrder() * zoom));
					int p1Y = (int) OFF_SCL_HEIGTH + zoomHeight;
					int p2X = (int) (OFF_SCL_WIDTH + (p.getOrder() * zoom));
					int p2Y = (int) ((p.getReachDist() >= maxEps || p.getReachDist() < 0)
					  ? OFF_SCL_HEIGTH 
					  : OFF_SCL_HEIGTH + zoomHeight - (scale(p.getReachDist()) * zoom));
					
					if(epsLineCalc.intersects(p2X, p2Y, p1X - OFF_SCL_WIDTH
					                         ,p1Y - OFF_SCL_HEIGTH)
							|| p.getReachDist() < 0)
					{
						curColor = Color.LIGHT_GRAY;
					}
					else
					{
						curColor = Color.BLUE;
					}
					
//					if(p.getReachDist() < 0)
//					{
//					  curColor = getNextColor();
//					}

					if(i == selIdx)
					{
						g2.setColor(Color.RED);
					}
					else
					{
						g2.setColor(curColor);
					}					
          
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
		
		public boolean isSelected(int i)
		{
			OpticsPoint p = alPoints.get(i);
			
			int p1X = (int) (OFF_SCL_WIDTH + (p.getOrder() * zoom));
			int p1Y = (int) OFF_SCL_HEIGTH + zoomHeight;
			int p2X = (int) (OFF_SCL_WIDTH + (p.getOrder() * zoom));
			int p2Y = (int) ((p.getReachDist() >= maxEps || p.getReachDist() < 0)
			  ? OFF_SCL_HEIGTH 
			  : OFF_SCL_HEIGTH + zoomHeight - (scale(p.getReachDist()) * zoom));
			boolean cl = !(epsLineCalc.intersects(p2X, p2Y
			                                     ,p1X - OFF_SCL_WIDTH
			                                     ,p1Y - OFF_SCL_HEIGTH)
					|| p.getReachDist() < 0);
			return cl;// || (p.getReachDist() < 0 && p.getCoreDist() > 0);
		}
	}





















	
	private void test()
	{
		ArrayList<OpticsPoint> alPoints = new ArrayList<OpticsPoint>();
		
		int a = 100;
		int s = -1;
		
		for(int i = 1; i < 1000;)
		{
			a += s;
			alPoints.add(new OpticsPoint(i++, a, 99));
			
			if(a == 50)
			{
				alPoints.add(new OpticsPoint(i++, -1, 99));
				alPoints.add(new OpticsPoint(i++, a, 99));
				alPoints.add(new OpticsPoint(i++, a, 99));
				alPoints.add(new OpticsPoint(i++, a, 99));
				s = s * -1;
			}
			else if(a == 100)
			{
				s = s * -1;
			}
		}
		
		chart.add(alPoints);
		lstPoints.setListData(alPoints.toArray());
		lstPoints.setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		lstPoints.revalidate();
		sclChart.doLayout();
	}
}



















