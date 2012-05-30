/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import viewer.QueryConstructionViewer;

/**
 *
 * @author lrentergent
 */
public class ObjectComponent extends Component {
    
    private String name;
    private int xpos;
    private int ypos;
    private String type;
    private boolean active;
    
    public ObjectComponent(String type, String name){
        
        this.setPreferredSize(new Dimension(120,70));
        this.name = name;
        this.type = type;        
        this.active = false;
        
    }
    
    /** paints a Secondo ObjectComponent into the RelationsPane
     height 80, width 50*/
    public void paintComponent(Graphics g, int x, int y){
        this.xpos = 10 + x*120;
        this.ypos = 10 + y*70;
        
        g.setColor(Color.BLACK);
        
        if (type == "operation") {
            g.drawOval(xpos, ypos, 90, 50);
        }
        else {
            g.drawRect(xpos, ypos, 90, 50);
            
            if (type == "rel") {
                ImageIcon icon = new ImageIcon(QueryConstructionViewer.class.getResource("queryconstruction/images/relation.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }

            if (type == "mpoint") {
                ImageIcon icon = new ImageIcon(QueryConstructionViewer.class.getResource("queryconstruction/images/mpoint.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "point") {
                ImageIcon icon = new ImageIcon(QueryConstructionViewer.class.getResource("queryconstruction/images/point.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "region") {
                ImageIcon icon = new ImageIcon(QueryConstructionViewer.class.getResource("queryconstruction/images/region.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "mregion") {
                ImageIcon icon = new ImageIcon(QueryConstructionViewer.class.getResource("queryconstruction/images/mregion.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
        }
        
        if (active) {
            g.setColor(Color.RED);
        }
        
        int w = g.getFontMetrics().stringWidth(name);
        String s = name;
        if (w > 80) {
            s = name.substring(0, 8);
        }
        
        g.drawString(s, xpos + 25, ypos + 30);
    }
    
    public String getName() {
        return this.name;
    }
    
    public boolean isActive() {
        return active;
    }
    
    public void setActive() {
        active = true;
    }
    
    public void setUnactive() {
        active = false;
    }
}
