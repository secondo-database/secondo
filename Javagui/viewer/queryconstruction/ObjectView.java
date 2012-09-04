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
package viewer.queryconstruction;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 * An instance of this class is the object component in the ObjectsPane and the MainPane.
 */
public class ObjectView extends JComponent {
    
    private String name;
    private String label;
    private char[] signature;
    private boolean isActive;
    private ArrayList<StreamView> paramStreams = new ArrayList<StreamView>();
    
    private int xpos = 10;
    private int ypos = 10;
    private ObjectType otype;
    private String type;
    private Color color = Color.BLACK;
    
    protected final static char obChar = 'o';
    protected final static char opChar = '#';
    protected final static char pChar = 'p';
    
    public ObjectView(){
        this.setOpaque(false);
        this.setPreferredSize(new Dimension(120, 70));
    }
    
    public ObjectView(String name, String type){
        this.name = name;
        this.label = name;
        this.type = type;
    }
    
    public ObjectView(ListExpr list){
        //generate an instance of ObjectType
        otype = new ObjectType(list);
        
        this.name = otype.getName();
        this.type = otype.getType();
        this.label = this.name;
        
    }
    
    protected void addParamStream(StreamView stream) {
        paramStreams.add(stream);
    }
    
    protected ObjectView copy(String label){
        ObjectView newObject = new ObjectView(this.name, this.type);
        newObject.setLabel(label);
        if (this.otype != null)
            newObject.setOType(otype);
        
        return newObject;
    }
    
    /** paints a Secondo ObjectView into the RelationsPane
     height 90, width 50*/
    protected void paintComponent(Graphics g){
        
        g.setColor(this.color);
        
        if (type.equals(ObjectType.OPERATION)) {
            g.drawOval(xpos, ypos, 90, 50);
        }
        else {
            g.drawRect(xpos, ypos, 90, 50);
            
            if (type.equals(ObjectType.RELATION)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/relation.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }

            if (type.equals(ObjectType.MPOINT)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/mpoint.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type.equals(ObjectType.POINT)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/point.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type.equals(ObjectType.REGION)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/region.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type.equals(ObjectType.MREGION)) {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/mregion.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
        }
        
        int w = g.getFontMetrics().stringWidth(label);
        String s = label;
        if (w > 90) {
            s = label.substring(0, 10);
            w = g.getFontMetrics().stringWidth(s);
        }
        
        g.drawString(s, xpos + 45 - w/2, ypos + 30);
    }
    
    /** 
     * paints a Secondo ObjectView into the mainPane
     */
    public void paintComponent(Graphics g, int x, int y, Color color){
        this.xpos = 10 + x*120;
        this.ypos = 10 + y*80;
        this.color = color;
        
        paintComponent(g);
    }
    
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
                    if (pS < paramStreams.size()){
                        for (Iterator iter = paramStreams.get(pS).getObjects().iterator(); iter.hasNext();) {
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
    
    protected String getOnlyName() {
        return this.name;
    }
    
    protected StreamView getParamStream(int index){
        if (paramStreams.size() > index)
            return paramStreams.get(index);
        return null;
    }
    
    protected String getConst(){
        String result = "[const ";
        result += this.getType() + " value undef]";
        
        return result;
    }
    
    protected String getLabel(){
        return label;
    }
    
    protected void setLabel(String label) {
        this.label = label;
    }
    
//    private void setObjectName(String name) {
//        this.name = name;
//        if (name.length() < 12) {
//            setLabel(name);
//        }
//    }
    
    public String getType(){
        return type;
    }
    
    public boolean isSecondoObject(){
        return (otype != null);
    }
    
    protected boolean isActive() {
        return isActive;
    }
    
    public ObjectType getOType() {
        return otype;
    }
    
    protected void setActive(boolean active){
        this.isActive = active;
    }
    
    protected void setOType(ObjectType otype) {
        this.otype = otype;
    }
    
    protected void setSignature(String sig) {
        this.signature = sig.toCharArray();
    }
}
