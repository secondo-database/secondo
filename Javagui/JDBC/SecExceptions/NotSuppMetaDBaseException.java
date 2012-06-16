package SecExceptions;

import SQL2Secondo.ErrorCodes;
import java.sql.SQLException;

public class NotSuppMetaDBaseException extends SQLException {
	
	public NotSuppMetaDBaseException(String s) {
		super(ErrorCodes.NoComp1Ln + ErrorCodes.NoComp2Ln + s.toUpperCase() + ErrorCodes.NoCompMetaData);
		
	}

}
