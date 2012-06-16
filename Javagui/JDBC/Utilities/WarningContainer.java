package Utilities;

import java.sql.SQLWarning;

public class WarningContainer extends SQLWarning {
	private static WarningContainer instance = null;
	
	public static WarningContainer getInstance() {
		if (instance == null) {
			instance = new WarningContainer();
		}
		return instance;
	}
	
	private WarningContainer() {
		super();
	}
	
	public static WarningContainer ClearWarning() {
		instance = new WarningContainer();
		return instance;
	}

}
