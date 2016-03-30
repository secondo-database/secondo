package appGui;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.logging.Logger;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JToolBar;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.RefreshLeftSideTree;
import appGuiUtil.RefreshRightSideTree;

public class ActionToolbar
  implements ActionListener
{
  public JToolBar jtoolbar;
  private MainGui mgui;
  JButton jbutCopy2Secondo;
  JButton jbutCopy2Postgres;
  JButton jbutReonnectSecondo;
  JButton jbutReonnectPostgres;
  
  public ActionToolbar(MainGui zGui)
  {
    this.mgui = zGui;
    this.jtoolbar = new JToolBar("Toolbar", 0);
    
    initToolbar();
  }
  
  private void initToolbar()
  {
	  LogFileHandler.mlogger.info("try to load toolbar");
    
    this.jbutCopy2Secondo = new JButton(new ImageIcon(ActionToolbar.class.getResource("/convert2Secondo.gif")));  
    this.jbutCopy2Secondo.setToolTipText("Open selection dialog to convert to SECONDO db");
    this.jbutCopy2Secondo.addActionListener(this);
    
    this.jbutCopy2Postgres = new JButton(new ImageIcon(ActionToolbar.class.getResource("/convert2Postgis.gif")));  
    this.jbutCopy2Postgres.setToolTipText("Open selection dialog to convert to PostgreSQL db");
    this.jbutCopy2Postgres.addActionListener(this);
    
//    this.jbutReonnectSecondo = new JButton(new ImageIcon(ActionToolbar.class.getResource("/try2reconnect_Secondo.gif")));
//    this.jbutReonnectSecondo.setToolTipText("Try to Reconnet the SECONDO-Server");
//    this.jbutReonnectSecondo.addActionListener(this);
//    
//
//    this.jbutReonnectPostgres = new JButton(new ImageIcon(ActionToolbar.class.getResource("/try2reconnect_postgis.gif")));
//    this.jbutReonnectPostgres.setToolTipText("Try to Reconnect the PostgreSQL/ PostGIS-Server");
//    this.jbutReonnectPostgres.addActionListener(this);
//    

    this.jtoolbar.add(this.jbutCopy2Secondo);
    this.jtoolbar.add(this.jbutCopy2Postgres);
    this.jtoolbar.addSeparator();
   // this.jtoolbar.add(this.jbutReonnectSecondo);
   // this.jtoolbar.add(this.jbutReonnectPostgres);
  }
  
  public void actionPerformed(ActionEvent e)
  {
    if (e.getSource() == this.jbutCopy2Secondo) {
      this.mgui.showRightConvert();
    } 
    else if (e.getSource() == this.jbutCopy2Postgres) {
      this.mgui.showLeftConvert();
    }
//    else if (e.getSource() == this.jbutReonnectSecondo) {
//      new RefreshLeftSideTree(this.mgui).start();
//    } 
//    else if (e.getSource() == this.jbutReonnectPostgres) {
//      new RefreshRightSideTree(this.mgui).start();
//    }
  }
}
