package appGui;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.logging.Logger;
import javax.swing.JMenuItem;
import secondoPostgisUtil.LogFileHandler;

public class TableSECConvertGUI
  extends TableGUI
{
  JMenuItem jMenuItemPopUpConvert;
  MainGui mgui;
  
  public TableSECConvertGUI(MainGui _mgui)
  {
    this.jMenuItemPopUpConvert = new JMenuItem("Convert...");
    this.jMenuItemPopUpConvert.addActionListener(this.alPopup);
    
    this.mgui = _mgui;
    


    LogFileHandler.mlogger.info("Construct TableSECConvertGUI(mgui)");
  }
  
  public TableSECConvertGUI(Component c)
  {
    super(c);
    this.jMenuItemPopUpConvert = new JMenuItem("Convert...");
    this.jMenuItemPopUpConvert.addActionListener(this.alPopup);
    

    LogFileHandler.mlogger.info("Construct TableSECConvertGUI(c)");
  }
  
  ActionListener alPopup = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == TableSECConvertGUI.this.jMenuItemPopUpExport) {
        TableSECConvertGUI.this.exportRows();
      } else {
        e.getSource();
      }
    }
  };
}
