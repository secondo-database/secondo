/*
 * MatchViewer.java
 *
 * Created on 15. August 2007, 00:40
 */

package movingregion;

/**
 *
 * @author  java
 */

import java.util.*;
import java.awt.*;

public class MatchViewer extends javax.swing.JPanel
{
    Match myMatch;
    ConvexHullTreeViewer source;
    ConvexHullTreeViewer target;
    /** Creates new form MatchViewer */
    public MatchViewer(Match myMatch)
    {
        this.myMatch=myMatch;
        initComponents();
        source=new ConvexHullTreeViewer(myMatch.getSource(),this);
        target=new ConvexHullTreeViewer(myMatch.getTarget(),this);
        this.regions.add(source);
        this.regions.add(target);
    }
    
    public void changeSelection(RegionTreeNode[] nodes)
    {
        Vector tmp=new Vector();
        for(int i=0;i<nodes.length;i++)
        {
            if(myMatch.getMatches(nodes[i])!=null)
            {
                for(int j=0;j<myMatch.getMatches(nodes[i]).length;j++)
                {
                    tmp.add(myMatch.getMatches(nodes[i])[j]);
                }
            }
            
        }
        RegionTreeNode[] res=new RegionTreeNode[tmp.size()];
        for(int i=0;i< tmp.size();i++)
        {
            res[i]=(RegionTreeNode)tmp.elementAt(i);
        }
        if(source.contains(nodes[0]))
            target.setActual(res);
        else
            source.setActual(res);
        this.repaint();
    }
    
    public void setHei(int height)
    {
        source.setHei((height-showName.getHeight()-40)/2);
        target.setHei((height-showName.getHeight()-40)/2);
        regions.setSize(regions.getWidth(), height-showName.getHeight()-40);
        
    }
    
    
    public void setWid(int width)
    {
        source.setWid(width-13);
        target.setWid(width-13);
        regions.setSize(width-13, regions.getHeight());
    }
    
    public void paint(Graphics g)
    {
        super.paint(g);
        g.setFont(new Font("SansSerif",Font.BOLD,14));
        FontMetrics fm=g.getFontMetrics();
        int y=0;//(int)(fm.getMaxAscent()*1.5);
        int maxStringWidth=this.getWidth()/4;
        y=showName.getHeight();
        if(showName.isSelected())
        {
            fm=g.getFontMetrics();

            String myName=this.myMatch.getName();
            String[] splitname=myName.split("§");
            for(int i=0;i<splitname.length;i++)
            {
                y=y+(int)(fm.getMaxAscent()*1.5);
                g.drawString(splitname[i],this.getWidth()-fm.stringWidth(splitname[i])-jScrollPane1.getVerticalScrollBar().getWidth()-10,y);            
            }
            g.setFont(new Font("SansSerif",Font.PLAIN,12));
            fm=g.getFontMetrics();        
            String Descrip=myMatch.getDescription();
            do
            {
                y=y+(int)(fm.getMaxAscent()*1.5);
                String tmp="";
                while (fm.stringWidth(tmp)<maxStringWidth)
                {
                    if(Descrip.indexOf(" ")!=-1)
                    {
                        tmp=tmp+" "+ Descrip.substring(0,Descrip.indexOf(" "));
                        Descrip=Descrip.substring(Descrip.indexOf(" ")+1);
                    }
                    else
                    {
                        tmp=tmp+" "+ Descrip;
                        Descrip="";
                    }
                }
                g.drawString(tmp,this.getWidth()-fm.stringWidth(tmp)-jScrollPane1.getVerticalScrollBar().getWidth()-10,y);            
            }
            while(Descrip.length()>0);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jScrollPane1 = new javax.swing.JScrollPane();
        regions = new javax.swing.JPanel();
        showName = new javax.swing.JCheckBox();

        setLayout(new java.awt.BorderLayout());

        regions.addComponentListener(new java.awt.event.ComponentAdapter() {
            public void componentMoved(java.awt.event.ComponentEvent evt) {
                regionsComponentMoved(evt);
            }
        });
        regions.setLayout(new java.awt.GridLayout(2, 0));
        jScrollPane1.setViewportView(regions);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

        showName.setSelected(true);
        showName.setText("show Match-Name");
        showName.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                showNameActionPerformed(evt);
            }
        });
        add(showName, java.awt.BorderLayout.PAGE_START);
    }// </editor-fold>//GEN-END:initComponents
    
    private void regionsComponentMoved(java.awt.event.ComponentEvent evt)//GEN-FIRST:event_regionsComponentMoved
    {//GEN-HEADEREND:event_regionsComponentMoved
        
        this.repaint();
    }//GEN-LAST:event_regionsComponentMoved

private void showNameActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_showNameActionPerformed
this.repaint();
}//GEN-LAST:event_showNameActionPerformed
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JPanel regions;
    private javax.swing.JCheckBox showName;
    // End of variables declaration//GEN-END:variables
    
}
