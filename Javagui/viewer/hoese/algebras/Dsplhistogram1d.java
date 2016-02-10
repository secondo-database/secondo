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

package viewer.hoese.algebras;

import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.awt.geom.*;
import java.awt.*;
import java.lang.Math;
import sj.lang.ListExpr;
import java.util.*;

import javax.swing.JFrame;
import javax.swing.JPanel;

import viewer.hoese.*;
import tools.Reporter;
import java.awt.Graphics;

import javax.swing.*;
import java.awt.event.*;


/**
 * A Displayclass for an histogram1d from the histogram1d-algebra
 *
 */
public class Dsplhistogram1d extends DsplGeneric implements  DisplayComplex, ExternDisplay
{

	 /** Creates a new Instance of this. */
	public  Dsplhistogram1d(){

	}
	/** The internal datatype representation */
	static ExtWin extWin = new ExtWin();

	boolean err;
        boolean undef = false;

	static double[] rangesVec = null;
	static double[] binsVec = null;
	static double[] rangesVecOld = null;
	static double[] binsVecOld = null;
	double[] binsHeight = null;

   /** Returns null. */
   public Shape getRenderObject(int no, AffineTransform af){
	   return null; //getHistBounds();
   }

   /**
    * must be implemented
    */
   public void draw(Graphics g, double time, AffineTransform af){
   }

   /** Returns 1. */
   public int numberOfShapes(){
	   return 1;
   }
   /** Returns false. */
   public boolean isPointType(int no){
	   return false;
   }
   /** Returns false. */
   public boolean isLineType(int no){
	   return false;
   }



   /**
    * Scans the numeric representation of a histogram1d datatype
    *
    * @param v
    *            the nestedlist representation of the histogram
    *            ((range*)(bin*))
    */
   protected void ScanValue(ListExpr v)
   {
	   int i = 0; //to count through ranges and bins
	   int j = 0;
	   int c = 0;

           if ( isUndefined(v) ){
              undef = true;
              return;
           }

           if(v.listLength()!=2){
              Reporter.writeError("ListLength of the graph type must be two.");
              err=true;
              return;
	   }

	   ListExpr ranges = v.first();
	   ListExpr bins = v.second();
	   ListExpr rangesRest = ranges;
	   ListExpr binsRest = bins;
	   ListExpr rangesCount = ranges;

	   if(ranges.listLength()!= ( bins.listLength()+1) ){
		   Reporter.writeError("Incorrect data format!");
		   err=true;
		   return;
	   }

	   while (!rangesCount.isEmpty())
	   {
		   c++;
		   rangesCount = rangesCount.rest();
	   }
	   this.rangesVecOld = rangesVec; //copy of ranges and bins to hold old data for second panel
	   this.binsVecOld = binsVec;
	   rangesVec = new double[c];
	   binsVec = new double[c-1];

	   while (!rangesRest.isEmpty())
	   {
		   ListExpr range = rangesRest.first();

		   if(rangesRest.first().atomType()!=ListExpr.REAL_ATOM){
			   Reporter.writeError("invalid representation of a range found");
			   return;
		   }

		   double rangeD = range.realValue();
//		   System.out.println(" rangeD: " + rangeD);
		   rangesVec[i] = rangeD;
		   rangesRest = rangesRest.rest();
		   i++;
	   }
	   while (!binsRest.isEmpty())
	   {
		   ListExpr bin = binsRest.first();
		   if( binsRest.first().atomType()!=ListExpr.REAL_ATOM ){

			   Reporter.writeError("invalid representation of a bin found");
			   err=true;
			   return;
		   }
		   double binD = bin.realValue();
//		   System.out.println(" binD: " + binD);
		   binsVec[j] = binD;
		   binsRest = binsRest.rest();
		   j++;
	   }
//	   System.out.println("ich bin durch im ScanValue!");

   }

	/**
	 * Init. the Dsplhistogram1d instance.
	 *
	 * @param type
	 *            The symbol histogram1d
	 * @param value
	 *            the nestedlist representation of the histogram1d
	 * @param qr
	 *            queryresult to display output.
	 */
  public void init(String name, int nameLength, int indent,  ListExpr type, ListExpr value, QueryResult qr){
    AttrName = extendString(name, nameLength, indent);
 		ScanValue(value);
                if (undef)
                {
                  qr.addEntry(new String( AttrName + ": undefined"));
                  return;
                }

                if (err){
			Reporter.writeError("Error in ListExpr :parsing aborted");
			qr.addEntry(new String("(" + AttrName + ": GA(histogram1d))"));
			return;
		} else{
			qr.addEntry(this);
//			extWin.setVisible(true);   //!!!GLEICH AM ANFANG FENSTER OFFEN
		}
	}


	/**
	 * shows this histogram in an external window
	 */
	public void displayExtern(){
		extWin.setSource(this);
		extWin.setVisible(true);
	}

	public boolean isExternDisplayed(){
		return this==extWin.hist && extWin.isVisible();
	}

	//###########################  E X T E R N   W I N D O W  #############################################

	/**
	 * to show the histograms in an external window
	 */
	private static class ExtWin extends JFrame  {
		private HistogramPanel myPan1;
		private HistogramPanel myPan2;
		Dsplhistogram1d hist = null;

		static double[] rangesVec = null;
		static double[] binsVec = null;
		static double[] rangesVecOld = null;
		static double[] binsVecOld = null;
    JButton saveBtn1 = new JButton("Save left");
    JButton saveBtn2 = new JButton("Save right");
    JButton saveBtn3 = new JButton("Save both");
    JPanel p1 = new JPanel(new GridLayout(1,2));

		/** creates a new external window **/
		public ExtWin(){

			super("Externes Histogramm");
			this.myPan1 = new HistogramPanel();
			this.myPan2 = new HistogramPanel();
			this.myPan1.setMainColor(new Color(66, 149, 210));
			this.myPan2.setMainColor(new Color(66, 149, 210));
      this.myPan1.setBorderColor(Color.WHITE);
      this.myPan2.setBorderColor(Color.WHITE);

			this.myPan1.setKeyListener();
			this.myPan2.setKeyListener();

      myPan1.setOpaque(true);
      myPan2.setOpaque(true);
      p1.setOpaque(true);

			this.getContentPane().setLayout(new BorderLayout());

			p1.add(myPan1);
			p1.add(myPan2);
      this.getContentPane().add(p1,BorderLayout.CENTER);

      JPanel controlPanel = new JPanel();
      controlPanel.add(saveBtn1);
      controlPanel.add(saveBtn2);
      controlPanel.add(saveBtn3);

      this.getContentPane().add(controlPanel,BorderLayout.SOUTH);

      ActionListener saveListener = new ActionListener(){
        public void actionPerformed(ActionEvent evt){
          JFileChooser fc = new JFileChooser(".");
          if(fc.showSaveDialog(null)==JFileChooser.APPROVE_OPTION){
            JComponent toSave = null;
            Object obj = evt.getSource();
            Color borderColor = myPan1.getBorderColor();
            Color bg = myPan1.getBackground();

            if(obj.equals(saveBtn1) ){
              toSave = myPan1;
            } else  if(obj.equals(saveBtn2)){
              toSave = myPan2;
            } else if(obj.equals(saveBtn3)){
              toSave = p1;
            }

            p1.setBackground(Color.WHITE);
            myPan1.setBackground(Color.WHITE);
            myPan2.setBackground(Color.WHITE);
            toSave.setOpaque(true);
            myPan1.setBorderColor(Color.BLACK);
            myPan2.setBorderColor(Color.BLACK);
            toSave.repaint();


            extern.psexport.PSCreator.export(toSave,fc.getSelectedFile());

            myPan1.setBorderColor(borderColor);
            myPan2.setBorderColor(borderColor);
            p1.setBackground(bg);
            myPan1.setBackground(bg);
            myPan2.setBackground(bg);
          }
        }
      };
      saveBtn1.addActionListener(saveListener);
      saveBtn2.addActionListener(saveListener);
      saveBtn3.addActionListener(saveListener);




			setSize(600,300);
			super.validate();

		}

		public void setSource(Dsplhistogram1d hist){
			this.hist = hist;
			this.myPan1.setHistogram(hist.rangesVec, hist.binsVec);
			this.myPan2.setHistogram(hist.rangesVecOld, hist.binsVecOld);
		}
	}

	//##################   H I S T O G R A M   P A N E L  ##########################################

	/**
	 *
	 * forms a panel for a histogram/home/fp0708/
	 */
	private static class HistogramPanel extends JPanel{

		double[] rangesVecPaint = null;
		double[] binsVecPaint = null;
		double[] binsHeightPaint = null;
		private Color mainColor = Color.white;
		private Color selectedColor = new Color(97,190,240);
    private Color borderColor = Color.WHITE;

		int selectedBin = -1; //to identify the accented column

		public HistogramPanel(){
			MouseNavigator mouseNavi = new MouseNavigator();
			this.addMouseListener(mouseNavi);
			this.addMouseMotionListener(mouseNavi);
		}


		/**
		 * sets the data of a histogram
		 *
		 * @param rangesVecPaint
		 * @param binsVecPaint
		 */
		public void setHistogram(double[] rangesVecPaint, double[] binsVecPaint){
			this.selectedBin = -1;
			this.rangesVecPaint = rangesVecPaint;
			this.binsVecPaint = binsVecPaint;
			if (this.rangesVecPaint == null || this.binsVecPaint == null){
				this.binsHeightPaint = null;
			}
			else{
				this.binsHeightPaint = this.getBinHeights(rangesVecPaint, binsVecPaint);
			}
		}

		/**
		 * sets the color of the columns
		 * @param mainColor
		 */
		public void setMainColor(Color mainColor){
			this.mainColor = mainColor;
		}

		/**
		 * sets the color of the accented column
		 *
		 * @param selectedColor
		 */
		public void setSelectedColor(Color selectedColor){
			this.selectedColor = selectedColor;
		}

    public Color getSelectedColor(){
      return this.selectedColor;
    }
    public void setBorderColor(Color borderColor){
      this.borderColor = borderColor;
    }

    public Color getBorderColor(){
      return this.borderColor;
    }
		/**
		 *
		 * @param ranges
		 * @param binAreas
		 * @return The computed heights of the columns
		 */
		private double[] getBinHeights(double[] ranges, double[] binAreas)
		{
			double[] out = new double[binAreas.length];
			for (int i = 0; i < binAreas.length; i++){
				out[i] = binAreas[i]/(ranges[i+1] - ranges[i]);
			}
			return out;
		}

		/**
		 * Gets the bound rectangle of the graph by creating a union of the bounds
		 * of both axis
		 *
		 * @return The bound rectangle of the histogram
		 */
		public Rectangle2D.Double getHistBounds()
		{
//			   System.out.println("ich bin im getHistBounds1!");
			double maxY = getMax(this.binsHeightPaint);
			double minY = getMin(this.binsHeightPaint);
			double minX = rangesVecPaint[0];
			double maxX = rangesVecPaint[rangesVecPaint.length-1];

//			System.out.println("maxY: "+maxY);
//			System.out.println("minY: "+minY);
//			System.out.println("maxX: "+maxX);
//			System.out.println("minX: "+minX);

			if (maxX < 0.0){
				maxX = 0.0;
			}
			if(maxY < 0.0){
				maxY = 0.0;
			}

			if(minY < 0.0){
				return new Rectangle2D.Double(minX, minY, maxX-minX, maxY-minY);
			}
			else
				return new Rectangle2D.Double(minX, minY, maxX-minX, maxY);
		}

		/**
		 *
		 * @param values
		 * @return The maximum of an array of doubles
		 */
		private double getMax(double[] values){
			double out = values[0];
			if (values.length == 1){
				return out;
			}
			for (int i = 1; i < values.length; i++){
				if (values[i] > out){
					out = values[i];
				}
			}
			return out;
		}

		/**
		 *
		 * @param values
		 * @return The minimum of an array of doubles
		 */
		private double getMin(double[] values){
			double out = values[0];
			if (values.length == 1){
				return out;
			}
			for (int i = 1; i < values.length; i++){
				if (values[i] < out){
					out = values[i];
				}
			}
			return out;
		}

		/**
		 *
		 * @param width
		 * @param height
		 * @param panSizeX
		 * @param panSizeY
		 * @return The units for x- and y-axis for painting
		 */
		private Point2D.Double getPaintUnits(double width, double height, double panSizeX, double panSizeY){
			if(width == 0.0){
				width = 200;
			}
			if (height == 0.0){
				height = 200;
			}
			return new Point2D.Double(panSizeX/width, panSizeY/height);
		}

		/**
		 * Draws the histogram in the panel.
		 */
		public void paintComponent(Graphics g){
      super.paintComponent(g);
//			   System.out.println("ich bin im paintComponents 1!");
      g.setColor(this.getBackground());
			g.fillRect(0, 0, this.getWidth(), this.getHeight());
//			   System.out.println("ich bin im paintComponents 2!");
			this.drawOneHistogram(g, 0,
					rangesVecPaint, binsVecPaint, this.mainColor );
//			   System.out.println("ich bin im paintComponents 3!");
		};


		/**
		 *
		 * @param g
		 * @param time
		 * @param _rangesVec
		 * @param _binsVec
		 * @param mainColor
		 */
		public void drawOneHistogram(Graphics g, double time,  double[] _rangesVec, double[] _binsVec, Color mainColor){
//			   System.out.println("ich bin im drawOneHistogram 1!");

			double binSum=0.0;
			double freq = 0.0; //relative frequency
			if (_rangesVec == null || _binsVec == null){
				return;
			}

			Rectangle2D rect = getHistBounds();
//			System.out.println("rect.getWidth: "+rect.getWidth());
//			System.out.println("rect.getHeight: "+rect.getHeight());
			if(rect==null){
				return;
			}
			//units for better painting
			Point2D.Double paintUnits = this.getPaintUnits(rect.getWidth(), rect.getHeight(), this.getWidth()-100, this.getHeight()-100);
			double unitX = paintUnits.x;
			double unitY = paintUnits.y;

			Graphics2D gfx = (Graphics2D)g;


//			   System.out.println("ich bin im drawOneHistogram 2!");

			for (int bin=0; bin <_binsVec.length; bin++){
//				System.out.println("_binsVec["+bin+"]: " + _binsVec[bin]);
				if (binSum >= 0.0){
					binSum = binSum+_binsVec[bin];
				}
			}
//			System.out.println("****binSum: "+binSum);


			double sumLength = 0.0; //length of total ranges
			if(_rangesVec[0] < 0.0)
			{
				sumLength = _rangesVec[0];
			}
			else
			{
				sumLength = 0.0;
			}

//			   System.out.println("ich bin im drawOneHistogram 2!");

			double xAxisBegin; //where to paint the beginning of the axis
			double yAxisBegin;

			int myNullX; //my point of origin for the coordinate system
			int myNullY;

			if (_rangesVec[0] > 0.0){
				xAxisBegin = 0.0;
			}
			else xAxisBegin = _rangesVec[0];

			if (getMin(this.binsHeightPaint) > 0.0){
				yAxisBegin = 0.0;
			}
			else yAxisBegin = getMin(this.binsHeightPaint);

			myNullX = (int)(50- xAxisBegin*unitX);
			if (getMax(this.binsHeightPaint) == 0.0 && getMin(this.binsHeightPaint) == 0.0){
			//	myNullY = (int)(500*unitY);
				myNullY = (int)(200*unitY+50+(yAxisBegin*unitY));
			}
			else{
				myNullY = (int)(rect.getHeight()*unitY+50+(yAxisBegin*unitY));
			}

			// paint the columns
			for(int i=0;i<_rangesVec.length-1;i++)
			{
//				   System.out.println("ich bin im drawOneHistogram 3!");

				double height = this.binsHeightPaint[i]; //height to paint
				double width = (_rangesVec[i+1] - _rangesVec[i]); //width to paint

				//is accented column?
				if (i == this.selectedBin){
					gfx.setColor(selectedColor);
				}
				else{
					gfx.setColor(mainColor);
				}

				if(height < 0.0)
				{
					gfx.fillRect((int) (sumLength*unitX)+myNullX, myNullY,(int) (width*unitX),(int) (-height*unitY));
					gfx.setColor(borderColor);
					gfx.drawRect((int) (sumLength*unitX)+myNullX, myNullY,(int) (width*unitX),(int) (-height*unitY)); //minus?
				}
				else
				{
//					   System.out.println("ich bin im drawOneHistogram 4!");
					gfx.fillRect(myNullX + (int) (sumLength*unitX), myNullY-(int)(height*unitY),(int) (width*unitX),(int) (height*unitY));
//					   System.out.println("ich bin im drawOneHistogram 5!");
					gfx.setColor(borderColor);
					gfx.drawRect(myNullX + (int) (sumLength*unitX), myNullY-(int)(height*unitY),(int) (width*unitX),(int) (height*unitY));
//					   System.out.println("ich bin im drawOneHistogram 6!");
				}

				//numbers
				gfx.setColor(Color.black);
				gfx.setFont(new Font("Helvetica", Font.PLAIN, 9));
//				   System.out.println("ich bin im drawOneHistogram 7!");

				//returns the exact data of the accented column in the display
				//only if there are only positiv bins
				if (i == this.selectedBin){
					gfx.setFont(new Font("Helvetica", Font.BOLD, 12));
//					System.out.println("binsVec: "+_binsVec[i]);
//					System.out.println("binSum: "+binSum);
//					System.out.println("freq: "+freq);
					if (binSum > 0.0){
						if(_binsVec[i] >= 0.0){
							freq = (_binsVec[i]/binSum)*100;
//							System.out.println("freq: "+freq);
							String relFrequency = Double.toString( freq);
              String relFrequencySmall =relFrequency.substring(0,Math.min(relFrequency.length(),relFrequency.indexOf(".")+3));
              String absFrequency = Double.toString( _binsVec[i] );
              //String absFrequencySmall =absFrequency.substring(0,Math.min(absFrequency.length(),absFrequency.indexOf(".")+3));
              String absFrequencySmall = absFrequency;
              String text1 = "Bin: " + i;
              int w1 = gfx.getFontMetrics().stringWidth(text1);
              gfx.drawString(text1, 10,15);
              String text2 = "Rel. freq.: "+relFrequencySmall+" %"; 
              int w2 = gfx.getFontMetrics().stringWidth(text2);
              gfx.drawString(text2, 10+w1+10, 15);

              gfx.drawString("Abs. freq.: "+absFrequencySmall, 10+w1+10+w2+10,15);
						}
					}

//					   System.out.println("ich bin im drawOneHistogram 8!");

					String _rangeSmallL = Double.toString( _rangesVec[i]);
          //String _rangeSmall =  _rangeSmallL.substring(0,Math.min(_rangeSmallL.length(),_rangeSmallL.indexOf(".")+3));
          String _rangeSmall = _rangeSmallL;
					gfx.drawString("Lower bound: " + _rangeSmall, 10,30);
					String _rangeBigL = Double.toString( _rangesVec[i+1]);
          //String _rangeBig =  _rangeBigL.substring(0,Math.min(_rangeBigL.length(),_rangeBigL.indexOf(".")+3));
          String _rangeBig =  _rangeBigL;
					gfx.drawString("Upper bound: " + _rangeBig, 10,45);
				}

				sumLength = sumLength + width;
//				   System.out.println("ich bin im drawOneHistogram 9!");

			}//end for

//			   System.out.println("ich bin im drawOneHistogram 10!");
			double binIntervalMax = this.intervalBorderMax(getMax(this.binsHeightPaint));
//			   System.out.println("ich bin im drawOneHistogram 11!");
			Vector binIntervals = this.paintSteps();
			Iterator ity = binIntervals.iterator();

			// write height (y-axis) sections
			while (ity.hasNext()){
//				System.out.println("ich bin im drawOneHistogram 12!");
				double heightToPaint = ((Double)(ity.next())).doubleValue();
					String binToWriteL = Double.toString(heightToPaint);
					String binToWrite =  binToWriteL.substring(0,Math.min(binToWriteL.length(),binToWriteL.indexOf(".")+5)); //section borders

					gfx.setFont(new Font("Helvetica", Font.PLAIN, 9));
					gfx.drawString(binToWrite,myNullX-40, myNullY-(int)(heightToPaint*unitY)+5);
					gfx.drawLine(myNullX-5, myNullY-(int)(heightToPaint*unitY)  ,myNullX+5,  myNullY-(int)(heightToPaint*unitY));
			}

//			   System.out.println("ich bin im drawOneHistogram 13!");

			//largest bin
			double largestBin = getMax(binsHeightPaint);
			String binToWriteL = Double.toString(getMax(binsHeightPaint));
			String binToWrite =  binToWriteL.substring(0,Math.min(binToWriteL.length(),binToWriteL.indexOf(".")+3)); //section borders

			gfx.setFont(new Font("Helvetica", Font.PLAIN, 9));
			gfx.drawString(binToWrite,myNullX-40, myNullY-(int)(largestBin*unitY)+5);
			gfx.drawLine(myNullX-5, myNullY-(int)(largestBin*unitY)  ,myNullX+5,  myNullY-(int)(largestBin*unitY));

//		   System.out.println("ich bin im drawOneHistogram 14!");

			//smallest bin if negative
			if (getMin(binsHeightPaint) < 0.0){

//				   System.out.println("ich bin im drawOneHistogram 15!");

				double smallestBin = getMin(binsHeightPaint);
				String smallbinToWriteL = Double.toString(getMin(binsHeightPaint));
				String smallbinToWrite =  smallbinToWriteL.substring(0,Math.min(smallbinToWriteL.length(),smallbinToWriteL.indexOf(".")+3)); //section borders

				gfx.setFont(new Font("Helvetica", Font.PLAIN, 9));
				gfx.drawString(smallbinToWrite,myNullX-40, myNullY-(int)(smallestBin*unitY)+5);
				gfx.drawLine(myNullX-5, myNullY-(int)(smallestBin*unitY)  ,myNullX+5,  myNullY-(int)(smallestBin*unitY));
			}

//			   System.out.println("ich bin im drawOneHistogram 16!");

			// write ranges (x-axis) sections
			Vector rangeIntervals = this.paintSectionRanges(_rangesVec);
			Iterator itx = rangeIntervals.iterator();

			//where begin to paint if _rangesVec[0] != 0
			double myXBegin = 0;

//			   System.out.println("ich bin im drawOneHistogram 17!");

			if(_rangesVec[0] > 0.0){
				myXBegin = _rangesVec[0];
			}
			while (itx.hasNext()){

//				   System.out.println("ich bin im drawOneHistogram 18!");

				double rangeToPaint = ((Double)(itx.next())).doubleValue();
				String rangeToWriteL = Double.toString(rangeToPaint);
				String rangeToWrite =  rangeToWriteL.substring(0,Math.min(rangeToWriteL.length(),rangeToWriteL.indexOf(".")+3)); //section borders

				gfx.setFont(new Font("Helvetica", Font.PLAIN, 9));

				gfx.drawString(rangeToWrite,myNullX+(int) ((rangeToPaint-myXBegin)*unitX)-10 , myNullY+20);
				gfx.drawLine(myNullX+(int)((rangeToPaint-myXBegin)*unitX),myNullY-5,myNullX+(int)((rangeToPaint-myXBegin)*unitX),myNullY+5);
			}

//			   System.out.println("ich bin im drawOneHistogram 19!");
			//last X-coordinate
			String rangeStrL = Double.toString(_rangesVec[_rangesVec.length-1]);
			String rangeStr =  rangeStrL.substring(0, Math.min(rangeStrL.length(),rangeStrL.indexOf(".")+3));
			gfx.drawLine(myNullX+(int) (sumLength*unitX),myNullY-5,myNullX+(int) (sumLength*unitX),myNullY+5);// X-Achse
			gfx.drawString(rangeStr,myNullX+(int) (sumLength*unitX)-10 , myNullY+20);

//			   System.out.println("ich bin im drawOneHistogram 20!");

			//only to test nullPoint
//			gfx.drawLine((int)(myNullX), (int)(myNullY), (int)(myNullX + 100), (int)(myNullY -100));

			//x-axis
			gfx.setColor(Color.BLACK);
			if (getMax(this.rangesVecPaint) < 0.0){
				gfx.drawLine(myNullX, myNullY,
						myNullX+(int)(getMin(this.rangesVecPaint)*unitX), myNullY);
			}
			if (getMin(this.rangesVecPaint) >= 0.0){
				gfx.drawLine(myNullX+(int)(xAxisBegin*unitX), myNullY,
						myNullX+(int)(getMax(this.rangesVecPaint)*unitX) - (int)(getMin(this.rangesVecPaint)*unitX), myNullY);
			}
			else{
				gfx.drawLine(myNullX+(int)(xAxisBegin*unitX), myNullY,
						myNullX+(int)(getMax(this.rangesVecPaint)*unitX), myNullY);
			}
			//y-axis

			if(getMin(this.binsHeightPaint)== 0.0 && getMax(this.binsHeightPaint)== 0.0){
				gfx.drawLine(myNullX, myNullY,
						myNullX, myNullY-(int)(200*unitY));
//				System.out.println("myNullX: "+myNullX);
//				System.out.println("myNullY: "+myNullY);
			}
//			   System.out.println("ich bin im drawOneHistogram 21!");
			if (getMax(this.binsHeightPaint)< 0.0){
				gfx.drawLine(myNullX, myNullY,
						myNullX, myNullY-(int)(getMin(this.binsHeightPaint)*unitY));
//				   System.out.println("ich bin im drawOneHistogram 22!");
			}
			else{
//				   System.out.println("ich bin im drawOneHistogram 23!");
				gfx.drawLine(myNullX, myNullY-(int)(yAxisBegin*unitY),
						myNullX, myNullY-(int)(getMax(this.binsHeightPaint)*unitY));
//				   System.out.println("ich bin im drawOneHistogram 24!");
			}

			//to show the panel with focus
			if (this.hasFocus()){
				g.setColor(Color.WHITE);
				g.drawRect(0, 0, this.getWidth()-1, this.getHeight()-1);

			}
//			   System.out.println("ich bin im drawOneHistogram end!");
		}

		////////////////////  AUXILIARY  FUNCTIONS   FOR   D R A W I N G ///////////////////////////////////

		/**
		 * @param max
		 * @return The maximum of an interval, especially for binHeights
		 */
		public double intervalBorderMax(double max){

			double maxRound;
			double a = 0.00001;
			while(a < max){
				a*=10;
			}
			a = a/100; //687 -> 690 ... a/10 -> 687 -> 700
			maxRound = Math.ceil(max/a)*a;
	//		System.out.println("maxRound: "+maxRound);
			return maxRound;
		}

		/**
		 *
		 * @param rangesVec
		 * @return Vector of ranges sections which should be written.
		 */
		private  Vector paintSectionRanges(double[] rangesVec){
			Vector rangesOut = new Vector(30);
			double stepSum  = Math.abs(rangesVec[0]) + Math.abs(rangesVec[rangesVec.length-1]);
			double myStep  = getDisplaySectionRanges(stepSum, 5.0);
			int i = 0;
			int p = 1;
			double sum = 0.0;
			rangesOut.add(new Double(rangesVec[0])); //smallest range section to paint
			while( myStep*p < stepSum){
				if ( (i+1) == rangesVec.length)
				{
					break;
				}
				if( rangesVec[i+1] <= 0.0 )
				{
					while (sum + Math.abs(Math.abs(rangesVec[i]) - Math.abs(rangesVec[i+1])) > myStep*p){
						p++;
					}
					while((i < rangesVec.length-1) &&(sum + Math.abs(Math.abs(rangesVec[i]) - Math.abs(rangesVec[i+1]))) < myStep * p){
						//		System.out.println("in der ersten While");
						sum = sum + Math.abs((Math.abs(rangesVec[i]) - Math.abs(rangesVec[i+1])));
//						System.out.println("sum: "+sum);
//						System.out.println("i: "+i);
						i++;
						if ( (i+1) == rangesVec.length)
						{
							break;
						}
					}
				}
				else
				{
					while ((sum +( Math.abs(rangesVec[i+1]) - rangesVec[i])) > myStep*p){
						p++;
					}
					//	System.out.println("bin hier im positiven von paintRanges2");
					while((i < rangesVec.length-1) &&(sum +( Math.abs(rangesVec[i+1]) - rangesVec[i])) < myStep * p){
						//		System.out.println("in der zweiten While");

						sum = sum + (Math.abs(rangesVec[i+1]) - rangesVec[i]);
						//		System.out.println("sum: "+sum);
						//		System.out.println("i: "+i);
						i++;
					}
				}
				rangesOut.add(new Double(rangesVec[i]));
				//	System.out.println("rangesOut["+i+"]"+rangesVec[i]);
				p++;

			}
			rangesOut.add(new Double(rangesVec[rangesVec.length-1]));
			return rangesOut;
		}


		/**
		 * @param interval interval over which the steps must be distributed
		 * @param minSteps minimum number of sections which should be written
		 * @return Section size to write the ranges.
		 */
		private static double getDisplaySectionRanges(double interval, double minSteps){
			interval = Math.abs(interval);
			double step;
			double stepMin = interval/minSteps;


			double magnitude = 1.0;
			while(stepMin > 10){
				stepMin/=10;
				magnitude*=10;
			}
			//stepMin is between 0 and 100
			while(stepMin < 1){
				stepMin*=10;
				magnitude/=10;
			}
			//stepMin between 1 and 10
			if(stepMin < 2.0){
				step = 2*magnitude;
			}
			else{
				step = 5*magnitude;
			}
		//	System.out.println("step of range " + step);
			return step;
		}

		/**
		 *
		 * @return Vector of numbers to write.
		 */
		private Vector paintSteps(){
//			   System.out.println("ich bin im paintSteps 1!");
			Vector numbers = new Vector(20);
			double max = this.getMax(this.binsHeightPaint);
			double min = this.getMin(this.binsHeightPaint);
//			System.out.println("max: "+max);
//			System.out.println("min: "+min);
			double distance = 0.0;
			if (min < 0.0)
			{
				distance = max-min;
			}
			else
			{
				distance = max;
				min = 0.0;
			}
			double step = this.getDisplaySectionBins(distance, 5);

			int countPositiv = (int)(max/step); //numbers of steps in positiv direction
			int countNegativ = (int)(Math.abs(min)/step);

			for (int i = 0; i <= countPositiv; i++){
				numbers.add(new Double(i*step));
			}
			for (int i = 1; i <= countNegativ; i++){
				numbers.add(new Double(-i*step));
			}
//			   System.out.println("ich bin im paintSteps end!");
			return numbers;
		}

		/**
		 *
		 * @param interval
		 * @param minSteps
		 * @return Section size to write the bins.
		 */
		private static double getDisplaySectionBins(double interval, double minSteps){
//			System.out.println("ich bin im getDisplaySectionBins 1!");
			if (interval == 0.0){
				interval = 10;
			}

			interval = Math.abs(interval);
			double step;
			double stepMin = interval/minSteps;

//			System.out.println("ich bin im getDisplaySectionBins 2!");
//			System.out.println("stepMin: " + stepMin);



			double magnitude = 1.0;
			while(stepMin > 10){
				stepMin/=10;
				magnitude*=10;
			}
			//stepMin is between 0 and 100
			while(stepMin < 1){
				stepMin*=10;
				magnitude/=10;
			}
			//stepMin between 1 and 10
			if(stepMin < 2.0){
				step = 2*magnitude;
			}
			else{
				step = 5*magnitude;
			}
//			System.out.println("step: " + step);
//			   System.out.println("ich bin im getDisplaySectionBins end!");
			return step;
		}



		///////////////////////////    K E Y L I S T E N E R   ////////////////////////////////////////////////////////
		public void setKeyListener(){
			this.setFocusable(true);
			this.addKeyListener(new KeyListener(){

				/**
				 * left arrow to go left in the histogram columns
				 * right arrow to go right in the histogram columns
				 */
				public void keyPressed(KeyEvent e) {
					if (e.getKeyCode() == KeyEvent.VK_LEFT){
						if (HistogramPanel.this.selectedBin == -1){
							//nothing selected yet
							if ((HistogramPanel.this.binsVecPaint != null)&&(HistogramPanel.this.binsVecPaint.length != 0)){
								HistogramPanel.this.selectedBin = HistogramPanel.this.binsVecPaint.length-1;
							}
						}
						else{
							HistogramPanel.this.selectedBin--;
							if (HistogramPanel.this.selectedBin < 0){
								HistogramPanel.this.selectedBin = HistogramPanel.this.binsVecPaint.length-1;
							}
						}
					}
					else if (e.getKeyCode() == KeyEvent.VK_RIGHT){
						if (HistogramPanel.this.selectedBin == -1){
							//nothing selected yet
							if ((HistogramPanel.this.binsVecPaint != null)&&(HistogramPanel.this.binsVecPaint.length != 0)){
								HistogramPanel.this.selectedBin = 0;
							}
						}
						else{
							HistogramPanel.this.selectedBin = (HistogramPanel.this.selectedBin+1)%HistogramPanel.this.binsVecPaint.length;
						}
					}
					HistogramPanel.this.repaint();
				}

				public void keyReleased(KeyEvent e) { }

				public void keyTyped(KeyEvent e) { }

			});

			this.addFocusListener(new FocusListener(){

				public void focusGained(FocusEvent arg0) {

					if (HistogramPanel.this.binsVecPaint != null && HistogramPanel.this.binsVecPaint.length > 0){
						HistogramPanel.this.selectedBin = 0;
					}
					HistogramPanel.this.repaint();
				}

				public void focusLost(FocusEvent arg0) {
					HistogramPanel.this.selectedBin = -1;
					HistogramPanel.this.repaint();
				}

			});
		}

		/**
		 * to get the focus on the chosen panel
		 * @author fp0708
		 *
		 */
		public class MouseNavigator extends MouseAdapter implements MouseMotionListener{

			public void mousePressed(MouseEvent e){
				HistogramPanel.this.grabFocus();
				HistogramPanel.this.repaint();
			}

			public void mouseDragged(MouseEvent e) { }

			public void mouseMoved(MouseEvent e) { }



		}
	}

	public String toString() {
		return "Histogram1d";
	}
}
