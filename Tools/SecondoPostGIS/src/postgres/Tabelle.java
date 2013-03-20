package postgres;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedList;

public class Tabelle {

	private StringBuffer sbName;
	private int iRows;
	public final static StringBuffer sbTableRowDelimiter = new StringBuffer(" - ");
	
	public Tabelle()
	{
		sbName = new StringBuffer();
		iRows =0;
	}
	
	
	public StringBuffer getSbName() {
		return sbName;
	}


	public void setSbName(StringBuffer sbName) {
		this.sbName.delete(0, this.sbName.length());
		this.sbName.append(sbName);
	}


	public int getiRows() {
		return iRows;
	}


	public void setiRows(int iRows) {
		this.iRows = iRows;
	}
	
	public StringBuffer getShowText()
	{
		StringBuffer sbReturn = new StringBuffer();
		sbReturn.append(this.getSbName());
		sbReturn.append(sbTableRowDelimiter);
		sbReturn.append(this.getiRows());
		return sbReturn;
	}
	
}
