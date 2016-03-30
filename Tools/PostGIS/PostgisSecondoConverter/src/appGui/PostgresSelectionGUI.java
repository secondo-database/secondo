package appGui;


import convert.ConvertingPGFile;
import convert.PostgresTypes;

import java.awt.Color;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.logging.Logger;
import java.util.regex.Pattern;

import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JButton;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import postgis.SendConvertStatementsPG;
import secondo.Add2Secondo;
import secondo.ConnectSecondo;
import secondo.SecondoObjectInfoClass;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.ConvertingFinishedGUI;
import appGuiUtil.Message;
import appGuiUtil.ProgressBarGUI;
import appGuiUtil.RefreshLeftSideTree;
import appGuiUtil.Warning;

public class PostgresSelectionGUI
  extends SelectionGUI
{
  private PostgresTypes pgType;
  private Pattern pSecDBName;
  private Pattern pSQLLIMIT;
  private MainGui mgui;
  
  public PostgresSelectionGUI(StringBuffer _sbDBName, StringBuffer _sbTBLName, MainGui _mgui)
  {
    super(_sbDBName, _sbTBLName, _mgui);
    

    this.mgui = _mgui;
    this.pgType = new PostgresTypes();
    this.pSQLLIMIT = Pattern.compile(".+([Ll]{1}[Ii]{1}[Mm]{1}[Ii]{1}[Tt]{1}[\\s]{1}[0-9]+).*");
  }
  
  public void init(DefaultListModel dlistModel)
  {
    super.init(dlistModel);
    
    this.jSrcollTextArea.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "where condition", 1, 2));
    this.jTextAreaSQL.setToolTipText("insert a PostgreSQL where-condition for your statement: start with where");
    this.jtextTableNewName.setToolTipText("name for the new SECONDO object");
    this.jButRun.setToolTipText("copy to SECONDO database");
  }
  
  @SuppressWarnings({ "rawtypes", "unchecked" })
public boolean startConverting(int iButton)
  {
    try
    {
      LogFileHandler.mlogger.info("click to start copy to secondo db");
      
      readConditionField();
      
      boolean bReturn = true;
      ConnectSecondo connSecondo = null;
      LinkedList<String> llDBNamen = new LinkedList();
      HashMap<String, LinkedList<SecondoObjectInfoClass>> hmObjects = new HashMap();
      try
      {
        connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
          gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
        if (!connSecondo.connect())
        {
          this.mprogbg.closeProgbar();
          new Warning("Can not connect to SECONDO database.\nPlease checkconnection parameters.");
          bReturn = false;
        }
        else
        {
          llDBNamen = connSecondo.getDatabaseNames();
          for (int i = 0; i < llDBNamen.size(); i++)
          {
            LinkedList<SecondoObjectInfoClass> lltmp = connSecondo.getObjects((String)llDBNamen.get(i));
            
            hmObjects.put((String)llDBNamen.get(i), new LinkedList(lltmp));
          }
          if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
            connSecondo.closeConnection();
          }
        }
      }
      catch (SecurityException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
        bReturn = false;
        new Message(e.getMessage());
      }
      catch (IOException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
        bReturn = false;
        new Message(e.getMessage());
      }
      finally
      {
        if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
          connSecondo.closeConnection();
        }
        if (!bReturn) {
          return false;
        }
      }
      if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
        connSecondo.closeConnection();
      }
      if (!bReturn) {
        return false;
      }
      this.msbTBLName.delete(0, this.msbTBLName.length());
      
      this.msbTBLName.append(this.jtextTableNewName.getText());
      
      LogFileHandler.mlogger.info("ask db name");
      
      AskForSecDBExistingName askSecNewName = new AskForSecDBExistingName();
      
      askSecNewName.init(llDBNamen, this.msbDBName.toString());
      
      askSecNewName.getClass();
      if (askSecNewName.getButtonStatus() == 2) {
        return false;
      }
      askSecNewName.getClass();
      if (askSecNewName.getButtonStatus() == 0)
      {
        this.msbTBLName.delete(0, this.msbTBLName.length());
        this.msbTBLName.append(this.jtextTableNewName.getText());
        
        LinkedList<SecondoObjectInfoClass> _llObj = new LinkedList();
        _llObj = (LinkedList)hmObjects.get(askSecNewName.getDBName().toString());
        for (int i = 0; i < _llObj.size(); i++) {
          if (((SecondoObjectInfoClass)_llObj.get(i)).getStrObjName().toString().intern() == this.msbTBLName.toString().intern())
          {
            AskForSecTblName askSecTable = new AskForSecTblName(this.msbTBLName.toString());
            if (askSecTable.askForSecondoTblName())
            {
              this.msbTBLName.delete(0, this.msbTBLName.length());
              this.msbTBLName.append(askSecTable.getStrSecondoTblName());
              break;
            }
            return false;
          }
        }
      }
      if (!new AskForSecTblName().checkTblName(this.msbTBLName.toString())) {
        return false;
      }
      if (!askSecNewName.checkDBName(askSecNewName.getDBName())) {
        return false;
      }
      this.mprogbg = new ProgressBarGUI("PostgreSQL to SECONDO Converting", "Converting to " + askSecNewName.getDBName());
      
      this.mprogbg.showProgbar();
      
      HashSet<String> hsTBL = new HashSet();
      @SuppressWarnings("rawtypes")
	HashMap hmCol = new HashMap();
      HashMap<String, StringBuffer> hmColOrderType = new HashMap();
      for (int i = 0; i < this.mdlmTODB.size(); i++) {
        hsTBL.add(this.mdlmTODB.get(i).toString().substring(0, 
          this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString())));
      }
      if (this.mprogbg.isCanceld())
      {
        LogFileHandler.mlogger.info("user cancel 1");
        this.mprogbg.closeProgbar();
        return false;
      }
      Iterator<String> itTBL = hsTBL.iterator();
      String strTBL = "";
      String strFirstCol = "";
      
      StringBuffer sbSQL = new StringBuffer();
      StringBuffer sbColHeader = new StringBuffer();
      StringBuffer sbFirstType = new StringBuffer();
      while (itTBL.hasNext())
      {
        sbSQL.delete(0, sbSQL.length());
        sbColHeader.delete(0, sbColHeader.length());
        
        sbSQL.append("select ");
        
        strTBL = (String)itTBL.next();
        strFirstCol = "";
        
        sbColHeader.append(strTBL);
        sbColHeader.append("\r\n");
        
        StringBuffer sbType = new StringBuffer();
        boolean bRead = true;
        for (int i = 0; i < this.mdlmTODB.size(); i++)
        {
          if (this.mprogbg.isCanceld())
          {
            this.mprogbg.closeProgbar();
            LogFileHandler.mlogger.info("user cancel 2");
            return false;
          }
          if (this.mdlmTODB.get(i).toString().startsWith(strTBL))
          {
            sbType.delete(0, sbType.length());
            

            sbType.append(this.mdlmTODB.get(i).toString().substring(this.mdlmTODB.get(i).toString().lastIndexOf(":") + 1, 
              this.mdlmTODB.get(i).toString().length()));
            if (bRead)
            {
              strFirstCol = this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"));
              
              bRead = false;
            }
            if (PostgresTypes.sbPGISPoint.toString().equals(sbType.toString()))
            {
              
              sbSQL.append("ST_Astext(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append("), ");
            }
            else if (PostgresTypes.sbPGISLineString.toString().equals(sbType.toString()))
            {
               
            	sbSQL.append("ST_Astext(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append("), ");
            }
            else if (PostgresTypes.sbPGISPolygon.toString().equals(sbType.toString()))
            {
             
              sbSQL.append("ST_Astext(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append("), ");
            }
            else if (PostgresTypes.sbPGISMulitpoint.toString().equals(sbType.toString()))
            {
              
            	sbSQL.append("ST_Astext(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append("), ");
            }
            else if (PostgresTypes.sbPGISMultiLineString.toString().equals(sbType.toString()))
            {
             
            	sbSQL.append("ST_Astext(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append("), ");
            }
            
            else if (PostgresTypes.sbPGDATE.toString().equals(sbType.toString()))
            {
              sbSQL.append("to_char(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append(",'YYYY-MM-DD'), ");
            }
            else if (PostgresTypes.sbPGTIMESTAMP.toString().equals(sbType.toString()))
            {
              sbSQL.append("to_char(");
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append(",'YYYY-MM-DD-HH24:MI:SS.MS'), ");
            }
            else if (PostgresTypes.sbPGINTERVAL.toString().equals(sbType.toString()))
            {
              sbSQL.append("EXTRACT(EPOCH from ");
              String strName = postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":")));
              sbSQL.append(strName);
              
              sbSQL.append(") as ");
              sbSQL.append(strName);
              sbSQL.append(" ,");
            }
            else
            {
              sbSQL.append(postgisWellForm_Geometry(this.mdlmTODB.get(i).toString().substring(
                this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
                this.mdlmTODB.get(i).toString().lastIndexOf(":"))));
              sbSQL.append(", ");
            }
            sbColHeader.append(this.mdlmTODB.get(i).toString().substring(
              this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length(), 
              this.mdlmTODB.get(i).toString().lastIndexOf(":")));
            sbColHeader.append(":");
            sbColHeader.append(sbType);
            sbColHeader.append(";");
          }
        }
        sbSQL.deleteCharAt(sbSQL.lastIndexOf(","));
        
        sbSQL.append("from ");
        sbSQL.append(strTBL);
        sbSQL.append(";");
        sbSQL.append(strFirstCol);
        
        sbColHeader.deleteCharAt(sbColHeader.lastIndexOf(";"));
        
        ((HashMap)hmCol).put(new StringBuffer(sbSQL), new StringBuffer(sbColHeader));
        hmColOrderType.put(sbSQL.toString(), new StringBuffer(sbFirstType));
      }
      if (this.mprogbg.isCanceld())
      {
        this.mprogbg.closeProgbar();
        LogFileHandler.mlogger.info("user cancel 3");
        return false;
      }
      LogFileHandler.mlogger.info("all statements selected");
      System.out.println(" 1 OK Etie");

      ArrayList<File> alTempFiles = new ArrayList<File>(hmCol.size());
      ThreadGroup thGroupConvert = new ThreadGroup("PG2SecConvert");
      
      System.out.println(" 2 OK Etie");
      ArrayList<SendConvertStatementsPG> alTmpSend = new ArrayList<>();  
      System.out.println(" 3 OK Etie");
      for (StringBuffer key :  ((HashMap<StringBuffer, StringBuffer>)hmCol).keySet()) //keySet()) (  (HashMap<String,StringBuffer>) hmCol)
    	
      {
    	  System.out.println(" 4 OK Etie");
        String strColSort = key.substring(key.lastIndexOf(";") + 1, key.length()).toString();
        String strSQL = key.substring(0, key.lastIndexOf(";")).toString();
        
        SendConvertStatementsPG sendPG = new SendConvertStatementsPG();
        System.out.println(" 5 OK Etie");
        sendPG.setParameters(new StringBuffer(this.msbDBName), new StringBuffer(strTBL), this.msbConditionFieldText, new StringBuffer(strSQL), 
          new StringBuffer((CharSequence)((HashMap)hmCol).get(key)), new StringBuffer(strColSort));
        
        Thread th = new Thread(thGroupConvert, sendPG);
        th.start();
        
        alTempFiles.add(sendPG.getMtempfile());
        alTmpSend.add(sendPG);
      }
      while (thGroupConvert.activeCount() > 0)
      {
        Thread.sleep(1000L);
        if (this.mprogbg.isCanceld())
        {
          LogFileHandler.mlogger.info("user cancel 4");
          this.mprogbg.closeProgbar();
          

          thGroupConvert.stop();
          return false;
        }
      }
      if (alTmpSend.size() == 1)
      {
        if (((SendConvertStatementsPG)alTmpSend.get(0)).wasError())
        {
          LogFileHandler.mlogger.severe("Error in converting process.");
          this.mprogbg.closeProgbar();
          new Message("Error in converting process.");
          return false;
        }
      }
      else
      {
        LogFileHandler.mlogger.severe("Error in converting process.");
        this.mprogbg.closeProgbar();
        new Message("Error in converting process.");
        return false;
      }
      if (this.mprogbg.isCanceld())
      {
        this.mprogbg.closeProgbar();
        LogFileHandler.mlogger.info("user cancel 5");
        return false;
      }
      ConvertingPGFile convertPGFile = new ConvertingPGFile();
      
      Add2Secondo add2Sec = new Add2Secondo();
      
      LogFileHandler.mlogger.info("start converting csv to sec file");
      
      askSecNewName.getClass();
      if (askSecNewName.getButtonStatus() == 1)
      {
        File fpRestoreFile = convertPGFile.convertFile2Restore(alTempFiles);
        if (this.mprogbg.isCanceld())
        {
          this.mprogbg.closeProgbar();
          LogFileHandler.mlogger.info("user cancel 5.0");
          return false;
        }
        LogFileHandler.mlogger.info("try to add to sec db");
        if (!add2Sec.addin(askSecNewName.getDBName(), fpRestoreFile))
        {
          this.mprogbg.closeProgbar();
          return false;
        }
      }
      else
      {
        askSecNewName.getClass();
        if (askSecNewName.getButtonStatus() == 0)
        {
          File fpRestoreFile = convertPGFile.convertFile2RestoreObject((File)alTempFiles.get(0));
          if (this.mprogbg.isCanceld())
          {
            this.mprogbg.closeProgbar();
            LogFileHandler.mlogger.info("user cancel 5.1");
            return false;
          }
          LogFileHandler.mlogger.info("try to add to sec db");
          if (!add2Sec.addin(askSecNewName.getDBName(), this.msbTBLName.toString(), fpRestoreFile))
          {
            this.mprogbg.closeProgbar();
            return false;
          }
        }
      }
      if (this.mprogbg.isCanceld())
      {
        this.mprogbg.closeProgbar();
        LogFileHandler.mlogger.info("user cancel 6");
        return false;
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
      }
      if (this.mprogbg.isCanceld())
      {
        this.mprogbg.closeProgbar();
        LogFileHandler.mlogger.info("user cancel 7");
        return false;
      }
      this.mprogbg.closeProgbar();
      
      new ConvertingFinishedGUI();
      LogFileHandler.mlogger.info("Copy finished to pg");
    }
    catch (InterruptedException irexp)
    {
      LogFileHandler.mlogger.severe(irexp.getMessage());
      this.mprogbg.closeProgbar();
      return false;
    }
    catch (Exception e)
    {
      this.mprogbg.closeProgbar();
      LogFileHandler.mlogger.severe(e.getMessage());
      return false;
    }
    catch (Error err)
    {
      this.mprogbg.closeProgbar();
      LogFileHandler.mlogger.severe(err.getMessage());
      return false;
    }
    return true;
  }
  
  private String postgisWellForm_Geometry(String strSpaltenName)
  {
    return "\"" + strSpaltenName + "\"";
  }
}
