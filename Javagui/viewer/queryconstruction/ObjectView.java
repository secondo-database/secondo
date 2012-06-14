/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import javax.swing.*;
import sj.lang.ListExpr;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class ObjectView extends JComponent {
    
    private String name;
    private String label;
    private int xpos;
    private int ypos;
    private ObjectType otype;
    private String type;
    private boolean active;
    private Color color = Color.BLACK;
    
    public ObjectView(String type, String name){
        
        this.name = name;
        this.type = type;
        
        this.label = this.name;
        this.active = false;
        
    }
    
    /** paints a Secondo ObjectView into the RelationsPane
     height 80, width 50*/
    public void paintComponent(Graphics g, int x, int y, Color color){
        this.xpos = 10 + x*120;
        this.ypos = 10 + y*70;
        
        if (color != null)
            g.setColor(color);
        else g.setColor(this.color);
        
        if (type == ObjectType.OPERATION) {
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
        if (w > 80) {
            s = label.substring(0, 10);
        }
        
        g.drawString(s, xpos + 45 - w/2, ypos + 30);
    }
    
    public void paintTable(Graphics g, int x, int y, Color color){
        this.xpos = 10 + x*120;
        this.ypos = 10 + y*30;
        
        if (color != null)
            g.setColor(color);
        else g.setColor(this.color);
        
        g.drawLine(xpos - 10, ypos + 20, xpos + 120, ypos + 20);
        
        int w = g.getFontMetrics().stringWidth(label);
        String s = label;
        if (w > 80) {
            s = label.substring(0, 12);
        }
        
        g.drawString(s, xpos + 10, ypos + 10);
    }
    
    public ObjectType getOType() {
        return otype;
    }
    
    public void setOType(ObjectType otype) {
        this.otype = otype;
    }
    
    public String[] getAttributes() {
        for (String s : otype.getAttributes()) {
            System.out.println(s);
        }
        return otype.getAttributes();
    }
    
    public String getName() {
        return this.name;
    }
    
    public void setName(String name) {
        this.name = name;
        if (name.length() < 12) {
            label = name;
        }
    }
    
    public String getType(){
        return type;
    }
    
    public boolean isActive() {
        return active;
    }
    
    public void setActive() {
        color = Color.BLACK;
        active = true;
    }
    
    public void setUnactive() {
        color = Color.GRAY;
        active = false;
    }
}
