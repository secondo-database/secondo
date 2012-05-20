/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import gui.SecondoObject;
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
    private String type;
    private boolean active;
    private Color color = Color.BLACK;
    private SecondoObject so;
    
    public ObjectView(String type, String name){
        
        this.name = name;
        this.label = name;
        this.type = type;    
        //this.so = so;
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
        
        if (type == "operation") {
            g.drawOval(xpos, ypos, 90, 50);
        }
        else {
            g.drawRect(xpos, ypos, 90, 50);
            
            if (type == "rel") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/relation.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }

            if (type == "mpoint") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/mpoint.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "point") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/point.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "region") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/region.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "mregion") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/mregion.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
        }
        
        
        
        int w = g.getFontMetrics().stringWidth(label);
        String s = label;
        if (w > 80) {
            s = label.substring(0, 8);
        }
        
        g.drawString(s, xpos + 25, ypos + 30);
    }
    
    public String getName() {
        return this.name;
    }
    
    public void setName(String name) {
        this.name = name;
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
