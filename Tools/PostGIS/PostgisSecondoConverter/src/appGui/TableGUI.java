package appGui;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.Vector;
import java.util.logging.Logger;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.TableModel;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;

public class TableGUI
  implements MouseListener, IGlobalParameters
{
  public JFrame mTableFrame;
  JPanel panelMainFrame;
  JScrollPane jscrollpaneTable;
  JTable jTable;
  Component mcmainframe;
  JPopupMenu jpopupTable;
  JMenuItem jMenuItemPopUpExport;
  Vector<Vector> rowData;
  Vector<String> vcolumnnames;
  String strFrameTitle;
  
  public TableGUI()
  {
    this.mTableFrame = new JFrame();
    this.panelMainFrame = new JPanel(new BorderLayout(5, 5));
    this.jpopupTable = new JPopupMenu();
    initElements();
  }
  
  public TableGUI(Component cp)
  {
    this.mTableFrame = new JFrame();
    this.panelMainFrame = new JPanel(new BorderLayout());
    this.jpopupTable = new JPopupMenu();
    
    this.mTableFrame.setLocationRelativeTo(cp);
    this.mcmainframe = cp;
    initElements();
  }
  
  private void initElements()
  {
    this.jMenuItemPopUpExport = new JMenuItem("Export rows...");
    this.jMenuItemPopUpExport.addActionListener(this.alPopup);
    this.jpopupTable.add(this.jMenuItemPopUpExport);
  }
  
  public void init(Vector<Vector> _rowData, Vector<String> _vcolumnnames, String _strFrameTitle)
  {
    LogFileHandler.mlogger.info("trying to show table gui");
    
    this.rowData = new Vector();
    this.vcolumnnames = new Vector();
    this.strFrameTitle = "";
    
    this.rowData = _rowData;
    this.vcolumnnames = _vcolumnnames;
    this.strFrameTitle = _strFrameTitle;
    
    this.jTable = new JTable(this.rowData, this.vcolumnnames)
    {
      private static final long serialVersionUID = 1L;
      
      public boolean isCellEditable(int x, int y)
      {
        return false;
      }
    };
    this.jTable.setGridColor(Color.blue);
    this.jTable.setBackground(Color.yellow);
    
    this.jTable.setShowHorizontalLines(true);
    this.jTable.setShowVerticalLines(true);
    this.jTable.setShowGrid(true);
    
    this.jTable.setAutoResizeMode(4);
    this.jTable.addMouseListener(this);
    
    this.jTable.setAutoCreateRowSorter(true);
    
    this.jscrollpaneTable = new JScrollPane(this.jTable);
    
    this.mTableFrame.setTitle(this.strFrameTitle);
    
    this.panelMainFrame.add(new JLabel(), "First");
    this.panelMainFrame.add(new JLabel(), "West");
    this.panelMainFrame.add(new JLabel(), "East");
    this.panelMainFrame.add(new JLabel(), "Last");
    this.panelMainFrame.add(this.jscrollpaneTable, "Center");
    
    this.panelMainFrame.setBackground(Color.blue);
    
    this.mTableFrame.add(this.panelMainFrame);
    
    this.mTableFrame.pack();
    if (this.mcmainframe == null)
    {
      Dimension d = this.mTableFrame.getToolkit().getScreenSize();
      
      this.mTableFrame.setLocation((int)((d.getWidth() - this.mTableFrame.getWidth()) / 2.0D), 
        (int)((d.getHeight() - this.mTableFrame.getHeight()) / 2.0D));
    }
    else
    {
      this.mTableFrame.setLocationRelativeTo(this.mcmainframe);
    }
    this.mTableFrame.setIconImage(gimp_S2P);
    this.mTableFrame.setVisible(true);
  }
  
  public void init(JTable jtblSecondoQuery, String _strFrameTitle)
  {
    LogFileHandler.mlogger.info("try to show table gui");
    
    this.rowData = new Vector();
    this.vcolumnnames = new Vector();
    String strTmp = "";
    for (int i = 0; i < jtblSecondoQuery.getModel().getColumnCount(); i++)
    {
      strTmp = "";
      strTmp = jtblSecondoQuery.getColumnName(i).toString();
      strTmp = strTmp.replace("\n", "");
      
      this.vcolumnnames.addElement(strTmp);
     
    }
    for (int i = 0; i < jtblSecondoQuery.getModel().getRowCount(); i++)
    {
      Vector<String> vtmp = new Vector();
      for (int t = 0; t < jtblSecondoQuery.getModel().getColumnCount(); t++)
      {
        strTmp = jtblSecondoQuery.getModel().getValueAt(i, t).toString();
        strTmp = strTmp.replace("\n", "");
        vtmp.addElement(strTmp);
      }
      this.rowData.addElement(vtmp);
    }
    this.strFrameTitle = "";
    
    this.strFrameTitle = _strFrameTitle;
    this.strFrameTitle = ("SECONDO select View - " + String.valueOf(jtblSecondoQuery.getModel().getRowCount()) + " rows");
    
    this.jTable = jtblSecondoQuery;
    
    this.jTable.setGridColor(Color.blue);
    this.jTable.setBackground(Color.yellow);
    
    this.jTable.setShowHorizontalLines(true);
    this.jTable.setShowVerticalLines(true);
    this.jTable.setShowGrid(true);
    
    this.jTable.setAutoResizeMode(0);
    this.jTable.addMouseListener(this);
    
    this.jscrollpaneTable = new JScrollPane(this.jTable);
    
    this.mTableFrame.setTitle(this.strFrameTitle);
    
    this.panelMainFrame.add(new JLabel(), "First");
    this.panelMainFrame.add(new JLabel(), "West");
    this.panelMainFrame.add(new JLabel(), "East");
    this.panelMainFrame.add(new JLabel(), "Last");
    this.panelMainFrame.add(this.jscrollpaneTable, "Center");
    
    this.panelMainFrame.setBackground(Color.blue);
    
    this.mTableFrame.add(this.panelMainFrame);
    
    this.mTableFrame.pack();
    if (this.mcmainframe == null)
    {
      Dimension d = this.mTableFrame.getToolkit().getScreenSize();
      
      this.mTableFrame.setLocation((int)((d.getWidth() - this.mTableFrame.getWidth()) / 2.0D), 
        (int)((d.getHeight() - this.mTableFrame.getHeight()) / 2.0D));
    }
    else
    {
      this.mTableFrame.setLocationRelativeTo(this.mcmainframe);
    }
    this.mTableFrame.setIconImage(gimp_S2P);
    this.mTableFrame.setVisible(true);
  }
  
  public void mouseClicked(MouseEvent arg0) {}
  
  public void mouseEntered(MouseEvent arg0) {}
  
  public void mouseExited(MouseEvent arg0) {}
  
  public void mousePressed(MouseEvent arg0) {}
  
  public void mouseReleased(MouseEvent arg0)
  {
    if (arg0.getButton() == 3) {
      this.jpopupTable.show(arg0.getComponent(), arg0.getX(), arg0.getY());
    }
  }
  
  void exportRows()
  {
    LogFileHandler.mlogger.info("export rows");
    
    JFileChooser jfilechooseer = new JFileChooser(System.getProperty("user.home"));
    
    jfilechooseer.setSelectedFile(new File(System.getProperty("user.home") + 
      globalStringBufDirectory + "table_export" + ".txt"));
    
    int ireturnVal = jfilechooseer.showSaveDialog(this.jTable);
    if (ireturnVal == 0)
    {
      File f = jfilechooseer.getSelectedFile();
      OutputStreamWriter writer = null;
      try
      {
        f.createNewFile();
        writer = new OutputStreamWriter(new FileOutputStream(f), "UTF-8");
        for (int i = 0; i < this.vcolumnnames.size(); i++)
        {
          writer.write((String)this.vcolumnnames.get(i));
          writer.write(";");
        }
        writer.write("\r\n");
        if (this.jTable.getSelectedRowCount() == 0)
        {
          for (int i = 0; i < this.rowData.size(); i++)
          {
            Vector vek = (Vector)this.rowData.get(i);
            for (int k = 0; k < vek.size(); k++)
            {
              if (vek.get(k) == null) {
                writer.write("-null-");
              } else {
                writer.write((String)vek.get(k));
              }
              writer.write(";");
            }
            writer.write("\r\n");
          }
        }
        else
        {
          int[] iRows = this.jTable.getSelectedRows();
          if (iRows.length > 0) {
            for (int i = 0; i < iRows.length; i++)
            {
              Vector vek = (Vector)this.rowData.get(iRows[i]);
              for (int k = 0; k < vek.size(); k++)
              {
                if (vek.get(k) == null) {
                  writer.write("-null-");
                } else {
                  writer.write((String)vek.get(k));
                }
                writer.write(";");
              }
              writer.write("\r\n");
            }
          }
        }
        writer.flush();
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
        try
        {
          writer.close();
        }
        catch (IOException e2)
        {
          LogFileHandler.mlogger.severe(e2.getMessage());
        }
      }
      finally
      {
        try
        {
          writer.close();
        }
        catch (IOException e1)
        {
          LogFileHandler.mlogger.severe(e1.getMessage());
        }
      }
    }
  }
  
  ActionListener alPopup = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == TableGUI.this.jMenuItemPopUpExport) {
        TableGUI.this.exportRows();
      }
    }
  };
}
