/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.*;
import javax.swing.JTextArea;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class ObjectView extends JComponent {
    
    private String name;
    private String label;
    private char[] signature;
    private boolean isActive;
    private boolean isOperation;
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
    
    public ObjectView(String type, String name){
        
        this.name = name;
        this.label = name;
        this.type = type;
        
        this.isOperation = type.equals(ObjectType.OPERATION);
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
        this.setObjectName();
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
        return this.name;
    }
    
    protected String getConst(){
        String result = "[const ";
        result += this.getType() + " value undef]";
        
        return result;
    }
    
    protected void setLabel(String label) {
        this.label = label;
    }
    
    protected void setObjectName() {
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
                        result += paramStreams.get(pS).getTypeString();
                    }
                    pS++;
                    break;
                default:
                    result += c;
                    break;
            }
        }
        
        name = result;
        if (name.length() < 12) {
            setLabel(name);
        }
    }
    
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
    
    public void setOType(ObjectType otype) {
        this.otype = otype;
    }
    
    protected void setSignature(String sig) {
        this.signature = sig.toCharArray();
    }
}
