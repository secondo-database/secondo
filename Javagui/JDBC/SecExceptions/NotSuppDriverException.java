package SecExceptions;

import SQL2Secondo.ErrorCodes;
import java.sql.SQLException;

public class NotSuppDriverException extends SQLException {
	
	public NotSuppDriverException(String s) {
		super(ErrorCodes.NoComp1Ln + ErrorCodes.NoComp2Ln + s.toUpperCase() + ErrorCodes.NoCompDriver);
	}
}
