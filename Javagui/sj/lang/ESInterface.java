/* The Extended Secondo Interface
   05-2003, Thomas Behr
 */

package  sj.lang;


import java.net.*;
import java.io.*;
import java.util.StringTokenizer;

/* this class extends the SecondoInterface
 * to make the change of server settings easy
 */
public class ESInterface extends SecondoInterface{

  /*Configuration parameters*/
  private String databasesServerAddress;
  private int databasesServerPort;
  private String UserName="";
  private String PassWd="";


  /* set the UserName for Secondo-Server */
  public void setUserName(String UserName){
   System.out.println("Warning: UserName is not supported by Secondo-Server");
   this.UserName = UserName;
  }

  /* set the Passwd for secondo-Server */
  public void setPassWd(String PassWd){
   System.out.println("Warning: PassWd is not supported by Secondo-Server");
   this.PassWd = PassWd;
  }

  /* set the Hostname of the Secondo-Server */
  public void setHostname(String HostName){
    databasesServerAddress = HostName;
  }

  /* set the Port of the Secondo-server */
  public void setPort(int Port){
   if (Port>=0)
     databasesServerPort = Port;
  }


  /* get the current UserName */
  public String getUserName(){
     return UserName;
  }

  /* get the current Passwd  */
  public String getPassWd(){
     return PassWd;
  }

  /* get the Hostname of the Secondo-Server */
  public String getHostname(){
     return databasesServerAddress;
  }

  /* get the current Port  */
  public int getPort(){
     return databasesServerPort;
  }


  /** returns true if this interface is connected to secondo server */
  public boolean isInitialized(){
    return initialized;
  } 


  public ESInterface()
  {
    super();
  }


  /**
   * connect to Secondo server with current settings
   * a old connection is terminated
   * @return True if success
   */
  public boolean connect() {
    terminate();   // terminate old connection
    return initialize(UserName,PassWd,databasesServerAddress,databasesServerPort);
  }

   public boolean isConnected(){
     return connected;
   } 
   

  /** calls super.terminate and set initialized to false */
  public void terminate(){
     super.terminate();
     connected = false;
     initialized = false; // allow a new connection
  }

 
  /** calls super.initialize and update connected state */
  public boolean initialize( String user, String pswd,
                             String host, int port ){
    if(!initialized)
       connected = super.initialize(user,pswd,host,port);
    return connected;
  }



  /** calls super.secondo 
    * if error points to lost connection then 
    * connected  is set to false 
    */
  public void secondo( String commandText,
                       ListExpr commandLE,
                       int commandLevel,
                       boolean commandAsText,
                       boolean resultAsText,
                       ListExpr resultList,
                       IntByReference errorCode,
                       IntByReference errorPos,
                       StringBuffer errorMessage ){
     super.secondo(commandText,commandLE,commandLevel,commandAsText,resultAsText,
                   resultList,errorCode,errorPos,errorMessage);
     if (errorCode.value==81)
        terminate();
  }  


  protected boolean connected=false;

}
