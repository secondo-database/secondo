package Utilities;

import tools.Reporter;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a column in the table of the query result
 */
public class ColHead {
	
	private String Name;
	private int NumType;
	private String NamType;
	
	public ColHead(String name, String typ) {
		Name = name;
		NamType = typ;
		if (typ.equals("int"))
			this.NumType = 4;
		else if (typ.equals("real"))
			this.NumType = 8;
		else if (typ.equals("string"))
			this.NumType = 12;
		else if (typ.equals("bool"))
			this.NumType = -7;
		else
			Reporter.reportInfo(typ, true);
				
	}
		
	public String getName() {
		return this.Name;
	}
	public int getNumType()  {
		return this.NumType;
	}
		
	public String getNamType() {
		return this.NamType;
	}
}


