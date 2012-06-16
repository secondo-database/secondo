package Ext_Tools;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a column name which is needed for the group-by part of a 
 * selection query. Since qualifiers are in use a class was therefore created.
 */
public class GroupElement {
	private String Name;
	private String Quali;
	
	public GroupElement(String name) {
		int PosColon;
		
		PosColon = name.indexOf(":");
		if (PosColon==-1) {
			this.Name = name;
			this.Quali = "";
		}
		else {
			this.Quali = name.substring(0, PosColon);
			this.Name = name.substring(PosColon+1);
		}
	}
	
	public boolean hasQuali() {
		return (this.Quali != "");
	}
	
	public void setQuali(String quali) {
		this.Quali = quali;
	}
	
	public String getAtt() {
		return this.Name;
	}
	
	public String getGElement(){
		String PreResult = "";
		if (this.Quali != "")
			PreResult = this.Quali + ":";
		PreResult += this.Name;
		
		return PreResult;
	}
	
	/* GroupClause runs through all Select-Elements. If it finds the first one which has the same 
	 * name as this element has got it will take the Qualifier of this Select-Element. This will 
	 * only be done in case this Group-Element has not had his own Qualifier. An Alias will not be
	 * accepted by the group-by part of secondo, but qualifiers will. 
	 */
	

}
