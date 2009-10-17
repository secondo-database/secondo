package  viewer.hoese.algebras;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


public class Dsplarel extends DsplGeneric{
	String entry;
    static final int INDENT = 4;		
    public void init (String name, int nameWidth, int indent,  ListExpr type, ListExpr value, QueryResult qr) {
		int select = value.first().intValue();
		value = value.rest();
		if (select == 0)
		{	
			String T = name;
			T = extendString(T, nameWidth, indent);
			entry=(T + " :");
			qr.addEntry(this);
			int maxAttribNameLen = maxAttributLength(type.second().second());
			if (nameWidth > maxAttribNameLen)
				maxAttribNameLen = nameWidth;
			while (!value.isEmpty()) {
				displayArelTuple(type.second().second(), value.first(), maxAttribNameLen, indent, qr);
				value = value.rest();
				if (!value.isEmpty())
					qr.addEntry(new Dsplend(indent + INDENT));
		    }
		}
		else
		{
			entry = "Saved TupleIds are:";
			qr.addEntry(entry);
			int i = 1;
			while (!value.isEmpty()){
				entry = "    TupleId no. " + i + ": " + value.first().intValue();
				qr.addEntry(entry);
				value = value.rest();
				i++;
			}
		}	
		return;
	}
	/**
	* A method which create an instance of each displayclass that appears as attribute-type, 
	* and calls its init method.
	* @see <a href="Dspltuplesrc.html#displayTuple">Source</a>
	*/
	private void displayArelTuple (ListExpr type, ListExpr value, int maxNameLen, int indent,QueryResult qr) {
		int i;
	  String s;
	  DsplBase dg;
	  while (!value.isEmpty()) {
	    s = type.first().first().symbolValue();
	    ListExpr subType = type.first().second();
	    while(subType.atomType()!=ListExpr.SYMBOL_ATOM){
	       subType = subType.first();
	    }
	    dg = LEUtils.getClassFromName(subType.symbolValue());
	    String typeName = subType.symbolValue();
	    // ensure to add exactly one entry per attribute
	    int oldnum = qr.getModel().getSize();
	    String name = type.first().first().symbolValue();
	    subType = type.first().second();
	     
	    dg.init(name, maxNameLen, indent +INDENT, subType, value.first(), qr);
        int newnum = qr.getModel().getSize();
        int diff = newnum-oldnum;
        if(diff<1){
        	tools.Reporter.writeError("missing entry for attribute "+s);
        	tools.Reporter.writeError("check the implementation of the class " + dg.getClass());
        	qr.addEntry("error");
        }
      if (!(typeName.equals("arel"))){
    	  if(diff>1){
    		  tools.Reporter.writeError("to many entries for attribute "+s+
                             "\n please check the implementation of the "+dg.getClass() + " class");
    	  }
      }
      type = type.rest();
      value = value.rest();
    }
    return;
	}

	/**
	* Calculate the length of the longest attribute name.
	* @param type A ListExpr of the attribute types
	* @return maximal length of attributenames
	* @see <a href="Dspltuplesrc.html#maxAttributLength">Source</a>
	*/
	private static final int maxAttributLength (ListExpr type) {
	    int max, len;
	    String s;
	    max = 0;
	    while (!type.isEmpty()) {
	      s = type.first().first().symbolValue();
	      len = s.length();
	      if (len > max)
	        max = len;
	      type = type.rest();
	    }
	    return  max;
	}
	
	public String toString(){
		return entry;
	}
	
	/**
	 * Auxiliary class to display the end of tuple sign in the correct position.
	 */
	public class Dsplend extends DsplGeneric{
		private int indent;
		private int n;
		public Dsplend (int indent){
			this.indent = indent;
		}
		public String toString(){
			String s = "---------";
			s = extendString(s, 0, indent );
			return s;
		}
	}
}


	
