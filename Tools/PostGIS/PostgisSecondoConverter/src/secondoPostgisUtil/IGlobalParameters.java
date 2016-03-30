package secondoPostgisUtil;

import java.awt.Image;
import java.awt.Toolkit;

/*
 * This class defines parameters which will be used during 
 * all the applications as global parameters
 */

public abstract interface IGlobalParameters
{
	public static final StringBuffer globalstrbufferHomeDir = new StringBuffer();  
	public static final StringBuffer globalStringBufDirectory = new StringBuffer(); 
	public static final StringBuffer gsbTableDelimiter = new StringBuffer();
	public static final StringBuffer globalStringBufConfigFile = new StringBuffer();
	public static final Image gimp_S2P = Toolkit.getDefaultToolkit().getImage(IGlobalParameters.class.getResource("/s2p.gif"));
	public static final StringBuffer gsbPG_Host = new StringBuffer();
	public static final StringBuffer gsbPG_Port = new StringBuffer();
	public static final StringBuffer gsbPG_User = new StringBuffer();
	public static final StringBuffer gsbPG_Pwd = new StringBuffer();
	public static final StringBuffer gsbSEC_Host = new StringBuffer();
	public static final StringBuffer gsbSEC_Port = new StringBuffer();
	public static final StringBuffer gsbSEC_User = new StringBuffer();
	public static final StringBuffer gsbSEC_Pwd = new StringBuffer();
	public static final StringBuffer gsbSEC_UseBinaryList = new StringBuffer();
	public static final int gMaxTableSize = 10000000;
	public static final String gStrMaxLIMITTABLEQUERY = "50";
}
