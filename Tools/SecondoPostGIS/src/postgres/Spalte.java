package postgres;

/**
 * @author Bill
 *
 */
public class Spalte {
	
	private StringBuffer sbName;
	private StringBuffer sbTyp;
	private boolean bPrimaryKey;
	private boolean bForeignKey;
	

	public Spalte()
	{
		sbName = new StringBuffer();
		sbTyp = new StringBuffer();
		bPrimaryKey = false;
		bForeignKey = false;
	}


	public StringBuffer getSbName() {
		return sbName;
	}


	public void setSbName(StringBuffer sbName) {
		this.sbName.delete(0, this.sbName.length());
		this.sbName.append(sbName);
	}


	public StringBuffer getSbTyp() {
		return sbTyp;
	}


	public void setSbTyp(StringBuffer sbTyp) {
		this.sbTyp.delete(0, this.sbTyp.length());
		this.sbTyp.append(sbTyp);
	}


	public boolean isbPrimaryKey() {
		return bPrimaryKey;
	}


	public void setbPrimaryKey(boolean bPrimaryKey) {
		this.bPrimaryKey = bPrimaryKey;
	}


	public boolean isbForeignKey() {
		return bForeignKey;
	}


	public void setbForeignKey(boolean bForeignKey) {
		this.bForeignKey = bForeignKey;
	}
	
	
	
}
