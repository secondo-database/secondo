package movingregion;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;

public class tegnecanvas extends JPanel implements ActionListener
{
    private MCIContents myParent;
    private boolean addHole=false;
    private boolean addFace=true;
    private JToolBar drawUtils;
    private JButton reDrawButt;
    private JButton drawNextSS;
    private JButton addHoleButt;
    private JButton addFaceButt;
    private Vector fistSnapPoints;   // The points in the first snapshot
    private int hei;
    private int wid;
    private int avtiveSnap=0;
    private Region[] Snaps=new Region[2];
    
    public tegnecanvas(MCIContents myParent)
    {
        Snaps[0]=new Region();
        Snaps[1]=new Region();
        this.myParent=myParent;
        setLayout(new BorderLayout());
        reDrawButt = new JButton("Draw");
        reDrawButt.addActionListener(this);
        addFaceButt = new JButton("addFace");
        addFaceButt.addActionListener(this);
        addHoleButt = new JButton("addHole");
        addHoleButt.addActionListener(this);
        drawNextSS = new JButton("Store Snapshot");
        drawNextSS.addActionListener(this);
        drawUtils=new JToolBar();
        drawUtils.add(reDrawButt);
        drawUtils.add(addFaceButt);
        drawUtils.add(addHoleButt);
        drawUtils.add(drawNextSS);
        add(drawUtils,BorderLayout.NORTH);
        enableEvents(AWTEvent.MOUSE_EVENT_MASK);
        fistSnapPoints = new Vector();
    }
    
    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == reDrawButt)
        {
            Snaps[0]=new Region();
            Snaps[1]=new Region();
            fistSnapPoints.removeAllElements();
            avtiveSnap=0;
            this.addFace=true;
            this.addHole=false;
            myParent.newDraw();
        }
        if (e.getSource() == drawNextSS)
        {
            this.finishLastOperation();
            avtiveSnap++;
            this.addFace=true;
            this.addHole=false;
            if(avtiveSnap==2)
            {                                
                myParent.store();                
            }
        }        
        if (e.getSource() == addFaceButt)
        {
            this.finishLastOperation();
            this.addFace=true;
            this.addHole=false;
        }                
        if (e.getSource() == addHoleButt)
        {
            this.finishLastOperation();
            this.addFace=false;
            this.addHole=true;
        }
        
        this.repaint();
    }
    
    protected void processMouseEvent(MouseEvent e)
    {
        int hid;
        Point punkt;
        hid = e.getID();
        if ((hid == MouseEvent.MOUSE_CLICKED) && (avtiveSnap!=2 ))
        {
            // Tegner opp et nytt punkt med linje.
            punkt = e.getPoint();
            fistSnapPoints.addElement(punkt);
            repaint();
        }
        super.processMouseEvent(e);
    }
    
    public void setWid(int width)
    {
        wid=width;
    }
    
    public void setHei(int heigth)
    {
        hei=heigth;
    }
    
    public void finishLastOperation()
    {
        if(this.avtiveSnap<=1&&fistSnapPoints.size()!=0)
        {
            LineWA[] tmp=new LineWA[fistSnapPoints.size()];
            for (int i=0;i<fistSnapPoints.size();i++)
            {
                Point tmpP=(Point)fistSnapPoints.elementAt(i);
                tmp[i]=new LineWA(tmpP.x,tmpP.y);
            }
            if(this.addFace)
            {
                Snaps[avtiveSnap].addFace(new Face(tmp));
            }
            if(this.addHole)
            {
                Snaps[avtiveSnap].getFace(Snaps[avtiveSnap].getNrOfFaces()-1).addHole(tmp);
            }
            fistSnapPoints.removeAllElements();
        }                
    }
    
    public Dimension getMinimumSize()
    {
        return(new Dimension(100,100));
    }
    
    public Dimension getPreferredSize()
    {
        return(new Dimension(wid,hei));
    }
    
    public Region getFirstSnapshot()
    {
        return(Snaps[0]);
    }
    
    public Region getSecondSnapshot()
    {
        return(Snaps[1]);
    }
    
    public void paint(Graphics g)
    {
        super.paint(g);
        if(avtiveSnap>=1)
        {
            Snaps[0].paintRegion(g,false);
            Snaps[1].paintRegion(g,true);
        }
        else
        {
            Snaps[0].paintRegion(g,true);
        }       
        int antallpunkt;
        Dimension storrelse;
        Point nvpunkt,gpunkt;
        storrelse = this.getSize();
        g.setColor(Color.black);
        g.drawRect(0,0,storrelse.width-1, storrelse.height-1);
        
        antallpunkt = fistSnapPoints.size();
        if (antallpunkt >0)
        {
            if(this.addFace)
                g.setColor(Color.BLUE);
            if(this.addHole)
                g.setColor(Color.RED);
            gpunkt = (Point)fistSnapPoints.firstElement();
            g.fillOval((gpunkt.x)-4, (gpunkt.y)-4, 8, 8);
            for (int i=1;i<antallpunkt;i++)
            {
                nvpunkt = (Point)fistSnapPoints.elementAt(i);
                g.drawLine(gpunkt.x, gpunkt.y, nvpunkt.x, nvpunkt.y);
                g.fillOval((nvpunkt.x)-4, (nvpunkt.y)-4, 8, 8);
                gpunkt = nvpunkt;
            }
        }
    }
}
