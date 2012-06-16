package Ext_Tools;

/**
 * 
 * <b>Task of this class: </b> <br/>
 * it contains the pair qualifier and attribute
 */
public class Qualifier {
	
	private String Att;
	private String Quali;
	
	public Qualifier(String att, String quali) {
		this.Quali = quali;
		this.Att = att;
	}
	
	public String getAtt() {
		return this.Att;
	}
	
	public String getQuali() {
		return this.Quali;
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * compares the passed Qualifier-Object with this one.
	 * @param Other
	 * @return true if passed Qualifier-Object equals this one
	 */
	public boolean equals(Qualifier Other) {
		boolean result = false;
		
		if (this.Quali.equalsIgnoreCase(Other.getQuali()) && this.Att.equalsIgnoreCase(Other.getAtt()))
			result = true;
		
		return result;
	}

}
