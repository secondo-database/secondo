

package movingregion;
import java.awt.*;


public class ConvexHullTreePolygonViewer extends javax.swing.JPanel
{
    Region root;
    RegionTreeNode[] actual=null;
    int hei=200;
    int wid=400;
    int maxX=0000;
    int maxY=0000;
    int minX=100000;
    int minY=100000;
    
    /** Creates new form ConvexHullTreeViewer */
    public ConvexHullTreePolygonViewer(Region root)
    {
        this.root=root;
        for(int i=0;i<root.getNrOfFaces();i++)
        {
            LineWA[] tmp=root.getFace(i).getCycle().getOutLine();
            for(int j=0;j<tmp.length;j++)
            {
                if(tmp[j].x>maxX)   maxX=tmp[j].x;
                if(tmp[j].y>maxY)   maxY=tmp[j].y;
                if(tmp[j].x<minX)   minX=tmp[j].x;
                if(tmp[j].y<minY)   minY=tmp[j].y;
            }
        }
        
    }
    
    public ConvexHullTreePolygonViewer(Region root,RegionTreeNode[] actual)
    {
        this(root);
        this.actual=actual;
    }
    public void setHei(int height)
    {
        hei=height;
    }
    
    public void setWid(int width)
    {
        wid=width;
    }
    
    public void setActual(RegionTreeNode[] actual)
    {
        this.actual=actual;
        this.repaint();
    }
    
    public void drawPolygonFromNode(Graphics g,Color c, ConvexHullTreeNode node )
    {
        double scaleX=wid/((maxX-minX)*1.0);
        double scaleY=hei/((maxY-minY)*1.0);
        g.setColor(c);
        Polygon tmp =new Polygon();
        LineWA[]tmpli=node.getOrderedOutLine();
        for(int i=0;i<tmpli.length;i++)
        {
            tmp.addPoint((int)((tmpli[i].x-minX)*scaleX),(int)((tmpli[i].y-minY)*scaleY));
            if(c==Color.ORANGE)
            {
                g.drawString(i+"",(int)((tmpli[i].x-minX)*scaleX),(int)((tmpli[i].y-minY)*scaleY));
                g.setColor(Color.BLUE);
                g.fillOval((int)((node.getCenter().x-minX)*scaleX)-2,(int)((node.getCenter().y-minY)*scaleY)-2,4,4);
                g.setColor(Color.GREEN);
                g.fillOval((int)((node.getSteinerPoint().x-minX)*scaleX)-2,(int)((node.getSteinerPoint().y-minY)*scaleY)-2,4,4);
                g.setColor(Color.ORANGE);
            }
        }
        g.drawPolygon(tmp);
        
    }
    
    public void drawNode(Graphics g, Color c, ConvexHullTreeNode node)
    {
        drawPolygonFromNode(g,c,node);
        ConvexHullTreeNode[] children=node.getChildren();
//        if(c==Color.ORANGE)
//            c=Color.BLUE;
        for(int i=0;i<children.length;i++)
        {
            drawNode(g,c,children[i]);
        }
    }
    
    public void drawFace(Graphics g, Color c, Face node)
    {
        
        ConvexHullTreeNode[] children=node.getCycle().getChildren();
        for(int i=0;i<children.length;i++)
        {
            
            if(c==null)
            {
                drawNode(g,Color.BLUE,children[i]);
            }
            else
            {
                drawNode(g,c,children[i]);
            }
        }
        for(int i=0;i<node.getNrOfHoles();i++)
        {
            
            if(c==null)
            {
                drawPolygonFromNode(g,Color.RED,node.getHole(i));
            }
            else
            {
                drawPolygonFromNode(g,c,node.getHole(i));
            }
            for(int j=0;j<node.getHole(i).getChildren().length;j++)
            {
                if(c==null)
                {
                    drawNode(g,Color.RED,node.getHole(i).getChildren()[j]);
                }
                else
                {
                    drawNode(g,c,node.getHole(i).getChildren()[j]);
                }
            }
        }
        if(c==null)
        {
            g.setColor(Color.GREEN);
        }
        else
        {
            g.setColor(c);
        }
        double scaleX=wid/((maxX-minX)*1.0);
        double scaleY=hei/((maxY-minY)*1.0);
        Polygon tmp =new Polygon();
        LineWA[]tmpli=node.getCycle().getLines();
        for(int i=0;i<tmpli.length;i++)
        {
            tmp.addPoint((int)((tmpli[i].x-minX)*scaleX),(int)((tmpli[i].y-minY)*scaleY));
            if(c==Color.ORANGE)
            {
                g.drawString(i+"",(int)((tmpli[i].x-minX)*scaleX),(int)((tmpli[i].y-minY)*scaleY));
                g.setColor(Color.BLUE);
                g.fillOval((int)((node.getCycle().getCenter().x-minX)*scaleX)-2,(int)((node.getCycle().getCenter().y-minY)*scaleY)-2,4,4);
                g.setColor(Color.GREEN);
                g.fillOval((int)((node.getCycle().getSteinerPoint().x-minX)*scaleX)-2,(int)((node.getCycle().getSteinerPoint().y-minY)*scaleY)-2,4,4);
                g.setColor(Color.ORANGE);
            }
        }
        g.drawPolygon(tmp);
    }
    
    public void paint(Graphics g)
    {
        g.setColor(Color.WHITE);
        g.fillRect(0,0,this.getWidth(),this.getHeight());        
        for(int j=0;j<root.getNrOfFaces();j++)
        {
            drawFace(g,null,root.getFace(j));
        }
        if(actual!=null)
        {
            for(int i=0;i<actual.length;i++)
            {
                if(actual[i] instanceof ConvexHullTreeNode)
                    drawPolygonFromNode(g,Color.ORANGE,(ConvexHullTreeNode)actual[i]);
                
                if(actual[i] instanceof Face)
                    drawPolygonFromNode(g,Color.ORANGE,((Face)actual[i]).getCycle());
            }
        }
    }
    
    public Dimension getPreferredSize()
    {
        return(new Dimension(wid,hei));
    }
        
    public Dimension getMaximumSize() 
    {
        
        return(new Dimension(wid,hei));
    }
    
    public Dimension getMinimumSize() 
    {
        return(new Dimension(wid,hei));
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents()
    {

        setLayout(new java.awt.GridLayout());

    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    // End of variables declaration//GEN-END:variables
    
}
