package components;

import javax.swing.*;
import java.awt.event.*;
import java.awt.*;
import java.util.Vector;

public class LongScrollBar extends JComponent{

/* creates a new Scrollbar with default values */
public LongScrollBar(){
   this(0,0,100);
}

/* creates a new Scrollbar with given values */
public LongScrollBar(long value, long min, long max){
  ChangeValueListeners = new Vector();
  setValues(value,min,max);
  unitIncrement = 1;
  blockIncrement = (max-min)/10;
  if(blockIncrement<1)
     blockIncrement=1;
  ML ml = new ML();
  addMouseListener(ml);
  addMouseMotionListener(ml);
}


/* add a Listener to informed if value changed */
public void addChangeValueListener(ChangeValueListener CL){
   if(CL==null) return;
   if(ChangeValueListeners.contains(CL)) return;
   ChangeValueListeners.add(CL);
}

/* remove a Listener */
public void removeChangeValueListener(ChangeValueListener CL){
   ChangeValueListeners.remove(CL);
}


/* set the Values */
public void setValues(long value, long min, long max){
 if(min>=max)
     max = min+1;
  if(value<min)
     value = min;
  if(value>max)
     value = max;
  if(this.value==value && minimum==min && maximum==max)
     return;
  this.value = value;
  this.minimum = min;
  this.maximum = max;
  repaint();
  ChangeValueEvent evt = new ChangeValueEvent(this,value);
  for(int i=0;i<ChangeValueListeners.size();i++){
      ((ChangeValueListener)ChangeValueListeners.get(i)).valueChanged(evt);
  }
}

/** returns the minimum size of this component */
public Dimension getMinimumSize(){
   return new Dimension(40,10);
}

/** set the BlockIncrement */
public void setBlockIncrement(long Inc){
  if(Inc<1)
     Inc=1;
  this.blockIncrement = Inc;
}

/** set the unit increment */
public void setUnitIncrement(long Inc){
  long D = maximum-minimum;
  if(Inc<1)
     Inc=1;
  this.unitIncrement = Inc;
}


/** returns the minimum */
public long getMinimum(){
  return minimum;
}

/** returns the maximum */
public long getMaximum(){
  return maximum;
}

/** returns the unit increment */
public long getUnitIncrement(){
  return unitIncrement;
}

/** returns the block increment */
public long getBlockIncrement(){
   return blockIncrement;
}

/** returns the actual value */
public long getValue(){
   return value;
}

/** set the actual value */
public void setValue(long value){
   setValues(value,minimum,maximum);
}

/** set the minimum value*/
public void setMinimum(long min){
   setValues(value,min,maximum);
}


/** set the maximum value */
public void setMaximum(long max){
   setValues(value,minimum,max);
}


/** paint the ScrollBar */
protected void paintComponent(Graphics g){
   super.paintComponent(g);
   int w = getWidth();
   int h = getHeight();
   g.setColor(java.awt.Color.black);
   g.drawRect(0,0,w-1,h-1);
   g.drawRect(0,0,ButtonSize,h);
   g.drawRect(w-ButtonSize,0,ButtonSize,h);
   g.setColor(java.awt.Color.gray);
   g.fillRect(1,1,ButtonSize-2,h-2);
   g.fillRect(w-(ButtonSize-1),1,ButtonSize-2,h-2);
   g.setColor(java.awt.Color.black);
   g.drawString("<",2,h/2+5);
   g.drawString(">",w-ButtonSize+2,h/2+5);
   // draw the marker
   int MarkerHalf = (MarkerSize+1)/ 2;
   int space = w-2*ButtonSize-(MarkerSize);
   double delta = ((double)value-minimum)/(maximum-minimum);
   int pos = (int) ( space*delta);
   g.setColor(java.awt.Color.blue);
   g.fillRect(pos +ButtonSize,0,MarkerSize,h);
   g.setColor(java.awt.Color.black);
   g.drawRect(pos +ButtonSize,0,MarkerSize,h);
   g.drawLine(pos+ButtonSize+MarkerHalf,0,pos+ButtonSize+MarkerHalf,h);
}



private void setToMousePos(int x){
  int MarkerHalf = (MarkerSize+1)/ 2;
  int w = getWidth();
  if(x<ButtonSize+MarkerHalf){ // left from  last valid value
     if(value>minimum)
        setValues(minimum,minimum,maximum); // set to start
     return;
  }
  if(x>w-(ButtonSize+MarkerHalf)){ // right from last valid value
     if(value<maximum)
        setValues(maximum,minimum,maximum); // set to end;
     return;
  }
  int space = w-2*ButtonSize-MarkerSize;
  x = x-ButtonSize-MarkerHalf;
  long interval = maximum-minimum;
  long v = x*interval/space+minimum;
  setValues(v,minimum,maximum);


}


public boolean next(){
   if(value>=maximum) return false;
   setValues(value+unitIncrement,minimum,maximum);
   return true;
}

public boolean back(){
   if(value<=minimum) return false;
   setValues(value-unitIncrement,minimum,maximum);
   return true;
}

public boolean stepBack(){
  if(value<=minimum) return false;
  setValues(value-blockIncrement,minimum,maximum);
  return true;
}

public boolean stepNext(){
  if(value>=maximum) return false;
  setValues(value+blockIncrement,minimum,maximum);
  return true;
}

private int getSliderPos(){
   int w = getWidth();
   int MarkerHalf = (MarkerSize+1)/ 2;
   int space = w-2*ButtonSize-(MarkerSize);
   double delta = ((double)value-minimum)/(maximum-minimum);
   int pos = (int) ( space*delta);
   return pos+MarkerHalf; // the middle if slider
}

protected long value;
protected long minimum;
protected long maximum;
protected long unitIncrement;
protected long blockIncrement;
private int ButtonSize = 15;
private int MarkerSize=10;
private Vector ChangeValueListeners;



private class ML extends MouseAdapter implements MouseMotionListener{

   public void mouseClicked(MouseEvent evt){
      int w = LongScrollBar.this.getWidth();
      int BS = LongScrollBar.this.ButtonSize;
      int x = evt.getX();
      if(x<BS)
        LongScrollBar.this.back();
      else if(x>w-BS)
        LongScrollBar.this.next();
      else{
          long sp = LongScrollBar.this.getSliderPos();
	  if(x<sp)
	     LongScrollBar.this.stepBack();
	  else if(x>sp)
	     LongScrollBar.this.stepNext();

      }
   }

   public void mouseMoved(MouseEvent evt){}

   public void mouseDragged(MouseEvent evt){
     LongScrollBar.this.setToMousePos(evt.getX());
   }

}


}
