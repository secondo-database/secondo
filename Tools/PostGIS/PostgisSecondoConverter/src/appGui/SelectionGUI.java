package appGui;



import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.StringReader;
import java.util.logging.Logger;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;
import postgis.IPGTextMessages;
import secondo.ISECTextMessages;
import secondoPostgisUtil.IDelimiter;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.IPublicTextMessages;
import appGuiUtil.JSplitPainInTheAss;
import appGuiUtil.ProgressBarGUI;

public abstract class SelectionGUI
  implements IGlobalParameters, IPGTextMessages, ISECTextMessages, IDelimiter, IPublicTextMessages
{
  public JFrame mPostgreSeclectionFrame;
  JPanel panelMainFrame;
  JLabel jlabeltableNameOldDB;
  JLabel jlabeltableNameNewDB;
  JLabel jlabeltableNameOld;
  JTextField jtextTableNewName;
  JPanel panelRadioButtons;
  JButton jButRun;
  JButton jButCancel;
  JPanel panelCommandButtons;
  JList jlistTableColumnATDB;
  JList jlistTableColumnTODB;
  JScrollPane jScrollListLinks;
  JScrollPane jScrollListRechts;
  JLabel lblATDB;
  JLabel lblTODB;
  JButton jButAddToLinks;
  JButton jButAddAllToLinks;
  JButton jButClearLinks;
  JButton jButClearAllLinks;
  JScrollPane jSrcollTextArea;
  JTextArea jTextAreaSQL;
  JSplitPane jsplitPaneTBL;
  JPanel panelSplitLinks;
  JPanel panelSplitRechts;
  Component mcmainframe;
  JPopupMenu jpopupTable;
  JMenuItem jMenuItemPopUpExport;
  String strFrameTitle;
  DefaultListModel mdlmTODB;
  DefaultListModel mdlmATDB;
  StringBuffer msbTblColDelimitter;
  StringBuffer msbDBName;
  StringBuffer msbTBLName;
  StringBuffer msbConditionFieldText;
  ProgressBarGUI mprogbg;
  
  public SelectionGUI(StringBuffer _sbDBName, StringBuffer _sbTBLName, MainGui _mgui)
  {
    this.mprogbg = new ProgressBarGUI("", "");
    
    this.msbDBName = new StringBuffer(_sbDBName);
    this.msbTBLName = new StringBuffer(_sbTBLName);
    
    this.mPostgreSeclectionFrame = new JFrame(_sbDBName.toString());
    this.panelMainFrame = new JPanel(new BorderLayout(5, 5));
    
    this.panelRadioButtons = new JPanel(new GridLayout(0, 4, 5, 5));
    this.panelCommandButtons = new JPanel(new GridLayout(0, 3, 5, 5));
    
    this.panelSplitLinks = new JPanel(new BorderLayout(5, 5));
    this.panelSplitRechts = new JPanel(new BorderLayout(5, 5));
    
    this.mdlmATDB = new DefaultListModel();
    this.mdlmTODB = new DefaultListModel();
    
    this.msbTblColDelimitter = new StringBuffer(" - ");
    this.msbConditionFieldText = new StringBuffer();
  }
  
  public void init(DefaultListModel _dlm)
  {
    LogFileHandler.mlogger.info("try to show selection gui");
    

    this.jlabeltableNameOldDB = new JLabel("Table/Object name:");
    this.jlabeltableNameNewDB = new JLabel("Table/Object name:");
    this.jlabeltableNameOld = new JLabel(this.msbTBLName.toString());
    this.jtextTableNewName = new JTextField(this.msbTBLName.toString());
    
    this.panelRadioButtons.add(this.jlabeltableNameOldDB);
    this.panelRadioButtons.add(this.jlabeltableNameOld);
    this.panelRadioButtons.add(this.jlabeltableNameNewDB);
    this.panelRadioButtons.add(this.jtextTableNewName);
    

    this.jButRun = new JButton("Copy...");
    this.jButRun.addActionListener(this.alRadioButtons);
    this.jButCancel = new JButton("Cancel");
    this.jButCancel.addActionListener(this.alRadioButtons);
    
    this.panelCommandButtons.add(this.jButRun);
    this.panelCommandButtons.add(new JLabel());
    this.panelCommandButtons.add(this.jButCancel);
    

    this.jTextAreaSQL = new JTextArea(3, 1);
    this.jSrcollTextArea = new JScrollPane(this.jTextAreaSQL);
    this.jSrcollTextArea.setPreferredSize(new Dimension(1, 100));
    


    this.jButAddToLinks = new JButton(">");
    this.jButAddToLinks.addActionListener(this.alRadioButtons);
    this.jButAddAllToLinks = new JButton(">>");
    this.jButAddAllToLinks.addActionListener(this.alRadioButtons);
    
    this.jButClearLinks = new JButton("remove");
    this.jButClearLinks.addActionListener(this.alRadioButtons);
    this.jButClearAllLinks = new JButton("remove all");
    this.jButClearAllLinks.addActionListener(this.alRadioButtons);
    
    this.lblATDB = new JLabel("AT DB - " + this.msbDBName.toString(), 0);
    this.lblTODB = new JLabel("TO DB", 0);
    
    this.jlistTableColumnATDB = new JList(this.mdlmATDB);
    this.jlistTableColumnATDB.setToolTipText("Choose some columns to convert these to the other database");
    this.jlistTableColumnATDB.setSelectionMode(2);
    this.jlistTableColumnATDB.setLayoutOrientation(0);
    this.jlistTableColumnATDB.setVisibleRowCount(-1);
    
    this.jlistTableColumnTODB = new JList(this.mdlmTODB);
    this.jlistTableColumnTODB.setToolTipText("Columns which will be added to the other database");
    this.jlistTableColumnTODB.setSelectionMode(2);
    this.jlistTableColumnTODB.setLayoutOrientation(0);
    this.jlistTableColumnTODB.setVisibleRowCount(-1);
    

    this.jScrollListLinks = new JScrollPane(this.jlistTableColumnATDB);
    this.jScrollListRechts = new JScrollPane(this.jlistTableColumnTODB);
    
    this.panelSplitLinks.add(this.lblATDB, "North");
    this.panelSplitLinks.add(this.jScrollListLinks, "Center");
    JPanel panelButtonsLinks = new JPanel(new FlowLayout());
    panelButtonsLinks.setComponentOrientation(ComponentOrientation.LEFT_TO_RIGHT);
    panelButtonsLinks.add(this.jButAddToLinks);
    panelButtonsLinks.add(this.jButAddAllToLinks);
    
    this.panelSplitLinks.add(panelButtonsLinks, "South");
    
    this.panelSplitRechts.add(this.lblTODB, "North");
    this.panelSplitRechts.add(this.jScrollListRechts, "Center");
    JPanel panelButtonsRechts = new JPanel(new FlowLayout());
    panelButtonsRechts.setComponentOrientation(ComponentOrientation.LEFT_TO_RIGHT);
    panelButtonsRechts.add(this.jButClearLinks);
    panelButtonsRechts.add(this.jButClearAllLinks);
    
    this.panelSplitRechts.add(panelButtonsRechts, "South");
    

    this.jsplitPaneTBL = new JSplitPane(1, 
      this.panelSplitLinks, this.panelSplitRechts);
    
    this.jsplitPaneTBL.setOneTouchExpandable(true);
    this.jsplitPaneTBL.setPreferredSize(new Dimension(600, 500));
    JSplitPainInTheAss.setDividerLocation(this.jsplitPaneTBL, 0.5D);
    

    this.strFrameTitle = ("Selection view of database " + this.msbDBName.toString());
    this.mPostgreSeclectionFrame.setTitle(this.strFrameTitle);
    
    this.panelMainFrame.add(this.panelRadioButtons, "First");
    
    JPanel panelCenter = new JPanel(new BorderLayout(5, 5));
    panelCenter.add(this.jsplitPaneTBL, "First");
    panelCenter.add(this.jSrcollTextArea, "Last");
    
    this.panelMainFrame.add(panelCenter, "Center");
    
    this.panelMainFrame.add(this.panelCommandButtons, "Last");
    

    panelButtonsLinks.setBackground(Color.yellow);
    
    panelButtonsRechts.setBackground(Color.yellow);
    
    panelCenter.setBackground(Color.blue);
    
    this.panelMainFrame.setBackground(Color.blue);
    this.panelSplitLinks.setBackground(Color.yellow);
    this.panelSplitRechts.setBackground(Color.yellow);
    
    this.mPostgreSeclectionFrame.add(this.panelMainFrame);
    this.mPostgreSeclectionFrame.pack();
    if (this.mcmainframe == null)
    {
      Dimension d = this.mPostgreSeclectionFrame.getToolkit().getScreenSize();
      
      this.mPostgreSeclectionFrame.setLocation((int)((d.getWidth() - this.mPostgreSeclectionFrame.getWidth()) / 2.0D), 
        (int)((d.getHeight() - this.mPostgreSeclectionFrame.getHeight()) / 2.0D));
    }
    else
    {
      this.mPostgreSeclectionFrame.setLocationRelativeTo(this.mcmainframe);
    }
    this.mPostgreSeclectionFrame.setIconImage(gimp_S2P);
    this.mPostgreSeclectionFrame.setVisible(true);
    for (int k = 0; k < _dlm.size(); k++) {
      this.mdlmATDB.addElement(_dlm.get(k));
    }
  }
  
  public void readConditionField()
  {
    this.msbConditionFieldText.delete(0, this.msbConditionFieldText.length());
    
    LineNumberReader lnr = new LineNumberReader(new StringReader(this.jTextAreaSQL.getText()));
    try
    {
      String line;
      while ((line = lnr.readLine()) != null)
      {
       System.out.println("SelectionGUI.java: " +line);
        this.msbConditionFieldText.append(line);
        this.msbConditionFieldText.append(" ");
      }
      if (this.msbConditionFieldText.length() > 1) {
        this.msbConditionFieldText.deleteCharAt(this.msbConditionFieldText.length() - 1);
      }
    }
    catch (IOException e1)
    {
      LogFileHandler.mlogger.severe(e1.getMessage());
    }
  }
  
  public boolean issbConditionFieldTextEmpty()
  {
    if (this.msbConditionFieldText.length() > 0) {
      return false;
    }
    return true;
  }
  
  ActionListener alRadioButtons = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == SelectionGUI.this.jButAddToLinks)
      {
        int[] iSelectedIndex = SelectionGUI.this.jlistTableColumnATDB.getSelectedIndices();
        if ((iSelectedIndex != null) && (iSelectedIndex.length > 0)) {
          for (int k = 0; k < iSelectedIndex.length; k++) {
            if (!SelectionGUI.this.mdlmTODB.contains(SelectionGUI.this.mdlmATDB.get(iSelectedIndex[k]))) {
              SelectionGUI.this.mdlmTODB.addElement(SelectionGUI.this.mdlmATDB.get(iSelectedIndex[k]));
            }
          }
        }
      }
      else if (e.getSource() == SelectionGUI.this.jButAddAllToLinks)
      {
        SelectionGUI.this.mdlmTODB.clear();
        for (int k = 0; k < SelectionGUI.this.mdlmATDB.size(); k++) {
          SelectionGUI.this.mdlmTODB.addElement(SelectionGUI.this.mdlmATDB.getElementAt(k));
        }
      }
      else if (e.getSource() == SelectionGUI.this.jButClearLinks)
      {
        int[] iSelectedIndex = SelectionGUI.this.jlistTableColumnTODB.getSelectedIndices();
        if ((iSelectedIndex != null) && (iSelectedIndex.length > 0))
        {
          int iRemoveAnahl = 0;
          for (int k = 0; k < iSelectedIndex.length; k++)
          {
            SelectionGUI.this.mdlmTODB.remove(iSelectedIndex[k] - iRemoveAnahl);
            iRemoveAnahl++;
          }
        }
      }
      else if (e.getSource() == SelectionGUI.this.jButClearAllLinks)
      {
        SelectionGUI.this.mdlmTODB.clear();
      }
      else if (e.getSource() == SelectionGUI.this.jButRun)
      {
        if (SelectionGUI.this.jlistTableColumnTODB.getModel().getSize() == 0) {
          return;
        }
        Thread th = new Thread(SelectionGUI.this.runConvert);
        th.start();
      }
      else if (e.getSource() == SelectionGUI.this.jButCancel)
      {
        SelectionGUI.this.mPostgreSeclectionFrame.setVisible(false);
      }
    }
  };
  
  public DefaultListModel convertDefaultTreeNodeModelToDefaultListModel(DefaultMutableTreeNode _dmtn, String strDBName, String _strSelTBL)
  {
    int iSelctionIndex = 0;
    for (int i = 0; i < _dmtn.getChildCount(); i++) {
      if (strDBName.equals(_dmtn.getChildAt(i).toString()))
      {
        iSelctionIndex = i;
        break;
      }
    }
    DefaultListModel dlm = new DefaultListModel();
    
    StringBuffer sbAdd2Dlm = new StringBuffer();
    StringBuffer sbTBL = new StringBuffer();
    StringBuffer sbCOL = new StringBuffer();
    if (_dmtn.getChildCount() > 0) {
      for (int k = 0; k < _dmtn.getChildAt(iSelctionIndex).getChildCount(); k++) {
        if (_dmtn.getChildAt(iSelctionIndex).getChildAt(k).toString().intern() == _strSelTBL.intern())
        {
          sbAdd2Dlm.delete(0, sbAdd2Dlm.length());
          sbTBL.delete(0, sbTBL.length());
          sbTBL.append(_dmtn.getChildAt(iSelctionIndex).getChildAt(k));
          sbTBL.append(this.msbTblColDelimitter);
          for (int g = 0; g < _dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildCount(); g++)
          {
            sbAdd2Dlm.delete(0, sbAdd2Dlm.length());
            sbCOL.delete(0, sbCOL.length());
            sbCOL.append(_dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g));
            
            sbAdd2Dlm.append(sbTBL);
            sbAdd2Dlm.append(sbCOL);
            
            dlm.addElement(new StringBuffer(sbAdd2Dlm));
          }
        }
      }
    }
    return dlm;
  }
  
  public Runnable runConvert = new Runnable()
  {
    public void run()
    {
      SelectionGUI.this.startConverting(0);
    }
  };
  
  public abstract boolean startConverting(int paramInt);
}
