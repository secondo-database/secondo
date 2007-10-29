/*
 * ConvexHullTreeTreeViewerNode.java
 *
 * Created on 10. Juli 2007, 23:55
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package movingregion;

import javax.swing.tree.*;
/**
 *
 * @author java
 */
public class ConvexHullTreeTreeViewerNode extends DefaultMutableTreeNode
{
    ConvexHullTreeNode myNode;
    
    /** Creates a new instance of ConvexHullTreeTreeViewerNode */
    public ConvexHullTreeTreeViewerNode(ConvexHullTreeNode node)
    {
        super("CHN mit "+node.getOutLine().length+" Ecken");        
        myNode=node;
        ConvexHullTreeNode[] tmp=myNode.getChildren();
        for(int i=0;i<tmp.length;i++)
        {
            this.add(new ConvexHullTreeTreeViewerNode(tmp[i]));
        }
    }
    
    public ConvexHullTreeNode getNode()
    {
        return(myNode);
    }
}
