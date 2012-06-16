package Ext_Tools;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a Table Reference. In Secondo this can only be a relationname with an
 * optional qualifier name following
 */
public class TableElement {
	
	private String Name;
	private String NickName;
		
	public TableElement(String name, String nickname) {
		this.Name = name;
		this.NickName = nickname.trim();
	}
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/>
	 * In case the qualifier is part of the name
	 * @param name
	 */
	public TableElement(String name) {
		int PosColon;
		
		PosColon = name.indexOf(" as ");
		if (PosColon==-1) {
			this.Name = name;
			this.NickName = "";
		}
		else {
			this.Name = name.substring(0, PosColon-1);
			this.NickName = name.substring(PosColon+5);
		}
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks whether the tableElement contains the passed qualifier or contains no
	 * qualifier an the name of the TableElement equals it.
	 * @param quali
	 * @return true if the TableElement contains the qualifier
	 */
	public boolean checkQuali(String quali) {
		boolean result = false;
		if (this.NickName != "") {
			if (this.NickName.equalsIgnoreCase(quali))
				result = true;
		}
		else
			if (this.Name.equalsIgnoreCase(quali)) {
				result = true;
				this.NickName = quali;     // e.g. tentest as tentest
			}
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the TableElement in form of a string
	 * @return
	 */
	public String getTElement() {
		String result = this.Name;
		if (this.NickName != "")
			result += " as " + this.NickName;
		
		return result;
	}
	
	public String getNickname() {
		return this.NickName;
	}
}
