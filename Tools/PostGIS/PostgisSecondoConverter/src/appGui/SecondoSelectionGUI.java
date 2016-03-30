package appGui;



import convert.ConvertingSECFile;

import java.awt.Color;
import java.io.File;
//import java.sql.Connection;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
//import java.util.Set;
import java.util.logging.Logger;

import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;

import postgis.Add2Postgres;
import postgis.ConnectPostgres;
import postgis.DatabaseName;
import postgis.Tabelle;
import secondo.SendConvertStatementSEC;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.ConvertingFinishedGUI;
import appGuiUtil.Message;
import appGuiUtil.ProgressBarGUI;
import appGuiUtil.RefreshRightSideTree;
import appGuiUtil.Warning;

public class SecondoSelectionGUI extends SelectionGUI
{
  MainGui mgui;
  JCheckBox jcheckBox;

  public SecondoSelectionGUI(StringBuffer _sbDBName, StringBuffer _sbTBLName, MainGui _mgui)
  {
    super(_sbDBName, _sbTBLName, _mgui);

    this.jcheckBox = new JCheckBox("Convert to PostGIS types");
   // this.jcheckBox.setSelected(false);
    this.jcheckBox.setSelected(true);

    this.panelCommandButtons.add(this.jcheckBox);
    this.panelCommandButtons.add(new JLabel(""));
    this.panelCommandButtons.add(new JLabel(""));

    this.mgui = _mgui;
  }

  public void init(DefaultListModel defaultListModel)
  {
    super.init(defaultListModel);

    this.jSrcollTextArea.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "filter condition", 1, 2));
    this.jTextAreaSQL.setToolTipText("insert a SECONDO filter condition for your statement");
    this.jtextTableNewName.setToolTipText("name of the new PostgreSQL table");
    this.jButRun.setToolTipText("copy to PostgreSQL database");
  }

  @SuppressWarnings("unused")
public boolean startConverting(int iButton)
{
    try
    {
      LogFileHandler.mlogger.info("click to start copy to pg db");

      for (int i = 0; i < this.mdlmTODB.size(); i++)
      {
        if (this.mdlmTODB.getElementAt(i).toString().indexOf(" - ") != this.mdlmTODB.getElementAt(i).toString().lastIndexOf(" - "))
          continue;
        LogFileHandler.mlogger.info("The copy action cannot not be done. The attribute information could not be identified.");
        new Message("The copy action cannot not be done."
        		+ "\nNo attribute information can be identified.");
        return false;
      }

      boolean bUsePostGisTypes = false;

      if (this.jcheckBox.isSelected()) {
        bUsePostGisTypes = true;
      }
      readConditionField();

      ConnectPostgres connPG = new ConnectPostgres(gsbPG_Host, Integer.valueOf(gsbPG_Port.toString()).intValue(), gsbPG_User, gsbPG_Pwd);

      LinkedList llPGDBs = new LinkedList();
      LinkedList llPGTemplates = new LinkedList();
      HashMap hmTabellen = new HashMap();

      AskForPGDBAndTemplateName askDBNameTemplate = new AskForPGDBAndTemplateName();

      DatabaseName db = new DatabaseName();
      if (connPG.connect())
      {
        try
        {
          llPGDBs = connPG.getDatabaseNames();
          llPGTemplates = connPG.getTemplateNames();

          Iterator itDatenbanknamen = llPGDBs.iterator();

          while (itDatenbanknamen.hasNext())
          {
            db = (DatabaseName)itDatenbanknamen.next();

            if (connPG.conn != null)
            {
              connPG.conn.close();
            }

            askDBNameTemplate.getClass(); if (db.getSbName().toString().intern() == "template0".intern()) {
              continue;
            }
            if (!connPG.connect(db.getSbName()))
            {
              if (connPG.conn != null)
              {
                connPG.conn.close();
              }

              new Message("Can not connect to PostgreSQL/ PostGIS database."
              		+ "Please check whether the connection parameters of Postgis database are set corretly.");
              return false;
            }

            LinkedList llTabellennamen = new LinkedList();

            llTabellennamen = connPG.getTableNamesFromDB();

            hmTabellen.put(db.getSbName().toString(), new LinkedList(llTabellennamen));

            db = new DatabaseName();

            if (connPG.conn == null)
              continue;
            connPG.conn.close();
          }

          itDatenbanknamen = llPGTemplates.iterator();

          while (itDatenbanknamen.hasNext())
          {
            db = (DatabaseName)itDatenbanknamen.next();

            if (connPG.conn != null)
            {
              connPG.conn.close();
            }

            askDBNameTemplate.getClass(); if (db.getSbName().toString().intern() == "template0".intern()) {
              continue;
            }
            if (!connPG.connect(db.getSbName()))
            {
              if (connPG.conn != null)
              {
                connPG.conn.close();
              }

              new Message("Can not connect to PostgreSQL/ PostGIS database."
              		+ "Please check whether the connection parameters of Postgis database are set corretly.");
              break;
            }

            LinkedList llTabellennamen = new LinkedList();

            llTabellennamen = connPG.getTableNamesFromDB();

            hmTabellen.put(db.getSbName().toString(), new LinkedList(llTabellennamen));

            db = new DatabaseName();

            if (connPG.conn == null)
              continue;
            connPG.conn.close();
          }

        }
        catch (Exception e)
        {
          LogFileHandler.mlogger.severe(e.getMessage());
        }
        catch (Error err)
        {
          LogFileHandler.mlogger.severe(err.getMessage());
        }
        finally
        {
          try {
            if (connPG.conn != null)
              connPG.conn.close();
          }
          catch (SQLException e) {
            LogFileHandler.mlogger.severe(e.getMessage());
            return false;
          }
        }
        try
        {
          if (connPG.conn == null)  connPG.conn.close();
        }
        
        catch (SQLException e) {
          LogFileHandler.mlogger.severe(e.getMessage());
          return false;
        }

      }
      else
      {
        new Message("Can not connect to PostgreSQL/ PostGIS database."
        		+ "Please check whether the connection parameters of Postgis database are set corretly.");
        return false;
      }

   
    	  askDBNameTemplate.init(llPGDBs, llPGTemplates, this.msbDBName.toString());

      this.msbTBLName.delete(0, this.msbTBLName.length());

      this.msbTBLName.append(this.jtextTableNewName.getText().toLowerCase());

      AskForPGTblName askForPGTableName = new AskForPGTblName(this.msbTBLName.toString());

      askDBNameTemplate.getClass(); if (askDBNameTemplate.getButtonStatus() == 2)
      {
        LogFileHandler.mlogger.info("Cancel from user at template ask");
        return false;
      }
      askDBNameTemplate.getClass(); if (askDBNameTemplate.getButtonStatus() == 1)
      {
        this.msbTBLName.delete(0, this.msbTBLName.length());
        this.msbTBLName.append(this.jtextTableNewName.getText().toLowerCase());

        askDBNameTemplate.getClass(); if ("no template".intern() != askDBNameTemplate.getTemplateName().intern())
        {
          LinkedList _llTabelle = (LinkedList)hmTabellen.get(askDBNameTemplate.getTemplateName());

          for (int i = 0; i < _llTabelle.size(); i++)
          {
            if (((Tabelle)_llTabelle.get(i)).getSbName().toString().intern() != this.msbTBLName.toString().intern())
              continue;
            if (askForPGTableName.askForPostgresTblName())
            {
              this.msbTBLName.delete(0, this.msbTBLName.length());
              this.msbTBLName.append(askForPGTableName.getStrPostgresTblName());
              break;
            }
            return false;
          }

        }

      }
      else
      {
        askDBNameTemplate.getClass(); if (askDBNameTemplate.getButtonStatus() == 0)
        {
          this.msbTBLName.delete(0, this.msbTBLName.length());
          this.msbTBLName.append(this.jtextTableNewName.getText().toLowerCase());

          LinkedList _llTabelle = (LinkedList)hmTabellen.get(askDBNameTemplate.getDBName());

          for (int i = 0; i < _llTabelle.size(); i++)
          {
            if (((Tabelle)_llTabelle.get(i)).getSbName().toString().intern() != this.msbTBLName.toString().intern())
              continue;
            if (askForPGTableName.askForPostgresTblName())
            {
              this.msbTBLName.delete(0, this.msbTBLName.length());
              this.msbTBLName.append(askForPGTableName.getStrPostgresTblName());
              break;
            }
            return false;
          }

        }

      }

      if (!askForPGTableName.checkTblName(this.msbTBLName.toString())) {
        return false;
      }

      this.mprogbg = new ProgressBarGUI("SECONDO to PostgreSQL Converting", "Converting to " + askDBNameTemplate.getDBName());

      this.mprogbg.showProgbar();

      HashSet hsTBL = new HashSet();
      HashMap hmSecCommands = new HashMap();
      HashMap hmTblHeader = new HashMap();
      HashMap hmSecCommadCount = new HashMap();

      StringBuffer sbTmp = new StringBuffer();

      if (this.mprogbg.isCanceld())
      {
        this.mprogbg.closeProgbar();
        return false;
      }

      int iFirstDel = 0;
      int iSecondDel = 0;
      for (int i = 0; i < this.mdlmTODB.size(); i++)
      {
        if (this.mprogbg.isCanceld())
        {
          LogFileHandler.mlogger.info("user cancel 1");
          return false;
        }

        sbTmp.delete(0, sbTmp.length());
        String sbTmpTbl = "";

        iFirstDel = 0;
        iSecondDel = 0;
        iFirstDel = this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString()) + this.msbTblColDelimitter.length();
        iSecondDel = this.mdlmTODB.get(i).toString().indexOf(this.msbTblColDelimitter.toString(), iFirstDel);

        sbTmpTbl = this.mdlmTODB.get(i).toString().substring(iFirstDel, iSecondDel);

        sbTmp.append(this.mdlmTODB.get(i).toString().substring(iSecondDel + this.msbTblColDelimitter.length(), 
          this.mdlmTODB.get(i).toString().length()));
        hsTBL.add(sbTmpTbl);

        if (hmTblHeader.containsKey(sbTmpTbl))
        {
          StringBuffer sbTmpHeader = new StringBuffer((CharSequence)hmTblHeader.get(sbTmpTbl));
          sbTmpHeader.append(";");
          sbTmpHeader.append(sbTmp);

          hmTblHeader.put(sbTmpTbl, new StringBuffer(sbTmpHeader));
        }
        else
        {
          hmTblHeader.put(sbTmpTbl, new StringBuffer(sbTmp));
          hmSecCommadCount.put(sbTmpTbl, new StringBuffer(sbTmpTbl));
        }

      }

      if (this.mprogbg.isCanceld())
      {
        LogFileHandler.mlogger.info("user cancel 2");
        return false;
      }

      Iterator itTBL = hsTBL.iterator();
      String strTBL = "";

      StringBuffer sbSQL = new StringBuffer();
      StringBuffer sbColHeader = new StringBuffer();

      while (itTBL.hasNext())
      {
        sbSQL.delete(0, sbSQL.length());
        sbColHeader.delete(0, sbColHeader.length());

        sbSQL.append("query ");

        strTBL = (String)itTBL.next();
        sbSQL.append(strTBL);

        sbSQL.append(" feedproject [");
        StringBuffer sbHeader = (StringBuffer)hmTblHeader.get(strTBL);
        String[] strSplitHeader = sbHeader.toString().split(";");
        for (int u = 0; u < strSplitHeader.length; u++)
        {
          sbSQL.append(strSplitHeader[u].subSequence(0, strSplitHeader[u].indexOf(":")));

          if (u < strSplitHeader.length - 1) {
            sbSQL.append(",");
          }
        }
        sbSQL.append("] ");

        sbSQL.append(this.msbConditionFieldText.toString());
        sbSQL.append(" ");

        hmSecCommands.put(strTBL, new StringBuffer(sbSQL));

        if (!this.mprogbg.isCanceld())
          continue;
        LogFileHandler.mlogger.info("user cancel 3");
        return false;
      }

      ArrayList alTempFiles = new ArrayList(hmSecCommands.size());
      ThreadGroup thGroupConvert = new ThreadGroup("Sec2PGConvert");
      ArrayList alTmpSend = new ArrayList();

      for (Object key : hmSecCommands.keySet())
      {
        SendConvertStatementSEC sendSEC = new SendConvertStatementSEC();

        StringBuffer sbFileHeader = new StringBuffer();
        sbFileHeader.append(this.msbTBLName);
        sbFileHeader.append("\r\n");
        sbFileHeader.append((StringBuffer)hmTblHeader.get(key));
        sendSEC.setParameters(this.msbDBName, (StringBuffer)hmSecCommands.get(key), sbFileHeader, (StringBuffer)hmSecCommadCount.get(key));

        Thread th = new Thread(thGroupConvert, sendSEC);
        th.start();

        alTempFiles.add(sendSEC.getMtempfile());
        alTmpSend.add(sendSEC);
      }

      while (thGroupConvert.activeCount() > 0) {
        try
        {
          Thread.sleep(1000L);

          if (!this.mprogbg.isCanceld())
            continue;
          thGroupConvert.stop();
          LogFileHandler.mlogger.info("user cancel 4");
          return false;
        }
        catch (InterruptedException e)
        {
          LogFileHandler.mlogger.severe(e.getMessage());
        }

      }

      if (alTmpSend.size() == 1)
      {
        if (((SendConvertStatementSEC)alTmpSend.get(0)).wasError())
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
        LogFileHandler.mlogger.info("user cancel 5");
        return false;
      }

      LogFileHandler.mlogger.info("starts convert csv to psql");

      ConvertingSECFile conSecFile = new ConvertingSECFile();
      conSecFile.setUsePostGis(bUsePostGisTypes);
      File fpPGSql = conSecFile.convertFile2Restore(alTempFiles);

      if (this.mprogbg.isCanceld())
      {
        LogFileHandler.mlogger.info("user cancel 6");
        return false;
      }

      LogFileHandler.mlogger.info("starts to add data to pg database");
      Add2Postgres add2PG = new Add2Postgres();
      askDBNameTemplate.getClass(); 
      if (askDBNameTemplate.getButtonStatus() == 1)
      {
        if (!add2PG.adding(askDBNameTemplate.getDBName(), askDBNameTemplate.getTemplateName(), fpPGSql, this.msbTBLName.toString()))
        {
          new Warning("Error in converting process.");
          this.mprogbg.closeProgbar();
          return false;
        }

      }
      else if (!add2PG.addingDB(askDBNameTemplate.getDBName(), this.msbTBLName.toString(), fpPGSql))
      {
        new Warning("Error in converting process.");
        this.mprogbg.closeProgbar();
        return false;
      }

      if (this.mprogbg.isCanceld())
      {
        LogFileHandler.mlogger.info("user cancel 7");
        return false;
      }

      RefreshRightSideTree refRightSide = new RefreshRightSideTree(this.mgui);
      refRightSide.start();
      try {
        refRightSide.join();
      }
      catch (InterruptedException e) {
        LogFileHandler.mlogger.severe(e.getMessage());
      }

      if (this.mprogbg.isCanceld())
      {
        LogFileHandler.mlogger.info("user cancel 8");
        return false;
      }

      this.mprogbg.closeProgbar();
      new ConvertingFinishedGUI();

      LogFileHandler.mlogger.info("Copy finished to pg");
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

  public DefaultListModel convertDefaultTreeNodeModelToDefaultListModel(DefaultMutableTreeNode _dmtn, String strDBName, String _strSelTBL)
  {
    LogFileHandler.mlogger.info("start this methode");

    int iSelctionIndex = 0;
    for (int i = 0; i < _dmtn.getChildCount(); i++)
    {
      if (!strDBName.equals(_dmtn.getChildAt(i).toString()))
        continue;
      iSelctionIndex = i;
      break;
    }

    DefaultListModel dlm = new DefaultListModel();

    StringBuffer sbAdd2Dlm = new StringBuffer();
    StringBuffer sbTBL = new StringBuffer();
    StringBuffer sbCOL = new StringBuffer();

    if (_dmtn.getChildCount() > 0)
    {
      for (int k = 0; k < _dmtn.getChildAt(iSelctionIndex).getChildCount(); k++)
      {
        sbAdd2Dlm.delete(0, sbAdd2Dlm.length());
        sbTBL.delete(0, sbTBL.length());
        sbTBL.append(_dmtn.getChildAt(iSelctionIndex).getChildAt(k));
        sbTBL.append(this.msbTblColDelimitter);

        if (_dmtn.getChildAt(iSelctionIndex).getChildAt(k).toString().intern() == "TYPES".intern())
        {
          continue;
        }
        for (int g = 0; g < _dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildCount(); g++)
        {
          if ((_dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g).toString() == null) || 
            (_dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g).toString().intern() != _strSelTBL.intern())) {
            continue;
          }
          sbAdd2Dlm.delete(0, sbAdd2Dlm.length());
          sbCOL.delete(0, sbCOL.length());
          sbCOL.append(_dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g));

          if (_dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g).getChildCount() > 0)
          {
            for (int h = 0; h < _dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g).getChildCount(); h++)
            {
              sbAdd2Dlm.delete(0, sbAdd2Dlm.length());
              sbAdd2Dlm.append(sbTBL);
              sbAdd2Dlm.append(sbCOL);
              sbAdd2Dlm.append(this.msbTblColDelimitter);
              sbAdd2Dlm.append(_dmtn.getChildAt(iSelctionIndex).getChildAt(k).getChildAt(g).getChildAt(h));

              dlm.addElement(new StringBuffer(sbAdd2Dlm));
            }
          }
          else
          {
            sbAdd2Dlm.append(sbTBL);
            sbAdd2Dlm.append(sbCOL);

            dlm.addElement(new StringBuffer(sbAdd2Dlm));
          }

        }

      }

    }

    return dlm;
  }
}


