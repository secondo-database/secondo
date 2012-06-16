package SecExceptions;

import java.sql.SQLException;

public class NotCompSQL92Exception extends SQLException {
	
	public NotCompSQL92Exception(String s) {
		super(s.toUpperCase() + " is not part of SQL92 and is therefore not supported in secondo!");
	}
}
