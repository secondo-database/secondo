package viewer.viewer3d;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import viewer.viewer3d.graphic3d.*;

/**
  * this class provides a JDialog to change the 3d-Options for
  * the visualization of a fuzzy spatial object
  */
public class Options3D extends JDialog{

/** creates a new Options-Dialog for W3D */
public Options3D(Frame F,World3D W3D){
 super(F,"3D-Options",true); 
 this.W3D = W3D;
 setSize(350,200);
 Container ContentPane = getContentPane();
 ContentPane.setLayout(new FlowLayout());
 JPanel C = new JPanel(new GridLayout(6,4));
 C.add(new JLabel(" "));
 C.add(new JLabel("X",JLabel.CENTER));
 C.add(new JLabel("Y",JLabel.CENTER));
 C.add(new JLabel("Z",JLabel.CENTER));
 WindowXText = new JTextField(""+W3D.getWindowX(),5);
 WindowYText = new JTextField(""+W3D.getWindowY(),5);
 C.add(new JLabel("Window"));
 C.add(WindowXText);
 C.add(WindowYText);
 C.add(new JLabel(""));   // dummy for ZValue

 /*        Eye - Values    */
 C.add(new JLabel("Eye"));
 EyeXText = new JTextField(""+ W3D.getEyeX(),5);
 C.add(EyeXText);
 EyeYText = new JTextField(""+W3D.getEyeY(),5);
 C.add(EyeYText);
 EyeZText = new JTextField(""+ W3D.getEyeZ(),5);
 C.add(EyeZText);
 /*     VRP-Values      */
 C.add(new JLabel("VRP"));
 VRPXText = new JTextField(""+W3D.getVRPX(),5);
 C.add(VRPXText);
 VRPYText = new JTextField(""+ W3D.getVRPY(),5);
 C.add(VRPYText);
 VRPZText = new JTextField(""+ W3D.getVRPZ(),5);
 C.add(VRPZText);

 /*  ViewUp - values */
 C.add(new JLabel("ViewUp"));
 ViewUpXText = new JTextField(""+ W3D.getViewUpX(),5);
 C.add(ViewUpXText);
 ViewUpYText = new JTextField(""+ W3D.getViewUpY(),5);
 C.add(ViewUpYText);
 ViewUpZText = new JTextField(""+ W3D.getViewUpZ(),5);
 C.add(ViewUpZText);
 C.add(ResetBtn);
 C.add(OkBtn);
 C.add(CancelBtn); 
 C.add(ApplyBtn);

 ResetBtn.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
               Options3D.this.reset();
            }});
 OkBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            if( Options3D.this.accept())
		  setVisible(false);		   
       }
       });

 CancelBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
            Options3D.this.reset();
            ResultValue = CANCEL; 
            Options3D.this.setVisible(false);
       }});

 ApplyBtn.addActionListener(new ActionListener(){
       public void actionPerformed(ActionEvent evt){
          Options3D.this.accept();
       }});

 getContentPane().add(C);
}


/** send the options to the World */
private boolean accept(){

  boolean ok = true;
  double WindowX=0,WindowY=0;
  double EyeX=0,EyeY=0,EyeZ=0;
  double VRPX=0,VRPY=0,VRPZ=0;
  double ViewUpX=0,ViewUpY=0,ViewUpZ=0;

  try{
   WindowX    =  Double.parseDouble(WindowXText.getText());
   WindowY    =  Double.parseDouble(WindowYText.getText());
   EyeX       =  Double.parseDouble(EyeXText.getText());
   EyeY       =  Double.parseDouble(EyeYText.getText());
   EyeZ       =  Double.parseDouble(EyeZText.getText());
   VRPX       =  Double.parseDouble(VRPXText.getText());
   VRPY       =  Double.parseDouble(VRPYText.getText());
   VRPZ       =  Double.parseDouble(VRPZText.getText());
   ViewUpX    =  Double.parseDouble(ViewUpXText.getText());
   ViewUpY    =  Double.parseDouble(ViewUpYText.getText());
   ViewUpZ    =  Double.parseDouble(ViewUpZText.getText());
  }
  catch(Exception e){
     ok = false;
     OptionPane.showMessageDialog(this,"invalid values in fields (not a double)",
                                  "error",JOptionPane.ERROR_MESSAGE);
  }

  boolean viewcheck=true;
  boolean windowok=true;

  if (ok) {
    viewcheck =
        FM3DGraphic.checkView(
                  EyeX, EyeY, EyeZ,
                  VRPX, VRPY, VRPZ,
                  ViewUpX, ViewUpY, ViewUpZ);
    windowok = WindowX >0 & WindowY>0;
 
    if (!viewcheck){
       OptionPane.showMessageDialog(this,"Eye,VRP,ViewUp cannot be on a line",
                                    "error",JOptionPane.ERROR_MESSAGE);
       ok = false;
       }
    else
       if (!windowok) {
          OptionPane.showMessageDialog(this,"Window must be greater then 0x0","error",
                                        JOptionPane.ERROR_MESSAGE);
          ok = false;
       }
  }   // if ok

  if(ok)  {    // apply fields
   W3D.setView( EyeX   , EyeY   , EyeZ ,
                VRPX   , VRPY   , VRPZ ,
                ViewUpX, ViewUpY, ViewUpZ);
   W3D.setWindow(WindowX,WindowY);
   W3D.update();
   W3D.repaint();
   ResultValue = OK;
 }
  return ok;
}


/** set the options to the last values */
public void reset(){
 WindowXText.setText(""+W3D.getWindowX());
 WindowYText.setText(""+W3D.getWindowY());
 EyeXText.setText(""+ W3D.getEyeX());
 EyeYText.setText(""+W3D.getEyeY());
 EyeZText.setText(""+ W3D.getEyeZ());
 VRPXText.setText(""+ W3D.getVRPX());
 VRPYText.setText(""+ W3D.getVRPY());
 VRPZText.setText(""+ W3D.getVRPZ());
 ViewUpXText.setText(""+ W3D.getViewUpX());
 ViewUpYText.setText(""+ W3D.getViewUpY());
 ViewUpZText.setText(""+ W3D.getViewUpZ());
}


public final static int OK = 0;
public final static int CANCEL = 1;
private World3D W3D;
private JTextField WindowXText;
private JTextField WindowYText;
private JTextField EyeXText;
private JTextField EyeYText;
private JTextField EyeZText;
private JTextField VRPXText;
private JTextField VRPYText;
private JTextField VRPZText;
private JTextField ViewUpXText;
private JTextField ViewUpYText;
private JTextField ViewUpZText;
private JButton ResetBtn = new JButton("reset");
private JButton OkBtn = new JButton("ok");
private JButton CancelBtn = new JButton("cancel");
private JButton ApplyBtn = new JButton("apply");
private String State="ok";
private int ResultValue;
private JOptionPane OptionPane = new JOptionPane(); // to show Messages
}
