package viewer.viewer3d.objects;


import javax.swing.*;
import components.*;
import java.awt.*;
import java.awt.event.*;

public class FuzzySettings extends JDialog{


public FuzzySettings(Frame F){
  super(F,"Color-Settings",true);
  getContentPane().setLayout(new BorderLayout());
  setSize(300,200);
  JPanel P0 = new JPanel();
  JPanel P1 = new JPanel(new GridLayout(2,2));
  P1.add(CHMin);
  P1.add(CHMax);
  P1.add(new JLabel("MinColor"));
  P1.add(new JLabel("MaxColor"));
  JPanel P2 = new JPanel();
  P2.add(OkBtn);
  P2.add(CancelBtn);
  ActionListener AL = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         Object Source = evt.getSource();
         if(Source.equals(OkBtn))
            Result = OK;
         else
            Result = CANCELED;
         setVisible(false);
      }}; 
  OkBtn.addActionListener(AL);
  CancelBtn.addActionListener(AL);
  P0.add(P1);
  getContentPane().add(P0,BorderLayout.CENTER);
  getContentPane().add(P2,BorderLayout.SOUTH);  
}


public int getReturnValue(){return Result;}

public Color getMinColor(){ return CHMin.getColor();}
public Color getMaxColor(){ return CHMax.getColor();}

public void setMinColor(Color C){ CHMin.setColor(C);}
public void setMaxColor(Color C){ CHMax.setColor(C);}

public final int OK = 0;
public final int CANCELED = 1;
private int Result;
private JButton OkBtn = new JButton("ok");
private JButton CancelBtn = new JButton("cancel");
private ColorChooser CHMin = new ColorChooser();
private ColorChooser CHMax= new ColorChooser();

}