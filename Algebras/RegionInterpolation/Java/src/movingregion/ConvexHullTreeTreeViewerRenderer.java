/*
 * ConvexHullTreeTreeViewerRenderer.java
 *
 * Created on 18. Juli 2007, 23:42
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;


import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import java.awt.*;
import javax.swing.*;
/**
 *
 * @author java
 */
public class ConvexHullTreeTreeViewerRenderer extends DefaultTreeCellRenderer
{
    
    /** Creates a new instance of ConvexHullTreeTreeViewerRenderer */
    public ConvexHullTreeTreeViewerRenderer()
    {
        setOpaque( true );
        
    }
    
    public Component getTreeCellRendererComponent(
            JTree tree, Object value, boolean sel, boolean expanded,
            boolean leaf, int row, boolean hasFocus )
    {
        
        // Die Originalmethode die Standardeinstellungen machen lassen
        super.getTreeCellRendererComponent( tree, value, sel, expanded, leaf, row, hasFocus );
        
        // Den Wert des Knotens abfragen
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
        Object inside = node.getUserObject();
        
        
        if(inside instanceof Region)
        {
            setBackground(Color.YELLOW);
            this.setText("Region with "+((Region)inside).getNrOfFaces()+" Faces");
        }
        
        if(inside instanceof Face)
        {
            setBackground(Color.GREEN);
            this.setText("Face with "+((Face)inside).getCycle().getOutLine().length+" vertices and "+((Face)inside).getNrOfHoles()+" Holes");
        }
        if(inside instanceof ConvexHullTreeNode)
        {
            if(((ConvexHullTreeNode)inside).isHole())
            {             
                setBackground(Color.RED);
                this.setText("Hole-CHN with "+((ConvexHullTreeNode)inside).getOutLine().length+" Vertices");   
            }
            else
            {
                setBackground(new Color(128,128,255));
                this.setText("CHN with "+((ConvexHullTreeNode)inside).getOutLine().length+" Vertices");
            }
        }
        
        if(sel)
        {
            if(!expanded)
            {
                tree.expandRow(row);
            }
            setBackground(Color.ORANGE);
        }
        return this;
    }
    
}