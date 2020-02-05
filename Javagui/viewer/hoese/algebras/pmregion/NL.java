package viewer.hoese.algebras.pmregion;

import java.util.ArrayList;
import java.util.List;
import sj.lang.ListExpr;

/**
 * Class NL provides a Nested List infrastructure for (de)serializing
 * objects.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class NL {
    /** The different types of list elements */
    public static final int
            UNK = 0,  // Undef
            NR = 1,   // Number (double)
            L = 2,    // Sublist
            STR = 3,  // String
            SYM = 4,  // Symbol
            BOOL = 5; // Boolean value
    
    /** the type of this element */
    private int type;
    
    // Information of the corresponding type 
    private Double nr;
    private List<NL> nl;
    private String str;
    private String sym;
    private Boolean bool;
    
    /** The parent nested list */
    private NL parent;
    
    /**
     * Construct an empty nested list.
     */
    public NL() {
        this.type = NL.L;
        nl = new ArrayList();
    }
    
    /**
     * Construct a nested list from a Secondo ListExpr
     * 
     * @param le secondo ListExpr
     */
    public NL (ListExpr le) {
        switch (le.atomType()) {
            case ListExpr.BOOL_ATOM:
                this.type = NL.BOOL;
                this.bool = le.boolValue();
                break;
            case ListExpr.INT_ATOM:
                this.type = NL.NR;
                this.nr = (double) le.intValue();
                break;
            case ListExpr.REAL_ATOM:
                this.type = NL.NR;
                this.nr = le.realValue();
                break;
            case ListExpr.SYMBOL_ATOM:
                this.type = NL.SYM;
                this.sym = le.symbolValue();
                break;
            case ListExpr.STRING_ATOM:
                this.type = NL.STR;
                this.str = le.stringValue();
                break;
            case ListExpr.NO_ATOM:
                this.type = NL.L;
                nl = new ArrayList();
                while (!le.isEmpty()) {
                    nl.add(new NL(le.first()));
                    le = le.rest();
                }
                break;
        }
    }
    
    /**
     * Construct a nested list of the given type
     * 
     * @param type type of nested list
     */
    public NL (int type) {
        this.type = type;
        if (type == NL.L) {
            nl = new ArrayList();
        }
    }
    
    /**
     * Add a sublist to this nested list and return it.
     * 
     * @return the new sublist
     */
    public NL nest() {
        NL sublist = new NL(NL.L);
        sublist.setParent(this);
        getNl().add(sublist);
        
        return sublist;
    }
        
    /**
     * Get the nested list element at the given index
     * 
     * @param index index in the nested list
     * @return the list element
     */
    public NL get(int index) {
        return nl.get(index);
    }
    
    /**
     * Returns the number of elements in this nested list.
     * 
     * @return number of elements
     */
    public int size () {
        return nl.size();
    }
        
    /**
     * Add a new boolean element.
     * 
     * @param b the boolean value to add
     * @return the new element
     */
    public NL addBoolean (Boolean b) {
        NL n = new NL(NL.BOOL);
        n.setBool(b);
        n.setParent(this);
        nl.add(n);
        
        return n;
    }
    
    /**
     * Add a new number element.
     * 
     * @param d value of the number to add
     * @return the new element
     */
    public NL addNr (Double d) {
        NL n = new NL(NL.NR);
        n.setNr(d);
        n.setParent(this);
        nl.add(n);
        
        return n;
    }
    
    /**
     * Add a new list element.
     * 
     * @param n the list element to add
     * @return the new element
     */
    public NL addNL (NL n) {
        nl.add(n);
        
        return n;
    }
    
    /**
     * Add a new string element
     * 
     * @param s the string to add
     * @return the new list element
     */
    public NL addStr (String s) {
        NL n = new NL(NL.STR);
        n.setStr(s);
        n.setParent(this);
        nl.add(n);
        
        return n;
    }
    
    /**
     * Add a new symbol element
     * 
     * @param s the symbol to add
     * @return the new list element
     */
    public NL addSym (String s) {
        NL n = new NL(NL.SYM);
        n.setSym(s);
        n.setParent(this);
        nl.add(n);
        
        return n;
    }

    /**
     * Get the parent nested list.
     * 
     * @return the parent nested list
     */
    public NL getParent() {
        return parent;
    }

    /**
     * Set the parent nested list.
     * 
     * @param parent the parent nested list
     */
    public void setParent(NL parent) {
        this.parent = parent;
    }

    /**
     * Get the type of this nested list element.
     * 
     * @return type of list element
     */
    public int getType() {
        return type;
    }

    /**
     * Set the type of this nested list element.
     * 
     * @param type type of list element
     */
    public void setType(int type) {
        this.type = type;
    }

    /**
     * Get the number of a number list element.
     * 
     * @return number value
     */
    public Double getNr() {
        return nr;
    }

    /**
     * Set the number of a number list element.
     * 
     * @param number number value
     */
    public void setNr(Double nr) {
        this.nr = nr;
    }

    /**
     * Get the list items of a sublist element.
     * 
     * @return list items
     */
    public List<NL> getNl() {
        return nl;
    }

    /**
     * Set the list items of a sublist element.
     * 
     * @param nl list items
     */
    public void setNl(List<NL> nl) {
        this.nl = nl;
    }

    /**
     * Get the string value of a string type list element
     * 
     * @return string value
     */
    public String getStr() {
        return str;
    }

    /**
     * Set the string value of a string type list element
     * 
     * @param string string value
     */
    public void setStr(String str) {
        this.str = str;
    }

    /**
     * Get the symbol value of a symbol type list element
     * 
     * @return symbol value
     */
    public String getSym() {
        return sym;
    }

    /**
     * Set the symbol value of a string type list element
     * 
     * @param sym symbol value
     */
    public void setSym(String sym) {
        this.sym = sym;
    }

    /**
     * Get the bool value of a boolean type list element
     * 
     * @return bool value
     */
    public Boolean getBool() {
        return bool;
    }

    /**
     * Set the bool value of a boolean type list element
     * 
     * @param bool bool value
     */
    public void setBool(Boolean bool) {
        this.bool = bool;
    }
    
    /**
     * Return a string representation of this nested list with given indention.
     * 
     * @param indention the number of blanks to indent
     * @return string representation of this nested list
     */
    public String toString(int indention) {
        String indent = "                                 ".substring(0, indention);
        switch (type) {
            case NL.BOOL:
                return Boolean.TRUE.equals(bool) ? "TRUE" : "FALSE";
            case NL.L:
                StringBuilder sb = new StringBuilder();
                sb.append("\n").append(indent).append("(");
                for (NL l : nl) {
                    sb.append("")
                            .append(l.toString(indention+4))
                            .append(" ");
                }
                sb.append(")");
                return sb.toString();
            case NL.NR:
                return String.format("%.12f", nr);
            case NL.STR:
                return "\""+str+"\"";
            case NL.SYM:
                return ""+sym;
            case NL.UNK:
                return "UNKNOWN";
        }
        
        return "";
    }
    
    /**
     * Return a string representation of this nested list
     * 
     * @return string representation
     */
    @Override
    public String toString() {
        return toString(0);
    }
}
