package LocalTest;
import communication.CommunicationInterface;
import DriverSet.Driver;
import SecExceptions.*;
import tools.Reporter;

public class Starten {

	
	/**
	 *
	 * <b> Task of this method </b> <br/>
	 * checks whether a connection can be established to the database
	 * @param args
	 * @throws Exception
	 */
	public static void main(String[] args) throws Exception{
		// TODO Auto-generated method stub
		
		//Driver TestDriver = new Driver();
		//String Ergebnis = TestDriver.getDBName(" iDBC : Secondo :  TestquEries  ");
		//System.out.println(Ergebnis);
		//Reporter.showMessage("Jetzt starten wir");
		CommunicationInterface CI = new CommunicationInterface();
		CI.initialize(Declarations.IP_ADRESS, Declarations.SEC_PORT, Declarations.OPT_PORT);
		if (CI.connectToDB("testqueries") /*&& CI.Testit()*/) {
			try { 
				Reporter.reportInfo("Alles startklar", true);
				if (!CI.executeCommand(Declarations.TQCreateTable2)) 
					throw new SecServerException(Declarations.TQCreateTable2); 
				/*if (!CI.executeCommand(Declarations.TQInsert2)) 
					throw new SecServerException(Declarations.TQInsert2);
				if (!CI.executeCommand(Declarations.TQInsert3)) 
					throw new SecServerException(Declarations.TQInsert3);*/
				
			}
			catch(SecServerException s) {
				Reporter.writeError(s.getMessage());
			}
			CI.closeDB();
		}
		
		
		
		
	}

}
