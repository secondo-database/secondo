
package viewer.hoese;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;

public class LicenseDialog extends JDialog{

  public LicenseDialog(Frame parent){
     super(parent,true);
      area = new JEditorPane();
      area.setContentType("text/html");
      JScrollPane scrollPane = new JScrollPane(area);
      getContentPane().setLayout(new BorderLayout());
      getContentPane().add(scrollPane, BorderLayout.CENTER);
      okBtn = new JButton("accept");
      getContentPane().add(okBtn,BorderLayout.SOUTH);
      
      okBtn.addActionListener(new ActionListener(){
        public void actionPerformed(ActionEvent evt){
           LicenseDialog.this.setVisible(false);
        }
      });
      setSize(800,600);
      area.setEditable(false);
  }


  public boolean  setSource(URL source){
    try{
       area.setPage(source);
       return true;
    } catch(Exception e){
       return false;
    }
  }



  JEditorPane area;
  JButton okBtn;

}


