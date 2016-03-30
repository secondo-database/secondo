package appGui;



import convert.SecondoTypes;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.logging.Logger;

import javax.swing.BorderFactory;
import javax.swing.DefaultComboBoxModel;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JSpinner.DefaultEditor;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListModel;
import javax.swing.SpinnerNumberModel;

import secondo.ConnectSecondo;
import secondo.ISECTextMessages;
import secondo.SecondoObjectInfoClass;
import sj.lang.IntByReference;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import secondoPostgisUtil.UtilFunctions;
import appGuiUtil.JCheckBoxList;
import appGuiUtil.JCheckboxWithObject;
import appGuiUtil.Message;
import appGuiUtil.ProgressBarGUI;
import appGuiUtil.RefreshLeftSideTree;
import appGuiUtil.SetUI;
import appGuiUtil.Warning;

public class GenerateMOGUI
  implements IGlobalParameters, ISECTextMessages
{
  ProgressBarGUI mprogbg;
  private final String strSORTASC = " asc";
  private final String strSORTDSC = " dsc";
  private String mstrObjName;
  private String mstrDBName;
  JFrame mjfrmMOGUI;
  JPanel jPanelTimeInstant;
  JPanel jPanelSortBy;
  JPanel jPanelGroupBy;
  JPanel jPanelCanbeMovingObjects;
  JPanel jPanelButtons;
  JPanel jPanelNewName;
  JTextField jtextNewName;
  JLabel jlabEnterNewName;
  JTextField jtextDuration;
  JLabel jlabEnterDuration;
  JPanel jPanelMain;
  JComboBox jcomboInstantColumn;
  JButton jButOK;
  JButton jButCancel;
  JScrollPane jscrollTextSortby;
  JTextArea jtextareaSortBy;
  JList jlistSortBy;
  JComboBox jComboxSortbyColumns;
  JButton jButAddSortBy;
  JButton jButRemoveSortBy;
  JScrollPane jscrollTextOrderBy;
  JTextArea jtextareaGroupBy;
  JList jlistGroupBy;
  JComboBox jComboxGroupByColumns;
  JButton jButGroupByAdd;
  JButton jButGroupByRemove;
  JScrollPane jscrollCheckListCanBeMO;
  JCheckBoxList jcheckboxlistCanBeMo;
  JSpinner jspinnerDurationDay;
  JSpinner jspinnerDurationMS;
  JCheckBox jcheckDuration;
  JPanel jpanelMakeMP;
  JButton jbutMakeMP;
  JButton jbutRemoveMP;
  JScrollPane jscrollMakeMP;
  JList jlistMakeMP;
  DefaultComboBoxModel mdefaultComboBoxInstant;
  DefaultComboBoxModel mdefaultComboSortBy;
  DefaultComboBoxModel mdefaultComboGroupBy;
  DefaultComboBoxModel mdefaultComboMPX;
  DefaultComboBoxModel mdefaultComboMPY;
  
  DefaultListModel mdefaultListModelColumnsCanbeMo;
  DefaultListModel mdefaultListModelMakeMP;
  DefaultListModel mdefaultListModelSortBy;
  DefaultListModel mdefaultListModelGroupBy;
  
  SecondoObjectInfoClass msecondoObj;
  SecondoTypes msecondoTypes;
  MainGui mgui;
  AskForSecTblName maskObjName;
  
  public GenerateMOGUI(MainGui _mgui)
  {
    this.mgui = _mgui;
    this.maskObjName = new AskForSecTblName();
    this.msecondoTypes = new SecondoTypes();
    this.mdefaultListModelMakeMP = new DefaultListModel<>();
    this.mdefaultListModelSortBy = new DefaultListModel();
    this.mdefaultListModelGroupBy = new DefaultListModel();
    
    this.mjfrmMOGUI = new JFrame();
    this.mjfrmMOGUI.setTitle("Generate Moving Objects");
    
    this.jPanelMain = new JPanel(new BorderLayout(5, 5));
    
    this.jPanelNewName = new JPanel(new GridLayout(0, 2, 5, 5));
    this.jPanelNewName.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "Moving object name", 1, 2));
    

    this.jPanelTimeInstant = new JPanel(new GridLayout(0, 2, 5, 5));
    this.jPanelTimeInstant.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "Choose instant column ", 1, 2));
    
    this.jPanelSortBy = new JPanel(new BorderLayout(5, 5));
    this.jPanelSortBy.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "sortby defintion ", 1, 2));
    
    this.jPanelGroupBy = new JPanel(new BorderLayout(5, 5));
    this.jPanelGroupBy.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "groupby defintion ", 1, 2));
    

    this.jPanelCanbeMovingObjects = new JPanel(new GridLayout(0, 1, 5, 5));
    this.jPanelCanbeMovingObjects.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "existing columns", 1, 2));
    
    this.jpanelMakeMP = new JPanel(new BorderLayout());
    

    this.jpanelMakeMP.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "make moving point from two columns ", 1, 2));
    
    this.jPanelButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    this.msecondoObj = new SecondoObjectInfoClass();
  }
  
  public void init(String _strRelationName, String _strDatabaseName)
  {
    this.mstrObjName = _strRelationName;
    this.mstrDBName = _strDatabaseName;
    
    this.mjfrmMOGUI.setTitle("Generate Moving Objects - db: " + _strDatabaseName + " Relation: " + _strRelationName);
    
    this.msecondoObj = getMySecondoObject();
    
    fillDLMCanBeMO();
    fillMPColumns();
    fillInstantCombo();
    fillSortByCombo();
    fillGroupByCombo();
    
    this.jtextNewName = new JTextField();
    this.jlabEnterNewName = new JLabel("Enter new relation (object) name:");
    
    this.jtextDuration = new JTextField();
    this.jlabEnterDuration = new JLabel("Duration split value: (days) (milliseconds)");
    
    this.jPanelNewName.add(this.jlabEnterNewName);
    this.jPanelNewName.add(this.jtextNewName);
    this.jPanelNewName.add(this.jlabEnterDuration);
    
    this.jspinnerDurationDay = new JSpinner(new SpinnerNumberModel(0, 0, 2147483647, 1));
    
    ((JSpinner.DefaultEditor)this.jspinnerDurationDay.getEditor()).getTextField()
      .addKeyListener(new KeyAdapter()
      {
        public void keyTyped(KeyEvent e)
        {
          if (!Character.isDigit(e.getKeyChar())) {
            e.consume();
          }
        }
      });
    this.jspinnerDurationMS = new JSpinner(new SpinnerNumberModel(0, 0, 2147483647, 1));
    
    ((JSpinner.DefaultEditor)this.jspinnerDurationMS.getEditor()).getTextField()
      .addKeyListener(new KeyAdapter()
      {
        public void keyTyped(KeyEvent e)
        {
          if (!Character.isDigit(e.getKeyChar())) {
            e.consume();
          }
        }
      });
    JPanel jpanelDuration = new JPanel(new GridLayout(0, 3, 5, 5));
    this.jcheckDuration = new JCheckBox("Use");
    //this.jcheckDuration.setSelected(false);
    this.jcheckDuration.setSelected(true); //EN
    this.jcheckDuration.addActionListener(this.actionListenerButtons);
    jpanelDuration.add(this.jcheckDuration);
    jpanelDuration.add(this.jspinnerDurationDay);
    jpanelDuration.add(this.jspinnerDurationMS);
    

    this.jspinnerDurationDay.setEnabled(this.jcheckDuration.isSelected());
    this.jspinnerDurationMS.setEnabled(this.jcheckDuration.isSelected());
    
    this.jPanelNewName.add(jpanelDuration);
    
    this.jcheckboxlistCanBeMo = new JCheckBoxList();
    this.jcheckboxlistCanBeMo.setModel(this.mdefaultListModelColumnsCanbeMo);
    
    this.jscrollCheckListCanBeMO = new JScrollPane(this.jcheckboxlistCanBeMo);
    this.jPanelCanbeMovingObjects.add(this.jscrollCheckListCanBeMO);
    
    this.jcomboInstantColumn = new JComboBox(this.mdefaultComboBoxInstant);
    this.jcomboInstantColumn.addItemListener(this.itemListenerInstant);
    this.jPanelTimeInstant.add(this.jcomboInstantColumn);
    

    this.jbutMakeMP = new JButton("Make moving point");
    this.jbutRemoveMP = new JButton("Remove moving point");
    JPanel jpanelMPButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    jpanelMPButtons.add(this.jbutMakeMP);
    jpanelMPButtons.add(this.jbutRemoveMP);
    
    this.jbutMakeMP.addActionListener(this.actionListenerButtons);
    this.jbutRemoveMP.addActionListener(this.actionListenerButtons);
    
    this.jlistMakeMP = new JList(this.mdefaultListModelMakeMP);
    this.jscrollMakeMP = new JScrollPane(this.jlistMakeMP);
    
    this.jpanelMakeMP.add(jpanelMPButtons, "North");
    this.jpanelMakeMP.add(this.jscrollMakeMP, "South");
    
    this.jtextareaSortBy = new JTextArea(2, 2);
    this.jtextareaSortBy.setEnabled(true);
    this.jscrollTextSortby = new JScrollPane(this.jtextareaSortBy);
    

    this.jlistSortBy = new JList(this.mdefaultListModelSortBy);
    this.jlistSortBy.setSelectionMode(0);
    this.jlistSortBy.addMouseListener(this.mlSortByList);
    
    this.jscrollTextSortby = new JScrollPane(this.jlistSortBy);
    

    this.jComboxSortbyColumns = new JComboBox(this.mdefaultComboSortBy);
    this.jButAddSortBy = new JButton("Add");
    this.jButRemoveSortBy = new JButton("Remove");
    
    this.jButAddSortBy.addActionListener(this.actionListenerButtons);
    this.jButRemoveSortBy.addActionListener(this.actionListenerButtons);
    
    this.jPanelSortBy.add(this.jComboxSortbyColumns, "First");
    this.jPanelSortBy.add(this.jButAddSortBy, "West");
    this.jPanelSortBy.add(this.jButRemoveSortBy, "East");
    this.jPanelSortBy.add(this.jscrollTextSortby, "Last");
    

    this.jtextareaGroupBy = new JTextArea(2, 2);
    this.jtextareaGroupBy.setEnabled(true);
    
    this.jlistGroupBy = new JList(this.mdefaultListModelGroupBy);
    this.jscrollTextOrderBy = new JScrollPane(this.jlistGroupBy);
    
    this.jlistGroupBy.setSelectionMode(0);
    
    this.jComboxGroupByColumns = new JComboBox(this.mdefaultComboGroupBy);
    this.jButGroupByAdd = new JButton("Add");
    this.jButGroupByAdd.addActionListener(this.actionListenerButtons);
    this.jButGroupByRemove = new JButton("Remove");
    this.jButGroupByRemove.addActionListener(this.actionListenerButtons);
    
    this.jPanelGroupBy.add(this.jComboxGroupByColumns, "First");
    this.jPanelGroupBy.add(this.jButGroupByAdd, "West");
    this.jPanelGroupBy.add(this.jButGroupByRemove, "East");
    this.jPanelGroupBy.add(this.jscrollTextOrderBy, "Last");
    

    this.jButCancel = new JButton("Cancel");
    this.jButOK = new JButton("Generate moving objects");
    
    this.jButOK.addActionListener(this.actionListenerButtons);
    this.jButCancel.addActionListener(this.actionListenerButtons);
    
    this.jPanelButtons.add(this.jButOK);
    this.jPanelButtons.add(this.jButCancel);
    
    JPanel jpNORTH = new JPanel(new BorderLayout(5, 5));
    JPanel jpCENTER = new JPanel(new BorderLayout(5, 5));
    
    jpNORTH.add(this.jPanelNewName, "North");
    jpNORTH.add(this.jPanelTimeInstant, "Center");
    
    jpCENTER.add(this.jPanelGroupBy, "East");
    jpCENTER.add(this.jPanelSortBy, "West");
    jpCENTER.add(this.jpanelMakeMP, "South");
    

    this.jPanelMain.add(jpNORTH, "North");
    this.jPanelMain.add(jpCENTER, "West");
    this.jPanelMain.add(this.jPanelCanbeMovingObjects, "Center");
    
    this.jPanelMain.add(this.jPanelButtons, "Last");
    this.mjfrmMOGUI.setIconImage(gimp_S2P);
    this.mjfrmMOGUI.add(this.jPanelMain);
    this.mjfrmMOGUI.pack();
    this.mjfrmMOGUI.setVisible(true);
  }
  
  Runnable runPressedOK = new Runnable()
  {
    public void run()
    {
      GenerateMOGUI.this.pressedOK();
    }
  };
  
  private void pressedOK()
  {
    int iSelColumns = 0;
    for (int i = 0; i < this.jcheckboxlistCanBeMo.getModel().getSize(); i++)
    {
      JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.jcheckboxlistCanBeMo.getModel().getElementAt(i);
      if (_jcheckboxObj.isSelected()) {
        iSelColumns++;
      }
    }
    new SetUI().setUIAndLanguage();
    if ((this.jtextNewName.getText() == null) || (this.jtextNewName.getText().toString().length() == 0) || 
      (this.jtextNewName.getText().toString().intern() == this.mstrObjName.intern()) || 
      (!this.maskObjName.checkTblName(this.jtextNewName.getText())))
    {
      new Message("Enter object name. Object name can not be equal to an existing object name or can not use an operator name.");
      return;
    }
    if (this.mdefaultListModelGroupBy.getSize() <= 0)
    {
      new Message("Enter a group by definition.");
      return;
    }
    if ((this.mdefaultListModelMakeMP.getSize() <= 0) && (iSelColumns <= 0))
    {
      new Message("No columns were selected.");
      return;
    }
    if (this.mdefaultComboBoxInstant.getSize() <= 0)
    {
      new Message("No instant attribute available.");
      return;
    }
    for (int i = 0; i < this.mdefaultListModelGroupBy.getSize(); i++) {
      for (int m = 0; m < this.jcheckboxlistCanBeMo.getModel().getSize(); m++)
      {
        JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.jcheckboxlistCanBeMo.getModel().getElementAt(m);
        if ((_jcheckboxObj.isSelected()) && (this.mdefaultListModelGroupBy.get(i).toString().intern() == _jcheckboxObj.getText().toString().intern()))
        {
          new Message("You cannot use a column at groupby definition and you cannot select the same column at existing column.");
          return;
        }
      }
    }
    this.mprogbg = new ProgressBarGUI("Gernate MO", "Converting to " + this.jtextNewName.getText().toString());
    

    String strNewOBJName = this.jtextNewName.getText().toString();
    int iDurationDay = 0;
    iDurationDay = Integer.valueOf(this.jspinnerDurationDay.getValue().toString()).intValue();
    int iDurationMS = 0;
    iDurationMS = Integer.valueOf(this.jspinnerDurationMS.getValue().toString()).intValue();
    
    String strDuration = "";
    

    strDuration = "[const duration value (" + String.valueOf(iDurationDay) + " " + String.valueOf(iDurationMS) + ")]";
    
    /*
     *save the value of items selected from the combobox 
     * 
     */
    String strTimeInstantColumn = this.mdefaultComboBoxInstant.getSelectedItem().toString();
    
    String strsortBy = "sortby[";
    for (int i = 0; i < this.mdefaultListModelSortBy.getSize(); i++)
    {
      strsortBy = strsortBy + this.mdefaultListModelSortBy.get(i);
      if (i < this.mdefaultListModelSortBy.getSize() - 1) {
        strsortBy = strsortBy + ",";
      }
    }
    strsortBy = strsortBy + "]";
    

    String strgroupBy = "groupby[";
    for (int i = 0; i < this.mdefaultListModelGroupBy.getSize(); i++)
    {
      strgroupBy = strgroupBy + this.mdefaultListModelGroupBy.get(i);
      if (i < this.mdefaultListModelGroupBy.getSize() - 1) {
        strgroupBy = strgroupBy + ",";
      }
    }
    strgroupBy = strgroupBy + ";";
    

    /*
     * Value of the Check box will be stored in an ArrayList.
     */
    ArrayList<String> nameOfSelectedItemFromCheckBox = new ArrayList();
    for (int i = 0; i < this.jcheckboxlistCanBeMo.getModel().getSize(); i++)
    {
      JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.jcheckboxlistCanBeMo.getModel().getElementAt(i);
      if (_jcheckboxObj.isSelected()) {
        nameOfSelectedItemFromCheckBox.add(_jcheckboxObj.getText());
      }
    }
    StringBuffer sbgroupbyNew = new StringBuffer();
    for (int i = 0; i < nameOfSelectedItemFromCheckBox.size(); i++)
    {
      sbgroupbyNew.append((String)nameOfSelectedItemFromCheckBox.get(i));
      sbgroupbyNew.append(" : group feed approximate["); 						/* Approximate is generated */
      sbgroupbyNew.append(strTimeInstantColumn);
      sbgroupbyNew.append(", ");
      sbgroupbyNew.append((String)nameOfSelectedItemFromCheckBox.get(i));
      if (this.jcheckDuration.isSelected())
      {
        sbgroupbyNew.append(", ");
        sbgroupbyNew.append(strDuration);
      }
      sbgroupbyNew.append("]");
      if (i < nameOfSelectedItemFromCheckBox.size() - 1) {
        sbgroupbyNew.append(",");
      }
    }
    StringBuffer sbExtend = new StringBuffer();
    sbExtend.append("extend[");
    for (int i = 0; i < this.mdefaultListModelMakeMP.getSize(); i++)
    {
      sbExtend.append(this.mdefaultListModelMakeMP.get(i));
      if (sbgroupbyNew.length() > 0) {
        sbgroupbyNew.append(",");
      }
      sbgroupbyNew.append(this.mdefaultListModelMakeMP.get(i).toString().substring(0, this.mdefaultListModelMakeMP.get(i).toString().indexOf(" :")));
      sbgroupbyNew.append(" : group feed approximate[");
      sbgroupbyNew.append(strTimeInstantColumn);
      sbgroupbyNew.append(", ");
      sbgroupbyNew.append(this.mdefaultListModelMakeMP.get(i).toString().substring(0, this.mdefaultListModelMakeMP.get(i).toString().indexOf(" :")));
      if (this.jcheckDuration.isSelected())
      {
        sbgroupbyNew.append(", ");
        sbgroupbyNew.append(strDuration);
      }
      sbgroupbyNew.append("]");
      if (i < this.mdefaultListModelMakeMP.getSize() - 1) {
        sbExtend.append(",");
      }
    }
    sbExtend.append("]");
    strgroupBy = strgroupBy + sbgroupbyNew.toString() + "]";
    
    System.out.println("Etie aproxi" +strNewOBJName);
    



    StringBuffer stringBufferLeft = new StringBuffer();
    
    stringBufferLeft.append("let ");
    stringBufferLeft.append(strNewOBJName);
    stringBufferLeft.append(" =");
    
    stringBufferLeft.append(this.mstrObjName);
    stringBufferLeft.append(" feed ");
    if (this.mdefaultListModelMakeMP.getSize() > 0)
    {
      stringBufferLeft.append(sbExtend.toString());
      stringBufferLeft.append(" ");
    }
    stringBufferLeft.append(strsortBy);
    stringBufferLeft.append(" ");
    stringBufferLeft.append(strgroupBy);
    
    stringBufferLeft.append(" consume");
    
    this.mprogbg.jbutCancel.setEnabled(false);
    this.mprogbg.showProgbar();
    
   
    
    ConnectSecondo connSecondo = null;
    try
    {
      connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
        gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
      if (!connSecondo.connect())
      {
        this.mprogbg.closeProgbar();
        new Warning("The connection to SECONDO database cannot be established."
          		+ "Please check whether the connection's parameters for Secondo are correct.");
      }
      else
      {
    	// The command for generating the m
        connSecondo.sendCommand(new StringBuffer("open database " + this.mstrDBName));
        if (connSecondo.getErrorCode().value != 0)
        {
          this.mprogbg.closeProgbar();
          if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
            connSecondo.closeConnection();
          }
          new Warning("The connection to SECONDO database cannot be established."
          		+ "Please check whether the connection's parameters for Secondo are correct.");
          return;
        }
        connSecondo.setQueryResults2Null();
        
        LogFileHandler.mlogger.info("send: " + stringBufferLeft.toString());
        
        /* Etie start */
        
        System.out.println( "Left " +stringBufferLeft);
        
        /* Etie end */
        
        connSecondo.sendCommand(stringBufferLeft);
        if (connSecondo.getErrorCode().value == 10)
        {
          this.mprogbg.closeProgbar();
          
          LogFileHandler.mlogger.warning("Relation (object) exists");
          
          connSecondo.setQueryResults2Null();
          

          new SetUI().setUIAndLanguage();
          


          int n = JOptionPane.showConfirmDialog(
            null, 
            "Your object name exists. Do you want to overwrite?", 
            "Question", 
            0);
          if (n == 1)
          {
            connSecondo.sendCommand(new StringBuffer("close database"));
            if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
              connSecondo.closeConnection();
            }
            return;
          }
          this.mprogbg = new ProgressBarGUI("Gernate MO", "Converting to " + strNewOBJName);
          this.mprogbg.jbutCancel.setEnabled(false);
          
          this.mprogbg.showProgbar();
          LogFileHandler.mlogger.info("object will be deleted and then newly generated");
          
          connSecondo.setQueryResults2Null();
          connSecondo.sendCommand(new StringBuffer("delete " + strNewOBJName));
          if (connSecondo.getErrorCode().value != 0)
          {
            this.mprogbg.closeProgbar();
            String strTemp = connSecondo.getErrorMessage().toString();
            
            connSecondo.sendCommand(new StringBuffer("close database"));
            if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
              connSecondo.closeConnection();
            }
            new Message(strTemp);
            return;
          }
          connSecondo.setQueryResults2Null();
          connSecondo.sendCommand(stringBufferLeft);
          if (connSecondo.getErrorCode().value != 0)
          {
            this.mprogbg.closeProgbar();
            String strTemp = connSecondo.getErrorMessage().toString();
            
            connSecondo.sendCommand(new StringBuffer("close database"));
            if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
              connSecondo.closeConnection();
            }
            new Message(strTemp);
          }
        }
        else if (connSecondo.getErrorCode().value != 0)
        {
          this.mprogbg.closeProgbar();
          
          String strTemp = connSecondo.getErrorMessage().toString();
          
          connSecondo.sendCommand(new StringBuffer("close database"));
          if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
            connSecondo.closeConnection();
          }
          new Message(strTemp);
          return;
        }
        connSecondo.setQueryResults2Null();
        connSecondo.sendCommand(new StringBuffer("close database"));
        if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
          connSecondo.closeConnection();
        }
        RefreshLeftSideTree refLeft = new RefreshLeftSideTree(this.mgui);
        refLeft.start();
        try
        {
          refLeft.join();
        }
        catch (InterruptedException e)
        {
          LogFileHandler.mlogger.severe(e.getMessage());
          this.mprogbg.closeProgbar();
          new Message(e.getMessage());
          
        }//EN
        if ((connSecondo != null) && (connSecondo.isSecondoConnected())) 
        {
            connSecondo.closeConnection();
            return;
        }
        this.mprogbg.closeProgbar();
        
        new SetUI().setUIAndLanguage();
        JOptionPane.showMessageDialog(null, "Moving objects are generated.", "Finish", 1);
        
        LogFileHandler.mlogger.info("MO generated");
      }
    }
    catch (SecurityException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    finally
    {
      if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
        connSecondo.closeConnection();
      }
    }
  }
  
  private void fillDLMCanBeMO()
  {
    this.mdefaultListModelColumnsCanbeMo = new DefaultListModel();
    for (int i = 0; i < this.msecondoObj.sizeTypes(); i++) {
      if (this.msecondoTypes.canBecomeMO(this.msecondoObj.getColTypes(i).toString()))
      {
        JCheckboxWithObject jcheckboxObj = new JCheckboxWithObject(this.msecondoObj.getColName(i).toString());
        jcheckboxObj.setToolTipText(this.msecondoObj.getColTypes(i).toString());
        
        this.mdefaultListModelColumnsCanbeMo.addElement(jcheckboxObj);
      }
    }
  }
  
  private void fillMPColumns()
  {
    this.mdefaultComboMPX = new DefaultComboBoxModel();
    this.mdefaultComboMPY = new DefaultComboBoxModel();
    for (int i = 0; i < this.msecondoObj.sizeTypes(); i++) {
      if ((this.msecondoObj.getColTypes(i).toString().intern() == SecondoTypes.sbSECINT.toString().intern()) || 
        (this.msecondoObj.getColTypes(i).toString().intern() == SecondoTypes.sbSECREAL.toString().intern()))
      {
        this.mdefaultComboMPX.addElement(this.msecondoObj.getColName(i).toString());
        this.mdefaultComboMPY.addElement(this.msecondoObj.getColName(i).toString());
      }
    }
  }
  
  private void fillInstantCombo()
  {
    this.mdefaultComboBoxInstant = new DefaultComboBoxModel();
    for (int i = 0; i < this.msecondoObj.sizeTypes(); i++) {
      if (this.msecondoObj.getColTypes(i).toString().intern() == SecondoTypes.sbSECINSTANT.toString().intern()) {
        this.mdefaultComboBoxInstant.addElement(this.msecondoObj.getColName(i).toString());
        /* Etie start */
        
        System.out.println( "Left " +mdefaultComboBoxInstant);
        
        /* Etie end */
      }
    }
    if (this.mdefaultComboBoxInstant.getSize() > 0) {
      this.mdefaultListModelSortBy.addElement(this.mdefaultComboBoxInstant.getElementAt(0) + " asc");
      
      /* Etie start */
      
      System.out.println( "Left asc " +mdefaultComboBoxInstant);
      
      /* Etie end */
    }
  }
  
  private void fillSortByCombo()
  {
    this.mdefaultComboSortBy = new DefaultComboBoxModel();
    for (int i = 0; i < this.msecondoObj.sizeTypes(); i++) {
      this.mdefaultComboSortBy.addElement(this.msecondoObj.getColName(i).toString());
    }
  }
  
  private void fillGroupByCombo()
  {
    this.mdefaultComboGroupBy = new DefaultComboBoxModel();
    for (int i = 0; i < this.msecondoObj.sizeTypes(); i++) {
      this.mdefaultComboGroupBy.addElement(this.msecondoObj.getColName(i).toString());
      
      /* Etie start */
      
      System.out.println( "Left " +mdefaultComboGroupBy);
      
      /* Etie end */
    }
  }
  
  public void showInputMakePoint()
  {
    JFrame jfrm = new JFrame();
    jfrm.setTitle("Making moving point from 2 columns");
    
    JPanel jpanelfrm = new JPanel(new GridLayout(0, 3, 5, 5));
    
    JComboBox jcomboX = new JComboBox(this.mdefaultComboMPX);
    JComboBox jcomboY = new JComboBox(this.mdefaultComboMPY);
    JButton jButMPOK = new JButton("OK");
    JButton jButMPCancel = new JButton("Cancel");
    final JTextField jtextName = new JTextField();
    
    jpanelfrm.add(new JLabel("point name:"));
    jpanelfrm.add(jtextName);
    jpanelfrm.add(new JLabel(" "));
    jpanelfrm.add(new JLabel("point elements:"));
    jpanelfrm.add(new JLabel("point x"));
    jpanelfrm.add(new JLabel("point y"));
    jpanelfrm.add(new JLabel("choose columns:"));
    jpanelfrm.add(jcomboX);
    jpanelfrm.add(jcomboY);
    jpanelfrm.add(jButMPOK);
    jpanelfrm.add(new JLabel(""));
    jpanelfrm.add(jButMPCancel);
    
    jfrm.add(jpanelfrm);
    
    new SetUI().setUIAndLanguage();
    final JDialog jdialog = new JDialog(jfrm, true);
    
    jButMPCancel.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent arg0)
      {
        jdialog.setVisible(false);
      }
    });
    jButMPOK.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if (jtextName.getText().length() <= 0) {
          return;
        }
        jtextName.setText(new UtilFunctions().firstCharUpperCase(jtextName.getText().toString()));
        
        StringBuffer sbAdd = new StringBuffer();
        
        sbAdd.append(jtextName.getText().toString());
        sbAdd.append(" : ");
        sbAdd.append("makepoint(.");
        sbAdd.append(GenerateMOGUI.this.mdefaultComboMPX.getSelectedItem().toString());
        sbAdd.append(",.");
        sbAdd.append(GenerateMOGUI.this.mdefaultComboMPY.getSelectedItem().toString());
        sbAdd.append(")");
        for (int i = 0; i < GenerateMOGUI.this.jcheckboxlistCanBeMo.getModel().getSize(); i++)
        {
          JCheckboxWithObject _checkbox = (JCheckboxWithObject)GenerateMOGUI.this.jcheckboxlistCanBeMo.getModel().getElementAt(i);
          if (_checkbox.getText().toString().intern() == jtextName.getText().toString().intern())
          {
            new SetUI().setUIAndLanguage();
            new Message("Please choose another attribute name.");
            return;
          }
        }
        for (int i = 0; i < GenerateMOGUI.this.mdefaultListModelMakeMP.size(); i++)
        {
          String str = GenerateMOGUI.this.mdefaultListModelMakeMP.getElementAt(i).toString().substring(0, GenerateMOGUI.this.mdefaultListModelMakeMP.getElementAt(i).toString().indexOf(" :"));
          if (str.intern() == jtextName.getText().toString().intern())
          {
            new SetUI().setUIAndLanguage();
            new Message("Please choose another attribute name.");
            return;
          }
        }
        GenerateMOGUI.this.mdefaultListModelMakeMP.add(0, sbAdd.toString());
        jdialog.setVisible(false);
 /* Etie start */
        
        System.out.println( "makeMP " +mdefaultListModelMakeMP);
        
        /* Etie end */
        
      }
    });
    jdialog.add(jpanelfrm);
    jdialog.setTitle("point from two columns");
    jdialog.pack();
    jdialog.setIconImage(gimp_S2P);
    jdialog.setVisible(true);
  }
  
  ActionListener actionListenerButtons = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == GenerateMOGUI.this.jButOK)
      {
        Thread threat = new Thread(GenerateMOGUI.this.runPressedOK);
        threat.start();
      }
      else if (e.getSource() == GenerateMOGUI.this.jButCancel)
      {
        GenerateMOGUI.this.mjfrmMOGUI.setVisible(false);
      }
      else if (e.getSource() == GenerateMOGUI.this.jButGroupByAdd)
      {
        if (!GenerateMOGUI.this.mdefaultListModelGroupBy.contains(GenerateMOGUI.this.mdefaultComboGroupBy.getSelectedItem())) {
          GenerateMOGUI.this.mdefaultListModelGroupBy.add(0, GenerateMOGUI.this.mdefaultComboGroupBy.getSelectedItem());
        }
      }
      else if (e.getSource() == GenerateMOGUI.this.jButGroupByRemove)
      {
        if (GenerateMOGUI.this.jlistGroupBy.getSelectedIndex() != -1) {
          GenerateMOGUI.this.mdefaultListModelGroupBy.removeElementAt(GenerateMOGUI.this.jlistGroupBy.getSelectedIndex());
        }
      }
      else if (e.getSource() == GenerateMOGUI.this.jButAddSortBy)
      {
        if ((!GenerateMOGUI.this.mdefaultListModelSortBy.contains(GenerateMOGUI.this.mdefaultComboSortBy.getSelectedItem() + " asc")) && 
          (!GenerateMOGUI.this.mdefaultListModelSortBy.contains(GenerateMOGUI.this.mdefaultComboSortBy.getSelectedItem() + " dsc"))) {
          GenerateMOGUI.this.mdefaultListModelSortBy.add(0, GenerateMOGUI.this.mdefaultComboSortBy.getSelectedItem() + " asc");
        }
      }
      else if (e.getSource() == GenerateMOGUI.this.jButRemoveSortBy)
      {
        if ((GenerateMOGUI.this.jlistSortBy.getSelectedIndex() != -1) && (GenerateMOGUI.this.jlistSortBy.getSelectedIndex() + 1 != GenerateMOGUI.this.mdefaultListModelSortBy.size())) {
          GenerateMOGUI.this.mdefaultListModelSortBy.removeElementAt(GenerateMOGUI.this.jlistSortBy.getSelectedIndex());
        }
      }
      else if (e.getSource() == GenerateMOGUI.this.jcheckDuration)
      {
        GenerateMOGUI.this.jspinnerDurationDay.setEnabled(GenerateMOGUI.this.jcheckDuration.isSelected());
        GenerateMOGUI.this.jspinnerDurationMS.setEnabled(GenerateMOGUI.this.jcheckDuration.isSelected());
      }
      else if (e.getSource() == GenerateMOGUI.this.jbutMakeMP)
      {
        GenerateMOGUI.this.showInputMakePoint();
      }
      else if (e.getSource() == GenerateMOGUI.this.jbutRemoveMP)
      {
        if (GenerateMOGUI.this.jlistMakeMP.getSelectedIndex() != -1) {
          GenerateMOGUI.this.mdefaultListModelMakeMP.removeElementAt(GenerateMOGUI.this.jlistMakeMP.getSelectedIndex());
        }
      }
    }
  };
  private ItemListener itemListenerInstant = new ItemListener()
  {
    public void itemStateChanged(ItemEvent e)
    {
      if (e.getStateChange() == 1)
      {
        GenerateMOGUI.this.mdefaultListModelSortBy.removeElementAt(GenerateMOGUI.this.mdefaultListModelSortBy.getSize() - 1);
        GenerateMOGUI.this.mdefaultListModelSortBy.add(GenerateMOGUI.this.mdefaultListModelSortBy.getSize(), e.getItem().toString() + " asc");
 /* Etie start */
        
  //      System.out.println( "Sortby " +mdefaultListModelSortBy);
        
        /* Etie end */
      }
    }
  };
  private MouseListener mlSortByList = new MouseListener()
  {
    public void mouseReleased(MouseEvent arg0) {}
    
    public void mousePressed(MouseEvent arg0) {}
    
    public void mouseExited(MouseEvent arg0) {}
    
    public void mouseEntered(MouseEvent arg0) {}
    
    public void mouseClicked(MouseEvent arg0)
    {
      if (arg0.getClickCount() == 2)
      {
        JList jlist = (JList)arg0.getSource();
        
        /* Declaration of the index counter EN*/
        int iindex = jlist.locationToIndex(arg0.getPoint());
        if (iindex == GenerateMOGUI.this.mdefaultListModelSortBy.getSize() - 1) {
          return;
        }
        String str = GenerateMOGUI.this.mdefaultListModelSortBy.get(iindex).toString();
        if (str.endsWith(" asc"))
        {
          str = str.substring(0, str.length() - " asc".length()) + " desc";
          GenerateMOGUI.this.mdefaultListModelSortBy.set(iindex, str);
        }
        else
        {
          str = str.substring(0, str.length() - " desc".length()) + " asc";
          GenerateMOGUI.this.mdefaultListModelSortBy.set(iindex, str);
        }
      }
    }
  };
  
  private SecondoObjectInfoClass getMySecondoObject()
  {
    ConnectSecondo connSecondo = null;
    
    LinkedList<SecondoObjectInfoClass> llsecObjInfo = new LinkedList();
    try
    {
      connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
        gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
      if (!connSecondo.connect())
      {
        new Warning("Can not connect to SECONDO database."
        		+ "Please check whether the connection parameters are correct.");
      }
      else
      {
        llsecObjInfo = connSecondo.getObjectsWithOutCount(this.mstrDBName, this.mstrObjName);
        if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
          connSecondo.closeConnection();
        }
      }
    }
    catch (SecurityException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    finally
    {
      if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
        connSecondo.closeConnection();
      }
    }
    SecondoObjectInfoClass secObj = new SecondoObjectInfoClass();
    for (int i = 0; i < llsecObjInfo.size(); i++) {
      if (((SecondoObjectInfoClass)llsecObjInfo.get(i)).getStrObjName().toString().intern() == this.mstrObjName.intern())
      {
        secObj = (SecondoObjectInfoClass)llsecObjInfo.get(i);
        return secObj;
      }
    }
    return secObj;
  }
}
