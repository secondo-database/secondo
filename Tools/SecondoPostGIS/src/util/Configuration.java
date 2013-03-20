/**
 * 
 */
package util;

import java.awt.BorderLayout;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;




/**
 * @author Bill
 *
 */
public class Configuration implements IMyLogger
{

	public Logger mlogger;
	
	private LinkedProperties mprop = null;
	private FileOutputStream mfileOPstream = null;
	private File mfp = null;
	
	//Variablen in der Configurationsdatei
	//PostGis
	private StringBuffer msbPG_Host;
	private StringBuffer msbPG_Port;
	private StringBuffer msbPG_User;
	private StringBuffer msbPG_Pwd;
		
		
	//Secondo
	private StringBuffer msbSEC_Host;
	private StringBuffer msbSEC_Port;
	private StringBuffer msbSEC_User;
	private StringBuffer msbSEC_Pwd;
	private StringBuffer msbSEC_UseBinaryList;

	
	
	/**
	 * 
	 */
	public Configuration() {
		super();
		
		//Init Werte für Globale Werte der Configurationsdatei:
		//=======================================================
		
		//PostGis
		
		Global.gsbPG_Host.append("192.168.178.35");
		Global.gsbPG_Port.append("5432");
		Global.gsbPG_User.append("postgres");
		Global.gsbPG_Pwd.append("123");
		
		
		//Secondo
		Global.gsbSEC_Host.append("192.168.142.128");
		Global.gsbSEC_Port.append("1234");
		Global.gsbSEC_User.append("");
		Global.gsbSEC_Pwd.append("");
		Global.gbSEC_UseBinaryList = true;
		
		
		makeFolder();
		
		initLogger(new StringBuffer(Configuration.class.getName()), true);
		
		mprop = new LinkedProperties();
		
		mfp = new File(Global.gsbHomeDir.toString());
		
		msbPG_Host = new StringBuffer("PG-Host");
		msbPG_Port = new StringBuffer("PG-Port");
		msbPG_User = new StringBuffer("PG-User");
		msbPG_Pwd = new StringBuffer("PG-Pwd");
		msbSEC_Host = new StringBuffer("SEC-Host");
		msbSEC_Port = new StringBuffer("SEC-Port");
		msbSEC_User = new StringBuffer("SEC-User");
		msbSEC_Pwd = new StringBuffer("SEC-Pwd");
		msbSEC_UseBinaryList = new StringBuffer("SEC-UseBinaryList");

		
	}
	
	public Configuration(int iValues) {
		super();
		
		//Init Werte für Globale Werte der Configurationsdatei:
		//=======================================================
		
		
		
		
		//PostGis
		/*Global.gsbPG_Host.append(Global.gsbPG_Host);
		Global.gsbPG_Port.append(Global.gsbPG_Port);
		Global.gsbPG_User.append(Global.gsbPG_User);
		Global.gsbPG_Pwd.append(Global.gsbPG_Pwd);
		
		
		//Secondo
		Global.gsbSEC_Host.append(Global.gsbSEC_Host);
		Global.gsbSEC_Port.append(Global.gsbSEC_Port);
		Global.gsbSEC_User.append(Global.gsbSEC_User);
		Global.gsbSEC_Pwd.append(Global.gsbSEC_Pwd);
		Global.gbSEC_UseBinaryList = true;
		*/
		
		makeFolder();
		
		initLogger(new StringBuffer(Configuration.class.getName()), true);
		
		mprop = new LinkedProperties();
		
		mfp = new File(Global.gsbHomeDir.toString());
		
		msbPG_Host = new StringBuffer("PG-Host");
		msbPG_Port = new StringBuffer("PG-Port");
		msbPG_User = new StringBuffer("PG-User");
		msbPG_Pwd = new StringBuffer("PG-Pwd");
		msbSEC_Host = new StringBuffer("SEC-Host");
		msbSEC_Port = new StringBuffer("SEC-Port");
		msbSEC_User = new StringBuffer("SEC-User");
		msbSEC_Pwd = new StringBuffer("SEC-Pwd");
		msbSEC_UseBinaryList = new StringBuffer("SEC-UseBinaryList");

		
	}
	
	
	public boolean makeFolder()
	{
		mfp = new File(Global.gsbHomeDir.toString());
		if(mfp.exists() == false)
		{
			return mfp.mkdir();
		}
		else
			return true;
		
	}
	
	
	@SuppressWarnings("finally")
	public boolean write()
	{
		boolean bReturn = false;
		
		mfp = new File(Global.gsbHomeDir.toString());
		
		if(mfp.exists() == false)
		{
			mfp.mkdir();
		}
		
		mfp = new File(Global.gsbConfigFile.toString());
		
		try 
		{
			mfileOPstream = new FileOutputStream(mfp,false);
			
			mprop.setProperty(msbPG_Host.toString(), Global.gsbPG_Host.toString());
			mprop.setProperty(msbPG_Port.toString(), Global.gsbPG_Port.toString());
			mprop.setProperty(msbPG_User.toString(), Global.gsbPG_User.toString());
			mprop.setProperty(msbPG_Pwd.toString(),  Global.gsbPG_Pwd.toString());
			
			mprop.setProperty(msbSEC_Host.toString(), Global.gsbSEC_Host.toString());
			mprop.setProperty(msbSEC_Port.toString(), Global.gsbSEC_Port.toString());
			mprop.setProperty(msbSEC_User.toString(), Global.gsbSEC_User.toString());
			mprop.setProperty(msbSEC_Pwd.toString(),  Global.gsbSEC_Pwd.toString());
			mprop.setProperty(msbSEC_UseBinaryList.toString(), Boolean.toString(Global.gbSEC_UseBinaryList));
	
			//save properties 
	
			mprop.store(mfileOPstream, "Configuration-File");
			
			mfileOPstream.close();
			bReturn = true;
		} 
		catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			//e.printStackTrace();
			this.mlogger.severe(e.toString());
			bReturn = false;
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			this.mlogger.severe(e.toString());
			bReturn = false;
		}
		finally
		{
			if(mfileOPstream!=null)
				try {
					mfileOPstream.close();
				} catch (final IOException e) {
					// TODO Auto-generated catch block
					this.mlogger.severe(e.toString());
				}
			
			if(bReturn == true)
				this.mlogger.info("Config geschrieben");
			else
				this.mlogger.warning("Fehler beim schreiben der Config-Datei");
			
			return bReturn;
			
		}
		

		
	}

	@SuppressWarnings("finally")  //weil wir mit dem catch nicht alle Exceoption und Error fangen
	public boolean read()
	{
		
		boolean bReturn = false;
		
		mfp = new File(Global.gsbConfigFile.toString());
		
		if(mfp.exists() == false || mfp.canRead() == false)
		{
			
			this.insertDialogs();
			
			//System.out.println("test");
			if(this.write() == false)
			{
				//System.out.println("Keine Config schreibbar");
				this.mlogger.warning("Keine Config schreibbar");
				return false;
			}
		}
		
		
		//Lese Config-Datei --> ggf. mit Standardwerten initalisieren
		
		try 
		{
			mprop.load(new FileInputStream(mfp));
		
			//vorher die Variablen löschen
			Global.gsbPG_Host.delete(0, Global.gsbPG_Host.length());
			Global.gsbPG_Port.delete(0, Global.gsbPG_Port.length());
			Global.gsbPG_User.delete(0, Global.gsbPG_User.length());
			Global.gsbPG_Pwd.delete(0, Global.gsbPG_Pwd.length());

			Global.gsbSEC_Host.delete(0, Global.gsbSEC_Host.length());
			Global.gsbSEC_Port.delete(0, Global.gsbSEC_Port.length());
			Global.gsbSEC_User.delete(0, Global.gsbSEC_User.length());
			Global.gsbSEC_Pwd.delete(0, Global.gsbSEC_Pwd.length());
			
			Global.gsbPG_Host.append(mprop.getProperty(msbPG_Host.toString(), Global.gsbPG_Host.toString()));
			Global.gsbPG_Port.append(mprop.getProperty(msbPG_Port.toString(), Global.gsbPG_Port.toString()));
			Global.gsbPG_User.append(mprop.getProperty(msbPG_User.toString(), Global.gsbPG_User.toString()));
			Global.gsbPG_Pwd.append(mprop.getProperty(msbPG_Pwd.toString(),  Global.gsbPG_Pwd.toString()));
			
			Global.gsbSEC_Host.append(mprop.getProperty(msbSEC_Host.toString(), Global.gsbSEC_Host.toString()));
			Global.gsbSEC_Port.append(mprop.getProperty(msbSEC_Port.toString(), Global.gsbSEC_Port.toString()));
			Global.gsbSEC_User.append(mprop.getProperty(msbSEC_User.toString(), Global.gsbSEC_User.toString()));
			Global.gsbSEC_Pwd.append(mprop.getProperty(msbSEC_Pwd.toString(),  Global.gsbSEC_Pwd.toString()));
			
			Global.gbSEC_UseBinaryList = Boolean.valueOf(mprop.getProperty(
					msbSEC_UseBinaryList.toString(), Boolean.toString(Global.gbSEC_UseBinaryList)));
			
			//Ports dürfen nur Zahlen enthalten:
			
			if(Global.gsbPG_Port.toString().matches("^((0-9)+)$") != true)
			{
				Global.gsbSEC_Port.delete(0, Global.gsbPG_Port.length());
				Global.gsbSEC_Port.append("5432");
			}
			if(Global.gsbSEC_Port.toString().matches("^((0-9)+)$") != true)
			{
				Global.gsbSEC_Port.delete(0, Global.gsbSEC_Port.length());
				Global.gsbSEC_Port.append("1234");
			}
			
			bReturn = true;
		} 
		catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			this.mlogger.severe(e.toString());
			bReturn =  false;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			this.mlogger.severe(e.toString());
			bReturn = false;
		}
		finally  
		{
			if(bReturn == true)
				this.mlogger.info("Config gelesen");
			else
				this.mlogger.info("Fehler beim lesen der Config-Datei");
				
			return bReturn;
		}
		
		
	}

	
	@Override
	public void initLogger(StringBuffer _strClassName, boolean _bLogConsole)
	{
		// TODO Auto-generated method stub

		
		this.mlogger = Logger.getLogger(_strClassName.toString());
		
		this.mlogger.setUseParentHandlers(false);
		this.mlogger.setLevel(Level.ALL);
		FileHandler fh = null;
		try 
		{
			//System.out.println("Hallo1");
			fh = new FileHandler(mstrLogDatei, miMaxSize, miCountRotate, mbLogAppend);
			//System.out.println("Hallo2");
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		this.mlogger.addHandler(fh);
		
		// ConsolenHandler konfigurieren wenn er gewÃ¼nscht ist
		if(_bLogConsole == true)
		{
			ConsoleHandler ch = new ConsoleHandler();
			ch.setLevel(Level.ALL);
			this.mlogger.addHandler(ch);
		}
		
	}
	
	
	
	public void insertDialogs()
	{
		
		
		JPasswordField passwordField = new JPasswordField();
		passwordField.setEchoChar('#');
		JLabel jlabelPasswordText = new JLabel("");
		
		
        
		
		String strReturnValue = (String)JOptionPane.showInputDialog(
                null,
                "Please enter Postgres/Postgis host",
                "Configuration input",
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,  
                Global.gsbPG_Host.toString());

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbPG_Host.delete(0, Global.gsbPG_Host.length());
			Global.gsbPG_Host.append(strReturnValue);
		}
		
		

		strReturnValue = (String)JOptionPane.showInputDialog(
                null,
                "Please enter Postgres/Postgis port",
                "Configuration input",
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,  
                Global.gsbPG_Port.toString());

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbPG_Port.delete(0, Global.gsbPG_Port.length());
			Global.gsbPG_Port.append(strReturnValue);
		}

		
		
		strReturnValue = (String)JOptionPane.showInputDialog(
                null,
                "Please enter Postgres/Postgis user",
                "Configuration input",
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,  
                Global.gsbPG_User.toString());

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbPG_User.delete(0, Global.gsbPG_User.length());
			Global.gsbPG_User.append(strReturnValue);
		}


		
		JPanel jpanel = new JPanel(new BorderLayout());
		jlabelPasswordText.setText("Please enter Postgres/Postgis password");
		jpanel.add(jlabelPasswordText, BorderLayout.NORTH);
		jpanel.add(passwordField, BorderLayout.SOUTH);
		
        JOptionPane.showConfirmDialog(null,
                jpanel,
                "Configuration input",
                JOptionPane.OK_CANCEL_OPTION);
       
        strReturnValue = String.valueOf(passwordField.getPassword());
        passwordField.setText("");
        
			
		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbPG_Pwd.delete(0, Global.gsbPG_Pwd.length());
			Global.gsbPG_Pwd.append(strReturnValue);
		}

		

		strReturnValue = (String)JOptionPane.showInputDialog(
                null,
                "Please enter Secondo host",
                "Configuration input",
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,  
                Global.gsbSEC_Host.toString());

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbSEC_Host.delete(0, Global.gsbSEC_Host.length());
			Global.gsbSEC_Host.append(strReturnValue);
		}

		
		
		strReturnValue = (String)JOptionPane.showInputDialog(
                null,
                "Please enter Secondo port",
                "Configuration input",
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,  
                Global.gsbSEC_Port.toString());

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbSEC_Port.delete(0, Global.gsbSEC_Port.length());
			Global.gsbSEC_Port.append(strReturnValue);
		}

		
		strReturnValue = (String)JOptionPane.showInputDialog(
                null,
                "Please enter Secondo user",
                "Configuration input",
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,  
                Global.gsbSEC_User.toString());

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbSEC_User.delete(0, Global.gsbSEC_User.length());
			Global.gsbSEC_User.append(strReturnValue);
		}
		
		

		jpanel = new JPanel(new BorderLayout());
		jlabelPasswordText.setText("Please enter Secondo password");
		jpanel.add(jlabelPasswordText, BorderLayout.NORTH);
		jpanel.add(passwordField, BorderLayout.SOUTH);
		
        
		 JOptionPane.showConfirmDialog(null,
	                jpanel,
	                "Configuration input",
	                JOptionPane.OK_CANCEL_OPTION);
	       
		 passwordField.setText("");
        

		if ((strReturnValue != null) && (strReturnValue.length() > 0)) 
		{
			Global.gsbSEC_Pwd.delete(0, Global.gsbSEC_Pwd.length());
			Global.gsbSEC_Pwd.append(strReturnValue);
		}


	}
	
	
	
}
