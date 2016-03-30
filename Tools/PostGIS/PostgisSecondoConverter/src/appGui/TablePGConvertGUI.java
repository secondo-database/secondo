package appGui;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.logging.Logger;
import javax.swing.JMenuItem;
import secondoPostgisUtil.LogFileHandler;

public class TablePGConvertGUI
  extends TableGUI
{
  JMenuItem jMenuItemPopUpConvert;
  MainGui mMain;
  
  public TablePGConvertGUI(MainGui _mgui)
  {
    this.jMenuItemPopUpConvert = new JMenuItem("Convert...");
    this.jMenuItemPopUpConvert.addActionListener(this.alPopup);
    this.mMain = _mgui;
    

    LogFileHandler.mlogger.info("Construct TablePGConvertGUI(gui)");
  }
  
  public TablePGConvertGUI(Component c)
  {
    super(c);
    this.jMenuItemPopUpConvert = new JMenuItem("Convert...");
    this.jMenuItemPopUpConvert.addActionListener(this.alPopup);
    



    LogFileHandler.mlogger.info("Construct TablePGConvertGUI(c)");
  }
  
  private void showTypeSelctionGUI()
  {
    LogFileHandler.mlogger.info("show pg type selection gui");
  }
  
  ActionListener alPopup = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == TablePGConvertGUI.this.jMenuItemPopUpExport) {
        TablePGConvertGUI.this.exportRows();
      } else {
        e.getSource();
      }
    }
  };
}
