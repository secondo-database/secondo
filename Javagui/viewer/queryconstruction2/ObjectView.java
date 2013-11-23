//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.queryconstruction2;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer2;

/**
 * An instance of this class is the object 
 * component in the ObjectsPane and the MainPane or an
 * operation component in the MainPane.
 */
public class ObjectView extends JComponent {
    
    private String name;
    private String label;
    private char[] signature;
    private boolean isActive;
    private ArrayList<StreamView> paramStreams = new ArrayList<StreamView>();
    
    private int xpos = 10;
    private int ypos = 10;
    private ListExpr list;
    private String type;
    private Color color = Color.BLACK;
    
    // define supported object types
    protected static final String OPERATION = "operation";
    protected static final String TRELATION = "trel";
    protected static final String RELATION = "rel";
    protected static final String MPOINT = "mpoint";
    protected static final String POINT = "point";
    protected static final String REGION = "region";
    protected static final String MREGION = "mregion";
    
    protected final static char obChar = 'o';
    protected final static char opChar = '#';
    protected final static char pChar = 'p';
    
    public ObjectView(){
        this.setOpaque(false);
        this.setPreferredSize(new Dimension(120, 70));
    }
    
    protected ObjectView(String name, String type){
        this.name = name;
        this.label = name;
        this.type = type;
    }
    
    protected ObjectView(ListExpr list){
        
        this.list = list.fourth().first();
        
        name = list.second().stringValue();
        
        //the object can be of atom type or a relation
        if (list.fourth().first().isAtom()) {
            type = list.fourth().first().symbolValue();
        }
        else {
            type = list.fourth().first().first().symbolValue();
        }
        
        this.label = this.name;
        
    }
    
    /**
     * Add a stream as input parameter.
     * @param stream 
     */
    protected void addParamStream(StreamView stream) {
        paramStreams.add(stream);
    }
    
    /**
     * Generate a copy of the object.
     * @param label
     * @return 
     */
    protected ObjectView copy(String label){
        ObjectView newObject = new ObjectView(this.name, this.type);
        newObject.setLabel(label);
        
        return newObject;
    }
    
    /**
     * Return a constant copy of the object, if it is an attribute.
     * @param onlyName return only the name and don't process the signature
     * @return 
     */
    protected String getConst(boolean onlyName){
        String result = "";
        if ((this.name.startsWith(".") || this.name.startsWith("attr")) 
                && !this.type.equals("param")) {
            result = "[const ";
            result += this.getType() + " value undef]";
        }
        else {
            if (onlyName)
                return this.name;
            else
                return this.getObjectName().trim() + " ";
        }
        return result;
    }
    
    protected String getLabel(){
        return label;
    }
    
    /**
     * Get the name or the result string of the object.
     * If it is an Operator, use the signature.
     * @return 
     */
    protected String getObjectName() {
        if (signature == null)
            return name;
        
        String result = "";
        int pS = 0;
        
        for (char c : signature) {
            switch(c) {
                case obChar: 
                    break;
                case opChar: 
                    result += this.name;
                    break;
                case pChar: 
                    // use the parameter objects.
                    if (pS < paramStreams.size()){
                        for (Iterator iter = paramStreams.get(pS).getObjects()
                                .iterator(); iter.hasNext();) {
                            ObjectView object = (ObjectView) iter.next();
                            result += object.getObjectName().trim();
                        }
                        pS++;
                    }
                    break;
                default:
                    result += c;
                    break;
            }
        }
        if (result.length() < 12)
            this.label = result;
        
        return result;
    }
    
    /**
     * Get only the object name.
     * @return 
     */
    protected String getOnlyName() {
        return this.name;
    }
    
    /**
     * @return type
     */
    protected String getType(){
        return type;
    }
    
    /**
     * Get the nested list, the object is generated of.
     * @return 
     */
    protected String getViewString() {
        if (this.list != null)
            return this.list.toString();
        else return this.getType();
    }
    
    /**
     * @return true, if the object should be visible
     */
    protected boolean isActive() {
        return isActive;
    }
    
    /**
     * Change the name and the label of the object.
     * @param name 
     */
    protected void rename(String name){
        this.name = name.replace(".", "");
        if (name.length() < 12)
            this.label = name;
        else
            label = label.replace(".", "");
    }
    
    protected void setActive(boolean active){
        this.isActive = active;
    }
    
    protected void setLabel(String label) {
        this.label = label;
    }
    
    protected void setSignature(String sig) {
        this.signature = sig.toCharArray();
    }
    
    /** 
     * Paint an ObjectView into the mainPane.
     */
    protected void paintComponent(Graphics g, int x, int y){
        this.xpos = 10 + x*120;
        this.ypos = 10 + y*80;
        
        paintComponent(g);
    }
    
    /** 
     * Paint an ObjectView object into the MainPane or the ObjectPane.
     * height 90, width 50
     */
    protected void paintComponent(Graphics g){
        g.setColor(Color.black);
        
        if (type.equals(OPERATION)) {
            g.drawOval(xpos, ypos, 90, 50);
        }
        else {
            g.drawRect(xpos, ypos, 90, 50);
            
            if (type.equals(RELATION)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer2
                        .class.getResource(
                        "queryconstruction2/images/relation.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }

            if (type.equals(MPOINT)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer2
                        .class.getResource(
                        "queryconstruction2/images/mpoint.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type.equals(POINT)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer2
                        .class.getResource(
                        "queryconstruction2/images/point.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type.equals(REGION)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer2
                        .class.getResource(
                        "queryconstruction2/images/region.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type.equals(MREGION)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer2
                        .class.getResource(
                        "queryconstruction2/images/mregion.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
        }
        
        int w = g.getFontMetrics().stringWidth(label);
        String s = label;
        if (w > 90) {
            s = label.substring(0, 5) + "..." +
                    label.substring(label.length()-5, label.length());
            w = g.getFontMetrics().stringWidth(s);
        }
        
        g.drawString(s, xpos + 45 - w/2, ypos + 30);
    }
}
