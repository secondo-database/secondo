


package viewer.hoese;

import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.event.*;


public class ScaleFactorDialog extends JDialog{
  
  private JTextField valueTF;
  private JButton cancelBtn;
  private JButton acceptBtn;
  private JButton resetBtn;
  private JButton defaultBtn;

  public ScaleFactorDialog(java.awt.Frame owner){
    super(owner, true);
    setSize(400,150);
    getContentPane().setLayout(new BorderLayout());
    JPanel jp1 = new JPanel();
    jp1.setLayout(new BoxLayout(jp1, BoxLayout.Y_AXIS));
    jp1.add(new JLabel("Please enter the scale factor to use"));
    valueTF = new JTextField(20);
    valueTF.setText(""+ProjectionManager.getScaleFactor());
    jp1.add(valueTF);
    JPanel jp = new JPanel();
    jp.add(jp1);


    cancelBtn = new JButton("Cancel");
    acceptBtn = new JButton("Accept");
    resetBtn  = new JButton("Reset");
    defaultBtn = new JButton("Default");

    ActionListener al = new ActionListener(){
      public void actionPerformed(ActionEvent evt){
         Object source = evt.getSource();
         if(source.equals(cancelBtn)){
            cancel();
         } else if(source.equals(acceptBtn)){
            accept();
         } else if(source.equals(resetBtn)){
           valueTF.setText(""+ProjectionManager.getScaleFactor());
         } else if(source.equals(defaultBtn)){
           valueTF.setText("1.0");
         }
      }
    };
    cancelBtn.addActionListener(al); 
    acceptBtn.addActionListener(al); 
    resetBtn.addActionListener(al);  
    defaultBtn.addActionListener(al);  

    JPanel cp = new JPanel();
    cp.add(cancelBtn); 
    cp.add(acceptBtn); 
    cp.add(resetBtn);  
    cp.add(defaultBtn);
     
    getContentPane().setLayout(new BorderLayout());
    getContentPane().add(jp, BorderLayout.CENTER);
    getContentPane().add(cp, BorderLayout.SOUTH);

  }


  private void cancel(){
    valueTF.setText(""+ProjectionManager.getScaleFactor());
    setVisible(false);
  }

   private void accept(){
     try{
       double value = Double.parseDouble(valueTF.getText().trim());
       if(value>0){
         ProjectionManager.setScaleFactor(value);
         setVisible(false);
       } else {
         JOptionPane.showMessageDialog(this, "Scale factor must be greater than 0");
       }
     } catch(Exception e){
         JOptionPane.showMessageDialog(this, "Scale factor must be a number");
     }
   }



}

