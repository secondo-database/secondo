package Ext_Tools;


/**
 * 
 * <b> Task of this class </b> <br/>
 * This class can contain also an alias in addition to the class Qualifier.
 */
public class ShadowQualifier extends Qualifier {
	
	private String AsExpr;
	
	public ShadowQualifier(String att, String quali) {
		super(att, quali);
	}
	
	public ShadowQualifier(Qualifier Elem) {
		super(Elem.getAtt(), Elem.getQuali());
	}
	
	public void setAsExpr(String AsEx) {
		this.AsExpr = AsEx;
	}
	
	public String getAsExpr() {
		String result = "";
		if (this.AsExpr != null)
			result = this.AsExpr;
		
		return result;
	}

}
