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
    private String[] paramValue;
    private int paramAt = 0;
    
    private int xpos = 10;
    private int ypos = 10;
    private ObjectType otype;
    private String type;
    private boolean active = false;
    private Color color = Color.BLACK;
    
    public ObjectView(){
        this.setOpaque(false);
        this.setPreferredSize(new Dimension(120, 70));
    }
    
    public ObjectView(String type, String name){
        
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
    
    protected void addParam(String param) {
        paramValue[paramAt] = param;
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
    
    /** paints a Secondo ObjectView into the mainPane
     * 
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
    
    protected void setObjectName(String name) {
        this.name = name;
        if (name.length() < 12) {
            label = name;
        }
    }
    
    protected String getParam(int i){
        
        if (paramValue != null)
            return paramValue[i];
        else
            return null;
    }
    
    public String getType(){
        return type;
    }
    
    public boolean isSecondoObject(){
        return (otype != null);
    }
    
    public ObjectType getOType() {
        return otype;
    }
    
    public void setOType(ObjectType otype) {
        this.otype = otype;
    }
    
    public boolean isActive() {
        return active;
    }
    
    public void setActive(boolean active) {
        color = Color.BLACK;
        this.active = active;
    }
    
    protected void setParams(String[] params) {
        paramValue = params;
    }
}
