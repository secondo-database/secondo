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


package  viewer.hoese;

import  java.awt.*;
import  java.awt.event.*;
import  javax.swing.*;
import  java.util.*;
import  java.text.*;
import viewer.HoeseViewer;


public class TimePanel extends javax.swing.JPanel               // implements ValueHasChangedListener
{

  /** class for drawing the time-ruler
   * @see <a href="TimePanelsrc.html#Lineal">Source</a>
   */
  class Lineal extends JComponent {
    private static final int SIZE = 20;

    public void setPreferredHeight (int ph) {
      setPreferredSize(new Dimension(SIZE, ph));
    }

    public void setPreferredWidth (int pw) {
      setPreferredSize(new Dimension(pw, SIZE));
    }

    public void paintComponent (Graphics g) {
      //            Rectangle drawHere = g.getClipBounds();
      Rectangle drawHere = ZeitScrollPane.getViewport().getViewRect();
      g.setColor(new Color(20, 200, 4));
      g.fillRect(drawHere.x, 0, drawHere.width, SIZE);
      g.setFont(new Font("SansSerif", Font.PLAIN, 10));
      g.setColor(Color.black);
      if (TimeBounds == null) {
        g.drawString("no Time objects", 5, 15);
        return;
      }
      long TimeBoundsStart = (long)(TimeBounds.getStart()*1440);
      String text;
      TimeBoundsStart = (TimeBoundsStart/TimeperPixel)*TimeperPixel;
      long t = drawHere.x*TimeperPixel + TimeBoundsStart;
      DateLabel.setText(LEUtils.convertTimeToString(((double)t)/1440).substring(0, 
          10));
      for (int d = drawHere.x; d < drawHere.x + drawHere.width; d++) {
        t = d*TimeperPixel + TimeBoundsStart;
        if ((t%bigdivision) == 0) {
          g.drawLine(d, SIZE - 1, d, 5);
          text = LEUtils.convertTimeToString(((double)t)/1440).substring(TimeFormatStart, 
              TimeFormatLen);
          g.drawString(text, d + 1, 10);
        } 
        else if ((t%smalldivision) == 0)
          g.drawLine(d, SIZE - 1, d, SIZE - 8);
      }
    }
}
  /**
   * The constructor , creates a panel with ruler, close-button etc.
   * @param   MainWindow parent MainWindow app.
   * @see <a href="TimePanelsrc.html#TimePanel">Source</a>
   */
  public TimePanel (HoeseViewer parent) {
    mw = parent;
    setLayout(new GridLayout(1, 1));
    LabelsPanel = new JPanel(new GridLayout(0, 1));
    LabelsPanel.setPreferredSize(new Dimension(80, 60));
    TimeObjectsPanel = new JPanel(new GridLayout(0, 1));
    //TimeObjectsPanel.setPreferredSize(new Dimension(1000, 60));
    ZeitScrollPane = new javax.swing.JScrollPane(TimeObjectsPanel);
    ZeitScrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    DateLabel = new JLabel("", JLabel.RIGHT);
    DateLabel.setBackground(new Color(20, 150, 4));
    //DateLabel.setHorizontalTextPosition(JLabel.RIGHT);//setPreferredSize(new Dimension(80,Lineal.SIZE));
    DateLabel.setOpaque(true);
    DateLabel.setForeground(Color.black);
    ZeitScrollPane.setCorner(JScrollPane.UPPER_LEFT_CORNER, DateLabel);
    columnView = new Lineal();
    columnView.setPreferredWidth(200);
    //      (int)((TimeBounds.getEnd() - TimeBounds.getStart()) *PixelTime));
    ZeitScrollPane.setColumnHeaderView(columnView);
    ZeitScrollPane.setRowHeaderView(LabelsPanel);
    //ZeitScrollPane.setPreferredSize(new Dimension(800,60));
    //setPreferredSize(new Dimension(mw.getWidth(),60));
    add(ZeitScrollPane);
  }

  /**
   Calculates the paraameter for displaying time-ruler
   * @see <a href="TimePanelsrc.html#calculateParameter">Source</a>
   */
  private void calculateParameter () {
    if(TimeBounds==null) return; // for empty objects 
    long TimeInMinutes = (long)((TimeBounds.getEnd() - TimeBounds.getStart())*1440);
    if (TimeInMinutes < 2880) {
      TimeperPixel = 1;         //1=1 Minute
      bigdivision = 60;         //ist eine Stunde
      smalldivision = 15;       //Viertelstunde
      TimeFormatStart = 11;                     // z.b.23.01.2000 17:30			
      TimeFormatLen = 16;
    } 
    else if (TimeInMinutes < (72*1440)) {
      TimeperPixel = 36;        //Min
      bigdivision = 1440;       //ist ein Tag
      smalldivision = 360;      //6h
      TimeFormatStart = 0;      // z.b.23.01.2000 17:30			
      TimeFormatLen = 5;
    } 
    else if (TimeInMinutes < (5*72*1440)) {
      TimeperPixel = 120;        //Min
      bigdivision = 5*1440;       //ist 5 Tag
      smalldivision = 1440;      //6h
      TimeFormatStart = 0;      // z.b.23.01.2000 17:30			
      TimeFormatLen = 5;
    } 
    else if (TimeInMinutes < (48*4*7*1440)) {
      TimeperPixel = 720;       //min
      bigdivision = 40320;      //4 weeks
      smalldivision = 10080;                    //1 week
      TimeFormatStart = 0;      // z.b.23.01.2000 17:30			
      TimeFormatLen = 5;
    } 
    else {      //if (TimeInMinutes<(48*4*7*1440)){
      TimeperPixel = 8784;      //min
      bigdivision = 527040;                     //4 weeks
      smalldivision = 43920;                    //1 week
      TimeFormatStart = 6;      // z.b.23.01.2000 17:30			
      TimeFormatLen = 10;
    }
    PixelTime = 1440/(double)TimeperPixel;
  }

  /**
   * Sets the time-object that should be displayed in the timepanel
   * @param ti The time-object
   * @see <a href="TimePanelsrc.html#setTimeObject">Source</a>
   */
  public void setTimeObject (Timed ti) {
    TimeBounds = ti.getBoundingInterval();
    TimeObject = ti;
    calculateParameter();
    LabelsPanel.removeAll();
    TimeObjectsPanel.removeAll();
    initComponents();
  }

  /**
   * Init. the timepanel according to the set timeobject
   * @see <a href="TimePanelsrc.html#initComponents">Source</a>
   */
  private void initComponents () {
    if(TimeBounds==null)
        return; 
    String AttrName = "unknown";
    if (TimeObject instanceof DsplBase)
      AttrName = ((DsplBase)TimeObject).getAttrName();
    JLabel jl = new JLabel(AttrName);
    //jl.setVerticalTextPosition(JLabel.BOTTOM);
    jl.setPreferredSize(new Dimension(80, 25));
    LabelsPanel.add(jl);
    columnView.setPreferredWidth((int)((TimeBounds.getEnd() - TimeBounds.getStart())*PixelTime));
//    TimeObjectsPanel.setPreferredSize(new Dimension((int)((TimeBounds.getEnd()
 //       - TimeBounds.getStart())*PixelTime), 25));
    JPanel jp = TimeObject.getTimeRenderer(PixelTime);
    TimeObjectsPanel.add(jp);
    HoeseViewer.setAllOpaque(this,true);
    HoeseViewer.setAllBackgrounds(this,mw.getBackground());
  }
  private Lineal columnView;
  private Interval TimeBounds;                  //=new sj.display.Interval(1,2,true,true);
  private javax.swing.JScrollPane ZeitScrollPane;
  private JLabel DateLabel;
  private JPanel LabelsPanel, TimeObjectsPanel;
  private long TimeperPixel;                    //min
  private long bigdivision;                     //4 weeks
  private long smalldivision;                   //1 week
  private int TimeFormatStart;                  // z.b.23.01.2000 17:30			
  private int TimeFormatLen;
  private double PixelTime;
  private Timed TimeObject;
  private HoeseViewer mw;
}



