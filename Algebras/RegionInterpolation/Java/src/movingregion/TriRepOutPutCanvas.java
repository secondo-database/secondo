package movingregion;

import java.awt.*;
import javax.swing.*;


public class TriRepOutPutCanvas extends JPanel
{
    private mLineRep trirep;
    private int maxy,maxx;
    private int timeShift=1;
    
    public TriRepOutPutCanvas(mLineRep tr,int timeShift)
    {
        super();
        trirep = tr;
        maxy = 0;
        maxx = 0;
        this.timeShift=timeShift;
    }
    
    public void setTimeShift(int timeShift)
    {
        this.timeShift=timeShift;
    }
    
    public void setHei(int height)
    {
        this.setSize(this.getHeight(),height);
    }
    
    
    public void setWid(int width)
    {
        this.setSize(width,this.getHeight());
    }
    
    public Dimension getMinimumSize()
    {
        return(new Dimension(10,10));
    }
    
    public Dimension getPreferredSize()
    {
        return(new Dimension(maxx+10,maxy+10));
    }
    
    private void paintPoint(Graphics g,PointWNL p)
    {
        if(p.t==0)
            g.setColor(Color.blue);
        else
            g.setColor(Color.red);
        int x=p.x + (int)(p.t*14*timeShift);
        int y=p.y  +((p.t-1)*14*timeShift)*-1;
        if(maxx<x)
            maxx=x;
        if(maxy<y)
            maxy=y;
        g.fillOval(x-2,y-2,4,4);
    }
    
    private void paintLine(Graphics g , PointWNL p1, PointWNL p2)
    {
        if(p1.t!=p2.t)
            g.setColor(Color.gray);
        if(p1.t==0&&p2.t==0)
            g.setColor(Color.blue);
        if(p1.t!=0&&p2.t!=0)
            g.setColor(Color.red);
        int x1=p1.x + (int)(p1.t*14*timeShift);
        int y1=p1.y  +((p1.t-1)*14*timeShift)*-1;
        int x2=p2.x + (int)(p2.t*14*timeShift);
        int y2=p2.y  +((p2.t-1)*14*timeShift)*-1;
        g.drawLine(x1,y1,x2,y2);
    }
    
    public void paint(Graphics g)
    {
        g.setColor(Color.WHITE);
        g.fillRect(0,0,this.getWidth(),this.getHeight());                
        for(int i=0;i<trirep.triangles.size();i++)
        {
            this.paintLine(g,trirep.getTriangle(i).p1,trirep.getTriangle(i).p2);
            this.paintLine(g,trirep.getTriangle(i).p2,trirep.getTriangle(i).p3);
            this.paintLine(g,trirep.getTriangle(i).p3,trirep.getTriangle(i).p1);
        }
        for(int i=0;i<trirep.triangles.size();i++)
        {
            this.paintPoint(g,trirep.getTriangle(i).p1);
            this.paintPoint(g,trirep.getTriangle(i).p2);
            this.paintPoint(g,trirep.getTriangle(i).p3);
        }
    }
}
