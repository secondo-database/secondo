package communication.optimizer;

public class ErrorCodes{

public static final int NO_ERROR=0;
public static final int NO_OPTIMIZER=1;
public static final int UNKNOW_HOST=2;
public static final int IO_ERROR=3;
public static final int SECURITY_ERROR=4;
public static final int NOT_CONNECTED =5;
public static final int PROTOCOL_ERROR = 6;
public static final int OPTIMIZATION_FAILED = 7;
public static final int NO_OPTIMIZATION_POSSIBLE = 8;
public static final int CONNECTION_BROKEN = 9;


public  static String getErrorMessage(int ErrorCode){

 switch(ErrorCode){
   case NO_ERROR     : return "no error";
   case NO_OPTIMIZER : return "server is not an optimizer";
   case UNKNOW_HOST  : return "host-name unknown";
   case IO_ERROR     : return "io-error";
   case SECURITY_ERROR : return "security error";
   case NOT_CONNECTED  : return "not connected";
   case PROTOCOL_ERROR : return "protocol error";
   case OPTIMIZATION_FAILED : return "optimization failed";
   case NO_OPTIMIZATION_POSSIBLE : return "no optimization possible";
   case CONNECTION_BROKEN : return "connection to server lost";
   default : return "unknow error-code";
 }



}






}
