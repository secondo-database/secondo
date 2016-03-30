package appGui;



import convert.ConvertingSECFile;
import convert.SecondoTypes;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.io.Writer;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JSpinner.DefaultEditor;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListModel;
import javax.swing.SpinnerDateModel;
import javax.swing.SpinnerNumberModel;

import postgis.Add2Postgres;
import postgis.ConnectPostgres;
import postgis.DatabaseName;
import postgis.IPGTextMessages;
import postgis.Tabelle;
import secondo.ConnectSecondo;
import secondo.ISECTextMessages;
import secondo.MySecondoObject;
import secondo.SecondoObjectInfoClass;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import secondoPostgisUtil.IDelimiter;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.ConvertingFinishedGUI;
import appGuiUtil.JCheckBoxList;
import appGuiUtil.JCheckboxWithObject;
import appGuiUtil.Message;
import appGuiUtil.ProgressBarGUI;
import appGuiUtil.RefreshRightSideTree;
import appGuiUtil.SetUI;
import appGuiUtil.Warning;


/*
 * This class convert a Moving Object from Seconto to Postgis 
 * 
 */
public class MO2Postgres
  implements IGlobalParameters, ISECTextMessages, IDelimiter, IPGTextMessages
{
  private String mstrObjName;
  private String mstrObjName_Postgis;
  private String mstrDBName;
  StringBuffer msbDBName;
  MainGui mgui;
  ProgressBarGUI mprogbg;
  JFrame mjframe;
  
  JPanel JPanelNewName;
  
  JPanel mjpanelMain;
  JPanel mjpanelInput;
  JPanel mJpanelButtons;
  JPanel mjpanelEND;
  JPanel mJPanelChooseAttributes;
  
  
  JLabel mjlabelNameTbl;
  JTextField mjtextNameTbl;
  
  /* This will be used for generating a new Table in Secondo */
  JLabel mjlabelNewSecTblName;
  JTextField mjTextNewSecTableName;
  String strNewSecObjName;
  String newSecondoObject;
  
  JLabel mjLabelColumnTimeName;
  JTextField mjtextColumnTimeName;
  JLabel mjlabelTimeInterval;
  JSpinner mjspinnerTimeInterval;
  JLabel mjlabCopy2PostGISTypes;
  JCheckBox mjcheckCopy2PostGISTypes;
  JCheckBoxList mjcheckList;
  JScrollPane mjscrollcheckList;
  DefaultListModel mdlmColumns;
  JTextArea mjtextareaFilter;
  JScrollPane mjscrollTextAreaFilter;
  JButton mjbutSplitUnitPoints;
  JButton mjbutCreateUnitPoints;
  JButton mjbutCancel;
  SecondoObjectInfoClass msecObj;
  SecondoTypes msecTypes;
  Pattern mpDatum = Pattern.compile("([\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[:]{1}[\\d]{2}[:]{1}[\\d]{2}[\\.]{1}[\\d]+[\"]{1}|[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[:]{1}[\\d]{2}[:]{1}[\\d]{2}[\"]{1}|[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[:]{1}[\\d]{2}[\"]{1}|[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[\"]{1})");
  private long mlStartTime = -1L;
  private long mlEndTime = -1L;
  
  ArrayList<String> alStringColumnames = new ArrayList();
  ArrayList<String> alStringMOColumnames = new ArrayList();
  ArrayList<String> alStringNoMOColumnames = new ArrayList();
  
  boolean unitPointCreated = false;
  boolean splitUnitPoints = false;
  
  public MO2Postgres(MainGui _mgui)
  {
    this.mgui = _mgui;
    this.msbDBName = new StringBuffer();
    this.mjframe = new JFrame();
    this.mjpanelMain = new JPanel(new BorderLayout(5, 5));
    this.mjpanelInput = new JPanel(new GridLayout(0, 2, 5, 5));
    
    this.mjpanelInput.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "Input parameter", 1, 2));
    
    this.mJPanelChooseAttributes = new JPanel(new BorderLayout(5, 5));
    this.mJPanelChooseAttributes.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "Attributes / Columns", 1, 2));
    

    this.mjpanelEND = new JPanel(new BorderLayout(5, 5));
    this.mJpanelButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    
    this.msecObj = new SecondoObjectInfoClass();
    this.msecTypes = new SecondoTypes();
  }
  
  public void init(String _strObjName, String _strDBName)
  {
    this.mstrObjName = _strObjName;
    mstrObjName_Postgis = _strObjName;
    this.mstrDBName = _strDBName;
    
    this.msecObj = getMySecondoObject();
    
    fillDLMCanBeMO();
    
    this. mjbutCreateUnitPoints = new JButton("CreateUnitPoints");
    this.mjbutSplitUnitPoints = new JButton("SplitUnitPoints");
    this.mjbutCancel = new JButton("Cancel");
    
    this.mjbutCancel.addActionListener(this.alButtons);
    this.mjbutSplitUnitPoints.addActionListener(this.alButtons);
    this.mjbutCreateUnitPoints.addActionListener(this.alButtons);
    
   // this.JPanelNewName = new JPanel();
    
    this.mjlabelNameTbl = new JLabel("PostgreSQL table name:");
    this.mjtextNameTbl = new JTextField();
    this.mjtextNameTbl.setText(_strObjName);
    this.mjLabelColumnTimeName = new JLabel("Columnname time:");
    this.mjtextColumnTimeName = new JTextField("timecolumn");
    
    
    this.mjlabelNewSecTblName = new JLabel("New Sec Obj with Start & Stop pts");
    this.mjTextNewSecTableName = new JTextField();
    
    
  
    this.mjlabCopy2PostGISTypes = new JLabel("Use PostGIS-Types:");
    this.mjcheckCopy2PostGISTypes = new JCheckBox("yes");
    
    /* EN on 20150724 For testing*/
    this.mjcheckCopy2PostGISTypes.setSelected(true); //false);
    
    this.mjpanelInput.add(this.mjlabelNameTbl);
    this.mjpanelInput.add(this.mjtextNameTbl);
    this.mjpanelInput.add(mjlabelNewSecTblName);
    this.mjpanelInput.add(mjTextNewSecTableName);
    
    
    this.mjpanelInput.add(this.mjLabelColumnTimeName);
    this.mjpanelInput.add(this.mjtextColumnTimeName);
    
   
    
    this.mjpanelInput.add(this.mjlabCopy2PostGISTypes);
    this.mjpanelInput.add(this.mjcheckCopy2PostGISTypes);
    
    this.mJpanelButtons.add(this.mjbutCreateUnitPoints);
    this.mJpanelButtons.add(this.mjbutSplitUnitPoints);
    this.mJpanelButtons.add(this.mjbutCancel);
    
    this.mjtextareaFilter = new JTextArea(3, 1);
    this.mjscrollTextAreaFilter = new JScrollPane(this.mjtextareaFilter);
    this.mjscrollTextAreaFilter.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "filter condition", 1, 2));
    
    this.mjpanelEND.add(this.mjscrollTextAreaFilter, "Center");
    this.mjpanelEND.add(this.mJpanelButtons, "South");
    
    this.mjcheckList = new JCheckBoxList();
    this.mjcheckList.setModel(this.mdlmColumns);
    this.mjscrollcheckList = new JScrollPane(this.mjcheckList);
    
    this.mJPanelChooseAttributes.add(this.mjscrollcheckList);
    
    this.mjpanelMain.add(this.mjpanelInput, "North");
    this.mjpanelMain.add(this.mJPanelChooseAttributes, "Center");
    
    this.mjpanelMain.add(this.mjpanelEND, "South");
    
    this.mjframe.add(this.mjpanelMain);
   // this.mjframe.setTitle("Copy MO to PostgreSQL - " + _strObjName);
    this.mjframe.setTitle("Create Unit Points from a Moving Point - " + _strObjName);
    this.mjframe.setIconImage(gimp_S2P);
    this.mjframe.pack();
    this.mjframe.setVisible(true);
    
    /*EN Adds this for the new Secondo Object from moving Point*
     *  
     */
    strNewSecObjName = this.mjTextNewSecTableName.getText().toString();
  }
  
  private void fillDLMCanBeMO()
  {
    this.mdlmColumns = new DefaultListModel();
    for (int i = 0; i < this.msecObj.sizeTypes(); i++)
    {
      JCheckboxWithObject jcheckboxObj = new JCheckboxWithObject(this.msecObj.getColName(i).toString());
      jcheckboxObj.setToolTipText(this.msecObj.getColTypes(i).toString());
      
      this.mdlmColumns.addElement(jcheckboxObj);
    }
  }
  
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
        new Warning("Can not connect to SECONDO database.\nPlease check the connection to secondo DB. ");
      }
      else
      {
        llsecObjInfo = connSecondo.getObjectsWithOutCount(this.mstrDBName, this.mstrObjName);
        if (connSecondo != null) {
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
 
  public Runnable runCreateUnitPoints = new Runnable()
  {
    public void run()
    {
      MO2Postgres.this.createUnitPoints();
    }
  };
  public Runnable runSplitUnitPoint = new Runnable()
  {
    public void run()
    {
      MO2Postgres.this.splitUnitPoints();
    }
  };
  
  private void createUnitPoints()
  {
    boolean bMOisSelected = false;
    boolean bFilterCondition = false;
    boolean bUsePostGISTypes = false;
    boolean bAttributeEqualTimecolumn = false;
    
    StringBuffer sbTblName = new StringBuffer(this.mjtextNameTbl.getText().toLowerCase());
    StringBuffer sbTimeColumn = new StringBuffer(this.mjtextColumnTimeName.getText());
    strNewSecObjName = this.mjTextNewSecTableName.getText().toString();
   
    
    
    StringBuffer sbMovingObject2Unit = new StringBuffer();
   
    float fIntervalValue = 1.0F;
    /* Etie
    fIntervalValue = Float.valueOf(this.mjspinnerTimeInterval.getValue().toString()).floatValue();*/
    
    bUsePostGISTypes = this.mjcheckCopy2PostGISTypes.isSelected();
    if ((sbTblName.length() == 0) || (sbTimeColumn.length() == 0))
    {
      new Message("Wrong value of one of the inputfields.");
      return;
    }
    
    for (int i = 0; i < this.mjcheckList.getModel().getSize(); i++)
    {
      JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.mjcheckList.getModel().getElementAt(i);
      if (_jcheckboxObj.isSelected())
      {
        if (this.msecTypes.isMOType(this.msecObj.getColTypes(i).toString())) //|| (this.msecTypes.isMOType(this.)))
        {
          bMOisSelected = true;
          alStringMOColumnames.add(_jcheckboxObj.getText());
        }
        else
        {
          alStringNoMOColumnames.add(_jcheckboxObj.getText());
        }
        alStringColumnames.add(_jcheckboxObj.getText());
        if (_jcheckboxObj.getText().intern() == sbTimeColumn.toString().intern()) {
          bAttributeEqualTimecolumn = true;
        }
      }
    }
    if (!bMOisSelected)
    {
      new Message("No Unit object selected.");
      return;
    }
    if (bAttributeEqualTimecolumn)
    {
      new Message("Equal columnname.");
      return;
    }
    
 



    LineNumberReader lnr = new LineNumberReader(new StringReader(this.mjtextareaFilter.getText()));
    

 
   
    
    StringBuffer stringBufferCommand = new StringBuffer();
    StringBuffer stringBufferCommandCount = new StringBuffer();
       
    stringBufferCommand.append("query ");
    stringBufferCommand.append(this.mstrObjName);
    stringBufferCommand.append(" feed ");
    

    stringBufferCommandCount.append(stringBufferCommand.toString());
    if (bFilterCondition)
    {
    //  stringBufferCommand.append(stringBufferFilterField.toString());
      stringBufferCommand.append(" extend[");
      
     // stringBufferCommandCount.append(stringBufferFilterField.toString());
      stringBufferCommandCount.append(" count;");
      

      StringBuffer sbProjectAttr = new StringBuffer();
      for (int i = 0; i < alStringMOColumnames.size(); i++)
      {
        sbProjectAttr.append("Tattr");
        sbProjectAttr.append(i);
        
        stringBufferCommand.append("Tattr");
        stringBufferCommand.append(i);
        stringBufferCommand.append(": deftime(.");
        stringBufferCommand.append((String)alStringMOColumnames.get(i));
        stringBufferCommand.append(")");
        if (i < alStringMOColumnames.size() - 1)
        {
          stringBufferCommand.append(",");
          sbProjectAttr.append(",");
        }
      }
      stringBufferCommand.append(" ] project[");
      stringBufferCommand.append(sbProjectAttr.toString());
      stringBufferCommand.append("]");
    }
    else
    {
    	 

    	
    	
      stringBufferCommand.append("projectextend[;");
      
      stringBufferCommandCount.append(" count;");
      /*
       * Etie process the moving Object
      
      */
     String valueStart = "\"start\"";
      
      for (int i = 0; i < alStringMOColumnames.size(); i++)
      {
        stringBufferCommand.append("Tattr");
        stringBufferCommand.append(i);
        stringBufferCommand.append(": inst(initial(.");
        stringBufferCommand.append((String)alStringMOColumnames.get(i));
        stringBufferCommand.append(")) ,");
        
      
        
        stringBufferCommand.append("Tattrib");
        stringBufferCommand.append(i);
        stringBufferCommand.append(": val(initial(.");
        stringBufferCommand.append((String)alStringMOColumnames.get(i));
        stringBufferCommand.append(")), ");
        stringBufferCommand.append("Kind: " +valueStart);	 // \"stard\"");
        
      }
      
      
      /* Etie added this for the mo */
      String valueEnd = "\"end\"";
      stringBufferCommand.append("] ");
      
   
    ConnectSecondo connSecondo = null;
    File mtempfile = null;
    Writer out = null;
    try
    {
	      try {
			connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
			    gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
	      } catch (NumberFormatException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
	      } catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	      if (!connSecondo.connect()) {
	        new Warning("Can not connect to SECONDO database.\nPlease check whether the connection to secondo database is possible.");
	      }
	      for (;;)
	      {
	        
	        connSecondo.sendCommand(new StringBuffer("open database " + this.mstrDBName));
	        if (connSecondo.getErrorCode().value != 0)
	        {
	          if (connSecondo != null) {
	            connSecondo.closeConnection();
	          }
	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
	        }
	        else
	        {
	          connSecondo.setQueryResults2Null();
	          connSecondo.sendCommand(stringBufferCommandCount);
	          if (connSecondo.getErrorCode().value == 0) {
	            break;
	          }
	          if (connSecondo != null) {
	            connSecondo.closeConnection();
	          }
	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
	        }
	      }
	
	          //Sending generate Units from MovingObjects
	          /* Unit Points created */
	    	
          connSecondo.sendCommand(createUnitFromMovingObjects());
		  if (unitPointCreated == true)
		  {
			  new Message("Table with Unit Points created under Secondo in the \n database " +this.mstrObjName);
			  unitPointCreated = false; 
		  } 
	    }finally{
    		}
    }
 }
  
  private void splitUnitPoints()
  {
    boolean bMOisSelected = false;
    boolean bFilterCondition = false;
    boolean bUsePostGISTypes = false;
    boolean bAttributeEqualTimecolumn = false;
    
    StringBuffer sbTblName = new StringBuffer(this.mjtextNameTbl.getText().toLowerCase());
    StringBuffer sbTimeColumn = new StringBuffer(this.mjtextColumnTimeName.getText());
    strNewSecObjName = this.mjTextNewSecTableName.getText().toString();
   
    
    
    StringBuffer sbMovingObject2Unit = new StringBuffer();
   
    float fIntervalValue = 1.0F;
    /* Etie
    fIntervalValue = Float.valueOf(this.mjspinnerTimeInterval.getValue().toString()).floatValue();*/
    
    bUsePostGISTypes = this.mjcheckCopy2PostGISTypes.isSelected();
    if ((sbTblName.length() == 0) || (sbTimeColumn.length() == 0))
    {
      new Message("Wrong value of one of the inputfields.");
      return;
    }
    
    for (int i = 0; i < this.mjcheckList.getModel().getSize(); i++)
    {
      JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.mjcheckList.getModel().getElementAt(i);
      if (_jcheckboxObj.isSelected())
      {
        if (this.msecTypes.isMOType(this.msecObj.getColTypes(i).toString())) //|| (this.msecTypes.isMOType(this.)))
        {
          bMOisSelected = true;
          alStringMOColumnames.add(_jcheckboxObj.getText());
        }
        else
        {
          alStringNoMOColumnames.add(_jcheckboxObj.getText());
        }
        alStringColumnames.add(_jcheckboxObj.getText());
        if (_jcheckboxObj.getText().intern() == sbTimeColumn.toString().intern()) {
          bAttributeEqualTimecolumn = true;
        }
      }
    }
    if (!bMOisSelected)
    {
      new Message("No moving object selected.");
      return;
    }
    if (bAttributeEqualTimecolumn)
    {
      new Message("Equal columnname.");
      return;
    }
    
 



    LineNumberReader lnr = new LineNumberReader(new StringReader(this.mjtextareaFilter.getText()));
    

 
   
    
    StringBuffer stringBufferCommand = new StringBuffer();
    StringBuffer stringBufferCommandCount = new StringBuffer();
       
    stringBufferCommand.append("query ");
    stringBufferCommand.append(this.mstrObjName);
    stringBufferCommand.append(" feed ");
    

    stringBufferCommandCount.append(stringBufferCommand.toString());
    if (bFilterCondition)
    {
    //  stringBufferCommand.append(stringBufferFilterField.toString());
      stringBufferCommand.append(" extend[");
      
     // stringBufferCommandCount.append(stringBufferFilterField.toString());
      stringBufferCommandCount.append(" count;");
      

      StringBuffer sbProjectAttr = new StringBuffer();
      for (int i = 0; i < alStringMOColumnames.size(); i++)
      {
        sbProjectAttr.append("Tattr");
        sbProjectAttr.append(i);
        
        stringBufferCommand.append("Tattr");
        stringBufferCommand.append(i);
        stringBufferCommand.append(": deftime(.");
        stringBufferCommand.append((String)alStringMOColumnames.get(i));
        stringBufferCommand.append(")");
        if (i < alStringMOColumnames.size() - 1)
        {
          stringBufferCommand.append(",");
          sbProjectAttr.append(",");
        }
      }
      stringBufferCommand.append(" ] project[");
      stringBufferCommand.append(sbProjectAttr.toString());
      stringBufferCommand.append("]");
    }
    else
    {
    	 

    	
    	
      stringBufferCommand.append("projectextend[;");
      
      stringBufferCommandCount.append(" count;");
      /*
       * Etie process the moving Object
      
      */
     String valueStart = "\"start\"";
      
      for (int i = 0; i < alStringMOColumnames.size(); i++)
      {
        stringBufferCommand.append("Tattr");
        stringBufferCommand.append(i);
        stringBufferCommand.append(": inst(initial(.");
        stringBufferCommand.append((String)alStringMOColumnames.get(i));
        stringBufferCommand.append(")) ,");
        
      
        
        stringBufferCommand.append("Tattrib");
        stringBufferCommand.append(i);
        stringBufferCommand.append(": val(initial(.");
        stringBufferCommand.append((String)alStringMOColumnames.get(i));
        stringBufferCommand.append(")), ");
        stringBufferCommand.append("Kind: " +valueStart);	 // \"stard\"");
        
      }
      
      
      /* Etie added this for the mo */
      String valueEnd = "\"end\"";
      stringBufferCommand.append("] ");
      
   
    ConnectSecondo connSecondo = null;
    File mtempfile = null;
    Writer out = null;
    try
    {
	      try {
			connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
			    gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
	      } catch (NumberFormatException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
	      } catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	      if (!connSecondo.connect()) {
	        new Warning("Can not connect to SECONDO database.\nPlease check whether the connection to secondo database is possible.");
	      }
	      for (;;)
	      {
	        
	        connSecondo.sendCommand(new StringBuffer("open database " + this.mstrDBName));
	        if (connSecondo.getErrorCode().value != 0)
	        {
	          if (connSecondo != null) {
	            connSecondo.closeConnection();
	          }
	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
	        }
	        else
	        {
	          connSecondo.setQueryResults2Null();
	          connSecondo.sendCommand(stringBufferCommandCount);
	          if (connSecondo.getErrorCode().value == 0) {
	            break;
	          }
	          if (connSecondo != null) {
	            connSecondo.closeConnection();
	          }
	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
	        }
	      }
	
	          //Sending generate Units from MovingObjects
	          /* Unit Points created */
	    	
          connSecondo.sendCommand(spliteUnitPoint());
		  if (splitUnitPoints == true)
		  {
			  new Message("Table of splited Unit Points created under Secondo with "
			  		+ "\n the relation name " +this.mstrObjName);
			  splitUnitPoints = false; 
		  } 
	    }finally{
    		}
    }
 }

  
  ActionListener alButtons = new ActionListener()
  {
    public void actionPerformed(ActionEvent arg0)
    {
      if (arg0.getSource() == MO2Postgres.this.mjbutCreateUnitPoints)
      {
        Thread th = new Thread(MO2Postgres.this.runCreateUnitPoints);
        th.start();
      }
      else if(arg0.getSource() == MO2Postgres.this.mjbutSplitUnitPoints)
      {
    	  Thread th = new Thread(MO2Postgres.this.runSplitUnitPoint);
    	  th.start();
      }
      else if (arg0.getSource() == MO2Postgres.this.mjbutCancel)
      {
        MO2Postgres.this.mjframe.setVisible(false);
      }
    }
  };
  
	private StringBuffer createUnitFromMovingObjects()
	{
		String newSecondoObject;
		StringBuffer stringBufferCreateUnitsFromMovingObects01 = new StringBuffer();
		
		System.out.println(" Test of new Object " +strNewSecObjName);
		if(strNewSecObjName.length()!= 0){
			newSecondoObject = strNewSecObjName;
		}
		else
		{
			 new Message("Please provide the name of the new Secondo Object ");
		      return new StringBuffer();
		}
		
		/*Start Unitpoint */
		
		stringBufferCreateUnitsFromMovingObects01.append("let " +newSecondoObject +" = ");
		 stringBufferCreateUnitsFromMovingObects01.append(this.mstrObjName);
		 stringBufferCreateUnitsFromMovingObects01.append(" feed ");
		// stringBufferCreateUnitsFromMovingObects.append("projectextend[");
		stringBufferCreateUnitsFromMovingObects01.append("projectextendstream[ ");
		
		
		

	      for (int i = 0; i < alStringNoMOColumnames.size(); i++)
	      {
	    	  stringBufferCreateUnitsFromMovingObects01.append((String)alStringNoMOColumnames.get(i));
	    	  if(i < alStringNoMOColumnames.size()-1)
	    	  {
	    		  stringBufferCreateUnitsFromMovingObects01.append(",");
	    	  }
	    	  else
	    	  {
	    		  stringBufferCreateUnitsFromMovingObects01.append("; ");
	    	  }
	        
	      }
	
	    /*
	     * Etie process the moving Object
	    
	    */
	    
	    for (int i = 0; i < alStringMOColumnames.size(); i++)
	    {
	    	
	    	stringBufferCreateUnitsFromMovingObects01.append("UPoint");
	    	stringBufferCreateUnitsFromMovingObects01.append(i);
	    	stringBufferCreateUnitsFromMovingObects01.append(": units(.");
	    	stringBufferCreateUnitsFromMovingObects01.append((String)alStringMOColumnames.get(i));
	    	stringBufferCreateUnitsFromMovingObects01.append(") "); 
	    }
	    stringBufferCreateUnitsFromMovingObects01.append("]"); 
	    stringBufferCreateUnitsFromMovingObects01.append(" consume;");
	    stringBufferCreateUnitsFromMovingObects01.toString();
		
		System.out.println(" Unit Point created "+stringBufferCreateUnitsFromMovingObects01);
		this.newSecondoObject = null;
		this.newSecondoObject = newSecondoObject;
		/*END Unit*/
		
		  unitPointCreated = true;  
     return stringBufferCreateUnitsFromMovingObects01;
	    
	}
	
	private StringBuffer spliteUnitPoint()
	{
		//String newSecondoObject;
		
		StringBuffer stringBufferCreateUnitsFromMovingObects = new StringBuffer();
		System.out.println(" Test of new Object " +strNewSecObjName);
		if(strNewSecObjName.length()!= 0){
			newSecondoObject = strNewSecObjName;
		}
		else
		{
			 new Message("Please provide the name of the new Secondo Object ");
		      return new StringBuffer();
		}
		stringBufferCreateUnitsFromMovingObects.append("let " +strNewSecObjName +" = ");
		
		//this.mstrObjName = this.newSecondoObject;
		
	  //  stringBufferCreateUnitsFromMovingObects.append(" query ");
		stringBufferCreateUnitsFromMovingObects.append(this.mstrObjName);
		stringBufferCreateUnitsFromMovingObects.append(" feed ");
		stringBufferCreateUnitsFromMovingObects.append("projectextend[");
		//stringBufferCreateUnitsFromMovingObects.append("projectextendstream[ ");
		
		
		

	      for (int i = 0; i < alStringNoMOColumnames.size(); i++)
	      {
	    	  stringBufferCreateUnitsFromMovingObects.append((String)alStringNoMOColumnames.get(i));
	    	  if(i < alStringNoMOColumnames.size()-1)
	    	  {
	    		  stringBufferCreateUnitsFromMovingObects.append(",");
	    	  }
	    	  else
	    	  {
	    		  stringBufferCreateUnitsFromMovingObects.append("; ");
	    	  }
	        
	      } 
	    /*
	     * Etie process the moving Object
	    
	    */
	      
	    
	    String valueStart = "\"start\"";
	    
	    for (int i = 0; i < alStringMOColumnames.size(); i++)
	    {
//	    	stringBufferCreateUnitsFromMovingObects.append("UPoint");
//	    	stringBufferCreateUnitsFromMovingObects.append(i);
//	    	stringBufferCreateUnitsFromMovingObects.append(": units(.");
//	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects.append("), "); 
	    	
	    	stringBufferCreateUnitsFromMovingObects.append("Time");
	    	stringBufferCreateUnitsFromMovingObects.append(i);
	    	stringBufferCreateUnitsFromMovingObects.append(": inst(initial(.");
	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
	    	stringBufferCreateUnitsFromMovingObects.append(")), ");  
	    	stringBufferCreateUnitsFromMovingObects.append("Pos");
	    	stringBufferCreateUnitsFromMovingObects.append(i);
	    	stringBufferCreateUnitsFromMovingObects.append(": val(initial(.");
	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
	    	stringBufferCreateUnitsFromMovingObects.append(")), ");
	    	stringBufferCreateUnitsFromMovingObects.append("Kind: " +valueStart);	 // \"stard\"");
	      
	      
	      /* Etie uncomment this*/
	      /*if (i < alStringMOColumnames.size() - 1) {
	        stringBufferCommand.append(",");
	      }
	      */
	      /*End Of Etien Uncomment this*/
	    }
	    
	    /* Etie added this for the mo */
	    String valueEnd = "\"end\"";
	  //  boolean bf = "-";
	    stringBufferCreateUnitsFromMovingObects.append("] ");
	    
	//    System.out.println(" Initial " +stringBufferCreateUnitsFromMovingObects);
	    
	    stringBufferCreateUnitsFromMovingObects.append(this.mstrObjName);
	    stringBufferCreateUnitsFromMovingObects.append(" feed ");
	   stringBufferCreateUnitsFromMovingObects.append(" projectextend[");
	  //  stringBufferCreateUnitsFromMovingObects.append(" projectextendstream[");

	      for (int i = 0; i < alStringNoMOColumnames.size(); i++)
	      {
	    	  stringBufferCreateUnitsFromMovingObects.append((String)alStringNoMOColumnames.get(i));
	    	  if(i < alStringNoMOColumnames.size()-1)
	    	  {
	    		  stringBufferCreateUnitsFromMovingObects.append(",");
	    	  }
	    	  else
	    	  {
	    		  stringBufferCreateUnitsFromMovingObects.append("; ");
	    	  }
	        
	      }
	    
	    /*
	     * Etie process the moving Object
	    
	    */
	    
	    for (int i = 0; i < alStringMOColumnames.size(); i++)
	    {
//	    	stringBufferCreateUnitsFromMovingObects.append("UPoint");
//	    	stringBufferCreateUnitsFromMovingObects.append(i);
//	    	stringBufferCreateUnitsFromMovingObects.append(": units(.");
//	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects.append("), "); 
	    	stringBufferCreateUnitsFromMovingObects.append("Time");
	    	stringBufferCreateUnitsFromMovingObects.append(i);
	    	stringBufferCreateUnitsFromMovingObects.append(": inst(final(.");
	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
	    	stringBufferCreateUnitsFromMovingObects.append(")), ");
	      
	    	stringBufferCreateUnitsFromMovingObects.append("Pos");
	    	stringBufferCreateUnitsFromMovingObects.append(i);
	    	stringBufferCreateUnitsFromMovingObects.append(": val(final(.");
	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
	    	stringBufferCreateUnitsFromMovingObects.append(")), ");
	    	stringBufferCreateUnitsFromMovingObects.append("Kind: " +valueEnd);
	        
	    }
	    
	    
	    stringBufferCreateUnitsFromMovingObects.append("]");
	    stringBufferCreateUnitsFromMovingObects.append(" concat ");
	    stringBufferCreateUnitsFromMovingObects.append("sortby ");
	    stringBufferCreateUnitsFromMovingObects.append("[Time0 asc] ");
	    stringBufferCreateUnitsFromMovingObects.append(" consume;");
	    stringBufferCreateUnitsFromMovingObects.toString();
	    
	
	  System.out.println(" Initial and Final " +stringBufferCreateUnitsFromMovingObects);
	  this.splitUnitPoints = true;
	
	  
     return stringBufferCreateUnitsFromMovingObects;
	    
	}
}






//package appGui;
//
//
//
//import convert.ConvertingSECFile;
//import convert.SecondoTypes;
//
//import java.awt.BorderLayout;
//import java.awt.Color;
//import java.awt.GridLayout;
//import java.awt.event.ActionEvent;
//import java.awt.event.ActionListener;
//import java.awt.event.KeyAdapter;
//import java.awt.event.KeyEvent;
//import java.io.BufferedWriter;
//import java.io.File;
//import java.io.FileOutputStream;
//import java.io.IOException;
//import java.io.LineNumberReader;
//import java.io.OutputStreamWriter;
//import java.io.StringReader;
//import java.io.Writer;
//import java.sql.Connection;
//import java.sql.SQLException;
//import java.sql.Timestamp;
//import java.text.SimpleDateFormat;
//import java.util.ArrayList;
//import java.util.Date;
//import java.util.HashMap;
//import java.util.Iterator;
//import java.util.LinkedList;
//import java.util.Vector;
//import java.util.logging.Logger;
//import java.util.regex.Matcher;
//import java.util.regex.Pattern;
//
//import javax.swing.BorderFactory;
//import javax.swing.DefaultListModel;
//import javax.swing.JButton;
//import javax.swing.JCheckBox;
//import javax.swing.JDialog;
//import javax.swing.JFormattedTextField;
//import javax.swing.JFrame;
//import javax.swing.JLabel;
//import javax.swing.JOptionPane;
//import javax.swing.JPanel;
//import javax.swing.JScrollPane;
//import javax.swing.JSpinner;
//import javax.swing.JSpinner.DefaultEditor;
//import javax.swing.JTextArea;
//import javax.swing.JTextField;
//import javax.swing.ListModel;
//import javax.swing.SpinnerDateModel;
//import javax.swing.SpinnerNumberModel;
//
//import postgis.Add2Postgres;
//import postgis.ConnectPostgres;
//import postgis.DatabaseName;
//import postgis.IPGTextMessages;
//import postgis.Tabelle;
//import secondo.ConnectSecondo;
//import secondo.ISECTextMessages;
//import secondo.MySecondoObject;
//import secondo.SecondoObjectInfoClass;
//import sj.lang.IntByReference;
//import sj.lang.ListExpr;
//import secondoPostgisUtil.IDelimiter;
//import secondoPostgisUtil.IGlobalParameters;
//import secondoPostgisUtil.LogFileHandler;
//import appGuiUtil.ConvertingFinishedGUI;
//import appGuiUtil.JCheckBoxList;
//import appGuiUtil.JCheckboxWithObject;
//import appGuiUtil.Message;
//import appGuiUtil.ProgressBarGUI;
//import appGuiUtil.RefreshRightSideTree;
//import appGuiUtil.SetUI;
//import appGuiUtil.Warning;
//
//
///*
// * This class convert a Moving Object from Seconto to Postgis 
// * 
// */
//public class MO2Postgres
//  implements IGlobalParameters, ISECTextMessages, IDelimiter, IPGTextMessages
//{
//  private String mstrObjName;
//  private String mstrObjName_Postgis;
//  private String mstrDBName;
//  StringBuffer msbDBName;
//  MainGui mgui;
//  ProgressBarGUI mprogbg;
//  JFrame mjframe;
//  
//  JPanel JPanelNewName;
//  
//  JPanel mjpanelMain;
//  JPanel mjpanelInput;
//  JPanel mJpanelButtons;
//  JPanel mjpanelEND;
//  JPanel mJPanelChooseAttributes;
//  
//  
//  JLabel mjlabelNameTbl;
//  JTextField mjtextNameTbl;
//  
//  /* This will be used for generating a new Table in Secondo */
//  JLabel mjlabelNewSecTblName;
//  JTextField mjTextNewSecTableName;
//  String strNewSecObjName;
//  String newSecondoObject;
//  
//  JLabel mjLabelColumnTimeName;
//  JTextField mjtextColumnTimeName;
//  JLabel mjlabelTimeInterval;
//  JSpinner mjspinnerTimeInterval;
//  JLabel mjlabCopy2PostGISTypes;
//  JCheckBox mjcheckCopy2PostGISTypes;
//  JCheckBoxList mjcheckList;
//  JScrollPane mjscrollcheckList;
//  DefaultListModel mdlmColumns;
//  JTextArea mjtextareaFilter;
//  JScrollPane mjscrollTextAreaFilter;
//  JButton mjbutSplitUnitPoints;
//  JButton mjbutCreateUnitPoints;
//  JButton mjbutCancel;
//  SecondoObjectInfoClass msecObj;
//  SecondoTypes msecTypes;
//  Pattern mpDatum = Pattern.compile("([\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[:]{1}[\\d]{2}[:]{1}[\\d]{2}[\\.]{1}[\\d]+[\"]{1}|[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[:]{1}[\\d]{2}[:]{1}[\\d]{2}[\"]{1}|[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[:]{1}[\\d]{2}[\"]{1}|[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[\"]{1})");
//  private long mlStartTime = -1L;
//  private long mlEndTime = -1L;
//  
//  ArrayList<String> alStringColumnames = new ArrayList();
//  ArrayList<String> alStringMOColumnames = new ArrayList();
//  ArrayList<String> alStringNoMOColumnames = new ArrayList();
//  
//  boolean unitPointCreated = false;
//  boolean splitUnitPoints = false;
//  
//  public MO2Postgres(MainGui _mgui)
//  {
//    this.mgui = _mgui;
//    this.msbDBName = new StringBuffer();
//    this.mjframe = new JFrame();
//    this.mjpanelMain = new JPanel(new BorderLayout(5, 5));
//    this.mjpanelInput = new JPanel(new GridLayout(0, 2, 5, 5));
//    
//    this.mjpanelInput.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "Input parameter", 1, 2));
//    
//    this.mJPanelChooseAttributes = new JPanel(new BorderLayout(5, 5));
//    this.mJPanelChooseAttributes.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "Attributes / Columns", 1, 2));
//    
//
//    this.mjpanelEND = new JPanel(new BorderLayout(5, 5));
//    this.mJpanelButtons = new JPanel(new GridLayout(0, 2, 5, 5));
//    
//    this.msecObj = new SecondoObjectInfoClass();
//    this.msecTypes = new SecondoTypes();
//  }
//  
//  public void init(String _strObjName, String _strDBName)
//  {
//    this.mstrObjName = _strObjName;
//    mstrObjName_Postgis = _strObjName;
//    this.mstrDBName = _strDBName;
//    
//    this.msecObj = getMySecondoObject();
//    
//    fillDLMCanBeMO();
//    
//    this. mjbutCreateUnitPoints = new JButton("CreateUnitPoints");
//    this.mjbutSplitUnitPoints = new JButton("SplitUnitPoints");
//    this.mjbutCancel = new JButton("Cancel");
//    
//    this.mjbutCancel.addActionListener(this.alButtons);
//    this.mjbutSplitUnitPoints.addActionListener(this.alButtons);
//    this.mjbutCreateUnitPoints.addActionListener(this.alButtons);
//    
//   // this.JPanelNewName = new JPanel();
//    
//    this.mjlabelNameTbl = new JLabel("PostgreSQL table name:");
//    this.mjtextNameTbl = new JTextField();
//    this.mjtextNameTbl.setText(_strObjName);
//    this.mjLabelColumnTimeName = new JLabel("Columnname time:");
//    this.mjtextColumnTimeName = new JTextField("timecolumn");
//    
//    
//    this.mjlabelNewSecTblName = new JLabel("New Sec Obj with Start & Stop pts");
//    this.mjTextNewSecTableName = new JTextField();
//    
//    
//  
//    this.mjlabCopy2PostGISTypes = new JLabel("Use PostGIS-Types:");
//    this.mjcheckCopy2PostGISTypes = new JCheckBox("yes");
//    
//    /* EN on 20150724 For testing*/
//    this.mjcheckCopy2PostGISTypes.setSelected(true); //false);
//    
//    this.mjpanelInput.add(this.mjlabelNameTbl);
//    this.mjpanelInput.add(this.mjtextNameTbl);
//    this.mjpanelInput.add(mjlabelNewSecTblName);
//    this.mjpanelInput.add(mjTextNewSecTableName);
//    
//    
//    this.mjpanelInput.add(this.mjLabelColumnTimeName);
//    this.mjpanelInput.add(this.mjtextColumnTimeName);
//    
//   
//    
//    this.mjpanelInput.add(this.mjlabCopy2PostGISTypes);
//    this.mjpanelInput.add(this.mjcheckCopy2PostGISTypes);
//    
//    this.mJpanelButtons.add(this.mjbutCreateUnitPoints);
//    this.mJpanelButtons.add(this.mjbutSplitUnitPoints);
//    this.mJpanelButtons.add(this.mjbutCancel);
//    
//    this.mjtextareaFilter = new JTextArea(3, 1);
//    this.mjscrollTextAreaFilter = new JScrollPane(this.mjtextareaFilter);
//    this.mjscrollTextAreaFilter.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "filter condition", 1, 2));
//    
//    this.mjpanelEND.add(this.mjscrollTextAreaFilter, "Center");
//    this.mjpanelEND.add(this.mJpanelButtons, "South");
//    
//    this.mjcheckList = new JCheckBoxList();
//    this.mjcheckList.setModel(this.mdlmColumns);
//    this.mjscrollcheckList = new JScrollPane(this.mjcheckList);
//    
//    this.mJPanelChooseAttributes.add(this.mjscrollcheckList);
//    
//    this.mjpanelMain.add(this.mjpanelInput, "North");
//    this.mjpanelMain.add(this.mJPanelChooseAttributes, "Center");
//    
//    this.mjpanelMain.add(this.mjpanelEND, "South");
//    
//    this.mjframe.add(this.mjpanelMain);
//   // this.mjframe.setTitle("Copy MO to PostgreSQL - " + _strObjName);
//    this.mjframe.setTitle("Create Unit Points from a Moving Point - " + _strObjName);
//    this.mjframe.setIconImage(gimp_S2P);
//    this.mjframe.pack();
//    this.mjframe.setVisible(true);
//    
//    /*EN Adds this for the new Secondo Object from moving Point*
//     *  
//     */
//    strNewSecObjName = this.mjTextNewSecTableName.getText().toString();
//  }
//  
//  private void fillDLMCanBeMO()
//  {
//    this.mdlmColumns = new DefaultListModel();
//    for (int i = 0; i < this.msecObj.sizeTypes(); i++)
//    {
//      JCheckboxWithObject jcheckboxObj = new JCheckboxWithObject(this.msecObj.getColName(i).toString());
//      jcheckboxObj.setToolTipText(this.msecObj.getColTypes(i).toString());
//      
//      this.mdlmColumns.addElement(jcheckboxObj);
//    }
//  }
//  
//  private SecondoObjectInfoClass getMySecondoObject()
//  {
//    ConnectSecondo connSecondo = null;
//    
//    LinkedList<SecondoObjectInfoClass> llsecObjInfo = new LinkedList();
//    try
//    {
//      connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
//        gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
//      if (!connSecondo.connect())
//      {
//        new Warning("Can not connect to SECONDO database.\nPlease check the connection to secondo DB. ");
//      }
//      else
//      {
//        llsecObjInfo = connSecondo.getObjectsWithOutCount(this.mstrDBName, this.mstrObjName);
//        if (connSecondo != null) {
//          connSecondo.closeConnection();
//        }
//      }
//    }
//    catch (SecurityException e)
//    {
//      LogFileHandler.mlogger.severe(e.getMessage());
//    }
//    catch (IOException e)
//    {
//      LogFileHandler.mlogger.severe(e.getMessage());
//    }
//    SecondoObjectInfoClass secObj = new SecondoObjectInfoClass();
//    for (int i = 0; i < llsecObjInfo.size(); i++) {
//      if (((SecondoObjectInfoClass)llsecObjInfo.get(i)).getStrObjName().toString().intern() == this.mstrObjName.intern())
//      {
//        secObj = (SecondoObjectInfoClass)llsecObjInfo.get(i);
//        return secObj;
//      }
//    }
//    return secObj;
//  }
// 
//  public Runnable runCreateUnitPoints = new Runnable()
//  {
//    public void run()
//    {
//      MO2Postgres.this.createUnitPoints();
//    }
//  };
//  public Runnable runSplitUnitPoint = new Runnable()
//  {
//    public void run()
//    {
//      MO2Postgres.this.splitUnitPoints();
//    }
//  };
//  
//  private void createUnitPoints()
//  {
//    boolean bMOisSelected = false;
//    boolean bFilterCondition = false;
//    boolean bUsePostGISTypes = false;
//    boolean bAttributeEqualTimecolumn = false;
//    
//    StringBuffer sbTblName = new StringBuffer(this.mjtextNameTbl.getText().toLowerCase());
//    StringBuffer sbTimeColumn = new StringBuffer(this.mjtextColumnTimeName.getText());
//    strNewSecObjName = this.mjTextNewSecTableName.getText().toString();
//   
//    
//    
//    StringBuffer sbMovingObject2Unit = new StringBuffer();
//   
//    float fIntervalValue = 1.0F;
//    /* Etie
//    fIntervalValue = Float.valueOf(this.mjspinnerTimeInterval.getValue().toString()).floatValue();*/
//    
//    bUsePostGISTypes = this.mjcheckCopy2PostGISTypes.isSelected();
//    if ((sbTblName.length() == 0) || (sbTimeColumn.length() == 0))
//    {
//      new Message("Wrong value of one of the inputfields.");
//      return;
//    }
//    
//    for (int i = 0; i < this.mjcheckList.getModel().getSize(); i++)
//    {
//      JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.mjcheckList.getModel().getElementAt(i);
//      if (_jcheckboxObj.isSelected())
//      {
//        if (this.msecTypes.isMOType(this.msecObj.getColTypes(i).toString())) //|| (this.msecTypes.isMOType(this.)))
//        {
//          bMOisSelected = true;
//          alStringMOColumnames.add(_jcheckboxObj.getText());
//        }
//        else
//        {
//          alStringNoMOColumnames.add(_jcheckboxObj.getText());
//        }
//        alStringColumnames.add(_jcheckboxObj.getText());
//        if (_jcheckboxObj.getText().intern() == sbTimeColumn.toString().intern()) {
//          bAttributeEqualTimecolumn = true;
//        }
//      }
//    }
//    if (!bMOisSelected)
//    {
//      new Message("No Unit object selected.");
//      return;
//    }
//    if (bAttributeEqualTimecolumn)
//    {
//      new Message("Equal columnname.");
//      return;
//    }
//    
// 
//
//
//
//    LineNumberReader lnr = new LineNumberReader(new StringReader(this.mjtextareaFilter.getText()));
//    
//
// 
//   
//    
//    StringBuffer stringBufferCommand = new StringBuffer();
//    StringBuffer stringBufferCommandCount = new StringBuffer();
//       
//    stringBufferCommand.append("query ");
//    stringBufferCommand.append(this.mstrObjName);
//    stringBufferCommand.append(" feed ");
//    
//
//    stringBufferCommandCount.append(stringBufferCommand.toString());
//    if (bFilterCondition)
//    {
//    //  stringBufferCommand.append(stringBufferFilterField.toString());
//      stringBufferCommand.append(" extend[");
//      
//     // stringBufferCommandCount.append(stringBufferFilterField.toString());
//      stringBufferCommandCount.append(" count;");
//      
//
//      StringBuffer sbProjectAttr = new StringBuffer();
//      for (int i = 0; i < alStringMOColumnames.size(); i++)
//      {
//        sbProjectAttr.append("Tattr");
//        sbProjectAttr.append(i);
//        
//        stringBufferCommand.append("Tattr");
//        stringBufferCommand.append(i);
//        stringBufferCommand.append(": deftime(.");
//        stringBufferCommand.append((String)alStringMOColumnames.get(i));
//        stringBufferCommand.append(")");
//        if (i < alStringMOColumnames.size() - 1)
//        {
//          stringBufferCommand.append(",");
//          sbProjectAttr.append(",");
//        }
//      }
//      stringBufferCommand.append(" ] project[");
//      stringBufferCommand.append(sbProjectAttr.toString());
//      stringBufferCommand.append("]");
//    }
//    else
//    {
//    	 
//
//    	
//    	
//      stringBufferCommand.append("projectextend[;");
//      
//      stringBufferCommandCount.append(" count;");
//      /*
//       * Etie process the moving Object
//      
//      */
//     String valueStart = "\"start\"";
//      
//      for (int i = 0; i < alStringMOColumnames.size(); i++)
//      {
//        stringBufferCommand.append("Tattr");
//        stringBufferCommand.append(i);
//        stringBufferCommand.append(": inst(initial(.");
//        stringBufferCommand.append((String)alStringMOColumnames.get(i));
//        stringBufferCommand.append(")) ,");
//        
//      
//        
//        stringBufferCommand.append("Tattrib");
//        stringBufferCommand.append(i);
//        stringBufferCommand.append(": val(initial(.");
//        stringBufferCommand.append((String)alStringMOColumnames.get(i));
//        stringBufferCommand.append(")), ");
//        stringBufferCommand.append("Kind: " +valueStart);	 // \"stard\"");
//        
//      }
//      
//      
//      /* Etie added this for the mo */
//      String valueEnd = "\"end\"";
//      stringBufferCommand.append("] ");
//      
//   
//    ConnectSecondo connSecondo = null;
//    File mtempfile = null;
//    Writer out = null;
//    try
//    {
//	      try {
//			connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
//			    gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
//	      } catch (NumberFormatException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//	      } catch (SecurityException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//	      if (!connSecondo.connect()) {
//	        new Warning("Can not connect to SECONDO database.\nPlease check whether the connection to secondo database is possible.");
//	      }
//	      for (;;)
//	      {
//	        
//	        connSecondo.sendCommand(new StringBuffer("open database " + this.mstrDBName));
//	        if (connSecondo.getErrorCode().value != 0)
//	        {
//	          if (connSecondo != null) {
//	            connSecondo.closeConnection();
//	          }
//	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
//	        }
//	        else
//	        {
//	          connSecondo.setQueryResults2Null();
//	          connSecondo.sendCommand(stringBufferCommandCount);
//	          if (connSecondo.getErrorCode().value == 0) {
//	            break;
//	          }
//	          if (connSecondo != null) {
//	            connSecondo.closeConnection();
//	          }
//	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
//	        }
//	      }
//	
//	          //Sending generate Units from MovingObjects
//	          /* Unit Points created */
//	    	
//          connSecondo.sendCommand(createUnitFromMovingObjects());
//		  if (unitPointCreated == true)
//		  {
//			  new Message("Table with Unit Points created under Secondo in the \n database " +this.mstrObjName);
//			  unitPointCreated = false; 
//		  } 
//	    }finally{
//    		}
//    }
// }
//  
//  private void splitUnitPoints()
//  {
//    boolean bMOisSelected = false;
//    boolean bFilterCondition = false;
//    boolean bUsePostGISTypes = false;
//    boolean bAttributeEqualTimecolumn = false;
//    
//    StringBuffer sbTblName = new StringBuffer(this.mjtextNameTbl.getText().toLowerCase());
//    StringBuffer sbTimeColumn = new StringBuffer(this.mjtextColumnTimeName.getText());
//    strNewSecObjName = this.mjTextNewSecTableName.getText().toString();
//   
//    
//    
//    StringBuffer sbMovingObject2Unit = new StringBuffer();
//   
//    float fIntervalValue = 1.0F;
//    /* Etie
//    fIntervalValue = Float.valueOf(this.mjspinnerTimeInterval.getValue().toString()).floatValue();*/
//    
//    bUsePostGISTypes = this.mjcheckCopy2PostGISTypes.isSelected();
//    if ((sbTblName.length() == 0) || (sbTimeColumn.length() == 0))
//    {
//      new Message("Wrong value of one of the inputfields.");
//      return;
//    }
//    
//    for (int i = 0; i < this.mjcheckList.getModel().getSize(); i++)
//    {
//      JCheckboxWithObject _jcheckboxObj = (JCheckboxWithObject)this.mjcheckList.getModel().getElementAt(i);
//      if (_jcheckboxObj.isSelected())
//      {
//        if (this.msecTypes.isMOType(this.msecObj.getColTypes(i).toString())) //|| (this.msecTypes.isMOType(this.)))
//        {
//          bMOisSelected = true;
//          alStringMOColumnames.add(_jcheckboxObj.getText());
//        }
//        else
//        {
//          alStringNoMOColumnames.add(_jcheckboxObj.getText());
//        }
//        alStringColumnames.add(_jcheckboxObj.getText());
//        if (_jcheckboxObj.getText().intern() == sbTimeColumn.toString().intern()) {
//          bAttributeEqualTimecolumn = true;
//        }
//      }
//    }
//    if (!bMOisSelected)
//    {
//      new Message("No moving object selected.");
//      return;
//    }
//    if (bAttributeEqualTimecolumn)
//    {
//      new Message("Equal columnname.");
//      return;
//    }
//    
// 
//
//
//
//    LineNumberReader lnr = new LineNumberReader(new StringReader(this.mjtextareaFilter.getText()));
//    
//
// 
//   
//    
//    StringBuffer stringBufferCommand = new StringBuffer();
//    StringBuffer stringBufferCommandCount = new StringBuffer();
//       
//    stringBufferCommand.append("query ");
//    stringBufferCommand.append(this.mstrObjName);
//    stringBufferCommand.append(" feed ");
//    
//
//    stringBufferCommandCount.append(stringBufferCommand.toString());
//    if (bFilterCondition)
//    {
//    //  stringBufferCommand.append(stringBufferFilterField.toString());
//      stringBufferCommand.append(" extend[");
//      
//     // stringBufferCommandCount.append(stringBufferFilterField.toString());
//      stringBufferCommandCount.append(" count;");
//      
//
//      StringBuffer sbProjectAttr = new StringBuffer();
//      for (int i = 0; i < alStringMOColumnames.size(); i++)
//      {
//        sbProjectAttr.append("Tattr");
//        sbProjectAttr.append(i);
//        
//        stringBufferCommand.append("Tattr");
//        stringBufferCommand.append(i);
//        stringBufferCommand.append(": deftime(.");
//        stringBufferCommand.append((String)alStringMOColumnames.get(i));
//        stringBufferCommand.append(")");
//        if (i < alStringMOColumnames.size() - 1)
//        {
//          stringBufferCommand.append(",");
//          sbProjectAttr.append(",");
//        }
//      }
//      stringBufferCommand.append(" ] project[");
//      stringBufferCommand.append(sbProjectAttr.toString());
//      stringBufferCommand.append("]");
//    }
//    else
//    {
//    	 
//
//    	
//    	
//      stringBufferCommand.append("projectextend[;");
//      
//      stringBufferCommandCount.append(" count;");
//      /*
//       * Etie process the moving Object
//      
//      */
//     String valueStart = "\"start\"";
//      
//      for (int i = 0; i < alStringMOColumnames.size(); i++)
//      {
//        stringBufferCommand.append("Tattr");
//        stringBufferCommand.append(i);
//        stringBufferCommand.append(": inst(initial(.");
//        stringBufferCommand.append((String)alStringMOColumnames.get(i));
//        stringBufferCommand.append(")) ,");
//        
//      
//        
//        stringBufferCommand.append("Tattrib");
//        stringBufferCommand.append(i);
//        stringBufferCommand.append(": val(initial(.");
//        stringBufferCommand.append((String)alStringMOColumnames.get(i));
//        stringBufferCommand.append(")), ");
//        stringBufferCommand.append("Kind: " +valueStart);	 // \"stard\"");
//        
//      }
//      
//      
//      /* Etie added this for the mo */
//      String valueEnd = "\"end\"";
//      stringBufferCommand.append("] ");
//      
//   
//    ConnectSecondo connSecondo = null;
//    File mtempfile = null;
//    Writer out = null;
//    try
//    {
//	      try {
//			connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
//			    gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
//	      } catch (NumberFormatException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//	      } catch (SecurityException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//	      if (!connSecondo.connect()) {
//	        new Warning("Can not connect to SECONDO database.\nPlease check whether the connection to secondo database is possible.");
//	      }
//	      for (;;)
//	      {
//	        
//	        connSecondo.sendCommand(new StringBuffer("open database " + this.mstrDBName));
//	        if (connSecondo.getErrorCode().value != 0)
//	        {
//	          if (connSecondo != null) {
//	            connSecondo.closeConnection();
//	          }
//	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
//	        }
//	        else
//	        {
//	          connSecondo.setQueryResults2Null();
//	          connSecondo.sendCommand(stringBufferCommandCount);
//	          if (connSecondo.getErrorCode().value == 0) {
//	            break;
//	          }
//	          if (connSecondo != null) {
//	            connSecondo.closeConnection();
//	          }
//	          new Warning("Can not connect to SECONDO database.\nPlease check connection parameters.");
//	        }
//	      }
//	
//	          //Sending generate Units from MovingObjects
//	          /* Unit Points created */
//	    	
//          connSecondo.sendCommand(spliteUnitPoint());
//		  if (splitUnitPoints == true)
//		  {
//			  new Message("Table of splited Unit Points created under Secondo with "
//			  		+ "\n the relation name " +this.mstrObjName);
//			  splitUnitPoints = false; 
//		  } 
//	    }finally{
//    		}
//    }
// }
//
//  
//  ActionListener alButtons = new ActionListener()
//  {
//    public void actionPerformed(ActionEvent arg0)
//    {
//      if (arg0.getSource() == MO2Postgres.this.mjbutCreateUnitPoints)
//      {
//        Thread th = new Thread(MO2Postgres.this.runCreateUnitPoints);
//        th.start();
//      }
//      else if(arg0.getSource() == MO2Postgres.this.mjbutSplitUnitPoints)
//      {
//    	  Thread th = new Thread(MO2Postgres.this.runSplitUnitPoint);
//    	  th.start();
//      }
//      else if (arg0.getSource() == MO2Postgres.this.mjbutCancel)
//      {
//        MO2Postgres.this.mjframe.setVisible(false);
//      }
//    }
//  };
//  
//	private StringBuffer createUnitFromMovingObjects()
//	{
//		String newSecondoObject;
//		StringBuffer stringBufferCreateUnitsFromMovingObects01 = new StringBuffer();
//		
//		System.out.println(" Test of new Object " +strNewSecObjName);
//		if(strNewSecObjName.length()!= 0){
//			newSecondoObject = strNewSecObjName;
//		}
//		else
//		{
//			 new Message("Please provide the name of the new Secondo Object ");
//		      return new StringBuffer();
//		}
//		
//		/*Start Unitpoint */
//		
//		stringBufferCreateUnitsFromMovingObects01.append("let " +newSecondoObject +" = ");
//		 stringBufferCreateUnitsFromMovingObects01.append(this.mstrObjName);
//		 stringBufferCreateUnitsFromMovingObects01.append(" feed ");
//		// stringBufferCreateUnitsFromMovingObects.append("projectextend[");
//		stringBufferCreateUnitsFromMovingObects01.append("projectextendstream[ ");
//		
//		
//		
//
//	      for (int i = 0; i < alStringNoMOColumnames.size(); i++)
//	      {
//	    	  stringBufferCreateUnitsFromMovingObects01.append((String)alStringNoMOColumnames.get(i));
//	    	  if(i < alStringNoMOColumnames.size()-1)
//	    	  {
//	    		  stringBufferCreateUnitsFromMovingObects01.append(",");
//	    	  }
//	    	  else
//	    	  {
//	    		  stringBufferCreateUnitsFromMovingObects01.append("; ");
//	    	  }
//	        
//	      }
//	
//	    /*
//	     * Etie process the moving Object
//	    
//	    */
//	    
//	    for (int i = 0; i < alStringMOColumnames.size(); i++)
//	    {
//	    	
//	    	stringBufferCreateUnitsFromMovingObects01.append("UPoint");
//	    	stringBufferCreateUnitsFromMovingObects01.append(i);
//	    	stringBufferCreateUnitsFromMovingObects01.append(": units(.");
//	    	stringBufferCreateUnitsFromMovingObects01.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects01.append(") "); 
//	    }
//	    stringBufferCreateUnitsFromMovingObects01.append("]"); 
//	    stringBufferCreateUnitsFromMovingObects01.append(" consume;");
//	    stringBufferCreateUnitsFromMovingObects01.toString();
//		
//		System.out.println(" Unit Point created "+stringBufferCreateUnitsFromMovingObects01);
//		this.newSecondoObject = null;
//		this.newSecondoObject = newSecondoObject;
//		/*END Unit*/
//		
//		  unitPointCreated = true;  
//     return stringBufferCreateUnitsFromMovingObects01;
//	    
//	}
//	
//	private StringBuffer spliteUnitPoint()
//	{
//		//String newSecondoObject;
//		
//		StringBuffer stringBufferCreateUnitsFromMovingObects = new StringBuffer();
//		System.out.println(" Test of new Object " +strNewSecObjName);
//		if(strNewSecObjName.length()!= 0){
//			newSecondoObject = strNewSecObjName;
//		}
//		else
//		{
//			 new Message("Please provide the name of the new Secondo Object ");
//		      return new StringBuffer();
//		}
//		stringBufferCreateUnitsFromMovingObects.append("let " +strNewSecObjName +" = ");
//		
//		//this.mstrObjName = this.newSecondoObject;
//		
//	  //  stringBufferCreateUnitsFromMovingObects.append(" query ");
//		stringBufferCreateUnitsFromMovingObects.append(this.mstrObjName);
//		stringBufferCreateUnitsFromMovingObects.append(" feed ");
//		stringBufferCreateUnitsFromMovingObects.append("projectextend[");
//		//stringBufferCreateUnitsFromMovingObects.append("projectextendstream[ ");
//		
//		
//		
//
//	      for (int i = 0; i < alStringNoMOColumnames.size(); i++)
//	      {
//	    	  stringBufferCreateUnitsFromMovingObects.append((String)alStringNoMOColumnames.get(i));
//	    	  if(i < alStringNoMOColumnames.size()-1)
//	    	  {
//	    		  stringBufferCreateUnitsFromMovingObects.append(",");
//	    	  }
//	    	  else
//	    	  {
//	    		  stringBufferCreateUnitsFromMovingObects.append("; ");
//	    	  }
//	        
//	      } 
//	    /*
//	     * Etie process the moving Object
//	    
//	    */
//	      
//	    
//	    String valueStart = "\"start\"";
//	    
//	    for (int i = 0; i < alStringMOColumnames.size(); i++)
//	    {
////	    	stringBufferCreateUnitsFromMovingObects.append("UPoint");
////	    	stringBufferCreateUnitsFromMovingObects.append(i);
////	    	stringBufferCreateUnitsFromMovingObects.append(": units(.");
////	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
////	    	stringBufferCreateUnitsFromMovingObects.append("), "); 
//	    	
//	    	stringBufferCreateUnitsFromMovingObects.append("Time");
//	    	stringBufferCreateUnitsFromMovingObects.append(i);
//	    	stringBufferCreateUnitsFromMovingObects.append(": inst(initial(.");
//	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects.append(")), ");  
//	    	stringBufferCreateUnitsFromMovingObects.append("Pos");
//	    	stringBufferCreateUnitsFromMovingObects.append(i);
//	    	stringBufferCreateUnitsFromMovingObects.append(": val(initial(.");
//	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects.append(")), ");
//	    	stringBufferCreateUnitsFromMovingObects.append("Kind: " +valueStart);	 // \"stard\"");
//	      
//	      
//	      /* Etie uncomment this*/
//	      /*if (i < alStringMOColumnames.size() - 1) {
//	        stringBufferCommand.append(",");
//	      }
//	      */
//	      /*End Of Etien Uncomment this*/
//	    }
//	    
//	    /* Etie added this for the mo */
//	    String valueEnd = "\"end\"";
//	  //  boolean bf = "-";
//	    stringBufferCreateUnitsFromMovingObects.append("] ");
//	    
//	//    System.out.println(" Initial " +stringBufferCreateUnitsFromMovingObects);
//	    
//	    stringBufferCreateUnitsFromMovingObects.append(this.mstrObjName);
//	    stringBufferCreateUnitsFromMovingObects.append(" feed ");
//	   stringBufferCreateUnitsFromMovingObects.append(" projectextend[");
//	  //  stringBufferCreateUnitsFromMovingObects.append(" projectextendstream[");
//
//	      for (int i = 0; i < alStringNoMOColumnames.size(); i++)
//	      {
//	    	  stringBufferCreateUnitsFromMovingObects.append((String)alStringNoMOColumnames.get(i));
//	    	  if(i < alStringNoMOColumnames.size()-1)
//	    	  {
//	    		  stringBufferCreateUnitsFromMovingObects.append(",");
//	    	  }
//	    	  else
//	    	  {
//	    		  stringBufferCreateUnitsFromMovingObects.append("; ");
//	    	  }
//	        
//	      }
//	    
//	    /*
//	     * Etie process the moving Object
//	    
//	    */
//	    
//	    for (int i = 0; i < alStringMOColumnames.size(); i++)
//	    {
////	    	stringBufferCreateUnitsFromMovingObects.append("UPoint");
////	    	stringBufferCreateUnitsFromMovingObects.append(i);
////	    	stringBufferCreateUnitsFromMovingObects.append(": units(.");
////	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
////	    	stringBufferCreateUnitsFromMovingObects.append("), "); 
//	    	stringBufferCreateUnitsFromMovingObects.append("Time");
//	    	stringBufferCreateUnitsFromMovingObects.append(i);
//	    	stringBufferCreateUnitsFromMovingObects.append(": inst(final(.");
//	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects.append(")), ");
//	      
//	    	stringBufferCreateUnitsFromMovingObects.append("Pos");
//	    	stringBufferCreateUnitsFromMovingObects.append(i);
//	    	stringBufferCreateUnitsFromMovingObects.append(": val(final(.");
//	    	stringBufferCreateUnitsFromMovingObects.append((String)alStringMOColumnames.get(i));
//	    	stringBufferCreateUnitsFromMovingObects.append(")), ");
//	    	stringBufferCreateUnitsFromMovingObects.append("Kind: " +valueEnd);
//	        
//	    }
//	    
//	    
//	    stringBufferCreateUnitsFromMovingObects.append("]");
//	    stringBufferCreateUnitsFromMovingObects.append(" concat ");
//	    stringBufferCreateUnitsFromMovingObects.append("sortby ");
//	    stringBufferCreateUnitsFromMovingObects.append("[Time0 asc] ");
//	    stringBufferCreateUnitsFromMovingObects.append(" consume;");
//	    stringBufferCreateUnitsFromMovingObects.toString();
//	    
//	
//	  System.out.println(" Initial and Final " +stringBufferCreateUnitsFromMovingObects);
//	 
//	  
//     return stringBufferCreateUnitsFromMovingObects;
//	    
//	}
//}
