package Ext_Tools;

import java.util.*;
import java.sql.SQLWarning;

/**
 * 
 * <b> Task of this class </b> <br/>
 * This class contains the answer provided by QueryClause and the list of ShadowQualifiers
 */
public class completeAnswer {
 
	
	private Vector<ShadowQualifier> ShadowList;
	private String Output;
	private boolean ShadowListExists;
	private boolean SubqueryInUse;
	//private SQLWarning warning;
	
	public completeAnswer(Vector<ShadowQualifier> ShList, String Out, boolean Subquery) {
		if (ShList != null && !ShList.isEmpty()) {
			this.ShadowList = ShList;
			this.ShadowListExists = true;
		}
		else
			this.ShadowListExists = false;
		this.Output = Out;
		this.SubqueryInUse = Subquery;
		//this.warning = warn;
	}
	
	public completeAnswer(String Out, boolean Subquery) {
		this.ShadowListExists = false;
		this.SubqueryInUse = Subquery;
		this.Output = Out;
		//this.warning = warn;
	}
	
		
	public boolean isSubqueryInUse() {
		return this.SubqueryInUse;
	}
	
	public String getOutput() {
		return this.Output;
	}
	
	public Vector<ShadowQualifier> getShadowList() {
		return this.ShadowList;
	}
	
	public boolean hasShadowList() {
		return this.ShadowListExists;
	}

}
