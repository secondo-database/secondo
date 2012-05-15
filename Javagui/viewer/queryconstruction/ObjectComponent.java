/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package viewer.queryconstruction;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.lang.Thread;
import viewer.QueryconstructionViewer;

/**
 *
 * @author lrentergent
 */
public class ObjectComponent extends JComponent implements MouseListener {
    
    private String name;
    private int xpos;
    private int ypos;
    private String type;
    private boolean active;
    
    public ObjectComponent(String type, String name){
        
        this.setPreferredSize(new Dimension(90,50));
        this.name = name;
        this.type = type;        
        this.active = false;
        addMouseListener(this);
        
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
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/relation.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }

            if (type == "mpoint") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/images/mpoint.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "point") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/images/point.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "region") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/images/region.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
            
            if (type == "mregion") {
                ImageIcon icon = new ImageIcon(QueryconstructionViewer.class.getResource("queryconstruction/images/images/mregion.gif"));
                g.drawImage(icon.getImage(), xpos + 5, ypos + 5, this);
            }
        }
        
        if (active) {
            g.setColor(Color.RED);
        }
        
        g.drawString(name, xpos + 25, ypos + 30);
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
    
    //Handle mouse events.
    public void mouseReleased(MouseEvent e) {
    }
    public void mouseClicked ( final MouseEvent arg0 ) {
        if (arg0.getClickCount () == 2) {
            System.out.println("es wurde doppelgeklickt");
        }
    }
    public void mouseEntered(MouseEvent e){}
    public void mouseExited(MouseEvent e){}
    public void mousePressed(MouseEvent e){}
}
