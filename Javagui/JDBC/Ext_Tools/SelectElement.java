package Ext_Tools;

/**
 * <b> Task of this class </b> <br/>
 * This class contains a select element consisting of a name
 * and an optional qualifier. If a qualifier is needed it will be added
 * by an object of the class Qualifier
 *
 */
public class SelectElement {
	
	private String Name;
	private String Quali;
	private String AsExpr;
	private boolean isSetFunc;
	private String SetFunc;
	
	
	/**
	 * <b> Task of this constructor </b> <br/>
	 * The parameter name might hold a qualifier which needs to be extracted
	 * @param name, AsEx
	 */
	public SelectElement(String name, String AsEx, boolean SetFunc) {
		int PosColon;
		int PosBracket;
		String tempName;
		
		this.AsExpr = AsEx;
		this.isSetFunc = SetFunc;
		this.SetFunc = "";
		tempName = name;
		if (this.isSetFunc) {
			PosBracket = tempName.indexOf("(");
			this.SetFunc = tempName.substring(0, PosBracket);
			tempName = tempName.substring(PosBracket+1, tempName.length()-1);
		}
		PosColon = tempName.indexOf(":");
		if (PosColon==-1) {
			this.Name = tempName;
			this.Quali = "";
		}
		else {
			this.Quali = tempName.substring(0, PosColon);
			this.Name = tempName.substring(PosColon+1);
		}
	}
	
	public boolean hasQuali() {
		return (this.Quali != "");
	}
	
	public boolean hasAsExp() {
		return (this.AsExpr != "");
	}
	
	public boolean hasSetFunc() {
		return this.isSetFunc;
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * To set an AsExpr in case of a Set_Function_specification combined with groupby
	 * @param asexp
	 */
	public void setAsExpr(String asexp) {
		this.AsExpr = asexp;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks weather the parameter Attname equals the attribute in the select element
	 * @param AttName
	 * @return
	 */
	public boolean checkName(String AttName) {
		return (this.Name.equalsIgnoreCase(AttName));
	}
	
	public boolean checkQuali(String QualiName) {
		return (this.Quali.equalsIgnoreCase(QualiName));
	}
	
	public void setQuali(String quali) {
		if (this.Quali == "")
			this.Quali = quali;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the SelectElement in form of a string
	 * @return
	 */
	public String getSElement(){
		String PreResult = "";
		if (this.isSetFunc)
			PreResult = this.SetFunc + "(";
		if (this.Quali != "")
			PreResult += this.Quali + ":";
		PreResult += this.Name;
		if (this.isSetFunc)
			PreResult += ")";
		if (this.AsExpr != "")
			PreResult += " as " + this.AsExpr;
		return PreResult;
	}
	
	public String getquali() {
		return this.Quali;
	}
	
	public String getAtt() {
		return this.Name;
	}
	
	public String getAsExpr() {
		return this.AsExpr;
	}
	

}
