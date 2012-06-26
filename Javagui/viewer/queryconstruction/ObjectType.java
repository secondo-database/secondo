/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import sj.lang.ListExpr;
import java.util.ArrayList;
import java.util.Iterator;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class ObjectType {
    
    private String name;
    private String type;
    private ListExpr list;
    
    // define supported types
    protected static final String OPERATION = "operation";
    protected static final String TRELATION = "trel";
    protected static final String RELATION = "rel";
    protected static final String MPOINT = "mpoint";
    protected static final String POINT = "point";
    protected static final String REGION = "region";
    protected static final String MREGION = "mregion";
    
    public ObjectType(ListExpr obj){
        
        this.list = obj.fourth().first();
        setName(obj);
        setType(obj);
        
    }
    
    public ObjectType(String name, ListExpr list){
        
        this.list = list;
        this.name = name;
        this.type = list.first().toString();
        
    }
    
    public void setName(ListExpr obj) {
        name = obj.second().stringValue();
    }
    
    public String getName() {
        return name;
    }
    
    public void setType(ListExpr obj) {
        String type = "";
        //the object can be of atom type or a relation
        if (obj.fourth().first().isAtom()) {
            type = obj.fourth().first().symbolValue();
        }
        else {
            type = obj.fourth().first().first().symbolValue();
        }
        this.type = type;
    }
    
    public String getType() {
        return type;
    }
    
    public String[] getAttributes() {
        if (type.equals(RELATION)) {
            ListExpr attlist = list.second().second();
            String attarray[] = new String[attlist.listLength()];
            int i = 0;
            while (attlist.listLength() > 0) {
                ListExpr att = attlist.first();
                attarray[i] = att.first().stringValue();
                attlist = attlist.rest();
                i++;
            }
            return attarray;
        }
        else return null;
    }
    
    public String[] getAttrTypes() {
        if (type.equals(RELATION)) {
            ListExpr attlist = list.second().second();
            String attarray[] = new String[attlist.listLength()];
            int i = 0;
            while (attlist.listLength() > 0) {
                ListExpr att = attlist.first();
                attarray[i] = att.second().stringValue();
                attlist = attlist.rest();
                i++;
            }
            return attarray;
        }
        else return null;
    }
}
