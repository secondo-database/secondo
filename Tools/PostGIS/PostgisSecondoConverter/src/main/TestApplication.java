package main;

	import appGui.MainGui;
	import java.io.File;
	import secondoPostgisUtil.Configuration;
	import secondoPostgisUtil.IGlobalParameters;
	import secondoPostgisUtil.UtilFunctions;
	import appGuiUtil.IPublicTextMessages;
	import appGuiUtil.Message;

	public class TestApplication 
		implements IGlobalParameters, IPublicTextMessages
	{
	  public static void main(String[] args)
	  {
	    globalStringBufDirectory.append(File.separatorChar);
	    /*
	     * In the file system under C:\Users\Mama a folder with the name 
	     * 01SecPostgis will be created
	     */
	    globalstrbufferHomeDir.append(System.getProperty("user.home") + globalStringBufDirectory + 
	      "01Etie" + globalStringBufDirectory);

	    /*
	     * In the file system under C:\Users\yos10 a folder with the name 
	     * 01SecPostgis will be created
	     */
	    
	    globalStringBufConfigFile.append(globalstrbufferHomeDir + "Configuration.cfg");  
	    
	    gsbTableDelimiter.append(" - ");
	    
	    Configuration configDatei = new Configuration();
	    if (!configDatei.read()) {
	      new Message("Config: Standard values used because none IP Address is entered by the user.");
	    }
	    UtilFunctions utilfunc = new UtilFunctions();
	    utilfunc.removeAllTempFiles();
	    
	    new MainGui();
	  }
	
	
}
