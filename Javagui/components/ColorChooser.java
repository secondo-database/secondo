package components;

import javax.swing.event.*;
import javax.swing.*;
import java.awt.*;


/**
 * this class provides a component to
 * select a color by the RGB-values
 */
public class ColorChooser extends JPanel{

/** creates a new ColorChooser */
public ColorChooser(){

  ColorDisplay = new JComponent() {
    public void paint(Graphics G){
        G.setColor(ColorChooser.this.CurrentColor);
        G.fillRect(0,0,getWidth()-3,getHeight());
    } };
  int w1=20;
  int h1=80;
  int w2=10;
  int h2=100;
  Dimension d = new Dimension(w1,h1);
  ColorDisplay.setPreferredSize(d);
  ColorDisplay.setSize(d);
  Dimension SD = new Dimension(w2,h2);
  Red.setPreferredSize(SD);
  Green.setPreferredSize(SD);
  Blue.setPreferredSize(SD);
  Red.setSize(SD);
  Green.setSize(SD);
  Red.setSize(SD);

  add(ColorDisplay);
  add(Red);
  add(Green);
  add(Blue);
  RedLabel   = new JLabel("r");
  GreenLabel = new JLabel("g");
  BlueLabel  = new JLabel("b");
  Dummy = new JComponent(){};

  Dimension DummyDim = new Dimension(w1,10);
  Dimension LabelDim = new Dimension(w2,10);
  Dummy.setPreferredSize(DummyDim);
  RedLabel.setPreferredSize(LabelDim);
  GreenLabel.setPreferredSize(LabelDim);
  BlueLabel.setPreferredSize(LabelDim);

  add(Dummy);
  add(RedLabel);
  add(GreenLabel);
  add(BlueLabel);
  doLayout();
  addListener();
  //validate();
}



/** adds listener for event handling */
private void addListener(){

 ChangeListener CL = new ChangeListener(){
                     public void stateChanged(ChangeEvent e){
                       ColorChooser.this.CurrentColor =
                                      new Color(Red.getValue(),
                                                Green.getValue(),
                                                Blue.getValue());
                       ColorChooser.this.ColorDisplay.repaint();

                     }};

  Red.addChangeListener(CL);
  Green.addChangeListener(CL);
  Blue.addChangeListener(CL);

}

/** get the size of the component */
public Dimension getPreferredSize(){
  return new Dimension(80,120);
}


/** order the child-components */
public void doLayout(){
 int w = getWidth() / 4;  // Color + 3 Slider
 if (RedLabel.getGraphics()!=null){
   int LabelHeight = RedLabel.getGraphics().getFontMetrics().getHeight();
   int h = getHeight()-LabelHeight;
   ColorDisplay.reshape(0,0,w,h);
   Red.reshape(w,0,w,h);
   Green.reshape(2*w,0,w,h);
   Blue.reshape(3*w,0,w,h);
   Dummy.reshape(0,h,w,LabelHeight);
   RedLabel.reshape(w,h,w,LabelHeight);
   GreenLabel.reshape(2*w,h,w,LabelHeight);
   BlueLabel.reshape(3*w,h,w,LabelHeight);
 }
}


/** set a new Size */
public void setSize(int w,int h){
 super.setSize(w,h);
 doLayout();
}


public void setLayout(LayoutManager LM){}

/** returns the selected color */
public Color getColor(){ return CurrentColor;}

/** set the color */
public void  setColor( Color C){
  CurrentColor=C;
  Red.setValue(C.getRed());
  Green.setValue(C.getGreen());
  Blue.setValue(C.getBlue());
}

  private JComponent ColorDisplay;
  private JComponent Dummy;
  private JLabel RedLabel;
  private JLabel GreenLabel;
  private JLabel BlueLabel;
  private Color CurrentColor;
  private JSlider Red   = new JSlider(JSlider.VERTICAL,0,255,0);
  private JSlider Green = new JSlider(JSlider.VERTICAL,0,255,0);
  private JSlider Blue  = new JSlider(JSlider.VERTICAL,0,255,0);

}


