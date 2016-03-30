package secondo;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.logging.Logger;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import secondoPostgisUtil.LogFileHandler;

public class ConnectSecondo
{
  private StringBuffer msbhostName;
  private int miport;
  private StringBuffer msbUser;
  private StringBuffer msbPwd;
  private boolean mbUseBinaryList;
  private ESInterface si = null;
  private IntByReference errorCode = new IntByReference();
  private IntByReference errorPos = new IntByReference();
  private ListExpr resultList = new ListExpr();
  private StringBuffer errorMessage = new StringBuffer();
  
  public ConnectSecondo(StringBuffer _msbhostName, int _miport, StringBuffer _msbUser, StringBuffer _msbPwd, boolean _mbUseBinaryList)
    throws SecurityException, IOException
  {
    this.msbhostName = new StringBuffer();
    this.miport = 0;
    this.msbUser = new StringBuffer();
    this.msbPwd = new StringBuffer();
    this.mbUseBinaryList = true;
    
    this.msbhostName = _msbhostName;
    this.miport = _miport;
    this.msbUser = _msbUser;
    this.msbPwd = _msbPwd;
    this.mbUseBinaryList = _mbUseBinaryList;
    
    this.si = new ESInterface();
  }
  
  public boolean connect()
  {
    this.si.setHostname(this.msbhostName.toString());
    this.si.setPort(this.miport);
    
    this.si.setUserName(this.msbUser.toString());
    this.si.setPassWd(this.msbPwd.toString());
    
    this.si.useBinaryLists(this.mbUseBinaryList);
    
    tools.Environment.MEASURE_TIME = false;
    if (!this.si.connect())
    {
      LogFileHandler.mlogger.severe("problem in connecting with a secondo server");
      return false;
    }
    return true;
  }
  
  public void closeConnection()
  {
    if ((this.si != null) && (this.si.isConnected()))
    {
      LogFileHandler.mlogger.info("close connection");
      this.si.terminate();
    }
  }
  
  public boolean isSecondoConnected()
  {
    if ((this.si != null) && (this.si.isConnected())) {
      return true;
    }
    return false;
  }
  /*
   * This function resets all error code, error pos, result list and error message to initial values
   */
  
  public void setQueryResults2Null()
  {
    this.errorCode.value = 0;
    this.errorPos.value = 0;
    this.resultList = new ListExpr();
    this.errorMessage.delete(0, this.errorMessage.length());
  }
  
  public LinkedList<String> getDatabaseNames()
  {
    LinkedList<String> llDatabasenames = new LinkedList();
    if (!isSecondoConnected())
    {
      LogFileHandler.mlogger.warning("Can not read databases because no connection");
      return llDatabasenames;
    }
    setQueryResults2Null();
    this.si.secondo("list databases;", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llDatabasenames;
    }
    if ((this.resultList.first().atomType() != 5) || 
      (!this.resultList.first().symbolValue().equals("inquiry")) || 
      (this.resultList.listLength() != 2) || 
      (!this.resultList.second().first().symbolValue().equals("databases"))) {
      return llDatabasenames;
    }
    ListExpr listExpDatabase = this.resultList.second().second();
    while (!listExpDatabase.isEmpty())
    {
      llDatabasenames.add(listExpDatabase.first().symbolValue());
      
      listExpDatabase = listExpDatabase.rest();
    }
    return llDatabasenames;
  }
  
  /*
   * 
   */
  public LinkedList<String> getAlgebras()
  {
    LinkedList<String> llAlgebras = new LinkedList();
    if (!isSecondoConnected())
    {
      LogFileHandler.mlogger.warning("Can not read algebras because no connection");
      return llAlgebras;
    }
    setQueryResults2Null();
    
    this.si.secondo("list algebras;", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llAlgebras;
    }
    if ((this.resultList.first().atomType() != 5) || 
      (!this.resultList.first().symbolValue().equals("inquiry")) || 
      (this.resultList.listLength() != 2) || 
      (!this.resultList.second().first().symbolValue().equals("algebras"))) {
      return llAlgebras;
    }
    ListExpr listExpDatabase = this.resultList.second().second();
    while (!listExpDatabase.isEmpty())
    {
      llAlgebras.add(listExpDatabase.first().symbolValue());
      listExpDatabase = listExpDatabase.rest();
    }
    return llAlgebras;
  }
  
  /*
   * 
   * 
   */
  public LinkedList<String> getTypes(String strDatabasename)
  {
    LinkedList<String> llTypes = new LinkedList();
    if (!isSecondoConnected())
    {
      LogFileHandler.mlogger.warning("Can not read types because no connection");
      return llTypes;
    }
    setQueryResults2Null();
    this.si.secondo("open database " + strDatabasename + ";", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llTypes;
    }
    this.si.secondo("list types;", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llTypes;
    }
    this.si.secondo("close database;", 
      new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llTypes;
    }
    if ((this.resultList.first().atomType() != 5) || 
      (!this.resultList.first().symbolValue().equals("inquiry")) || 
      (this.resultList.listLength() != 2) || 
      (!this.resultList.second().first().symbolValue().equals("types"))) {
      return llTypes;
    }
    if (this.resultList.second().second().listLength() <= 1) {
      return llTypes;
    }
    ListExpr liExpType = this.resultList.second().second().second();
    if (!liExpType.first().symbolValue().equals("TYPE")) {
      return llTypes;
    }
    this.resultList = this.resultList.second();
    this.resultList = this.resultList.second();
    while (!this.resultList.isEmpty()) {
      if (this.resultList.first().listLength() == -1)
      {
        this.resultList = this.resultList.rest();
      }
      else
      {
        llTypes.add(this.resultList.first().second().symbolValue());
        this.resultList = this.resultList.rest();
      }
    }
    return llTypes;
  }
  /*
   * 
 	*/
  public LinkedList<SecondoObjectInfoClass> getObjects(String strDatabasename)
  {
    LinkedList<SecondoObjectInfoClass> llObjects = new LinkedList();
    if (!isSecondoConnected())
    {
      LogFileHandler.mlogger.warning("Can not read Objects because no connection");
      return llObjects;
    }
    setQueryResults2Null();
    this.si.secondo("open database " + strDatabasename + ";", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llObjects;
    }
    this.si.secondo("list objects;", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llObjects;
    }
    if ((this.resultList.first().atomType() != 5) || 
      (!this.resultList.first().symbolValue().equals("inquiry")) || 
      (this.resultList.listLength() != 2) || 
      (!this.resultList.second().first().symbolValue().equals("objects")))
    {
      this.si.secondo("close database;", 
        new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
      if (this.errorCode.value != 0)
      {
        LogFileHandler.mlogger.severe(this.errorMessage.toString());
        return llObjects;
      }
      return llObjects;
    }
    if (this.resultList.second().second().listLength() <= 1)
    {
      this.si.secondo("close database;", 
        new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
      if (this.errorCode.value != 0)
      {
        LogFileHandler.mlogger.severe(this.errorMessage.toString());
        return llObjects;
      }
      return llObjects;
    }
    ListExpr liExpType = this.resultList.second().second().second();
    if (!liExpType.first().symbolValue().equals("OBJECT"))
    {
      this.si.secondo("close database;", 
        new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
      if (this.errorCode.value != 0)
      {
        LogFileHandler.mlogger.severe(this.errorMessage.toString());
        return llObjects;
      }
      return llObjects;
    }
    ListExpr VL = this.resultList.second();
    
    ListExpr Value = VL.second();
    LinkedList<String> llObjectNames = new LinkedList();
    
    ListExpr tmp = Value.rest();
    if (tmp.isEmpty())
    {
      this.si.secondo("close database;", 
        new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
      if (this.errorCode.value != 0)
      {
        LogFileHandler.mlogger.severe(this.errorMessage.toString());
        return llObjects;
      }
      return llObjects;
    }
    SecondoObjectInfoClass secObjInfo = new SecondoObjectInfoClass();
    
    HashMap<String, SecondoObjectInfoClass> hmSecObjInfo = new HashMap();
    while (!tmp.isEmpty())
    {
      llObjectNames.add(tmp.first().second().symbolValue());
      
      ListExpr le1 = tmp.first().rest();
      
      UtilSecFunction uSec = new UtilSecFunction();
      if (uSec.getColumAndTypeFromListObject(le1.third()))
      {
        secObjInfo = new SecondoObjectInfoClass();
        secObjInfo.setStrObjName(tmp.first().second().symbolValue());
        secObjInfo.setColNames(uSec.malColName);
        secObjInfo.setColTypes(uSec.malColType);
        hmSecObjInfo.put(tmp.first().second().symbolValue(), secObjInfo);
      }
      tmp = tmp.rest();
    }
    llObjectNames = new UtilSecFunction().removeSec2XXXINFOObjects(llObjectNames);
    for (int i = 0; i < llObjectNames.size(); i++)
    {
      secObjInfo = new SecondoObjectInfoClass();
      secObjInfo.setStrObjName((String)llObjectNames.get(i));
      
      this.si.secondo("query " + (String)llObjectNames.get(i) + " feed count;", 
        this.resultList, this.errorCode, this.errorPos, this.errorMessage);
      if (this.errorCode.value != 0)
      {
        secObjInfo.setiCount(-1);
        llObjects.add(secObjInfo);
        setQueryResults2Null();
      }
      else
      {
        if (this.resultList.second().atomType() == 1)
        {
          secObjInfo.setiCount(this.resultList.second().intValue());
          if (hmSecObjInfo.containsKey(llObjectNames.get(i)))
          {
            secObjInfo.setColNames(((SecondoObjectInfoClass)hmSecObjInfo.get(llObjectNames.get(i))).getNames());
            secObjInfo.setColTypes(((SecondoObjectInfoClass)hmSecObjInfo.get(llObjectNames.get(i))).getTypes());
          }
        }
        else
        {
          secObjInfo.setiCount(-1);
        }
        llObjects.add(secObjInfo);
      }
    }
    this.si.secondo("close database;", 
      new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llObjects;
    }
    return llObjects;
  }
  /* 
   * @param strDatabasename
   * @param strObj
   * @return
   */
  
  public LinkedList<SecondoObjectInfoClass> getObjectsWithOutCount(String strDatabasename, String strObj)
  {
    LinkedList<SecondoObjectInfoClass> llObjects = new LinkedList();
    if (!isSecondoConnected())
    {
      LogFileHandler.mlogger.warning("Can not read Objects because the connection is not possible");
      return llObjects;
    }
    setQueryResults2Null();
    this.si.secondo("open database " + strDatabasename + ";", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llObjects;
    }
    this.si.secondo("list objects;", 
      this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llObjects;
    }
    this.si.secondo("close database;", 
      new ListExpr(), this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      return llObjects;
    }
    if ((this.resultList.first().atomType() != 5) || 
      (!this.resultList.first().symbolValue().equals("inquiry")) || 
      (this.resultList.listLength() != 2) || 
      (!this.resultList.second().first().symbolValue().equals("objects"))) {
      return llObjects;
    }
    if (this.resultList.second().second().listLength() <= 1) {
      return llObjects;
    }
    ListExpr liExpType = this.resultList.second().second().second();
    if (!liExpType.first().symbolValue().equals("OBJECT")) {
      return llObjects;
    }
    ListExpr VL = this.resultList.second();
    
    ListExpr Value = VL.second();
    LinkedList<String> llObjectNames = new LinkedList();
    
    ListExpr tmp = Value.rest();
    if (tmp.isEmpty()) {
      return llObjects;
    }
    SecondoObjectInfoClass secObjInfo = new SecondoObjectInfoClass();
    
    HashMap<String, SecondoObjectInfoClass> hmSecObjInfo = new HashMap();
    while (!tmp.isEmpty())
    {
      llObjectNames.add(tmp.first().second().symbolValue());
      
      ListExpr le1 = tmp.first().rest();
      
      UtilSecFunction uSec = new UtilSecFunction();
      if (uSec.getColumAndTypeFromListObject(le1.third()))
      {
        secObjInfo = new SecondoObjectInfoClass();
        secObjInfo.setStrObjName(tmp.first().second().symbolValue());
        secObjInfo.setColNames(uSec.malColName);
        secObjInfo.setColTypes(uSec.malColType);
        
        hmSecObjInfo.put(tmp.first().second().symbolValue(), secObjInfo);
      }
      tmp = tmp.rest();
    }
    llObjectNames = new UtilSecFunction().removeSec2XXXINFOObjects(llObjectNames);
    for (int i = 0; i < llObjectNames.size(); i++) {
      if (strObj.intern() == ((String)llObjectNames.get(i)).toString().intern())
      {
        secObjInfo = new SecondoObjectInfoClass();
        secObjInfo.setStrObjName((String)llObjectNames.get(i));
        if (hmSecObjInfo.containsKey(llObjectNames.get(i)))
        {
          secObjInfo.setColNames(((SecondoObjectInfoClass)hmSecObjInfo.get(llObjectNames.get(i))).getNames());
          secObjInfo.setColTypes(((SecondoObjectInfoClass)hmSecObjInfo.get(llObjectNames.get(i))).getTypes());
        }
        llObjects.add(secObjInfo);
      }
    }
    return llObjects;
  }
  
  /**
   * @param _sbCommand
   * @return true when the command was successfully sent
   */
  
  public boolean sendCommand(StringBuffer _sbCommand)
  {
    this.si.secondo(_sbCommand.toString(), this.resultList, this.errorCode, this.errorPos, this.errorMessage);
    if (this.errorCode.value != 0)
    {
      LogFileHandler.mlogger.warning("Error in this command: " + _sbCommand.toString());
      LogFileHandler.mlogger.severe(this.errorMessage.toString());
      
      return false;
    }
    return true;
  }
  
  public StringBuffer getMsbhostName()
  {
    return this.msbhostName;
  }
  
  public void setMsbhostName(StringBuffer msbhostName)
  {
    this.msbhostName = msbhostName;
  }
  
  public int getMiport()
  {
    return this.miport;
  }
  
  public void setMiport(int miport)
  {
    this.miport = miport;
  }
  
  public StringBuffer getMsbUser()
  {
    return this.msbUser;
  }
  
  public void setMsbUser(StringBuffer msbUser)
  {
    this.msbUser = msbUser;
  }
  
  public StringBuffer getMsbPwd()
  {
    return this.msbPwd;
  }
  
  public void setMsbPwd(StringBuffer msbPwd)
  {
    this.msbPwd = msbPwd;
  }
  
  public boolean isMbUseBinaryList()
  {
    return this.mbUseBinaryList;
  }
  
  public void setMbUseBinaryList(boolean mbUseBinaryList)
  {
    this.mbUseBinaryList = mbUseBinaryList;
  }
  
  public IntByReference getErrorCode()
  {
    return this.errorCode;
  }
  
  public IntByReference getErrorPos()
  {
    return this.errorPos;
  }
  
  public ListExpr getResultList()
  {
    return this.resultList;
  }
  
  public StringBuffer getErrorMessage()
  {
    return this.errorMessage;
  }
}
