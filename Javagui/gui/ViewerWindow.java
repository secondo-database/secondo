
package gui;

import viewer.SecondoViewer;
import viewer.MenuVector;
import javax.swing.*;
import java.awt.event.*;


public class ViewerWindow extends JFrame{

 ViewerWindow(SecondoViewer sv, MainWindow mw){
    this.mw = mw;
    this. viewer = sv;
    setSize(640,480);
    setTitle("Javagui: " + sv.getName());
    getContentPane().add(sv);
    addWindowListener( new WindowAdapter(){
       public void windowClosing(WindowEvent evt){
        close();
       }
    });
    createMenu();
 }

 private void close(){
    mw.closeSeparatedViewer(viewer);    
 }


private void createMenu(){
    JMenuBar menu = new JMenuBar();
    JMenu vMenu = new JMenu("Frame");
    JMenuItem MI_Close = new JMenuItem("Close");
    MI_Close.addActionListener( new ActionListener(){
      public void actionPerformed(ActionEvent evt){
        ViewerWindow.this.setVisible(false);
        ViewerWindow.this.close();
      }
    }); 
    vMenu.add(MI_Close);
    menu.add(vMenu);
   
    MenuVector mv = viewer.getMenuVector();
    if(mv!=null){
      for(int i=0;i<mv.getSize();i++){
         menu.add(mv.get(i));
      }
    }


    setJMenuBar(menu);

}


 SecondoViewer viewer;
 MainWindow mw;

}
